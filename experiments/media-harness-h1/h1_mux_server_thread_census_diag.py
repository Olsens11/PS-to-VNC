#!/usr/bin/env python3
"""
File synopsis:
    Pi-side decoder for the H1 EE-thread census diagnostic ELF.

The PS2 diagnostic reuses the existing telemetry diagnostic_word while media is
active. This wrapper preserves the qualified joint-prefill 1:1 mux scheduler
and decorates telemetry with decoded EE thread ID, known H1 role, status,
initial priority, and current priority. No transport framing, queue policy,
media codec, or scheduler weight changes are made here.
"""

from __future__ import annotations

import h1_mux_server_joint_prefill_balanced_diag as balanced

base = balanced.base

CENSUS_MASK = 0xF0000000
CENSUS_ROLES = {
    0xC0000000: "OTHER",
    0xD0000000: "H1_AUDIO",
    0xE0000000: "H1_RECEIVER",
}

STATUS_NAMES = {
    0x01: "RUN",
    0x02: "READY",
    0x04: "WAIT",
    0x08: "SUSPEND",
    0x0C: "WAITSUSPEND",
    0x10: "DORMANT",
}

_original_parse_telemetry = base.H1Session._parse_telemetry


def _parse_telemetry_with_census(
    self: base.H1Session,
    payload: bytes,
) -> dict[str, int | str]:
    result = _original_parse_telemetry(self, payload)
    word = result["diagnostic_word"]
    marker = word & CENSUS_MASK

    if marker in CENSUS_ROLES:
        thread_id = (word >> 20) & 0xFF
        status = (word >> 14) & 0x3F
        initial_priority = (word >> 7) & 0x7F
        current_priority = word & 0x7F

        result["census_thread_id"] = thread_id
        result["census_role"] = CENSUS_ROLES[marker]
        result["census_status"] = status
        result["census_status_name"] = STATUS_NAMES.get(
            status,
            f"UNKNOWN_0x{status:02x}",
        )
        result["census_initial_priority"] = initial_priority
        result["census_current_priority"] = current_priority

    return result


base.H1Session._parse_telemetry = _parse_telemetry_with_census


if __name__ == "__main__":
    raise SystemExit(base.main())
