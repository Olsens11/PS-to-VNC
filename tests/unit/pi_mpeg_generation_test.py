#!/usr/bin/env python3
"""Deterministic host proof for R17 Pi MPEG control/producer ownership."""

from __future__ import annotations

from pathlib import Path
import socket
import struct
import subprocess
import sys
import threading
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]
PI = ROOT / "pi"
if str(PI) not in sys.path:
    sys.path.insert(0, str(PI))

import mpeg_generation as mpeg
import mpeg_runtime_profile
import mpeg_runtime_profile_generated as generated
import wire_protocol as protocol
import wire_server


class FakeProducer:
    def __init__(
        self,
        plan: mpeg.MpegCapturePlan,
        payload: bytes = b"abcdefgh",
        *,
        retire_fails: bool = False,
    ) -> None:
        self.plan = plan
        self.buffer = bytearray(payload)
        self.retire_fails = retire_fails
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
        if self.retire_fails:
            raise mpeg.MpegGenerationError("injected producer retirement failure")
        self.buffer.clear()

    def force_close(self, _timeout: float) -> bool:
        self.force_close_calls += 1
        self.buffer.clear()
        return not self.retire_fails


class ProducerFactory:
    def __init__(
        self,
        payload: bytes = b"abcdefgh",
        *,
        retire_fails: bool = False,
        launch_fails: bool = False,
    ) -> None:
        self.payload = payload
        self.retire_fails = retire_fails
        self.launch_fails = launch_fails
        self.items: list[FakeProducer] = []
        self.capacities: list[int] = []

    def __call__(
        self,
        plan: mpeg.MpegCapturePlan,
        buffer_capacity: int,
        _notify,
    ) -> FakeProducer:
        self.capacities.append(buffer_capacity)
        if self.launch_fails:
            raise mpeg.MpegGenerationError("injected producer launch failure")
        item = FakeProducer(
            plan,
            self.payload,
            retire_fails=self.retire_fails,
        )
        self.items.append(item)
        return item


