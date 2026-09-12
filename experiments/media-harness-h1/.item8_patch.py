#!/usr/bin/env python3
"""One-shot implementation patch for CP2P item #8 START-driven MPEG producer."""

from pathlib import Path

ROOT = Path(".")
MARKER = ROOT / "experiments/media-harness-h1/h1_cp2p_mpeg_producer.py"
if MARKER.exists():
    print("ITEM8_PATCH=ALREADY_APPLIED")
    raise SystemExit(0)


def read(path: str) -> str:
    return (ROOT / path).read_text()


def write(path: str, text: str) -> None:
    target = ROOT / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(text.rstrip() + "\n")


def replace_once(path: str, old: str, new: str) -> None:
    text = read(path)
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{path}: expected one match, got {count}: {old[:120]!r}")
    write(path, text.replace(old, new, 1))


write(
    "experiments/media-harness-h1/h1_cp2p_mpeg_producer.py",
    r'''#!/usr/bin/env python3
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
''',
)

write(
    "experiments/media-harness-h1/h1_cp2p_mpeg_producer_test.py",
    r'''#!/usr/bin/env python3
"""Host contracts for CP2P item #8's generation-scoped Pi MPEG producer."""

from __future__ import annotations

from pathlib import Path
import tempfile
import unittest

import h1_mux_server as base
from h1_cp2p_capture_geometry import H1Cp2pCapturePlan
from h1_cp2p_mpeg_producer import H1Cp2pMpegProducer


class FakeProcess:
    def __init__(self) -> None:
        self.returncode: int | None = None

    def poll(self):
        return self.returncode

    def wait(self, timeout=None):
        if self.returncode is None:
            self.returncode = 0
        return self.returncode


class FakeThread:
    def join(self, timeout=None) -> None:
        pass

    def is_alive(self) -> bool:
        return False


class FakeProducer:
    def __init__(self, payload: bytes) -> None:
        self.buffer = bytearray(payload)
        self.bytes_produced = len(payload)
        self.stop_called = False
        self.terminate_called = False
        self.process = FakeProcess()
        self.thread = FakeThread()

    def stop(self) -> None:
        self.stop_called = True
        self.process.returncode = 0

    def terminate(self) -> None:
        self.terminate_called = True
        self.process.returncode = -15

    def check(self) -> None:
        pass

    def take(self, maximum: int, alignment: int = 1) -> bytes:
        count = min(maximum, len(self.buffer))
        count -= count % alignment
        data = bytes(self.buffer[:count])
        del self.buffer[:count]
        return data

    def done_and_empty(self) -> bool:
        return self.stop_called and not self.buffer


class StuckProducer(FakeProducer):
    def stop(self) -> None:
        self.stop_called = True

    def done_and_empty(self) -> bool:
        return False


class Clock:
    def __init__(self) -> None:
        self.now = 0.0

    def monotonic(self) -> float:
        return self.now

    def sleep(self, amount: float) -> None:
        self.now += max(amount, 0.1)


def plan(generation: int = 7) -> H1Cp2pCapturePlan:
    return H1Cp2pCapturePlan(
        session_id=0x1234ABCD,
        generation=generation,
        desktop_width=704,
        desktop_height=480,
        capture_x=32,
        capture_y=48,
        capture_width=320,
        capture_height=240,
        display=":0",
        command=(
            "ffmpeg", "-f", "x11grab", "-video_size", "320x240",
            "-i", ":0+32,48", "pipe:1",
        ),
    )


class ProducerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.evidence = Path(self.temp.name)

    def tearDown(self) -> None:
        self.temp.cleanup()

    def test_start_launches_exact_prepared_plan_and_emits_no_pstv_data(self) -> None:
        observed = []
        attached = []
        fake = FakeProducer(b"mpeg-bytes")

        def factory(capture_plan, archive_path):
            observed.append((capture_plan, archive_path))
            archive_path.write_bytes(b"mpeg-bytes")
            return fake

        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=factory,
            attach=attached.append,
        )
        evidence = owner.start_exact(plan())

        self.assertEqual(owner.active_generation(), 7)
        self.assertIs(observed[0][0], owner.plan)
        self.assertEqual(list(owner.plan.command), evidence["command"])
        self.assertEqual(evidence["state"], "live-local-producer-public-mpeg-gate-closed")
        self.assertFalse(evidence["pstv_mpeg_data_emission"])
        self.assertEqual(attached, [fake])
        self.assertTrue((self.evidence / "mpeg_producer_live.json").exists())

    def test_exact_retirement_stops_and_drains_all_unsent_bytes(self) -> None:
        payload = b"x" * 150000
        fake = FakeProducer(payload)
        attached = []
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
            attach=attached.append,
        )
        owner.start_exact(plan(9))
        result = owner.retire_exact(9)

        self.assertTrue(fake.stop_called)
        self.assertFalse(fake.terminate_called)
        self.assertEqual(result.bytes_produced, len(payload))
        self.assertEqual(result.bytes_discarded_unsent, len(payload))
        self.assertEqual(owner.active_generation(), 0)
        self.assertEqual(attached[-1], None)
        self.assertFalse((self.evidence / "mpeg_producer_live.json").exists())
        self.assertTrue((self.evidence / "mpeg_producer_retired.json").exists())

    def test_wrong_generation_cannot_retire_live_producer(self) -> None:
        fake = FakeProducer(b"abc")
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
        )
        owner.start_exact(plan(11))
        with self.assertRaises(base.ProtocolError):
            owner.retire_exact(12)
        self.assertEqual(owner.active_generation(), 11)
        self.assertFalse(fake.stop_called)

    def test_stuck_producer_fails_closed_after_terminate_grace(self) -> None:
        clock = Clock()
        fake = StuckProducer(b"abc")
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
            monotonic=clock.monotonic,
            sleep=clock.sleep,
        )
        owner.start_exact(plan(13))
        with self.assertRaises(base.ProtocolError):
            owner.retire_exact(13, timeout=0.2)
        self.assertTrue(fake.stop_called)
        self.assertTrue(fake.terminate_called)
        self.assertEqual(owner.active_generation(), 13)


if __name__ == "__main__":
    unittest.main(verbosity=2)''',
)

