#!/usr/bin/env python3
"""
File synopsis:
Static source-contract check for the first live MPEG-calibration integration
tranche. It verifies ordering/ownership seams that are difficult to exercise in
the ordinary host build because the live H1 coordinator is PS2-specific.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
H1 = ROOT / "experiments" / "media-harness-h1"
CAL = H1 / "mpeg_presentation_calibration"

coordinator = (H1 / "h1_interaction_coordinator.c").read_text()
coordinator_h = (H1 / "h1_interaction_coordinator.h").read_text()
main = (H1 / "h1_main_rfb_visible_interaction_pcm.c").read_text()
makefile = (
    ROOT / "mk" / "media-harness-h1-cp2o-visible-rfb-interaction-pcm.mk"
).read_text()

def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)

service_start = coordinator.index(
    "static int h1_interaction_service_controller_state")
resume_start = coordinator.index(
    "static int h1_interaction_resume_desktop_mouse_if_ready")
service_body = coordinator[service_start:resume_start]

require(
    "pstvnc_h1_mpeg_calibration_entry_hold_observe" in service_body,
    "controller service must observe the held entry gesture before ordinary routing")
require(
    "h1_interaction_service_active_calibration" in service_body,
    "controller service must hand active calibration states to the proven binding")
require(
    "pstvnc_local_controller_route" in service_body,
    "ordinary local-controller routing disappeared")
require(
    service_body.index("h1_interaction_service_active_calibration")
    < service_body.index("pstvnc_local_controller_route"),
    "active calibration must get first refusal before ordinary routing")

active_start = coordinator.index(
    "static int h1_interaction_service_active_calibration")
active_end = coordinator.index(
    "static int h1_interaction_service_controller_state")
active_body = coordinator[active_start:active_end]
require(
    "pstvnc_h1_mpeg_calibration_interaction_binding_service_controller"
    in active_body,
    "active-calibration helper must invoke the proven interaction binding")

require(
    "pstvnc_h1_mpeg_calibration_entry_hold_poll" in coordinator,
    "application service must poll the nonblocking 750 ms deadline")
require(
    "GetTimerSystemTime()" in coordinator and
    "TimerBusClock2USec" in coordinator,
    "live hold timing must derive from the PS2 monotonic system timer")
require(
    "pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension"
    in coordinator[resume_start:],
    "desktop resume path must honor calibration-owned mouse suspension")
require(
    "pstvnc_h1_mpeg_calibration_interaction_binding_t mpeg_calibration"
    in coordinator_h,
    "coordinator must own one calibration interaction binding")
require(
    "pstvnc_h1_mpeg_calibration_entry_hold_t mpeg_calibration_entry_hold"
    in coordinator_h,
    "coordinator must own one entry-hold state")
require(
    "pstvnc_h1_interaction_coordinator_rfb_policy" in coordinator_h,
    "coordinator must expose its borrowed calibration RFB flow policy")

require(
    "pstvnc_h1_rfb_session_runtime_run_with_flow_policy" in main,
    "live CP2O RFB run must use the generic flow-policy entry point")
require(
    "pstvnc_h1_interaction_coordinator_rfb_policy" in main,
    "live CP2O RFB run must borrow the coordinator calibration policy")
require(
    "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service" not in main,
    "CP2O main must not bypass the calibration flow policy")

required_objects = [
    "mpeg_presentation_calibration_geometry.o",
    "mpeg_presentation_calibration_state.o",
    "h1_mpeg_calibration_adapter.o",
    "h1_mpeg_calibration_foreground.o",
    "h1_mpeg_calibration_rfb_gate.o",
    "h1_mpeg_calibration_rfb_schedule.o",
    "h1_mpeg_calibration_render.o",
    "h1_mpeg_calibration_runtime.o",
    "h1_mpeg_calibration_rfb_flow.o",
    "h1_mpeg_calibration_interaction_binding.o",
    "h1_mpeg_calibration_entry_hold.o",
]
for object_name in required_objects:
    require(
        object_name in makefile,
        f"CP2O PS2 build is missing calibration object {object_name}")

print("MPEG_CALIBRATION_LIVE_WIRING_SOURCE_TEST=PASS")
