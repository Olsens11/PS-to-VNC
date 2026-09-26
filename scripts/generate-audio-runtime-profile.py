#!/usr/bin/env python3
"""Generate the deterministic selected clean AUDIO runtime profile constants.

The canonical values live only in src/config/audio_runtime_profile.json. This
tool validates that narrow owner projection and renders the checked-in C
constants. It performs no runtime CONFIG parsing or AUDIO lifecycle work.
"""

from __future__ import annotations
import argparse
import json
from pathlib import Path
import sys

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATH = Path("src/config/audio_runtime_profile.json")
OUTPUT_PATH = Path("src/config/audio_runtime_profile_generated.h")
PI_OUTPUT_PATH = Path("pi/audio_runtime_profile_generated.py")

TOP = {"schema_version","profile_id","transport","pcm","session","provenance"}
TRANSPORT = {"queue_capacity","initial_credit_bytes","credit_batch_bytes",
             "credit_flush_on_empty","credit_return_enabled"}
PCM = {"rate_hz","channels","bits_per_sample","volume_percent"}
SESSION = {"playback_buffer_capacity","startup_reservoir_bytes","worker_priority",
           "worker_stack_bytes","reservoir_poll_us","clock_poll_us"}
PROVENANCE = {"source_stage","forensic_commit","qualified_lineage"}


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


def validate_profile(profile):
    if not isinstance(profile, dict) or set(profile) != TOP:
        raise ValueError("canonical AUDIO profile has an unexpected top-level schema")
    if profile["schema_version"] != 1 or profile["profile_id"] != "current-audio":
        raise ValueError("unsupported AUDIO profile identity")

    t = _mapping(profile["transport"], TRANSPORT, "transport")
    p = _mapping(profile["pcm"], PCM, "pcm")
    s = _mapping(profile["session"], SESSION, "session")
    provenance = _mapping(profile["provenance"], PROVENANCE, "provenance")

    queue = _u32(t, "queue_capacity")
    initial = _u32(t, "initial_credit_bytes")
    batch = _u32(t, "credit_batch_bytes")
    if initial > queue or batch > queue:
        raise ValueError("AUDIO credit exceeds queue capacity")
    for name in ("credit_flush_on_empty", "credit_return_enabled"):
        if not isinstance(t[name], bool):
            raise ValueError(f"{name} must be boolean")
    if not t["credit_return_enabled"] and batch != 0:
        raise ValueError("disabled AUDIO credit return requires zero batch")

    rate = _u32(p, "rate_hz")
    if rate > 0x7FFFFFFF:
        raise ValueError("PCM rate exceeds clean signed API bound")
    if _u32(p, "channels") not in (1, 2):
        raise ValueError("PCM channels must be mono or stereo")
    if _u32(p, "bits_per_sample") not in (8, 16):
        raise ValueError("PCM sample width is unsupported")
    volume = _u32(p, "volume_percent", allow_zero=True)
    if volume > 100:
        raise ValueError("PCM volume exceeds 100 percent")

    playback = _u32(s, "playback_buffer_capacity")
    reservoir = _u32(s, "startup_reservoir_bytes")
    stack = _u32(s, "worker_stack_bytes")
    priority = _u32(s, "worker_priority")
    _u32(s, "reservoir_poll_us")
    _u32(s, "clock_poll_us")
    if playback > queue or reservoir > queue:
        raise ValueError("AUDIO session buffering exceeds Transport queue")
    if stack > 0x7FFFFFFF:
        raise ValueError("AUDIO worker stack exceeds clean signed API bound")
    if priority > 127:
        raise ValueError("AUDIO worker priority exceeds clean bound")

    if provenance["source_stage"] != "A002-AUDIO-RUNTIME-PROFILE-AUTHORITY-R36":
        raise ValueError("AUDIO profile lost R36 provenance")
    if provenance["forensic_commit"] != "3426f28b93de9519ca93e5f0e0aaf8b67cfca845":
        raise ValueError("AUDIO profile forensic authority changed")
    if provenance["qualified_lineage"] != "P11_COMPAT_PLUS_PCM":
        raise ValueError("AUDIO profile qualified lineage changed")
    return profile


def load_profile(root):
    profile = json.loads((root / SOURCE_PATH).read_text(encoding="utf-8"))
    return validate_profile(profile)


