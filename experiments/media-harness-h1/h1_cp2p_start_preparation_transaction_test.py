#!/usr/bin/env python3
"""Host contract for atomic CP2P START suppression/capture preparation."""

from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace
import socket
import struct
import tempfile
import unittest

import h1_mux_server as base
from h1_cp2p_rfb_suppression import H1Cp2pRfbPiBridge
from h1_mux_server_cp2p_start_receiver import H1Cp2pSuppressionStartReceiver


def make_start_frame(
    *,
    session_id: int,
    generation: int,
    draw_x: int = 32,
    draw_y: int = 48,
    draw_width: int = 320,
    draw_height: int = 240,
) -> base.Frame:
    suppression_x = draw_x - 1
    suppression_y = draw_y - 1
    suppression_right = draw_x + draw_width + 2
    suppression_bottom = draw_y + draw_height + 1
    payload = struct.pack(
        ">11I",
        1,
        session_id,
        generation,
        draw_x,
        draw_y,
        draw_width,
        draw_height,
        suppression_x,
        suppression_y,
        suppression_right - suppression_x,
        suppression_bottom - suppression_y,
    )
    return base.Frame(base.FRAME_DATA, base.CHANNEL_MPEG2, 0, 0, payload)


class CompoundStartRollbackTests(unittest.TestCase):
    def test_capture_failure_retires_suppression_and_keeps_generation_stale(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            bridge_sock, fake_vnc = socket.socketpair()
            bridge = H1Cp2pRfbPiBridge(
                bridge_sock,
                queue_capacity=1024 * 1024,
                max_payload=8192,
                send_data=lambda payload: None,
                desktop_width=640,
                desktop_height=448,
            )
            session = SimpleNamespace(
                profile={
                    "session_id": 0x12345678,
                    "desktop_width": 640,
                    "desktop_height": 448,
                },
                evidence=Path(temp_dir),
                # START validation does not depend on DISPLAY, so this forces the
                # later exact-capture preparation step to fail after suppression
                # has been installed.
                display="",
            )
            receiver = H1Cp2pSuppressionStartReceiver(session, bridge)
            frame = make_start_frame(
                session_id=session.profile["session_id"],
                generation=7,
            )

            try:
                with self.assertRaisesRegex(
                    base.ProtocolError,
                    "non-empty X11 display",
                ):
                    receiver._handle_start_frame(frame)

                self.assertIsNone(receiver.peek_prepared())
                self.assertIsNone(receiver.capture_plan)
                self.assertEqual(bridge.suppression_generation, 0)
                pending, active, rect = bridge._suppression_state()
                self.assertFalse(pending)
                self.assertFalse(active)
                self.assertIsNone(rect)

                for name in (
                    "mpeg_start_prepared.json",
                    "rfb_suppression_prepared.json",
                    "mpeg_capture_prepared.json",
                ):
                    self.assertFalse((Path(temp_dir) / name).exists())

                # Rollback clears usable state but deliberately does not lower
                # item-#5's generation high-water. Reusing generation 7 remains
                # stale even after the environmental fault is corrected.
                session.display = ":0"
                with self.assertRaisesRegex(base.ProtocolError, "stale MPEG START generation"):
                    receiver._handle_start_frame(frame)

                self.assertIsNone(receiver.peek_prepared())
                self.assertEqual(bridge.suppression_generation, 0)
            finally:
                bridge.stop()
                fake_vnc.close()


class FailingRollbackProducer:
    def __init__(self, generation: int) -> None:
        self.generation = int(generation)
        self.retire_calls = 0

    def active_generation(self) -> int:
        return self.generation

    def retire_exact(self, generation: int) -> None:
        self.retire_calls += 1
        raise base.ProtocolError("forced producer stop/drain failure")


class ProducerRollbackBarrierTests(unittest.TestCase):
    def test_failed_producer_rollback_retains_suppression_and_prepared_state(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            bridge_sock, fake_vnc = socket.socketpair()
            bridge = H1Cp2pRfbPiBridge(
                bridge_sock,
                queue_capacity=1024 * 1024,
                max_payload=8192,
                send_data=lambda payload: None,
                desktop_width=640,
                desktop_height=448,
            )
            session = SimpleNamespace(
                profile={
                    "session_id": 0x12345678,
                    "desktop_width": 640,
                    "desktop_height": 448,
                },
                evidence=Path(temp_dir),
                display=":0",
            )
            receiver = H1Cp2pSuppressionStartReceiver(session, bridge)
            frame = make_start_frame(
                session_id=session.profile["session_id"],
                generation=17,
            )

            try:
                receiver._handle_start_frame(frame)
                self.assertEqual(receiver.peek_prepared().generation, 17)
                self.assertEqual(receiver.capture_plan.generation, 17)
                self.assertEqual(bridge.suppression_generation, 17)

                failing = FailingRollbackProducer(17)
                receiver.producer = failing
                with self.assertRaisesRegex(
                    base.ProtocolError,
                    "suppression retained fail-closed",
                ):
                    receiver._rollback_compound_preparation(
                        17,
                        suppression_installed=True,
                    )

                self.assertEqual(failing.retire_calls, 1)
                self.assertEqual(receiver.peek_prepared().generation, 17)
                self.assertEqual(receiver.capture_plan.generation, 17)
                self.assertEqual(bridge.suppression_generation, 17)
                pending, active, rect = bridge._suppression_state()
                self.assertTrue(pending)
                self.assertFalse(active)
                self.assertIsNotNone(rect)
                self.assertTrue((Path(temp_dir) / "mpeg_start_prepared.json").exists())
                self.assertTrue((Path(temp_dir) / "rfb_suppression_prepared.json").exists())
                self.assertTrue((Path(temp_dir) / "mpeg_capture_prepared.json").exists())
            finally:
                bridge.stop()
                fake_vnc.close()


if __name__ == "__main__":
    unittest.main(verbosity=2)
