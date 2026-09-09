#!/usr/bin/env python3
"""Verify CP2O is CP2N visible interaction plus the existing PCM path only."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
H1 = ROOT / "experiments/media-harness-h1"
MAIN = H1 / "h1_main_rfb_visible_interaction_pcm.c"
GATE = H1 / "h1_config_rfb_pcm_activation_gate.c"
RUNNER = H1 / "h1_mux_server_cumulative39_rfb_pcm_bridge.py"
MAKEFILE = ROOT / "mk/media-harness-h1-cp2o-visible-rfb-interaction-pcm.mk"


def fail(detail: str) -> None:
    print(f"CP2O_RFB_PCM_CONTRACT=FAIL detail={detail}")
    raise SystemExit(1)


def strip_c_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*?$", "", text, flags=re.M)


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        fail(f"missing_{label}:{needle}")


def check_source() -> None:
    main = strip_c_comments(MAIN.read_text(encoding="utf-8"))
    gate = strip_c_comments(GATE.read_text(encoding="utf-8"))
    runner = RUNNER.read_text(encoding="utf-8")
    makefile = MAKEFILE.read_text(encoding="utf-8")

    for needle in (
        "pstvnc_h1_audio_load_modules_once(",
        "pstvnc_h1_audio_start(",
        "pstvnc_h1_audio_finished(",
        "pstvnc_h1_audio_shutdown(",
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(",
        "pstvnc_h1_interaction_coordinator_present",
        "pstvnc_h1_interaction_coordinator_service",
        "PSTVNC_H1_RFB_ON_VISIBLE",
        "PSTVNC_H1_VIDEO_OFF",
    ):
        require(main, needle, "cp2o_main")

    if "pstvnc_h1_video_run_session(" in main:
        fail("cp2o_main_started_mpeg")

    for needle in (
        "config->rfb_mode != PSTVNC_H1_RFB_ON_VISIBLE",
        "config->audio_mode != PSTVNC_H1_AUDIO_PCM",
        "config->video_mode != PSTVNC_H1_VIDEO_OFF",
        "pstvnc_h1_config_validate_inner(&normalized)",
    ):
        require(gate, needle, "cp2o_gate")

    for needle in (
        "import h1_mux_server_cumulative39_rfb_bridge as rfb_base",
        "self.audio_command()",
        "base.ProducerBuffer(",
        "self._send_from(",
        "adapter.request_quiesce()",
        "adapter.wait_quiesce_complete(",
        "self.validate_result(metadata)",
    ):
        require(runner, needle, "cp2o_runner")

    if "ffmpeg" in runner:
        fail("cp2o_runner_contains_mpeg_producer")

    for needle in (
        "include mk/media-harness-h1-cumulative39-thread-census.mk",
        "h1_main_rfb_visible_interaction_pcm.c",
        "h1_config_rfb_pcm_activation_gate.c",
        "h1_rfb_transport_live_visible.c",
        "h1_rfb_session_runtime_visible.c",
        "$(BUILD_DIR)/h1_interaction_coordinator.o",
    ):
        require(makefile, needle, "cp2o_makefile")

    # The inherited cumulative target owns the canonical audio implementation.
    # CP2O must not compile a second audio-runtime source/object of its own.
    if "h1_audio_runtime_thread_census_diag.o:" in makefile or \
       "h1_audio_runtime.c -o" in makefile:
        fail("cp2o_makefile_duplicates_audio_implementation")

    print("CP2O_RFB_PCM_SOURCE=PASS")


def discover_nm(explicit: str | None) -> str:
    candidates = [explicit] if explicit else []
    candidates.extend(("mips64r5900el-ps2-elf-nm", "ee-nm", "nm"))
    for candidate in candidates:
        if candidate:
            found = shutil.which(candidate)
            if found:
                return found
    fail("no_nm_tool_found")
    raise AssertionError("unreachable")


def symbols(nm: str, path: Path, undefined: bool) -> set[str]:
    result = subprocess.run(
        [nm, "-u" if undefined else "--defined-only", str(path)],
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
    if not main_object.is_file():
        fail(f"missing_build_object:{main_object}")

    nm = discover_nm(nm_explicit)
    undefined = symbols(nm, main_object, True)
    for required in (
        "pstvnc_h1_audio_load_modules_once",
        "pstvnc_h1_audio_start",
        "pstvnc_h1_audio_finished",
        "pstvnc_h1_audio_shutdown",
        "pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service",
        "pstvnc_h1_interaction_coordinator_present",
        "pstvnc_h1_interaction_coordinator_service",
    ):
        if required not in undefined:
            fail(f"main_missing_reference:{required}")

    print("CP2O_RFB_PCM_OBJECTS=PASS")


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
