#!/usr/bin/env python3
"""
File synopsis:
Static source-contract check for live MPEG-calibration ownership, RFB-flow, and
native presentation wiring in the CP2O experiment.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
H1 = ROOT / "experiments" / "media-harness-h1"

coordinator = (H1 / "h1_interaction_coordinator.c").read_text()
coordinator_h = (H1 / "h1_interaction_coordinator.h").read_text()
presenter = (H1 / "h1_interaction_calibration_presenter.c").read_text()
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
    "controller service must observe held entry before ordinary routing")
require(
    "h1_interaction_service_active_calibration" in service_body,
    "controller service must hand active calibration to proven binding")
require(
    "pstvnc_local_controller_route" in service_body,
    "ordinary local-controller routing disappeared")
require(
    service_body.index("h1_interaction_service_active_calibration")
    < service_body.index("pstvnc_local_controller_route"),
    "active calibration must get first refusal before ordinary routing")
require(
    "pstvnc_h1_mpeg_calibration_entry_hold_poll" in coordinator,
    "application service must poll the nonblocking 750 ms deadline")
require(
    "GetTimerSystemTime()" in coordinator and "TimerBusClock2USec" in coordinator,
    "live hold timing must derive from the PS2 monotonic timer")
require(
    "pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension"
    in coordinator[resume_start:],
    "desktop resume must honor calibration-owned mouse suspension")
require(
    "pstvnc_h1_mpeg_calibration_interaction_binding_t mpeg_calibration"
    in coordinator_h,
    "coordinator must own one calibration binding")
require(
    "pstvnc_h1_mpeg_calibration_entry_hold_t mpeg_calibration_entry_hold"
    in coordinator_h,
    "coordinator must own one entry-hold state")
require(
    "pstvnc_h1_interaction_coordinator_rfb_policy" in coordinator_h,
    "coordinator must expose borrowed calibration RFB policy")

require(
    "pstvnc_h1_rfb_session_runtime_run_with_flow_policy" in main,
    "live CP2O RFB run must use generic flow-policy entry point")
require(
    "pstvnc_h1_interaction_coordinator_rfb_policy" in main,
    "live CP2O RFB run must borrow coordinator calibration policy")
require(
    "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service" not in main,
    "CP2O main must not bypass calibration flow policy")

for needle in (
    "pstvnc_h1_interaction_coordinator_present_inner",
    "pstvnc_h1_interaction_coordinator_service_inner",
    "pstvnc_h1_mpeg_calibration_runtime_prepare_render_plan",
    "pstvnc_h1_mpeg_calibration_rasterize",
    "coordinator->gs_pixels",
    "pstvnc_ps2_graphics_present",
):
    require(needle in presenter, f"native calibration presenter missing {needle}")

for needle in (
    "-Dpstvnc_h1_interaction_coordinator_present=pstvnc_h1_interaction_coordinator_present_inner",
    "-Dpstvnc_h1_interaction_coordinator_service=pstvnc_h1_interaction_coordinator_service_inner",
    "$(BUILD_DIR)/h1_interaction_calibration_presenter.o",
    "$(BUILD_DIR)/h1_mpeg_calibration_raster.o",
):
    require(needle in makefile, f"CP2O wrapper build missing {needle}")

required_objects = [
    "mpeg_presentation_calibration_geometry.o",
    "mpeg_presentation_calibration_state.o",
    "h1_mpeg_calibration_adapter.o",
    "h1_mpeg_calibration_foreground.o",
    "h1_mpeg_calibration_rfb_gate.o",
    "h1_mpeg_calibration_rfb_schedule.o",
    "h1_mpeg_calibration_render.o",
    "h1_mpeg_calibration_raster.o",
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
