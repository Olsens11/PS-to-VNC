#!/usr/bin/env python3
"""
File synopsis:
    Runs the qualified EXP3 finite live-X11 producer with the P11/P10
    608x416 / 4 Mb/s shaped MPEG-2 encode and the E2 sequence-end suffix.

Purpose:
    P11 qualified the PS2 playback side at 608x416, RGB16, a 512 KiB
    compressed queue, 448 KiB startup prefill, absolute 30000/1001 pacing,
    and P9 stage timing. P12 changes only the producer source from a fixed
    archive to true live X11 capture while preserving those playback settings.

    The underlying network_live_stream_server.py continues to own PSTV
    framing, receiver-credit flow, archive accounting, CRC/start-code
    telemetry, and terminal result validation. This wrapper changes only the
    x11grab ffmpeg command and appends the already-qualified 16-byte E2
    sequence-end suffix at real ffmpeg stdout EOF.

P12 encode contract:
    * X11 input: 704x462 @ 30000/1001
    * output: 608x416 yuv420p
    * MPEG-2 Main Profile / Main Level
    * target bitrate: 4,000,000 bit/s
    * max bitrate: 5,000,000 bit/s
    * VBV buffer: 1,835,008 bits
    * GOP: 15
    * B frames: 0
    * explicit output rate: 30000/1001
"""

from __future__ import annotations

import shlex
import subprocess

import network_live_stream_server as live
from network_live_stream_server_sequence_end import (
    MPEG_SEQUENCE_END,
    TERMINAL_SUFFIX,
    TerminalSuffixStream,
)


EXPECTED_SOURCE_SIZE = "704x462"
EXPECTED_INPUT_RATE = "30000/1001"
EXPECTED_DURATION = "24.0"
EXPECTED_BITRATE_ARGUMENT = "4M"
EXPECTED_GOP = "15"

OUTPUT_FILTER = "scale=608:416:flags=bicubic,format=yuv420p"
TARGET_BITRATE = "4000000"
MAX_BITRATE = "5000000"
VBV_BITS = "1835008"

_ORIGINAL_POPEN = subprocess.Popen


def _value_after(command: list[str], option: str) -> str:
    count = command.count(option)
    if count != 1:
        raise RuntimeError(
            f"P12 expected exactly one {option}, found {count}"
        )
    index = command.index(option)
    if index + 1 >= len(command):
        raise RuntimeError(f"P12 option {option} has no value")
    return command[index + 1]


def _p12_capture_command(command: list[str]) -> list[str]:
    """Validate the qualified O3 capture skeleton, then shape only the encode."""

    if _value_after(command, "-framerate") != EXPECTED_INPUT_RATE:
        raise RuntimeError("P12 input frame-rate authority mismatch")
    if _value_after(command, "-video_size") != EXPECTED_SOURCE_SIZE:
        raise RuntimeError("P12 X11 source-size authority mismatch")
    if _value_after(command, "-t") != EXPECTED_DURATION:
        raise RuntimeError("P12 duration authority mismatch")
    if _value_after(command, "-c:v") != "mpeg2video":
        raise RuntimeError("P12 codec authority mismatch")
    if _value_after(command, "-b:v") != EXPECTED_BITRATE_ARGUMENT:
        raise RuntimeError("P12 base bitrate argument mismatch")
    if _value_after(command, "-g") != EXPECTED_GOP:
        raise RuntimeError("P12 GOP authority mismatch")
    if _value_after(command, "-bf") != "0":
        raise RuntimeError("P12 B-frame authority mismatch")
    if _value_after(command, "-vf") != "pad=704:480:0:0:black,format=yuv420p":
        raise RuntimeError("P12 qualified O3 filter skeleton changed")

    display_input = _value_after(command, "-i")

    shaped = [
        "ffmpeg",
        "-hide_banner",
        "-loglevel",
        "warning",
        "-f",
        "x11grab",
        "-framerate",
        EXPECTED_INPUT_RATE,
        "-video_size",
        EXPECTED_SOURCE_SIZE,
        "-i",
        display_input,
        "-t",
        EXPECTED_DURATION,
        "-vf",
        OUTPUT_FILTER,
        "-an",
        "-c:v",
        "mpeg2video",
        "-profile:v",
        "main",
        "-level:v",
        "main",
        "-pix_fmt",
        "yuv420p",
        "-b:v",
        TARGET_BITRATE,
        "-maxrate:v",
        MAX_BITRATE,
        "-minrate:v",
        "0",
        "-bufsize:v",
        VBV_BITS,
        "-g",
        EXPECTED_GOP,
        "-bf",
        "0",
        "-r",
        EXPECTED_INPUT_RATE,
        "-f",
        "mpeg2video",
        "pipe:1",
    ]

    return shaped


def _p12_popen(*args, **kwargs):
    command = args[0] if args else kwargs.get("args")
    is_capture = (
        isinstance(command, (list, tuple))
        and len(command) > 1
        and command[0] == "ffmpeg"
        and "x11grab" in command
    )

    if not is_capture:
        return _ORIGINAL_POPEN(*args, **kwargs)

    shaped = _p12_capture_command(list(command))

    print(f"P12_FFMPEG_COMMAND={shlex.join(shaped)}", flush=True)

    if args:
        new_args = (shaped, *args[1:])
        process = _ORIGINAL_POPEN(*new_args, **kwargs)
    else:
        new_kwargs = dict(kwargs)
        new_kwargs["args"] = shaped
        process = _ORIGINAL_POPEN(**new_kwargs)

    if process.stdout is None:
        raise RuntimeError("P12 capture ffmpeg stdout pipe was not created")

    process.stdout = TerminalSuffixStream(process.stdout)

    print(
        "P12_LIVE_ENCODER=ACTIVE "
        "output=608x416 fps=30000/1001 profile=main level=main "
        "target_bps=4000000 max_bps=5000000 vbv_bits=1835008 "
        "gop=15 bframes=0",
        flush=True,
    )

    return process


def main() -> int:
    if len(TERMINAL_SUFFIX) != 16:
        raise RuntimeError("P12 qualified terminal suffix is not 16 bytes")
    if not TERMINAL_SUFFIX.startswith(MPEG_SEQUENCE_END):
        raise RuntimeError("P12 qualified sequence-end prefix changed")

    # network_live_stream_server imports this same subprocess module object.
    # Only its x11grab Popen is rewritten; later software-decode checks and all
    # unrelated subprocesses use the original Popen unchanged.
    subprocess.Popen = _p12_popen

    print(
        "P12_TRUE_LIVE_WRAPPER=ACTIVE "
        "source=X11 output=608x416 queue_expected=524288 "
        "prefill_expected=458752 rgb16_expected=YES "
        "sequence_end_suffix_bytes=16",
        flush=True,
    )

    return live.main()


if __name__ == "__main__":
    raise SystemExit(main())
