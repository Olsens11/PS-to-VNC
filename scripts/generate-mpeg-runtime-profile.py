#!/usr/bin/env python3
"""Generate deterministic owner projections from the canonical MPEG profile.

The selected MPEG runtime values live only in
src/config/mpeg_runtime_profile.json. This tool renders checked-in C and Python
projections and fails closed when either generated artifact is stale. It does
not start MPEG, establish Wire, launch FFmpeg, or own a generation lifecycle.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-MPEG-CONTROL-PRODUCER-R17.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATH = Path("src/config/mpeg_runtime_profile.json")
C_OUTPUT_PATH = Path("src/config/mpeg_runtime_profile_generated.h")
PY_OUTPUT_PATH = Path("pi/mpeg_runtime_profile_generated.py")

_TOP_LEVEL_KEYS = {
    "schema_version", "profile_id", "transport", "decoder", "worker",
    "ps2_worker_runtime", "scheduler", "provenance",
}
_TRANSPORT_KEYS = {
    "queue_capacity", "initial_credit_bytes", "credit_batch_bytes",
    "credit_flush_on_empty", "credit_return_enabled",
}
_DECODER_KEYS = {
    "max_width", "max_height", "bytes_per_pixel", "feed_payload_capacity",
    "transfer_alignment", "buffer_alignment",
}
_WORKER_KEYS = {"worker_stack_bytes", "worker_priority"}
_PS2_WORKER_RUNTIME_KEYS = {"join_poll_delay_us", "join_poll_max_count"}
_SCHEDULER_KEYS = {
    "fps_numerator", "fps_denominator", "drop_enabled",
    "drop_threshold_milliframes",
}
_PROVENANCE_KEYS = {"source_stage", "qualification_scope"}


def _mapping(value, keys, name):
    if not isinstance(value, dict) or set(value) != keys:
        raise ValueError(f"{name} has an unexpected schema")
    return value


def _u32(mapping, name, allow_zero=False):
    value = mapping[name]
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError(f"{name} must be an integer")
    if value < 0 or value > 0xFFFFFFFF or (not allow_zero and value == 0):
        raise ValueError(f"{name} is outside the accepted uint32 range")
    return value


def load_profile(root):
    profile = json.loads((root / SOURCE_PATH).read_text(encoding="utf-8"))
    if not isinstance(profile, dict) or set(profile) != _TOP_LEVEL_KEYS:
        raise ValueError("canonical MPEG profile has an unexpected top-level schema")
    if profile["schema_version"] != 1 or profile["profile_id"] != "current-mpeg":
        raise ValueError("unsupported MPEG profile identity")

    transport = _mapping(profile["transport"], _TRANSPORT_KEYS, "transport")
    decoder = _mapping(profile["decoder"], _DECODER_KEYS, "decoder")
    worker = _mapping(profile["worker"], _WORKER_KEYS, "worker")
    ps2_worker = _mapping(
        profile["ps2_worker_runtime"], _PS2_WORKER_RUNTIME_KEYS, "ps2_worker_runtime"
    )
    scheduler = _mapping(profile["scheduler"], _SCHEDULER_KEYS, "scheduler")
    provenance = _mapping(profile["provenance"], _PROVENANCE_KEYS, "provenance")

    queue_capacity = _u32(transport, "queue_capacity")
    if _u32(transport, "initial_credit_bytes") > queue_capacity:
        raise ValueError("initial MPEG credit exceeds queue capacity")
    if _u32(transport, "credit_batch_bytes") > queue_capacity:
        raise ValueError("MPEG credit batch exceeds queue capacity")
    for name in ("credit_flush_on_empty", "credit_return_enabled"):
        if not isinstance(transport[name], bool):
            raise ValueError(f"{name} must be boolean")

    max_width = _u32(decoder, "max_width")
    max_height = _u32(decoder, "max_height")
    if max_width > 0xFFFF or max_height > 0xFFFF:
        raise ValueError("MPEG maximum geometry exceeds coordinate range")
    _u32(decoder, "bytes_per_pixel")
    _u32(decoder, "feed_payload_capacity")
    transfer = _u32(decoder, "transfer_alignment")
    buffer_alignment = _u32(decoder, "buffer_alignment")
    if transfer & (transfer - 1) or buffer_alignment & (buffer_alignment - 1):
        raise ValueError("MPEG alignments must be powers of two")

    _u32(worker, "worker_stack_bytes")
    if _u32(worker, "worker_priority") > 127:
        raise ValueError("MPEG worker priority exceeds clean bound")
    _u32(ps2_worker, "join_poll_delay_us")
    _u32(ps2_worker, "join_poll_max_count")
    _u32(scheduler, "fps_numerator")
    _u32(scheduler, "fps_denominator")
    if not isinstance(scheduler["drop_enabled"], bool):
        raise ValueError("drop_enabled must be boolean")
    _u32(scheduler, "drop_threshold_milliframes", allow_zero=True)

    if provenance["source_stage"] != "A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7":
        raise ValueError("MPEG profile lost A003-R7 provenance")
    if not isinstance(provenance["qualification_scope"], str):
        raise ValueError("qualification_scope must be text")
    return profile


def render_c(profile):
    t = profile["transport"]
    d = profile["decoder"]
    w = profile["worker"]
    p = profile["ps2_worker_runtime"]
    s = profile["scheduler"]
    return """/*
 * GENERATED FILE - DO NOT EDIT.
 * Source: src/config/mpeg_runtime_profile.json
 * Generator: scripts/generate-mpeg-runtime-profile.py
 *
 * These are Configuration-selected mechanism values only. They do not create
 * Wire authority, launch a producer, or activate an MPEG generation.
 */

