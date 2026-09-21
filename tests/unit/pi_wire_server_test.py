#!/usr/bin/env python3
"""Host proof for the maintained Pi Wire protocol/server foundation."""

from __future__ import annotations

from pathlib import Path
import socket
import sys
import threading
import time
import unittest

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "pi"))

import rfb_relay as rfb
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
RFB_CREDIT_GOLDEN = bytes.fromhex(
    "50535456"
    "01040100"
    "00000002"
    "00000004"
    "00000008"
)
RFB_DATA_GOLDEN = bytes.fromhex(
    "50535456"
    "01030100"
    "00000003"
    "00000003"
    "616263"
)


def recv_exact_socket(connection: socket.socket, byte_count: int) -> bytes:
    parts: list[bytes] = []
    remaining = byte_count
    while remaining:
        chunk = connection.recv(remaining)
        if not chunk:
            raise EOFError("socket closed during test framed read")
        parts.append(chunk)
        remaining -= len(chunk)
    return b"".join(parts)


def recv_wire_frame(
    connection: socket.socket,
) -> tuple[protocol.WireHeader, bytes]:
    raw_header = recv_exact_socket(connection, protocol.HEADER_BYTES)
    header = protocol.decode_header(raw_header)
    payload = recv_exact_socket(connection, header.payload_length)
    return header, payload


def wait_for(predicate, description: str, timeout: float = 2.0) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.001)
    raise AssertionError(f"timed out waiting for {description}")


def fill_provider_send_buffer(provider: socket.socket) -> int:
    provider.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 4096)
    provider.setblocking(False)
    total = 0
    block = b"Z" * 4096

    while True:
        try:
            sent = provider.send(block)
        except BlockingIOError:
            break
        if sent <= 0:
            break
        total += sent

    if total == 0:
        raise AssertionError("failed to prefill provider send buffer")
    return total


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
            try:
                chunk = peer.recv(4096)
            except ConnectionResetError:
                break
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

    def test_exact_rfb_data_and_credit_bytes(self) -> None:
        self.assertEqual(
            protocol.encode_rfb_credit_frame(8, sequence=2),
            RFB_CREDIT_GOLDEN,
        )
        self.assertEqual(
            protocol.encode_rfb_data_frame(b"abc", sequence=3),
            RFB_DATA_GOLDEN,
        )

        credit_header = protocol.decode_header(
            RFB_CREDIT_GOLDEN[: protocol.HEADER_BYTES]
        )
        self.assertTrue(protocol.is_rfb_credit_header(credit_header))
        self.assertEqual(
            protocol.decode_rfb_credit_payload(
                RFB_CREDIT_GOLDEN[protocol.HEADER_BYTES :]
            ),
            8,
        )

        data_header = protocol.decode_header(
            RFB_DATA_GOLDEN[: protocol.HEADER_BYTES]
        )
        self.assertTrue(protocol.is_rfb_data_header(data_header))
        self.assertEqual(
            RFB_DATA_GOLDEN[protocol.HEADER_BYTES :],
            b"abc",
        )

        with self.assertRaises(protocol.WireProtocolError):
            protocol.encode_rfb_credit_payload(0)
        with self.assertRaises(protocol.WireProtocolError):
            protocol.decode_rfb_credit_payload(b"\x00\x00\x00\x00")


