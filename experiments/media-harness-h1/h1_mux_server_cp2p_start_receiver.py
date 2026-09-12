#!/usr/bin/env python3
"""
File synopsis:
    CP2P Pi runner through item #10: visible RFB + optional PCM + START-owned
    exact-generation MPEG on the existing one-socket PSTV transport.

The runner preserves one physical PSTV socket and one H1 reader thread. It
substitutes a CP2P-only upstream RFB bridge while leaving the qualified CP2O
bridge implementation untouched. START first becomes immutable prepared state,
then installs one pending suppression generation, then prepares the exact
capture plan from that same immutable START.

Suppression becomes active immediately before the first NEW
FramebufferUpdateRequest after START, so any older request already outstanding
at calibration acceptance may finish normally while PS2 presentation remains
frozen in WAIT_FIRST_FRAME.

Item #10 opens only the public MPEG CONFIG/scheduler seam. START still launches
the exact producer, and the scheduler may emit channel-4 DATA only while it
holds the matching generation's #11B emission lease. Retirement closes that
lease before producer stop/drain, so no stale generation can be reopened.

Compound START preparation is fail-closed. If a later preparation step fails
after suppression was installed, the exact suppression generation and usable
prepared START state are rolled back together while the generation high-water
remains stale/rejected.
"""

from __future__ import annotations

import json
from pathlib import Path
import struct
import time

import h1_mux_server_cumulative39_rfb_pcm_bridge as cp2o
from h1_cp2p_capture_geometry import H1Cp2pCapturePlan, prepare_exact_capture_plan
from h1_cp2p_mpeg_producer import H1Cp2pMpegProducer
from h1_cp2p_rfb_suppression import (
    H1Cp2pRfbPiBridge,
    open_cp2p_rfb_session_adapter,
)
from h1_cp2p_start_receiver import H1Cp2pStartReceiver

# CP2O imported its opener into module scope. Replace that one experiment-local
# construction seam before ParentSession.__init__ runs; CP2O source and its
# default bridge remain unchanged for every non-CP2P runner.
cp2o.open_rfb_session_adapter = open_cp2p_rfb_session_adapter

base = cp2o.base
_ParentSession = cp2o.H1Cumulative39RfbPcmSession


def _install_cp2p_all_guns_profile() -> None:
    if "CP2P_ALL_GUNS" in base.PROFILES:
        return
    profile = dict(base.PROFILES["P11_COMPAT_PLUS_PCM"])
    profile.update({
        "profile_id": 3,
        "rfb_mode": 2,
        "rfb_queue_capacity": 32768,
        "rfb_credit_batch_bytes": 8192,
        "rfb_credit_flush_on_empty": 1,
        "rfb_credit_return_enabled": 1,
        "rfb_initial_credit_bytes": 32768,
    })
    base.PROFILES["CP2P_ALL_GUNS"] = profile


_install_cp2p_all_guns_profile()


