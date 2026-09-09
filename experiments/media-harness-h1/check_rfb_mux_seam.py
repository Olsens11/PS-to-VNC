#!/usr/bin/env python3
"""
File synopsis:
    Verifies ownership and ordering for the cumulative H1 logical RFB channel-1
    mux seam while the public CONFIG RFB-ON gate remains closed.

The checker proves that the unchanged through-Issue-39 RFB parser is compiled
against experiment-owned mux I/O names; the configurable queue, credit policy,
live transport adapter, and lifecycle cleanup are linked; channel-1 resources
are prepared before H1's sole physical receiver starts; inbound DATA and
initial/returned credit are wired; and no second RFB socket path is introduced.

This is source/object/build evidence, not hardware qualification. The checker
also requires the authoritative CONFIG gate and HELLO capability gate to remain
closed at this checkpoint.
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
CREDIT_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_credit_policy.c"
CREDIT_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_credit_policy.h"
RESOURCES_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_runtime_resources.c"
RESOURCES_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_runtime_resources.h"
LIVE_C = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_transport_live.c"
LIVE_H = ROOT / "experiments" / "media-harness-h1" / "h1_rfb_transport_live.h"
LIFECYCLE_C = ROOT / "experiments" / "media-harness-h1" / "h1_transport_runtime_rfb_lifecycle.c"
TRANSPORT_C = ROOT / "experiments" / "media-harness-h1" / "h1_transport_runtime.c"
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
ADAPTER_BIND_NAMES = (
    "pstvnc_h1_rfb_mux_io_bind",
    "pstvnc_h1_rfb_mux_io_unbind",
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
LIVE_NAMES = (
    "pstvnc_h1_rfb_transport_prepare",
    "pstvnc_h1_rfb_transport_release",
    "pstvnc_h1_rfb_transport_accept_data",
    "pstvnc_h1_rfb_transport_send_initial_credit",
    "pstvnc_h1_rfb_transport_read_exact",
    "pstvnc_h1_rfb_transport_poll_receive",
    "pstvnc_h1_rfb_transport_write_exact",
)
PUBLIC_LIFECYCLE_NAMES = (
    "pstvnc_h1_transport_start",
    "pstvnc_h1_transport_shutdown",
)
INNER_LIFECYCLE_NAMES = (
    "pstvnc_h1_transport_start_inner",
    "pstvnc_h1_transport_shutdown_inner",
)
INTERNAL_SEND = "pstvnc_h1_transport_send_frame_internal"


def fail(message: str) -> None:
    print(f"H1_RFB_MUX_SEAM=FAIL detail={message}")
    raise SystemExit(1)


def require_text(haystack: str, needle: str, label: str) -> None:
    if needle not in haystack:
        fail(f"missing_{label}:{needle}")


def require_before(haystack: str, first: str, second: str, label: str) -> None:
    first_index = haystack.find(first)
    second_index = haystack.find(second)
    if first_index < 0 or second_index < 0 or first_index >= second_index:
        fail(f"bad_order_{label}:{first}_before_{second}")


def check_source() -> None:
    make_text = MAKEFILE.read_text(encoding="utf-8")
    adapter_c = ADAPTER_C.read_text(encoding="utf-8")
    adapter_h = ADAPTER_H.read_text(encoding="utf-8")
    channel_c = CHANNEL_C.read_text(encoding="utf-8")
    channel_h = CHANNEL_H.read_text(encoding="utf-8")
    credit_c = CREDIT_C.read_text(encoding="utf-8")
    credit_h = CREDIT_H.read_text(encoding="utf-8")
    resources_c = RESOURCES_C.read_text(encoding="utf-8")
    resources_h = RESOURCES_H.read_text(encoding="utf-8")
    live_c = LIVE_C.read_text(encoding="utf-8")
    live_h = LIVE_H.read_text(encoding="utf-8")
    lifecycle_c = LIFECYCLE_C.read_text(encoding="utf-8")
    transport_c = TRANSPORT_C.read_text(encoding="utf-8")
    transport_h = TRANSPORT_H.read_text(encoding="utf-8")
    config_c = CONFIG_C.read_text(encoding="utf-8")

    for obj in (
        "h1_rfb_mux_io.o",
        "h1_rfb_channel.o",
        "h1_rfb_credit_policy.o",
        "h1_rfb_runtime_resources.o",
        "h1_rfb_transport_live.o",
        "h1_transport_runtime_rfb_lifecycle.o",
    ):
        require_text(make_text, f"$(BUILD_DIR)/{obj}", "linked_object")

    require_text(make_text, "-DPSTVNC_H1_RFB_MUX_PREP=1", "cumulative_hook_define")

    for direct, mux in zip(DIRECT_NAMES, MUX_NAMES, strict=True):
        require_text(make_text, f"-D{direct}={mux}", "compile_time_rfb_io_remap")
        require_text(adapter_c, f"int {mux}(", "adapter_definition")
        require_text(adapter_h, f"int {mux}(", "adapter_declaration")

    for name in ADAPTER_BIND_NAMES:
        require_text(adapter_c, f"{name}(", "adapter_binding_definition")
        require_text(adapter_h, f"{name}(", "adapter_binding_declaration")

    for name in CHANNEL_NAMES:
        require_text(channel_c, f"{name}(", "channel_definition")
        require_text(channel_h, f"{name}(", "channel_declaration")

    require_text(
        credit_c,
        "pstvnc_h1_rfb_credit_should_return(",
        "credit_policy_definition",
    )
    require_text(
        credit_h,
        "pstvnc_h1_rfb_credit_should_return(",
        "credit_policy_declaration",
    )

    for name in RESOURCE_NAMES:
        require_text(resources_c, f"{name}(", "resource_definition")
        require_text(resources_h, f"{name}(", "resource_declaration")

    for name in LIVE_NAMES:
        require_text(live_c, f"{name}(", "live_definition")
        require_text(live_h, f"{name}(", "live_declaration")

    for public, inner in zip(PUBLIC_LIFECYCLE_NAMES, INNER_LIFECYCLE_NAMES, strict=True):
        require_text(make_text, f"-D{public}={inner}", "transport_lifecycle_inner_remap")
        require_text(lifecycle_c, f"int {public}(", "public_lifecycle_wrapper")
        require_text(lifecycle_c, f"{inner}(", "inner_lifecycle_call")

    require_text(
        transport_h,
        "pstvnc_h1_rfb_runtime_resources_t rfb_resources;",
        "embedded_rfb_resource_bundle",
    )
    require_text(transport_c, f"int {INTERNAL_SEND}(", "serialized_internal_send")
    require_text(
        transport_c,
        "pstvnc_h1_rfb_transport_accept_data(",
        "receiver_rfb_data_dispatch",
    )
    require_text(
        transport_c,
        "pstvnc_h1_rfb_transport_send_initial_credit(runtime)",
        "rfb_initial_credit_call",
    )
    require_text(
        live_c,
        "pstvnc_h1_rfb_credit_should_return(",
        "parser_consumed_credit_policy",
    )
    require_text(
        live_c,
        "pstvnc_h1_rfb_channel_take_credit(channel)",
        "parser_consumed_credit_take",
    )
    require_text(
        live_c,
        "PSTVNC_TRANSPORT_CHANNEL_RFB",
        "logical_rfb_channel_identity",
    )

    # The ordering requirement that motivated CP2G: RFB resources must be ready
    # before a receiver thread can ever consume channel-1 DATA.
    require_before(
        transport_c,
        "pstvnc_h1_rfb_transport_prepare(runtime)",
        "h1_start_receiver(runtime)",
        "rfb_prepare_before_receiver",
    )

    # Cleanup must happen only after the physical receiver/socket owner stops.
    require_before(
        lifecycle_c,
        "pstvnc_h1_transport_shutdown_inner(runtime)",
        "pstvnc_h1_rfb_transport_release(runtime)",
        "physical_stop_before_rfb_release",
    )

    # The adapter may delegate only to the logical runtime. A second physical
    # RFB socket/recv/send path is forbidden by the one-stream architecture.
    for forbidden in ("socket(", "connect(", "recv(", "send("):
        if forbidden in adapter_c:
            fail(f"adapter_contains_physical_socket_io:{forbidden}")

    # Public activation remains fail closed until the Pi bridge and orchestration
    # checkpoint are independently verified.
    require_text(
        config_c,
        "config->rfb_mode != PSTVNC_H1_RFB_OFF",
        "rfb_config_gate",
    )
    if "PSTVNC_TRANSPORT_CAP_RFB" in transport_c:
        fail("rfb_capability_advertised_before_activation_gate")

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
    command.append("-u" if undefined_only else "--defined-only")
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
    symbols: set[str] = set()
    for line in output.splitlines():
        fields = line.split()
        if fields:
            symbols.add(fields[-1])
    return symbols


def check_objects(build_dir: Path, nm_explicit: str | None) -> None:
    objects = {
        "rfb": build_dir / "rfb_session39.o",
        "adapter": build_dir / "h1_rfb_mux_io.o",
        "channel": build_dir / "h1_rfb_channel.o",
        "credit": build_dir / "h1_rfb_credit_policy.o",
        "resources": build_dir / "h1_rfb_runtime_resources.o",
        "live": build_dir / "h1_rfb_transport_live.o",
        "transport": build_dir / "h1_transport_runtime.o",
        "lifecycle": build_dir / "h1_transport_runtime_rfb_lifecycle.o",
    }

    for path in objects.values():
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    nm = discover_nm(nm_explicit)
    defined = {
        name: nm_symbols(nm_output(nm, path, False))
        for name, path in objects.items()
    }
    undefined = {
        name: nm_symbols(nm_output(nm, path, True))
        for name, path in objects.items()
    }

    for direct in DIRECT_NAMES:
        if direct in undefined["rfb"]:
            fail(f"rfb_session_still_references_direct_io:{direct}")

    for mux in MUX_NAMES:
        if mux not in undefined["rfb"]:
            fail(f"rfb_session_missing_mux_reference:{mux}")
        if mux not in defined["adapter"]:
            fail(f"adapter_missing_definition:{mux}")

    for name in ADAPTER_BIND_NAMES:
        if name not in defined["adapter"]:
            fail(f"adapter_missing_binding_definition:{name}")

    for name in CHANNEL_NAMES:
        if name not in defined["channel"]:
            fail(f"channel_object_missing_definition:{name}")

    if "pstvnc_h1_rfb_credit_should_return" not in defined["credit"]:
        fail("credit_object_missing_policy_definition")

    for name in RESOURCE_NAMES:
        if name not in defined["resources"]:
            fail(f"resources_object_missing_definition:{name}")

    for name in LIVE_NAMES:
        if name not in defined["live"]:
            fail(f"live_object_missing_definition:{name}")

    if INTERNAL_SEND not in defined["transport"]:
        fail("transport_missing_serialized_internal_send")
    if INTERNAL_SEND not in undefined["live"]:
        fail("live_transport_not_using_serialized_internal_send")

    for required in (
        "pstvnc_h1_rfb_transport_prepare",
        "pstvnc_h1_rfb_transport_accept_data",
        "pstvnc_h1_rfb_transport_send_initial_credit",
    ):
        if required not in undefined["transport"]:
            fail(f"transport_missing_live_rfb_reference:{required}")

    for public, inner in zip(PUBLIC_LIFECYCLE_NAMES, INNER_LIFECYCLE_NAMES, strict=True):
        if public not in defined["lifecycle"]:
            fail(f"lifecycle_wrapper_missing_public_definition:{public}")
        if public in defined["transport"]:
            fail(f"transport_object_still_defines_public_lifecycle:{public}")
        if inner not in defined["transport"]:
            fail(f"transport_object_missing_inner_lifecycle:{inner}")
        if inner not in undefined["lifecycle"]:
            fail(f"lifecycle_wrapper_missing_inner_reference:{inner}")

    if "pstvnc_h1_rfb_transport_release" not in undefined["lifecycle"]:
        fail("lifecycle_wrapper_missing_rfb_release_reference")

    print(f"H1_RFB_MUX_SEAM_OBJECTS=PASS nm={nm}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Verify cumulative H1 RFB parser remap, independent queue/credit "
            "mechanics, pre-receiver ordering, adapter binding, and cleanup "
            "while the public RFB activation gate remains closed."
        )
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help=(
            "optional PS2 build directory containing the cumulative RFB objects; "
            "when omitted only source/build-rule checks run"
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
