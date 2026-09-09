#!/usr/bin/env python3
"""
Verify the deliberately narrow CP2M visible-RFB + PS2 mouse/keyboard contract.

CP2M must preserve the one-physical-socket CP2L/CP2K/CP2J transport and visible
presentation path while adding only the historical transient Test11F L1+D-pad
arrow-key chord. The controller worker remains a semantic producer; pointer and
key RFB writes remain serialized by application/main at complete server-message
boundaries. AUDIO, MPEG, OSK, local UI, and Issue #40 remain outside scope.

This is an architectural/source/object gate, not hardware qualification.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
H1 = ROOT / "experiments/media-harness-h1"
MAKEFILE = ROOT / "mk/media-harness-h1-cp2m-visible-rfb-keyboard.mk"

SERVICE_H = H1 / "h1_rfb_keyboard_input_service.h"
SERVICE_C = H1 / "h1_rfb_keyboard_input_service.c"
CHORD_H = H1 / "h1_rfb_keyboard_chord.h"
CHORD_C = H1 / "h1_rfb_keyboard_chord.c"
MAIN_C = H1 / "h1_main_rfb_visible_keyboard_input.c"
MAIN_ENTRY = H1 / "h1_main_rfb_visible_keyboard_input_entry.c"

CP2L_SERVICE = H1 / "h1_rfb_input_service.c"
CP2L_MAIN = H1 / "h1_main_rfb_visible_input.c"
CP2K_MAIN = H1 / "h1_main_rfb_visible.c"


def fail(message: str) -> None:
    print(f"CP2M_VISIBLE_RFB_KEYBOARD_CONTRACT=FAIL detail={message}")
    raise SystemExit(1)


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        fail(f"missing_{label}:{needle}")


def strip_c_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*?$", "", text, flags=re.M)


def check_source() -> None:
    service_h = SERVICE_H.read_text(encoding="utf-8")
    service_code = strip_c_comments(SERVICE_C.read_text(encoding="utf-8"))
    chord_h = CHORD_H.read_text(encoding="utf-8")
    chord_code = strip_c_comments(CHORD_C.read_text(encoding="utf-8"))
    main_code = strip_c_comments(MAIN_C.read_text(encoding="utf-8"))
    main_entry = MAIN_ENTRY.read_text(encoding="utf-8")
    cp2l_service = strip_c_comments(CP2L_SERVICE.read_text(encoding="utf-8"))
    cp2l_main = strip_c_comments(CP2L_MAIN.read_text(encoding="utf-8"))
    cp2k_main = strip_c_comments(CP2K_MAIN.read_text(encoding="utf-8"))
    makefile = MAKEFILE.read_text(encoding="utf-8")

    for forbidden in (
        "socket(",
        "connect(",
        "recv(",
        "pstvnc_ps2_graphics_",
        "pstvnc_h1_audio_",
        "pstvnc_h1_video_",
        "pstvnc_osk_",
        "pstvnc_local_ui_",
    ):
        if forbidden in service_code:
            fail(f"keyboard_service_forbidden_owner:{forbidden}")

    for needle in (
        "pstvnc_input_runtime_init(",
        "pstvnc_input_runtime_start(",
        "pstvnc_input_runtime_pop_event(",
        "pstvnc_input_runtime_last_error(",
        "pstvnc_input_runtime_shutdown(",
        "pstvnc_input_runtime_suspend_mouse_interpretation(",
        "pstvnc_input_runtime_rebase_suspended_mouse_state(",
        "pstvnc_input_runtime_resume_mouse_interpretation(",
        "PSTVNC_INPUT_EVENT_CONTROLLER_STATE",
        "PSTVNC_INPUT_EVENT_MOUSE_UPDATE",
        "PSTVNC_INPUT_EVENT_KEYBOARD_TAP",
        "pstvnc_h1_rfb_keyboard_chord_route(",
        "pstvnc_keyboard_build_tap_sequence(",
        "pstvnc_rfb_session_send_key_event(",
        "pstvnc_rfb_session_send_pointer_event(",
        "PSTVNC_RFB_POINTER_BUTTON_LEFT",
        "PSTVNC_RFB_POINTER_BUTTON_RIGHT",
    ):
        require(service_code, needle, "keyboard_service_mechanism")

    for needle in (
        "PSTVNC_CONTROLLER_BUTTON_L1",
        "PSTVNC_CONTROLLER_BUTTON_UP",
        "PSTVNC_CONTROLLER_BUTTON_DOWN",
        "PSTVNC_CONTROLLER_BUTTON_LEFT",
        "PSTVNC_CONTROLLER_BUTTON_RIGHT",
        "PSTVNC_H1_KEYSYM_UP",
        "PSTVNC_H1_KEYSYM_DOWN",
        "PSTVNC_H1_KEYSYM_LEFT",
        "PSTVNC_H1_KEYSYM_RIGHT",
        "state->connection_epoch_started",
        "result->enter_chord",
        "result->exit_chord",
    ):
        require(chord_code, needle, "test11f_chord")

    for forbidden in (
        "pstvnc_rfb_",
        "pstvnc_input_runtime_",
        "socket(",
        "connect(",
        "recv(",
        "PAD_",
    ):
        if forbidden in chord_code:
            fail(f"pure_chord_forbidden_owner:{forbidden}")

    require(
        chord_h,
        "PSTVNC_H1_RFB_KEYBOARD_CHORD_MAX_TAPS 4u",
        "bounded_chord_result",
    )
    require(service_h, "keyboard_taps_sent", "keyboard_stats")
    require(service_h, "keyboard_chord_entries", "keyboard_stats")

    for needle in (
        "pstvnc_ps2_graphics_init()",
        "pstvnc_display_prepare_gs16(",
        "pstvnc_ps2_graphics_present(",
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(",
        "pstvnc_h1_rfb_keyboard_input_service_service",
        "pstvnc_h1_rfb_keyboard_input_service_shutdown(",
        "PSTVNC_H1_AUDIO_OFF",
        "PSTVNC_H1_VIDEO_OFF",
        "H1_CP2M_VISIBLE_RFB_KEYBOARD_START",
        "keyboard_taps_sent",
        "keyboard_messages_sent",
    ):
        require(main_code, needle, "cp2m_main")

    for forbidden in (
        "pstvnc_h1_audio_start(",
        "pstvnc_h1_video_chassis_init(",
        "pstvnc_h1_video_run_session(",
        "pstvnc_osk_",
        "pstvnc_local_ui_",
    ):
        if forbidden in main_code:
            fail(f"cp2m_main_forbidden_owner:{forbidden}")

    require(
        main_entry,
        "#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE",
        "mode2_scope",
    )

    for needle in (
        "H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_keyboard_input_entry.c",
        "EXTRA_EE_OBJS +=",
        "$(BUILD_DIR)/h1_rfb_keyboard_chord.o",
        "$(BUILD_DIR)/h1_rfb_keyboard_input_service.o",
        "$(EE_BIN):",
        "h1_rfb_transport_live_visible.c",
        "h1_rfb_session_runtime_visible.c",
        "include mk/media-harness-h1-cumulative39-thread-census.mk",
    ):
        require(makefile, needle, "cp2m_makefile")

    if "pstvnc_rfb_session_send_key_event(" in cp2l_service:
        fail("cp2l_mouse_service_silently_gained_keyboard")

    if "pstvnc_h1_rfb_keyboard_input_service" in cp2l_main:
        fail("cp2l_main_silently_gained_cp2m_keyboard")

    if "run_with_presenter_and_service" in cp2k_main:
        fail("cp2k_presenter_silently_gained_application_service")

    print("CP2M_VISIBLE_RFB_KEYBOARD_SOURCE=PASS")


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
    paths = {
        "main": build_dir / "h1_main.o",
        "service": build_dir / "h1_rfb_keyboard_input_service.o",
        "chord": build_dir / "h1_rfb_keyboard_chord.o",
    }
    for path in paths.values():
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    nm = discover_nm(nm_explicit)
    defined = {name: symbols(nm, path, False) for name, path in paths.items()}
    undefined = {name: symbols(nm, path, True) for name, path in paths.items()}

    for required in (
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service",
        "pstvnc_h1_rfb_keyboard_input_service_service",
        "pstvnc_h1_rfb_keyboard_input_service_shutdown",
    ):
        if required not in undefined["main"]:
            fail(f"main_missing_cp2m_reference:{required}")

    for required in (
        "pstvnc_input_runtime_init",
        "pstvnc_input_runtime_start",
        "pstvnc_input_runtime_pop_event",
        "pstvnc_input_runtime_last_error",
        "pstvnc_input_runtime_shutdown",
        "pstvnc_input_runtime_suspend_mouse_interpretation",
        "pstvnc_input_runtime_rebase_suspended_mouse_state",
        "pstvnc_input_runtime_resume_mouse_interpretation",
        "pstvnc_h1_rfb_keyboard_chord_route",
        "pstvnc_keyboard_build_tap_sequence",
        "pstvnc_rfb_session_send_key_event",
        "pstvnc_rfb_session_send_pointer_event",
    ):
        if required not in undefined["service"]:
            fail(f"service_missing_reference:{required}")

    for forbidden in (
        "pstvnc_ps2_graphics_present",
        "socket",
        "connect",
        "recv",
    ):
        if forbidden in undefined["service"]:
            fail(f"service_forbidden_object_reference:{forbidden}")

    if "pstvnc_h1_rfb_keyboard_chord_route" not in defined["chord"]:
        fail("chord_router_not_defined")

    for symbol in undefined["chord"]:
        if symbol.startswith("pstvnc_rfb_") or \
           symbol.startswith("pstvnc_input_runtime_"):
            fail(f"pure_chord_has_cross_domain_reference:{symbol}")

    print(f"CP2M_VISIBLE_RFB_KEYBOARD_OBJECTS=PASS nm={nm}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--nm")
    args = parser.parse_args()

    check_source()
    if args.build_dir is not None:
        check_objects(args.build_dir, args.nm)

    print("CP2M_VISIBLE_RFB_KEYBOARD_CONTRACT=PASS")
    print("CP2M_PHYSICAL_PSTV_STREAMS=1")
    print("CP2M_RFB_MODE=2_VISIBLE")
    print("CP2M_MOUSE=CP2L_THROUGH_ISSUE39_SEMANTICS")
    print("CP2M_KEYBOARD=TEST11F_TRANSIENT_L1_DPAD_ARROWS")
    print("CP2M_AUDIO=OFF")
    print("CP2M_MPEG=OFF")
    print("CP2M_OSK=OFF")
    print("CP2M_LOCAL_UI=OFF")
    print("CP2M_ISSUE40=NO")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