def render_c(profile):
    t, p, s = profile["transport"], profile["pcm"], profile["session"]
    return """/*
 * GENERATED FILE - DO NOT EDIT.
 * Source: src/config/audio_runtime_profile.json
 * Generator: scripts/generate-audio-runtime-profile.py
 *
 * Selected Configuration values only; no media-clock offset is duplicated
 * here and no AUDIO runtime mechanism is activated by these constants.
 */

#ifndef PSTVNC_CONFIG_AUDIO_RUNTIME_PROFILE_GENERATED_H
#define PSTVNC_CONFIG_AUDIO_RUNTIME_PROFILE_GENERATED_H

#include <stdint.h>

#define PSTVNC_CONFIG_AUDIO_QUEUE_CAPACITY UINT32_C({queue})
#define PSTVNC_CONFIG_AUDIO_INITIAL_CREDIT_BYTES UINT32_C({initial})
#define PSTVNC_CONFIG_AUDIO_CREDIT_BATCH_BYTES UINT32_C({batch})
#define PSTVNC_CONFIG_AUDIO_CREDIT_FLUSH_ON_EMPTY {flush}
#define PSTVNC_CONFIG_AUDIO_CREDIT_RETURN_ENABLED {returns}

#define PSTVNC_CONFIG_AUDIO_PCM_RATE_HZ UINT32_C({rate})
#define PSTVNC_CONFIG_AUDIO_PCM_CHANNELS UINT32_C({channels})
#define PSTVNC_CONFIG_AUDIO_PCM_BITS_PER_SAMPLE UINT32_C({bits})
#define PSTVNC_CONFIG_AUDIO_PCM_VOLUME_PERCENT UINT32_C({volume})

#define PSTVNC_CONFIG_AUDIO_PLAYBACK_BUFFER_CAPACITY UINT32_C({playback})
#define PSTVNC_CONFIG_AUDIO_STARTUP_RESERVOIR_BYTES UINT32_C({reservoir})
#define PSTVNC_CONFIG_AUDIO_WORKER_PRIORITY {priority}
#define PSTVNC_CONFIG_AUDIO_WORKER_STACK_BYTES UINT32_C({stack})
#define PSTVNC_CONFIG_AUDIO_RESERVOIR_POLL_US UINT32_C({reservoir_poll})
#define PSTVNC_CONFIG_AUDIO_CLOCK_POLL_US UINT32_C({clock_poll})

#endif /* PSTVNC_CONFIG_AUDIO_RUNTIME_PROFILE_GENERATED_H */
""".format(
        queue=t["queue_capacity"], initial=t["initial_credit_bytes"],
        batch=t["credit_batch_bytes"], flush=1 if t["credit_flush_on_empty"] else 0,
        returns=1 if t["credit_return_enabled"] else 0,
        rate=p["rate_hz"], channels=p["channels"], bits=p["bits_per_sample"],
        volume=p["volume_percent"], playback=s["playback_buffer_capacity"],
        reservoir=s["startup_reservoir_bytes"], priority=s["worker_priority"],
        stack=s["worker_stack_bytes"], reservoir_poll=s["reservoir_poll_us"],
        clock_poll=s["clock_poll_us"])


def render_pi(profile):
    t, p = profile["transport"], profile["pcm"]
    return """#!/usr/bin/env python3
# GENERATED FILE - DO NOT EDIT.
# Source: src/config/audio_runtime_profile.json
# Generator: scripts/generate-audio-runtime-profile.py
#
# Narrow Pi producer projection only. Ordinary product AUDIO activation remains
# outside this generated value authority.

CHANNEL_WINDOW_BYTES = {window}
PCM_RATE_HZ = {rate}
PCM_CHANNELS = {channels}
PCM_BITS_PER_SAMPLE = {bits}
PCM_FRAME_BYTES = PCM_CHANNELS * (PCM_BITS_PER_SAMPLE // 8)
""".format(
        window=t["queue_capacity"],
        rate=p["rate_hz"],
        channels=p["channels"],
        bits=p["bits_per_sample"],
    )


def expected_output(root):
    return render_c(load_profile(root))


def expected_pi_output(root):
    return render_pi(load_profile(root))


def _check_one_output(root, path, expected):
    target = root / path
    try:
        actual = target.read_text(encoding="utf-8")
    except FileNotFoundError:
        print(f"AUDIO_PROFILE_GENERATED_MISSING={path}", file=sys.stderr)
        return False
    if actual != expected:
        print(f"AUDIO_PROFILE_GENERATED_STALE={path}", file=sys.stderr)
        return False
    return True


def check_output(root):
    profile = load_profile(root)
    valid = _check_one_output(root, OUTPUT_PATH, render_c(profile))
    valid = _check_one_output(root, PI_OUTPUT_PATH, render_pi(profile)) and valid
    if valid:
        print("AUDIO_RUNTIME_PROFILE_GENERATED=PASS")
    return valid


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--root", type=Path, default=REPOSITORY_ROOT)
    args = parser.parse_args()
    root = args.root.resolve()
    try:
        if args.check:
            return 0 if check_output(root) else 1
        profile = load_profile(root)
        outputs = (
            (OUTPUT_PATH, render_c(profile)),
            (PI_OUTPUT_PATH, render_pi(profile)),
        )
        for output_path, content in outputs:
            path = root / output_path
            path.write_text(content, encoding="utf-8", newline="\n")
            print(f"AUDIO_PROFILE_GENERATED_WRITE={output_path}")
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"AUDIO_RUNTIME_PROFILE_GENERATION_ERROR={exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
