#!/usr/bin/env python3
"""Host tests for CP2P item #7 exact START-derived capture geometry."""

from __future__ import annotations

from dataclasses import dataclass, replace
import unittest

import h1_mux_server as base
from h1_cp2p_capture_geometry import prepare_exact_capture_plan


@dataclass(frozen=True)
class DummyStart:
    session_id: int = 0x12345678
    generation: int = 7
    draw_x: int = 32
    draw_y: int = 48
    draw_width: int = 320
    draw_height: int = 240


class ExactCaptureTests(unittest.TestCase):
    def test_ffmpeg_source_is_exact_start_rectangle_not_desktop(self) -> None:
        request = DummyStart()
        plan = prepare_exact_capture_plan(
            request,
            desktop_width=1280,
            desktop_height=720,
            display=":0.0",
        )

        self.assertEqual(
            (
                plan.capture_x,
                plan.capture_y,
                plan.capture_width,
                plan.capture_height,
            ),
            (32, 48, 320, 240),
        )
        self.assertEqual((plan.desktop_width, plan.desktop_height), (1280, 720))

        command = list(plan.command)
        video_size_index = command.index("-video_size")
        input_index = command.index("-i")
        self.assertEqual(command[video_size_index + 1], "320x240")
        self.assertEqual(command[input_index + 1], ":0.0+32,48")

        # Desktop geometry is a validation surface, never an encode/capture
        # substitute. The old full-desktop-grab/crop path must not return here.
        self.assertNotIn("1280x720", command)
        self.assertNotIn("-vf", command)
        self.assertFalse(any("crop=" in value for value in command))
        self.assertFalse(any("scale=" in value for value in command))

        # Item #8 owns lifetime; the prepared command must not inherit the old
        # finite diagnostic duration from the historical media harness.
        self.assertNotIn("-t", command)

    def test_same_start_geometry_is_independent_of_larger_current_desktop(self) -> None:
        request = DummyStart(draw_x=64, draw_y=64, draw_width=256, draw_height=192)
        commands = []
        for desktop in ((640, 448), (704, 462), (704, 480), (1280, 720)):
            plan = prepare_exact_capture_plan(
                request,
                desktop_width=desktop[0],
                desktop_height=desktop[1],
                display=":1",
            )
            commands.append(plan.command)
            self.assertEqual(
                (
                    plan.capture_x,
                    plan.capture_y,
                    plan.capture_width,
                    plan.capture_height,
                ),
                (64, 64, 256, 192),
            )

        self.assertTrue(all(command == commands[0] for command in commands[1:]))

    def test_bounds_are_checked_against_current_desktop(self) -> None:
        request = DummyStart(draw_x=400, draw_y=224, draw_width=256, draw_height=224)
        with self.assertRaises(base.ProtocolError):
            prepare_exact_capture_plan(
                request,
                desktop_width=640,
                desktop_height=448,
                display=":0",
            )

        plan = prepare_exact_capture_plan(
            request,
            desktop_width=704,
            desktop_height=480,
            display=":0",
        )
        self.assertEqual(plan.capture_x + plan.capture_width, 656)
        self.assertEqual(plan.capture_y + plan.capture_height, 448)

    def test_invalid_or_unaligned_start_geometry_fails_closed(self) -> None:
        for request in (
            replace(DummyStart(), generation=0),
            replace(DummyStart(), draw_x=-1),
            replace(DummyStart(), draw_width=319),
            replace(DummyStart(), draw_height=8),
        ):
            with self.assertRaises(base.ProtocolError):
                prepare_exact_capture_plan(
                    request,
                    desktop_width=704,
                    desktop_height=480,
                    display=":0",
                )

    def test_encoder_profile_is_reused_without_reusing_old_source_geometry(self) -> None:
        plan = prepare_exact_capture_plan(
            DummyStart(),
            desktop_width=704,
            desktop_height=480,
            display=":0",
        )
        command = list(plan.command)
        self.assertEqual(command[command.index("-framerate") + 1], base.VIDEO_RATE)
        self.assertEqual(command[command.index("-b:v") + 1], base.VIDEO_BITRATE)
        self.assertEqual(command[command.index("-maxrate:v") + 1], base.VIDEO_MAXRATE)
        self.assertEqual(command[command.index("-bufsize:v") + 1], base.VIDEO_VBV_BITS)
        self.assertNotIn(base.VIDEO_SOURCE_SIZE, command)
        self.assertFalse(any(base.VIDEO_FILTER == value for value in command))


if __name__ == "__main__":
    unittest.main(verbosity=2)
