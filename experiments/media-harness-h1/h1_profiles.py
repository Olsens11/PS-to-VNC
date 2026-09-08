#!/usr/bin/env python3
"""
File synopsis:
    Named Pi-side CONFIG v2 profiles for the resident H1 media harness.

The first two profiles intentionally share the exact qualified P11 video
mechanisms. P11_COMPAT_VIDEO_ONLY proves the new resident/mux/config machinery
with audio disabled. P11_COMPAT_PLUS_PCM changes only the addition of the
known 48 kHz / 16-bit / stereo PCM audio path on the same physical PSTV mux.

Presentation offsets are signed microseconds on the Pi and encoded as raw
32-bit two's-complement field values on the wire. Queue sizes and other
experimental values are not clamped to guessed safe ranges.
"""

from __future__ import annotations

import copy
import struct
from typing import Mapping

CONFIG_VERSION = 2
CONFIG_FIELD_COUNT = 53
CONFIG_PAYLOAD_BYTES = 8 + CONFIG_FIELD_COUNT * 8

FIELD_IDS = {
    "session_id": 1,
    "audio_mode": 2,
    "video_mode": 3,
    "audio_queue_capacity": 4,
    "mpeg_queue_capacity": 5,
    "audio_credit_batch_bytes": 6,
    "mpeg_credit_batch_bytes": 7,
    "audio_credit_flush_on_empty": 8,
    "mpeg_credit_flush_on_empty": 9,
    "audio_credit_return_enabled": 10,
    "mpeg_credit_return_enabled": 11,
    "audio_initial_credit_bytes": 12,
    "mpeg_initial_credit_bytes": 13,
    "audio_start_mode": 14,
    "audio_start_target_bytes": 15,
    "audio_start_delay_us": 16,
    "audio_chunk_bytes": 17,
    "audio_idle_delay_us": 18,
    "audio_thread_priority": 19,
    "audio_thread_stack_size": 20,
    "audio_rate": 21,
    "audio_channels": 22,
    "audio_bits": 23,
    "audio_volume": 24,
    "audio_presentation_offset_us": 25,
    "mpeg_start_target_bytes": 26,
    "mpeg_prefill_wait_us": 27,
    "mpeg_prefill_max_loops": 28,
    "mpeg_empty_delay_us": 29,
    "mpeg_feed_bytes": 30,
    "video_fps_num": 31,
    "video_fps_den": 32,
    "video_scheduler_mode": 33,
    "video_presentation_offset_us": 34,
    "video_pixel_mode": 35,
    "video_max_width": 36,
    "video_max_height": 37,
    "video_draw_width": 38,
    "video_draw_height": 39,
    "video_draw_x": 40,
    "video_draw_y": 41,
    "video_stage_markers": 42,
    "video_stage_hold_vsyncs": 43,
    "video_ipu_reset_each_session": 44,
    "video_drop_enabled": 45,
    "video_drop_threshold_milliframes": 46,
    "receiver_thread_priority": 47,
    "receiver_thread_stack_size": 48,
    "max_data_payload": 49,
    "socket_receive_buffer_bytes": 50,
    "socket_send_buffer_bytes": 51,
    "queue_allocation_order": 52,
    "media_epoch_lead_us": 53,
}

SIGNED_FIELDS = {
    "audio_presentation_offset_us",
    "video_presentation_offset_us",
}

AUDIO_OFF = 0
AUDIO_PCM = 1
VIDEO_OFF = 0
VIDEO_MPEG2_ES = 1
AUDIO_START_IMMEDIATE = 0
AUDIO_START_TARGET = 1
VIDEO_SCHED_ABSOLUTE = 0
VIDEO_RGB16 = 0
ALLOCATE_AUDIO_FIRST = 0
ALLOCATE_MPEG_FIRST = 1


def _p11_compat_video_only() -> dict[str, int]:
    """Return the exact effective P11-compatible H1 video profile."""

    return {
        "profile_id": 0,
        "session_id": 0,
        "audio_mode": AUDIO_OFF,
        "video_mode": VIDEO_MPEG2_ES,
        "audio_queue_capacity": 0,
        "mpeg_queue_capacity": 524288,
        "audio_credit_batch_bytes": 0,
        "mpeg_credit_batch_bytes": 8192,
        "audio_credit_flush_on_empty": 0,
        "mpeg_credit_flush_on_empty": 1,
        "audio_credit_return_enabled": 0,
        "mpeg_credit_return_enabled": 1,
        "audio_initial_credit_bytes": 0,
        "mpeg_initial_credit_bytes": 524288,
        "audio_start_mode": AUDIO_START_IMMEDIATE,
        "audio_start_target_bytes": 0,
        "audio_start_delay_us": 0,
        "audio_chunk_bytes": 0,
        "audio_idle_delay_us": 0,
        "audio_thread_priority": 0,
        "audio_thread_stack_size": 0,
        "audio_rate": 0,
        "audio_channels": 0,
        "audio_bits": 0,
        "audio_volume": 0,
        "audio_presentation_offset_us": 0,
        "mpeg_start_target_bytes": 458752,
        "mpeg_prefill_wait_us": 1000,
        "mpeg_prefill_max_loops": 10000,
        "mpeg_empty_delay_us": 1000,
        "mpeg_feed_bytes": 2048,
        "video_fps_num": 30000,
        "video_fps_den": 1001,
        "video_scheduler_mode": VIDEO_SCHED_ABSOLUTE,
        "video_presentation_offset_us": 0,
        "video_pixel_mode": VIDEO_RGB16,
        "video_max_width": 704,
        "video_max_height": 480,
        "video_draw_width": 640,
        "video_draw_height": 512,
        "video_draw_x": 0,
        "video_draw_y": 0,
        "video_stage_markers": 1,
        "video_stage_hold_vsyncs": 30,
        "video_ipu_reset_each_session": 1,
        "video_drop_enabled": 0,
        "video_drop_threshold_milliframes": 0,
        "receiver_thread_priority": 63,
        "receiver_thread_stack_size": 16384,
        "max_data_payload": 8192,
        "socket_receive_buffer_bytes": 0,
        "socket_send_buffer_bytes": 0,
        "queue_allocation_order": ALLOCATE_MPEG_FIRST,
        # P11 armed its epoch at the first real VSYNC with no artificial lead.
        "media_epoch_lead_us": 0,
    }