runner_path = "experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py"
runner = read(runner_path)
runner = runner.replace(
    "from h1_cp2p_capture_geometry import H1Cp2pCapturePlan, prepare_exact_capture_plan\n",
    "from h1_cp2p_capture_geometry import H1Cp2pCapturePlan, prepare_exact_capture_plan\nfrom h1_cp2p_mpeg_producer import H1Cp2pMpegProducer\n",
    1,
)
runner = runner.replace(
    "    def __init__(self, session, suppression_bridge: H1Cp2pRfbPiBridge) -> None:\n        super().__init__(session)\n        self.suppression_bridge = suppression_bridge\n        self.capture_plan: H1Cp2pCapturePlan | None = None\n",
    "    def __init__(\n        self,\n        session,\n        suppression_bridge: H1Cp2pRfbPiBridge,\n        producer: H1Cp2pMpegProducer | None = None,\n    ) -> None:\n        super().__init__(session)\n        self.suppression_bridge = suppression_bridge\n        self.capture_plan: H1Cp2pCapturePlan | None = None\n        self.producer = producer\n",
    1,
)
runner = runner.replace(
    "        rollback_error: BaseException | None = None\n\n        if suppression_installed:\n",
    "        rollback_error: BaseException | None = None\n\n        if self.producer is not None and self.producer.active_generation() == int(generation):\n            try:\n                self.producer.retire_exact(generation)\n            except BaseException as exc:\n                rollback_error = exc\n\n        if suppression_installed:\n",
    1,
)
old_guard = '''        # Item #8 will replace this dormant-producer guard with an exact
        # stop/drain operation before suppression is removed. Until then, never
        # acknowledge retirement over an unexpected live legacy producer.
        if getattr(self.session, "video_producer", None) is not None:
            raise base.ProtocolError(
                "CP2P RETIRE cannot acknowledge while MPEG producer is live"
            )

        self.suppression_bridge.retire_suppression_exact(generation)
'''
new_guard = '''        # Exact producer stop/drain is the first destructive retirement step.
        # Suppression remains installed until every locally buffered, unsent MPEG
        # byte is discarded and the process/reader are proven quiescent.
        if self.producer is not None:
            if self.producer.active_generation() != generation:
                raise base.ProtocolError(
                    "CP2P RETIRE producer generation mismatch "
                    f"active={self.producer.active_generation()} requested={generation}"
                )
            self.producer.retire_exact(generation)
        elif getattr(self.session, "video_producer", None) is not None:
            raise base.ProtocolError(
                "CP2P RETIRE found live MPEG producer without generation owner"
            )

        self.suppression_bridge.retire_suppression_exact(generation)
'''
if old_guard not in runner:
    raise SystemExit("runner: item11A dormant producer guard missing")
runner = runner.replace(old_guard, new_guard, 1)

old_capture = '''            capture_evidence = self.capture_plan.to_dict()
            capture_evidence["state"] = "prepared-producer-dormant"
            (Path(self.session.evidence) / "mpeg_capture_prepared.json").write_text(
                json.dumps(capture_evidence, indent=2, sort_keys=True) + "\\n"
            )
'''
new_capture = '''            capture_evidence = self.capture_plan.to_dict()
            capture_evidence["state"] = "prepared-before-producer-launch"
            (Path(self.session.evidence) / "mpeg_capture_prepared.json").write_text(
                json.dumps(capture_evidence, indent=2, sort_keys=True) + "\\n"
            )

            if self.producer is not None:
                producer_evidence = self.producer.start_exact(self.capture_plan)
                capture_evidence["state"] = "producer-live-local-gate-closed"
                capture_evidence["producer_archive_path"] = producer_evidence["archive_path"]
                (Path(self.session.evidence) / "mpeg_capture_prepared.json").write_text(
                    json.dumps(capture_evidence, indent=2, sort_keys=True) + "\\n"
                )
'''
if old_capture not in runner:
    raise SystemExit("runner: capture evidence block missing")
