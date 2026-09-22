#!/usr/bin/env python3
"""Run the R13 attachment suite with R15's scheduling-stable wake assertion."""

from __future__ import annotations

import threading
import unittest

import pi_rfb_attachment_test_legacy as legacy


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


legacy.RfbAttachmentIntegrationTests.test_quiesce_request_wakes_idle_wire_without_peer_or_provider_traffic = (
    test_quiesce_request_wakes_idle_wire_without_peer_or_provider_traffic
)


if __name__ == "__main__":
    unittest.main(module=legacy, verbosity=2)