def _p11_compat_plus_pcm() -> dict[str, int]:
    profile = _p11_compat_video_only()
    profile.update(
        {
            "profile_id": 1,
            "audio_mode": AUDIO_PCM,
            "audio_queue_capacity": 524288,
            "audio_credit_batch_bytes": 4096,
            "audio_credit_flush_on_empty": 1,
            "audio_credit_return_enabled": 1,
            "audio_initial_credit_bytes": 524288,
            "audio_start_mode": AUDIO_START_TARGET,
            "audio_start_target_bytes": 458752,
            "audio_start_delay_us": 0,
            "audio_chunk_bytes": 4096,
            "audio_idle_delay_us": 1000,
            "audio_thread_priority": 65,
            "audio_thread_stack_size": 16384,
            "audio_rate": 48000,
            "audio_channels": 2,
            "audio_bits": 16,
            "audio_volume": 100,
            "audio_presentation_offset_us": 0,
            "queue_allocation_order": ALLOCATE_AUDIO_FIRST,
        }
    )
    return profile


PROFILES = {
    "P11_COMPAT_VIDEO_ONLY": _p11_compat_video_only(),
    "P11_COMPAT_PLUS_PCM": _p11_compat_plus_pcm(),
}


def profile_names() -> tuple[str, ...]:
    return tuple(PROFILES)


def resolve_profile(
    name: str,
    session_id: int,
    overrides: Mapping[str, int] | None = None,
) -> dict[str, int]:
    if name not in PROFILES:
        raise ValueError(f"unknown H1 profile {name!r}")

    profile = copy.deepcopy(PROFILES[name])
    profile["session_id"] = int(session_id)

    if overrides:
        unknown = set(overrides) - (set(FIELD_IDS) | {"profile_id"})
        if unknown:
            raise ValueError(f"unknown H1 override fields: {sorted(unknown)}")
        for key, value in overrides.items():
            profile[key] = int(value)

    validate_profile_shape(profile)
    return profile


def validate_profile_shape(profile: Mapping[str, int]) -> None:
    expected = set(FIELD_IDS) | {"profile_id"}
    actual = set(profile)

    if actual != expected:
        raise ValueError(
            "H1 profile key mismatch "
            f"missing={sorted(expected - actual)} "
            f"unknown={sorted(actual - expected)}"
        )

    for key, value in profile.items():
        value = int(value)
        if key in SIGNED_FIELDS:
            if value < -(1 << 31) or value > (1 << 31) - 1:
                raise ValueError(f"{key} is outside int32 range")
        elif value < 0 or value > 0xFFFFFFFF:
            raise ValueError(f"{key} is outside uint32 range")


def build_config_payload(profile: Mapping[str, int]) -> bytes:
    validate_profile_shape(profile)

    payload = bytearray(
        struct.pack(">II", CONFIG_VERSION, int(profile["profile_id"]))
    )

    for name, field_id in sorted(FIELD_IDS.items(), key=lambda item: item[1]):
        value = int(profile[name])
        raw = value & 0xFFFFFFFF
        payload.extend(struct.pack(">II", field_id, raw))

    result = bytes(payload)
    if len(result) != CONFIG_PAYLOAD_BYTES:
        raise AssertionError(
            f"H1 CONFIG payload length {len(result)} != {CONFIG_PAYLOAD_BYTES}"
        )
    return result


def self_test() -> None:
    video = resolve_profile("P11_COMPAT_VIDEO_ONLY", 1)
    combined = resolve_profile("P11_COMPAT_PLUS_PCM", 2)

    assert video["mpeg_queue_capacity"] == 524288
    assert video["mpeg_start_target_bytes"] == 458752
    assert video["mpeg_feed_bytes"] == 2048
    assert video["video_fps_num"] == 30000
    assert video["video_fps_den"] == 1001
    assert video["media_epoch_lead_us"] == 0
    assert video["audio_mode"] == AUDIO_OFF

    assert combined["audio_mode"] == AUDIO_PCM
    assert combined["audio_rate"] == 48000
    assert combined["audio_channels"] == 2
    assert combined["audio_bits"] == 16
    assert combined["audio_chunk_bytes"] == 4096

    shifted = resolve_profile(
        "P11_COMPAT_PLUS_PCM",
        3,
        {"audio_presentation_offset_us": -43000},
    )
    payload = build_config_payload(shifted)
    assert len(payload) == CONFIG_PAYLOAD_BYTES
    assert payload[-8:] == struct.pack(">II", 53, 0)

    print("H1_PI_PROFILES_SELF_TEST=PASS")


if __name__ == "__main__":
    self_test()