runner = runner.replace(old_capture, new_capture, 1)

runner = runner.replace(
    "        super().__init__(sock, profile, evidence, duration, display)\n        self.cp2p_start_receiver: H1Cp2pStartReceiver | None = None\n",
    "        super().__init__(sock, profile, evidence, duration, display)\n        self.cp2p_start_receiver: H1Cp2pStartReceiver | None = None\n        self.cp2p_mpeg_producer = H1Cp2pMpegProducer(\n            Path(evidence),\n            attach=lambda producer: setattr(self, \"video_producer\", producer),\n        )\n",
    1,
)
runner = runner.replace(
    "            self.cp2p_start_receiver = H1Cp2pSuppressionStartReceiver(\n                self,\n                adapter.bridge,\n            )\n",
    "            self.cp2p_start_receiver = H1Cp2pSuppressionStartReceiver(\n                self,\n                adapter.bridge,\n                self.cp2p_mpeg_producer,\n            )\n",
    1,
)
old_cleanup = '''    def cleanup(self) -> None:
        try:
            super().cleanup()
        finally:
            if self.cp2p_start_receiver is not None:
                self.cp2p_start_receiver.stop()
                self.cp2p_start_receiver = None
'''
new_cleanup = '''    def cleanup(self) -> None:
        try:
            self.cp2p_mpeg_producer.shutdown()
        finally:
            try:
                super().cleanup()
            finally:
                if self.cp2p_start_receiver is not None:
                    self.cp2p_start_receiver.stop()
                    self.cp2p_start_receiver = None
'''
if old_cleanup not in runner:
    raise SystemExit("runner: cleanup block missing")
runner = runner.replace(old_cleanup, new_cleanup, 1)
runner = runner.replace(
    "    CP2P Pi runner for items #5/#6/#7: visible RFB + optional PCM, immutable\n    MPEG START receive/validation, generation-scoped Pi-side RFB suppression,\n    and exact START-derived capture-geometry preparation.\n",
    "    CP2P Pi runner through item #8: visible RFB + optional PCM, immutable\n    START receive/validation, generation-scoped RFB suppression, exact capture\n    preparation, and one START-driven exact-generation local FFmpeg producer.\n",
    1,
)
runner = runner.replace(
    "MPEG production is still dormant here. The FFmpeg command is prepared and saved\nto evidence but no process or MPEG DATA is started; producer activation remains\nitem #8 and must consume this exact generation only after suppression is ready.\n",
    "Item #8 now launches the exact prepared FFmpeg command after suppression and\ncapture preparation succeed. The item-#10 public MPEG gate remains closed, so\nproducer bytes stay local/bounded and no MPEG DATA is emitted onto PSTV yet.\n",
    1,
)
write(runner_path, runner)

# Extend the established #11A integration test with explicit producer-before-
# suppression ordering using a tiny fake generation owner.
retirement_path = "experiments/media-harness-h1/h1_cp2p_retirement_control_test.py"
retirement = read(retirement_path)
retirement = retirement.replace(
    "class RetirementControlTests(unittest.TestCase):\n",
    r'''class FakeGenerationProducer:
    def __init__(self, generation: int, bridge) -> None:
        self.generation = int(generation)
        self.bridge = bridge
        self.retired = []

    def active_generation(self) -> int:
        return self.generation

    def start_exact(self, plan):
        self.generation = int(plan.generation)
        return {"archive_path": "fake.m2v"}

    def retire_exact(self, generation: int):
        # The critical #8/#11A ordering assertion: suppression still belongs to
        # this generation while producer stop/drain completes.
        if self.bridge.suppression_generation != int(generation):
            raise AssertionError("suppression removed before producer retirement")
        self.retired.append(int(generation))
        self.generation = 0


class RetirementControlTests(unittest.TestCase):
''',
    1,
)
insert_marker = "    def test_wrong_generation_or_session_does_not_mutate_state(self) -> None:\n"
new_test = r'''    def test_live_generation_producer_retires_before_suppression_and_ack(self) -> None:
        producer = FakeGenerationProducer(0, self.bridge)
        receiver = H1Cp2pSuppressionStartReceiver(
            self.session,
            self.bridge,
            producer,
        )
        frame = base.Frame(
            base.FRAME_DATA,
            base.CHANNEL_MPEG2,
            0,
            1,
            start_payload(self.session.profile["session_id"], 8),
        )
        receiver._handle_start_frame(frame)
        self.assertEqual(producer.active_generation(), 8)
        self.assertEqual(self.bridge.suppression_generation, 8)

        receiver._handle_retire_frame(self.retire_frame(8))
        self.assertEqual(producer.retired, [8])
        self.assertEqual(self.bridge.suppression_generation, 0)
        self.assertEqual(len(self.session.sent), 1)
        self.assertEqual(self.session.sent[0][0], base.FRAME_MPEG_RETIRE)

'''
if insert_marker not in retirement:
    raise SystemExit("retirement test marker missing")
retirement = retirement.replace(insert_marker, new_test + insert_marker, 1)
write(retirement_path, retirement)

print("ITEM8_PATCH=APPLIED")
