#!/usr/bin/env python3
"""R33 source-boundary proof for retained Transport and local MPEG abort."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
BRIDGE_C = ROOT / "src/transport/bridge.c"
RUN_C = ROOT / "src/app_mpeg_run.c"
FRAME_C = ROOT / "src/app_mpeg_frame.c"
APP_C = ROOT / "src/app.c"


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
    in_string = False
    in_char = False
    escaped = False
    i = brace
    while i < len(text):
        ch = text[i]
        if in_string or in_char:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif in_string and ch == '"':
                in_string = False
            elif in_char and ch == "'":
                in_char = False
            i += 1
            continue
        if ch == '"':
            in_string = True
        elif ch == "'":
            in_char = True
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[start : i + 1]
        i += 1

    raise AssertionError(f"unterminated function body: {name}")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


bridge = strip_comments(BRIDGE_C.read_text(encoding="utf-8"))
run = strip_comments(RUN_C.read_text(encoding="utf-8"))
frame = strip_comments(FRAME_C.read_text(encoding="utf-8"))
ordinary_app = strip_comments(APP_C.read_text(encoding="utf-8"))

begin_abort = function_body(
    bridge,
    "pstvnc_transport_session_begin_abort",
)
one_shot_abort = function_body(
    bridge,
    "pstvnc_transport_session_abort",
)
retained = function_body(
    bridge,
    "pstvnc_transport_session_abort_storage_retained",
)
local_abort = function_body(
    run,
    "pstvnc_app_mpeg_run_session_abort_service",
)
abandon = function_body(
    frame,
    "pstvnc_app_mpeg_frame_consumer_abandon_claim",
)

for required in (
    "pstvnc_transport_runtime_request_stop(",
    "pstvnc_transport_runtime_wait_receiver_done(",
):
    require(required in begin_abort, f"begin-abort missing {required}")

for forbidden in (
    "pstvnc_transport_runtime_release(",
    "pstvnc_transport_bridge_finish_release(",
    "pstvnc_transport_session_close(",
):
    require(forbidden not in begin_abort, f"begin-abort reclaimed storage: {forbidden}")

require(
    begin_abort.index("pstvnc_transport_runtime_request_stop(")
    < begin_abort.index("pstvnc_transport_runtime_wait_receiver_done("),
    "begin-abort must stop/wake before waiting receiver completion",
)

for required in (
    "pstvnc_transport_session_begin_abort(",
    "pstvnc_transport_session_close(",
):
    require(required in one_shot_abort, f"one-shot abort lost compatibility: {required}")

for required in (
    "pstvnc_transport_bridge_runtime_active",
    "pstvnc_transport_bridge_active_ticket",
    "receiver_done",
    "PSTVNC_TRANSPORT_RECEIVER_COMPLETION_PROVEN",
):
    require(required in retained, f"retained-ticket proof missing {required}")

for required in (
    "pstvnc_transport_session_abort_storage_retained(",
    "pstvnc_app_mpeg_frame_consumer_abandon_claim(",
    "pstvnc_mpeg_worker_request_stop(",
    "pstvnc_mpeg_worker_status(",
    "pstvnc_mpeg_worker_join(",
    "pstvnc_mpeg_worker_outcome(",
    "pstvnc_mpeg_worker_release(",
    "pstvnc_mpeg_ps2_worker_runtime_release(",
    "PSTVNC_APP_MPEG_RUN_SESSION_ABORTING",
    "PSTVNC_APP_MPEG_RUN_SESSION_ABORT_READY",
):
    require(required in local_abort, f"local abort missing {required}")

ordered = (
    "pstvnc_transport_session_abort_storage_retained(",
    "pstvnc_app_mpeg_frame_consumer_abandon_claim(",
    "pstvnc_mpeg_worker_request_stop(",
    "pstvnc_mpeg_worker_status(",
    "pstvnc_mpeg_worker_join(",
    "pstvnc_mpeg_worker_outcome(",
    "pstvnc_mpeg_worker_release(",
    "pstvnc_mpeg_ps2_worker_runtime_release(",
)
positions = [local_abort.index(item) for item in ordered]
require(
    positions == sorted(positions),
    "R33 local abort owner ordering drifted",
)

for forbidden in (
    "pstvnc_transport_mpeg_send_retire(",
    "pstvnc_transport_mpeg_take_retire_completion(",
    "pstvnc_transport_mpeg_mark_producer_done(",
    "pstvnc_transport_mpeg_run_finalize(",
    "pstvnc_rfb_flow_policy_set_frozen(",
    "pstvnc_mpeg_presentation_begin_retirement(",
    "pstvnc_mpeg_presentation_seal_retirement(",
    "pstvnc_mpeg_compositor_reveal_retired(",
    "pstvnc_app_mpeg_frame_consumer_service(",
):
    require(forbidden not in local_abort, f"R33 masquerades as normal retirement: {forbidden}")

for forbidden in (
    "pstvnc_mpeg_compositor_present(",
    "pstvnc_app_mpeg_frame_consumer_service(",
    "pstvnc_mpeg_presentation_",
):
    require(forbidden not in abandon, f"claim abandonment gained presentation effect: {forbidden}")

require(
    "pstvnc_mpeg_worker_release_frame(" in abandon,
    "claim abandonment must release the exact worker claim",
)
require(
    "pstvnc_app_mpeg_run_session_abort_service(" not in ordinary_app,
    "R33 must not wire ordinary app.c",
)
require(
    "pstvnc_transport_session_begin_abort(" not in ordinary_app,
    "R33 must not alter ordinary app teardown yet",
)

print("APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS")
