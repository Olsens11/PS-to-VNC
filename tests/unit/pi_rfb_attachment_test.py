#!/usr/bin/env python3
"""Host proof for A003 R13 Pi RFB attachment and ordered quiesce."""

from __future__ import annotations

import errno
from pathlib import Path
import socket
import sys
import threading
import time
import unittest

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "pi"))

import rfb_attachment as attachment
import wire_protocol as protocol
import wire_server as server


def recv_exact(connection: socket.socket, byte_count: int) -> bytes:
    parts: list[bytes] = []
    remaining = byte_count
    while remaining:
        chunk = connection.recv(remaining)
        if not chunk:
            raise EOFError("socket closed during framed test read")
        parts.append(chunk)
        remaining -= len(chunk)
    return b"".join(parts)


def recv_frame(
    connection: socket.socket,
) -> tuple[protocol.WireHeader, bytes]:
    raw_header = recv_exact(connection, protocol.HEADER_BYTES)
    header = protocol.decode_header(raw_header)
    return header, recv_exact(connection, header.payload_length)


def wait_for(predicate, description: str, timeout: float = 2.0) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.001)
    raise AssertionError(f"timed out waiting for {description}")


def fill_send_buffer(provider: socket.socket) -> int:
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
    provider.setblocking(True)
    if total == 0:
        raise AssertionError("failed to fill provider send buffer")
    return total


class ControlledConnectSocket:
    """Wrap one socketpair end with deterministic nonblocking-connect results."""

    def __init__(
        self,
        raw: socket.socket,
        *,
        connect_result: int,
        socket_error: int = 0,
    ) -> None:
        self.raw = raw
        self.connect_result = connect_result
        self.socket_error = socket_error
        self.connect_addresses: list[tuple[str, int]] = []
        self.closed = False
        self.fail_send = False

    def fileno(self) -> int:
        return self.raw.fileno()

    def setblocking(self, enabled: bool) -> None:
        self.raw.setblocking(enabled)

    def connect_ex(self, address: tuple[str, int]) -> int:
        self.connect_addresses.append(address)
        return self.connect_result

    def getsockopt(self, level: int, option: int) -> int:
        if level == socket.SOL_SOCKET and option == socket.SO_ERROR:
            return self.socket_error
        return self.raw.getsockopt(level, option)

    def recv(self, byte_count: int) -> bytes:
        return self.raw.recv(byte_count)

    def send(self, payload: bytes | bytearray) -> int:
        if self.fail_send:
            raise OSError("injected provider write failure")
        return self.raw.send(payload)

    def close(self) -> None:
        self.closed = True
        self.raw.close()


class ProviderSocketFactory:
    def __init__(self, sockets: list[ControlledConnectSocket]) -> None:
        self.sockets = sockets
        self.calls = 0

    def __call__(self) -> ControlledConnectSocket:
        if self.calls >= len(self.sockets):
            raise AssertionError("unexpected extra provider connect attempt")
        provider = self.sockets[self.calls]
        self.calls += 1
        return provider


def flow() -> attachment.RfbFlowConfig:
    return attachment.RfbFlowConfig(
        provider_read_credit_limit=8,
        provider_write_capacity=8,
        max_data_payload=4,
    )


def start_active_session(
    wire_server: server.WireServer,
) -> tuple[
    socket.socket,
    threading.Thread,
    list[server.WireSessionOutcome],
]:
    peer, product = socket.socketpair()
    outcomes: list[server.WireSessionOutcome] = []

    def serve() -> None:
        outcomes.append(wire_server.serve_connection(product))

    worker = threading.Thread(target=serve)
    worker.start()

    peer.sendall(protocol.encode_hello_frame())
    header, payload = recv_frame(peer)
    if not protocol.is_accept_header(header):
        raise AssertionError("session did not receive ACCEPT")
    if protocol.decode_accept_payload(payload) == 0:
        raise AssertionError("ACCEPT session ID must be nonzero")
    return peer, worker, outcomes


