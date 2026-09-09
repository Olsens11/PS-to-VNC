#!/usr/bin/env python3
"""Verify the narrow CP2K visible-RFB presentation contract from source/runtime."""

from pathlib import Path
import socket

import h1_mux_server as base
from h1_profiles import resolve_profile
from h1_rfb_session_adapter import open_rfb_session_adapter

ROOT = Path(__file__).resolve().parents[2]
H1 = ROOT / "experiments/media-harness-h1"

runtime_h = (H1 / "h1_rfb_session_runtime.h").read_text()
runtime_c = (H1 / "h1_rfb_session_runtime.c").read_text()
main_c = (H1 / "h1_main_rfb_visible.c").read_text()
main_entry = (H1 / "h1_main_rfb_visible_entry.c").read_text()
transport_visible = (H1 / "h1_rfb_transport_live_visible.c").read_text()
runtime_visible = (H1 / "h1_rfb_session_runtime_visible.c").read_text()
config_h = (H1 / "h1_config.h").read_text()
activation_gate = (H1 / "h1_config_rfb_activation_gate.c").read_text()
pi_adapter = (H1 / "h1_rfb_session_adapter.py").read_text()
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

if "PSTVNC_H1_RFB_ON_VISIBLE = 2" not in config_h:
    raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL visible_mode_not_reserved")
if "pstvnc_h1_rfb_mode_is_enabled(config->rfb_mode)" not in activation_gate:
    raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL cumulative_gate_not_mode2_aware")

required_pi_mode2 = [
    "RFB_ON_VISIBLE = 2",
    "_rfb_mode_is_enabled",
    "RFB_ON_RESERVED, RFB_ON_VISIBLE",
]
for token in required_pi_mode2:
    if token not in pi_adapter:
        raise SystemExit(f"CP2K_VISIBLE_RFB_CONTRACT=FAIL pi_mode2_adapter:{token}")

for name, text in (
    ("main_entry", main_entry),
    ("transport_visible", transport_visible),
    ("runtime_visible", runtime_visible),
):
    if "#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE" not in text:
        raise SystemExit(f"CP2K_VISIBLE_RFB_CONTRACT=FAIL missing_mode2_scope:{name}")

if "H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_entry.c" not in makefile:
    raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL build_does_not_select_visible_entry")
if "h1_rfb_transport_live_visible.c" not in makefile:
    raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL visible_transport_wrapper_not_selected")
if "h1_rfb_session_runtime_visible.c" not in makefile:
    raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL visible_runtime_wrapper_not_selected")

if "include mk/media-harness-h1-cumulative39-thread-census.mk" not in makefile:
    raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL build_not_based_on_cp2j_population")

# Runtime regression for the exact failure found on the first CP2K hardware
# attempt: validate-only accepted CONFIG mode 2, but the Pi adapter rejected it
# when H1Session was actually constructed. Instantiate the real adapter with a
# socketpair-backed upstream so CI now crosses that runtime boundary.
h1_sock, fake_ps2 = socket.socketpair()
upstream_sock, fake_vnc = socket.socketpair()
adapter = None
try:
    profile = resolve_profile(
        "H1_RFB_ONLY",
        0xC2A00002,
        {"rfb_mode": 2},
    )
    session = base.H1Session(
        h1_sock,
        profile,
        Path("/tmp/h1-cp2k-pi-mode2-runtime"),
        1.0,
        ":0.0",
    )
    connector_calls: list[tuple[str, int]] = []

    def connector(host: str, port: int) -> socket.socket:
        connector_calls.append((host, port))
        return upstream_sock

    adapter = open_rfb_session_adapter(session, connector=connector)
    if adapter is None:
        raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL pi_mode2_adapter_inert")
    if connector_calls != [("127.0.0.1", 5900)]:
        raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL pi_mode2_connector")
    if int(adapter.session.profile["rfb_mode"]) != 2:
        raise SystemExit("CP2K_VISIBLE_RFB_CONTRACT=FAIL pi_mode2_profile_lost")
finally:
    if adapter is not None:
        adapter.stop()
    for sock in (fake_vnc, fake_ps2, h1_sock):
        try:
            sock.close()
        except OSError:
            pass

print("CP2K_VISIBLE_RFB_CONTRACT=PASS")
print("CP2K_RFB_MODE=2_VISIBLE")
print("CP2K_PI_MODE2_RUNTIME_ADAPTER=PASS")
print("CP2K_PRESENTATION=THROUGH_ISSUE39_DISPLAY_AND_PS2_GRAPHICS")
print("CP2K_RFB_TRANSPORT=CP2J_MECHANICS_MECHANICALLY_SCOPED_TO_MODE2")
print("CP2K_AUDIO=OFF")
print("CP2K_MPEG=OFF")
print("CP2K_INPUT=OFF")
print("CP2K_OSK=OFF")
