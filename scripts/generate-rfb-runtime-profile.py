#!/usr/bin/env python3
"""Generate deterministic owner projections from the canonical RFB profile.

The only selected numeric RFB runtime values live in
src/config/rfb_runtime_profile.json. This tool renders checked-in C and Python
projection constants and can fail closed when either generated artifact is
stale. It does not activate RFB, open sockets, create Wire sessions, or allocate
runtime resources.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-RFB-SHARED-RUNTIME-PROFILE-R14.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATH = Path("src/config/rfb_runtime_profile.json")
C_OUTPUT_PATH = Path("src/config/rfb_runtime_profile_generated.h")
PY_OUTPUT_PATH = Path("pi/rfb_runtime_profile_generated.py")

_TOP_LEVEL_KEYS = {
    "schema_version", "profile_id", "rfb_mode", "rfb_window_bytes",
    "rfb_credit_batch_bytes", "rfb_credit_flush_on_empty",
    "rfb_credit_return_enabled", "receiver_thread_priority",
    "receiver_thread_stack_size", "max_data_payload", "provenance",
}
_PROVENANCE_KEYS = {
    "baseline_checkpoint", "baseline_result", "starting_profile",
    "qualification_scope", "pi_provider_write_capacity_hardware_qualified",
}


def _positive_int(profile: dict[str, object], name: str) -> int:
    value = profile[name]
    if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
        raise ValueError(f"{name} must be a positive integer")
    if value > 0xFFFFFFFF:
        raise ValueError(f"{name} exceeds uint32")
    return value


def load_profile(root: Path) -> dict[str, object]:
    profile = json.loads((root / SOURCE_PATH).read_text(encoding="utf-8"))
    if not isinstance(profile, dict) or set(profile) != _TOP_LEVEL_KEYS:
        raise ValueError("canonical RFB profile has an unexpected top-level schema")
    if profile["schema_version"] != 1:
        raise ValueError("unsupported RFB profile schema version")
    if profile["profile_id"] != "current-rfb-only":
        raise ValueError("unexpected selected RFB profile identity")
    if profile["rfb_mode"] not in ("ON", "OFF"):
        raise ValueError("rfb_mode must be semantic ON or OFF")

    window = _positive_int(profile, "rfb_window_bytes")
    batch = _positive_int(profile, "rfb_credit_batch_bytes")
    stack = _positive_int(profile, "receiver_thread_stack_size")
    priority = _positive_int(profile, "receiver_thread_priority")
    payload = _positive_int(profile, "max_data_payload")
    if batch > window:
        raise ValueError("rfb_credit_batch_bytes exceeds rfb_window_bytes")
    if payload > window:
        raise ValueError("max_data_payload exceeds rfb_window_bytes")
    if stack % 16 != 0:
        raise ValueError("receiver_thread_stack_size must be 16-byte aligned")
    if priority > 127:
        raise ValueError("receiver_thread_priority exceeds clean profile bound")

    for name in ("rfb_credit_flush_on_empty", "rfb_credit_return_enabled"):
        if not isinstance(profile[name], bool):
            raise ValueError(f"{name} must be boolean")

    provenance = profile["provenance"]
    if not isinstance(provenance, dict) or set(provenance) != _PROVENANCE_KEYS:
        raise ValueError("canonical RFB provenance schema is incomplete")
    if provenance["baseline_checkpoint"] != "CP2N":
        raise ValueError("selected RFB baseline must retain CP2N provenance")
    if provenance["pi_provider_write_capacity_hardware_qualified"] is not False:
        raise ValueError("Pi provider-write projection must remain hardware-unqualified")
    return profile


def render_c(profile: dict[str, object]) -> str:
    enabled = 1 if profile["rfb_mode"] == "ON" else 0
    flush = 1 if profile["rfb_credit_flush_on_empty"] else 0
    credit_return = 1 if profile["rfb_credit_return_enabled"] else 0
    return f"""/*
 * GENERATED FILE - DO NOT EDIT.
 * Source: src/config/rfb_runtime_profile.json
 * Generator: scripts/generate-rfb-runtime-profile.py
 *
 * These are selected Configuration values only. They do not create a Wire
 * Session, activate RFB, or own runtime lifecycle.
 */