def finish_wire(
    peer: socket.socket,
    worker: threading.Thread,
    outcomes: list[server.WireSessionOutcome],
) -> server.WireSessionOutcome:
    peer.shutdown(socket.SHUT_WR)
    worker.join(timeout=2.0)
    if worker.is_alive():
        raise AssertionError("Wire worker did not retire")
    if len(outcomes) != 1:
        raise AssertionError("expected one Wire outcome")
    peer.close()
    return outcomes[0]


class RfbFlowConfigTests(unittest.TestCase):
    def test_flow_limits_are_explicit_and_protocol_bounded(self) -> None:
        valid = flow()
        self.assertEqual(valid.provider_read_credit_limit, 8)
        self.assertEqual(valid.provider_write_capacity, 8)
        self.assertEqual(valid.max_data_payload, 4)

        with self.assertRaises(ValueError):
            attachment.RfbFlowConfig(0, 8, 4)
        with self.assertRaises(ValueError):
            attachment.RfbFlowConfig(8, 0, 4)
        with self.assertRaises(ValueError):
            attachment.RfbFlowConfig(8, 8, 0)
        with self.assertRaises(ValueError):
            attachment.RfbFlowConfig(8, 8, 9)
        with self.assertRaises(ValueError):
            attachment.RfbFlowConfig(
                protocol.UINT32_MAX + 1,
                8,
                4,
            )


