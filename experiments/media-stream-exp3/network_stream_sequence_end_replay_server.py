#!/usr/bin/env python3
"""
File synopsis:
    EXP3 generic finite MPEG archive replay with the hardware-qualified
    sequence-end suffix.

This wrapper is intentionally separate from the E2 alignment diagnostic.
It accepts an arbitrary MPEG-2 elementary stream, appends the exact terminal
suffix qualified by E2 (00 00 01 B7 followed by twelve zero bytes), and then
reuses the existing O3 EOF replay transport/telemetry implementation.

Unlike network_live_sequence_end_replay_server.py, this file does not require
mod-16 == 0 input or output. Its purpose is ordinary finite-stream replay after
E2 established explicit MPEG sequence termination as the correct mechanism.
"""

from __future__ import annotations

import argparse
import pathlib
import sys
import tempfile

import network_live_eof_replay_server as replay


MPEG_SEQUENCE_END = bytes.fromhex("000001b7")
TERMINAL_SUFFIX = MPEG_SEQUENCE_END + (b"\x00" * 12)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--listen", default="192.168.50.1")
    parser.add_argument("--port", type=int, default=5904)
    parser.add_argument("--queue-capacity", type=int, default=32768)
    parser.add_argument("--receiver-stack", type=int, default=16384)
    parser.add_argument("--receiver-priority", type=int, default=63)
    args = parser.parse_args()

    source_path = pathlib.Path(args.input)
    source = source_path.read_bytes()

    if not source:
        raise SystemExit("input archive is empty")

    if len(TERMINAL_SUFFIX) != 16:
        raise SystemExit("qualified terminal suffix must be exactly 16 bytes")
    if not TERMINAL_SUFFIX.startswith(MPEG_SEQUENCE_END):
        raise SystemExit("qualified terminal suffix lost MPEG sequence-end prefix")

    existing_sequence_ends = source.count(MPEG_SEQUENCE_END)
    if existing_sequence_ends != 0:
        raise SystemExit(
            "input already contains MPEG sequence-end code; generic replay "
            "expects the un-terminated source archive"
        )

    payload = source + TERMINAL_SUFFIX

    print(
        "O3_GENERIC_SEQUENCE_END_REPLAY_SOURCE "
        f"input={source_path} original_bytes={len(source)} "
        f"suffix_bytes={len(TERMINAL_SUFFIX)} wire_payload_bytes={len(payload)} "
        f"original_mod16={len(source) % 16} wire_mod16={len(payload) % 16} "
        f"sequence_end_hex=000001b7 terminal_zero_bytes=12",
        flush=True,
    )

    temporary_path: pathlib.Path | None = None

    try:
        with tempfile.NamedTemporaryFile(
            prefix="ps2vnc-o3-generic-seqend-",
            suffix=".m2v",
            delete=False,
        ) as temporary:
            temporary.write(payload)
            temporary.flush()
            temporary_path = pathlib.Path(temporary.name)

        old_argv = sys.argv
        sys.argv = [
            "network_live_eof_replay_server.py",
            "--input",
            str(temporary_path),
            "--append-zero-bytes",
            "0",
            "--listen",
            args.listen,
            "--port",
            str(args.port),
            "--queue-capacity",
            str(args.queue_capacity),
            "--receiver-stack",
            str(args.receiver_stack),
            "--receiver-priority",
            str(args.receiver_priority),
        ]

        try:
            result = replay.main()
        finally:
            sys.argv = old_argv

        print("O3_GENERIC_SEQUENCE_END_REPLAY_OVERALL=PASS", flush=True)
        return result
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


if __name__ == "__main__":
    raise SystemExit(main())
