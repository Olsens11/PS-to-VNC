#!/usr/bin/env python3
"""
File synopsis:
    Owns CP2P item #8's one generation-scoped Pi MPEG producer.

A valid START has already established immutable generation state, pending RFB
suppression, and an exact START-derived capture plan before this owner is called.
This module launches that exact FFmpeg command and owns the process/stdout spool
until exact retirement.

The public item-#10 MPEG CONFIG gate deliberately remains closed. Therefore this
owner does NOT write PSTV frames and does not consume MPEG channel credit. Output
is archived and held in the existing bounded ProducerBuffer; when its bounded
unsent queue fills, normal pipe/TCP-style backpressure stalls FFmpeg rather than
creating an unbounded queue.

Retirement is exact-generation and stronger than merely signaling FFmpeg. It
requests process stop, drains/discards every locally buffered unsent MPEG byte so
the ProducerBuffer reader can reach EOF, proves the reader thread/process are
quiescent, and only then releases producer ownership. Item #11A calls this before
removing RFB suppression or acknowledging retirement to the PS2.
"""

from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import subprocess
import time
from typing import Callable

import h1_mux_server as base
from h1_cp2p_capture_geometry import H1Cp2pCapturePlan

DRAIN_CHUNK_BYTES = 64 * 1024
DEFAULT_RETIRE_TIMEOUT_SECONDS = 8.0
TERMINATE_GRACE_SECONDS = 2.0


@dataclass(frozen=True)
class H1Cp2pProducerRetirement:
    generation: int
    bytes_produced: int
    bytes_discarded_unsent: int
    process_returncode: int | None
    archive_path: str

    def to_dict(self) -> dict[str, object]:
        return {
            "generation": self.generation,
            "bytes_produced": self.bytes_produced,
            "bytes_discarded_unsent": self.bytes_discarded_unsent,
            "process_returncode": self.process_returncode,
            "archive_path": self.archive_path,
        }


ProducerFactory = Callable[[H1Cp2pCapturePlan, Path], object]
ProducerAttach = Callable[[object | None], None]


def _default_producer_factory(
    plan: H1Cp2pCapturePlan,
    archive_path: Path,
) -> base.ProducerBuffer:
    process = subprocess.Popen(
        list(plan.command),
        stdout=subprocess.PIPE,
        stderr=None,
        bufsize=0,
        start_new_session=True,
    )
    try:
        return base.ProducerBuffer(
            f"mpeg-generation-{plan.generation}",
            process,
            8192,
            archive_path,
            terminal_suffix=b"",
        )
    except BaseException:
        try:
            process.terminate()
        except BaseException:
            pass
        raise


