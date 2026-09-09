#!/usr/bin/env python3
"""Verify the CP2N real-module visible-RFB interaction checkpoint contract."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
H1 = ROOT / "experiments/media-harness-h1"
MAKEFILE = ROOT / "mk/media-harness-h1-cp2n-visible-rfb-interaction.mk"
COORDINATOR_C = H1 / "h1_interaction_coordinator.c"
COORDINATOR_H = H1 / "h1_interaction_coordinator.h"
MAIN_C = H1 / "h1_main_rfb_visible_interaction.c"
MAIN_ENTRY = H1 / "h1_main_rfb_visible_interaction_entry.c"


def fail(message: str) -> None:
    print(f"CP2N_VISIBLE_RFB_INTERACTION_CONTRACT=FAIL detail={message}")
    raise SystemExit(1)


def strip_c_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*?$", "", text, flags=re.M)


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        fail(f"missing_{label}:{needle}")


def check_source() -> None:
    coordinator = strip_c_comments(COORDINATOR_C.read_text(encoding="utf-8"))
    coordinator_h = COORDINATOR_H.read_text(encoding="utf-8")
    main = strip_c_comments(MAIN_C.read_text(encoding="utf-8"))
    entry = MAIN_ENTRY.read_text(encoding="utf-8")
    makefile = MAKEFILE.read_text(encoding="utf-8")

    for needle in (
        "pstvnc_input_runtime_init(",
        "pstvnc_input_runtime_start(",
        "pstvnc_input_runtime_pop_event(",
        "pstvnc_input_runtime_suspend_mouse_interpretation(",
        "pstvnc_input_runtime_rebase_suspended_mouse_state(",
        "pstvnc_input_runtime_resume_mouse_interpretation(",
        "pstvnc_local_controller_route(",
        "pstvnc_local_controller_release_is_proven(",
        "pstvnc_local_ui_open_osk(",
        "pstvnc_local_ui_close_osk(",
        "pstvnc_local_ui_complete_input_quarantine(",
        "pstvnc_osk_reset_for_open(",
        "pstvnc_osk_activate_selected(",
        "pstvnc_osk_activate_direct_key(",
        "pstvnc_keyboard_build_tap_sequence(",
        "pstvnc_rfb_session_send_pointer_event(",
        "pstvnc_rfb_session_send_key_event(",
        "pstvnc_local_ui_prepare_presentation(",
        "pstvnc_display_prepare_gs16(",
        "pstvnc_ps2_graphics_present(",
    ):
        require(coordinator, needle, "real_module_composition")

    for forbidden in (
        "pstvnc_h1_rfb_keyboard_chord",
        "PSTVNC_CONTROLLER_BUTTON_L1",
        "socket(",
        "connect(",
        "recv(",
    ):
        if forbidden in coordinator:
            fail(f"coordinator_forbidden_mechanism:{forbidden}")

    for needle in (
        "pstvnc_h1_interaction_coordinator_present",
        "pstvnc_h1_interaction_coordinator_service",
        "pstvnc_h1_interaction_coordinator_shutdown",
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(",
        "PSTVNC_H1_AUDIO_OFF",
        "PSTVNC_H1_VIDEO_OFF",
        "H1_CP2N_VISIBLE_RFB_INTERACTION_START",
    ):
        require(main, needle, "cp2n_main")

    for forbidden in (
        "pstvnc_h1_rfb_keyboard_chord",
        "pstvnc_h1_audio_start(",
        "pstvnc_h1_video_run_session(",
        "socket(",
        "connect(",
        "recv(",
    ):
        if forbidden in main:
            fail(f"cp2n_main_forbidden_mechanism:{forbidden}")

    require(
        entry,
        "#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE",
        "visible_mode_binding",
    )

    for needle in (
        "H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_interaction_entry.c",
        "$(BUILD_DIR)/h1_interaction_coordinator.o",
        "include mk/media-harness-h1-cumulative39-thread-census.mk",
        "h1_rfb_transport_live_visible.c",
        "h1_rfb_session_runtime_visible.c",
    ):
        require(makefile, needle, "cp2n_makefile")

    for needle in (
        "pstvnc_h1_interaction_stats_t",
        "pstvnc_input_runtime_t input_runtime",
        "pstvnc_local_controller_t local_controller",
        "pstvnc_local_ui_t local_ui",
        "pstvnc_osk_t osk",
    ):
        require(coordinator_h, needle, "coordinator_ownership")

    print("CP2N_VISIBLE_RFB_INTERACTION_SOURCE=PASS")


def discover_nm(explicit: str | None) -> str:
    candidates = []
    if explicit:
        candidates.append(explicit)
    candidates.extend(("mips64r5900el-ps2-elf-nm", "ee-nm", "nm"))
    for candidate in candidates:
        resolved = shutil.which(candidate)
        if resolved:
            return resolved
    fail("no_nm_tool_found")
    raise AssertionError("unreachable")


def symbols(nm: str, path: Path, undefined: bool) -> set[str]:
    command = [nm, "-u" if undefined else "--defined-only", str(path)]
    result = subprocess.run(
        command,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if result.returncode != 0:
        fail(f"nm_failed:{path}:{result.stdout.strip()}")
    return {line.split()[-1] for line in result.stdout.splitlines() if line.split()}


def check_objects(build_dir: Path, nm_explicit: str | None) -> None:
    main_object = build_dir / "h1_main.o"
    coordinator_object = build_dir / "h1_interaction_coordinator.o"

    for path in (main_object, coordinator_object):
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    nm = discover_nm(nm_explicit)
    main_undefined = symbols(nm, main_object, True)
    coordinator_defined = symbols(nm, coordinator_object, False)
    coordinator_undefined = symbols(nm, coordinator_object, True)

    for required in (
        "pstvnc_h1_interaction_coordinator_present",
        "pstvnc_h1_interaction_coordinator_service",
        "pstvnc_h1_interaction_coordinator_shutdown",
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service",
    ):
        if required not in main_undefined:
            fail(f"main_missing_reference:{required}")

    for required in (
        "pstvnc_h1_interaction_coordinator_present",
        "pstvnc_h1_interaction_coordinator_service",
        "pstvnc_h1_interaction_coordinator_shutdown",
    ):
        if required not in coordinator_defined:
            fail(f"coordinator_missing_definition:{required}")

    for required in (
        "pstvnc_input_runtime_init",
        "pstvnc_input_runtime_start",
        "pstvnc_input_runtime_pop_event",
        "pstvnc_local_controller_route",
        "pstvnc_local_ui_open_osk",
        "pstvnc_osk_activate_selected",
        "pstvnc_keyboard_build_tap_sequence",
        "pstvnc_rfb_session_send_pointer_event",
        "pstvnc_rfb_session_send_key_event",
        "pstvnc_local_ui_prepare_presentation",
        "pstvnc_display_prepare_gs16",
        "pstvnc_ps2_graphics_present",
    ):
        if required not in coordinator_undefined:
            fail(f"coordinator_missing_real_module_reference:{required}")

    for forbidden in (
        "pstvnc_h1_rfb_keyboard_chord_route",
        "socket",
        "connect",
        "recv",
    ):
        if forbidden in coordinator_undefined:
            fail(f"coordinator_forbidden_undefined_reference:{forbidden}")

    print("CP2N_VISIBLE_RFB_INTERACTION_OBJECTS=PASS")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--nm")
    args = parser.parse_args()

    check_source()
    if args.build_dir is not None:
        check_objects(args.build_dir, args.nm)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
