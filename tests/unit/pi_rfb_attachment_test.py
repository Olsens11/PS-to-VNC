#!/usr/bin/env python3
"""Run the R13 attachment suite with R15 wake and R16A failure assertions."""

from __future__ import annotations

import errno
import threading
import time
import unittest

import pi_rfb_attachment_test_legacy as legacy


def assert_provider_failure_frame(
    self,
    peer,
    expected_reason: int,
) -> None:
    header, payload = legacy.recv_frame(peer)
    self.assertTrue(legacy.protocol.is_rfb_provider_failure_header(header))
    self.assertFalse(legacy.protocol.is_rfb_data_header(header))
    self.assertFalse(legacy.protocol.is_rfb_credit_header(header))
    self.assertEqual(
        legacy.protocol.decode_rfb_provider_failure_payload(payload),
        expected_reason,
    )


def wait_for_provider_failure_report_confirmation(item, label: str) -> None:
    # sendall() completion and the immediately-following attachment-local report
    # confirmation can land on adjacent host scheduling slices. The peer already
    # receiving ERROR proves serialization; wait only for that local state edge.
    legacy.wait_for(
        lambda: not item.wants_provider_failure_report,
        f"{label} report confirmation",
    )


def test_connect_failure_and_provider_eof_report_typed_fact_without_wire_failure(
    self,
) -> None:
    refused_raw, refused_peer = legacy.socket.socketpair()
    refused = legacy.ControlledConnectSocket(
        refused_raw,
        connect_result=errno.ECONNREFUSED,
    )
    failed_attachment = legacy.attachment.RfbAttachment(
        legacy.flow(),
        socket_factory=legacy.ProviderSocketFactory([refused]),
    )
    wire_server = legacy.server.WireServer(
        session_ids=legacy.server.SessionIdAllocator(130),
        rfb_attachment_factory=lambda: failed_attachment,
    )
    peer, worker, outcomes = legacy.start_active_session(wire_server)
    try:
        peer.sendall(legacy.protocol.encode_rfb_credit_frame(4, sequence=2))
        assert_provider_failure_frame(
            self,
            peer,
            legacy.protocol.RFB_PROVIDER_FAILURE_CONNECT,
        )
        legacy.wait_for(
            lambda: failed_attachment.state
            is legacy.attachment.RfbAttachmentState.FAILED,
            "reported local connect failure",
        )
        self.assertEqual(
            failed_attachment.provider_failure,
            legacy.attachment.RfbProviderFailure.CONNECT,
        )
        wait_for_provider_failure_report_confirmation(
            failed_attachment,
            "connect failure",
        )
        self.assertEqual(failed_attachment.stats.connect_failures, 1)
        self.assertTrue(worker.is_alive())

        outcome = legacy.finish_wire(peer, worker, outcomes)
        self.assertTrue(outcome.accepted)
        self.assertFalse(outcome.protocol_failed)
    finally:
        refused_peer.close()
        if worker.is_alive():
            worker.join(timeout=1.0)

    raw, provider_peer = legacy.socket.socketpair()
    controlled = legacy.ControlledConnectSocket(raw, connect_result=0)
    eof_attachment = legacy.attachment.RfbAttachment(
        legacy.flow(),
        socket_factory=legacy.ProviderSocketFactory([controlled]),
    )
    wire_server = legacy.server.WireServer(
        session_ids=legacy.server.SessionIdAllocator(131),
        rfb_attachment_factory=lambda: eof_attachment,
    )
    peer, worker, outcomes = legacy.start_active_session(wire_server)
    try:
        peer.sendall(legacy.protocol.encode_rfb_credit_frame(4, sequence=2))
        initial_header, _initial_payload = legacy.recv_frame(peer)
        self.assertTrue(legacy.protocol.is_rfb_credit_header(initial_header))

        provider_peer.close()
        assert_provider_failure_frame(
            self,
            peer,
            legacy.protocol.RFB_PROVIDER_FAILURE_READ,
        )
        legacy.wait_for(
            lambda: eof_attachment.state
            is legacy.attachment.RfbAttachmentState.FAILED,
            "reported provider EOF failure",
        )
        self.assertEqual(
            eof_attachment.provider_failure,
            legacy.attachment.RfbProviderFailure.READ,
        )
        wait_for_provider_failure_report_confirmation(
            eof_attachment,
            "provider EOF",
        )
        self.assertTrue(worker.is_alive())

        # Late old-session channel-1 authority is contained by the dead
        # attachment. It neither causes a replacement connect nor emits a
        # second terminal report.
        peer.sendall(legacy.protocol.encode_rfb_credit_frame(1, sequence=3))
        peer.settimeout(0.05)
        with self.assertRaises(TimeoutError):
            peer.recv(1)
        peer.settimeout(None)
        self.assertTrue(worker.is_alive())

        outcome = legacy.finish_wire(peer, worker, outcomes)
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
    # A local lifecycle misuse is not a provider mechanism failure and therefore
    # still produces no provider-terminal ERROR representation.
    raw, provider_peer = legacy.socket.socketpair()
    controlled = legacy.ControlledConnectSocket(raw, connect_result=0)
    item = legacy.attachment.RfbAttachment(
        legacy.flow(),
        socket_factory=legacy.ProviderSocketFactory([controlled]),
    )
    wire_server = legacy.server.WireServer(
        session_ids=legacy.server.SessionIdAllocator(150),
        rfb_attachment_factory=lambda: item,
    )
    peer, worker, outcomes = legacy.start_active_session(wire_server)
    try:
        peer.sendall(legacy.protocol.encode_rfb_credit_frame(4, sequence=2))
        legacy.recv_frame(peer)

        peer.sendall(legacy.protocol.encode_rfb_data_frame(b"", sequence=3))
        legacy.wait_for(
            lambda: item.state is legacy.attachment.RfbAttachmentState.FAILED,
            "out-of-order marker local failure",
        )
        self.assertIsNone(item.provider_failure)
        self.assertFalse(item.wants_provider_failure_report)
        self.assertTrue(worker.is_alive())

        peer.settimeout(0.05)
        with self.assertRaises(TimeoutError):
            peer.recv(1)
        peer.settimeout(None)

        outcome = legacy.finish_wire(peer, worker, outcomes)
        self.assertTrue(outcome.accepted)
        self.assertFalse(outcome.protocol_failed)
    finally:
        provider_peer.close()
        if worker.is_alive():
            worker.join(timeout=1.0)

    # A genuine provider write failure is distinguishable and crosses through
    # the sole Wire owner as WRITE while the physical Wire session stays live.
    raw, provider_peer = legacy.socket.socketpair()
    legacy.fill_send_buffer(raw)
    controlled = legacy.ControlledConnectSocket(raw, connect_result=0)
    item = legacy.attachment.RfbAttachment(
        legacy.flow(),
        socket_factory=legacy.ProviderSocketFactory([controlled]),
    )
    wire_server = legacy.server.WireServer(
        session_ids=legacy.server.SessionIdAllocator(151),
        rfb_attachment_factory=lambda: item,
    )
    peer, worker, outcomes = legacy.start_active_session(wire_server)
    try:
        peer.sendall(legacy.protocol.encode_rfb_credit_frame(4, sequence=2))
        legacy.recv_frame(peer)
        peer.sendall(legacy.protocol.encode_rfb_data_frame(b"abcd", sequence=3))
        legacy.wait_for(
            lambda: item.relay is not None
            and item.relay.queued_provider_write_bytes == 4,
            "queued provider write before failure",
        )

        self.assertTrue(item.request_quiesce())
        request_header, request_payload = legacy.recv_frame(peer)
        self.assertEqual(request_payload, b"")
        self.assertTrue(legacy.protocol.is_rfb_data_header(request_header))

        peer.sendall(legacy.protocol.encode_rfb_data_frame(b"", sequence=4))
        legacy.wait_for(
            lambda: item.state is legacy.attachment.RfbAttachmentState.DRAINING,
            "write-failure drain state",
        )

        controlled.fail_send = True
        provider_peer.settimeout(1.0)
        while item.state is legacy.attachment.RfbAttachmentState.DRAINING:
            try:
                provider_peer.recv(65536)
            except TimeoutError:
                break
            time.sleep(0.001)

        assert_provider_failure_frame(
            self,
            peer,
            legacy.protocol.RFB_PROVIDER_FAILURE_WRITE,
        )
        legacy.wait_for(
            lambda: item.state is legacy.attachment.RfbAttachmentState.FAILED,
            "reported provider write failure",
        )
        self.assertEqual(
            item.provider_failure,
            legacy.attachment.RfbProviderFailure.WRITE,
        )
        wait_for_provider_failure_report_confirmation(
            item,
            "provider write",
        )
        self.assertEqual(item.stats.commit_markers_sent, 0)
        self.assertTrue(worker.is_alive())

        peer.settimeout(0.05)
        with self.assertRaises(TimeoutError):
            peer.recv(1)
        peer.settimeout(None)

        outcome = legacy.finish_wire(peer, worker, outcomes)
        self.assertTrue(outcome.accepted)
        self.assertFalse(outcome.protocol_failed)
    finally:
        try:
            provider_peer.close()
        except OSError:
            pass
        if worker.is_alive():
            worker.join(timeout=1.0)


