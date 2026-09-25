#!/usr/bin/env python3
"""File synopsis:
Owns the selected ordinary Raspberry Pi MPEG product-composition inputs that are
outside the Configuration-owned R17 producer tuning profile.

R25 binds native display :0 and exact 704x462 clean desktop coordinates to the
R17 capture owner. The 8.0-second retirement deadline is recovered product
policy from frozen H1 forensic source
experiments/media-harness-h1/h1_cp2p_mpeg_producer.py at commit
3426f28b93de9519ca93e5f0e0aaf8b67cfca845, where
DEFAULT_RETIRE_TIMEOUT_SECONDS is exactly 8.0.

This module does not create a Wire Session, launch FFmpeg, own generation
identity, alter RFB provider state, or select R17 channel/buffer/frame-rate
tuning. Those mechanisms and values remain with their existing owners.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-MPEG-ORDINARY-PRODUCT-COMPOSITION-R25.
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class MpegProductCompositionProfile:
    display: str
    desktop_width: int
    desktop_height: int
    retirement_timeout_seconds: float


_SELECTED_DISPLAY = ":0"
_SELECTED_DESKTOP_WIDTH = 704
_SELECTED_DESKTOP_HEIGHT = 462
_SELECTED_RETIREMENT_TIMEOUT_SECONDS = 8.0


def selected_mpeg_product_profile() -> MpegProductCompositionProfile:
    """Return the one repository-selected Pi MPEG composition profile."""

    profile = MpegProductCompositionProfile(
        display=_SELECTED_DISPLAY,
        desktop_width=_SELECTED_DESKTOP_WIDTH,
        desktop_height=_SELECTED_DESKTOP_HEIGHT,
        retirement_timeout_seconds=_SELECTED_RETIREMENT_TIMEOUT_SECONDS,
    )
    if (
        not profile.display
        or profile.desktop_width <= 0
        or profile.desktop_height <= 0
        or profile.desktop_width > 0xFFFF
        or profile.desktop_height > 0xFFFF
        or profile.retirement_timeout_seconds <= 0.0
    ):
        raise ValueError("selected Pi MPEG product composition profile is invalid")
    return profile
