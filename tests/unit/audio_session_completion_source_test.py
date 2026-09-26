#!/usr/bin/env python3
"""R38 source-boundary proof for AUDIO completion observation."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SESSION_C = (ROOT / "src/audio/session.c").read_text(encoding="utf-8")
SESSION_H = (ROOT / "src/audio/session.h").read_text(encoding="utf-8")
RUNTIME_C = (ROOT / "src/audio/ps2_runtime.c").read_text(encoding="utf-8")
APP_C = "\n".join(path.read_text(encoding="utf-8") for path in ROOT.glob("src/app*.c"))


def function_body(text: str, name: str) -> str:
    start = text.find(name)
    if start < 0:
        raise AssertionError(f"missing function: {name}")
    brace = text.find("{", start)
    if brace < 0:
        raise AssertionError(f"missing body: {name}")
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace:index + 1]
    raise AssertionError(f"unterminated body: {name}")


session_poll = function_body(SESSION_C, "pstvnc_audio_session_poll")
runtime_poll = function_body(
    RUNTIME_C,
    "pstvnc_audio_ps2_runtime_thread_poll_completion",
)

for required in (
    "thread_ops.poll_completion",
    "PSTVNC_AUDIO_SESSION_COMPLETION_PENDING",
    "PSTVNC_AUDIO_SESSION_COMPLETION_DONE",
    "PSTVNC_AUDIO_SESSION_THREAD_STATUS_FAILED",
):
    if required not in SESSION_C + SESSION_H:
        raise AssertionError(f"missing R38 session contract: {required}")

for forbidden in (
    "pstvnc_transport_audio_status",
    "pstvnc_transport_audio_activity_snapshot",
    "pstvnc_media_clock_",
    "delay_us",
    "worker_finished",
    "outcome",
):
    if forbidden in session_poll:
        raise AssertionError(f"session completion poll crossed owner boundary: {forbidden}")

for required in ("ReferSemaStatus", "PollSema", "completion_observed"):
    if required not in runtime_poll:
        raise AssertionError(f"missing R38 PS2 poll mechanism: {required}")

for forbidden in ("WaitSema", "DelayThread", "ReferThreadStatus"):
    if forbidden in runtime_poll:
        raise AssertionError(f"PS2 completion poll blocks: {forbidden}")

if "pstvnc_audio_session_poll(" in APP_C:
    raise AssertionError("R38 entered ordinary Application AUDIO composition")

print("AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS")
