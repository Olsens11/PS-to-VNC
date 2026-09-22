#!/usr/bin/env python3
"""Host contract for A003 R14 profile authority and R15 Pi composition."""

from __future__ import annotations

import copy
import json
from pathlib import Path
import shutil
import socket
import subprocess
import sys
import tempfile
import threading
import unittest

ROOT = Path(__file__).resolve().parents[2]
PI = ROOT / "pi"
if str(PI) not in sys.path:
    sys.path.insert(0, str(PI))

import rfb_attachment
import rfb_runtime_profile
import rfb_runtime_profile_generated as generated
import wire_protocol
import wire_runtime

CANONICAL = ROOT / "src/config/rfb_runtime_profile.json"
GENERATOR = ROOT / "scripts/generate-rfb-runtime-profile.py"


def read_exact(peer: socket.socket, byte_count: int) -> bytes:
    chunks: list[bytes] = []
    remaining = byte_count
    while remaining:
        chunk = peer.recv(remaining)
        if not chunk:
            raise EOFError("peer closed during test frame read")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)


def recv_frame(peer: socket.socket) -> tuple[wire_protocol.WireHeader, bytes]:
    header = wire_protocol.decode_header(
        read_exact(peer, wire_protocol.HEADER_BYTES)
    )
    return header, read_exact(peer, header.payload_length)


class ControlledProviderSocket:
    """Connected socketpair endpoint with an observable product connect edge."""

    def __init__(self, raw: socket.socket, endpoints: list[tuple[str, int]]) -> None:
        self._raw = raw
        self._endpoints = endpoints
        self.closed = False

    def setblocking(self, flag: bool) -> None:
        self._raw.setblocking(flag)

    def connect_ex(self, endpoint: tuple[str, int]) -> int:
        self._endpoints.append(endpoint)
        return 0

    def recv(self, byte_count: int) -> bytes:
        return self._raw.recv(byte_count)

    def send(self, payload: bytes) -> int:
        return self._raw.send(payload)

    def fileno(self) -> int:
        return self._raw.fileno()

    def close(self) -> None:
        self.closed = True
        self._raw.close()


