#!/usr/bin/env python3
"""
File synopsis:
    EXP3 O3 EOF E2 diagnostic wrapper.

This replays the exact failed O3 live archive after appending one literal MPEG
sequence-end start code (00 00 01 B7) followed by twelve zero bytes. The 16-byte
suffix deliberately preserves the original stream's mod-16 == 0 terminal
alignment and therefore preserves zero final DMA padding.

E2 isolates whether an explicit MPEG sequence-end code gives SMS a principled
terminal condition even when the exact qword-aligned EOF boundary that hung in
the original live run is retained.
"""

from __future__ import annotations

import argparse
import pathlib
import sys
import tempfile

import network_live_eof_replay_server as replay


SUFFIX = bytes.fromhex("000001b7") + (b"\x00" * 12)


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

    payload = source + SUFFIX

    print(
        "O3_EOF_E2_SOURCE "
        f"input={source_path} original_bytes={len(source)} "
        f"suffix_bytes={len(SUFFIX)} wire_payload_bytes={len(payload)} "
        f"original_mod16={len(source) % 16} e2_mod16={len(payload) % 16} "
        f"sequence_end_hex=000001b7 terminal_zero_bytes=12",
        flush=True,
    )

    if len(SUFFIX) != 16:
        raise SystemExit("E2 suffix must be exactly 16 bytes")
    if len(source) % 16 != 0 or len(payload) % 16 != 0:
        raise SystemExit("E2 must preserve mod-16 == 0 terminal alignment")

    temporary_path: pathlib.Path | None = None

    try:
        with tempfile.NamedTemporaryFile(
            prefix="ps2vnc-o3-e2-",
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

        print("O3_EOF_E2_OVERALL=PASS", flush=True)
        return result
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


if __name__ == "__main__":
    raise SystemExit(main())
