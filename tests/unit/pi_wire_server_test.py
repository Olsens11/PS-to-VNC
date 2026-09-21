#!/usr/bin/env python3
"""Host proof for the maintained Pi Wire protocol/server foundation."""

from __future__ import annotations

from pathlib import Path
import socket
import sys
import unittest

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "pi"))

import wire_protocol as protocol
import wire_server as server


HELLO_GOLDEN = bytes.fromhex(
    "50535456"
    "01010000"
    "00000001"
    "00000008"
    "00000001"
    "00000001"
)
ACCEPT_GOLDEN = bytes.fromhex(
    "50535456"
    "010c0000"
    "00000001"
    "00000004"
    "12345678"
)
REJECT_GOLDEN = bytes.fromhex(
    "50535456"
    "010d0000"
    "00000001"
    "00000004"
    "00000002"
)


def run_session(
    wire_server: server.WireServer,
    request: bytes,
    *,
    post_bytes: bytes = b"",
) -> tuple[server.WireSessionOutcome, bytes]:
    peer, product = socket.socketpair()
    try:
        peer.sendall(request + post_bytes)
        peer.shutdown(socket.SHUT_WR)
        outcome = wire_server.serve_connection(product)

        response_parts: list[bytes] = []
        while True:
            chunk = peer.recv(4096)
            if not chunk:
                break
            response_parts.append(chunk)

        return outcome, b"".join(response_parts)
    finally:
        peer.close()


class SendFailureSocket:
    def __init__(self, incoming: bytes) -> None:
        self._incoming = bytearray(incoming)
        self.closed = False

    def recv(self, byte_count: int) -> bytes:
        if not self._incoming:
            return b""
        result = bytes(self._incoming[:byte_count])
        del self._incoming[:byte_count]
        return result

    def sendall(self, _data: bytes) -> None:
        raise OSError("injected send failure")

    def close(self) -> None:
        self.closed = True


class WireProtocolTests(unittest.TestCase):
    def test_exact_golden_establishment_bytes(self) -> None:
        self.assertEqual(protocol.encode_hello_frame(), HELLO_GOLDEN)
        self.assertEqual(
            protocol.encode_accept_frame(0x12345678),
            ACCEPT_GOLDEN,
        )
        self.assertEqual(
            protocol.encode_not_accepted_frame(
                protocol.REJECT_PRODUCT_VERSION
            ),
            REJECT_GOLDEN,
        )

        hello_header = protocol.decode_header(
            HELLO_GOLDEN[: protocol.HEADER_BYTES]
        )
        self.assertTrue(protocol.is_hello_header(hello_header))
        self.assertEqual(
            protocol.decode_hello_payload(
                HELLO_GOLDEN[protocol.HEADER_BYTES :]
            ),
            (1, 1),
        )

        accept_header = protocol.decode_header(
            ACCEPT_GOLDEN[: protocol.HEADER_BYTES]
        )
        self.assertTrue(protocol.is_accept_header(accept_header))
        self.assertEqual(
            protocol.decode_accept_payload(
                ACCEPT_GOLDEN[protocol.HEADER_BYTES :]
            ),
            0x12345678,
        )

        reject_header = protocol.decode_header(
            REJECT_GOLDEN[: protocol.HEADER_BYTES]
        )
        self.assertTrue(protocol.is_not_accepted_header(reject_header))
        self.assertEqual(
            protocol.decode_not_accepted_payload(
                REJECT_GOLDEN[protocol.HEADER_BYTES :]
            ),
            protocol.REJECT_PRODUCT_VERSION,
        )

    def test_zero_session_and_unknown_reason_rejected(self) -> None:
        with self.assertRaises(protocol.WireProtocolError):
            protocol.encode_accept_payload(0)
        with self.assertRaises(protocol.WireProtocolError):
            protocol.decode_accept_payload(b"\x00\x00\x00\x00")
        with self.assertRaises(protocol.WireProtocolError):
            protocol.encode_not_accepted_payload(4)


