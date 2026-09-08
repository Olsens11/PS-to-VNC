#!/usr/bin/env python3
"""
File synopsis:
    Named Pi-side CONFIG v4 profiles for the resident H1 media harness.

The first two profiles intentionally share the qualified P11 video mechanisms.
P11_COMPAT_VIDEO_ONLY proves the resident/mux/config machinery with audio
disabled. P11_COMPAT_PLUS_PCM adds the known 48 kHz / 16-bit / stereo PCM path
on the same physical PSTV mux.

CONFIG v4 adds explicit RFB queue/credit fields so channel-1 experiments can use
the same session-level control philosophy as AUDIO and MPEG. RFB is still OFF in
the current qualified media/census profiles until live channel-1 dispatch and
the Pi bridge are independently build-verified.

The Pi also retains active desktop and X11 capture geometry as test-harness
metadata. Neither is duplicated onto the H1 media wire because only the Pi
capture side consumes them today. 704x462 is therefore merely the current
qualified Issue-39 desktop default, not a permanent architectural maximum.
Future safe-area/display work can change desktop_width/desktop_height while the
same MPEG x/y/width/height contract continues to operate in active desktop
coordinates.

Presentation offsets are signed microseconds on the Pi and encoded as raw
32-bit two's-complement field values on the wire. Queue sizes and other
experimental values are not clamped to guessed safe ranges.
"""

from __future__ import annotations

import copy
import struct
from typing import Mapping

CONFIG_VERSION = 4
CONFIG_FIELD_COUNT = 61
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
    "rfb_mode": 54,
    "video_encode_width": 55,
    "video_encode_height": 56,
    "rfb_queue_capacity": 57,
    "rfb_credit_batch_bytes": 58,
    "rfb_credit_flush_on_empty": 59,
    "rfb_credit_return_enabled": 60,
    "rfb_initial_credit_bytes": 61,
}

# Pi-owned session/capture parameters are recorded in profiles/evidence but are
# not serialized to the PS2 H1 media transport.
PI_ONLY_FIELDS = {
    "desktop_width",
    "desktop_height",
    "video_capture_x",
    "video_capture_y",
    "video_capture_width",
    "video_capture_height",
}

SIGNED_FIELDS = {
    "audio_presentation_offset_us",
    "video_presentation_offset_us",
}

AUDIO_OFF = 0
AUDIO_PCM = 1
VIDEO_OFF = 0
VIDEO_MPEG2_ES = 1
RFB_OFF = 0
RFB_ON_RESERVED = 1
AUDIO_START_IMMEDIATE = 0
AUDIO_START_TARGET = 1
VIDEO_SCHED_ABSOLUTE = 0
VIDEO_RGB16 = 0
ALLOCATE_AUDIO_FIRST = 0
ALLOCATE_MPEG_FIRST = 1

CURRENT_DESKTOP_WIDTH = 704
CURRENT_DESKTOP_HEIGHT = 462


def _p11_compat_video_only() -> dict[str, int]:
    """Return the P11-compatible H1 video profile with stage colors disabled."""

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
        "video_encode_width": 608,
        "video_encode_height": 416,
        "video_draw_width": 640,
        "video_draw_height": 512,
        "video_draw_x": 0,
        "video_draw_y": 0,
        # Current qualified desktop; intentionally session metadata rather than
        # a permanent platform constant.
        "desktop_width": CURRENT_DESKTOP_WIDTH,
        "desktop_height": CURRENT_DESKTOP_HEIGHT,
        # Historical P11/H1 capture remains full-desktop then scaled. Future
        # cumulative profiles can set these equal to the MPEG presentation rect.
        "video_capture_x": 0,
        "video_capture_y": 0,
        "video_capture_width": CURRENT_DESKTOP_WIDTH,
        "video_capture_height": CURRENT_DESKTOP_HEIGHT,
        # Historical H1 diagnostic color cycles remain available by override,
        # but ordinary video/media qualification no longer displays them.
        "video_stage_markers": 0,
        "video_stage_hold_vsyncs": 0,
        "video_ipu_reset_each_session": 1,
        "video_drop_enabled": 0,
        "video_drop_threshold_milliframes": 0,
        "receiver_thread_priority": 63,
        "receiver_thread_stack_size": 16384,
        "max_data_payload": 8192,
        "socket_receive_buffer_bytes": 0,
        "socket_send_buffer_bytes": 0,
        "queue_allocation_order": ALLOCATE_MPEG_FIRST,
        "media_epoch_lead_us": 0,
        "rfb_mode": RFB_OFF,
        # RFB-OFF is truly inert in the current qualified media profiles.
        "rfb_queue_capacity": 0,
        "rfb_credit_batch_bytes": 0,
        "rfb_credit_flush_on_empty": 0,
        "rfb_credit_return_enabled": 0,
        "rfb_initial_credit_bytes": 0,
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
            # Historical EXP2 authority; priority experiments override this.
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
        allowed = set(FIELD_IDS) | PI_ONLY_FIELDS | {"profile_id"}
        unknown = set(overrides) - allowed
        if unknown:
            raise ValueError(f"unknown H1 override fields: {sorted(unknown)}")
        for key, value in overrides.items():
            profile[key] = int(value)

    validate_profile_shape(profile)
    return profile


