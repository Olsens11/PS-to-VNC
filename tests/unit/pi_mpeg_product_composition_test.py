#!/usr/bin/env python3
"""Deterministic host proof for R25 ordinary Pi MPEG product composition.

The fixture exercises the real accepted WireServer and R17 generation owner.
Only producer process ownership is replaced with an in-memory fake, so no
FFmpeg process, provider daemon, systemd state, or live display is touched.
"""

from __future__ import annotations

from pathlib import Path
import socket
import sys
import threading
import time
import unittest
from unittest import mock

ROOT = Path(__file__).resolve().parents[2]
PI = ROOT / "pi"
if str(PI) not in sys.path:
    sys.path.insert(0, str(PI))

import mpeg_generation as mpeg
import mpeg_product_profile
import mpeg_runtime_profile
import rfb_attachment
import wire_protocol as protocol
import wire_runtime


class FakeProducer:
    def __init__(self, plan: mpeg.MpegCapturePlan, payload: bytes) -> None:
        self.plan = plan
        self.buffer = bytearray(payload)
        self.retire_calls = 0
        self.force_close_calls = 0

    @property
    def available(self) -> int:
        return len(self.buffer)

    def take(self, maximum: int) -> bytes:
        count = min(maximum, len(self.buffer))
        result = bytes(self.buffer[:count])
        del self.buffer[:count]
        return result

    def retire(self, _timeout: float) -> None:
        self.retire_calls += 1
        self.buffer.clear()

    def force_close(self, _timeout: float) -> bool:
        self.force_close_calls += 1
        self.buffer.clear()
        return True


class ProducerFactory:
    def __init__(self) -> None:
        self.items: list[FakeProducer] = []

    def __call__(
        self,
        plan: mpeg.MpegCapturePlan,
        _buffer_capacity: int,
        _notify,
    ) -> FakeProducer:
        item = FakeProducer(
            plan,
            payload=f"generation-{plan.generation}".encode("ascii"),
        )
        self.items.append(item)
        return item


def read_exact(peer: socket.socket, byte_count: int) -> bytes:
    data = bytearray()
    while len(data) < byte_count:
        chunk = peer.recv(byte_count - len(data))
        if not chunk:
            raise EOFError("test peer closed during framed read")
        data.extend(chunk)
    return bytes(data)


def read_frame(peer: socket.socket) -> tuple[protocol.WireHeader, bytes]:
    header = protocol.decode_header(read_exact(peer, protocol.HEADER_BYTES))
    payload = read_exact(peer, header.payload_length)
    return header, payload


def read_until(
    peer: socket.socket,
    predicate,
    description: str,
) -> tuple[protocol.WireHeader, bytes]:
    for _attempt in range(12):
        header, payload = read_frame(peer)
        if predicate(header, payload):
            return header, payload
    raise AssertionError(f"did not observe {description}")


def start_control(
    session_id: int,
    generation: int,
) -> protocol.MpegStartControl:
    return protocol.MpegStartControl(
        session_id=session_id,
        generation=generation,
        base_x=32,
        base_y=16,
        base_width=640,
        base_height=416,
        suppression_x=16,
        suppression_y=8,
        suppression_width=672,
        suppression_height=440,
    )


def serve_once(server):
    server_side, peer = socket.socketpair()
    peer.settimeout(2.0)
    result: dict[str, object] = {}

    def target() -> None:
        result["outcome"] = server.serve_connection(server_side)

    thread = threading.Thread(target=target, daemon=True)
    thread.start()
    return peer, thread, result


def establish(peer: socket.socket) -> int:
    peer.sendall(protocol.encode_hello_frame())
    header, payload = read_frame(peer)
    if not protocol.is_accept_header(header):
        raise AssertionError(f"expected ACCEPT, got {header}")
    return protocol.decode_accept_payload(payload)


def finish_peer(
    peer: socket.socket,
    thread: threading.Thread,
    result: dict[str, object],
):
    try:
        peer.shutdown(socket.SHUT_WR)
    except OSError:
        pass
    thread.join(timeout=2.0)
    if thread.is_alive():
        raise AssertionError("WireServer session did not finish")
    try:
        peer.close()
    except OSError:
        pass
    return result["outcome"]


def wait_for(predicate, description: str) -> None:
    deadline = time.monotonic() + 2.0
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.001)
    raise AssertionError(f"timed out waiting for {description}")


