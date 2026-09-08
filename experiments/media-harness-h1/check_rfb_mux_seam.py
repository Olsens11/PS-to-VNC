#!/usr/bin/env python3
"""
File synopsis:
    Verifies the H1 cumulative build's RFB-I/O ownership seam, resident logical
    channel mechanics, dormant runtime-resource ownership, and CP2D lifecycle
    wrapper before RFB is enabled at runtime.

The check has two layers:

1. Source/build-rule inspection proves that the through-Issue-39
   `rfb_session.c` translation unit is compiled with all three `rfb_io.h`
   symbols mechanically renamed to H1 experiment-owned mux-adapter names, that
   the fail-closed adapter is linked, that the host-tested logical RFB channel
   mechanics are resident, that the dormant queue/semaphore resource bundle is
   part of the cumulative PS2 source population, and that the cumulative-only
   public transport lifecycle is wrapped around renamed inner start/shutdown
   definitions.
2. When `--build-dir` is supplied after a PS2 build, symbol inspection proves
   the actual object ownership for all of those pieces.

This checker does not claim RFB runtime support. CONFIG still rejects
`rfb_mode=ON`, the adapter remains fail closed, and receiver DATA dispatch,
channel-1 CREDIT/DATA, and Pi bridge behavior remain absent.

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
RESOURCES_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_runtime_resources.c"
RESOURCES_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_runtime_resources.h"
LIFECYCLE_C = ROOT / "experiments" / "media-harness-h1" / "h1_transport_runtime_rfb_lifecycle.c"
TRANSPORT_H = ROOT / "experiments" / "media-harness-h1" / "h1_transport_runtime.h"
CONFIG_C = ROOT / "experiments" / "media-harness-h1" / "h1_config.c"

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
RESOURCE_NAMES = (
    "pstvnc_h1_rfb_runtime_resources_init",
    "pstvnc_h1_rfb_runtime_resources_activate",
    "pstvnc_h1_rfb_runtime_resources_release",
)
PUBLIC_LIFECYCLE_NAMES = (
    "pstvnc_h1_transport_start",
    "pstvnc_h1_transport_shutdown",
)
INNER_LIFECYCLE_NAMES = (
    "pstvnc_h1_transport_start_inner",
    "pstvnc_h1_transport_shutdown_inner",
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
    resources_c = RESOURCES_C.read_text(encoding="utf-8")
    resources_h = RESOURCES_H.read_text(encoding="utf-8")
    lifecycle_c = LIFECYCLE_C.read_text(encoding="utf-8")
    transport_h = TRANSPORT_H.read_text(encoding="utf-8")
    config_c = CONFIG_C.read_text(encoding="utf-8")

    require_text(make_text, "$(BUILD_DIR)/h1_rfb_mux_io.o", "adapter_object")
    require_text(make_text, "$(BUILD_DIR)/h1_rfb_channel.o", "channel_object")
    require_text(make_text, "$(BUILD_DIR)/h1_rfb_runtime_resources.o", "resources_object")
    require_text(make_text, "$(BUILD_DIR)/h1_transport_runtime_rfb_lifecycle.o", "lifecycle_object")
    require_text(
        make_text,
        "experiments/media-harness-h1/h1_rfb_channel.c",
        "channel_build_rule",
    )
    require_text(
        make_text,
        "experiments/media-harness-h1/h1_rfb_runtime_resources.c",
        "resources_build_rule",
    )
    require_text(
        make_text,
        "experiments/media-harness-h1/h1_transport_runtime_rfb_lifecycle.c",
        "lifecycle_build_rule",
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

    for name in RESOURCE_NAMES:
        require_text(resources_c, f"{name}(", "resource_definition")
        require_text(resources_h, f"{name}(", "resource_declaration")

    for public, inner in zip(PUBLIC_LIFECYCLE_NAMES, INNER_LIFECYCLE_NAMES, strict=True):
        require_text(
            make_text,
            f"-D{public}={inner}",
            "transport_lifecycle_inner_remap",
        )
        require_text(lifecycle_c, f"int {public}(", "public_lifecycle_wrapper")
        require_text(lifecycle_c, f"{inner}(", "inner_lifecycle_call")

    require_text(
        transport_h,
        "pstvnc_h1_rfb_runtime_resources_t rfb_resources;",
        "embedded_rfb_resource_bundle",
    )
    require_text(
        lifecycle_c,
        "pstvnc_h1_rfb_runtime_resources_init(&runtime->rfb_resources);",
        "lifecycle_resource_init",
    )
    require_text(
        lifecycle_c,
        "pstvnc_h1_rfb_runtime_resources_activate(",
        "lifecycle_resource_activate",
    )
    require_text(
        lifecycle_c,
        "pstvnc_h1_rfb_runtime_resources_release(",
        "lifecycle_resource_release",
    )

    # Preparation remains fail closed: CONFIG still rejects RFB ON and the mux
    # adapter still cannot carry bytes. Lifecycle ownership is therefore not a
    # claim of operational channel-1 transport.
    require_text(
        config_c,
        "config->rfb_mode != PSTVNC_H1_RFB_OFF",
        "rfb_config_gate",
    )
    if adapter_c.count("return -1;") < 3:
        fail("adapter_is_not_fail_closed_before_runtime_binding")

    # The dormant bundle must retain the evidence-based capacity and an OFF
    # path that succeeds without creating a queue. This guards against turning
    # lifecycle wiring into implicit channel activation.
    require_text(
        resources_c,
        "PSTVNC_H1_RFB_QUEUE_REFERENCE_BYTES",
        "evidence_based_rfb_capacity",
    )
    require_text(resources_c, "if (!enabled)", "disabled_noop_branch")

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


def nm_symbols(output: str) -> set[str]:
    """Return exact symbol tokens from ordinary nm or nm -u output."""
    symbols: set[str] = set()
    for line in output.splitlines():
        fields = line.split()
        if fields:
            symbols.add(fields[-1])
    return symbols


def check_objects(build_dir: Path, nm_explicit: str | None) -> None:
    rfb_object = build_dir / "rfb_session39.o"
    adapter_object = build_dir / "h1_rfb_mux_io.o"
    channel_object = build_dir / "h1_rfb_channel.o"
    resources_object = build_dir / "h1_rfb_runtime_resources.o"
    transport_object = build_dir / "h1_transport_runtime.o"
    lifecycle_object = build_dir / "h1_transport_runtime_rfb_lifecycle.o"

    for path in (
        rfb_object,
        adapter_object,
        channel_object,
        resources_object,
        transport_object,
        lifecycle_object,
    ):
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    nm = discover_nm(nm_explicit)
    rfb_undefined = nm_symbols(nm_output(nm, rfb_object, True))
    adapter_defined = nm_symbols(nm_output(nm, adapter_object, False))
    channel_defined = nm_symbols(nm_output(nm, channel_object, False))
    resources_defined = nm_symbols(nm_output(nm, resources_object, False))
    transport_defined = nm_symbols(nm_output(nm, transport_object, False))
    lifecycle_defined = nm_symbols(nm_output(nm, lifecycle_object, False))
    lifecycle_undefined = nm_symbols(nm_output(nm, lifecycle_object, True))

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

    for name in RESOURCE_NAMES:
        if name not in resources_defined:
            fail(f"resources_object_missing_definition:{name}")

    for public, inner in zip(PUBLIC_LIFECYCLE_NAMES, INNER_LIFECYCLE_NAMES, strict=True):
        if public not in lifecycle_defined:
            fail(f"lifecycle_wrapper_missing_public_definition:{public}")
        if public in transport_defined:
            fail(f"transport_object_still_defines_public_lifecycle:{public}")
        if inner not in transport_defined:
            fail(f"transport_object_missing_inner_lifecycle:{inner}")
        if inner not in lifecycle_undefined:
            fail(f"lifecycle_wrapper_missing_inner_reference:{inner}")

    for name in RESOURCE_NAMES:
        if name not in lifecycle_undefined:
            fail(f"lifecycle_wrapper_missing_resource_reference:{name}")

    print(f"H1_RFB_MUX_SEAM_OBJECTS=PASS nm={nm}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Verify the H1 cumulative RFB parser, logical channel, dormant "
            "resource ownership, and cumulative lifecycle wrapper without "
            "claiming live RFB support."
        )
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help=(
            "optional PS2 build directory containing the cumulative RFB seam, "
            "channel, resources, transport, and lifecycle objects; when omitted "
            "only source/build-rule checks run"
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
