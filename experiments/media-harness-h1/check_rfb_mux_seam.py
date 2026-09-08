#!/usr/bin/env python3
"""
File synopsis:
    Verifies the H1 cumulative build's RFB-I/O ownership seam and resident
    logical-channel preparation before RFB is enabled at runtime.

The check has two layers:

1. Source/build-rule inspection proves that the through-Issue-39
   `rfb_session.c` translation unit is compiled with all three `rfb_io.h`
   symbols mechanically renamed to H1 experiment-owned mux-adapter names, that
   the fail-closed adapter is linked, and that the host-tested logical RFB
   channel mechanics are also part of the cumulative PS2 source population.
2. When `--build-dir` is supplied after a PS2 build, symbol inspection proves
   that `rfb_session39.o` has unresolved references to the H1 mux names rather
   than the direct socket names, `h1_rfb_mux_io.o` defines those H1 names, and
   `h1_rfb_channel.o` contains the expected logical-channel implementation.

This checker does not claim RFB runtime support. CONFIG still rejects
`rfb_mode=ON`, and the adapter remains fail closed until a later checkpoint
binds the resident channel mechanics to H1's live receiver/send owners.

Context: RFB_MUX_INTEGRATION_PREP.md and RFB_MUX_PREP_CHECKPOINTS.md.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
MAKEFILE = ROOT / "mk" / "media-harness-h1-cumulative39-thread-census.mk"
ADAPTER_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_mux_io.c"
ADAPTER_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_mux_io.h"
CHANNEL_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_channel.c"
CHANNEL_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_channel.h"

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
CHANNEL_NAMES = (
    "pstvnc_h1_rfb_channel_init",
    "pstvnc_h1_rfb_channel_accept_data",
    "pstvnc_h1_rfb_channel_poll",
    "pstvnc_h1_rfb_channel_read_available",
    "pstvnc_h1_rfb_channel_take_credit",
    "pstvnc_h1_rfb_channel_write_logical",
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
    channel_c = CHANNEL_C.read_text(encoding="utf-8")
    channel_h = CHANNEL_H.read_text(encoding="utf-8")

    require_text(make_text, "$(BUILD_DIR)/h1_rfb_mux_io.o", "adapter_object")
    require_text(make_text, "$(BUILD_DIR)/h1_rfb_channel.o", "channel_object")
    require_text(
        make_text,
        "experiments/media-harness-h1/h1_rfb_channel.c",
        "channel_build_rule",
    )

    for direct, mux in zip(DIRECT_NAMES, MUX_NAMES, strict=True):
        require_text(
            make_text,
            f"-D{direct}={mux}",
            "compile_time_rfb_io_remap",
        )
        require_text(adapter_c, f"int {mux}(", "adapter_definition")
        require_text(adapter_h, f"int {mux}(", "adapter_declaration")

    for name in CHANNEL_NAMES:
        require_text(channel_c, f"{name}(", "channel_definition")
        require_text(channel_h, f"{name}(", "channel_declaration")

    # The adapter remains deliberately fail closed. This prevents a future
    # reader from mistaking resident channel mechanics for completed live
    # channel-1 transport support.
    if adapter_c.count("return -1;") < 3:
        fail("adapter_is_not_fail_closed_before_runtime_binding")

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
    channel_object = build_dir / "h1_rfb_channel.o"

    for path in (rfb_object, adapter_object, channel_object):
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    nm = discover_nm(nm_explicit)
    rfb_undefined = nm_output(nm, rfb_object, True)
    adapter_defined = nm_output(nm, adapter_object, False)
    channel_defined = nm_output(nm, channel_object, False)

    for direct in DIRECT_NAMES:
        if direct in rfb_undefined:
            fail(f"rfb_session_still_references_direct_io:{direct}")

    for mux in MUX_NAMES:
        if mux not in rfb_undefined:
            fail(f"rfb_session_missing_mux_reference:{mux}")
        if mux not in adapter_defined:
            fail(f"adapter_missing_definition:{mux}")

    for name in CHANNEL_NAMES:
        if name not in channel_defined:
            fail(f"channel_object_missing_definition:{name}")

    print(f"H1_RFB_MUX_SEAM_OBJECTS=PASS nm={nm}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Verify the H1 cumulative RFB parser is bound to the experiment-owned "
            "mux I/O seam and that logical RFB channel mechanics are resident "
            "without claiming live RFB support."
        )
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help=(
            "optional PS2 build directory containing rfb_session39.o, "
            "h1_rfb_mux_io.o, and h1_rfb_channel.o; when omitted only "
            "source/build-rule checks run"
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
