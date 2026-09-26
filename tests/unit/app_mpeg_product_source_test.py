#!/usr/bin/env python3
"""R34 source-boundary proof for ordinary MPEG semantic activation."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
APP = (ROOT / "src/app.c").read_text(encoding="utf-8")
PRODUCT = (ROOT / "src/app_mpeg_product.c").read_text(encoding="utf-8")
HEADER = (ROOT / "src/app_mpeg_product.h").read_text(encoding="utf-8")


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


app = strip_comments(APP)
product = strip_comments(PRODUCT + "\n" + HEADER)

# Binding install is exact R32 authority and must happen before worker start.
init_i = app.index("pstvnc_input_runtime_init(")
binding_i = app.index("pstvnc_input_runtime_set_product_action_bindings(", init_i)
start_i = app.index("pstvnc_input_runtime_start(", binding_i)
require(init_i < binding_i < start_i, "R32 bindings must install after init and before Input start")
require(
    "desired_product_bindings.desired.bindings" in app[binding_i:start_i]
    and "desired_product_bindings.desired.binding_count" in app[binding_i:start_i],
    "ordinary Input install must consume exact R32 snapshot pointer/count",
)

# Fresh attempt-local product owner and exact Transport access.
require("pstvnc_transport_access_acquire(" in app, "R34 must acquire exact session Transport access")
require("pstvnc_app_mpeg_product_init(" in app, "R34 must construct fresh product owner per attempt")
require("pstvnc_mpeg_presentation_t presentation;" in product, "product owner must contain fresh P3")
require("pstvnc_app_mpeg_calibration_t calibration;" in product, "product owner must contain fresh P9")
require("pstvnc_app_mpeg_run_t run;" in product, "product owner must contain fresh R21")

# Semantic routing only: no physical-button vocabulary in coordinator.
require("PSTVNC_INPUT_EVENT_PRODUCT_ACTION" in app, "Application must route PRODUCT_ACTION")
require("PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION" in product, "R34 must own only MPEG_CALIBRATION semantic route")
for forbidden in (
    "PSTVNC_CONTROLLER_BUTTON_",
    "button_mask",
    "PSTVNC_PRODUCT_ACTION_TRIGGER_",
    "PSTVNC_PRODUCT_ACTION_SETTLE_POLLS",
    "PSTVNC_PRODUCT_ACTION_HOLD_POLLS",
):
    require(forbidden not in product, f"R34 duplicated physical binding logic: {forbidden}")

# Accepted owners only.
for required in (
    "pstvnc_app_mpeg_calibration_begin(",
    "pstvnc_app_mpeg_calibration_service_controller(",
    "pstvnc_app_mpeg_activation_start_protected(",
    "pstvnc_app_mpeg_run_service(",
    "pstvnc_app_mpeg_run_session_abort_service(",
):
    require(required in product, f"missing accepted owner composition: {required}")

require(
    app.count("pstvnc_input_runtime_set_product_action_desktop_eligible(") >= 2,
    "Application must publish live DESKTOP eligibility transitions",
)
require(
    "pstvnc_ps2_media_clock_binding_current_tick(" in app,
    "live R22 service must use exact current session clock tick",
)
require(
    "pstvnc_transport_session_begin_abort(" in app
    and "pstvnc_transport_session_close(" in app,
    "live MPEG session failure must use R33 two-phase Transport teardown",
)
require(
    "pstvnc_transport_session_abort()" in app,
    "no-MPEG attempt must retain accepted one-shot abort path",
)

# R34 is activation/live only. Normal same-session retirement/reveal is deferred.
combined = app + "\n" + product
for forbidden in (
    "pstvnc_app_mpeg_run_begin_retirement(",
    "pstvnc_app_mpeg_run_retirement_service(",
    "pstvnc_app_mpeg_run_record_restored_rfb_presented(",
    "pstvnc_app_mpeg_run_reveal_restored(",
    "pstvnc_transport_mpeg_send_retire(",
    "pstvnc_transport_mpeg_mark_producer_done(",
    "pstvnc_transport_mpeg_run_finalize(",
    "pstvnc_mpeg_compositor_reveal_retired(",
):
    require(forbidden not in combined, f"R34 entered deferred retirement/reveal: {forbidden}")

for forbidden in (
    '"POST ',
    "pstvnc_audio_",
    "audsrv",
    "fopen(",
    "fwrite(",
    "watchdog",
    "mailbox",
):
    require(forbidden not in product, f"R34 scope creep: {forbidden}")

print("APP_MPEG_PRODUCT_SOURCE_TEST=PASS")
