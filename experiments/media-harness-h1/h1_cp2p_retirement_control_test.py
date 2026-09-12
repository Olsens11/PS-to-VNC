#!/usr/bin/env python3
"""Host contract for CP2P item #11A Pi exact-generation retirement control."""

from __future__ import annotations

from pathlib import Path
import socket
import struct
import tempfile
import unittest

import h1_mux_server as base
from h1_cp2p_retirement_control import (
    MPEG_RETIRE_FRAME_KIND,
    decode_mpeg_retire_payload,
    encode_mpeg_retire_payload,
)
from h1_cp2p_rfb_suppression import H1Cp2pRfbPiBridge
from h1_mux_server_cp2p_start_receiver import H1Cp2pSuppressionStartReceiver


class FakeSession:
    def __init__(self, evidence: Path) -> None:
        self.profile = {
            "session_id": 0x12345678,
            "desktop_width": 640,
            "desktop_height": 448,
        }
        self.evidence = evidence
        self.display = ":0"
        self.video_producer = None
        self.sent: list[tuple[int, int, bytes, int]] = []

    def send_frame(self, kind: int, channel: int, payload: bytes = b"", flags: int = 0) -> int:
        self.sent.append((kind, channel, bytes(payload), flags))
        return len(self.sent)


def make_start_frame(session_id: int, generation: int) -> base.Frame:
    x, y, width, height = 32, 48, 320, 240
    sx, sy = x - 1, y - 1
    sw, sh = width + 3, height + 2
    payload = struct.pack(
        ">11I",
        1,
        session_id,
        generation,
        x,
        y,
        width,
        height,
        sx,
        sy,
        sw,
        sh,
    )
    return base.Frame(base.FRAME_DATA, base.CHANNEL_MPEG2, 0, 0, payload)


def make_retire_frame(session_id: int, generation: int) -> base.Frame:
    return base.Frame(
        MPEG_RETIRE_FRAME_KIND,
        base.CHANNEL_CONTROL,
        0,
        0,
        encode_mpeg_retire_payload(session_id, generation),
    )


class FakeGenerationProducer:
    def __init__(self, generation: int, bridge) -> None:
        self.generation = int(generation)
        self.bridge = bridge
        self.retired = []

    def active_generation(self) -> int:
        return self.generation

    def start_exact(self, plan):
        self.generation = int(plan.generation)
        return {"archive_path": "fake.m2v"}

    def retire_exact(self, generation: int):
        # The critical #8/#11A ordering assertion: suppression still belongs to
        # this generation while producer stop/drain completes.
        if self.bridge.suppression_generation != int(generation):
            raise AssertionError("suppression removed before producer retirement")
        self.retired.append(int(generation))
        self.generation = 0