class H1Cp2pSuppressionStartReceiver(H1Cp2pStartReceiver):
    """START authority plus ordered suppression and exact capture preparation."""

    def __init__(
        self,
        session,
        suppression_bridge: H1Cp2pRfbPiBridge,
        producer: H1Cp2pMpegProducer | None = None,
    ) -> None:
        super().__init__(session)
        self.suppression_bridge = suppression_bridge
        self.capture_plan: H1Cp2pCapturePlan | None = None
        self.producer = producer

    def _rollback_compound_preparation(
        self,
        generation: int,
        *,
        suppression_installed: bool,
    ) -> None:
        """Remove all usable state for one failed START while preserving high-water."""

        rollback_error: BaseException | None = None

        if self.producer is not None:
            active_producer_generation = int(self.producer.active_generation())
            if active_producer_generation != 0:
                if active_producer_generation != int(generation):
                    raise base.ProtocolError(
                        "CP2P START rollback producer generation mismatch "
                        f"active={active_producer_generation} requested={int(generation)}; "
                        "suppression retained fail-closed"
                    )
                try:
                    self.producer.retire_exact(generation)
                except BaseException as exc:
                    # A producer that cannot be proven stopped/drained must stay
                    # protected by its exact RFB suppression. Do not continue
                    # destructive rollback and expose a still-live video source.
                    raise base.ProtocolError(
                        f"CP2P START generation {int(generation)} producer rollback "
                        "failed; suppression retained fail-closed"
                    ) from exc

        if suppression_installed:
            try:
                self.suppression_bridge.retire_suppression_exact(generation)
            except BaseException as exc:
                rollback_error = exc

        prepared = self.peek_prepared()
        if prepared is not None and prepared.generation == int(generation):
            try:
                self.release_prepared_exact(generation)
            except BaseException as exc:
                if rollback_error is None:
                    rollback_error = exc

        self.capture_plan = None

        # These files are evidence of usable prepared state. A failed compound
        # transaction must not leave them implying that suppression/capture can
        # later be activated. Failure to remove evidence is itself fail-closed.
        for name in (
            "mpeg_start_prepared.json",
            "rfb_suppression_prepared.json",
            "mpeg_capture_prepared.json",
        ):
            try:
                (Path(self.session.evidence) / name).unlink(missing_ok=True)
            except BaseException as exc:
                if rollback_error is None:
                    rollback_error = exc

        if rollback_error is not None:
            raise base.ProtocolError(
                f"CP2P START generation {int(generation)} rollback failed"
            ) from rollback_error

    def retire_generation_exact(self, generation: int):
        """Retire one exact prepared Pi generation before acknowledging the PS2."""

        generation = int(generation)
        request = self.peek_prepared()
        if request is None or request.generation != generation:
            raise base.ProtocolError(
                "CP2P RETIRE prepared-generation mismatch "
                f"prepared={getattr(request, 'generation', 0)} requested={generation}"
            )
        if self.capture_plan is None or self.capture_plan.generation != generation:
            raise base.ProtocolError(
                "CP2P RETIRE exact capture generation is not prepared"
            )
        if self.suppression_bridge.suppression_generation != generation:
            raise base.ProtocolError(
                "CP2P RETIRE suppression generation mismatch "
                f"active={self.suppression_bridge.suppression_generation} "
                f"requested={generation}"
            )

        # Exact producer stop/drain is the first destructive retirement step.
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
                "CP2P RETIRE cannot acknowledge while MPEG producer is live without generation owner"
            )

        self.suppression_bridge.retire_suppression_exact(generation)
        released = super().retire_generation_exact(generation)
        self.capture_plan = None

        for name in (
            "rfb_suppression_prepared.json",
            "mpeg_capture_prepared.json",
        ):
            (Path(self.session.evidence) / name).unlink(missing_ok=True)

        return released

    def _handle_start_frame(self, frame: base.Frame) -> None:
        try:
            super()._handle_start_frame(frame)
        except BaseException:
            prepared = self.peek_prepared()
            if prepared is not None:
                self._rollback_compound_preparation(
                    prepared.generation,
                    suppression_installed=False,
                )
            raise

        request = self.peek_prepared()
        if request is None:
            raise base.ProtocolError("validated MPEG START did not remain prepared")

        suppression_installed = False
        try:
            self.suppression_bridge.install_suppression(request)
            suppression_installed = True

            self.capture_plan = prepare_exact_capture_plan(
                request,
                desktop_width=self.suppression_bridge.desktop_width,
                desktop_height=self.suppression_bridge.desktop_height,
                display=self.session.display,
            )

            suppression_evidence = {
                "generation": int(request.generation),
                "desktop_width": int(self.suppression_bridge.desktop_width),
                "desktop_height": int(self.suppression_bridge.desktop_height),
                "raw_pixel_budget_bytes": int(
                    self.suppression_bridge.desktop_raw_byte_budget
                ),
                "suppression_x": int(request.suppression_x),
                "suppression_y": int(request.suppression_y),
                "suppression_width": int(request.suppression_width),
                "suppression_height": int(request.suppression_height),
                "state": "pending-next-new-rfb-update-request",
            }
            (Path(self.session.evidence) / "rfb_suppression_prepared.json").write_text(
                json.dumps(suppression_evidence, indent=2, sort_keys=True) + "\n"
            )

            capture_evidence = self.capture_plan.to_dict()
            capture_evidence["state"] = "prepared-before-producer-launch"
            (Path(self.session.evidence) / "mpeg_capture_prepared.json").write_text(
                json.dumps(capture_evidence, indent=2, sort_keys=True) + "\n"
            )

            if self.producer is not None:
                producer_evidence = self.producer.start_exact(self.capture_plan)
                capture_evidence["state"] = "producer-live-awaiting-exact-emission-lease"
                capture_evidence["producer_archive_path"] = producer_evidence["archive_path"]
                (Path(self.session.evidence) / "mpeg_capture_prepared.json").write_text(
                    json.dumps(capture_evidence, indent=2, sort_keys=True) + "\n"
                )
        except BaseException:
            self._rollback_compound_preparation(
                request.generation,
                suppression_installed=suppression_installed,
            )
            raise

        print(
            "H1_CP2P_RFB_SUPPRESSION_PREPARED="
            + json.dumps(suppression_evidence, sort_keys=True),
            flush=True,
        )
        print(
            "H1_CP2P_MPEG_CAPTURE_PREPARED="
            + json.dumps(capture_evidence, sort_keys=True),
            flush=True,
        )