#ifndef PSTVNC_CONFIG_MPEG_RUNTIME_PROFILE_GENERATED_H
#define PSTVNC_CONFIG_MPEG_RUNTIME_PROFILE_GENERATED_H

#include <stdint.h>

#define PSTVNC_CONFIG_MPEG_QUEUE_CAPACITY UINT32_C({queue})
#define PSTVNC_CONFIG_MPEG_INITIAL_CREDIT_BYTES UINT32_C({initial})
#define PSTVNC_CONFIG_MPEG_CREDIT_BATCH_BYTES UINT32_C({batch})
#define PSTVNC_CONFIG_MPEG_CREDIT_FLUSH_ON_EMPTY {flush}
#define PSTVNC_CONFIG_MPEG_CREDIT_RETURN_ENABLED {returns}

#define PSTVNC_CONFIG_MPEG_MAX_WIDTH UINT32_C({max_width})
#define PSTVNC_CONFIG_MPEG_MAX_HEIGHT UINT32_C({max_height})
#define PSTVNC_CONFIG_MPEG_BYTES_PER_PIXEL UINT32_C({bpp})
#define PSTVNC_CONFIG_MPEG_FEED_PAYLOAD_CAPACITY UINT32_C({feed})
#define PSTVNC_CONFIG_MPEG_TRANSFER_ALIGNMENT UINT32_C({transfer})
#define PSTVNC_CONFIG_MPEG_BUFFER_ALIGNMENT UINT32_C({buffer_alignment})

#define PSTVNC_CONFIG_MPEG_WORKER_STACK_BYTES UINT32_C({worker_stack})
#define PSTVNC_CONFIG_MPEG_WORKER_PRIORITY {worker_priority}
#define PSTVNC_CONFIG_MPEG_JOIN_POLL_DELAY_US UINT32_C({join_delay})
#define PSTVNC_CONFIG_MPEG_JOIN_POLL_MAX_COUNT UINT32_C({join_count})

#define PSTVNC_CONFIG_MPEG_FPS_NUMERATOR UINT32_C({fps_num})
#define PSTVNC_CONFIG_MPEG_FPS_DENOMINATOR UINT32_C({fps_den})
#define PSTVNC_CONFIG_MPEG_DROP_ENABLED {drop}
#define PSTVNC_CONFIG_MPEG_DROP_THRESHOLD_MILLIFRAMES UINT32_C({drop_threshold})

#endif /* PSTVNC_CONFIG_MPEG_RUNTIME_PROFILE_GENERATED_H */
""".format(
        queue=t["queue_capacity"],
        initial=t["initial_credit_bytes"],
        batch=t["credit_batch_bytes"],
        flush=1 if t["credit_flush_on_empty"] else 0,
        returns=1 if t["credit_return_enabled"] else 0,
        max_width=d["max_width"],
        max_height=d["max_height"],
        bpp=d["bytes_per_pixel"],
        feed=d["feed_payload_capacity"],
        transfer=d["transfer_alignment"],
        buffer_alignment=d["buffer_alignment"],
        worker_stack=w["worker_stack_bytes"],
        worker_priority=w["worker_priority"],
        join_delay=p["join_poll_delay_us"],
        join_count=p["join_poll_max_count"],
        fps_num=s["fps_numerator"],
        fps_den=s["fps_denominator"],
        drop=1 if s["drop_enabled"] else 0,
        drop_threshold=s["drop_threshold_milliframes"],
    )


def render_python(profile):
    t = profile["transport"]
    d = profile["decoder"]
    s = profile["scheduler"]
    return '''"""GENERATED FILE - DO NOT EDIT.

Source: src/config/mpeg_runtime_profile.json
Generator: scripts/generate-mpeg-runtime-profile.py

This is the narrow Pi producer projection: bounded local producer buffering,
accepted capture maximums, and the selected MPEG presentation frame rate. Wire
credit remains dynamic protocol authority and is not manufactured here.
"""

MPEG_PRODUCER_BUFFER_BYTES = {queue}
MPEG_MAX_WIDTH = {max_width}
MPEG_MAX_HEIGHT = {max_height}
MPEG_FPS_NUMERATOR = {fps_num}
MPEG_FPS_DENOMINATOR = {fps_den}
'''.format(
        queue=t["queue_capacity"],
        max_width=d["max_width"],
        max_height=d["max_height"],
        fps_num=s["fps_numerator"],
        fps_den=s["fps_denominator"],
    )


def expected_outputs(root):
    profile = load_profile(root)
    return {
        C_OUTPUT_PATH: render_c(profile),
        PY_OUTPUT_PATH: render_python(profile),
    }


def check_outputs(root):
    ok = True
    for relative_path, expected in expected_outputs(root).items():
        path = root / relative_path
        try:
            actual = path.read_text(encoding="utf-8")
        except FileNotFoundError:
            print(f"MPEG_PROFILE_GENERATED_MISSING={relative_path}", file=sys.stderr)
            ok = False
            continue
        if actual != expected:
            print(f"MPEG_PROFILE_GENERATED_STALE={relative_path}", file=sys.stderr)
            ok = False
    if ok:
        print("MPEG_RUNTIME_PROFILE_GENERATED=PASS")
    return ok


def write_outputs(root):
    for relative_path, content in expected_outputs(root).items():
        path = root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8", newline="\n")
        print(f"MPEG_PROFILE_GENERATED_WRITE={relative_path}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--root", type=Path, default=REPOSITORY_ROOT)
    args = parser.parse_args()
    root = args.root.resolve()
    try:
        if args.check:
            return 0 if check_outputs(root) else 1
        write_outputs(root)
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"MPEG_RUNTIME_PROFILE_GENERATION_ERROR={exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