class RetirementControlTests(unittest.TestCase):
    def test_codec_rejects_wrong_session_and_zero_generation(self) -> None:
        payload = encode_mpeg_retire_payload(7, 9)
        decoded = decode_mpeg_retire_payload(payload, expected_session_id=7)
        self.assertEqual(decoded.session_id, 7)
        self.assertEqual(decoded.generation, 9)
        with self.assertRaisesRegex(base.ProtocolError, "session mismatch"):
            decode_mpeg_retire_payload(payload, expected_session_id=8)
        with self.assertRaisesRegex(base.ProtocolError, "generation must be nonzero"):
            decode_mpeg_retire_payload(
                struct.pack(">3I", 1, 7, 0),
                expected_session_id=7,
            )

    def _prepared_receiver(self, temp_dir: str):
        bridge_sock, peer = socket.socketpair()
        bridge = H1Cp2pRfbPiBridge(
            bridge_sock,
            queue_capacity=1024 * 1024,
            max_payload=8192,
            send_data=lambda payload: None,
            desktop_width=640,
            desktop_height=448,
        )
        session = FakeSession(Path(temp_dir))
        receiver = H1Cp2pSuppressionStartReceiver(session, bridge)
        receiver._handle_start_frame(
            make_start_frame(session.profile["session_id"], 7)
        )
        return session, receiver, bridge, peer

    def test_exact_retire_clears_runtime_state_before_ack_and_keeps_generation_stale(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session, receiver, bridge, peer = self._prepared_receiver(temp_dir)
            try:
                self.assertEqual(bridge.suppression_generation, 7)
                self.assertIsNotNone(receiver.capture_plan)
                self.assertIsNotNone(receiver.peek_prepared())

                receiver._handle_retire_frame(
                    make_retire_frame(session.profile["session_id"], 7)
                )

                self.assertEqual(bridge.suppression_generation, 0)
                self.assertIsNone(receiver.capture_plan)
                self.assertIsNone(receiver.peek_prepared())
                self.assertEqual(len(session.sent), 1)
                kind, channel, payload, flags = session.sent[0]
                self.assertEqual(kind, MPEG_RETIRE_FRAME_KIND)
                self.assertEqual(channel, base.CHANNEL_CONTROL)
                self.assertEqual(flags, 0)
                ack = decode_mpeg_retire_payload(
                    payload,
                    expected_session_id=session.profile["session_id"],
                )
                self.assertEqual(ack.generation, 7)
                self.assertTrue(
                    (Path(temp_dir) / "mpeg_retire_completed.json").exists()
                )

                with self.assertRaisesRegex(base.ProtocolError, "stale MPEG START generation"):
                    receiver._handle_start_frame(
                        make_start_frame(session.profile["session_id"], 7)
                    )
            finally:
                bridge.stop()
                peer.close()

    def test_live_generation_producer_retires_before_suppression_and_ack(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            bridge_sock, peer = socket.socketpair()
            bridge = H1Cp2pRfbPiBridge(
                bridge_sock,
                queue_capacity=1024 * 1024,
                max_payload=8192,
                send_data=lambda payload: None,
                desktop_width=640,
                desktop_height=448,
            )
            session = FakeSession(Path(temp_dir))
            producer = FakeGenerationProducer(0, bridge)
            receiver = H1Cp2pSuppressionStartReceiver(session, bridge, producer)
            try:
                receiver._handle_start_frame(
                    make_start_frame(session.profile["session_id"], 8)
                )
                self.assertEqual(producer.active_generation(), 8)
                self.assertEqual(bridge.suppression_generation, 8)

                receiver._handle_retire_frame(
                    make_retire_frame(session.profile["session_id"], 8)
                )
                self.assertEqual(producer.retired, [8])
                self.assertEqual(bridge.suppression_generation, 0)
                self.assertEqual(len(session.sent), 1)
                self.assertEqual(session.sent[0][0], MPEG_RETIRE_FRAME_KIND)
            finally:
                bridge.stop()
                peer.close()

    def test_wrong_generation_never_cleans_or_acknowledges(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session, receiver, bridge, peer = self._prepared_receiver(temp_dir)
            try:
                with self.assertRaisesRegex(base.ProtocolError, "prepared-generation mismatch"):
                    receiver._handle_retire_frame(
                        make_retire_frame(session.profile["session_id"], 8)
                    )
                self.assertEqual(bridge.suppression_generation, 7)
                self.assertIsNotNone(receiver.capture_plan)
                self.assertIsNotNone(receiver.peek_prepared())
                self.assertEqual(session.sent, [])
            finally:
                bridge.stop()
                peer.close()

    def test_unexpected_live_producer_blocks_ack_and_preserves_generation(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session, receiver, bridge, peer = self._prepared_receiver(temp_dir)
            try:
                session.video_producer = object()
                with self.assertRaisesRegex(base.ProtocolError, "producer is live"):
                    receiver._handle_retire_frame(
                        make_retire_frame(session.profile["session_id"], 7)
                    )
                self.assertEqual(bridge.suppression_generation, 7)
                self.assertIsNotNone(receiver.capture_plan)
                self.assertIsNotNone(receiver.peek_prepared())
                self.assertEqual(session.sent, [])
            finally:
                session.video_producer = None
                bridge.stop()
                peer.close()


if __name__ == "__main__":
    unittest.main(verbosity=2)