class WireServerTests(unittest.TestCase):
    def test_accepted_session_is_provisional_then_active_and_idle_safe(self) -> None:
        wire_server = server.WireServer(session_ids=server.SessionIdAllocator(10))
        peer, product = socket.socketpair()
        outcomes: list[server.WireSessionOutcome] = []

        def serve() -> None:
            outcomes.append(wire_server.serve_connection(product))

        worker = threading.Thread(target=serve)
        worker.start()
        try:
            peer.sendall(protocol.encode_hello_frame())
            response = b""
            expected_bytes = (
                protocol.HEADER_BYTES + protocol.ONE_WORD.size
            )
            while len(response) < expected_bytes:
                response += peer.recv(expected_bytes - len(response))

            header = protocol.decode_header(
                response[: protocol.HEADER_BYTES]
            )
            self.assertTrue(protocol.is_accept_header(header))
            self.assertEqual(header.sequence, 1)
            self.assertEqual(
                protocol.decode_accept_payload(
                    response[protocol.HEADER_BYTES :]
                ),
                10,
            )

            # No rider or heartbeat follows ACCEPT. The owner must remain alive
            # waiting on the active physical session until the peer ends it.
            self.assertTrue(worker.is_alive())

            peer.shutdown(socket.SHUT_WR)
            worker.join(timeout=1.0)
            self.assertFalse(worker.is_alive())
            self.assertEqual(len(outcomes), 1)

            outcome = outcomes[0]
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
            self.assertEqual(outcome.session_id, 10)
            self.assertEqual(outcome.next_receive_sequence, 2)
            self.assertEqual(outcome.next_send_sequence, 2)
        finally:
            peer.close()
            if worker.is_alive():
                worker.join(timeout=1.0)

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
        allocator = server.SessionIdAllocator(20)
        owner = server.WireConnectionOwner(failing, allocator)
        outcome = owner.establish()

        self.assertFalse(outcome.accepted)
        self.assertIsNone(outcome.session_id)
        self.assertEqual(owner.state, server.WireSessionState.INACTIVE)
        self.assertEqual(allocator.allocate(), 21)

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




class RfbRelayTests(unittest.TestCase):
    def test_provider_reads_require_ps2_credit_and_fragment_to_budget(self) -> None:
        relay_socket, fake_provider = socket.socketpair()
        relay = rfb.RfbRelay(
            relay_socket,
            provider_read_credit_limit=8,
            provider_write_capacity=8,
            max_data_payload=4,
        )
        try:
            fake_provider.sendall(b"abcdef")

            # Bytes are already waiting in the kernel, but the relay must not
            # read a single provider byte before the PS2 grants RFB credit.
            self.assertIsNone(relay.read_provider_ready())
            self.assertEqual(relay.stats.provider_read_calls, 0)

            self.assertTrue(relay.add_provider_read_credit(3))
            self.assertEqual(relay.read_provider_ready(), b"abc")
            self.assertEqual(relay.provider_read_credit, 0)

            self.assertTrue(relay.add_provider_read_credit(3))
            self.assertEqual(relay.read_provider_ready(), b"def")
            self.assertEqual(relay.provider_read_credit, 0)
            self.assertEqual(relay.stats.provider_bytes_read, 6)
        finally:
            relay.close()
            fake_provider.close()

    def test_provider_write_capacity_returns_credit_only_after_drain(self) -> None:
        relay_socket, fake_provider = socket.socketpair()
        relay = rfb.RfbRelay(
            relay_socket,
            provider_read_credit_limit=8,
            provider_write_capacity=8,
            max_data_payload=4,
        )
        try:
            self.assertEqual(relay.activate(), 8)
            self.assertEqual(relay.ps2_write_credit, 8)

            self.assertTrue(relay.accept_ps2_data(b"abcd"))
            self.assertEqual(relay.queued_provider_write_bytes, 4)
            self.assertEqual(relay.ps2_write_credit, 4)
            self.assertEqual(relay.pending_credit_return, 0)

            self.assertEqual(relay.drain_provider_write_ready(), 4)
            self.assertEqual(
                recv_exact_socket(fake_provider, 4),
                b"abcd",
            )
            self.assertEqual(relay.queued_provider_write_bytes, 0)
            self.assertEqual(relay.pending_credit_return, 4)

            # Local free space is not peer authority until the Wire owner has
            # actually serialized the replacement CREDIT.
            self.assertEqual(relay.ps2_write_credit, 4)
            relay.confirm_credit_sent(4)
            self.assertEqual(relay.pending_credit_return, 0)
            self.assertEqual(relay.ps2_write_credit, 8)
        finally:
            relay.close()
            fake_provider.close()

    def test_session_b_starts_with_fresh_queue_and_credit_state(self) -> None:
        a_socket, a_peer = socket.socketpair()
        relay_a = rfb.RfbRelay(
            a_socket,
            provider_read_credit_limit=8,
            provider_write_capacity=8,
            max_data_payload=4,
        )
        try:
            self.assertEqual(relay_a.activate(), 8)
            self.assertTrue(relay_a.accept_ps2_data(b"abcd"))
            self.assertTrue(relay_a.add_provider_read_credit(4))
        finally:
            relay_a.close()
            a_peer.close()

        b_socket, b_peer = socket.socketpair()
        relay_b = rfb.RfbRelay(
            b_socket,
            provider_read_credit_limit=8,
            provider_write_capacity=8,
            max_data_payload=4,
        )
        try:
            self.assertFalse(relay_b.activated)
            self.assertFalse(relay_b.terminal)
            self.assertEqual(relay_b.provider_read_credit, 0)
            self.assertEqual(relay_b.ps2_write_credit, 0)
            self.assertEqual(relay_b.pending_credit_return, 0)
            self.assertEqual(relay_b.queued_provider_write_bytes, 0)
            self.assertEqual(relay_b.activate(), 8)
        finally:
            relay_b.close()
            b_peer.close()


