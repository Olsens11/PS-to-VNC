#!/usr/bin/env python3
"""File synopsis:
Projects Configuration's canonical MPEG profile onto the Pi producer boundary.

The selected numeric values are generated from
src/config/mpeg_runtime_profile.json. This module validates that narrow
projection and returns an immutable value for generation/capture ownership.
It does not establish Wire, create a generation, query desktop geometry, launch
FFmpeg, consume CREDIT, or send MPEG bytes.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-MPEG-CONTROL-PRODUCER-R17.
"""

from __future__ import annotations

from dataclasses import dataclass

import mpeg_runtime_profile_generated as generated


@dataclass(frozen=True)
class MpegProducerProfile:
    buffer_capacity: int
    max_width: int
    max_height: int
    fps_numerator: int
    fps_denominator: int


def selected_mpeg_producer_profile() -> MpegProducerProfile:
    """Return the one Configuration-owned Pi producer projection."""

    values = MpegProducerProfile(
        buffer_capacity=generated.MPEG_PRODUCER_BUFFER_BYTES,
        max_width=generated.MPEG_MAX_WIDTH,
        max_height=generated.MPEG_MAX_HEIGHT,
        fps_numerator=generated.MPEG_FPS_NUMERATOR,
        fps_denominator=generated.MPEG_FPS_DENOMINATOR,
    )
    if (
        values.buffer_capacity <= 0
        or values.max_width <= 0
        or values.max_height <= 0
        or values.max_width > 0xFFFF
        or values.max_height > 0xFFFF
        or values.fps_numerator <= 0
        or values.fps_denominator <= 0
    ):
        raise ValueError("selected MPEG producer profile is invalid")
    return values