class ProductProfileTests(unittest.TestCase):
    def test_selected_product_profile_is_exact_and_grounded(self) -> None:
        profile = mpeg_product_profile.selected_mpeg_product_profile()
        self.assertEqual(profile.display, ":0")
        self.assertEqual(
            (profile.desktop_width, profile.desktop_height),
            (704, 462),
        )
        self.assertEqual(profile.retirement_timeout_seconds, 8.0)

        source = (PI / "mpeg_product_profile.py").read_text(encoding="utf-8")
        self.assertIn(
            "3426f28b93de9519ca93e5f0e0aaf8b67cfca845",
            source,
        )
        self.assertIn("DEFAULT_RETIRE_TIMEOUT_SECONDS", source)

        producer = mpeg_runtime_profile.selected_mpeg_producer_profile()
        self.assertEqual(producer.channel_window_bytes, 524288)
        self.assertEqual(producer.buffer_capacity, 524288)
        self.assertEqual((producer.max_width, producer.max_height), (704, 480))
        self.assertEqual(
            (producer.fps_numerator, producer.fps_denominator),
            (30000, 1001),
        )

        runtime_source = (PI / "wire_runtime.py").read_text(encoding="utf-8")
        for scattered_literal in ('":0"', "704", "462", "8.0"):
            self.assertNotIn(scattered_literal, runtime_source)

    def test_product_runtime_supplies_both_rider_factories(self) -> None:
        server = wire_runtime.build_product_wire_server(
            "127.0.0.1",
            5902,
        )
        self.assertIsNotNone(server.rfb_attachment_factory)
        self.assertIsNotNone(server.mpeg_generation_factory)
        self.assertTrue(callable(server.rfb_attachment_factory))
        self.assertTrue(callable(server.mpeg_generation_factory))

    def test_stager_enrolls_product_profile_without_live_mutation(self) -> None:
        installer = (
            ROOT / "scripts/pi/install-wire-runtime.sh"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "/usr/lib/ps-to-vnc/mpeg_product_profile.py",
            installer,
        )
        self.assertIn(
            'verify_file 0644 "$MPEG_PRODUCT_PROFILE_SOURCE" '
            '"$MPEG_PRODUCT_PROFILE_DEST"',
            installer,
        )
        for forbidden in (
            "systemctl daemon-reload",
            "systemctl enable",
            "systemctl disable",
            "systemctl start",
            "systemctl stop",
            "systemctl restart",
        ):
            self.assertNotIn(forbidden, installer)