class RfbWireCompositionTests(unittest.TestCase):
    def test_stalled_provider_write_does_not_block_opposite_wire_progress(
        self,
    ) -> None:
        wire_peer, product_wire = socket.socketpair()
        provider_socket, fake_provider = socket.socketpair()

        # Fill only provider_socket -> fake_provider direction. The reverse
        # fake_provider -> provider_socket direction remains independently
        # usable, which lets the test prove full-provider-write isolation.
        prefill_bytes = fill_provider_send_buffer(provider_socket)

        relay = rfb.RfbRelay(
            provider_socket,
            provider_read_credit_limit=8,
            provider_write_capacity=8,
            max_data_payload=4,
        )
        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(90)
        )
        outcomes: list[server.WireSessionOutcome] = []

        def serve() -> None:
            outcomes.append(
                wire_server.serve_connection(product_wire, relay)
            )

        worker = threading.Thread(target=serve)
        worker.start()

        try:
            wire_peer.sendall(protocol.encode_hello_frame())

            accept_header, accept_payload = recv_wire_frame(wire_peer)
            self.assertTrue(protocol.is_accept_header(accept_header))
            self.assertEqual(accept_header.sequence, 1)
            self.assertEqual(
                protocol.decode_accept_payload(accept_payload),
                90,
            )

            initial_header, initial_payload = recv_wire_frame(wire_peer)
            self.assertTrue(protocol.is_rfb_credit_header(initial_header))
            self.assertEqual(initial_header.sequence, 2)
            self.assertEqual(
                protocol.decode_rfb_credit_payload(initial_payload),
                8,
            )

            # Consume half of Pi-granted provider-write capacity. The provider's
            # send direction is intentionally full, so this must remain queued
            # and must not earn replacement credit yet.
            wire_peer.sendall(
                protocol.encode_rfb_data_frame(b"abcd", sequence=2)
            )
            wait_for(
                lambda: relay.queued_provider_write_bytes == 4,
                "bounded provider-write queue fill",
            )
            self.assertEqual(relay.ps2_write_credit, 4)

            wire_peer.settimeout(0.05)
            with self.assertRaises(TimeoutError):
                wire_peer.recv(1)
            wire_peer.settimeout(None)
            self.assertEqual(relay.pending_credit_return, 0)

            # Even while the provider-write direction is stalled, reverse
            # provider bytes and an RFB CREDIT frame remain serviceable through
            # the same sole Wire owner.
            fake_provider.sendall(b"WXYZ")
            wire_peer.sendall(
                protocol.encode_rfb_credit_frame(4, sequence=3)
            )
            data_header, data_payload = recv_wire_frame(wire_peer)
            self.assertTrue(protocol.is_rfb_data_header(data_header))
            self.assertEqual(data_header.sequence, 3)
            self.assertEqual(data_payload, b"WXYZ")

            # Zero-length channel-1 DATA is consumed only as the reserved
            # lifecycle marker and never enters the provider byte stream.
            wire_peer.sendall(
                protocol.encode_rfb_data_frame(b"", sequence=4)
            )
            wait_for(
                lambda: relay.stats.reserved_zero_length_markers == 1,
                "reserved zero-length RFB marker",
            )
            self.assertEqual(relay.queued_provider_write_bytes, 4)

            # Drain the artificial provider backlog. Once the provider socket is
            # actually writable, the queued client bytes leave and only then
            # does the Wire owner return matching capacity as CREDIT.
            fake_provider.settimeout(1.0)
            received = bytearray()
            while b"abcd" not in received:
                received.extend(fake_provider.recv(65536))
            self.assertGreaterEqual(len(received), prefill_bytes + 4)

            credit_header, credit_payload = recv_wire_frame(wire_peer)
            self.assertTrue(protocol.is_rfb_credit_header(credit_header))
            self.assertEqual(credit_header.sequence, 4)
            self.assertEqual(
                protocol.decode_rfb_credit_payload(credit_payload),
                4,
            )
            self.assertEqual(relay.pending_credit_return, 0)
            self.assertEqual(relay.ps2_write_credit, 8)

            # Provider EOF is RFB-local. Granting read budget makes the EOF
            # observable, but the containing Wire Session remains alive and can
            # still consume later channel-1 control/credit traffic.
            fake_provider.close()
            wire_peer.sendall(
                protocol.encode_rfb_credit_frame(1, sequence=5)
            )
            wait_for(lambda: relay.terminal, "provider-local EOF")
            self.assertTrue(worker.is_alive())

            wire_peer.sendall(
                protocol.encode_rfb_credit_frame(1, sequence=6)
            )
            time.sleep(0.01)
            self.assertTrue(worker.is_alive())

            wire_peer.shutdown(socket.SHUT_WR)
            worker.join(timeout=1.0)
            self.assertFalse(worker.is_alive())
            self.assertEqual(len(outcomes), 1)
            self.assertTrue(outcomes[0].accepted)
            self.assertFalse(outcomes[0].protocol_failed)
            self.assertEqual(outcomes[0].session_id, 90)
        finally:
            wire_peer.close()
            if worker.is_alive():
                worker.join(timeout=1.0)
            try:
                fake_provider.close()
            except OSError:
                pass