def test_quiesce_request_wakes_idle_wire_without_peer_or_provider_traffic(
    self,
) -> None:
    raw, provider_peer = legacy.socket.socketpair()
    controlled = legacy.ControlledConnectSocket(raw, connect_result=0)
    item = legacy.attachment.RfbAttachment(
        legacy.flow(),
        socket_factory=legacy.ProviderSocketFactory([controlled]),
    )
    wire_server = legacy.server.WireServer(
        session_ids=legacy.server.SessionIdAllocator(139),
        rfb_attachment_factory=lambda: item,
    )
    peer, worker, outcomes = legacy.start_active_session(wire_server)
    try:
        peer.sendall(legacy.protocol.encode_rfb_credit_frame(4, sequence=2))
        initial_header, _initial_payload = legacy.recv_frame(peer)
        self.assertTrue(legacy.protocol.is_rfb_credit_header(initial_header))
        self.assertEqual(
            item.state,
            legacy.attachment.RfbAttachmentState.RUNNING,
        )

        request_results: list[bool] = []
        requester = threading.Thread(
            target=lambda: request_results.append(item.request_quiesce())
        )
        requester.start()
        requester.join(timeout=1.0)
        self.assertFalse(requester.is_alive())
        self.assertEqual(request_results, [True])

        # No peer CREDIT, provider bytes, endpoint close, or timeout poll wakes
        # the Wire owner. Receiving REQUEST proves the local wake worked. The
        # sender's immediately-following local state confirmation may run on the
        # next host scheduling slice, so wait for that state instead of racing
        # the sending thread.
        peer.settimeout(1.0)
        request_header, request_payload = legacy.recv_frame(peer)
        self.assertTrue(legacy.protocol.is_rfb_data_header(request_header))
        self.assertEqual(request_header.sequence, 3)
        self.assertEqual(request_payload, b"")
        legacy.wait_for(
            lambda: item.state
            is legacy.attachment.RfbAttachmentState.WAIT_BOUNDARY,
            "REQUEST sender local WAIT_BOUNDARY transition",
        )
        self.assertEqual(item.stats.request_markers_sent, 1)
        self.assertFalse(controlled.closed)

        peer.settimeout(0.05)
        with self.assertRaises(TimeoutError):
            peer.recv(1)
        peer.settimeout(None)

        provider_peer.settimeout(0.05)
        with self.assertRaises(TimeoutError):
            provider_peer.recv(1)
        provider_peer.settimeout(None)

        peer.sendall(legacy.protocol.encode_rfb_data_frame(b"", sequence=3))
        commit_header, commit_payload = legacy.recv_frame(peer)
        self.assertTrue(legacy.protocol.is_rfb_data_header(commit_header))
        self.assertEqual(commit_header.sequence, 4)
        self.assertEqual(commit_payload, b"")
        self.assertTrue(controlled.closed)
        self.assertTrue(item.relay.closed)
        legacy.wait_for(
            lambda: item.state
            is legacy.attachment.RfbAttachmentState.WAIT_COMPLETE,
            "COMMIT sender local WAIT_COMPLETE transition",
        )

        peer.sendall(legacy.protocol.encode_rfb_data_frame(b"", sequence=4))
        legacy.wait_for(
            lambda: item.state
            is legacy.attachment.RfbAttachmentState.STOPPED,
            "idle wake COMPLETE stopped state",
        )
        self.assertIsNone(item.quiesce_wake_reader)
        self.assertTrue(worker.is_alive())

        outcome = legacy.finish_wire(peer, worker, outcomes)
        self.assertTrue(outcome.accepted)
        self.assertFalse(outcome.protocol_failed)
    finally:
        try:
            provider_peer.close()
        except OSError:
            pass
        if worker.is_alive():
            worker.join(timeout=1.0)