class WireServerTests(unittest.TestCase):
    def test_accepted_session_is_provisional_then_active_and_idle_safe(self) -> None:
        wire_server = server.WireServer(session_ids=server.SessionIdAllocator(10))
        outcome, response = run_session(wire_server, protocol.encode_hello_frame())

        self.assertTrue(outcome.accepted)
        self.assertFalse(outcome.protocol_failed)
        self.assertEqual(outcome.session_id, 10)
        self.assertEqual(outcome.next_receive_sequence, 2)
        self.assertEqual(outcome.next_send_sequence, 2)

        header = protocol.decode_header(response[: protocol.HEADER_BYTES])
        self.assertTrue(protocol.is_accept_header(header))
        self.assertEqual(header.sequence, 1)
        self.assertEqual(
            protocol.decode_accept_payload(response[protocol.HEADER_BYTES :]),
            10,
        )

    def test_wire_and_product_version_rejections_never_activate(self) -> None:
        for wire_version, product_version, reason in (
            (2, 1, protocol.REJECT_WIRE_VERSION),
            (1, 2, protocol.REJECT_PRODUCT_VERSION),
        ):
            with self.subTest(reason=reason):
                wire_server = server.WireServer()
                outcome, response = run_session(
                    wire_server,
                    protocol.encode_hello_frame(
                        wire_version=wire_version,
                        product_version=product_version,
                    ),
                )
                self.assertFalse(outcome.accepted)
                self.assertEqual(outcome.rejection_reason, reason)
                self.assertIsNone(outcome.session_id)
                self.assertEqual(
                    protocol.decode_not_accepted_payload(
                        response[protocol.HEADER_BYTES :]
                    ),
                    reason,
                )

    def test_valid_wire_but_malformed_establishment_fails_closed(self) -> None:
        wrong = protocol.encode_frame(
            protocol.FRAME_ACCEPT,
            1,
            protocol.encode_accept_payload(7),
        )
        wire_server = server.WireServer()
        outcome, response = run_session(wire_server, wrong)

        self.assertFalse(outcome.accepted)
        self.assertTrue(outcome.protocol_failed)
        self.assertIsNone(outcome.session_id)
        self.assertEqual(
            protocol.decode_not_accepted_payload(
                response[protocol.HEADER_BYTES :]
            ),
            protocol.REJECT_MALFORMED,
        )

    def test_invalid_wire_header_closes_without_active_session(self) -> None:
        bad = bytearray(protocol.encode_hello_frame())
        bad[0] ^= 0xFF
        wire_server = server.WireServer()
        outcome, response = run_session(wire_server, bytes(bad))

        self.assertFalse(outcome.accepted)
        self.assertTrue(outcome.protocol_failed)
        self.assertIsNone(outcome.session_id)
        self.assertEqual(response, b"")

    def test_accept_send_failure_never_publishes_active(self) -> None:
        failing = SendFailureSocket(protocol.encode_hello_frame())
        owner = server.WireConnectionOwner(
            failing,
            server.SessionIdAllocator(20),
        )
        outcome = owner.establish()

        self.assertFalse(outcome.accepted)
        self.assertIsNone(outcome.session_id)
        self.assertEqual(owner.state, server.WireSessionState.INACTIVE)

    def test_consecutive_sessions_receive_distinct_monotonic_ids(self) -> None:
        wire_server = server.WireServer(session_ids=server.SessionIdAllocator(30))
        first, _ = run_session(wire_server, protocol.encode_hello_frame())
        second, _ = run_session(wire_server, protocol.encode_hello_frame())

        self.assertEqual(first.session_id, 30)
        self.assertEqual(second.session_id, 31)
        self.assertNotEqual(first.session_id, second.session_id)

    def test_session_protocol_failure_does_not_kill_server_owner(self) -> None:
        wire_server = server.WireServer(session_ids=server.SessionIdAllocator(40))
        failed, _ = run_session(
            wire_server,
            protocol.encode_hello_frame(),
            post_bytes=b"X",
        )
        fresh, _ = run_session(wire_server, protocol.encode_hello_frame())

        self.assertTrue(failed.accepted)
        self.assertTrue(failed.protocol_failed)
        self.assertEqual(failed.session_id, 40)
        self.assertTrue(fresh.accepted)
        self.assertFalse(fresh.protocol_failed)
        self.assertEqual(fresh.session_id, 41)

    def test_session_id_allocator_fails_closed_on_exhaustion(self) -> None:
        allocator = server.SessionIdAllocator(protocol.UINT32_MAX)
        self.assertEqual(allocator.allocate(), protocol.UINT32_MAX)
        with self.assertRaises(server.SessionIdExhausted):
            allocator.allocate()


class RepositoryBoundaryTests(unittest.TestCase):
    def test_wire_service_is_ordinary_supervised_service(self) -> None:
        unit = (
            REPO_ROOT / "systemd/pi/ps-to-vnc-wire.service"
        ).read_text(encoding="utf-8")
        self.assertIn("User=ps2", unit)
        self.assertIn("Group=ps2", unit)
        self.assertIn(
            "ExecStart=/usr/bin/python3 /usr/lib/ps-to-vnc/wire_server.py "
            "--listen 192.168.50.1 --port 5902",
            unit,
        )
        self.assertIn("Restart=on-failure", unit)
        self.assertNotIn("StandardInput=socket", unit)
        self.assertNotIn("ps-to-vnc-rfb", unit)

    def test_stager_has_no_live_systemctl_mutation(self) -> None:
        installer = (
            REPO_ROOT / "scripts/pi/install-wire-runtime.sh"
        ).read_text(encoding="utf-8")
        for forbidden in (
            "systemctl daemon-reload",
            "systemctl enable",
            "systemctl disable",
            "systemctl start",
            "systemctl stop",
            "systemctl restart",
        ):
            self.assertNotIn(forbidden, installer)
        self.assertIn("/usr/lib/ps-to-vnc/wire_protocol.py", installer)
        self.assertIn("/usr/lib/ps-to-vnc/wire_server.py", installer)
        self.assertIn(
            "/etc/systemd/system/ps-to-vnc-wire.service",
            installer,
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
