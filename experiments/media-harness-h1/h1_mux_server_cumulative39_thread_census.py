#!/usr/bin/env python3
"""
File synopsis:
    Pi-side runner for the cumulative through-Issue-39 H1 thread-census ELF.

This composes three already-separated diagnostic concerns without changing the
physical transport or media scheduler:

    1. the qualified joint-prefill 1:1 AUDIO/MPEG scheduler;
    2. EE-thread census decoding through existing telemetry;
    3. profile-driven X11 MPEG capture geometry.

The capture rectangle is Pi-owned test metadata. The MPEG encoder receives only
that desktop rectangle, then scales it to the configured 16x16-grid encode
size. Future cumulative RFB+MPEG profiles can set capture x/y/width/height equal
to video_draw_x/y/width/height so the MPEG patch is captured from and presented
at the same logical desktop location.

Historical P11-compatible profiles remain reproducible because their default
capture rectangle is the complete 704x462 desktop and their encode rectangle is
608x416.
"""

from __future__ import annotations

import json

import h1_mux_server_thread_census_diag as census

base = census.base

VIDEO_SOURCE_WIDTH = 704
VIDEO_SOURCE_HEIGHT = 462
VIDEO_SOURCE_SIZE = f"{VIDEO_SOURCE_WIDTH}x{VIDEO_SOURCE_HEIGHT}"


def _video_command_profiled(self: base.H1Session) -> list[str]:
    capture_x = int(self.profile["video_capture_x"])
    capture_y = int(self.profile["video_capture_y"])
    capture_width = int(self.profile["video_capture_width"])
    capture_height = int(self.profile["video_capture_height"])
    encode_width = int(self.profile["video_encode_width"])
    encode_height = int(self.profile["video_encode_height"])

    if capture_width <= 0 or capture_height <= 0:
        raise base.ProtocolError("MPEG capture rectangle must be non-empty")
    if capture_x < 0 or capture_y < 0:
        raise base.ProtocolError("MPEG capture origin must be non-negative")
    if capture_x + capture_width > VIDEO_SOURCE_WIDTH:
        raise base.ProtocolError("MPEG capture rectangle exceeds desktop width")
    if capture_y + capture_height > VIDEO_SOURCE_HEIGHT:
        raise base.ProtocolError("MPEG capture rectangle exceeds desktop height")
    if encode_width <= 0 or encode_height <= 0:
        raise base.ProtocolError("MPEG encode rectangle must be non-empty")
    if encode_width % 16 != 0 or encode_height % 16 != 0:
        raise base.ProtocolError("MPEG encode dimensions must be 16-pixel aligned")

    video_filter = (
        f"crop={capture_width}:{capture_height}:{capture_x}:{capture_y},"
        f"scale={encode_width}:{encode_height}:flags=bicubic,"
        "format=yuv420p"
    )

    print(
        "H1_CUMULATIVE39_VIDEO_LAYOUT="
        + json.dumps(
            {
                "capture_x": capture_x,
                "capture_y": capture_y,
                "capture_width": capture_width,
                "capture_height": capture_height,
                "encode_width": encode_width,
                "encode_height": encode_height,
                "draw_x": int(self.profile["video_draw_x"]),
                "draw_y": int(self.profile["video_draw_y"]),
                "draw_width": int(self.profile["video_draw_width"]),
                "draw_height": int(self.profile["video_draw_height"]),
                "rfb_mode": int(self.profile["rfb_mode"]),
            },
            sort_keys=True,
        ),
        flush=True,
    )

    return [
        "ffmpeg",
        "-hide_banner",
        "-loglevel",
        "warning",
        "-f",
        "x11grab",
        "-framerate",
        base.VIDEO_RATE,
        "-video_size",
        VIDEO_SOURCE_SIZE,
        "-i",
        self.display,
        "-t",
        f"{self.duration:.6f}",
        "-vf",
        video_filter,
        "-an",
        "-c:v",
        "mpeg2video",
        "-profile:v",
        "main",
        "-level:v",
        "main",
        "-pix_fmt",
        "yuv420p",
        "-b:v",
        base.VIDEO_BITRATE,
        "-maxrate:v",
        base.VIDEO_MAXRATE,
        "-minrate:v",
        "0",
        "-bufsize:v",
        base.VIDEO_VBV_BITS,
        "-g",
        base.VIDEO_GOP,
        "-bf",
        "0",
        "-r",
        base.VIDEO_RATE,
        "-f",
        "mpeg2video",
        "pipe:1",
    ]


base.H1Session.video_command = _video_command_profiled


if __name__ == "__main__":
    raise SystemExit(base.main())