class H1Cp2pStartReceiveSession(_ParentSession):
    """CP2P item #10 all-guns composition on the existing CP2O shell."""

    def _validate_composition_policy(self, profile: dict[str, int]) -> None:
        if int(profile["rfb_mode"]) != 2:
            raise base.ProtocolError("CP2P item #10 requires visible RFB mode 2")
        if int(profile["video_mode"]) != 1:
            raise base.ProtocolError("CP2P item #10 requires MPEG2 video mode")
        if int(profile["audio_mode"]) not in (0, base.AUDIO_PCM):
            raise base.ProtocolError("CP2P item #10 permits only audio OFF or PCM")

    def __init__(
        self,
        sock,
        profile: dict[str, int],
        evidence,
        duration: float,
        display: str,
    ) -> None:
        super().__init__(sock, profile, evidence, duration, display)
        self.cp2p_start_receiver: H1Cp2pStartReceiver | None = None
        self.cp2p_mpeg_producer = H1Cp2pMpegProducer(
            Path(evidence),
            attach=lambda producer: setattr(self, "video_producer", producer),
        )
        self._cp2p_emission_opened_generations: set[int] = set()
        self._cp2p_mpeg_generation_order: list[int] = []
        self._cp2p_mpeg_sent_bytes: dict[int, int] = {}
        self._cp2p_mpeg_archive_paths: dict[int, Path] = {}
        self._cp2p_schedule_index = 0

        adapter = self.rfb_session_adapter
        if adapter is None or not isinstance(adapter.bridge, H1Cp2pRfbPiBridge):
            super().cleanup()
            raise base.ProtocolError("CP2P runner did not acquire suppression-capable RFB bridge")

        try:
            # Parent construction has already opened/registered the CP2P RFB
            # adapter. START wraps the same reader call chain outside channel 1.
            self.cp2p_start_receiver = H1Cp2pSuppressionStartReceiver(
                self,
                adapter.bridge,
                self.cp2p_mpeg_producer,
            )
            self.cp2p_start_receiver.start()
        except BaseException:
            super().cleanup()
            raise

        print(
            "H1_CP2P_START_RECEIVER=ARMED "
            f"session_id={int(self.profile['session_id'])} "
            f"desktop={int(self.profile['desktop_width'])}x"
            f"{int(self.profile['desktop_height'])} "
            f"raw_budget={adapter.bridge.desktop_raw_byte_budget}",
            flush=True,
        )

    def _send_cp2p_mpeg_once(self) -> bool:
        """Send at most one exact-generation MPEG DATA frame under a #11B lease."""

        receiver = self.cp2p_start_receiver
        if receiver is None:
            return False
        request = receiver.peek_prepared()
        if request is None:
            return False

        generation = int(request.generation)
        owner = self.cp2p_mpeg_producer
        if owner.active_generation() != generation:
            return False

        if generation not in self._cp2p_emission_opened_generations:
            try:
                owner.open_emission_exact(generation)
            except base.ProtocolError:
                if owner.active_generation() != generation:
                    return False
                raise
            self._cp2p_emission_opened_generations.add(generation)
            print(f"H1_CP2P_MPEG_EMISSION=OPEN generation={generation}", flush=True)

        try:
            producer = owner.begin_emission_exact(generation)
        except base.ProtocolError:
            if owner.active_generation() != generation:
                return False
            raise
        if producer is None:
            return False

        archive_path = owner.archive_path
        before = int(self.mpeg.bytes_sent)
        try:
            producer.check()
            sent = self._send_from(producer, self.mpeg, base.MAX_PAYLOAD, 1)
        finally:
            owner.finish_emission_exact(generation)

        if not sent:
            return False

        sent_now = int(self.mpeg.bytes_sent) - before
        if sent_now <= 0 or archive_path is None:
            raise base.ProtocolError("CP2P MPEG send accounting lost generation evidence")

        previous_path = self._cp2p_mpeg_archive_paths.get(generation)
        if previous_path is None:
            self._cp2p_mpeg_archive_paths[generation] = Path(archive_path)
            self._cp2p_mpeg_generation_order.append(generation)
            self._cp2p_mpeg_sent_bytes[generation] = 0
        elif previous_path != Path(archive_path):
            raise base.ProtocolError("CP2P MPEG generation archive identity changed")

        self._cp2p_mpeg_sent_bytes[generation] += sent_now
        return True

    def _run_finite_payload_window(self) -> None:
        """Run RFB + optional PCM + START-owned MPEG until the finite deadline."""

        adapter = self.rfb_session_adapter
        if adapter is None:
            raise base.ProtocolError("CP2P run missing RFB adapter")

        deadline = time.monotonic() + self.duration
        schedule = (
            base.CHANNEL_AUDIO,
            base.CHANNEL_MPEG2,
            base.CHANNEL_MPEG2,
            base.CHANNEL_MPEG2,
        )

        while True:
            self.check_reader()
            adapter.check()
            if self.audio_producer is not None:
                self.audio_producer.check()

            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break

            sent = False
            for _ in range(len(schedule)):
                channel = schedule[self._cp2p_schedule_index]
                self._cp2p_schedule_index = (self._cp2p_schedule_index + 1) % len(schedule)

                if channel == base.CHANNEL_AUDIO:
                    if self.audio_producer is None:
                        continue
                    sent = self._send_from(
                        self.audio_producer, self.audio, base.MAX_PAYLOAD, 4
                    )
                else:
                    sent = self._send_cp2p_mpeg_once()

                if sent:
                    break

            if not sent:
                with self.condition:
                    self.condition.wait(timeout=min(remaining, 0.001))

        if self.audio_producer is None:
            return

        self.audio_stop_requested = True
        self.audio_producer.stop()
        print("H1_AUDIO_STOP_REQUEST=DURATION", flush=True)

        while not self.audio_producer.done_and_empty():
            self.check_reader()
            adapter.check()
            self.audio_producer.check()
            sent = self._send_from(
                self.audio_producer, self.audio, base.MAX_PAYLOAD, 4
            )
            if not sent:
                with self.condition:
                    self.condition.wait(timeout=0.001)

    def _retire_live_producer_for_session_end(self) -> None:
        """Close local production after RFB quiesce; session end needs no RETIRE ACK."""

        generation = int(self.cp2p_mpeg_producer.active_generation())
        if generation == 0:
            return
        self.cp2p_mpeg_producer.retire_exact(generation)
        print(
            f"H1_CP2P_MPEG_PRODUCER=SESSION_END_RETIRED generation={generation}",
            flush=True,
        )

    def _mpeg_wire_bytes(self) -> bytes:
        chunks: list[bytes] = []
        total = 0

        for generation in self._cp2p_mpeg_generation_order:
            count = int(self._cp2p_mpeg_sent_bytes.get(generation, 0))
            path = self._cp2p_mpeg_archive_paths.get(generation)
            if count <= 0 or path is None:
                raise base.ProtocolError(
                    f"CP2P MPEG generation {generation} has invalid sent-byte evidence"
                )
            data = path.read_bytes()
            if len(data) < count:
                raise base.ProtocolError(
                    "CP2P MPEG archive shorter than bytes actually sent "
                    f"generation={generation} archive={len(data)} sent={count}"
                )
            chunks.append(data[:count])
            total += count

        if total != int(self.mpeg.bytes_sent):
            raise base.ProtocolError(
                "CP2P MPEG per-generation accounting mismatch "
                f"segments={total} channel={int(self.mpeg.bytes_sent)}"
            )
        return b"".join(chunks)

    def _send_rfb_pcm_media_end(self) -> dict[str, int]:
        """Seal actual PSTV wire totals; unsent producer tail is never reported."""

        self._retire_live_producer_for_session_end()
        video_bytes = self._mpeg_wire_bytes()
        picture_starts = video_bytes.count(b"\x00\x00\x01\x00")
        sequence_headers = video_bytes.count(b"\x00\x00\x01\xb3")
        sequence_ends = video_bytes.count(base.MPEG_SEQUENCE_END)

        if int(self.mpeg.bytes_sent) == 0 or picture_starts == 0:
            raise base.ProtocolError(
                "CP2P item #10 session ended without proven live MPEG DATA"
            )

        words = [
            base.MEDIA_END_VERSION,
            self.profile["session_id"],
            self.audio.bytes_sent,
            self.audio.frames_sent,
            self.audio.last_data_sequence,
            self.audio.crc32,
            self.mpeg.bytes_sent,
            self.mpeg.frames_sent,
            self.mpeg.last_data_sequence,
            self.mpeg.crc32,
            picture_starts,
            sequence_headers,
            sequence_ends,
            base.STOP_REASON_FINITE_DURATION,
            0,
            0,
        ]
        payload = struct.pack(">16I", *words)
        if len(payload) != base.MEDIA_END_BYTES:
            raise AssertionError("CP2P MEDIA_END payload size mismatch")

        metadata = {
            "session_id": self.profile["session_id"],
            "audio_bytes": self.audio.bytes_sent,
            "audio_frames": self.audio.frames_sent,
            "audio_last_sequence": self.audio.last_data_sequence,
            "audio_crc32": self.audio.crc32,
            "mpeg_bytes": self.mpeg.bytes_sent,
            "mpeg_frames": self.mpeg.frames_sent,
            "mpeg_last_sequence": self.mpeg.last_data_sequence,
            "mpeg_crc32": self.mpeg.crc32,
            "picture_starts": picture_starts,
            "sequence_headers": sequence_headers,
            "sequence_ends": sequence_ends,
            "stop_reason": base.STOP_REASON_FINITE_DURATION,
        }

        self.send_frame(base.FRAME_MEDIA_END, base.CHANNEL_CONTROL, payload)
        print("H1_MEDIA_END_SENT=" + json.dumps(metadata, sort_keys=True), flush=True)
        return metadata

    def validate_result(self, metadata: dict[str, int]) -> None:
        """Validate all-guns integrity while allowing intentional queue residue."""

        self.check_reader()
        if self.result is None:
            raise base.ProtocolError("PS2 session result missing")

        result = self.result
        failures: list[str] = []
        checks = {
            "version": (result["version"], 1),
            "session_id": (result["session_id"], self.profile["session_id"]),
            "transport_error": (result["transport_error"], 0),
            "integrity_pass": (result["integrity_pass"], 1),
            "profile_id": (result["profile_id"], self.profile["profile_id"]),
            "audio_mode": (result["audio_mode"], self.profile["audio_mode"]),
            "video_mode": (result["video_mode"], self.profile["video_mode"]),
            "producer_picture_starts": (
                result["producer_picture_starts"], metadata["picture_starts"]
            ),
            "producer_mpeg_crc32": (
                result["producer_mpeg_crc32"], metadata["mpeg_crc32"]
            ),
            "ps2_mpeg_crc32": (result["ps2_mpeg_crc32"], metadata["mpeg_crc32"]),
            "mpeg_bytes_enqueued": (
                result["mpeg_bytes_enqueued"], metadata["mpeg_bytes"]
            ),
            "producer_stop_reason": (
                result["producer_stop_reason"], base.STOP_REASON_FINITE_DURATION
            ),
        }

        if self.profile["audio_mode"] == base.AUDIO_PCM:
            checks.update(
                {
                    "producer_audio_crc32": (
                        result["producer_audio_crc32"], metadata["audio_crc32"]
                    ),
                    "ps2_audio_crc32": (
                        result["ps2_audio_crc32"], metadata["audio_crc32"]
                    ),
                    "audio_bytes_enqueued": (
                        result["audio_bytes_enqueued"], metadata["audio_bytes"]
                    ),
                    "audio_bytes_consumed": (
                        result["audio_bytes_consumed"], metadata["audio_bytes"]
                    ),
                }
            )

        for name, (actual, expected) in checks.items():
            if actual != expected:
                failures.append(f"{name}:actual={actual}:expected={expected}")

        if result["mpeg_bytes_consumed"] > metadata["mpeg_bytes"]:
            failures.append(
                "mpeg_bytes_consumed exceeds enqueued wire bytes "
                f"actual={result['mpeg_bytes_consumed']} enqueued={metadata['mpeg_bytes']}"
            )
        if result["pictures_decoded"] == 0:
            failures.append("pictures_decoded:actual=0:expected=>0")
        if result["pictures_displayed"] == 0:
            failures.append("pictures_displayed:actual=0:expected=>0")
        if result["pictures_displayed"] > result["pictures_decoded"]:
            failures.append(
                "pictures_displayed exceeds pictures_decoded "
                f"displayed={result['pictures_displayed']} decoded={result['pictures_decoded']}"
            )

        summary = {
            "profile": self.profile,
            "metadata": metadata,
            "result": result,
            "mpeg_residual_bytes_at_session_end": (
                int(metadata["mpeg_bytes"]) - int(result["mpeg_bytes_consumed"])
            ),
            "validation_failures": failures,
        }
        (self.evidence / "summary.json").write_text(
            json.dumps(summary, indent=2, sort_keys=True) + "\n"
        )

        if failures:
            raise base.ProtocolError(
                "CP2P item #10 result validation failed: " + "; ".join(failures)
            )

        print(
            "H1_CP2P_ITEM10_SESSION_VALIDATION=PASS "
            f"profile_id={self.profile['profile_id']} "
            f"session_id={self.profile['session_id']} "
            f"decoded={result['pictures_decoded']} "
            f"displayed={result['pictures_displayed']} "
            f"mpeg_bytes={metadata['mpeg_bytes']} "
            f"mpeg_residual={summary['mpeg_residual_bytes_at_session_end']} "
            f"audio_bytes_played={result['audio_bytes_played']}",
            flush=True,
        )

    def cleanup(self) -> None:
        try:
            self.cp2p_mpeg_producer.shutdown()
        finally:
            try:
                super().cleanup()
            finally:
                if self.cp2p_start_receiver is not None:
                    self.cp2p_start_receiver.stop()
                    self.cp2p_start_receiver = None


base.H1Session = H1Cp2pStartReceiveSession


if __name__ == "__main__":
    raise SystemExit(base.main())
