#!/usr/bin/env python3
"""
File synopsis:
    Implements CP2P item #7's Pi-side exact capture-geometry preparation.

The current desktop width/height are used ONLY to validate that the PS2-accepted
START rectangle is inside the active source surface. Capture itself is the exact
START base rectangle: FFmpeg x11grab is opened at DISPLAY+X,Y with video_size
W x H. No startup-profile video_capture geometry, no crop-to-fit stage, and no
fixed desktop resolution is substituted.

This module prepares the command only. It deliberately does not launch FFmpeg or
emit MPEG bytes; producer activation remains item #8 and must occur only after
item #6 suppression has been installed for the same immutable generation.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass

import h1_mux_server as base


@dataclass(frozen=True)
class H1Cp2pCapturePlan:
    session_id: int
    generation: int
    desktop_width: int
    desktop_height: int
    capture_x: int
    capture_y: int
    capture_width: int
    capture_height: int
    display: str
    command: tuple[str, ...]

    def to_dict(self) -> dict[str, object]:
        result = asdict(self)
        result["command"] = list(self.command)
        return result


def prepare_exact_capture_plan(
    request,
    *,
    desktop_width: int,
    desktop_height: int,
    display: str,
) -> H1Cp2pCapturePlan:
    """Validate current surface bounds and prepare one exact-region MPEG command."""

    desktop_width = int(desktop_width)
    desktop_height = int(desktop_height)
    if desktop_width <= 0 or desktop_height <= 0:
        raise base.ProtocolError("CP2P capture requires positive active desktop geometry")
    if desktop_width > 0xFFFF or desktop_height > 0xFFFF:
        raise base.ProtocolError("CP2P capture desktop exceeds RFB 16-bit geometry")
    if not display:
        raise base.ProtocolError("CP2P capture requires a non-empty X11 display")

    session_id = int(request.session_id)
    generation = int(request.generation)
    x = int(request.draw_x)
    y = int(request.draw_y)
    width = int(request.draw_width)
    height = int(request.draw_height)

    if generation <= 0:
        raise base.ProtocolError("CP2P capture generation must be nonzero")
    if x < 0 or y < 0:
        raise base.ProtocolError("CP2P exact capture origin must be non-negative")
    if width < 16 or height < 16 or width % 16 != 0 or height % 16 != 0:
        raise base.ProtocolError(
            "CP2P exact capture dimensions must be >=16 and 16-pixel aligned"
        )
    if x + width > desktop_width or y + height > desktop_height:
        raise base.ProtocolError("CP2P exact capture rectangle exceeds current desktop")

    source = f"{display}+{x},{y}"
    source_size = f"{width}x{height}"

    command = (
        "ffmpeg",
        "-hide_banner",
        "-loglevel",
        "warning",
        "-f",
        "x11grab",
        "-framerate",
        base.VIDEO_RATE,
        "-video_size",
        source_size,
        "-i",
        source,
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
    )

    return H1Cp2pCapturePlan(
        session_id=session_id,
        generation=generation,
        desktop_width=desktop_width,
        desktop_height=desktop_height,
        capture_x=x,
        capture_y=y,
        capture_width=width,
        capture_height=height,
        display=display,
        command=command,
    )
