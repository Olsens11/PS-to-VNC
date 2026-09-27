#!/usr/bin/env python3
"""R42 source-boundary proof for ordinary cross-platform AUDIO composition."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
APP = (ROOT / "src/app.c").read_text(encoding="utf-8")
PI_RUNTIME = (ROOT / "pi/wire_runtime.py").read_text(encoding="utf-8")
PI_AUDIO_PROFILE = (ROOT / "pi/audio_product_profile.py").read_text(
    encoding="utf-8"
)


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)


def function_body(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0:
        raise AssertionError(f"missing function: {signature}")
    brace = text.find("{", start)
    if brace < 0:
        raise AssertionError(f"missing body: {signature}")
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1 : index]
    raise AssertionError(f"unterminated function: {signature}")


code = strip_comments(APP)
entry = function_body(code, "int pstvnc_app_run(void)")
run = function_body(code, "int pstvnc_app_run_with_session_profiles(")
retire = function_body(code, "static int retire_attempt_owners(")

rfb_select = entry.index("pstvnc_config_rfb_runtime_profile_selected(")
audio_select = entry.index("pstvnc_config_audio_runtime_profile_selected(")
audio_valid = entry.index("pstvnc_config_audio_runtime_profile_valid(")
mpeg_select = entry.index("pstvnc_config_mpeg_runtime_profile_selected(")
clock_select = entry.index("pstvnc_config_media_clock_profile_selected(")
enter = entry.index("pstvnc_app_run_with_session_profiles(")
if not (
    rfb_select
    < audio_select
    < audio_valid
    < mpeg_select
    < clock_select
    < enter
):
    raise AssertionError("R42 selected authority is not resolved before startup")

iop = run.index("pstvnc_ps2_system_prepare_iop(")
resident = run.index("pstvnc_audio_ps2_resident_prepare(")
network = run.index("pstvnc_ps2_network_init(")
open_media = run.index("pstvnc_transport_session_open_with_audio_mpeg(")
clock_binding = run.index("pstvnc_ps2_media_clock_binding_init(")
clock_time_ops = run.index("pstvnc_ps2_media_clock_binding_time_ops(")
audio_init = run.index("pstvnc_app_audio_product_init(")
if not (
    iop
    < resident
    < network
    < open_media
    < clock_binding
    < clock_time_ops
    < audio_init
):
    raise AssertionError("R42 resident/session owner startup order is invalid")

started = run.index("pstvnc_app_mpeg_product_has_started_run(")
audio_start = run.index("pstvnc_app_audio_product_start(", started)
audio_service = run.index("pstvnc_app_audio_product_service(", audio_start)
clock_armed = run.index("pstvnc_media_clock_is_armed(", audio_service)
audio_gate = run.index(
    "pstvnc_app_audio_product_first_presentation_ready(",
    clock_armed,
)
mpeg_live = run.index("pstvnc_app_mpeg_product_service_live(", audio_gate)
if not started < audio_start < audio_service < clock_armed < audio_gate < mpeg_live:
    raise AssertionError("R42 start/service/first-presentation order is invalid")

begin_abort = retire.index("pstvnc_transport_session_begin_abort(")
mpeg_abort = retire.index("pstvnc_app_mpeg_product_service_session_abort(")
audio_abort = retire.index("pstvnc_app_audio_product_service_session_abort(")
clock_release = retire.index("pstvnc_ps2_media_clock_binding_release(")
transport_close = retire.index("pstvnc_transport_session_close(")
if not begin_abort < mpeg_abort < audio_abort < clock_release < transport_close:
    raise AssertionError("R42 abnormal teardown owner order is invalid")

for forbidden in (
    "pstvnc_audio_session_",
    "pstvnc_audio_ps2_runtime_init(",
    "pstvnc_transport_audio_activate(",
    "pstvnc_transport_audio_status(",
    "pstvnc_transport_audio_read_available(",
    "audsrv_quit",
):
    if forbidden in code:
        raise AssertionError(
            f"ordinary Application bypassed accepted AUDIO owners: {forbidden}"
        )

if "audio_product_profile.selected_audio_pcm_factory()" not in PI_RUNTIME:
    raise AssertionError("ordinary Pi runtime lost selected lazy AUDIO factory")
if "audio_pcm_factory=audio_pcm_factory" not in PI_RUNTIME:
    raise AssertionError("ordinary Pi runtime does not inject AUDIO factory")
if "AUDIO_RETIREMENT_ESCALATION_SECONDS = 2.0" not in PI_AUDIO_PROFILE:
    raise AssertionError("R42 Pi retirement escalation horizon is not 2.0 seconds")
for forbidden in (
    "pw-record",
    "wpctl",
    "subprocess.Popen",
    "sendall(",
    "recv(",
):
    if forbidden in PI_AUDIO_PROFILE:
        raise AssertionError(
            f"Pi product profile gained lower mechanism ownership: {forbidden}"
        )

print("APP_AUDIO_COMPOSITION_SOURCE_TEST=PASS")
