#!/usr/bin/env python3
"""
File synopsis:
    Composes the cumulative through-Issue-39 H1 Pi runner with the verified raw
    RFB channel-1 session adapter and clean finite-session quiesce protocol,
    while leaving the PS2 RFB activation gate closed.

RFB-OFF inherits the existing cumulative media runner unchanged. RFB-ON opens
exactly one ordinary upstream VNC connection and runs an RFB-only finite session:

    normal RFB traffic for --duration
    Pi zero-length REQUEST marker
    PS2 finishes the current complete RFB message and sends BOUNDARY
    Pi shuts down + joins the upstream VNC reader, then sends COMMIT
    PS2 proves channel-1 queue empty and sends COMPLETE
    Pi sends ordinary MEDIA_END
    PS2 sends ordinary SESSION_RESULT

The raw bridge never parses RFB boundaries; the unchanged through-Issue-39 PS2
parser remains protocol authority. The canonical h1_tool.py control surface is
not redirected here until CONFIG/CAP_RFB are deliberately opened.

Temporary preparation-time upstream selection:
    H1_RFB_UPSTREAM_HOST  default 127.0.0.1
    H1_RFB_UPSTREAM_PORT  default 5900
"""

from __future__ import annotations

import json
import os
import socket
import struct
import threading
import time
from typing import Callable

import h1_mux_server_cumulative39_thread_census as cumulative
from h1_rfb_session_adapter import H1RfbSessionAdapter, open_rfb_session_adapter

base = cumulative.base
_ParentSession = base.H1Session

RFB_CONNECTOR: Callable[[str, int], socket.socket] | None = None
RFB_QUIESCE_TIMEOUT_SECONDS = 30.0


def _rfb_upstream() -> tuple[str, int]:
    host = os.environ.get("H1_RFB_UPSTREAM_HOST", "127.0.0.1")
    raw_port = os.environ.get("H1_RFB_UPSTREAM_PORT", "5900")
    try:
        port = int(raw_port, 0)
    except ValueError as exc:
        raise base.ProtocolError(
            f"invalid H1_RFB_UPSTREAM_PORT {raw_port!r}"
        ) from exc
    return host, port


class H1Cumulative39RfbBridgeSession(_ParentSession):
    """Current cumulative H1Session plus optional Pi-local RFB upstream."""

    def __init__(
        self,
        sock: socket.socket,
        profile: dict[str, int],
        evidence,
        duration: float,
        display: str,
    ) -> None:
        super().__init__(sock, profile, evidence, duration, display)
        self.rfb_session_adapter: H1RfbSessionAdapter | None = None

        if int(profile["rfb_mode"]) == 0:
            return

        if int(profile["audio_mode"]) != 0 or int(profile["video_mode"]) != 0:
            raise base.ProtocolError(
                "first RFB mux runner permits RFB-only sessions; AUDIO/MPEG remain off"
            )

        host, port = _rfb_upstream()
        kwargs = {"host": host, "port": port}
        if RFB_CONNECTOR is not None:
            kwargs["connector"] = RFB_CONNECTOR

        self.rfb_session_adapter = open_rfb_session_adapter(self, **kwargs)
        if self.rfb_session_adapter is None:
            raise base.ProtocolError("RFB-ON profile failed to attach Pi bridge")

        print(
            f"H1_RFB_UPSTREAM_ATTACHED={host}:{port}",
            flush=True,
        )

    def _send_rfb_media_end(self) -> dict[str, int]:
        """Terminate only after RFB COMPLETE; no media payload exists in this mode."""

        words = [
            base.MEDIA_END_VERSION,
            self.profile["session_id"],
            0,  # audio bytes
            0,  # audio frames
            0,  # audio last sequence
            0,  # audio crc32
            0,  # mpeg bytes
            0,  # mpeg frames
            0,  # mpeg last sequence
            0,  # mpeg crc32
            0,  # picture starts
            0,  # sequence headers
            0,  # sequence ends
            base.STOP_REASON_FINITE_DURATION,
            0,
            0,
        ]
        payload = struct.pack(">16I", *words)
        if len(payload) != base.MEDIA_END_BYTES:
            raise AssertionError("H1 RFB MEDIA_END payload size mismatch")

        metadata = {
            "session_id": self.profile["session_id"],
            "audio_bytes": 0,
            "audio_frames": 0,
            "audio_last_sequence": 0,
            "audio_crc32": 0,
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
        print(
            "H1_RFB_MEDIA_END_SENT=" + json.dumps(metadata, sort_keys=True),
            flush=True,
        )
        return metadata

    def _run_rfb_only(self) -> None:
        adapter = self.rfb_session_adapter
        if adapter is None:
            raise base.ProtocolError("RFB-only run missing RFB adapter")

        reader = threading.Thread(
            target=self.reader,
            name="h1-ps2-reader",
            daemon=True,
        )
        reader.start()

        if not self.hello_event.wait(timeout=10.0):
            raise base.ProtocolError("timed out waiting for PS2 HELLO")
        self.check_reader()

        self.send_frame(base.FRAME_CONFIG, base.CHANNEL_CONTROL, self.config_payload)
        print(
            "H1_CONFIG_SENT=" + json.dumps(self.profile, sort_keys=True),
            flush=True,
        )

        if not self.config_ack_event.wait(timeout=10.0):
            raise base.ProtocolError("timed out waiting for exact CONFIG ACK")
        self.check_reader()

        # The bridge was started during construction but cannot read VNC bytes
        # until the PS2 sends initial channel-1 receiver credit after CONFIG ACK.
        deadline = time.monotonic() + self.duration
        while True:
            self.check_reader()
            adapter.check()
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break
            time.sleep(min(remaining, 0.05))

        print(
            f"H1_RFB_QUIESCE_REQUEST duration={self.duration:.6f}",
            flush=True,
        )
        adapter.request_quiesce()
        adapter.wait_quiesce_complete(timeout=RFB_QUIESCE_TIMEOUT_SECONDS)
        self.check_reader()

        print(
            "H1_RFB_QUIESCE=COMPLETE "
            f"boundary={int(adapter.quiesce_boundary_received)} "
            f"bridge_stopped={int(adapter.bridge_quiesced)} "
            f"commit={int(adapter.quiesce_commit_sent)} "
            f"complete={int(adapter.quiesce_complete_received)}",
            flush=True,
        )

        metadata = self._send_rfb_media_end()

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
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        self.sock.close()
        reader.join(timeout=1.0)
        if reader.is_alive():
            raise base.ProtocolError("H1 PS2 reader did not stop after RFB session")

    def run(self) -> None:
        if int(self.profile["rfb_mode"]) == 0:
            super().run()
            return
        self._run_rfb_only()

    def cleanup(self) -> None:
        # Preserve lifecycle order: physical PSTV owner closes first through the
        # inherited cleanup, then the logical/upstream RFB bridge is detached.
        try:
            super().cleanup()
        finally:
            if self.rfb_session_adapter is not None:
                self.rfb_session_adapter.stop()
                self.rfb_session_adapter = None


base.H1Session = H1Cumulative39RfbBridgeSession


if __name__ == "__main__":
    raise SystemExit(base.main())
