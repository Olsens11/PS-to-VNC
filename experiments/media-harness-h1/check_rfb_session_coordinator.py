#!/usr/bin/env python3
"""
File synopsis:
    Verifies the cumulative H1 headless RFB session coordinator and clean RFB
    quiesce mechanism while the public hardware activation gate remains closed.

The check is architectural rather than a substitute for hardware. Source checks
prove that the coordinator allocates only CPU framebuffer authority, uses the
unchanged through-Issue-39 RFB session API over the already-bound mux identity,
keeps first-stage RFB mutually exclusive with AUDIO/MPEG, stops at complete RFB
message boundaries, and completes the four zero-length channel-1 quiesce markers
without starting graphics/input/UI ownership. Optional object checks prove the
pinned PS2 build actually links those references and still does not link app.c.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
MAKEFILE = ROOT / "mk" / "media-harness-h1-cumulative39-thread-census.mk"
MAIN_C = ROOT / "experiments" / "media-harness-h1" / "h1_main.c"
RUNTIME_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_session_runtime.c"
RUNTIME_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_session_runtime.h"
SNAPSHOT_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_transport_snapshot.c"
LIVE_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_transport_live.c"
LIVE_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_transport_live.h"
CONFIG_C = ROOT / "experiments" / "media-harness-h1" / "h1_config.c"
TRANSPORT_C = ROOT / "experiments" / "media-harness-h1" / "h1_transport_runtime.c"
PI_ADAPTER = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_session_adapter.py"
PI_RUNNER = (
    ROOT
    / "experiments"
    / "media-harness-h1"
    / "h1_mux_server_cumulative39_rfb_bridge.py"
)


def fail(message: str) -> None:
    print(f"H1_RFB_SESSION_COORDINATOR=FAIL detail={message}")
    raise SystemExit(1)


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        fail(f"missing_{label}:{needle}")


def strip_c_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*?$", "", text, flags=re.M)


def check_source() -> None:
    make = MAKEFILE.read_text(encoding="utf-8")
    main = MAIN_C.read_text(encoding="utf-8")
    runtime = RUNTIME_C.read_text(encoding="utf-8")
    runtime_code = strip_c_comments(runtime)
    runtime_h = RUNTIME_H.read_text(encoding="utf-8")
    snapshot_code = strip_c_comments(SNAPSHOT_C.read_text(encoding="utf-8"))
    live = LIVE_C.read_text(encoding="utf-8")
    live_code = strip_c_comments(live)
    live_h = LIVE_H.read_text(encoding="utf-8")
    config = CONFIG_C.read_text(encoding="utf-8")
    transport = TRANSPORT_C.read_text(encoding="utf-8")
    pi_adapter = PI_ADAPTER.read_text(encoding="utf-8")
    pi_runner = PI_RUNNER.read_text(encoding="utf-8")

    for obj in (
        "h1_rfb_transport_live.o",
        "h1_rfb_transport_snapshot.o",
        "h1_rfb_session_runtime.o",
    ):
        require(make, f"$(BUILD_DIR)/{obj}", "linked_object")

    require(main, '#include "h1_rfb_session_runtime.h"', "main_runtime_include")
    require(main, "pstvnc_h1_rfb_session_runtime_run(", "main_runtime_call")
    require(main, "PSTVNC_H1_RFB_ON_RESERVED", "main_rfb_gate_branch")
    require(main, "H1_RFB=HYBRID_NOT_ENABLED", "first_stage_hybrid_rejection")
    require(main, "h1_wait_for_transport_end(&transport)", "post_quiesce_media_end_wait")

    for needle in (
        "pstvnc_framebuffer_init(",
        "pstvnc_framebuffer_set_geometry(",
        "pstvnc_rfb_session_start(",
        "transport->socket_fd",
        "pstvnc_rfb_session_receive_initial_frame(",
        "pstvnc_rfb_session_try_receive_update(",
        "pstvnc_rfb_session_request_update(",
        "pstvnc_h1_rfb_transport_snapshot(",
        "pstvnc_h1_rfb_transport_send_quiesce_boundary(",
        "pstvnc_h1_rfb_transport_send_quiesce_complete(",
        "snapshot.queue_current != 0u",
    ):
        require(runtime_code, needle, "headless_runtime_mechanism")

    for forbidden in (
        "pstvnc_ps2_graphics_",
        "pstvnc_input_runtime_",
        "pstvnc_local_ui_",
        "pstvnc_osk_",
        "pstvnc_keyboard_",
        "socket(",
        "connect(",
        "recv(",
        "send(",
    ):
        if forbidden in runtime_code:
            fail(f"headless_runtime_forbidden_owner:{forbidden}")

    for forbidden in ("socket(", "connect(", "recv(", "send("):
        if forbidden in snapshot_code:
            fail(f"snapshot_contains_socket_io:{forbidden}")

    # Zero-length channel-1 DATA is lifecycle control only. Non-empty DATA must
    # still go through the raw byte queue and unchanged parser seam.
    for needle in (
        "if (payload_length == 0u)",
        "h1_rfb_accept_quiesce_marker(runtime)",
        "runtime->rfb_quiesce_request_received = 1u",
        "runtime->rfb_quiesce_commit_received = 1u",
        "pstvnc_h1_transport_send_frame_internal(",
        "PSTVNC_TRANSPORT_FRAME_DATA",
        "PSTVNC_TRANSPORT_CHANNEL_RFB",
        "runtime->rfb_quiesce_boundary_sent = 1u",
        "runtime->rfb_quiesce_complete_sent = 1u",
    ):
        require(live_code, needle, "rfb_quiesce_transport")

    require(
        live_h,
        "pstvnc_h1_rfb_transport_snapshot(",
        "locked_snapshot_declaration",
    )
    require(
        live_h,
        "pstvnc_h1_rfb_transport_send_quiesce_boundary(",
        "quiesce_boundary_declaration",
    )
    require(
        live_h,
        "pstvnc_h1_rfb_transport_send_quiesce_complete(",
        "quiesce_complete_declaration",
    )
    require(
        runtime_h,
        "quiesce_complete_sent",
        "parser_quiesce_progress_stats",
    )

    # Pi must stop+join its upstream raw reader on BOUNDARY before COMMIT and
    # send ordinary MEDIA_END only after COMPLETE.
    for needle in (
        "self.bridge.stop()",
        "self.quiesce_boundary_received = True",
        "self.quiesce_commit_sent = True",
        "self.quiesce_complete_received = True",
        "self.session.send_frame(base.FRAME_DATA, CHANNEL_RFB, b\"\")",
    ):
        require(pi_adapter, needle, "pi_quiesce_adapter")

    for needle in (
        "adapter.request_quiesce()",
        "adapter.wait_quiesce_complete(",
        "metadata = self._send_rfb_media_end()",
        "self.validate_result(metadata)",
    ):
        require(pi_runner, needle, "pi_quiesce_runner")

    # Public hardware activation remains explicitly fail closed at this checkpoint.
    require(
        config,
        "config->rfb_mode != PSTVNC_H1_RFB_OFF",
        "config_gate_still_closed",
    )
    if "PSTVNC_TRANSPORT_CAP_RFB" in transport:
        fail("rfb_capability_advertised_before_activation_checkpoint")

    if "app39.o" in make or "src/app.c" in make:
        fail("old_app_coordinator_linked")

    print("H1_RFB_SESSION_COORDINATOR_SOURCE=PASS")


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
        "snapshot": build_dir / "h1_rfb_transport_snapshot.o",
        "live": build_dir / "h1_rfb_transport_live.o",
    }
    for path in paths.values():
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    if (build_dir / "app39.o").exists() or (build_dir / "app.o").exists():
        fail("old_app_object_present")

    nm = discover_nm(nm_explicit)
    defined = {name: symbols(nm, path, False) for name, path in paths.items()}
    undefined = {name: symbols(nm, path, True) for name, path in paths.items()}

    if "pstvnc_h1_rfb_session_runtime_run" not in defined["runtime"]:
        fail("runtime_run_definition_missing")
    if "pstvnc_h1_rfb_session_runtime_run" not in undefined["main"]:
        fail("main_not_linked_to_rfb_runtime")
    if "pstvnc_h1_rfb_transport_snapshot" not in defined["snapshot"]:
        fail("snapshot_definition_missing")
    if "pstvnc_h1_rfb_transport_snapshot" not in undefined["runtime"]:
        fail("runtime_not_using_locked_snapshot")

    for required in (
        "pstvnc_rfb_session_start",
        "pstvnc_rfb_session_receive_initial_frame",
        "pstvnc_rfb_session_try_receive_update",
        "pstvnc_rfb_session_request_update",
        "pstvnc_framebuffer_init",
        "pstvnc_framebuffer_set_geometry",
        "pstvnc_h1_rfb_transport_send_quiesce_boundary",
        "pstvnc_h1_rfb_transport_send_quiesce_complete",
    ):
        if required not in undefined["runtime"]:
            fail(f"runtime_missing_clean_module_reference:{required}")

    for required in (
        "pstvnc_h1_rfb_transport_send_quiesce_boundary",
        "pstvnc_h1_rfb_transport_send_quiesce_complete",
    ):
        if required not in defined["live"]:
            fail(f"live_quiesce_definition_missing:{required}")

    for forbidden in (
        "pstvnc_ps2_graphics_init",
        "pstvnc_ps2_graphics_present",
        "pstvnc_input_runtime_start",
        "pstvnc_app_run",
    ):
        if forbidden in undefined["runtime"]:
            fail(f"runtime_object_references_forbidden_owner:{forbidden}")

    print(f"H1_RFB_SESSION_COORDINATOR_OBJECTS=PASS nm={nm}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--nm")
    args = parser.parse_args()

    check_source()
    if args.build_dir is not None:
        check_objects(args.build_dir, args.nm)

    print("H1_RFB_SESSION_COORDINATOR=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