def test_replacement_wire_session_allocates_fresh_attachment(self) -> None:
    attachments: list[legacy.attachment.RfbAttachment] = []
    provider_factory = legacy.ProviderSocketFactory([])

    def make_attachment() -> legacy.attachment.RfbAttachment:
        item = legacy.attachment.RfbAttachment(
            legacy.flow(),
            socket_factory=provider_factory,
        )
        attachments.append(item)
        return item

    wire_server = legacy.server.WireServer(
        session_ids=legacy.server.SessionIdAllocator(170),
        rfb_attachment_factory=make_attachment,
    )

    peer_a, worker_a, outcomes_a = legacy.start_active_session(wire_server)
    try:
        self.assertEqual(len(attachments), 1)
        outcome_a = legacy.finish_wire(peer_a, worker_a, outcomes_a)
        self.assertTrue(outcome_a.accepted)
        self.assertEqual(outcome_a.session_id, 170)
    finally:
        if worker_a.is_alive():
            worker_a.join(timeout=1.0)

    peer_b, worker_b, outcomes_b = legacy.start_active_session(wire_server)
    try:
        self.assertEqual(len(attachments), 2)
        self.assertIsNot(attachments[0], attachments[1])
        outcome_b = legacy.finish_wire(peer_b, worker_b, outcomes_b)
        self.assertTrue(outcome_b.accepted)
        self.assertEqual(outcome_b.session_id, 171)
    finally:
        if worker_b.is_alive():
            worker_b.join(timeout=1.0)

    self.assertEqual(provider_factory.calls, 0)


legacy.RfbAttachmentIntegrationTests.test_connect_failure_and_provider_eof_are_rfb_local = (
    test_connect_failure_and_provider_eof_report_typed_fact_without_wire_failure
)
legacy.RfbAttachmentIntegrationTests.test_out_of_order_marker_and_provider_write_failure_stay_rfb_local = (
    test_out_of_order_marker_and_provider_write_failure_stay_rfb_local
)
legacy.RfbAttachmentIntegrationTests.test_quiesce_request_wakes_idle_wire_without_peer_or_provider_traffic = (
    test_quiesce_request_wakes_idle_wire_without_peer_or_provider_traffic
)
legacy.RfbAttachmentIntegrationTests.test_replacement_wire_session_allocates_fresh_attachment = (
    test_replacement_wire_session_allocates_fresh_attachment
)


if __name__ == "__main__":
    unittest.main(module=legacy, verbosity=2)
