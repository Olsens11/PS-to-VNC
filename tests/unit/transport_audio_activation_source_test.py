#!/usr/bin/env python3
"""R40 source-boundary proof for deferred AUDIO initial-credit activation."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
RUNTIME = (ROOT / "src/transport/runtime.c").read_text(encoding="utf-8")
BRIDGE = (ROOT / "src/transport/bridge.c").read_text(encoding="utf-8")
WIRE = (ROOT / "pi/wire_server.py").read_text(encoding="utf-8")
PRODUCT = (ROOT / "pi/wire_runtime.py").read_text(encoding="utf-8")


def body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        raise AssertionError(f"missing function signature: {signature}")
    brace = source.find("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1 : index]
    raise AssertionError(f"unterminated function: {signature}")


activate = body(
    RUNTIME,
    "pstvnc_transport_result_t pstvnc_transport_runtime_audio_activate(",
)
start_receiver = body(
    RUNTIME,
    "int pstvnc_transport_runtime_start_receiver(",
)
accept_audio = body(
    RUNTIME,
    "static int pstvnc_transport_runtime_accept_audio_frame(",
)
bridge_activate = body(
    BRIDGE,
    "pstvnc_transport_result_t pstvnc_transport_audio_activate(",
)

publish = activate.find(
    "runtime->audio_activation_state = PSTVNC_TRANSPORT_AUDIO_ACTIVATING;"
)
send = activate.find(
    "pstvnc_transport_runtime_send_credit(\n"
    "            runtime,\n"
    "            PSTVNC_TRANSPORT_CHANNEL_AUDIO,"
)
active = activate.find(
    "runtime->audio_activation_state = PSTVNC_TRANSPORT_AUDIO_ACTIVE;"
)
if min(publish, send, active) < 0 or not publish < send < active:
    raise AssertionError(
        "R40 activation ordering lost ACTIVATING -> CREDIT submit -> ACTIVE"
    )

if "PSTVNC_TRANSPORT_CHANNEL_AUDIO" in re.sub(
    r"/\*.*?\*/|//.*?$", "", start_receiver, flags=re.DOTALL | re.MULTILINE
):
    raise AssertionError("receiver startup still emits AUDIO initial credit")

for required in (
    "PSTVNC_TRANSPORT_CHANNEL_RFB",
    "PSTVNC_TRANSPORT_CHANNEL_MPEG2",
):
    if required not in start_receiver:
        raise AssertionError(f"R40 changed existing startup credit: {required}")

if (
    "runtime->audio_activation_state == PSTVNC_TRANSPORT_AUDIO_DORMANT"
    not in accept_audio
):
    raise AssertionError("R40 inbound AUDIO lacks DORMANT rejection")
if "PSTVNC_TRANSPORT_AUDIO_ACTIVATING" in accept_audio:
    raise AssertionError("R40 incorrectly rejects ACTIVATING by explicit state")

if "pstvnc_transport_bridge_access_result(transport_access)" not in bridge_activate:
    raise AssertionError("public AUDIO activation is not exact-ticket fenced")
if "pstvnc_transport_runtime_audio_activate(" not in bridge_activate:
    raise AssertionError("bridge AUDIO activation does not delegate to runtime owner")

if "audio_pcm_factory=self.audio_pcm_factory" not in WIRE:
    raise AssertionError("Wire owner does not retain lazy AUDIO factory")
if "self.audio_pcm_factory(outcome.session_id)" in WIRE:
    raise AssertionError("Wire establishment still eagerly creates AUDIO owner")
if "factory(self.session_id)" not in WIRE:
    raise AssertionError("first CREDIT does not create exact-session AUDIO owner")
if "created.add_credit(amount)" not in WIRE:
    raise AssertionError("first decoded CREDIT is not applied to created owner")

if "audio_pcm_factory=" in PRODUCT or "audio_pcm_producer" in PRODUCT:
    raise AssertionError("R40 accidentally enabled ordinary Pi AUDIO composition")

print("TRANSPORT_AUDIO_ACTIVATION_SOURCE_TEST=PASS")