class RfbAttachmentIntegrationTests(unittest.TestCase):
    def test_active_idle_wire_never_opens_provider(self) -> None:
        raw, provider_peer = socket.socketpair()
        controlled = ControlledConnectSocket(
            raw,
            connect_result=errno.EINPROGRESS,
            socket_error=errno.EINPROGRESS,
        )
        provider_factory = ProviderSocketFactory([controlled])
        attachments: list[attachment.RfbAttachment] = []

        def make_attachment() -> attachment.RfbAttachment:
            item = attachment.RfbAttachment(
                flow(),
                socket_factory=provider_factory,
            )
            attachments.append(item)
            return item

        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(100),
            rfb_attachment_factory=make_attachment,
        )
        peer, worker, outcomes = start_active_session(wire_server)
        try:
            self.assertEqual(len(attachments), 1)
            self.assertEqual(
                attachments[0].state,
                attachment.RfbAttachmentState.IDLE,
            )
            self.assertEqual(provider_factory.calls, 0)
            self.assertTrue(worker.is_alive())

            outcome = finish_wire(peer, worker, outcomes)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
            self.assertEqual(provider_factory.calls, 0)
        finally:
            # This provider endpoint must remain unused by the idle-session
            # proof, so the attachment never takes ownership of it.
            raw.close()
            provider_peer.close()
            if worker.is_alive():
                worker.join(timeout=1.0)

    def test_first_credit_starts_one_pending_connect_without_blocking_wire_eof(
        self,
    ) -> None:
        raw, provider_peer = socket.socketpair()
        fill_send_buffer(raw)
        controlled = ControlledConnectSocket(
            raw,
            connect_result=errno.EINPROGRESS,
            socket_error=errno.EINPROGRESS,
        )
        provider_factory = ProviderSocketFactory([controlled])
        item = attachment.RfbAttachment(
            flow(),
            socket_factory=provider_factory,
        )
        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(110),
            rfb_attachment_factory=lambda: item,
        )
        peer, worker, outcomes = start_active_session(wire_server)
        try:
            self.assertEqual(provider_factory.calls, 0)

            peer.sendall(protocol.encode_rfb_credit_frame(3, sequence=2))
            wait_for(
                lambda: item.state
                is attachment.RfbAttachmentState.CONNECTING,
                "pending provider connect",
            )
            self.assertEqual(provider_factory.calls, 1)
            self.assertEqual(
                controlled.connect_addresses,
                [
                    (
                        attachment.INTERNAL_PROVIDER_HOST,
                        attachment.INTERNAL_PROVIDER_PORT,
                    )
                ],
            )

            peer.sendall(protocol.encode_rfb_credit_frame(2, sequence=3))
            time.sleep(0.02)
            self.assertEqual(provider_factory.calls, 1)

            peer.settimeout(0.05)
            with self.assertRaises(TimeoutError):
                peer.recv(1)
            peer.settimeout(None)

            outcome = finish_wire(peer, worker, outcomes)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
            self.assertEqual(outcome.next_receive_sequence, 4)
        finally:
            provider_peer.close()
            if worker.is_alive():
                worker.join(timeout=1.0)

    def test_connect_success_composes_one_relay_and_preserves_r10_flow(self) -> None:
        raw, provider_peer = socket.socketpair()
        controlled = ControlledConnectSocket(raw, connect_result=0)
        provider_factory = ProviderSocketFactory([controlled])
        item = attachment.RfbAttachment(
            flow(),
            socket_factory=provider_factory,
        )
        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(120),
            rfb_attachment_factory=lambda: item,
        )
        peer, worker, outcomes = start_active_session(wire_server)
        try:
            peer.sendall(protocol.encode_rfb_credit_frame(4, sequence=2))

            credit_header, credit_payload = recv_frame(peer)
            self.assertTrue(protocol.is_rfb_credit_header(credit_header))
            self.assertEqual(credit_header.sequence, 2)
            self.assertEqual(
                protocol.decode_rfb_credit_payload(credit_payload),
                8,
            )

            self.assertEqual(provider_factory.calls, 1)
            self.assertEqual(item.stats.connect_attempts, 1)
            self.assertEqual(item.stats.connect_successes, 1)
            self.assertIsNotNone(item.relay)
            self.assertEqual(item.relay.provider_read_credit, 4)

            provider_peer.sendall(b"ABCD")
            data_header, data_payload = recv_frame(peer)
            self.assertTrue(protocol.is_rfb_data_header(data_header))
            self.assertEqual(data_header.sequence, 3)
            self.assertEqual(data_payload, b"ABCD")

            peer.sendall(
                protocol.encode_rfb_data_frame(b"xy", sequence=3)
            )
            self.assertEqual(recv_exact(provider_peer, 2), b"xy")

            returned_header, returned_payload = recv_frame(peer)
            self.assertTrue(protocol.is_rfb_credit_header(returned_header))
            self.assertEqual(returned_header.sequence, 4)
            self.assertEqual(
                protocol.decode_rfb_credit_payload(returned_payload),
                2,
            )

            outcome = finish_wire(peer, worker, outcomes)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
            self.assertEqual(provider_factory.calls, 1)
        finally:
            provider_peer.close()
            if worker.is_alive():
                worker.join(timeout=1.0)

    def test_connect_failure_and_provider_eof_are_rfb_local(self) -> None:
        refused_raw, refused_peer = socket.socketpair()
        refused = ControlledConnectSocket(
            refused_raw,
            connect_result=errno.ECONNREFUSED,
        )
        refused_factory = ProviderSocketFactory([refused])
        failed_attachment = attachment.RfbAttachment(
            flow(),
            socket_factory=refused_factory,
        )
        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(130),
            rfb_attachment_factory=lambda: failed_attachment,
        )
        peer, worker, outcomes = start_active_session(wire_server)
        try:
            peer.sendall(protocol.encode_rfb_credit_frame(4, sequence=2))
            wait_for(
                lambda: failed_attachment.state
                is attachment.RfbAttachmentState.FAILED,
                "local connect failure",
            )
            self.assertTrue(worker.is_alive())
            self.assertEqual(failed_attachment.stats.connect_failures, 1)

            outcome = finish_wire(peer, worker, outcomes)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
        finally:
            refused_peer.close()
            if worker.is_alive():
                worker.join(timeout=1.0)

        raw, provider_peer = socket.socketpair()
        controlled = ControlledConnectSocket(raw, connect_result=0)
        provider_factory = ProviderSocketFactory([controlled])
        eof_attachment = attachment.RfbAttachment(
            flow(),
            socket_factory=provider_factory,
        )
        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(131),
            rfb_attachment_factory=lambda: eof_attachment,
        )
        peer, worker, outcomes = start_active_session(wire_server)
        try:
            peer.sendall(protocol.encode_rfb_credit_frame(4, sequence=2))
            initial_header, _initial_payload = recv_frame(peer)
            self.assertTrue(protocol.is_rfb_credit_header(initial_header))

            provider_peer.close()
            wait_for(
                lambda: eof_attachment.state
                is attachment.RfbAttachmentState.FAILED,
                "provider EOF local failure",
            )
            self.assertTrue(worker.is_alive())

            peer.sendall(protocol.encode_rfb_credit_frame(1, sequence=3))
            time.sleep(0.01)
            self.assertTrue(worker.is_alive())

            outcome = finish_wire(peer, worker, outcomes)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
        finally:
            try:
                provider_peer.close()
            except OSError:
                pass
            if worker.is_alive():
                worker.join(timeout=1.0)

    def test_request_boundary_drains_and_retires_before_commit_complete(
        self,
    ) -> None:
        raw, provider_peer = socket.socketpair()
        prefill_bytes = fill_send_buffer(raw)
        controlled = ControlledConnectSocket(raw, connect_result=0)
        provider_factory = ProviderSocketFactory([controlled])
        item = attachment.RfbAttachment(
            flow(),
            socket_factory=provider_factory,
        )
        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(140),
            rfb_attachment_factory=lambda: item,
        )
        peer, worker, outcomes = start_active_session(wire_server)
        try:
            peer.sendall(protocol.encode_rfb_credit_frame(4, sequence=2))
            initial_header, initial_payload = recv_frame(peer)
            self.assertTrue(protocol.is_rfb_credit_header(initial_header))
            self.assertEqual(
                protocol.decode_rfb_credit_payload(initial_payload),
                8,
            )

            peer.sendall(
                protocol.encode_rfb_data_frame(b"abcd", sequence=3)
            )
            wait_for(
                lambda: item.relay is not None
                and item.relay.queued_provider_write_bytes == 4,
                "pre-BOUNDARY provider write queue",
            )

            provider_peer.sendall(b"Q")
            provider_header, provider_payload = recv_frame(peer)
            self.assertTrue(protocol.is_rfb_data_header(provider_header))
            self.assertEqual(provider_payload, b"Q")
            self.assertEqual(provider_header.sequence, 3)

            self.assertTrue(item.request_quiesce())
            peer.sendall(protocol.encode_rfb_credit_frame(1, sequence=4))
            request_header, request_payload = recv_frame(peer)
            self.assertTrue(protocol.is_rfb_data_header(request_header))
            self.assertEqual(request_header.sequence, 4)
            self.assertEqual(request_payload, b"")
            self.assertEqual(
                item.state,
                attachment.RfbAttachmentState.WAIT_BOUNDARY,
            )

            peer.sendall(protocol.encode_rfb_data_frame(b"", sequence=5))
            wait_for(
                lambda: item.state
                is attachment.RfbAttachmentState.DRAINING,
                "BOUNDARY drain state",
            )
            self.assertEqual(item.stats.boundary_markers_received, 1)

            reads_at_boundary = item.relay.stats.provider_read_calls
            provider_peer.sendall(b"R")
            time.sleep(0.02)
            self.assertEqual(
                item.relay.stats.provider_read_calls,
                reads_at_boundary,
            )

            peer.settimeout(0.05)
            with self.assertRaises(TimeoutError):
                peer.recv(1)
            peer.settimeout(None)

            provider_peer.settimeout(1.0)
            drained = bytearray()
            while b"abcd" not in drained:
                drained.extend(provider_peer.recv(65536))
            self.assertGreaterEqual(len(drained), prefill_bytes + 4)

            commit_header, commit_payload = recv_frame(peer)
            self.assertTrue(protocol.is_rfb_data_header(commit_header))
            self.assertEqual(commit_header.sequence, 5)
            self.assertEqual(commit_payload, b"")
            self.assertTrue(controlled.closed)
            self.assertTrue(item.relay.closed)
            self.assertEqual(item.stats.provider_retirements, 1)
            self.assertEqual(
                item.state,
                attachment.RfbAttachmentState.WAIT_COMPLETE,
            )

            peer.settimeout(0.05)
            with self.assertRaises(TimeoutError):
                peer.recv(1)
            peer.settimeout(None)

            peer.sendall(protocol.encode_rfb_data_frame(b"", sequence=6))
            wait_for(
                lambda: item.state
                is attachment.RfbAttachmentState.STOPPED,
                "COMPLETE stopped state",
            )
            self.assertEqual(item.stats.request_markers_sent, 1)
            self.assertEqual(item.stats.commit_markers_sent, 1)
            self.assertEqual(item.stats.complete_markers_received, 1)
            self.assertTrue(worker.is_alive())

            outcome = finish_wire(peer, worker, outcomes)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
        finally:
            try:
                provider_peer.close()
            except OSError:
                pass
            if worker.is_alive():
                worker.join(timeout=1.0)

    def test_out_of_order_marker_and_provider_write_failure_stay_rfb_local(
        self,
    ) -> None:
        raw, provider_peer = socket.socketpair()
        controlled = ControlledConnectSocket(raw, connect_result=0)
        item = attachment.RfbAttachment(
            flow(),
            socket_factory=ProviderSocketFactory([controlled]),
        )
        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(150),
            rfb_attachment_factory=lambda: item,
        )
        peer, worker, outcomes = start_active_session(wire_server)
        try:
            peer.sendall(protocol.encode_rfb_credit_frame(4, sequence=2))
            recv_frame(peer)

            peer.sendall(protocol.encode_rfb_data_frame(b"", sequence=3))
            wait_for(
                lambda: item.state
                is attachment.RfbAttachmentState.FAILED,
                "out-of-order marker local failure",
            )
            self.assertTrue(worker.is_alive())

            outcome = finish_wire(peer, worker, outcomes)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
        finally:
            provider_peer.close()
            if worker.is_alive():
                worker.join(timeout=1.0)

        raw, provider_peer = socket.socketpair()
        fill_send_buffer(raw)
        controlled = ControlledConnectSocket(raw, connect_result=0)
        item = attachment.RfbAttachment(
            flow(),
            socket_factory=ProviderSocketFactory([controlled]),
        )
        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(151),
            rfb_attachment_factory=lambda: item,
        )
        peer, worker, outcomes = start_active_session(wire_server)
        try:
            peer.sendall(protocol.encode_rfb_credit_frame(4, sequence=2))
            recv_frame(peer)
            peer.sendall(
                protocol.encode_rfb_data_frame(b"abcd", sequence=3)
            )
            wait_for(
                lambda: item.relay is not None
                and item.relay.queued_provider_write_bytes == 4,
                "queued provider write before failure",
            )

            self.assertTrue(item.request_quiesce())
            peer.sendall(protocol.encode_rfb_credit_frame(1, sequence=4))
            request_header, request_payload = recv_frame(peer)
            self.assertEqual(request_payload, b"")
            self.assertTrue(protocol.is_rfb_data_header(request_header))

            peer.sendall(protocol.encode_rfb_data_frame(b"", sequence=5))
            wait_for(
                lambda: item.state
                is attachment.RfbAttachmentState.DRAINING,
                "write-failure drain state",
            )

            controlled.fail_send = True
            provider_peer.settimeout(1.0)
            while item.state is attachment.RfbAttachmentState.DRAINING:
                try:
                    provider_peer.recv(65536)
                except TimeoutError:
                    break
                time.sleep(0.001)

            wait_for(
                lambda: item.state
                is attachment.RfbAttachmentState.FAILED,
                "provider write local failure",
            )
            self.assertTrue(worker.is_alive())
            self.assertEqual(item.stats.commit_markers_sent, 0)

            peer.settimeout(0.05)
            with self.assertRaises(TimeoutError):
                peer.recv(1)
            peer.settimeout(None)

            outcome = finish_wire(peer, worker, outcomes)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
        finally:
            try:
                provider_peer.close()
            except OSError:
                pass
            if worker.is_alive():
                worker.join(timeout=1.0)

    def test_session_b_constructs_fresh_attachment_state(self) -> None:
        a_raw, a_provider_peer = socket.socketpair()
        b_raw, b_provider_peer = socket.socketpair()
        provider_factory = ProviderSocketFactory(
            [
                ControlledConnectSocket(a_raw, connect_result=0),
                ControlledConnectSocket(b_raw, connect_result=0),
            ]
        )
        attachments: list[attachment.RfbAttachment] = []

        def make_attachment() -> attachment.RfbAttachment:
            item = attachment.RfbAttachment(
                flow(),
                socket_factory=provider_factory,
            )
            attachments.append(item)
            return item

        wire_server = server.WireServer(
            session_ids=server.SessionIdAllocator(160),
            rfb_attachment_factory=make_attachment,
        )

        peer_a, worker_a, outcomes_a = start_active_session(wire_server)
        peer_a.sendall(protocol.encode_rfb_credit_frame(4, sequence=2))
        recv_frame(peer_a)
        outcome_a = finish_wire(peer_a, worker_a, outcomes_a)
        self.assertTrue(outcome_a.accepted)
        self.assertFalse(outcome_a.protocol_failed)
        self.assertEqual(provider_factory.calls, 1)
        self.assertEqual(
            attachments[0].state,
            attachment.RfbAttachmentState.STOPPED,
        )

        peer_b, worker_b, outcomes_b = start_active_session(wire_server)
        try:
            self.assertEqual(len(attachments), 2)
            self.assertIsNot(attachments[0], attachments[1])
            self.assertEqual(
                attachments[1].state,
                attachment.RfbAttachmentState.IDLE,
            )
            self.assertIsNone(attachments[1].relay)
            self.assertEqual(provider_factory.calls, 1)

            peer_b.sendall(protocol.encode_rfb_credit_frame(4, sequence=2))
            recv_frame(peer_b)
            self.assertEqual(provider_factory.calls, 2)
            self.assertIsNotNone(attachments[1].relay)
            self.assertIsNot(
                attachments[0].relay,
                attachments[1].relay,
            )

            outcome_b = finish_wire(peer_b, worker_b, outcomes_b)
            self.assertTrue(outcome_b.accepted)
            self.assertFalse(outcome_b.protocol_failed)
        finally:
            a_provider_peer.close()
            b_provider_peer.close()
            if worker_b.is_alive():
                worker_b.join(timeout=1.0)


