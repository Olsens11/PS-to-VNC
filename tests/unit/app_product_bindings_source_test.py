#!/usr/bin/env python3
"""R32 source-boundary proof for resident desired product bindings."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
APP_C = ROOT / "src/app.c"
HELPER_C = ROOT / "src/app_product_bindings.c"
HELPER_H = ROOT / "src/app_product_bindings.h"


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


app_code = strip_comments(APP_C.read_text(encoding="utf-8"))
helper_code = strip_comments(
    HELPER_C.read_text(encoding="utf-8")
    + "\n"
    + HELPER_H.read_text(encoding="utf-8")
)

wait_index = app_code.index("pstvnc_ps2_network_wait_link()")
acquire_index = app_code.index("pstvnc_app_product_bindings_acquire(")
connect_index = app_code.index("pstvnc_ps2_network_connect_pstv()")

require(
    wait_index < acquire_index < connect_index,
    "R32 acquisition must occur after link wait and before first PSTV connect",
)
require(
    app_code.count("pstvnc_app_product_bindings_acquire(") == 1,
    "ordinary Application must acquire desired bindings exactly once",
)
require(
    acquire_index < app_code.index("for (;;) {", acquire_index),
    "R32 acquisition must remain outside the provider replacement loop",
)

for required in (
    "PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES",
    "PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES",
    "pstvnc_management_config_get(",
    "pstvnc_config_product_action_bindings_parse(",
    "memset(&candidate, 0, sizeof(candidate))",
    "document_length",
    "PSTVNC_APP_PRODUCT_BINDINGS_STATUS_FETCH_FAILED_ZERO",
    "PSTVNC_APP_PRODUCT_BINDINGS_STATUS_INVALID_CONFIG_ZERO",
    "PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID",
):
    require(required in helper_code, f"missing R32 composition mechanism: {required}")

management_index = helper_code.index("pstvnc_management_config_get(")
parse_index = helper_code.index("pstvnc_config_product_action_bindings_parse(")
zero_index = helper_code.index("memset(&candidate, 0, sizeof(candidate))")

require(
    zero_index < management_index < parse_index,
    "R32 must establish zero authority before fetch and parse only after fetch",
)
require(
    re.search(
        r"pstvnc_config_product_action_bindings_parse\s*\(\s*"
        r"\(const char \*\)document\s*,\s*document_length\s*,",
        helper_code,
        flags=re.DOTALL,
    ) is not None,
    "R32 must pass exact returned body length into R30",
)
require(
    "unsigned char document[PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES]" in helper_code,
    "R32 raw document storage must be fixed and bounded",
)
require(
    "static unsigned char document" not in helper_code,
    "R32 raw document storage must not become resident/global authority",
)

combined = app_code + "\n" + helper_code

for forbidden in (
    "pstvnc_input_runtime_set_product_action_bindings",
    "PSTVNC_INPUT_EVENT_PRODUCT_ACTION",
    "pstvnc_app_mpeg_calibration",
    "pstvnc_app_mpeg_activation",
    "pstvnc_app_mpeg_run_start",
    '"POST ',
    "malloc(",
    "calloc(",
    "realloc(",
    "free(",
    "watchdog",
    "mailbox",
):
    require(forbidden not in combined, f"R32 scope creep: {forbidden}")

for forbidden_owner in (
    "ui/",
    "rfb/",
    "transport/",
    "mpeg/",
    "audio/",
    "media/clock.h",
):
    require(
        forbidden_owner not in helper_code,
        f"R32 helper gained forbidden owner dependency: {forbidden_owner}",
    )

print("APP_PRODUCT_BINDINGS_SOURCE_TEST=PASS")
