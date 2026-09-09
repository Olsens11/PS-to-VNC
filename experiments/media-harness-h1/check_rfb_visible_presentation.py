#!/usr/bin/env python3
"""Verify the narrow CP2K visible-RFB presentation contract from source."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
H1 = ROOT / "experiments/media-harness-h1"

runtime_h = (H1 / "h1_rfb_session_runtime.h").read_text()
runtime_c = (H1 / "h1_rfb_session_runtime.c").read_text()
main_c = (H1 / "h1_main_rfb_visible.c").read_text()
makefile = (ROOT / "mk/media-harness-h1-cp2k-visible-rfb.mk").read_text()

required_runtime = [
    "pstvnc_h1_rfb_session_runtime_run_with_presenter",
    "pstvnc_h1_rfb_present_callback_t",
    "runtime->stats.initial_presentations = 1u",
    "runtime->framebuffer.dirty",
    "runtime->stats.incremental_presentations++",
]

for token in required_runtime:
    if token not in runtime_h and token not in runtime_c:
        raise SystemExit(f"CP2K_VISIBLE_RFB_CONTRACT=FAIL missing_runtime:{token}")

required_main = [
    "pstvnc_ps2_graphics_init()",
    "pstvnc_display_prepare_gs16(",
    "pstvnc_ps2_graphics_present(",
    "pstvnc_h1_rfb_session_runtime_run_with_presenter(",
    "H1_RFB_VISIBLE=PASS",
    "PSTVNC_H1_AUDIO_OFF",
    "PSTVNC_H1_VIDEO_OFF",
]

for token in required_main:
    if token not in main_c:
        raise SystemExit(f"CP2K_VISIBLE_RFB_CONTRACT=FAIL missing_main:{token}")

for forbidden in [
    "pstvnc_h1_video_chassis_init(",
    "pstvnc_h1_video_run_session(",
    "pstvnc_h1_audio_start(",
    "pstvnc_input_runtime_start(",
    "pstvnc_rfb_session_send_pointer_event(",
    "pstvnc_rfb_session_send_key_event(",
    "pstvnc_osk_",
    "pstvnc_local_ui_",
]:
    if forbidden in main_c:
        raise SystemExit(f"CP2K_VISIBLE_RFB_CONTRACT=FAIL forbidden_main:{forbidden}")

if "H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible.c" not in makefile:
    raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL build_does_not_select_visible_main")

if "include mk/media-harness-h1-cumulative39-thread-census.mk" not in makefile:
    raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL build_not_based_on_cp2j_population")

print("CP2K_VISIBLE_RFB_CONTRACT=PASS")
print("CP2K_PRESENTATION=THROUGH_ISSUE39_DISPLAY_AND_PS2_GRAPHICS")
print("CP2K_RFB_TRANSPORT=EXISTING_CP2J_ONE_SOCKET_CHANNEL1")
print("CP2K_AUDIO=OFF")
print("CP2K_MPEG=OFF")
print("CP2K_INPUT=OFF")
print("CP2K_OSK=OFF")
