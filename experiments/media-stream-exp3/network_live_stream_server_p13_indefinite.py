#!/usr/bin/env python3
"""
File synopsis:
    Runs the qualified EXP3 P12 live-X11 MPEG profile as an open-ended P13
    session while preserving the exact P11 PS2 playback binary and wire path.

Purpose:
    P12 qualified a 24-second true-live session. P13 changes session lifetime
    only: FFmpeg has no duration limit, and an operator SIGINT/SIGTERM requests
    an orderly encoder stop. The existing O3/P12 terminal machinery then
    appends the hardware-qualified 16-byte MPEG sequence-end suffix, drains the
    compressed queue, sends exact terminal metadata, and validates the PS2
    result contract.

    No new PS2 protocol generation/session field is introduced in this first
    P13 qualification because the repository explicitly requires reuse of the
    P11 PS2 ELF. The existing CONFIG/ACK is the wire-level begin boundary for
    this test; the existing terminal TELEMETRY frame is the wire-level end
    boundary. A per-run apparatus session id is logged for evidence identity.

Preserved P12 encode contract:
    * X11 input: 704x462 @ 30000/1001
    * output: 608x416 yuv420p
    * MPEG-2 Main Profile / Main Level
    * target bitrate: 4,000,000 bit/s
    * max bitrate: 5,000,000 bit/s
    * VBV buffer: 1,835,008 bits
    * GOP: 15
    * B frames: 0

Controlled-stop contract:
    * capture FFmpeg is placed in its own process session;
    * SIGINT or SIGTERM delivered to this Python server is consumed as one
      controlled media-stop request;
    * the server sends SIGINT to capture FFmpeg exactly once;
    * only expected signal-stop FFmpeg return codes are normalized to success;
    * an unrequested clean FFmpeg EOF is treated as failure, not normal P13 end.
"""

from __future__ import annotations

import os
import shlex
import signal
import subprocess
import time

import network_live_stream_server as live
import network_live_stream_server_p12_608x416_4m_vbv as p12
from network_live_stream_server_sequence_end import (
    MPEG_SEQUENCE_END,
    TERMINAL_SUFFIX,
    TerminalSuffixStream,
)


_ORIGINAL_POPEN = subprocess.Popen

_capture_process: "ControlledCaptureProcess | None" = None
_stop_requested = False
_stop_signal = 0
_session_id = ""


def _signal_name(signum: int) -> str:
    try:
        return signal.Signals(signum).name
    except ValueError:
        return str(signum)


class ControlledCaptureProcess:
    """Proxy capture FFmpeg so P13 can stop it without aborting the sender."""

    def __init__(self, process: subprocess.Popen) -> None:
        if process.stdout is None:
            raise RuntimeError("P13 capture ffmpeg stdout pipe was not created")

        self._process = process
        self.stdout = TerminalSuffixStream(process.stdout)
        self._stop_signal_sent = False

    def request_controlled_stop(self) -> None:
        if self._stop_signal_sent or self._process.poll() is not None:
            return

        self._stop_signal_sent = True
        self._process.send_signal(signal.SIGINT)

    def wait(self, *args, **kwargs) -> int:
        raw_status = self._process.wait(*args, **kwargs)

        print(
            "P13_FFMPEG_EXIT "
            f"raw_status={raw_status} "
            f"controlled_stop={'YES' if _stop_requested else 'NO'}",
            flush=True,
        )

        if not _stop_requested:
            if raw_status == 0:
                print(
                    "P13_UNCONTROLLED_ENCODER_EOF=FAIL "
                    "reason=open_ended_session_ended_without_stop_request",
                    flush=True,
                )
                return 90
            return raw_status

        expected_signal_statuses = {
            0,
            255,
            128 + signal.SIGINT,
            -signal.SIGINT,
        }
        if raw_status in expected_signal_statuses:
            print(
                "P13_CONTROLLED_ENCODER_STOP=PASS "
                f"raw_status={raw_status}",
                flush=True,
            )
            return 0

        print(
            "P13_CONTROLLED_ENCODER_STOP=FAIL "
            f"unexpected_raw_status={raw_status}",
            flush=True,
        )
        return raw_status

    def __getattr__(self, name):
        return getattr(self._process, name)


