#!/usr/bin/env python3
"""File synopsis:
Defines the narrow ordinary Pi AUDIO product-composition profile and exact-
session producer factory for R42.

The profile reuses the generated R36 PCM projection and adds only the
Foreman-adopted 2.0-second capture-retirement escalation horizon recovered from
forensic H1. The horizon is not dormancy or success authority: the accepted R39
AudioPcmProducer still requires actual process exit and reader-thread dormancy.

This module owns no Wire I/O, session identity allocation, capture mechanism,
credit accounting, retry policy, or protocol/version behavior.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A006-ORDINARY-CROSS-PLATFORM-AUDIO-COMPOSITION-R42.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable

import audio_pcm_producer as audio


AUDIO_RETIREMENT_ESCALATION_SECONDS = 2.0


@dataclass(frozen=True)
class AudioProductCompositionProfile:
    """Immutable ordinary-product values consumed by one R39 factory."""

    pcm: audio.AudioPcmProfile
    retirement_timeout_seconds: float

    def __post_init__(self) -> None:
        if self.retirement_timeout_seconds <= 0:
            raise ValueError("AUDIO retirement escalation horizon must be positive")


def selected_audio_product_profile() -> AudioProductCompositionProfile:
    """Project accepted R36 PCM authority plus the R42 cleanup horizon."""

    return AudioProductCompositionProfile(
        pcm=audio.selected_audio_pcm_profile(),
        retirement_timeout_seconds=AUDIO_RETIREMENT_ESCALATION_SECONDS,
    )


def make_audio_pcm_producer(
    session_id: int,
    profile: AudioProductCompositionProfile,
) -> audio.AudioPcmProducer:
    """Create one fresh R39 owner bound to one exact accepted Wire Session."""

    if not isinstance(profile, AudioProductCompositionProfile):
        raise ValueError("AUDIO product profile must be selected composition authority")

    return audio.AudioPcmProducer(
        session_id=session_id,
        retirement_timeout_seconds=profile.retirement_timeout_seconds,
        profile=profile.pcm,
    )


def selected_audio_pcm_factory() -> Callable[[int], audio.AudioPcmProducer]:
    """Return the one lazy ordinary-product factory retained by WireServer."""

    profile = selected_audio_product_profile()

    def make_audio(session_id: int) -> audio.AudioPcmProducer:
        return make_audio_pcm_producer(session_id, profile)

    return make_audio