class RfbAttachmentRepositoryBoundaryTests(unittest.TestCase):
    def test_selected_endpoint_and_default_service_boundary(self) -> None:
        source = (
            REPO_ROOT / "pi/rfb_attachment.py"
        ).read_text(encoding="utf-8")
        unit = (
            REPO_ROOT / "systemd/pi/ps-to-vnc-wire.service"
        ).read_text(encoding="utf-8")
        wire_server = (
            REPO_ROOT / "pi/wire_server.py"
        ).read_text(encoding="utf-8")

        self.assertIn('INTERNAL_PROVIDER_HOST = "127.0.0.1"', source)
        self.assertIn("INTERNAL_PROVIDER_PORT = 5900", source)
        self.assertNotIn("5903", source)

        self.assertIn(
            "self.serve_connection(connection)",
            wire_server,
        )
        self.assertNotIn("--rfb", wire_server)
        self.assertNotIn("127.0.0.1:5900", unit)
        self.assertNotIn("5903", unit)
        self.assertNotIn("provider_read_credit_limit", unit)
        self.assertNotIn("provider_write_capacity", unit)
        self.assertNotIn("max_data_payload", unit)

    def test_wire_runtime_stager_tracks_attachment_without_live_mutation(
        self,
    ) -> None:
        installer = (
            REPO_ROOT / "scripts/pi/install-wire-runtime.sh"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "/usr/lib/ps-to-vnc/rfb_attachment.py",
            installer,
        )
        self.assertIn(
            'verify_file 0644 "$RFB_ATTACHMENT_SOURCE" '
            '"$RFB_ATTACHMENT_DEST"',
            installer,
        )
        for forbidden in (
            "systemctl daemon-reload",
            "systemctl enable ",
            "systemctl disable ",
            "systemctl start ",
            "systemctl stop ",
            "systemctl restart ",
        ):
            self.assertNotIn(forbidden, installer)


if __name__ == "__main__":
    unittest.main(verbosity=2)
