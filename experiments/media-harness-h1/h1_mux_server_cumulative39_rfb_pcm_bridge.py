#!/usr/bin/env python3
"""
File synopsis:
    CP2O Pi-side cumulative runner for visible RFB plus the existing PCM producer
    on one physical PSTV connection.

This is a narrow descendant of the current cumulative RFB bridge. It reuses the
same upstream RFB adapter/quiesce handshake, H1Session reader, channel credit
accounting, pw-record PCM capture, ProducerBuffer, result validation, and evidence
format. MPEG remains OFF.

Session order:
    HELLO -> CONFIG/ACK -> RFB + PCM for finite duration
    -> stop and drain Pi PCM producer -> RFB REQUEST/BOUNDARY/COMMIT/COMPLETE
    -> MEDIA_END with exact PCM totals -> PS2 drains audio -> SESSION_RESULT.
"""

from __future__ import annotations

import json
import subprocess
import struct
import threading
import time

import h1_mux_server_cumulative39_rfb_bridge as rfb_base
from h1_rfb_session_adapter import H1RfbSessionAdapter, open_rfb_session_adapter

base = rfb_base.base
_ParentSession = rfb_base._ParentSession


class H1Cumulative39RfbPcmSession(_ParentSession):
    """Visible cumulative RFB with optional canonical PCM and no MPEG."""

    def _validate_composition_policy(self, profile: dict[str, int]) -> None:
        if int(profile["rfb_mode"]) != 2:
            raise base.ProtocolError("CP2O runner requires visible RFB mode 2")
        if int(profile["video_mode"]) != 0:
            raise base.ProtocolError("CP2O runner requires MPEG/video OFF")
        if int(profile["audio_mode"]) not in (0, base.AUDIO_PCM):
            raise base.ProtocolError("CP2O runner permits only audio OFF or PCM")

    def __init__(
        self,
        sock,
        profile: dict[str, int],
        evidence,
        duration: float,
        display: str,
    ) -> None:
        super().__init__(sock, profile, evidence, duration, display)
        self.rfb_session_adapter: H1RfbSessionAdapter | None = None

        self._validate_composition_policy(profile)

        host, port = rfb_base._rfb_upstream()
        kwargs = {"host": host, "port": port}
        if rfb_base.RFB_CONNECTOR is not None:
            kwargs["connector"] = rfb_base.RFB_CONNECTOR

        self.rfb_session_adapter = open_rfb_session_adapter(self, **kwargs)
        if self.rfb_session_adapter is None:
            raise base.ProtocolError("CP2O RFB bridge failed to attach")

        print(f"H1_RFB_UPSTREAM_ATTACHED={host}:{port}", flush=True)

    def video_command(self) -> list[str]:
        """Keep validate-only meaningful: CP2O never starts the MPEG producer."""
        return []

    def _start_pcm_producer(self) -> None:
        if int(self.profile["audio_mode"]) == 0:
            return

        command = self.audio_command()
        print("H1_AUDIO_COMMAND=" + json.dumps(command), flush=True)
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=None,
            bufsize=0,
            start_new_session=True,
        )
        self.audio_producer = base.ProducerBuffer(
            "audio",
            process,
            8192,
            self.evidence / "audio.pcm",
        )

    def _run_finite_payload_window(self) -> None:
        """Run RFB continuously while feeding PCM until capture duration expires."""
        adapter = self.rfb_session_adapter
        if adapter is None:
            raise base.ProtocolError("CP2O run missing RFB adapter")

        deadline = time.monotonic() + self.duration

        while True:
            self.check_reader()
            adapter.check()
            if self.audio_producer is not None:
                self.audio_producer.check()

            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break

            sent = False
            if self.audio_producer is not None:
                sent = self._send_from(
                    self.audio_producer,
                    self.audio,
                    base.MAX_PAYLOAD,
                    4,
                )

            if not sent:
                with self.condition:
                    self.condition.wait(timeout=min(remaining, 0.001))

        if self.audio_producer is None:
            return

        self.audio_stop_requested = True
        self.audio_producer.stop()
        print("H1_AUDIO_STOP_REQUEST=DURATION", flush=True)

        # Drain every byte already captured before sealing MEDIA_END. RFB remains
        # live during this short drain so the only lifecycle boundary is the
        # existing ordered RFB quiesce handshake below.
        while not self.audio_producer.done_and_empty():
            self.check_reader()
            adapter.check()
            self.audio_producer.check()

            sent = self._send_from(
                self.audio_producer,
                self.audio,
                base.MAX_PAYLOAD,
                4,
            )
            if not sent:
                with self.condition:
                    self.condition.wait(timeout=0.001)

    def _send_rfb_pcm_media_end(self) -> dict[str, int]:
        words = [
            base.MEDIA_END_VERSION,
            self.profile["session_id"],
            self.audio.bytes_sent,
            self.audio.frames_sent,
            self.audio.last_data_sequence,
            self.audio.crc32,
            0, 0, 0, 0,  # MPEG bytes/frames/sequence/CRC
            0, 0, 0,     # MPEG picture/header/end counts
            base.STOP_REASON_FINITE_DURATION,
            0,
            0,
        ]
        payload = struct.pack(">16I", *words)
        if len(payload) != base.MEDIA_END_BYTES:
            raise AssertionError("CP2O MEDIA_END payload size mismatch")

        metadata = {
            "session_id": self.profile["session_id"],
            "audio_bytes": self.audio.bytes_sent,
            "audio_frames": self.audio.frames_sent,
            "audio_last_sequence": self.audio.last_data_sequence,
            "audio_crc32": self.audio.crc32,
            "mpeg_bytes": 0,
            "mpeg_frames": 0,
            "mpeg_last_sequence": 0,
            "mpeg_crc32": 0,
            "picture_starts": 0,
            "sequence_headers": 0,
            "sequence_ends": 0,
            "stop_reason": base.STOP_REASON_FINITE_DURATION,
        }

        self.send_frame(base.FRAME_MEDIA_END, base.CHANNEL_CONTROL, payload)
        print("H1_MEDIA_END_SENT=" + json.dumps(metadata, sort_keys=True), flush=True)
        return metadata

    def run(self) -> None:
        adapter = self.rfb_session_adapter
        if adapter is None:
            raise base.ProtocolError("CP2O run missing RFB adapter")

        reader = threading.Thread(
            target=self.reader,
            name="h1-ps2-reader",
            daemon=True,
        )
        reader.start()

        if not self.hello_event.wait(timeout=10.0):
            raise base.ProtocolError("timed out waiting for PS2 HELLO")
        self.check_reader()

        if self.hello is None or (
            int(self.hello["capabilities"]) & rfb_base.CAP_RFB
        ) != rfb_base.CAP_RFB:
            raise base.ProtocolError("wrong PS2 ELF: cumulative RFB capability absent")

        self.send_frame(base.FRAME_CONFIG, base.CHANNEL_CONTROL, self.config_payload)
        print("H1_CONFIG_SENT=" + json.dumps(self.profile, sort_keys=True), flush=True)

        if not self.config_ack_event.wait(timeout=10.0):
            raise base.ProtocolError("timed out waiting for exact CONFIG ACK")
        self.check_reader()

        self._start_pcm_producer()
        self._run_finite_payload_window()

        print(
            f"H1_RFB_QUIESCE_REQUEST duration={self.duration:.6f}",
            flush=True,
        )
        adapter.request_quiesce()
        adapter.wait_quiesce_complete(timeout=rfb_base.RFB_QUIESCE_TIMEOUT_SECONDS)
        self.check_reader()

        print(
            "H1_RFB_QUIESCE=COMPLETE "
            f"boundary={int(adapter.quiesce_boundary_received)} "
            f"bridge_stopped={int(adapter.bridge_quiesced)} "
            f"commit={int(adapter.quiesce_commit_sent)} "
            f"complete={int(adapter.quiesce_complete_received)}",
            flush=True,
        )

        metadata = self._send_rfb_pcm_media_end()

        if not self.result_event.wait(timeout=60.0):
            raise base.ProtocolError("timed out waiting for H1 PS2 SESSION_RESULT")
        self.check_reader()
        self.validate_result(metadata)

        quiesce_summary = {
            "requested": adapter.quiesce_requested,
            "boundary_received": adapter.quiesce_boundary_received,
            "bridge_quiesced": adapter.bridge_quiesced,
            "commit_sent": adapter.quiesce_commit_sent,
            "complete_received": adapter.quiesce_complete_received,
            "bridge_stats": vars(adapter.bridge.stats),
        }
        (self.evidence / "rfb_quiesce.json").write_text(
            json.dumps(quiesce_summary, indent=2, sort_keys=True) + "\n"
        )

        self.stop_event.set()
        try:
            self.sock.shutdown(2)
        except OSError:
            pass
        self.sock.close()
        reader.join(timeout=1.0)
        if reader.is_alive():
            raise base.ProtocolError("H1 PS2 reader did not stop after CP2O session")

    def cleanup(self) -> None:
        try:
            super().cleanup()
        finally:
            if self.rfb_session_adapter is not None:
                self.rfb_session_adapter.stop()
                self.rfb_session_adapter = None


base.H1Session = H1Cumulative39RfbPcmSession


if __name__ == "__main__":
    raise SystemExit(base.main())
