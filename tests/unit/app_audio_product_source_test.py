#!/usr/bin/env python3
"""R41 source-boundary proof for Application AUDIO lifecycle composition."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/app_audio_product.c").read_text(encoding="utf-8")
HEADER = (ROOT / "src/app_audio_product.h").read_text(encoding="utf-8")
APP = (ROOT / "src/app.c").read_text(encoding="utf-8")
PI_RUNTIME = (ROOT / "pi/wire_runtime.py").read_text(encoding="utf-8")


def body(signature: str) -> str:
    start = SOURCE.find(signature)
    if start < 0:
        raise AssertionError(f"missing function: {signature}")
    brace = SOURCE.find("{", start)
    depth = 0
    for index in range(brace, len(SOURCE)):
        if SOURCE[index] == "{":
            depth += 1
        elif SOURCE[index] == "}":
            depth -= 1
            if depth == 0:
                return SOURCE[brace + 1 : index]
    raise AssertionError(f"unterminated function: {signature}")


start = body("pstvnc_app_audio_product_result_t pstvnc_app_audio_product_start(")
gate = body(
    "pstvnc_app_audio_product_first_presentation_ready("
)
abort = body(
    "pstvnc_app_audio_product_service_session_abort("
)

runtime_init = start.find("pstvnc_audio_ps2_runtime_init(")
runtime_ops = start.find("pstvnc_audio_ps2_runtime_operations(")
activate = start.find("pstvnc_transport_audio_activate(")
session_start = start.find("pstvnc_audio_session_start(")
if min(runtime_init, runtime_ops, activate, session_start) < 0:
    raise AssertionError("R41 start is missing an accepted lower-owner edge")
if not runtime_init < runtime_ops < activate < session_start:
    raise AssertionError(
        "R41 start ordering is not runtime init/ops -> Transport activate -> "
        "AUDIO session start"
    )

if "pstvnc_transport_audio_status(" not in gate:
    raise AssertionError("first-presentation gate does not use AUDIO status")
for forbidden in (
    "pstvnc_transport_audio_read_available(",
    "delay_us(",
    "reservoir_poll_us",
    "clock_poll_us",
):
    if forbidden in gate:
        raise AssertionError(
            f"first-presentation gate gained consuming/time inference: {forbidden}"
        )

retained = abort.find("pstvnc_transport_session_abort_storage_retained(")
request_stop = abort.find("pstvnc_audio_session_request_stop(")
release = abort.find("pstvnc_audio_session_release(")
runtime_release = abort.find("pstvnc_audio_ps2_runtime_release(")
if min(retained, request_stop, release, runtime_release) < 0:
    raise AssertionError("R41 abort path is missing an ownership fence")
if not retained < request_stop < release < runtime_release:
    raise AssertionError(
        "R41 abnormal cleanup does not order retained proof before local reclaim"
    )

for forbidden in (
    "pstvnc_media_clock_arm(",
    "pstvnc_media_clock_reset(",
    "pstvnc_media_clock_epoch(",
):
    if forbidden in SOURCE:
        raise AssertionError(
            f"Application AUDIO coordinator gained clock ownership: {forbidden}"
        )

for required in (
    "PSTVNC_APP_AUDIO_PRODUCT_DORMANT",
    "PSTVNC_APP_AUDIO_PRODUCT_STARTING",
    "PSTVNC_APP_AUDIO_PRODUCT_ACTIVE",
    "PSTVNC_APP_AUDIO_PRODUCT_FINITE_COMPLETE",
    "PSTVNC_APP_AUDIO_PRODUCT_FAULTED",
    "PSTVNC_APP_AUDIO_PRODUCT_ABORT_READY",
):
    if required not in HEADER:
        raise AssertionError(f"missing R41 lifecycle state: {required}")

if "app_audio_product" in APP:
    raise AssertionError("R41 was wired into ordinary src/app.c")
if "audio_pcm_factory=" in PI_RUNTIME or "audio_pcm_producer" in PI_RUNTIME:
    raise AssertionError("R41 enabled ordinary Pi AUDIO factory composition")

print("APP_AUDIO_PRODUCT_SOURCE_TEST=PASS")