#ifndef PSTVNC_CONFIG_RFB_RUNTIME_PROFILE_GENERATED_H
#define PSTVNC_CONFIG_RFB_RUNTIME_PROFILE_GENERATED_H

#include <stdint.h>

#define PSTVNC_CONFIG_RFB_SELECTED_MODE_ON {enabled}
#define PSTVNC_CONFIG_RFB_WINDOW_BYTES UINT32_C({profile["rfb_window_bytes"]})
#define PSTVNC_CONFIG_RFB_CREDIT_BATCH_BYTES UINT32_C({profile["rfb_credit_batch_bytes"]})
#define PSTVNC_CONFIG_RFB_CREDIT_FLUSH_ON_EMPTY {flush}
#define PSTVNC_CONFIG_RFB_CREDIT_RETURN_ENABLED {credit_return}
#define PSTVNC_CONFIG_RFB_RECEIVER_THREAD_PRIORITY {profile["receiver_thread_priority"]}
#define PSTVNC_CONFIG_RFB_RECEIVER_THREAD_STACK_SIZE UINT32_C({profile["receiver_thread_stack_size"]})
#define PSTVNC_CONFIG_RFB_MAX_DATA_PAYLOAD UINT32_C({profile["max_data_payload"]})

#endif /* PSTVNC_CONFIG_RFB_RUNTIME_PROFILE_GENERATED_H */
"""


def render_python(profile: dict[str, object]) -> str:
    return f'''"""GENERATED FILE - DO NOT EDIT.

Source: src/config/rfb_runtime_profile.json
Generator: scripts/generate-rfb-runtime-profile.py

These are selected Configuration values only. They do not create a Wire
Session, activate RFB, or own runtime lifecycle.
"""

RFB_MODE = "{profile["rfb_mode"]}"
RFB_WINDOW_BYTES = {profile["rfb_window_bytes"]}
RFB_CREDIT_BATCH_BYTES = {profile["rfb_credit_batch_bytes"]}
RFB_CREDIT_FLUSH_ON_EMPTY = {profile["rfb_credit_flush_on_empty"]}
RFB_CREDIT_RETURN_ENABLED = {profile["rfb_credit_return_enabled"]}
RECEIVER_THREAD_PRIORITY = {profile["receiver_thread_priority"]}
RECEIVER_THREAD_STACK_SIZE = {profile["receiver_thread_stack_size"]}
MAX_DATA_PAYLOAD = {profile["max_data_payload"]}
'''


def expected_outputs(root: Path) -> dict[Path, str]:
    profile = load_profile(root)
    return {
        C_OUTPUT_PATH: render_c(profile),
        PY_OUTPUT_PATH: render_python(profile),
    }


def check_outputs(root: Path) -> bool:
    ok = True
    for relative_path, expected in expected_outputs(root).items():
        path = root / relative_path
        try:
            actual = path.read_text(encoding="utf-8")
        except FileNotFoundError:
            print(f"RFB_PROFILE_GENERATED_MISSING={relative_path}", file=sys.stderr)
            ok = False
            continue
        if actual != expected:
            print(f"RFB_PROFILE_GENERATED_STALE={relative_path}", file=sys.stderr)
            ok = False
    if ok:
        print("RFB_RUNTIME_PROFILE_GENERATED=PASS")
    return ok


def write_outputs(root: Path) -> None:
    for relative_path, content in expected_outputs(root).items():
        path = root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8", newline="\n")
        print(f"RFB_PROFILE_GENERATED_WRITE={relative_path}")


def main() -> int:
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
        print(f"RFB_RUNTIME_PROFILE_GENERATION_ERROR={exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
