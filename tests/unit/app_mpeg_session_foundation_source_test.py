#!/usr/bin/env python3
"""Deterministic R27 ordinary Application source-boundary proof."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
APP = ROOT / "src/app.c"


def code_without_comments(path: Path) -> str:
    text = path.read_text(encoding="utf-8")
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*?$", "", text, flags=re.MULTILINE)
    return text


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


code = code_without_comments(APP)

require(
    len(re.findall(r"\bpstvnc_transport_session_open_with_mpeg\s*\(", code)) == 1,
    "ordinary Application must invoke exactly one MPEG-capable Transport constructor",
)
require(
    re.search(r"\bpstvnc_transport_session_open\s*\(", code) is None,
    "ordinary Application must not use plain RFB-only Transport constructor",
)
require(
    re.search(r"\bpstvnc_media_clock_arm\s*\(", code) is None,
    "R27 must not arm the media clock",
)

for forbidden in (
    "pstvnc_transport_mpeg_run_open",
    "pstvnc_transport_mpeg_send_start",
    "pstvnc_transport_mpeg_send_retire",
    "pstvnc_app_mpeg_calibration",
    "pstvnc_app_mpeg_activation",
    "pstvnc_app_mpeg_run_start",
    "pstvnc_app_mpeg_run_service",
    "pstvnc_app_mpeg_run_begin_retirement",
    "pstvnc_mpeg_worker_",
    "pstvnc_mpeg_decoder_",
    "pstvnc_mpeg_presentation_",
    "START+SELECT",
):
    require(forbidden not in code, f"R27 scope creep: {forbidden}")

run_start = code.index("int pstvnc_app_run(void)")
run_code = code[run_start:]
rfb_select = run_code.index("pstvnc_config_rfb_runtime_profile_selected")
mpeg_select = run_code.index("pstvnc_config_mpeg_runtime_profile_selected")
clock_select = run_code.index("pstvnc_config_media_clock_profile_selected")
enter_session = run_code.index("pstvnc_app_run_with_session_profiles")

require(rfb_select < enter_session, "RFB profile selection must precede session entry")
require(mpeg_select < enter_session, "MPEG profile selection must precede session entry")
require(clock_select < enter_session, "media-clock profile selection must precede session entry")

configured_start = code.index("int pstvnc_app_run_with_session_profiles")
configured_code = code[configured_start:run_start]
iop = configured_code.index("pstvnc_ps2_system_prepare_iop")
connect = configured_code.index("pstvnc_ps2_network_connect_pstv")
open_mpeg = configured_code.index("pstvnc_transport_session_open_with_mpeg")
binding = configured_code.index("pstvnc_ps2_media_clock_binding_init")
clock_init = configured_code.index("pstvnc_media_clock_init")

require(iop < connect < open_mpeg < binding < clock_init, "R27 startup owner order is invalid")

print("APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS")
