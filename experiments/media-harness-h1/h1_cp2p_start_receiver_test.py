#!/usr/bin/env python3
"""
File synopsis:
    Host tests for CP2P item #5: fixed START decoding, active-desktop/session
    validation, prepared-generation semantics, and same-reader receive dispatch.
"""

from __future__ import annotations

import struct
import tempfile
from pathlib import Path
import unittest

import h1_mux_server as base
import h1_cp2p_start_receiver as start

SESSION_ID = 0x12345678
DESKTOP_WIDTH = 704
DESKTOP_HEIGHT = 462


def make_payload(
    *,
    version: int = 1,
    session_id: int = SESSION_ID,
    generation: int = 7,
    draw_x: int = 32,
    draw_y: int = 32,
    draw_width: int = 320,
    draw_height: int = 240,
    suppression_x: int = 16,
    suppression_y: int = 16,
    suppression_width: int = 352,
    suppression_height: int = 272,
) -> bytes:
    return struct.pack(
        ">11I",
        version,
        session_id,
        generation,
        draw_x,
        draw_y,
        draw_width,
        draw_height,
        suppression_x,
        suppression_y,
        suppression_width,
        suppression_height,
    )


def decode(payload: bytes):
    return start.decode_mpeg_start_payload(
        payload,
        expected_session_id=SESSION_ID,
        desktop_width=DESKTOP_WIDTH,
        desktop_height=DESKTOP_HEIGHT,
    )


class DecodeTests(unittest.TestCase):
    def test_valid_payload_preserves_exact_geometry(self) -> None:
        request = decode(make_payload())
        self.assertEqual(request.session_id, SESSION_ID)
        self.assertEqual(request.generation, 7)
        self.assertEqual(
            (request.draw_x, request.draw_y, request.draw_width, request.draw_height),
            (32, 32, 320, 240),
        )
        self.assertEqual(
            (
                request.suppression_x,
                request.suppression_y,
                request.suppression_width,
                request.suppression_height,
            ),
            (16, 16, 352, 272),
        )

    def assert_protocol_error(self, payload: bytes) -> None:
        with self.assertRaises(base.ProtocolError):
            decode(payload)

    def test_rejects_wrong_length(self) -> None:
        self.assert_protocol_error(make_payload()[:-1])

    def test_rejects_wrong_version(self) -> None:
        self.assert_protocol_error(make_payload(version=2))

    def test_rejects_wrong_session(self) -> None:
        self.assert_protocol_error(make_payload(session_id=SESSION_ID + 1))

    def test_rejects_zero_generation(self) -> None:
        self.assert_protocol_error(make_payload(generation=0))

    def test_rejects_signed_int_overflow(self) -> None:
        self.assert_protocol_error(make_payload(draw_x=0x80000000))

    def test_rejects_unaligned_base_geometry(self) -> None:
        self.assert_protocol_error(make_payload(draw_width=321))

    def test_rejects_suppression_not_containing_base(self) -> None:
        self.assert_protocol_error(make_payload(suppression_width=320))

    def test_rejects_base_outside_active_desktop(self) -> None:
        self.assert_protocol_error(
            make_payload(
                draw_x=400,
                draw_width=320,
                suppression_x=400,
                suppression_width=320,
            )
        )

    def test_rejects_suppression_outside_active_desktop(self) -> None:
        self.assert_protocol_error(
            make_payload(
                suppression_x=16,
                suppression_width=700,
            )
        )


class PreparedStateTests(unittest.TestCase):
    def test_one_prepared_generation_then_strictly_newer_after_release(self) -> None:
        state = start.H1Cp2pPreparedStartState()
        first = decode(make_payload(generation=7))
        state.prepare(first)
        self.assertIs(state.peek(), first)

        with self.assertRaises(base.ProtocolError):
            state.prepare(decode(make_payload(generation=8)))

        released = state.release_exact(7)
        self.assertIs(released, first)
        self.assertIsNone(state.peek())

        with self.assertRaises(base.ProtocolError):
            state.prepare(decode(make_payload(generation=7)))

        newer = decode(make_payload(generation=8))
        state.prepare(newer)
        self.assertIs(state.peek(), newer)

    def test_release_requires_exact_generation(self) -> None:
        state = start.H1Cp2pPreparedStartState()
        state.prepare(decode(make_payload(generation=9)))
        with self.assertRaises(base.ProtocolError):
            state.release_exact(8)
        self.assertEqual(state.peek().generation, 9)  # type: ignore[union-attr]


