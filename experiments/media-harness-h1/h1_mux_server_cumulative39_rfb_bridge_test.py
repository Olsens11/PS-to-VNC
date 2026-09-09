#!/usr/bin/env python3
"""Host composition test for the cumulative39 Pi RFB bridge runner."""

from __future__ import annotations

from pathlib import Path
import socket

import h1_mux_server_cumulative39_rfb_bridge as runner
from h1_profiles import resolve_profile


def main() -> int:
    calls: list[tuple[str, int]] = []

    def forbidden(host: str, port: int) -> socket.socket:
        calls.append((host, port))
        raise AssertionError("RFB-OFF cumulative runner opened upstream VNC")

    runner.RFB_CONNECTOR = forbidden
    off_h1, off_ps2 = socket.socketpair()
    off = runner.base.H1Session(
        off_h1,
        resolve_profile("P11_COMPAT_VIDEO_ONLY", 0x3001),
        Path("/tmp/h1-c39-rfb-off"),
        1.0,
        ":0.0",
    )
    try:
        assert off.rfb_session_adapter is None
        assert calls == []
    finally:
        off.cleanup()
        off_ps2.close()

    h1_sock, fake_ps2 = socket.socketpair()
    bridge_sock, fake_vnc = socket.socketpair()
    calls = []

    def connector(host: str, port: int) -> socket.socket:
        calls.append((host, port))
        assert host == "127.0.0.1"
        assert port == 5900
        return bridge_sock

    runner.RFB_CONNECTOR = connector
    on_profile = resolve_profile("H1_RFB_ONLY", 0x3002)
    on = runner.base.H1Session(
        h1_sock,
        on_profile,
        Path("/tmp/h1-c39-rfb-on"),
        1.0,
        ":0.0",
    )
    try:
        assert calls == [("127.0.0.1", 5900)]
        assert on.profile["profile_id"] == 2
        assert on.profile["audio_mode"] == 0
        assert on.profile["video_mode"] == 0
        assert on.profile["audio_queue_capacity"] == 0
        assert on.profile["mpeg_queue_capacity"] == 0
        assert on.profile["video_encode_width"] == 0
        assert on.profile["video_encode_height"] == 0
        assert on.profile["rfb_mode"] == 1
        assert on.profile["rfb_queue_capacity"] == 32768
        assert on.profile["rfb_credit_batch_bytes"] == 8192
        assert on.profile["rfb_credit_flush_on_empty"] == 1
        assert on.profile["rfb_credit_return_enabled"] == 1
        assert on.profile["rfb_initial_credit_bytes"] == 32768
        assert on.video_command() == []
        assert on.rfb_session_adapter is not None
        assert on.rfb_session_adapter.started
        assert on.rfb_session_adapter.bridge.queue_capacity == 32768
        assert on.rfb_session_adapter.bridge.max_payload == 8192
    finally:
        on.cleanup()
        fake_ps2.close()
        fake_vnc.close()
        runner.RFB_CONNECTOR = None

    print("H1_CUMULATIVE39_RFB_BRIDGE_RUNNER_TEST=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
