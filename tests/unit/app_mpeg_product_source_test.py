#!/usr/bin/env python3
"""R34/R35 source-boundary proof for ordinary MPEG product composition."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
APP = (ROOT / "src/app.c").read_text(encoding="utf-8")
PRODUCT_C = (ROOT / "src/app_mpeg_product.c").read_text(encoding="utf-8")
HEADER = (ROOT / "src/app_mpeg_product.h").read_text(encoding="utf-8")


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)


def function_body(text: str, name: str) -> str:
    start = text.find(name + "(")
    if start < 0:
        raise AssertionError(f"missing function: {name}")
    brace = text.find("{", start)
    if brace < 0:
        raise AssertionError(f"missing function body: {name}")

    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start : index + 1]
    raise AssertionError(f"unterminated function: {name}")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


app = strip_comments(APP)
product_c = strip_comments(PRODUCT_C)
product = strip_comments(PRODUCT_C + "\n" + HEADER)

# R32 binding install remains exact and precedes Input start.
init_i = app.index("pstvnc_input_runtime_init(")
binding_i = app.index("pstvnc_input_runtime_set_product_action_bindings(", init_i)
start_i = app.index("pstvnc_input_runtime_start(", binding_i)
require(init_i < binding_i < start_i, "R32 bindings must install before Input start")
require(
    "desired_product_bindings.desired.bindings" in app[binding_i:start_i]
    and "desired_product_bindings.desired.binding_count" in app[binding_i:start_i],
    "ordinary Input install must consume exact resident R32 snapshot",
)

# Fresh attempt-local product ownership remains unchanged.
for required in (
    "pstvnc_transport_access_acquire(",
    "pstvnc_app_mpeg_product_init(",
):
    require(required in app, f"ordinary Application lost {required}")
for required in (
    "pstvnc_mpeg_presentation_t presentation;",
    "pstvnc_app_mpeg_calibration_t calibration;",
    "pstvnc_app_mpeg_run_t run;",
):
    require(required in product, f"product owner lost {required}")

# Semantic routing only: no physical binding vocabulary or new action.
require("PSTVNC_INPUT_EVENT_PRODUCT_ACTION" in app, "Application must route PRODUCT_ACTION")
require(
    "PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION" in product,
    "R35 must reuse the accepted MPEG_CALIBRATION semantic action",
)
for forbidden in (
    "PSTVNC_CONTROLLER_BUTTON_",
    "button_mask",
    "PSTVNC_PRODUCT_ACTION_TRIGGER_",
    "PSTVNC_PRODUCT_ACTION_SETTLE_POLLS",
    "PSTVNC_PRODUCT_ACTION_HOLD_POLLS",
):
    require(forbidden not in product, f"product duplicated Input binding logic: {forbidden}")

# Product coordinator may compose only accepted public owner seams.
for required in (
    "pstvnc_app_mpeg_calibration_begin(",
    "pstvnc_app_mpeg_calibration_service_controller(",
    "pstvnc_app_mpeg_activation_start_protected(",
    "pstvnc_app_mpeg_run_service(",
    "pstvnc_app_mpeg_run_begin_retirement(",
    "pstvnc_app_mpeg_run_retirement_service(",
    "pstvnc_app_mpeg_run_record_restored_rfb_presented(",
    "pstvnc_app_mpeg_run_reveal_restored(",
    "pstvnc_app_mpeg_run_session_abort_service(",
):
    require(required in product_c, f"missing accepted owner composition: {required}")

route = function_body(product_c, "pstvnc_app_mpeg_product_route_action")
require(
    route.count("pstvnc_app_mpeg_run_begin_retirement(") == 1,
    "semantic MPEG_OWNED action must enter R23 exactly once through its public seam",
)
require(
    "PSTVNC_APP_MPEG_RUN_MPEG_OWNED" in route
    and "PSTVNC_APP_MPEG_RUN_IDLE" in route,
    "route must keep explicit IDLE-vs-MPEG_OWNED state policy",
)
require(
    route.index("PSTVNC_APP_MPEG_RUN_MPEG_OWNED")
    < route.index("pstvnc_app_mpeg_run_begin_retirement("),
    "R23 entry must be state-gated",
)

# R22 and R23 remain distinct and both consume exact session-clock ticks.
live = function_body(product_c, "pstvnc_app_mpeg_product_service_live")
retirement = function_body(product_c, "pstvnc_app_mpeg_product_service_retirement")
require("pstvnc_app_mpeg_run_service(" in live, "R22 live owner seam missing")
require("pstvnc_app_mpeg_run_retirement_service(" not in live, "R22 may not service R23")
require("pstvnc_app_mpeg_run_retirement_service(" in retirement, "R23 service seam missing")
require("pstvnc_app_mpeg_run_service(" not in retirement, "R23 may not re-enter R22")

require(
    "pstvnc_ps2_media_clock_binding_current_tick(" in app,
    "clocked MPEG service must use current session tick",
)
clocked = app[
    app.index("if (pstvnc_app_mpeg_product_has_started_run("):
    app.index("receive_result = pstvnc_rfb_session_try_receive_update(")
]
require(
    "pstvnc_app_mpeg_product_is_retiring(" in clocked
    and "pstvnc_app_mpeg_product_service_retirement(" in clocked
    and "pstvnc_app_mpeg_product_service_live(" in clocked,
    "Application must branch R22/R23 service by exact retirement state",
)

# R23C restoration request serialization remains the existing R19/P2 helper.
require(
    "service_rfb_flow_request(" in clocked,
    "RETIRING must re-enter the ordinary R19/P2 request path",
)
for forbidden in (
    "pstvnc_transport_mpeg_send_retire(",
    "pstvnc_transport_mpeg_take_retire_completion(",
    "pstvnc_transport_mpeg_mark_producer_done(",
    "pstvnc_transport_mpeg_run_finalize(",
    "pstvnc_mpeg_presentation_begin_retirement(",
    "pstvnc_mpeg_presentation_seal_retirement(",
    "pstvnc_mpeg_compositor_reveal_retired(",
):
    require(
        forbidden not in app + "\n" + product_c,
        f"R35 bypassed accepted run owner: {forbidden}",
    )

# The R24 marker follows the real existing desktop presentation boundary.
receive_start = app.index(
    "pstvnc_rfb_flow_policy_record_update_complete("
)
receive_end = app.index(
    "if (!service_rfb_flow_request(&session, &rfb_flow_policy))",
    receive_start,
)
receive = app[receive_start:receive_end]
require(
    "pstvnc_app_mpeg_product_restoration_pending(" in receive,
    "Application must identify exact RESTORE_PENDING publication",
)
present_i = receive.index("present_current_application_frame(")
marker_i = receive.index("pstvnc_app_mpeg_product_record_restored_desktop_presented(")
require(
    present_i < marker_i,
    "restored desktop must cross presentation boundary before R24 marker",
)
require(
    "framebuffer.dirty || restoration_pending" in receive,
    "RESTORE_PENDING must force presentation even for unchanged FULL pixels",
)

# Reveal is ordinary-cadence product service; product owns retry classification.
require(
    "pstvnc_app_mpeg_product_service_reveal(" in clocked,
    "Application must service R24 reveal at ordinary cadence",
)
reveal = function_body(product_c, "pstvnc_app_mpeg_product_service_reveal")
for retryable in (
    "PSTVNC_APP_MPEG_RUN_REVEAL_PLATFORM_FAILED",
    "PSTVNC_APP_MPEG_RUN_REVEAL_SYNC_INVALID",
):
    require(retryable in reveal, f"R24 retryable result lost: {retryable}")
require(
    "pstvnc_app_mpeg_calibration_begin(" not in reveal,
    "R35 reveal must not automatically reopen P9 calibration",
)

# R34C/R35P abnormal owner path remains distinct from normal same-session R35.
retire_start = app.index("static int retire_attempt_owners(")
retire_end = app.index("int pstvnc_app_run_with_session_profiles(", retire_start)
abnormal = app[retire_start:retire_end]
require(
    "pstvnc_app_mpeg_product_requires_session_abort(" in abnormal
    and "pstvnc_transport_session_begin_abort(" in abnormal
    and "pstvnc_app_mpeg_product_service_session_abort(" in abnormal
    and "pstvnc_transport_session_close(" in abnormal,
    "abnormal MPEG teardown must retain R35P two-phase composition",
)
require(
    "pstvnc_app_mpeg_product_service_retirement(" not in abnormal
    and "pstvnc_app_mpeg_product_service_reveal(" not in abnormal,
    "abnormal teardown must not replay normal R35 retirement/reveal",
)

# Scope exclusions remain exact.
for forbidden in (
    '"POST ',
    "pstvnc_audio_",
    "audsrv",
    "fopen(",
    "fwrite(",
    "watchdog",
    "mailbox",
):
    require(forbidden not in product_c, f"R35 scope creep: {forbidden}")

print("APP_MPEG_PRODUCT_SOURCE_TEST=PASS")