class CanonicalRfbRuntimeProfileTests(unittest.TestCase):
    def test_canonical_selected_values_and_provenance(self) -> None:
        profile = json.loads(CANONICAL.read_text(encoding="utf-8"))
        self.assertEqual(profile["rfb_mode"], "ON")
        self.assertEqual(profile["rfb_window_bytes"], 32768)
        self.assertEqual(profile["rfb_credit_batch_bytes"], 8192)
        self.assertTrue(profile["rfb_credit_flush_on_empty"])
        self.assertTrue(profile["rfb_credit_return_enabled"])
        self.assertEqual(profile["receiver_thread_priority"], 63)
        self.assertEqual(profile["receiver_thread_stack_size"], 16384)
        self.assertEqual(profile["max_data_payload"], 8192)
        self.assertEqual(profile["provenance"]["baseline_checkpoint"], "CP2N")
        self.assertFalse(
            profile["provenance"]["pi_provider_write_capacity_hardware_qualified"]
        )
        self.assertNotIn("session_id", profile)
        self.assertNotIn("audio_mode", profile)
        self.assertNotIn("mpeg", profile)

    def test_pi_projection_uses_one_window_and_off_is_absent(self) -> None:
        flow = rfb_runtime_profile.selected_rfb_flow_config()
        self.assertIsInstance(flow, rfb_attachment.RfbFlowConfig)
        assert flow is not None
        self.assertEqual(flow.provider_read_credit_limit, 32768)
        self.assertEqual(flow.provider_write_capacity, 32768)
        self.assertEqual(flow.max_data_payload, 8192)
        self.assertEqual(
            flow.provider_write_capacity, flow.provider_read_credit_limit
        )
        self.assertIsNone(rfb_runtime_profile.project_rfb_flow_config("OFF"))
        with self.assertRaises(ValueError):
            rfb_runtime_profile.project_rfb_flow_config("VISIBLE")

    def test_generated_projection_is_current_and_has_no_session_identity(self) -> None:
        completed = subprocess.run(
            [sys.executable, str(GENERATOR), "--check"],
            cwd=ROOT, check=False, capture_output=True, text=True,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(generated.RFB_MODE, "ON")
        self.assertEqual(generated.RFB_WINDOW_BYTES, 32768)
        for path in (
            ROOT / "src/config/rfb_runtime_profile_generated.h",
            ROOT / "pi/rfb_runtime_profile_generated.py",
        ):
            self.assertNotIn(
                "session_id", path.read_text(encoding="utf-8").lower()
            )

    def test_canonical_change_makes_checked_in_projection_stale(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            temp_root = Path(tmp)
            for relative in (
                Path("src/config/rfb_runtime_profile.json"),
                Path("src/config/rfb_runtime_profile_generated.h"),
                Path("pi/rfb_runtime_profile_generated.py"),
            ):
                destination = temp_root / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(ROOT / relative, destination)

            first = subprocess.run(
                [sys.executable, str(GENERATOR), "--check", "--root", str(temp_root)],
                check=False, capture_output=True, text=True,
            )
            self.assertEqual(first.returncode, 0, first.stderr)

            data = json.loads(
                (temp_root / "src/config/rfb_runtime_profile.json").read_text(
                    encoding="utf-8"
                )
            )
            changed = copy.deepcopy(data)
            changed["rfb_window_bytes"] += 1
            (temp_root / "src/config/rfb_runtime_profile.json").write_text(
                json.dumps(changed, indent=2) + "\n", encoding="utf-8"
            )

            stale = subprocess.run(
                [sys.executable, str(GENERATOR), "--check", "--root", str(temp_root)],
                check=False, capture_output=True, text=True,
            )
            self.assertNotEqual(stale.returncode, 0)
            self.assertIn("RFB_PROFILE_GENERATED_STALE=", stale.stderr)

            regenerate = subprocess.run(
                [sys.executable, str(GENERATOR), "--root", str(temp_root)],
                check=False, capture_output=True, text=True,
            )
            self.assertEqual(regenerate.returncode, 0, regenerate.stderr)
            final = subprocess.run(
                [sys.executable, str(GENERATOR), "--check", "--root", str(temp_root)],
                check=False, capture_output=True, text=True,
            )
            self.assertEqual(final.returncode, 0, final.stderr)


class R15ProductCompositionTests(unittest.TestCase):
    def test_off_and_invalid_profile_are_inert_before_listener_creation(self) -> None:
        original_selector = wire_runtime.rfb_runtime_profile.selected_rfb_flow_config
        try:
            wire_runtime.rfb_runtime_profile.selected_rfb_flow_config = lambda: None
            off_server = wire_runtime.build_product_wire_server()
            self.assertIsNone(off_server.rfb_attachment_factory)

            def invalid_selector() -> None:
                raise ValueError("invalid selected profile")

            wire_runtime.rfb_runtime_profile.selected_rfb_flow_config = invalid_selector
            with self.assertRaises(ValueError):
                wire_runtime.build_product_wire_server()
        finally:
            wire_runtime.rfb_runtime_profile.selected_rfb_flow_config = original_selector

    def test_selected_default_is_lazy_and_fresh_for_each_wire_session(self) -> None:
        endpoints: list[tuple[str, int]] = []
        provider_peers: list[socket.socket] = []
        controlled_sockets: list[ControlledProviderSocket] = []
        attachments: list[rfb_attachment.RfbAttachment] = []
        original_make = wire_runtime._make_attachment

        def make_selected_attachment(
            flow: rfb_attachment.RfbFlowConfig,
        ) -> rfb_attachment.RfbAttachment:
            self.assertEqual(flow.provider_read_credit_limit, 32768)
            self.assertEqual(flow.provider_write_capacity, 32768)
            self.assertEqual(flow.max_data_payload, 8192)
            raw, provider_peer = socket.socketpair()
            controlled = ControlledProviderSocket(raw, endpoints)
            provider_peers.append(provider_peer)
            controlled_sockets.append(controlled)
            item = rfb_attachment.RfbAttachment(
                flow,
                socket_factory=lambda controlled=controlled: controlled,
            )
            attachments.append(item)
            return item

        wire_runtime._make_attachment = make_selected_attachment
        try:
            product_server = wire_runtime.build_product_wire_server()
            self.assertIsNotNone(product_server.rfb_attachment_factory)

            for session_index in range(2):
                server_peer, product_peer = socket.socketpair()
                outcomes = []
                worker = threading.Thread(
                    target=lambda peer=server_peer: outcomes.append(
                        product_server.serve_connection(peer)
                    )
                )
                worker.start()
                try:
                    product_peer.sendall(wire_protocol.encode_hello_frame(sequence=1))
                    accept_header, accept_payload = recv_frame(product_peer)
                    self.assertTrue(wire_protocol.is_accept_header(accept_header))
                    self.assertEqual(accept_header.sequence, 1)
                    self.assertEqual(len(accept_payload), 4)

                    self.assertEqual(len(attachments), session_index + 1)
                    item = attachments[session_index]
                    self.assertIs(
                        item.state,
                        rfb_attachment.RfbAttachmentState.IDLE,
                    )
                    self.assertEqual(len(endpoints), session_index)
                    self.assertIsNone(item.connecting_socket)
                    self.assertIsNone(item.provider_socket)

                    product_peer.sendall(
                        wire_protocol.encode_rfb_credit_frame(
                            32768,
                            sequence=2,
                        )
                    )
                    credit_header, credit_payload = recv_frame(product_peer)
                    self.assertTrue(wire_protocol.is_rfb_credit_header(credit_header))
                    self.assertEqual(credit_header.sequence, 2)
                    self.assertEqual(
                        wire_protocol.decode_rfb_credit_payload(credit_payload),
                        32768,
                    )
                    self.assertEqual(
                        endpoints[-1],
                        (rfb_attachment.INTERNAL_PROVIDER_HOST,
                         rfb_attachment.INTERNAL_PROVIDER_PORT),
                    )
                    self.assertEqual(endpoints[-1], ("127.0.0.1", 5900))
                    self.assertEqual(item.stats.connect_attempts, 1)
                    self.assertEqual(item.stats.connect_successes, 1)
                finally:
                    product_peer.close()
                    worker.join(timeout=1.0)
                    self.assertFalse(worker.is_alive())
                    self.assertEqual(len(outcomes), 1)
                    for provider_peer in provider_peers:
                        try:
                            provider_peer.close()
                        except OSError:
                            pass

            self.assertIsNot(attachments[0], attachments[1])
            self.assertIsNot(
                attachments[0].quiesce_wake_reader,
                attachments[1].quiesce_wake_reader,
            )
            self.assertTrue(controlled_sockets[0].closed)
            self.assertTrue(controlled_sockets[1].closed)
        finally:
            wire_runtime._make_attachment = original_make
            for provider_peer in provider_peers:
                try:
                    provider_peer.close()
                except OSError:
                    pass

    def test_default_service_uses_composition_without_profile_literals(self) -> None:
        service = (
            ROOT / "systemd/pi/ps-to-vnc-wire.service"
        ).read_text(encoding="utf-8")
        runtime = (ROOT / "pi/wire_runtime.py").read_text(encoding="utf-8")
        wire_server = (ROOT / "pi/wire_server.py").read_text(encoding="utf-8")
        app = (ROOT / "src/app.c").read_text(encoding="utf-8")

        self.assertIn("/usr/lib/ps-to-vnc/wire_runtime.py", service)
        for numeric in ("32768", "8192", "16384"):
            self.assertNotIn(numeric, service)
            self.assertNotIn(numeric, runtime)
        self.assertNotIn("5903", runtime)
        self.assertNotIn("192.168.50.1:5900", runtime)
        self.assertIn("selected_rfb_flow_config()", runtime)
        self.assertIn("rfb_attachment_factory=attachment_factory", runtime)
        self.assertIn("self.serve_connection(connection)", wire_server)
        self.assertIn("pstvnc_config_rfb_runtime_profile_selected", app)
        self.assertNotIn("FRAME_CONFIG", runtime)
        self.assertNotIn("FRAME_CONFIG", wire_server)


if __name__ == "__main__":
    unittest.main(verbosity=2)
