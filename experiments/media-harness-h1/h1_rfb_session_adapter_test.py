#!/usr/bin/env python3
"""
Host integration test for H1Session reader + credit-driven Pi RFB bridge.

Socketpair endpoints prove that the existing H1Session.reader() remains the
physical PSTV receive owner while registered RFB channel-1 CREDIT/DATA are
consumed by the experiment adapter, global sequence numbers remain correct,
upstream reads stop at PS2-granted credit, reverse-direction bytes are exact,
RFB-OFF opens no upstream socket, and the four zero-length quiesce markers stop
and join the upstream bridge before COMMIT.
"""

from __future__ import annotations

from pathlib import Path
import socket
import struct
import threading
import time

import h1_mux_server as base
from h1_profiles import resolve_profile
from h1_rfb_session_adapter import open_rfb_session_adapter


def wait_for(predicate, description: str, timeout: float = 2.0) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.005)
    raise AssertionError(f"timed out waiting for {description}")


def send_ps2_frame(
    sock: socket.socket,
    *,
    sequence: int,
    kind: int,
    channel: int,
    payload: bytes = b"",
    flags: int = 0,
) -> None:
    sock.sendall(base.encode_header(kind, channel, flags, sequence, len(payload)))
    if payload:
        sock.sendall(payload)


def make_session(sock: socket.socket, profile: dict[str, int]) -> base.H1Session:
    return base.H1Session(sock, profile, Path("/tmp/h1-rfb-session-adapter-test"), 1.0, ":0.0")