class ProductWireCompositionTests(unittest.TestCase):
    def test_rejected_and_malformed_q4_never_invoke_mpeg_factory(self) -> None:
        calls: list[int] = []

        def record_generation(
            session_id: int,
            composition,
            producer_profile,
        ):
            calls.append(session_id)
            return mpeg.MpegGenerationController(
                session_id=session_id,
                desktop_width=composition.desktop_width,
                desktop_height=composition.desktop_height,
                display=composition.display,
                retirement_timeout_seconds=composition.retirement_timeout_seconds,
                profile=producer_profile,
            )

        with mock.patch.object(
            wire_runtime,
            "_make_mpeg_generation",
            side_effect=record_generation,
        ):
            server = wire_runtime.build_product_wire_server()

            peer, thread, result = serve_once(server)
            peer.sendall(
                protocol.encode_hello_frame(
                    product_version=protocol.PRODUCT_ESTABLISHMENT_VERSION - 1
                )
            )
            header, _payload = read_frame(peer)
            self.assertTrue(protocol.is_not_accepted_header(header))
            thread.join(timeout=2.0)
            self.assertFalse(thread.is_alive())
            peer.close()
            self.assertFalse(result["outcome"].accepted)
            self.assertEqual(calls, [])

            peer, thread, result = serve_once(server)
            peer.sendall(protocol.encode_hello_frame(sequence=2))
            header, _payload = read_frame(peer)
            self.assertTrue(protocol.is_not_accepted_header(header))
            thread.join(timeout=2.0)
            self.assertFalse(thread.is_alive())
            peer.close()
            self.assertTrue(result["outcome"].protocol_failed)
            self.assertEqual(calls, [])

    def test_exact_q4_constructs_one_fresh_inert_controller(self) -> None:
        calls: list[tuple[int, object, object]] = []
        controllers: list[mpeg.MpegGenerationController] = []

        def record_generation(
            session_id: int,
            composition,
            producer_profile,
        ):
            calls.append((session_id, composition, producer_profile))
            controller = mpeg.MpegGenerationController(
                session_id=session_id,
                desktop_width=composition.desktop_width,
                desktop_height=composition.desktop_height,
                display=composition.display,
                retirement_timeout_seconds=composition.retirement_timeout_seconds,
                profile=producer_profile,
            )
            controllers.append(controller)
            return controller

        with mock.patch.object(
            wire_runtime,
            "_make_mpeg_generation",
            side_effect=record_generation,
        ):
            server = wire_runtime.build_product_wire_server()
            peer, thread, result = serve_once(server)
            session_id = establish(peer)

            wait_for(lambda: len(calls) == 1, "exact MPEG factory invocation")
            self.assertEqual(calls[0][0], session_id)
            composition = calls[0][1]
            self.assertEqual(composition.display, ":0")
            self.assertEqual(
                (composition.desktop_width, composition.desktop_height),
                (704, 462),
            )
            self.assertEqual(composition.retirement_timeout_seconds, 8.0)

            controller = controllers[0]
            self.assertEqual(controller.session_id, session_id)
            self.assertIsNone(controller.producer)
            self.assertIsNone(controller.plan)
            self.assertEqual(controller.highest_generation, 0)
            self.assertEqual(controller.credit_bytes, 0)

            outcome = finish_peer(peer, thread, result)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)

    def test_credit_is_inert_start_is_first_producer_edge_and_wire_serializes(self) -> None:
        producer_factory = ProducerFactory()
        controllers: list[mpeg.MpegGenerationController] = []
        attachments: list[rfb_attachment.RfbAttachment] = []
        original_make_attachment = wire_runtime._make_attachment

        def make_generation(
            session_id: int,
            composition,
            producer_profile,
        ):
            controller = mpeg.MpegGenerationController(
                session_id=session_id,
                desktop_width=composition.desktop_width,
                desktop_height=composition.desktop_height,
                display=composition.display,
                retirement_timeout_seconds=composition.retirement_timeout_seconds,
                profile=producer_profile,
                producer_factory=producer_factory,
            )
            controllers.append(controller)
            return controller

        def make_attachment(flow):
            attachment = original_make_attachment(flow)
            attachments.append(attachment)
            return attachment

        with (
            mock.patch.object(
                wire_runtime,
                "_make_mpeg_generation",
                side_effect=make_generation,
            ),
            mock.patch.object(
                wire_runtime,
                "_make_attachment",
                side_effect=make_attachment,
            ),
        ):
            server = wire_runtime.build_product_wire_server()
            peer, thread, result = serve_once(server)
            session_id = establish(peer)

            wait_for(lambda: len(controllers) == 1, "controller attachment")
            controller = controllers[0]
            self.assertEqual(producer_factory.items, [])

            # Ordinary RFB composition may publish its initial credit, but no
            # channel-1 client CREDIT is sent so its provider remains lazy.
            header, _payload = read_frame(peer)
            self.assertTrue(protocol.is_rfb_credit_header(header))

            peer.sendall(protocol.encode_mpeg_credit_frame(128, sequence=2))
            wait_for(lambda: controller.credit_bytes == 128, "MPEG credit")
            self.assertEqual(producer_factory.items, [])

            peer.sendall(
                protocol.encode_mpeg_start_frame(
                    start_control(session_id, 1),
                    sequence=3,
                )
            )
            wait_for(lambda: len(producer_factory.items) == 1, "generation 1 producer")
            data_header, data_payload = read_until(
                peer,
                lambda h, _p: protocol.is_mpeg_data_header(h),
                "generation 1 MPEG DATA",
            )
            self.assertEqual(data_header.channel, protocol.CHANNEL_MPEG2)
            self.assertEqual(data_payload, b"generation-1")

            peer.sendall(
                protocol.encode_mpeg_retire_frame(
                    protocol.MpegRetireControl(
                        session_id=session_id,
                        generation=1,
                    ),
                    sequence=4,
                )
            )
            _header, payload = read_until(
                peer,
                lambda h, _p: protocol.is_mpeg_retire_header(h),
                "generation 1 RETIRE completion",
            )
            self.assertEqual(
                protocol.decode_mpeg_retire_payload(payload),
                protocol.MpegRetireControl(session_id=session_id, generation=1),
            )
            wait_for(
                lambda: controller.state is mpeg.MpegGenerationState.IDLE,
                "generation 1 retirement confirmation",
            )
            self.assertEqual(controller.highest_generation, 1)

            peer.sendall(
                protocol.encode_mpeg_start_frame(
                    start_control(session_id, 2),
                    sequence=5,
                )
            )
            wait_for(lambda: len(producer_factory.items) == 2, "generation 2 producer")
            _header, payload = read_until(
                peer,
                lambda h, _p: protocol.is_mpeg_data_header(h),
                "generation 2 MPEG DATA",
            )
            self.assertEqual(payload, b"generation-2")

            peer.sendall(
                protocol.encode_mpeg_retire_frame(
                    protocol.MpegRetireControl(
                        session_id=session_id,
                        generation=2,
                    ),
                    sequence=6,
                )
            )
            _header, payload = read_until(
                peer,
                lambda h, _p: protocol.is_mpeg_retire_header(h),
                "generation 2 RETIRE completion",
            )
            self.assertEqual(
                protocol.decode_mpeg_retire_payload(payload),
                protocol.MpegRetireControl(session_id=session_id, generation=2),
            )
            wait_for(
                lambda: controller.state is mpeg.MpegGenerationState.IDLE,
                "generation 2 retirement confirmation",
            )
            self.assertEqual(controller.highest_generation, 2)
            self.assertEqual(len(controllers), 1)
            self.assertEqual(attachments[0].stats.connect_attempts, 0)

            # Same controller rejects dead-generation identity. WireServer owns
            # the resulting terminal session and closes the exact controller.
            peer.sendall(
                protocol.encode_mpeg_start_frame(
                    start_control(session_id, 1),
                    sequence=7,
                )
            )
            thread.join(timeout=2.0)
            self.assertFalse(thread.is_alive())
            peer.close()

            outcome = result["outcome"]
            self.assertTrue(outcome.accepted)
            self.assertTrue(outcome.protocol_failed)
            self.assertEqual(len(controllers), 1)
            self.assertEqual(controller.highest_generation, 2)

    def test_close_failure_marks_session_failed(self) -> None:
        controllers: list[mpeg.MpegGenerationController] = []
        close_calls: list[int] = []

        def make_generation(
            session_id: int,
            composition,
            producer_profile,
        ):
            controller = mpeg.MpegGenerationController(
                session_id=session_id,
                desktop_width=composition.desktop_width,
                desktop_height=composition.desktop_height,
                display=composition.display,
                retirement_timeout_seconds=composition.retirement_timeout_seconds,
                profile=producer_profile,
            )
            original_close = controller.close

            def failing_close() -> bool:
                close_calls.append(session_id)
                original_close()
                return False

            controller.close = failing_close
            controllers.append(controller)
            return controller

        with mock.patch.object(
            wire_runtime,
            "_make_mpeg_generation",
            side_effect=make_generation,
        ):
            server = wire_runtime.build_product_wire_server()
            peer, thread, result = serve_once(server)
            session_id = establish(peer)
            wait_for(lambda: len(controllers) == 1, "controller attachment")

            outcome = finish_peer(peer, thread, result)
            self.assertTrue(outcome.accepted)
            self.assertTrue(outcome.protocol_failed)
            self.assertEqual(close_calls, [session_id])

    def test_factory_failure_is_terminal_but_next_session_is_fresh(self) -> None:
        calls: list[int] = []
        controllers: list[mpeg.MpegGenerationController] = []

        def make_generation(
            session_id: int,
            composition,
            producer_profile,
        ):
            calls.append(session_id)
            if len(calls) == 1:
                raise ValueError("injected R25 factory failure")
            controller = mpeg.MpegGenerationController(
                session_id=session_id,
                desktop_width=composition.desktop_width,
                desktop_height=composition.desktop_height,
                display=composition.display,
                retirement_timeout_seconds=composition.retirement_timeout_seconds,
                profile=producer_profile,
            )
            controllers.append(controller)
            return controller

        with mock.patch.object(
            wire_runtime,
            "_make_mpeg_generation",
            side_effect=make_generation,
        ):
            server = wire_runtime.build_product_wire_server()

            peer_a, thread_a, result_a = serve_once(server)
            session_a = establish(peer_a)
            thread_a.join(timeout=2.0)
            self.assertFalse(thread_a.is_alive())
            peer_a.close()
            outcome_a = result_a["outcome"]
            self.assertTrue(outcome_a.accepted)
            self.assertTrue(outcome_a.protocol_failed)
            self.assertEqual(calls, [session_a])

            peer_b, thread_b, result_b = serve_once(server)
            session_b = establish(peer_b)
            self.assertNotEqual(session_b, session_a)
            wait_for(lambda: len(controllers) == 1, "Session B controller")
            self.assertEqual(controllers[0].session_id, session_b)

            outcome_b = finish_peer(peer_b, thread_b, result_b)
            self.assertTrue(outcome_b.accepted)
            self.assertFalse(outcome_b.protocol_failed)
            self.assertEqual(calls, [session_a, session_b])


class ScopeTests(unittest.TestCase):
    def test_r25_does_not_add_wire_or_ps2_activation_mechanism(self) -> None:
        runtime = (PI / "wire_runtime.py").read_text(encoding="utf-8")
        profile = (PI / "mpeg_product_profile.py").read_text(encoding="utf-8")
        combined = runtime + "\n" + profile

        for forbidden in (
            "sendall(",
            "recv(",
            "START+SELECT",
            "pstvnc_app_mpeg",
            "systemctl ",
            "subprocess.Popen",
        ):
            self.assertNotIn(forbidden, combined)


if __name__ == "__main__":
    unittest.main(verbosity=2)
