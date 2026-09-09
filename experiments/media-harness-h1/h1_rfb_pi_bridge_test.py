#!/usr/bin/env python3
"""Host test for credit-driven bidirectional H1 RFB Pi bridge behavior."""

from __future__ import annotations

import socket
import threading
import time

from h1_rfb_pi_bridge import H1RfbPiBridge, RfbBridgeError


def wait_for(predicate, description: str, timeout: float = 2.0) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.005)
    raise AssertionError(f"timed out waiting for {description}")


def main() -> int:
    bridge_socket, fake_vnc = socket.socketpair()
    captured: list[bytes] = []
    captured_lock = threading.Lock()

    def capture(payload: bytes) -> None:
        with captured_lock:
            captured.append(bytes(payload))

    bridge = H1RfbPiBridge(
        bridge_socket,
        queue_capacity=32,
        max_payload=8,
        send_data=capture,
    )
    bridge.start()

    try:
        # VNC server data must not leave the Pi until PS2 grants channel-1 credit.
        server_bytes = b"abcdefghijklm"
        fake_vnc.sendall(server_bytes)
        time.sleep(0.05)
        with captured_lock:
            assert b"".join(captured) == b""

        bridge.add_credit(8)
        wait_for(
            lambda: sum(len(part) for part in captured) == 8,
            "first 8 credited bytes",
        )
        with captured_lock:
            assert b"".join(captured) == server_bytes[:8]
        assert bridge.credit == 0

        # Replenishment releases exactly the remaining five bytes; no arbitrary
        # bridge-side reservoir is needed.
        bridge.add_credit(5)
        wait_for(
            lambda: sum(len(part) for part in captured) == len(server_bytes),
            "remaining credited bytes",
        )
        with captured_lock:
            assert b"".join(captured) == server_bytes
        assert all(0 < len(part) <= 8 for part in captured)
        assert bridge.credit == 0

        # Opposite direction is exact raw-byte forwarding from PS2 channel 1 to
        # the already-owned upstream VNC socket.
        client_bytes = b"RFB-client"
        # Exercise framing-sized writes exactly as H1 would deliver them.
        bridge.accept_client_data(client_bytes[:8])
        bridge.accept_client_data(client_bytes[8:])
        received = bytearray()
        while len(received) < len(client_bytes):
            received.extend(fake_vnc.recv(len(client_bytes) - len(received)))
        assert bytes(received) == client_bytes

        # Invalid producer credit cannot silently exceed the configured PS2
        # receive window.
        try:
            bridge.add_credit(33)
        except RfbBridgeError:
            pass
        else:
            raise AssertionError("over-capacity RFB credit was accepted")

        bridge.check()
        assert bridge.stats.credit_frames_received == 2
        assert bridge.stats.credit_bytes_received == len(server_bytes)
        assert bridge.stats.server_bytes_received == len(server_bytes)
        assert bridge.stats.channel_bytes_sent == len(server_bytes)
        assert bridge.stats.channel_frames_received == 2
        assert bridge.stats.channel_bytes_received == len(client_bytes)
        assert bridge.stats.server_bytes_sent == len(client_bytes)

    finally:
        bridge.stop()
        fake_vnc.close()

    print("H1_RFB_PI_BRIDGE_TEST=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
