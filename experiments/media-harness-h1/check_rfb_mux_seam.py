#!/usr/bin/env python3
"""
File synopsis:
    Verifies the H1 cumulative build's RFB-I/O ownership seam before RFB is
    enabled at runtime.

The check has two layers:

1. Source/build-rule inspection proves that the through-Issue-39
   `rfb_session.c` translation unit is compiled with all three `rfb_io.h`
   symbols mechanically renamed to H1 experiment-owned mux-adapter names, and
   that the adapter object is linked into the cumulative H1 target.
2. When `--build-dir` is supplied after a PS2 build, symbol inspection proves
   that `rfb_session39.o` has unresolved references to the H1 mux names rather
   than the direct socket names, and that `h1_rfb_mux_io.o` defines those H1
   names.

This checker does not claim RFB runtime support. CP1 intentionally uses
fail-closed adapter bodies while `rfb_mode=ON` remains rejected.

Context: RFB_MUX_INTEGRATION_PREP.md and RFB_MUX_PREP_CHECKPOINTS.md.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
MAKEFILE = ROOT / "mk" / "media-harness-h1-cumulative39-thread-census.mk"
ADAPTER_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_mux_io.c"
ADAPTER_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_mux_io.h"

DIRECT_NAMES = (
    "pstvnc_rfb_io_read_exact",
    "pstvnc_rfb_io_poll_receive",
    "pstvnc_rfb_io_write_exact",
)
MUX_NAMES = (
    "pstvnc_h1_rfb_mux_io_read_exact",
    "pstvnc_h1_rfb_mux_io_poll_receive",
    "pstvnc_h1_rfb_mux_io_write_exact",
)


def fail(message: str) -> None:
    print(f"H1_RFB_MUX_SEAM=FAIL detail={message}")
    raise SystemExit(1)


def require_text(haystack: str, needle: str, label: str) -> None:
    if needle not in haystack:
        fail(f"missing_{label}:{needle}")


def check_source() -> None:
    make_text = MAKEFILE.read_text(encoding="utf-8")
    adapter_c = ADAPTER_C.read_text(encoding="utf-8")
    adapter_h = ADAPTER_H.read_text(encoding="utf-8")

    require_text(make_text, "$(BUILD_DIR)/h1_rfb_mux_io.o", "adapter_object")

    for direct, mux in zip(DIRECT_NAMES, MUX_NAMES, strict=True):
        require_text(
            make_text,
            f"-D{direct}={mux}",
            "compile_time_rfb_io_remap",
        )
        require_text(adapter_c, f"int {mux}(", "adapter_definition")
        require_text(adapter_h, f"int {mux}(", "adapter_declaration")

    # CP1 remains deliberately fail closed. This prevents a future reader from
    # mistaking the seam checkpoint for completed channel-1 transport support.
    if adapter_c.count("return -1;") < 3:
        fail("CP1_adapter_is_not_fail_closed")

    print("H1_RFB_MUX_SEAM_SOURCE=PASS")


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


def nm_output(nm: str, path: Path, undefined_only: bool) -> str:
    command = [nm]
    if undefined_only:
        command.append("-u")
    else:
        command.append("--defined-only")
    command.append(str(path))

    result = subprocess.run(
        command,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if result.returncode != 0:
        fail(f"nm_failed:{path}:{result.stdout.strip()}")
    return result.stdout


def check_objects(build_dir: Path, nm_explicit: str | None) -> None:
    rfb_object = build_dir / "rfb_session39.o"
    adapter_object = build_dir / "h1_rfb_mux_io.o"

    for path in (rfb_object, adapter_object):
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    nm = discover_nm(nm_explicit)
    rfb_undefined = nm_output(nm, rfb_object, True)
    adapter_defined = nm_output(nm, adapter_object, False)

    for direct in DIRECT_NAMES:
        if direct in rfb_undefined:
            fail(f"rfb_session_still_references_direct_io:{direct}")

    for mux in MUX_NAMES:
        if mux not in rfb_undefined:
            fail(f"rfb_session_missing_mux_reference:{mux}")
        if mux not in adapter_defined:
            fail(f"adapter_missing_definition:{mux}")

    print(f"H1_RFB_MUX_SEAM_OBJECTS=PASS nm={nm}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Verify the H1 cumulative RFB parser is bound to the experiment-owned "
            "mux I/O seam rather than direct VNC socket I/O."
        )
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help=(
            "optional PS2 build directory containing rfb_session39.o and "
            "h1_rfb_mux_io.o; when omitted only source/build-rule checks run"
        ),
    )
    parser.add_argument(
        "--nm",
        help="optional explicit nm-compatible tool; auto-discovered otherwise",
    )
    return parser


def main() -> int:
    args = build_parser().parse_args()
    check_source()
    if args.build_dir is not None:
        check_objects(args.build_dir.resolve(), args.nm)
    print("H1_RFB_MUX_SEAM=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
