#!/usr/bin/env python3
"""R28 source-boundary proof for semantic product-action bindings."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
ACTION_C = ROOT / "src/input/product_action.c"
ACTION_H = ROOT / "src/input/product_action.h"
INPUT_C = ROOT / "src/input/input.c"
INPUT_H = ROOT / "src/input/input.h"


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


action_source = ACTION_C.read_text(encoding="utf-8")
action_header = ACTION_H.read_text(encoding="utf-8")
input_source = INPUT_C.read_text(encoding="utf-8")
input_header = INPUT_H.read_text(encoding="utf-8")
action_code = strip_comments(action_source + "\n" + action_header)
all_code = strip_comments(
    action_source + "\n" + action_header + "\n" +
    input_source + "\n" + input_header
)

# R28 owns semantic values but selects no physical MPEG-calibration chord.
for forbidden in (
    "PSTVNC_CONTROLLER_BUTTON_START",
    "PSTVNC_CONTROLLER_BUTTON_SELECT",
    "PAD_START",
    "PAD_SELECT",
    "750000",
):
    require(forbidden not in action_code, f"compiled historical binding leaked: {forbidden}")

require("START+SELECT" not in action_source, "historical START+SELECT adapter leaked")
require(
    re.search(
        r"static\s+const\s+pstvnc_product_action_binding_t\s+\w+\s*\[",
        action_code,
    ) is None,
    "R28 must not compile a default binding table",
)

# The ordinary semantic event exists but no product effect or urgent path does.
for forbidden in (
    "pstvnc_app_mpeg_",
    "pstvnc_media_clock_arm",
    "pstvnc_transport_",
    "pstvnc_rfb_",
    "pstvnc_local_ui_",
    "urgent_control",
    "mailbox",
    "src/config/text",
    "fopen(",
):
    require(forbidden not in all_code, f"R28 cross-domain scope creep: {forbidden}")

require(
    "PSTVNC_INPUT_EVENT_PRODUCT_ACTION" in input_source + input_header,
    "typed PRODUCT_ACTION event is missing",
)
require(
    "PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION" in action_code,
    "semantic MPEG_CALIBRATION action is missing",
)

print("PRODUCT_ACTION_SOURCE_TEST=PASS")
