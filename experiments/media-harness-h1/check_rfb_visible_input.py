#!/usr/bin/env python3
"""
Verify the deliberately narrow CP2L visible-RFB + PS2 mouse-input contract.

This is an architectural/source/object gate, not hardware qualification. CP2L
must retain one physical PSTV socket, CP2K visible presentation, CP2J RFB mux
transport mechanics, and the through-Issue-39 semantic controller runtime while
adding only main-thread mouse/pointer RFB publication at complete server-message
boundaries. AUDIO, MPEG, keyboard, OSK, and local UI remain inactive.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
H1 = ROOT / "experiments/media-harness-h1"
MAKEFILE = ROOT / "mk/media-harness-h1-cp2l-visible-rfb-input.mk"
RUNTIME_H = H1 / "h1_rfb_session_runtime.h"
RUNTIME_C = H1 / "h1_rfb_session_runtime.c"
INPUT_H = H1 / "h1_rfb_input_service.h"
INPUT_C = H1 / "h1_rfb_input_service.c"
MAIN_C = H1 / "h1_main_rfb_visible_input.c"
MAIN_ENTRY = H1 / "h1_main_rfb_visible_input_entry.c"
CP2K_MAIN = H1 / "h1_main_rfb_visible.c"


def fail(message: str) -> None:
    print(f"CP2L_VISIBLE_RFB_INPUT_CONTRACT=FAIL detail={message}")
    raise SystemExit(1)


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        fail(f"missing_{label}:{needle}")


def strip_c_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*?$", "", text, flags=re.M)


def check_source() -> None:
    runtime_h = RUNTIME_H.read_text(encoding="utf-8")
    runtime_c = RUNTIME_C.read_text(encoding="utf-8")
    runtime_code = strip_c_comments(runtime_c)
    input_h = INPUT_H.read_text(encoding="utf-8")
    input_c = INPUT_C.read_text(encoding="utf-8")
    input_code = strip_c_comments(input_c)
    main_c = MAIN_C.read_text(encoding="utf-8")
    main_code = strip_c_comments(main_c)
    main_entry = MAIN_ENTRY.read_text(encoding="utf-8")
    cp2k_main = CP2K_MAIN.read_text(encoding="utf-8")
    makefile = MAKEFILE.read_text(encoding="utf-8")

    for needle in (
        "pstvnc_h1_rfb_service_callback_t",
        "application_service_calls",
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(",
    ):
        require(runtime_h, needle, "runtime_service_api")

    for needle in (
        "h1_rfb_service_application(",
        "runtime->stats.application_service_calls++",
        "transport->rfb_quiesce_request_received",
        "PSTVNC_RFB_SESSION_RECEIVE_IDLE",
        "pstvnc_rfb_session_request_update(",
    ):
        require(runtime_code, needle, "runtime_service_boundary")

    # The service callback must remain an application-side seam. It does not
    # gain physical receive/socket ownership or any display/media/UI mechanism.
    for forbidden in (
        "socket(",
        "connect(",
        "recv(",
        "pstvnc_rfb_session_send_key_event(",
        "pstvnc_ps2_graphics_",
        "pstvnc_h1_audio_",
        "pstvnc_h1_video_",
        "pstvnc_osk_",
        "pstvnc_local_ui_",
    ):
        if forbidden in input_code:
            fail(f"input_service_forbidden_owner:{forbidden}")

    for needle in (
        "pstvnc_input_runtime_init(",
        "pstvnc_input_runtime_start(",
        "pstvnc_input_runtime_pop_event(",
        "pstvnc_input_runtime_last_error(",
        "pstvnc_input_runtime_shutdown(",
        "PSTVNC_INPUT_EVENT_CONTROLLER_STATE",
        "PSTVNC_INPUT_EVENT_MOUSE_UPDATE",
        "pstvnc_rfb_session_send_pointer_event(",
        "PSTVNC_RFB_POINTER_BUTTON_LEFT",
        "PSTVNC_RFB_POINTER_BUTTON_RIGHT",
    ):
        require(input_code, needle, "input_service_mechanism")

    # Keyboard is fail-closed vocabulary only; no keyboard serializer is legal.
    require(input_code, "PSTVNC_INPUT_EVENT_KEYBOARD_TAP", "keyboard_fail_closed")
    if "pstvnc_rfb_session_send_key_event(" in input_code:
        fail("keyboard_serializer_enabled")

    for needle in (
        "pstvnc_ps2_graphics_init()",
        "pstvnc_display_prepare_gs16(",
        "pstvnc_ps2_graphics_present(",
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(",
        "pstvnc_h1_rfb_input_service_service",
        "pstvnc_h1_rfb_input_service_shutdown(",
        "PSTVNC_H1_AUDIO_OFF",
        "PSTVNC_H1_VIDEO_OFF",
        "H1_CP2L_VISIBLE_RFB_INPUT_START",
    ):
        require(main_code, needle, "cp2l_main")

    for forbidden in (
        "pstvnc_h1_audio_start(",
        "pstvnc_h1_video_chassis_init(",
        "pstvnc_h1_video_run_session(",
        "pstvnc_rfb_session_send_key_event(",
        "pstvnc_osk_",
        "pstvnc_local_ui_",
    ):
        if forbidden in main_code:
            fail(f"cp2l_main_forbidden_owner:{forbidden}")

    # CP2K's tested coordinator remains presenter-only in source. The new service
    # seam must not silently activate input in the CP2K entry point.
    if "run_with_presenter_and_service" in cp2k_main or \
       "pstvnc_h1_rfb_input_service" in cp2k_main:
        fail("cp2k_presenter_silently_gained_input")

    require(
        main_entry,
        "#define PSTVNC_H1_RFB_ON_RESERVED PSTVNC_H1_RFB_ON_VISIBLE",
        "mode2_scope",
    )

    for needle in (
        "H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_input_entry.c",
        "h1_rfb_input_service.o",
        "h1_rfb_transport_live_visible.c",
        "h1_rfb_session_runtime_visible.c",
        "include mk/media-harness-h1-cumulative39-thread-census.mk",
    ):
        require(makefile, needle, "cp2l_makefile")

    # Ensure the header itself describes a single semantic-input owner rather
    # than growing a generic transport abstraction.
    require(input_h, "pstvnc_input_runtime_t input_runtime", "input_owner")
    require(input_h, "pointer_messages_sent", "input_stats")

    print("CP2L_VISIBLE_RFB_INPUT_SOURCE=PASS")


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
        "runtime": build_dir / "h1_rfb_session_runtime.o",
        "input_service": build_dir / "h1_rfb_input_service.o",
    }
    for path in paths.values():
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    nm = discover_nm(nm_explicit)
    defined = {name: symbols(nm, path, False) for name, path in paths.items()}
    undefined = {name: symbols(nm, path, True) for name, path in paths.items()}

    if "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service" not in \
            defined["runtime"]:
        fail("runtime_service_entry_not_defined")

    for required in (
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service",
        "pstvnc_h1_rfb_input_service_service",
        "pstvnc_h1_rfb_input_service_shutdown",
    ):
        if required not in undefined["main"]:
            fail(f"main_missing_cp2l_reference:{required}")

    for required in (
        "pstvnc_input_runtime_init",
        "pstvnc_input_runtime_start",
        "pstvnc_input_runtime_pop_event",
        "pstvnc_input_runtime_last_error",
        "pstvnc_input_runtime_shutdown",
        "pstvnc_rfb_session_send_pointer_event",
    ):
        if required not in undefined["input_service"]:
            fail(f"input_service_missing_reference:{required}")

    for forbidden in (
        "pstvnc_rfb_session_send_key_event",
        "pstvnc_ps2_graphics_present",
        "socket",
        "connect",
        "recv",
    ):
        if forbidden in undefined["input_service"]:
            fail(f"input_service_forbidden_object_reference:{forbidden}")

    print(f"CP2L_VISIBLE_RFB_INPUT_OBJECTS=PASS nm={nm}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--nm")
    args = parser.parse_args()

    check_source()
    if args.build_dir is not None:
        check_objects(args.build_dir, args.nm)

    print("CP2L_VISIBLE_RFB_INPUT_CONTRACT=PASS")
    print("CP2L_PHYSICAL_PSTV_STREAMS=1")
    print("CP2L_RFB_MODE=2_VISIBLE")
    print("CP2L_INPUT=MOUSE_POINTER_ONLY_THROUGH_ISSUE39_SEMANTICS")
    print("CP2L_AUDIO=OFF")
    print("CP2L_MPEG=OFF")
    print("CP2L_KEYBOARD=OFF")
    print("CP2L_OSK=OFF")
    print("CP2L_LOCAL_UI=OFF")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