class RepositoryBoundaryTests(unittest.TestCase):

    def test_rfb_relay_has_no_wire_socket_or_provider_endpoint_policy(self) -> None:
        relay_source = (
            REPO_ROOT / "pi/rfb_relay.py"
        ).read_text(encoding="utf-8")

        for forbidden in (
            "192.168.50.1:5900",
            "127.0.0.1:5903",
            "TigerVNC",
            "X0tigervnc",
            "_connection",
            "sendall(",
        ):
            self.assertNotIn(forbidden, relay_source)

        server_source = (
            REPO_ROOT / "pi/wire_server.py"
        ).read_text(encoding="utf-8")
        self.assertIn("self.serve_connection(connection)", server_source)
        self.assertNotIn("--rfb-provider", server_source)

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
        self.assertIn("/usr/lib/ps-to-vnc/rfb_relay.py", installer)
        self.assertIn("/usr/lib/ps-to-vnc/wire_server.py", installer)
        self.assertIn(
            "/etc/systemd/system/ps-to-vnc-wire.service",
            installer,
        )

    def test_stager_enforces_exact_identity_and_modes(self) -> None:
        installer = (
            REPO_ROOT / "scripts/pi/install-wire-runtime.sh"
        ).read_text(encoding="utf-8")

        # Stage refuses an unknown pre-existing target before install, then
        # verifies the installed bytes. Remove repeats the same identity fence
        # before deleting anything.
        self.assertIn(
            'if [ -e "$target" ] && ! cmp -s "$source" "$target"',
            installer,
        )
        self.assertIn(
            'install -D -m "$mode" "$source" "$target"',
            installer,
        )
        self.assertIn(
            'cmp -s "$source" "$target" || {',
            installer,
        )
        self.assertIn(
            'verify_file 0755 "$PROTOCOL_SOURCE" "$PROTOCOL_DEST"',
            installer,
        )
        self.assertIn(
            'verify_file 0644 "$RFB_RELAY_SOURCE" "$RFB_RELAY_DEST"',
            installer,
        )
        self.assertIn(
            'verify_file 0755 "$SERVER_SOURCE" "$SERVER_DEST"',
            installer,
        )
        self.assertIn(
            'verify_file 0644 "$UNIT_SOURCE" "$UNIT_DEST"',
            installer,
        )
        self.assertIn(
            'systemd-analyze verify "$UNIT_DEST"',
            installer,
        )
        self.assertIn(
            'cmp -s "$source" "$target" || {',
            installer,
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