class SameReaderDispatchTests(unittest.TestCase):
    def test_start_is_consumed_and_next_frame_returns_on_same_call_chain(self) -> None:
        original_receive = base.receive_frame
        original_installed = start._SHIM_INSTALLED
        original_downstream = start._DOWNSTREAM_RECEIVE_FRAME

        fake_sock = object()
        frames = [
            base.Frame(base.FRAME_DATA, base.CHANNEL_MPEG2, 0, 1, make_payload()),
            base.Frame(base.FRAME_TELEMETRY, base.CHANNEL_TELEMETRY, 0, 2, b"next"),
        ]

        def fake_receive(sock):
            self.assertIs(sock, fake_sock)
            return frames.pop(0)

        class DummySession:
            pass

        with tempfile.TemporaryDirectory() as tmp:
            session = DummySession()
            session.sock = fake_sock
            session.profile = {
                "session_id": SESSION_ID,
                "desktop_width": DESKTOP_WIDTH,
                "desktop_height": DESKTOP_HEIGHT,
            }
            session.evidence = Path(tmp)
            session.expected_rx_sequence = 1

            receiver = start.H1Cp2pStartReceiver(session)  # type: ignore[arg-type]

            try:
                base.receive_frame = fake_receive
                start._SHIM_INSTALLED = False
                start._DOWNSTREAM_RECEIVE_FRAME = None
                receiver.start()

                returned = base.receive_frame(fake_sock)
                self.assertEqual(returned.kind, base.FRAME_TELEMETRY)
                self.assertEqual(returned.sequence, 2)
                self.assertEqual(session.expected_rx_sequence, 2)

                prepared = receiver.peek_prepared()
                self.assertIsNotNone(prepared)
                self.assertEqual(prepared.generation, 7)  # type: ignore[union-attr]
                self.assertTrue((Path(tmp) / "mpeg_start_prepared.json").exists())
            finally:
                receiver.stop()
                with start._START_REGISTRY_LOCK:
                    start._START_BY_PSTV_SOCKET.pop(fake_sock, None)
                base.receive_frame = original_receive
                start._SHIM_INSTALLED = original_installed
                start._DOWNSTREAM_RECEIVE_FRAME = original_downstream

    def test_nonzero_start_flags_are_rejected_after_sequence_consumption(self) -> None:
        original_receive = base.receive_frame
        original_installed = start._SHIM_INSTALLED
        original_downstream = start._DOWNSTREAM_RECEIVE_FRAME

        fake_sock = object()

        def fake_receive(sock):
            return base.Frame(
                base.FRAME_DATA,
                base.CHANNEL_MPEG2,
                1,
                1,
                make_payload(),
            )

        class DummySession:
            pass

        with tempfile.TemporaryDirectory() as tmp:
            session = DummySession()
            session.sock = fake_sock
            session.profile = {
                "session_id": SESSION_ID,
                "desktop_width": DESKTOP_WIDTH,
                "desktop_height": DESKTOP_HEIGHT,
            }
            session.evidence = Path(tmp)
            session.expected_rx_sequence = 1
            receiver = start.H1Cp2pStartReceiver(session)  # type: ignore[arg-type]

            try:
                base.receive_frame = fake_receive
                start._SHIM_INSTALLED = False
                start._DOWNSTREAM_RECEIVE_FRAME = None
                receiver.start()
                with self.assertRaises(base.ProtocolError):
                    base.receive_frame(fake_sock)
                self.assertEqual(session.expected_rx_sequence, 2)
                self.assertIsNone(receiver.peek_prepared())
            finally:
                receiver.stop()
                with start._START_REGISTRY_LOCK:
                    start._START_BY_PSTV_SOCKET.pop(fake_sock, None)
                base.receive_frame = original_receive
                start._SHIM_INSTALLED = original_installed
                start._DOWNSTREAM_RECEIVE_FRAME = original_downstream


if __name__ == "__main__":
    unittest.main(verbosity=2)
