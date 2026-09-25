#!/usr/bin/env python3
"""R29 source-boundary proof for live Input product-action publication."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
RUNTIME_C = ROOT / "src/input/input_runtime.c"
RUNTIME_H = ROOT / "src/input/input_runtime.h"


def code_without_comments(path: Path) -> str:
    text = path.read_text(encoding="utf-8")
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*?$", "", text, flags=re.MULTILINE)
    return text


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


code = code_without_comments(RUNTIME_C)
header = code_without_comments(RUNTIME_H)
combined = code + "\n" + header

require(
    "pstvnc_product_action_resolver_observe" in code,
    "R29 runtime must consume the accepted R28 resolver",
)
require(
    "PSTVNC_INPUT_EVENT_PRODUCT_ACTION" in code,
    "R29 runtime must publish the accepted ordinary semantic event",
)
require(
    combined.count("pstvnc_input_queue_t event_queue") == 1,
    "R29 must retain exactly one ordinary semantic FIFO owner",
)

# Physical product mapping remains deliberately unselected.
for forbidden in (
    "PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION",
    "PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE",
    "PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE",
    "PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD",
    "750000",
):
    require(
        forbidden not in combined,
        f"R29 compiled forbidden physical/action policy: {forbidden}",
    )

# Input recognition cannot execute or reach through to product-effect owners.
for forbidden in (
    "pstvnc_app_mpeg",
    "pstvnc_media_clock_arm",
    "pstvnc_transport_",
    "pstvnc_rfb_",
    "pstvnc_local_ui",
    "pstvnc_local_controller",
    "pstvnc_mpeg_",
    "pstvnc_audio_",
):
    require(
        forbidden not in combined,
        f"R29 Input runtime crossed product-effect boundary: {forbidden}",
    )

for forbidden_header in (
    "app.h",
    "local_ui.h",
    "local_controller.h",
    "transport/",
    "rfb/",
    "config/",
    "mpeg/",
    "audio/",
):
    require(
        forbidden_header not in combined,
        f"R29 Input runtime gained forbidden owner include/reference: {forbidden_header}",
    )

require(
    "mailbox" not in combined.lower(),
    "R29 must not create an urgent or parallel mailbox",
)

print("INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS")
