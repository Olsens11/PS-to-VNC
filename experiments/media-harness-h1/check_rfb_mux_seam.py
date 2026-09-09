#!/usr/bin/env python3
"""Verify cumulative H1 logical RFB mux ownership while RFB ON stays gated."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
H1 = ROOT / "experiments" / "media-harness-h1"
MAKEFILE = ROOT / "mk" / "media-harness-h1-cumulative39-thread-census.mk"
ADAPTER_C = H1 / "h1_rfb_mux_io.c"
ADAPTER_H = H1 / "h1_rfb_mux_io.h"
CHANNEL_C = H1 / "h1_rfb_channel.c"
CHANNEL_H = H1 / "h1_rfb_channel.h"
CREDIT_C = H1 / "h1_rfb_credit_policy.c"
CREDIT_H = H1 / "h1_rfb_credit_policy.h"
RESOURCES_C = H1 / "h1_rfb_runtime_resources.c"
RESOURCES_H = H1 / "h1_rfb_runtime_resources.h"
LIVE_C = H1 / "h1_rfb_transport_live.c"
LIVE_H = H1 / "h1_rfb_transport_live.h"
LIFECYCLE_C = H1 / "h1_transport_runtime_rfb_lifecycle.c"
TRANSPORT_C = H1 / "h1_transport_runtime.c"
TRANSPORT_H = H1 / "h1_transport_runtime.h"
CONFIG_C = H1 / "h1_config.c"

DIRECT = (
    "pstvnc_rfb_io_read_exact",
    "pstvnc_rfb_io_poll_receive",
    "pstvnc_rfb_io_write_exact",
)
MUX = (
    "pstvnc_h1_rfb_mux_io_read_exact",
    "pstvnc_h1_rfb_mux_io_poll_receive",
    "pstvnc_h1_rfb_mux_io_write_exact",
)
BIND = ("pstvnc_h1_rfb_mux_io_bind", "pstvnc_h1_rfb_mux_io_unbind")
CHANNEL = (
    "pstvnc_h1_rfb_channel_init",
    "pstvnc_h1_rfb_channel_accept_data",
    "pstvnc_h1_rfb_channel_poll",
    "pstvnc_h1_rfb_channel_read_available",
    "pstvnc_h1_rfb_channel_take_credit",
    "pstvnc_h1_rfb_channel_write_logical",
)
RESOURCES = (
    "pstvnc_h1_rfb_runtime_resources_init",
    "pstvnc_h1_rfb_runtime_resources_activate",
    "pstvnc_h1_rfb_runtime_resources_release",
)
LIVE = (
    "pstvnc_h1_rfb_transport_prepare",
    "pstvnc_h1_rfb_transport_release",
    "pstvnc_h1_rfb_transport_accept_data",
    "pstvnc_h1_rfb_transport_send_initial_credit",
    "pstvnc_h1_rfb_transport_read_exact",
    "pstvnc_h1_rfb_transport_poll_receive",
    "pstvnc_h1_rfb_transport_write_exact",
)
PUBLIC = ("pstvnc_h1_transport_start", "pstvnc_h1_transport_shutdown")
INNER = ("pstvnc_h1_transport_start_inner", "pstvnc_h1_transport_shutdown_inner")
INTERNAL_SEND = "pstvnc_h1_transport_send_frame_internal"


def fail(detail: str) -> None:
    print(f"H1_RFB_MUX_SEAM=FAIL detail={detail}")
    raise SystemExit(1)


def need(text: str, token: str, label: str) -> None:
    if token not in text:
        fail(f"missing_{label}:{token}")


def before(text: str, first: str, second: str, label: str) -> None:
    a = text.find(first)
    b = text.find(second)
    if a < 0 or b < 0 or a >= b:
        fail(f"bad_order_{label}:{first}_before_{second}")


def function_body(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0:
        fail(f"missing_function:{signature}")
    brace = text.find("{", start)
    if brace < 0:
        fail(f"missing_function_brace:{signature}")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[brace : pos + 1]
    fail(f"unterminated_function:{signature}")
    raise AssertionError("unreachable")


def check_source() -> None:
    make = MAKEFILE.read_text(encoding="utf-8")
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
    lifecycle = LIFECYCLE_C.read_text(encoding="utf-8")
    transport = TRANSPORT_C.read_text(encoding="utf-8")
    transport_h = TRANSPORT_H.read_text(encoding="utf-8")
    config = CONFIG_C.read_text(encoding="utf-8")

    for obj in (
        "h1_rfb_mux_io.o", "h1_rfb_channel.o", "h1_rfb_credit_policy.o",
        "h1_rfb_runtime_resources.o", "h1_rfb_transport_live.o",
        "h1_transport_runtime_rfb_lifecycle.o",
    ):
        need(make, f"$(BUILD_DIR)/{obj}", "linked_object")
    need(make, "-DPSTVNC_H1_RFB_MUX_PREP=1", "cumulative_hook_define")

    for direct, mux in zip(DIRECT, MUX, strict=True):
        need(make, f"-D{direct}={mux}", "rfb_io_remap")
        need(adapter_c, f"int {mux}(", "adapter_definition")
        need(adapter_h, f"int {mux}(", "adapter_declaration")

    for name in BIND:
        need(adapter_c, f"{name}(", "adapter_binding")
        need(adapter_h, f"{name}(", "adapter_binding_declaration")
    for name in CHANNEL:
        need(channel_c, f"{name}(", "channel_definition")
        need(channel_h, f"{name}(", "channel_declaration")
    need(credit_c, "pstvnc_h1_rfb_credit_should_return(", "credit_policy")
    need(credit_h, "pstvnc_h1_rfb_credit_should_return(", "credit_policy_declaration")
    for name in RESOURCES:
        need(resources_c, f"{name}(", "resource_definition")
        need(resources_h, f"{name}(", "resource_declaration")
    for name in LIVE:
        need(live_c, f"{name}(", "live_definition")
        need(live_h, f"{name}(", "live_declaration")

    for public, inner in zip(PUBLIC, INNER, strict=True):
        need(make, f"-D{public}={inner}", "lifecycle_remap")
        need(lifecycle, f"int {public}(", "lifecycle_wrapper")
        need(lifecycle, f"{inner}(", "inner_lifecycle_call")

    need(transport_h, "pstvnc_h1_rfb_runtime_resources_t rfb_resources;", "embedded_rfb_bundle")
    need(transport, f"int {INTERNAL_SEND}(", "serialized_internal_send")
    need(transport, "pstvnc_h1_rfb_transport_accept_data(", "rfb_data_dispatch")
    need(transport, "pstvnc_h1_rfb_transport_send_initial_credit(runtime)", "rfb_initial_credit")
    need(live_c, "pstvnc_h1_rfb_credit_should_return(", "consumed_credit_policy")
    need(live_c, "pstvnc_h1_rfb_channel_take_credit(channel)", "consumed_credit_take")
    need(live_c, "PSTVNC_TRANSPORT_CHANNEL_RFB", "rfb_channel_identity")

    # Resource preparation must precede the only receiver-thread launch.
    before(
        transport,
        "pstvnc_h1_rfb_transport_prepare(runtime)",
        "h1_start_receiver(runtime)",
        "rfb_prepare_before_receiver",
    )

    # Scope shutdown ordering to the public shutdown body: start-failure cleanup
    # is intentionally allowed to call the same release helper earlier in file.
    shutdown_body = function_body(
        lifecycle,
        "int pstvnc_h1_transport_shutdown(\n",
    )
    before(
        shutdown_body,
        "pstvnc_h1_transport_shutdown_inner(runtime)",
        "pstvnc_h1_rfb_transport_release(runtime)",
        "physical_stop_before_rfb_release",
    )

    for forbidden in ("socket(", "connect(", "recv(", "send("):
        if forbidden in adapter_c:
            fail(f"adapter_contains_physical_socket_io:{forbidden}")

    need(config, "config->rfb_mode != PSTVNC_H1_RFB_OFF", "rfb_config_gate")
    if "PSTVNC_TRANSPORT_CAP_RFB" in transport:
        fail("rfb_capability_advertised_before_activation_gate")

    print("H1_RFB_MUX_SEAM_SOURCE=PASS")


def find_nm(explicit: str | None) -> str:
    candidates = [explicit] if explicit else []
    candidates += ["mips64r5900el-ps2-elf-nm", "ee-nm", "nm"]
    for candidate in candidates:
        if candidate:
            resolved = shutil.which(candidate)
            if resolved:
                return resolved
    fail("no_nm_tool_found")
    raise AssertionError("unreachable")


def symbols(nm: str, path: Path, undefined: bool) -> set[str]:
    command = [nm, "-u" if undefined else "--defined-only", str(path)]
    result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if result.returncode != 0:
        fail(f"nm_failed:{path}:{result.stdout.strip()}")
    return {line.split()[-1] for line in result.stdout.splitlines() if line.split()}


def check_objects(build_dir: Path, explicit_nm: str | None) -> None:
    paths = {
        "rfb": build_dir / "rfb_session39.o",
        "adapter": build_dir / "h1_rfb_mux_io.o",
        "channel": build_dir / "h1_rfb_channel.o",
        "credit": build_dir / "h1_rfb_credit_policy.o",
        "resources": build_dir / "h1_rfb_runtime_resources.o",
        "live": build_dir / "h1_rfb_transport_live.o",
        "transport": build_dir / "h1_transport_runtime.o",
        "lifecycle": build_dir / "h1_transport_runtime_rfb_lifecycle.o",
    }
    for path in paths.values():
        if not path.is_file():
            fail(f"missing_build_object:{path}")

    nm = find_nm(explicit_nm)
    defined = {name: symbols(nm, path, False) for name, path in paths.items()}
    undefined = {name: symbols(nm, path, True) for name, path in paths.items()}

    for direct in DIRECT:
        if direct in undefined["rfb"]:
            fail(f"rfb_session_still_references_direct_io:{direct}")
    for mux in MUX:
        if mux not in undefined["rfb"] or mux not in defined["adapter"]:
            fail(f"rfb_mux_symbol_ownership:{mux}")
    for name in BIND:
        if name not in defined["adapter"]:
            fail(f"adapter_missing_binding:{name}")
    for name in CHANNEL:
        if name not in defined["channel"]:
            fail(f"channel_missing:{name}")
    if "pstvnc_h1_rfb_credit_should_return" not in defined["credit"]:
        fail("credit_policy_object_missing")
    for name in RESOURCES:
        if name not in defined["resources"]:
            fail(f"resources_missing:{name}")
    for name in LIVE:
        if name not in defined["live"]:
            fail(f"live_missing:{name}")

    if INTERNAL_SEND not in defined["transport"] or INTERNAL_SEND not in undefined["live"]:
        fail("serialized_internal_send_ownership")
    for name in (
        "pstvnc_h1_rfb_transport_prepare",
        "pstvnc_h1_rfb_transport_accept_data",
        "pstvnc_h1_rfb_transport_send_initial_credit",
    ):
        if name not in undefined["transport"]:
            fail(f"transport_missing_live_reference:{name}")

    for public, inner in zip(PUBLIC, INNER, strict=True):
        if public not in defined["lifecycle"] or public in defined["transport"]:
            fail(f"public_lifecycle_ownership:{public}")
        if inner not in defined["transport"] or inner not in undefined["lifecycle"]:
            fail(f"inner_lifecycle_ownership:{inner}")
    if "pstvnc_h1_rfb_transport_release" not in undefined["lifecycle"]:
        fail("lifecycle_missing_rfb_release")

    print(f"H1_RFB_MUX_SEAM_OBJECTS=PASS nm={nm}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--nm")
    args = parser.parse_args()
    check_source()
    if args.build_dir is not None:
        check_objects(args.build_dir.resolve(), args.nm)
    print("H1_RFB_MUX_SEAM=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