def start_control(
    session_id: int = 41,
    generation: int = 1,
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


def make_controller(
    *,
    session_id: int = 41,
    timeout: float = 1.0,
    factory: ProducerFactory | None = None,
    suppression: mpeg.SuppressionPreparation | None = None,
) -> tuple[mpeg.MpegGenerationController, ProducerFactory]:
    actual_factory = factory or ProducerFactory()
    return (
        mpeg.MpegGenerationController(
            session_id=session_id,
            desktop_width=704,
            desktop_height=462,
            display=":0",
            retirement_timeout_seconds=timeout,
            producer_factory=actual_factory,
            suppression=suppression,
        ),
        actual_factory,
    )


def read_exact(peer: socket.socket, byte_count: int) -> bytes:
    data = bytearray()
    while len(data) < byte_count:
        chunk = peer.recv(byte_count - len(data))
        if not chunk:
            raise EOFError("test peer closed during frame read")
        data.extend(chunk)
    return bytes(data)


def read_frame(
    peer: socket.socket,
) -> tuple[protocol.WireHeader, bytes]:
    header = protocol.decode_header(read_exact(peer, protocol.HEADER_BYTES))
    return header, read_exact(peer, header.payload_length)


class MpegProtocolTests(unittest.TestCase):
    def test_control_bytes_exactly_match_accepted_ps2_contract(self) -> None:
        control = start_control(session_id=0x10203040, generation=0x55667788)
        payload = protocol.encode_mpeg_start_payload(control)
        self.assertEqual(
            payload,
            struct.pack(
                ">11I",
                1,
                0x10203040,
                0x55667788,
                32,
                16,
                640,
                416,
                16,
                8,
                672,
                440,
            ),
        )
        self.assertEqual(protocol.decode_mpeg_start_payload(payload), control)

        frame = protocol.encode_mpeg_start_frame(control, sequence=7)
        header = protocol.decode_header(frame[: protocol.HEADER_BYTES])
        self.assertTrue(protocol.is_mpeg_start_header(header))
        self.assertEqual(header.kind, 11)
        self.assertEqual(header.channel, 0)
        self.assertEqual(header.flags, 0)
        self.assertEqual(header.payload_length, 44)

        retire = protocol.MpegRetireControl(
            session_id=0x10203040,
            generation=0x55667788,
        )
        retire_payload = protocol.encode_mpeg_retire_payload(retire)
        self.assertEqual(
            retire_payload,
            struct.pack(">3I", 1, 0x10203040, 0x55667788),
        )
        self.assertEqual(protocol.decode_mpeg_retire_payload(retire_payload), retire)
        retire_frame = protocol.encode_mpeg_retire_frame(retire, sequence=8)
        retire_header = protocol.decode_header(
            retire_frame[: protocol.HEADER_BYTES]
        )
        self.assertTrue(protocol.is_mpeg_retire_header(retire_header))
        self.assertEqual(retire_header.kind, 10)
        self.assertEqual(retire_header.channel, 0)
        self.assertEqual(retire_header.payload_length, 12)

    def test_malformed_control_and_wrong_envelopes_are_rejected(self) -> None:
        control = start_control()
        payload = bytearray(protocol.encode_mpeg_start_payload(control))
        payload[:4] = struct.pack(">I", 2)
        with self.assertRaises(protocol.WireProtocolError):
            protocol.decode_mpeg_start_payload(bytes(payload))
        with self.assertRaises(protocol.WireProtocolError):
            protocol.decode_mpeg_start_payload(bytes(payload[:-1]))
        with self.assertRaises(protocol.WireProtocolError):
            protocol.decode_mpeg_retire_payload(b"\x00" * 12)

        wrong_channel = protocol.WireHeader(
            version=1,
            kind=protocol.FRAME_MPEG_START,
            channel=protocol.CHANNEL_MPEG2,
            flags=0,
            sequence=2,
            payload_length=44,
        )
        wrong_flags = protocol.WireHeader(
            version=1,
            kind=protocol.FRAME_MPEG_RETIRE,
            channel=protocol.CHANNEL_CONTROL,
            flags=1,
            sequence=3,
            payload_length=12,
        )
        self.assertFalse(protocol.is_mpeg_start_header(wrong_channel))
        self.assertFalse(protocol.is_mpeg_retire_header(wrong_flags))

        data = protocol.encode_mpeg_data_frame(b"x", sequence=4)
        data_header = protocol.decode_header(data[: protocol.HEADER_BYTES])
        self.assertTrue(protocol.is_mpeg_data_header(data_header))
        self.assertFalse(protocol.is_rfb_data_header(data_header))
        with self.assertRaises(protocol.WireProtocolError):
            protocol.encode_mpeg_data_frame(b"", sequence=4)


class MpegProfileTests(unittest.TestCase):
    def test_generated_configuration_projection_is_current(self) -> None:
        completed = subprocess.run(
            [
                sys.executable,
                str(ROOT / "scripts/generate-mpeg-runtime-profile.py"),
                "--check",
            ],
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)
        profile = mpeg_runtime_profile.selected_mpeg_producer_profile()
        self.assertEqual(profile.channel_window_bytes, 524288)
        self.assertEqual(profile.buffer_capacity, 524288)
        self.assertEqual((profile.max_width, profile.max_height), (704, 480))
        self.assertEqual(
            (profile.fps_numerator, profile.fps_denominator),
            (30000, 1001),
        )
        self.assertEqual(generated.MPEG_CHANNEL_WINDOW_BYTES, 524288)


class MpegGenerationTests(unittest.TestCase):
    def test_start_validates_session_generation_geometry_and_exact_capture(self) -> None:
        controller, factory = make_controller()
        plan = controller.start_exact(start_control())
        try:
            self.assertEqual(plan.capture, mpeg.MpegRectangle(32, 16, 640, 416))
            self.assertEqual(
                plan.suppression,
                mpeg.MpegRectangle(16, 8, 672, 440),
            )
            self.assertNotEqual(plan.capture, plan.suppression)
            self.assertEqual(plan.command[0], "ffmpeg")
            self.assertIn("640x416", plan.command)
            self.assertIn(":0+32,16", plan.command)
            self.assertIn("30000/1001", plan.command)
            self.assertEqual(factory.capacities, [524288])
            self.assertEqual(controller.suppression.generation, 1)
        finally:
            controller.close()

        for invalid in (
            start_control(session_id=42),
            protocol.MpegStartControl(
                **{**start_control().__dict__, "generation": 0}
            ),
            protocol.MpegStartControl(
                **{**start_control().__dict__, "base_width": 639}
            ),
            protocol.MpegStartControl(
                **{**start_control().__dict__, "suppression_x": 64}
            ),
            protocol.MpegStartControl(
                **{**start_control().__dict__, "base_height": 480}
            ),
        ):
            item, _ = make_controller()
            try:
                with self.assertRaises(mpeg.MpegGenerationError):
                    item.start_exact(invalid)
            finally:
                item.close()

    def test_one_generation_and_launch_failure_fail_closed(self) -> None:
        controller, _ = make_controller()
        controller.start_exact(start_control(generation=3))
        try:
            with self.assertRaises(mpeg.MpegGenerationError):
                controller.start_exact(start_control(generation=4))
        finally:
            controller.close()

        failing_factory = ProducerFactory(launch_fails=True)
        suppression = mpeg.SuppressionPreparation()
        failed, _ = make_controller(
            factory=failing_factory,
            suppression=suppression,
        )
        try:
            with self.assertRaises(mpeg.MpegGenerationError):
                failed.start_exact(start_control(generation=5))
            self.assertIs(failed.state, mpeg.MpegGenerationState.FAILED)
            self.assertEqual(suppression.generation, 0)
            self.assertEqual(failed.highest_generation, 5)
        finally:
            failed.close()

    def test_credit_is_session_scoped_but_emission_is_exact_generation(self) -> None:
        controller, factory = make_controller()
        controller.add_credit(8)
        controller.start_exact(start_control(generation=1))
        lease = controller.begin_emission(4)
        self.assertIsNotNone(lease)
        assert lease is not None
        self.assertEqual(lease.generation, 1)
        self.assertEqual(lease.payload, b"abcd")
        self.assertEqual(controller.credit_bytes, 4)
        controller.finish_emission(lease)

        completion = controller.retire_exact(
            protocol.MpegRetireControl(session_id=41, generation=1)
        )
        controller.confirm_retire_completion(completion)
        self.assertEqual(controller.credit_bytes, 4)

        controller.start_exact(start_control(generation=2))
        self.assertIsNot(factory.items[0], factory.items[1])
        self.assertEqual(controller.suppression.generation, 2)
        with self.assertRaises(mpeg.MpegGenerationError):
            controller.start_exact(start_control(generation=1))
        controller.close()

    def test_retire_closes_admission_and_waits_for_inflight_lease(self) -> None:
        controller, factory = make_controller(timeout=1.0)
        controller.add_credit(8)
        controller.start_exact(start_control())
        lease = controller.begin_emission(4)
        self.assertIsNotNone(lease)
        assert lease is not None

        results: list[protocol.MpegRetireControl] = []
        errors: list[BaseException] = []

        def retire() -> None:
            try:
                results.append(
                    controller.retire_exact(
                        protocol.MpegRetireControl(
                            session_id=41,
                            generation=1,
                        )
                    )
                )
            except BaseException as exc:
                errors.append(exc)

        worker = threading.Thread(target=retire)
        worker.start()
        deadline = time.monotonic() + 1.0
        while (
            controller.state is not mpeg.MpegGenerationState.RETIRING
            and time.monotonic() < deadline
        ):
            time.sleep(0.001)
        self.assertIs(controller.state, mpeg.MpegGenerationState.RETIRING)
        self.assertTrue(worker.is_alive())
        self.assertIsNone(controller.begin_emission(4))

        controller.finish_emission(lease)
        worker.join(timeout=1.0)
        self.assertFalse(worker.is_alive())
        self.assertEqual(errors, [])
        self.assertEqual(len(results), 1)
        self.assertEqual(factory.items[0].retire_calls, 1)
        self.assertEqual(controller.suppression.generation, 0)
        self.assertIs(
            controller.state,
            mpeg.MpegGenerationState.RETIRED_PENDING_COMPLETION,
        )
        controller.confirm_retire_completion(results[0])
        self.assertIs(controller.state, mpeg.MpegGenerationState.IDLE)
        controller.close()

    def test_timeout_or_producer_failure_never_becomes_retire_completion(self) -> None:
        controller, _ = make_controller(timeout=0.02)
        controller.add_credit(4)
        controller.start_exact(start_control())
        lease = controller.begin_emission(4)
        self.assertIsNotNone(lease)
        with self.assertRaises(mpeg.MpegGenerationError):
            controller.retire_exact(
                protocol.MpegRetireControl(session_id=41, generation=1)
            )
        self.assertIs(controller.state, mpeg.MpegGenerationState.FAILED)
        assert lease is not None
        controller.finish_emission(lease)
        controller.close()

        failing_factory = ProducerFactory(retire_fails=True)
        failed, _ = make_controller(factory=failing_factory)
        failed.start_exact(start_control(generation=7))
        with self.assertRaises(mpeg.MpegGenerationError):
            failed.retire_exact(
                protocol.MpegRetireControl(session_id=41, generation=7)
            )
        self.assertIs(failed.state, mpeg.MpegGenerationState.FAILED)
        self.assertNotEqual(
            failed.state,
            mpeg.MpegGenerationState.RETIRED_PENDING_COMPLETION,
        )
        failed.close()

    def test_channel_credit_is_bounded_independently(self) -> None:
        controller, _ = make_controller()
        controller.add_credit(controller.profile.channel_window_bytes)
        with self.assertRaises(mpeg.MpegGenerationError):
            controller.add_credit(1)
        controller.close()


class MpegWireIntegrationTests(unittest.TestCase):
    def test_one_wire_owner_serializes_data_and_retire_completion(self) -> None:
        controllers: list[mpeg.MpegGenerationController] = []
        factories: list[ProducerFactory] = []

        def make_generation(session_id: int) -> mpeg.MpegGenerationController:
            factory = ProducerFactory(payload=b"WIRE")
            item, _ = make_controller(
                session_id=session_id,
                factory=factory,
            )
            controllers.append(item)
            factories.append(factory)
            return item

        service = wire_server.WireServer(
            session_ids=wire_server.SessionIdAllocator(200),
            mpeg_generation_factory=make_generation,
        )
        product, peer = socket.socketpair()
        outcomes: list[wire_server.WireSessionOutcome] = []
        worker = threading.Thread(
            target=lambda: outcomes.append(service.serve_connection(product))
        )
        worker.start()
        try:
            peer.sendall(protocol.encode_hello_frame())
            accept_header, accept_payload = read_frame(peer)
            self.assertTrue(protocol.is_accept_header(accept_header))
            self.assertEqual(protocol.decode_accept_payload(accept_payload), 200)

            peer.sendall(protocol.encode_mpeg_credit_frame(4, sequence=2))
            peer.sendall(
                protocol.encode_mpeg_start_frame(
                    start_control(session_id=200, generation=9),
                    sequence=3,
                )
            )
            data_header, data_payload = read_frame(peer)
            self.assertTrue(protocol.is_mpeg_data_header(data_header))
            self.assertEqual(data_header.sequence, 2)
            self.assertEqual(data_payload, b"WIRE")

            peer.sendall(
                protocol.encode_mpeg_retire_frame(
                    protocol.MpegRetireControl(
                        session_id=200,
                        generation=9,
                    ),
                    sequence=4,
                )
            )
            retire_header, retire_payload = read_frame(peer)
            self.assertTrue(protocol.is_mpeg_retire_header(retire_header))
            self.assertEqual(retire_header.sequence, 3)
            self.assertEqual(
                protocol.decode_mpeg_retire_payload(retire_payload),
                protocol.MpegRetireControl(
                    session_id=200,
                    generation=9,
                ),
            )
            self.assertIs(
                controllers[0].state,
                mpeg.MpegGenerationState.IDLE,
            )
            self.assertEqual(factories[0].items[0].retire_calls, 1)
        finally:
            peer.close()
            worker.join(timeout=1.0)
        self.assertFalse(worker.is_alive())
        self.assertEqual(len(outcomes), 1)
        self.assertTrue(outcomes[0].accepted)
        self.assertFalse(outcomes[0].protocol_failed)


if __name__ == "__main__":
    unittest.main(verbosity=2)