def _p13_capture_command(command: list[str]) -> list[str]:
    """
    Start from P12's exact validated shaped command and remove only -t 24.0.

    P13 should still be launched with --duration 24.0 so the inherited P12
    validator proves the input skeleton is the exact qualified P12 apparatus.
    The duration token exists only as a compatibility/identity assertion and is
    absent from the actual FFmpeg process.
    """

    shaped = p12._p12_capture_command(command)

    if shaped.count("-t") != 1:
        raise RuntimeError(
            f"P13 expected one P12 duration option, found {shaped.count('-t')}"
        )

    duration_index = shaped.index("-t")
    if duration_index + 1 >= len(shaped):
        raise RuntimeError("P13 P12 duration option has no value")
    if shaped[duration_index + 1] != p12.EXPECTED_DURATION:
        raise RuntimeError("P13 inherited P12 duration authority mismatch")

    del shaped[duration_index : duration_index + 2]

    if "-t" in shaped:
        raise RuntimeError("P13 actual FFmpeg command still contains -t")

    return shaped


def _p13_popen(*args, **kwargs):
    global _capture_process

    command = args[0] if args else kwargs.get("args")
    is_capture = (
        isinstance(command, (list, tuple))
        and len(command) > 1
        and command[0] == "ffmpeg"
        and "x11grab" in command
    )

    if not is_capture:
        return _ORIGINAL_POPEN(*args, **kwargs)

    if _capture_process is not None:
        raise RuntimeError("P13 attempted to create more than one capture process")

    shaped = _p13_capture_command(list(command))
    process_kwargs = dict(kwargs)

    if "start_new_session" in process_kwargs:
        raise RuntimeError("P13 capture unexpectedly supplied start_new_session")
    process_kwargs["start_new_session"] = True

    print(f"P13_FFMPEG_COMMAND={shlex.join(shaped)}", flush=True)

    if args:
        new_args = (shaped, *args[1:])
        process = _ORIGINAL_POPEN(*new_args, **process_kwargs)
    else:
        process_kwargs["args"] = shaped
        process = _ORIGINAL_POPEN(**process_kwargs)

    controlled = ControlledCaptureProcess(process)
    _capture_process = controlled

    print(
        "P13_LIVE_ENCODER=ACTIVE "
        "lifetime=open-ended controlled_stop=SIGINT_or_SIGTERM "
        "output=608x416 fps=30000/1001 profile=main level=main "
        "target_bps=4000000 max_bps=5000000 vbv_bits=1835008 "
        "gop=15 bframes=0",
        flush=True,
    )

    if _stop_requested:
        controlled.request_controlled_stop()

    return controlled


def _request_controlled_stop(signum, _frame) -> None:
    global _stop_requested, _stop_signal

    if _stop_requested:
        return

    _stop_requested = True
    _stop_signal = int(signum)

    message = (
        "P13_MEDIA_STOP_REQUESTED=YES "
        f"session_id={_session_id} signal={_signal_name(_stop_signal)}\n"
    )
    os.write(1, message.encode("utf-8", errors="replace"))

    if _capture_process is not None:
        _capture_process.request_controlled_stop()


def main() -> int:
    global _session_id

    if len(TERMINAL_SUFFIX) != 16:
        raise RuntimeError("P13 qualified terminal suffix is not 16 bytes")
    if not TERMINAL_SUFFIX.startswith(MPEG_SEQUENCE_END):
        raise RuntimeError("P13 qualified sequence-end prefix changed")

    _session_id = f"p13-{time.time_ns()}-{os.getpid()}"

    previous_sigint = signal.getsignal(signal.SIGINT)
    previous_sigterm = signal.getsignal(signal.SIGTERM)

    signal.signal(signal.SIGINT, _request_controlled_stop)
    signal.signal(signal.SIGTERM, _request_controlled_stop)
    subprocess.Popen = _p13_popen

    print(
        "P13_MEDIA_BEGIN=YES "
        f"session_id={_session_id} "
        "lifetime=open-ended "
        "wire_begin=O3_CONFIG_ACK "
        "wire_end=O3_TERMINAL_TELEMETRY "
        "ps2_binary_expected=P11 "
        "queue_expected=524288 prefill_expected=458752 "
        "rgb16_expected=YES no_frame_drop_expected=YES",
        flush=True,
    )

    try:
        result = live.main()
    finally:
        subprocess.Popen = _ORIGINAL_POPEN
        signal.signal(signal.SIGINT, previous_sigint)
        signal.signal(signal.SIGTERM, previous_sigterm)

    if not _stop_requested:
        raise RuntimeError(
            "P13 live.main returned without an explicit controlled stop request"
        )
    if result != 0:
        raise RuntimeError(f"P13 underlying live server returned {result}")

    print(
        "P13_MEDIA_END=PASS "
        f"session_id={_session_id} "
        f"stop_signal={_signal_name(_stop_signal)} "
        "terminal_contract=PASS",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
