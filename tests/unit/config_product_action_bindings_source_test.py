#!/usr/bin/env python3
"""R30 source-boundary proof for human-readable product-action bindings."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
CONFIG_C = ROOT / "src/config/product_action_bindings.c"
CONFIG_H = ROOT / "src/config/product_action_bindings.h"


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


source = CONFIG_C.read_text(encoding="utf-8")
header = CONFIG_H.read_text(encoding="utf-8")
code = strip_comments(source + "\n" + header)

# R30 may name every symbolic button token, but it must not compile a selected
# physical product binding or revive the discarded H1 timing adapter.
require(
    re.search(
        r"static\s+const\s+pstvnc_product_action_binding_t\s+\w+\s*(?:\[|=)",
        code,
    ) is None,
    "R30 must not compile a default product-action binding",
)
for forbidden in (
    "750000",
    "PAD_START",
    "PAD_SELECT",
    "START+SELECT",
):
    require(forbidden not in code, f"discarded H1 adapter leaked into R30: {forbidden}")

# Configuration produces typed data only. It cannot install, route, persist, or
# execute the binding and cannot reach product-effect or transport owners.
for forbidden in (
    "pstvnc_input_runtime_set_product_action_bindings",
    "pstvnc_product_action_resolver_observe",
    "pstvnc_app_",
    "pstvnc_local_ui_",
    "pstvnc_rfb_",
    "pstvnc_transport_",
    "pstvnc_mpeg_",
    "pstvnc_media_clock_",
    "pstvnc_audio_",
    "fopen(",
    "fwrite(",
    "socket(",
    "connect(",
    "/ps2vnc.conf",
    "mailbox",
    "urgent_control",
):
    require(forbidden not in code, f"R30 scope creep: {forbidden}")

# The one accepted R28 seam is used only to validate the completed typed value.
require(
    code.count("pstvnc_product_action_resolver_init") == 1,
    "R30 must validate through exactly one accepted R28 initialization seam",
)
require(
    "PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION" in code,
    "R30 recognized action key must map to the accepted semantic action",
)
require(
    "PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_COUNT 1u" in header,
    "R30 recognized model must remain explicitly bounded to one current action",
)

print("CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS")
