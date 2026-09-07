#!/usr/bin/env python3
"""
File synopsis:
    Runs the existing EXP3 O3 finite live-X11 server while adding the exact
    hardware-qualified E2 MPEG terminal suffix at ffmpeg stdout exhaustion.

Purpose:
    E2 proved that an explicit MPEG sequence-end makes SMS terminate normally
    even when the terminal DMA remains exactly qword aligned. This wrapper is
    intentionally smaller than changing the qualified O3 server in place: all
    ordinary ffmpeg MPEG bytes, PSTV framing, credit flow, archive accounting,
    CRC/start-code telemetry, and PS2 behavior remain owned by
    network_live_stream_server.py.

Mechanism:
    The capture ffmpeg stdout stream is proxied. Every byte from ffmpeg passes
    through unchanged. At the first real stdout EOF, the proxy emits exactly:

        00 00 01 B7 + twelve 00 bytes

    This is the 16-byte E2 suffix qualified on hardware. Because it is 16
    bytes long, it preserves whatever mod-16 alignment ffmpeg naturally ended
    on while providing a deterministic MPEG sequence-end marker.
"""

from __future__ import annotations

import subprocess

import network_live_stream_server as live


MPEG_SEQUENCE_END = bytes.fromhex("000001b7")
TERMINAL_SUFFIX = MPEG_SEQUENCE_END + (b"\x00" * 12)


class TerminalSuffixStream:
    """Pass ffmpeg stdout through, then emit the E2 suffix exactly once."""

    def __init__(self, source) -> None:
        self.source = source
        self.suffix_offset = 0
        self.source_eof = False
        self.sequence_tail = b""
        self.sequence_ends_seen = 0
        self.reported_suffix = False

    def _observe(self, payload: bytes) -> None:
        if not payload:
            return

        combined = self.sequence_tail + payload
        self.sequence_ends_seen += combined.count(MPEG_SEQUENCE_END)
        self.sequence_tail = combined[-3:]

    def read(self, count: int = -1) -> bytes:
        if not self.source_eof:
            payload = self.source.read(count)
            if payload:
                self._observe(payload)
                return payload

            self.source_eof = True

            if self.sequence_ends_seen != 0:
                raise RuntimeError(
                    "capture ffmpeg unexpectedly emitted an MPEG sequence-end; "
                    "O3 sequence-end wrapper expected zero before terminal suffix"
                )

        if self.suffix_offset >= len(TERMINAL_SUFFIX):
            return b""

        if count is None or count < 0:
            take = len(TERMINAL_SUFFIX) - self.suffix_offset
        else:
            take = min(count, len(TERMINAL_SUFFIX) - self.suffix_offset)

        start = self.suffix_offset
        self.suffix_offset += take
        payload = TERMINAL_SUFFIX[start : start + take]

        if not self.reported_suffix:
            print(
                "O3_SEQUENCE_END_SUFFIX_APPENDED=PASS "
                "suffix_bytes=16 sequence_end_hex=000001b7 "
                "terminal_zero_bytes=12",
                flush=True,
            )
            self.reported_suffix = True

        return payload

    def close(self) -> None:
        self.source.close()

    def __getattr__(self, name):
        return getattr(self.source, name)


_ORIGINAL_POPEN = subprocess.Popen


def _sequence_end_popen(*args, **kwargs):
    process = _ORIGINAL_POPEN(*args, **kwargs)

    command = args[0] if args else kwargs.get("args")
    is_capture = (
        isinstance(command, (list, tuple))
        and len(command) > 1
        and command[0] == "ffmpeg"
        and "x11grab" in command
        and process.stdout is not None
    )

    if is_capture:
        process.stdout = TerminalSuffixStream(process.stdout)

    return process


def main() -> int:
    if len(TERMINAL_SUFFIX) != 16:
        raise RuntimeError("qualified terminal suffix is not exactly 16 bytes")
    if not TERMINAL_SUFFIX.startswith(MPEG_SEQUENCE_END):
        raise RuntimeError("qualified terminal suffix lost MPEG sequence-end prefix")

    # network_live_stream_server imported the same subprocess module object.
    # Replacing Popen here therefore affects its capture spawn. Non-x11grab
    # processes, including the final archive software-decode check, delegate
    # unchanged to the original Popen implementation.
    subprocess.Popen = _sequence_end_popen

    print(
        "O3_SEQUENCE_END_WRAPPER=ACTIVE "
        "suffix_bytes=16 sequence_end_hex=000001b7 terminal_zero_bytes=12",
        flush=True,
    )

    return live.main()


if __name__ == "__main__":
    raise SystemExit(main())
