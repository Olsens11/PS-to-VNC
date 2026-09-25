#!/usr/bin/env python3
"""
R26 source-boundary proof.

This deterministic repository test verifies that the selected Config/PS2
binding remains mechanism-only: it must not arm or reimplement the A002 media
clock, and ordinary Application composition remains deferred.
"""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]

CONFIG_C = ROOT / "src/config/media_clock_profile.c"
PLATFORM_C = ROOT / "src/platform/ps2_media_clock.c"
PLATFORM_H = ROOT / "src/platform/ps2_media_clock.h"
APP_C = ROOT / "src/app.c"


def source_without_comments(path: Path) -> str:
    text = path.read_text(encoding="utf-8")
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*?$", "", text, flags=re.MULTILINE)
    return text


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


config_code = source_without_comments(CONFIG_C)
platform_code = source_without_comments(PLATFORM_C)
platform_header = source_without_comments(PLATFORM_H)
r26_code = "\n".join((config_code, platform_code, platform_header))
app_code = source_without_comments(APP_C)

# Selected immutable Configuration values are exactly the grounded R26 0/0/0.
require(config_code.count(".epoch_lead_us = 0u") == 1, "epoch lead is not exact 0")
require(
    config_code.count(".audio_presentation_offset_us = 0") == 1,
    "audio presentation offset is not exact 0",
)
require(
    config_code.count(".video_presentation_offset_us = 0") == 1,
    "video presentation offset is not exact 0",
)

# R26 never becomes an epoch/deadline owner and never invokes the arm boundary.
require(
    re.search(r"\bpstvnc_media_clock_arm\s*\(", r26_code) is None,
    "R26 must not call pstvnc_media_clock_arm",
)
for forbidden_identifier in ("epoch_tick", "armed", "deadline_tick"):
    require(
        re.search(rf"\b{forbidden_identifier}\b", r26_code) is None,
        f"R26 duplicates media-clock state: {forbidden_identifier}",
    )

# The PS2 binding is exactly semaphore + H1 timer/yield mechanism adaptation.
for required_call in (
    "CreateSema(",
    "DeleteSema(",
    "WaitSema(",
    "SignalSema(",
    "GetTimerSystemTime(",
    "DelayThread(",
):
    require(
        required_call in platform_code,
        f"missing PS2 binding mechanism: {required_call}",
    )
require(
    re.search(r"\bkBUSCLK\b", platform_code) is not None,
    "R26 tick rate must remain the kBUSCLK domain",
)

# R27 later composes this accepted R26 authority into ordinary session startup.
# Preserve R26's mechanism boundary while requiring the newer authorized caller.
for composed_name in (
    "pstvnc_config_media_clock_profile_selected",
    "pstvnc_ps2_media_clock_binding_init",
    "pstvnc_media_clock_init",
):
    require(
        composed_name in app_code,
        f"R27 ordinary app.c must consume accepted R26 authority: {composed_name}",
    )

require(
    re.search(r"\bpstvnc_media_clock_arm\s*\(", app_code) is None,
    "ordinary app.c must not arm the R26/A002 media clock before semantic MPEG entry",
)

print("MEDIA_CLOCK_PRODUCT_BINDING_SOURCE_TEST=PASS")