def main() -> int:
    # RFB-OFF must be truly inert on the Pi side.
    off_h1, off_ps2 = socket.socketpair()
    off_profile = resolve_profile("P11_COMPAT_VIDEO_ONLY", 0x1001)
    off_session = make_session(off_h1, off_profile)
    connector_calls: list[tuple[str, int]] = []

    def forbidden_connector(host: str, port: int) -> socket.socket:
        connector_calls.append((host, port))
        raise AssertionError("RFB-OFF attempted to open upstream VNC")

    assert open_rfb_session_adapter(
        off_session,
        connector=forbidden_connector,
    ) is None
    assert connector_calls == []
    off_h1.close()
    off_ps2.close()

    h1_sock, fake_ps2 = socket.socketpair()
    bridge_sock, fake_vnc = socket.socketpair()

    # Reproduce the production condition from socket.create_connection(timeout=5):
    # a connected upstream socket can retain a finite read timeout. The adapter
    # contract must clear it because an idle RFB incremental request can remain
    # silent indefinitely without indicating transport failure.
    bridge_sock.settimeout(0.05)

    profile = resolve_profile(
        "P11_COMPAT_VIDEO_ONLY",
        0x2002,
        {
            "rfb_mode": 1,
            "rfb_queue_capacity": 32,
            "rfb_credit_batch_bytes": 8,
            "rfb_credit_flush_on_empty": 1,
            "rfb_credit_return_enabled": 1,
            "rfb_initial_credit_bytes": 32,
            "max_data_payload": 8,
        },
    )
    session = make_session(h1_sock, profile)

    connector_calls = []

    def connector(host: str, port: int) -> socket.socket:
        connector_calls.append((host, port))
        assert host == "127.0.0.1"
        assert port == 5900
        return bridge_sock

    adapter = open_rfb_session_adapter(session, connector=connector)
    assert adapter is not None
    assert connector_calls == [("127.0.0.1", 5900)]
    assert adapter.upstream.gettimeout() is None

    reader = threading.Thread(target=session.reader, name="h1-rfb-adapter-test-reader")
    reader.start()

    try:
        hello_payload = struct.pack(
            ">6I",
            base.REQUIRED_CAPS,
            base.MAX_PAYLOAD,
            0,
            0,
            0,
            0,
        )
        send_ps2_frame(
            fake_ps2,
            sequence=1,
            kind=base.FRAME_HELLO,
            channel=base.CHANNEL_CONTROL,
            payload=hello_payload,
        )
        wait_for(session.hello_event.is_set, "base HELLO dispatch")

        send_ps2_frame(
            fake_ps2,
            sequence=2,
            kind=base.FRAME_CONFIG,
            channel=base.CHANNEL_CONTROL,
            payload=session.config_payload,
            flags=base.CONFIG_ACK_FLAG,
        )
        wait_for(session.config_ack_event.is_set, "base CONFIG ACK dispatch")

        server_bytes = b"abcdefghijklm"
        fake_vnc.sendall(server_bytes)
        fake_ps2.settimeout(0.05)
        try:
            unexpected = fake_ps2.recv(1)
        except TimeoutError:
            unexpected = b""
        finally:
            fake_ps2.settimeout(None)
        assert unexpected == b""

        send_ps2_frame(
            fake_ps2,
            sequence=3,
            kind=base.FRAME_CREDIT,
            channel=1,
            payload=struct.pack(">I", 8),
        )
        outbound = base.receive_frame(fake_ps2)
        assert outbound.sequence == 1
        assert outbound.kind == base.FRAME_DATA
        assert outbound.channel == 1
        assert outbound.flags == 0
        assert outbound.payload == server_bytes[:8]
        assert adapter.bridge.credit == 0

        send_ps2_frame(
            fake_ps2,
            sequence=4,
            kind=base.FRAME_CREDIT,
            channel=base.CHANNEL_AUDIO,
            payload=struct.pack(">I", 4),
        )
        wait_for(lambda: session.audio.credit == 4, "interleaved base AUDIO credit")
        session.check_reader()

        client_bytes = b"RFBcli"
        send_ps2_frame(
            fake_ps2,
            sequence=5,
            kind=base.FRAME_DATA,
            channel=1,
            payload=client_bytes,
        )
        fake_vnc.settimeout(1.0)
        assert fake_vnc.recv(len(client_bytes)) == client_bytes
        fake_vnc.settimeout(None)

        send_ps2_frame(
            fake_ps2,
            sequence=6,
            kind=base.FRAME_CREDIT,
            channel=1,
            payload=struct.pack(">I", 5),
        )
        outbound = base.receive_frame(fake_ps2)
        assert outbound.sequence == 2
        assert outbound.kind == base.FRAME_DATA
        assert outbound.channel == 1
        assert outbound.payload == server_bytes[8:]
        assert adapter.bridge.credit == 0

        send_ps2_frame(
            fake_ps2,
            sequence=7,
            kind=base.FRAME_CREDIT,
            channel=base.CHANNEL_MPEG2,
            payload=struct.pack(">I", 3),
        )
        wait_for(lambda: session.mpeg.credit == 3, "interleaved base MPEG credit")
        session.check_reader()
        adapter.check()

        # Stay completely idle for longer than the finite timeout the connector
        # originally supplied. This is normal RFB behavior: the server may send
        # nothing until the framebuffer changes. The bridge must remain healthy.
        time.sleep(0.10)
        adapter.check()

        stats = adapter.bridge.stats
        assert stats.credit_frames_received == 2
        assert stats.credit_bytes_received == len(server_bytes)
        assert stats.server_bytes_received == len(server_bytes)
        assert stats.channel_bytes_sent == len(server_bytes)
        assert stats.channel_frames_received == 1
        assert stats.channel_bytes_received == len(client_bytes)
        assert stats.server_bytes_sent == len(client_bytes)
        assert session.expected_rx_sequence == 8

        # Pi REQUEST: zero-length channel-1 DATA contains no RFB bytes.
        adapter.request_quiesce()
        request = base.receive_frame(fake_ps2)
        assert request.sequence == 3
        assert request.kind == base.FRAME_DATA
        assert request.channel == 1
        assert request.payload == b""

        # PS2 BOUNDARY arrives on the globally next receive sequence. The reader
        # must stop+join the upstream bridge before emitting Pi COMMIT.
        send_ps2_frame(
            fake_ps2,
            sequence=8,
            kind=base.FRAME_DATA,
            channel=1,
            payload=b"",
        )
        assert adapter.quiesce_boundary_event.wait(timeout=2.0)

        commit = base.receive_frame(fake_ps2)
        assert commit.sequence == 4
        assert commit.kind == base.FRAME_DATA
        assert commit.channel == 1
        assert commit.payload == b""

        # sendall() can make COMMIT visible to the peer a few instructions before
        # the adapter records its local post-send flag. Wait for that local state
        # instead of making the test scheduler-sensitive.
        wait_for(lambda: adapter.quiesce_commit_sent, "local COMMIT state")
        assert adapter.bridge_quiesced
        assert adapter.quiesce_boundary_received
        assert adapter.bridge.thread is not None
        assert not adapter.bridge.thread.is_alive()

        # Upstream socket shutdown is part of the deterministic boundary->commit
        # transition, so the VNC peer now observes EOF.
        fake_vnc.settimeout(1.0)
        assert fake_vnc.recv(1) == b""
        fake_vnc.settimeout(None)

        # PS2 COMPLETE closes the four-phase handshake. No RFB parser byte was
        # invented by any of the zero-length PSTV lifecycle markers.
        send_ps2_frame(
            fake_ps2,
            sequence=9,
            kind=base.FRAME_DATA,
            channel=1,
            payload=b"",
        )
        adapter.wait_quiesce_complete(timeout=2.0)
        assert adapter.quiesce_complete_received
        assert session.expected_rx_sequence == 10
        session.check_reader()

    finally:
        session.stop_event.set()
        try:
            h1_sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        try:
            fake_ps2.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        reader.join(timeout=2.0)
        assert not reader.is_alive()
        adapter.stop()
        fake_vnc.close()
        fake_ps2.close()
        try:
            h1_sock.close()
        except OSError:
            pass

    assert session.reader_error is None
    print("H1_RFB_SESSION_ADAPTER_TEST=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