def validate_profile_shape(profile: Mapping[str, int]) -> None:
    expected = set(FIELD_IDS) | PI_ONLY_FIELDS | {"profile_id"}
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

    desktop_width = int(profile["desktop_width"])
    desktop_height = int(profile["desktop_height"])
    capture_x = int(profile["video_capture_x"])
    capture_y = int(profile["video_capture_y"])
    capture_width = int(profile["video_capture_width"])
    capture_height = int(profile["video_capture_height"])

    if desktop_width <= 0 or desktop_height <= 0:
        raise ValueError("active desktop geometry must be non-empty")
    if capture_width <= 0 or capture_height <= 0:
        raise ValueError("video capture rectangle must be non-empty")
    if capture_x + capture_width > desktop_width:
        raise ValueError("video capture rectangle exceeds active desktop width")
    if capture_y + capture_height > desktop_height:
        raise ValueError("video capture rectangle exceeds active desktop height")


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
    assert video["video_encode_width"] == 608
    assert video["video_encode_height"] == 416
    assert video["video_draw_width"] == 640
    assert video["video_draw_height"] == 512
    assert video["desktop_width"] == CURRENT_DESKTOP_WIDTH
    assert video["desktop_height"] == CURRENT_DESKTOP_HEIGHT
    assert video["video_capture_width"] == CURRENT_DESKTOP_WIDTH
    assert video["video_capture_height"] == CURRENT_DESKTOP_HEIGHT
    assert video["video_stage_markers"] == 0
    assert video["rfb_mode"] == RFB_OFF
    assert video["rfb_queue_capacity"] == 0
    assert video["rfb_credit_batch_bytes"] == 0
    assert video["rfb_credit_flush_on_empty"] == 0
    assert video["rfb_credit_return_enabled"] == 0
    assert video["rfb_initial_credit_bytes"] == 0
    assert video["media_epoch_lead_us"] == 0
    assert video["audio_mode"] == AUDIO_OFF

    assert combined["audio_mode"] == AUDIO_PCM
    assert combined["audio_rate"] == 48000
    assert combined["audio_channels"] == 2
    assert combined["audio_bits"] == 16
    assert combined["audio_chunk_bytes"] == 4096

    # The operator-selected future RFB policy can already be represented by the
    # profile vocabulary even while the PS2 activation gate remains closed.
    rfb_candidate = resolve_profile(
        "P11_COMPAT_VIDEO_ONLY",
        3,
        {
            "rfb_mode": RFB_ON_RESERVED,
            "rfb_queue_capacity": 32768,
            "rfb_credit_batch_bytes": 8192,
            "rfb_credit_flush_on_empty": 1,
            "rfb_credit_return_enabled": 1,
            "rfb_initial_credit_bytes": 32768,
        },
    )
    assert rfb_candidate["rfb_queue_capacity"] == 32768
    assert rfb_candidate["rfb_credit_batch_bytes"] == 8192
    assert rfb_candidate["rfb_initial_credit_bytes"] == 32768

    # Same-location future composition example within today's desktop geometry.
    same_location = resolve_profile(
        "P11_COMPAT_PLUS_PCM",
        4,
        {
            "video_capture_x": 160,
            "video_capture_y": 32,
            "video_capture_width": 320,
            "video_capture_height": 240,
            "video_encode_width": 320,
            "video_encode_height": 240,
            "video_draw_x": 160,
            "video_draw_y": 32,
            "video_draw_width": 320,
            "video_draw_height": 240,
        },
    )
    assert same_location["video_capture_x"] == same_location["video_draw_x"]
    assert same_location["video_capture_y"] == same_location["video_draw_y"]
    assert same_location["video_capture_width"] == same_location["video_draw_width"]
    assert same_location["video_capture_height"] == same_location["video_draw_height"]

    shifted = resolve_profile(
        "P11_COMPAT_PLUS_PCM",
        5,
        {"audio_presentation_offset_us": -43000},
    )
    payload = build_config_payload(shifted)
    assert len(payload) == CONFIG_PAYLOAD_BYTES
    assert payload[-40:] == struct.pack(
        ">IIIIIIIIII",
        57,
        0,
        58,
        0,
        59,
        0,
        60,
        0,
        61,
        0,
    )

    print("H1_PI_PROFILES_SELF_TEST=PASS")


if __name__ == "__main__":
    self_test()
