#!/usr/bin/env python3
"""
File synopsis:
    Pi-side runner for the cumulative through-Issue-39 H1 thread-census ELF.

This composes three already-separated diagnostic concerns without changing the
physical transport or media scheduler:

    1. the qualified joint-prefill 1:1 AUDIO/MPEG scheduler;
    2. EE-thread census decoding through existing telemetry;
    3. profile-driven active-desktop and X11 MPEG capture geometry.

The capture rectangle is Pi-owned test metadata. The MPEG encoder receives only
that desktop rectangle, then scales it to the configured 16x16-grid encode
size. Future cumulative RFB+MPEG profiles can set capture x/y/width/height equal
to video_draw_x/y/width/height so the MPEG patch is captured from and presented
at the same logical desktop location.

The active desktop size itself is profile metadata rather than a hard platform
constant. Today's qualified default is 704x462; future safe-area/display work
can change desktop_width/desktop_height without changing this capture contract.

Every decoded census sample is accumulated by EE thread ID. At session exit the
runner prints one stable sorted line per observed thread, including known H1
role, initial/current priorities, observed scheduler states, and sample count.
"""

from __future__ import annotations

import json
from typing import Any

import h1_mux_server_thread_census_diag as census

base = census.base


def _video_command_profiled(self: base.H1Session) -> list[str]:
    desktop_width = int(self.profile["desktop_width"])
    desktop_height = int(self.profile["desktop_height"])
    capture_x = int(self.profile["video_capture_x"])
    capture_y = int(self.profile["video_capture_y"])
    capture_width = int(self.profile["video_capture_width"])
    capture_height = int(self.profile["video_capture_height"])
    encode_width = int(self.profile["video_encode_width"])
    encode_height = int(self.profile["video_encode_height"])

    if desktop_width <= 0 or desktop_height <= 0:
        raise base.ProtocolError("active desktop geometry must be non-empty")
    if capture_width <= 0 or capture_height <= 0:
        raise base.ProtocolError("MPEG capture rectangle must be non-empty")
    if capture_x < 0 or capture_y < 0:
        raise base.ProtocolError("MPEG capture origin must be non-negative")
    if capture_x + capture_width > desktop_width:
        raise base.ProtocolError("MPEG capture rectangle exceeds desktop width")
    if capture_y + capture_height > desktop_height:
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
                "desktop_width": desktop_width,
                "desktop_height": desktop_height,
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
        f"{desktop_width}x{desktop_height}",
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

_original_parse_telemetry = base.H1Session._parse_telemetry
_original_run = base.H1Session.run


def _parse_telemetry_collecting(
    self: base.H1Session,
    payload: bytes,
) -> dict[str, Any]:
    result = _original_parse_telemetry(self, payload)

    thread_id = result.get("census_thread_id")
    if thread_id is None:
        return result

    records = getattr(self, "cumulative39_census_records", None)
    if records is None:
        records = {}
        self.cumulative39_census_records = records

    thread_id = int(thread_id)
    record = records.get(thread_id)
    if record is None:
        record = {
            "role": str(result.get("census_role", "OTHER")),
            "initial_priorities": set(),
            "current_priorities": set(),
            "states": set(),
            "samples": 0,
        }
        records[thread_id] = record

    role = str(result.get("census_role", "OTHER"))
    if role != "OTHER":
        record["role"] = role

    record["initial_priorities"].add(int(result["census_initial_priority"]))
    record["current_priorities"].add(int(result["census_current_priority"]))
    record["states"].add(str(result["census_status_name"]))
    record["samples"] += 1
    return result


def _print_census_summary(self: base.H1Session) -> None:
    records = getattr(self, "cumulative39_census_records", {})
    print("H1_CUMULATIVE39_CENSUS_BEGIN", flush=True)

    for thread_id in sorted(records):
        record = records[thread_id]
        initial = ",".join(str(value) for value in sorted(record["initial_priorities"]))
        current = ",".join(str(value) for value in sorted(record["current_priorities"]))
        states = ",".join(sorted(record["states"]))
        print(
            "H1_CENSUS_THREAD "
            f"id={thread_id} "
            f"role={record['role']} "
            f"initial_priority={initial} "
            f"current_priority={current} "
            f"states={states} "
            f"samples={record['samples']}",
            flush=True,
        )

    print(
        f"H1_CUMULATIVE39_CENSUS_COUNT={len(records)}",
        flush=True,
    )
    print("H1_CUMULATIVE39_CENSUS_END", flush=True)


def _run_with_census_summary(self: base.H1Session) -> None:
    self.cumulative39_census_records = {}
    try:
        return _original_run(self)
    finally:
        _print_census_summary(self)


base.H1Session._parse_telemetry = _parse_telemetry_collecting
base.H1Session.run = _run_with_census_summary


if __name__ == "__main__":
    raise SystemExit(base.main())
