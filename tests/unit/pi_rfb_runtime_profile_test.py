#!/usr/bin/env python3
"""Run R14 profile authority with R15 freshness/composition assertions."""

from __future__ import annotations

import threading
import unittest

import pi_rfb_runtime_profile_test_legacy as legacy


def test_r15_default_runtime_activation_uses_composition(self) -> None:
    service = (
        legacy.ROOT / "systemd/pi/ps-to-vnc-wire.service"
    ).read_text(encoding="utf-8")
    runtime = (legacy.ROOT / "pi/wire_runtime.py").read_text(encoding="utf-8")
    wire_server = (legacy.ROOT / "pi/wire_server.py").read_text(encoding="utf-8")
    app = (legacy.ROOT / "src/app.c").read_text(encoding="utf-8")

    self.assertIn("/usr/lib/ps-to-vnc/wire_runtime.py", service)
    self.assertIn("selected_rfb_flow_config()", runtime)
    self.assertNotIn("rfb_runtime_profile", wire_server)
    self.assertIn("pstvnc_config_rfb_runtime_profile_selected", app)
    self.assertNotIn("return pstvnc_app_run_with_transport_config(NULL);", app)
    for literal in ("32768", "8192", "16384", "5903"):
        self.assertNotIn(literal, service)
        self.assertNotIn(literal, runtime)


def test_selected_default_is_lazy_and_fresh_for_each_wire_session(self) -> None:
    endpoints: list[tuple[str, int]] = []
    provider_peers: list[legacy.socket.socket] = []
    controlled_sockets: list[legacy.ControlledProviderSocket] = []
    attachments: list[legacy.rfb_attachment.RfbAttachment] = []
    wake_readers: list[legacy.socket.socket] = []
    original_make = legacy.wire_runtime._make_attachment

    def make_selected_attachment(
        flow: legacy.rfb_attachment.RfbFlowConfig,
    ) -> legacy.rfb_attachment.RfbAttachment:
        self.assertEqual(flow.provider_read_credit_limit, 32768)
        self.assertEqual(flow.provider_write_capacity, 32768)
        self.assertEqual(flow.max_data_payload, 8192)
        raw, provider_peer = legacy.socket.socketpair()
        controlled = legacy.ControlledProviderSocket(raw, endpoints)
        provider_peers.append(provider_peer)
        controlled_sockets.append(controlled)
        item = legacy.rfb_attachment.RfbAttachment(
            flow,
            socket_factory=lambda controlled=controlled: controlled,
        )
        attachments.append(item)
        return item

    legacy.wire_runtime._make_attachment = make_selected_attachment
    try:
        product_server = legacy.wire_runtime.build_product_wire_server()
        self.assertIsNotNone(product_server.rfb_attachment_factory)

        for session_index in range(2):
            server_peer, product_peer = legacy.socket.socketpair()
            outcomes = []
            worker = threading.Thread(
                target=lambda peer=server_peer: outcomes.append(
                    product_server.serve_connection(peer)
                )
            )
            worker.start()
            try:
                product_peer.sendall(
                    legacy.wire_protocol.encode_hello_frame(sequence=1)
                )
                accept_header, accept_payload = legacy.recv_frame(product_peer)
                self.assertTrue(
                    legacy.wire_protocol.is_accept_header(accept_header)
                )
                self.assertEqual(accept_header.sequence, 1)
                self.assertEqual(len(accept_payload), 4)

                self.assertEqual(len(attachments), session_index + 1)
                item = attachments[session_index]
                self.assertIs(
                    item.state,
                    legacy.rfb_attachment.RfbAttachmentState.IDLE,
                )
                self.assertEqual(len(endpoints), session_index)
                self.assertIsNone(item.connecting_socket)
                self.assertIsNone(item.provider_socket)

                wake_reader = item.quiesce_wake_reader
                self.assertIsNotNone(wake_reader)
                assert wake_reader is not None
                wake_readers.append(wake_reader)

                product_peer.sendall(
                    legacy.wire_protocol.encode_rfb_credit_frame(
                        32768,
                        sequence=2,
                    )
                )
                credit_header, credit_payload = legacy.recv_frame(product_peer)
                self.assertTrue(
                    legacy.wire_protocol.is_rfb_credit_header(credit_header)
                )
                self.assertEqual(credit_header.sequence, 2)
                self.assertEqual(
                    legacy.wire_protocol.decode_rfb_credit_payload(
                        credit_payload
                    ),
                    32768,
                )
                self.assertEqual(
                    endpoints[-1],
                    (
                        legacy.rfb_attachment.INTERNAL_PROVIDER_HOST,
                        legacy.rfb_attachment.INTERNAL_PROVIDER_PORT,
                    ),
                )
                self.assertEqual(endpoints[-1], ("127.0.0.1", 5900))
                self.assertEqual(item.stats.connect_attempts, 1)
                self.assertEqual(item.stats.connect_successes, 1)
            finally:
                product_peer.close()
                worker.join(timeout=1.0)
                self.assertFalse(worker.is_alive())
                self.assertEqual(len(outcomes), 1)

        self.assertIsNot(attachments[0], attachments[1])
        self.assertIsNot(wake_readers[0], wake_readers[1])
        self.assertIsNone(attachments[0].quiesce_wake_reader)
        self.assertIsNone(attachments[1].quiesce_wake_reader)
        self.assertTrue(controlled_sockets[0].closed)
        self.assertTrue(controlled_sockets[1].closed)
    finally:
        legacy.wire_runtime._make_attachment = original_make
        for provider_peer in provider_peers:
            try:
                provider_peer.close()
            except OSError:
                pass


legacy.CanonicalRfbRuntimeProfileTests.test_default_runtime_activation_remains_absent = (
    test_r15_default_runtime_activation_uses_composition
)
legacy.R15ProductCompositionTests.test_selected_default_is_lazy_and_fresh_for_each_wire_session = (
    test_selected_default_is_lazy_and_fresh_for_each_wire_session
)


if __name__ == "__main__":
    unittest.main(module=legacy, verbosity=2)