class H1Cp2pMpegProducer:
    """Own exactly one live START-derived FFmpeg generation."""

    def __init__(
        self,
        evidence: Path,
        *,
        producer_factory: ProducerFactory = _default_producer_factory,
        attach: ProducerAttach | None = None,
        monotonic: Callable[[], float] = time.monotonic,
        sleep: Callable[[float], None] = time.sleep,
    ) -> None:
        self.evidence = Path(evidence)
        self.producer_factory = producer_factory
        self.attach = attach
        self.monotonic = monotonic
        self.sleep = sleep

        self.generation = 0
        self.plan: H1Cp2pCapturePlan | None = None
        self.producer: object | None = None
        self.archive_path: Path | None = None

    def active_generation(self) -> int:
        return int(self.generation)

    def start_exact(self, plan: H1Cp2pCapturePlan) -> dict[str, object]:
        generation = int(plan.generation)
        if generation <= 0:
            raise base.ProtocolError("CP2P MPEG producer generation must be nonzero")
        if self.producer is not None or self.generation != 0 or self.plan is not None:
            raise base.ProtocolError(
                f"CP2P MPEG producer already owns generation {self.generation}"
            )
        if not plan.command or plan.command[0] != "ffmpeg":
            raise base.ProtocolError("CP2P MPEG producer requires prepared FFmpeg command")

        archive_path = self.evidence / f"mpeg-generation-{generation}.m2v"
        try:
            producer = self.producer_factory(plan, archive_path)
        except BaseException as exc:
            raise base.ProtocolError(
                f"failed to launch exact MPEG producer generation {generation}"
            ) from exc
        if producer is None:
            raise base.ProtocolError("MPEG producer factory returned no producer")

        self.generation = generation
        self.plan = plan
        self.producer = producer
        self.archive_path = archive_path
        if self.attach is not None:
            self.attach(producer)

        evidence = {
            "session_id": int(plan.session_id),
            "generation": generation,
            "capture_x": int(plan.capture_x),
            "capture_y": int(plan.capture_y),
            "capture_width": int(plan.capture_width),
            "capture_height": int(plan.capture_height),
            "display": plan.display,
            "command": list(plan.command),
            "archive_path": str(archive_path),
            "state": "live-local-producer-public-mpeg-gate-closed",
            "pstv_mpeg_data_emission": False,
        }
        try:
            (self.evidence / "mpeg_producer_live.json").write_text(
                json.dumps(evidence, indent=2, sort_keys=True) + "\n"
            )
        except BaseException:
            # A live producer without durable prepared evidence is not a usable
            # START transaction. Stop it before reporting launch failure upward.
            try:
                self.retire_exact(generation)
            except BaseException:
                pass
            raise
        return evidence

    @staticmethod
    def _producer_returncode(producer: object) -> int | None:
        process = getattr(producer, "process", None)
        if process is None:
            return None
        poll = getattr(process, "poll", None)
        if poll is None:
            return None
        return poll()

    def _prove_reader_quiescent(self, producer: object) -> None:
        thread = getattr(producer, "thread", None)
        if thread is None:
            return
        join = getattr(thread, "join", None)
        if join is not None:
            join(timeout=1.0)
        is_alive = getattr(thread, "is_alive", None)
        if is_alive is not None and is_alive():
            raise base.ProtocolError("MPEG producer reader thread did not quiesce")

    def retire_exact(
        self,
        generation: int,
        *,
        timeout: float = DEFAULT_RETIRE_TIMEOUT_SECONDS,
    ) -> H1Cp2pProducerRetirement:
        generation = int(generation)
        if timeout <= 0:
            raise ValueError("MPEG producer retirement timeout must be positive")
        if (
            generation <= 0
            or self.producer is None
            or self.plan is None
            or self.archive_path is None
            or self.generation != generation
            or self.plan.generation != generation
        ):
            raise base.ProtocolError(
                "CP2P MPEG producer retirement generation mismatch "
                f"active={self.generation} requested={generation}"
            )

        producer = self.producer
        discarded = 0
        producer.stop()
        deadline = self.monotonic() + timeout
        terminated = False

        while True:
            producer.check()
            payload = producer.take(DRAIN_CHUNK_BYTES, 1)
            discarded += len(payload)
            if producer.done_and_empty():
                break

            if self.monotonic() >= deadline:
                if terminated:
                    raise base.ProtocolError(
                        f"MPEG producer generation {generation} did not stop/drain"
                    )
                producer.terminate()
                terminated = True
                deadline = self.monotonic() + TERMINATE_GRACE_SECONDS
            self.sleep(0.005)

        self._prove_reader_quiescent(producer)

        process = getattr(producer, "process", None)
        returncode = self._producer_returncode(producer)
        if process is not None and returncode is None:
            # EOF normally implies exit, but prove it rather than assuming it.
            try:
                returncode = process.wait(timeout=1.0)
            except subprocess.TimeoutExpired:
                producer.terminate()
                try:
                    returncode = process.wait(timeout=TERMINATE_GRACE_SECONDS)
                except subprocess.TimeoutExpired as exc:
                    raise base.ProtocolError(
                        f"MPEG producer generation {generation} process remained live"
                    ) from exc

        produced = int(getattr(producer, "bytes_produced", discarded))
        archive_path = str(self.archive_path)
        result = H1Cp2pProducerRetirement(
            generation=generation,
            bytes_produced=produced,
            bytes_discarded_unsent=discarded,
            process_returncode=returncode,
            archive_path=archive_path,
        )

        self.producer = None
        self.plan = None
        self.generation = 0
        self.archive_path = None
        if self.attach is not None:
            self.attach(None)

        (self.evidence / "mpeg_producer_live.json").unlink(missing_ok=True)
        (self.evidence / "mpeg_producer_retired.json").write_text(
            json.dumps(result.to_dict(), indent=2, sort_keys=True) + "\n"
        )
        return result

    def shutdown(self) -> None:
        """Best-effort session teardown; exact retirement remains the product path."""

        if self.producer is None:
            return
        generation = self.generation
        try:
            self.retire_exact(generation, timeout=2.0)
            return
        except BaseException:
            producer = self.producer
            if producer is not None:
                try:
                    producer.terminate()
                except BaseException:
                    pass
            self.producer = None
            self.plan = None
            self.generation = 0
            self.archive_path = None
            if self.attach is not None:
                self.attach(None)
