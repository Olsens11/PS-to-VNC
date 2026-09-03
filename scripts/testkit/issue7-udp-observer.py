#!/usr/bin/env python3
"""File synopsis:
Capture Issue #7 runtime identity and stage datagrams as timestamped JSONL.

The observer owns only UDP receipt and byte-preserving payload recording. It
does not infer qualification, trigger recovery, or control the DUT.
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import signal
import socket


def now_iso() -> str:
    return datetime.now(timezone.utc).astimezone().isoformat()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--log", required=True, type=Path)
    parser.add_argument("--bind", required=True)
    parser.add_argument("--port", required=True, type=int)
    parser.add_argument("--owner-token", required=True)
    args = parser.parse_args()

    args.log.parent.mkdir(parents=True, exist_ok=True)

    stop_requested = False

    def request_stop(_signum, _frame) -> None:
        nonlocal stop_requested
        stop_requested = True

    signal.signal(signal.SIGTERM, request_stop)
    signal.signal(signal.SIGINT, request_stop)

    receiver = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    receiver.settimeout(0.5)
    receiver.bind((args.bind, args.port))

    print(
        "ISSUE7_UDP_OBSERVER_STARTED=YES "
        f"bind={args.bind}:{args.port} "
        f"owner={args.owner_token}",
        flush=True,
    )

    sequence = 0

    with args.log.open(
        "a",
        encoding="utf-8",
        buffering=1,
    ) as stream:
        while not stop_requested:
            try:
                payload, sender = receiver.recvfrom(65535)
            except socket.timeout:
                continue
            except OSError:
                if stop_requested:
                    break
                raise

            sequence += 1

            record = {
                "sequence": sequence,
                "timestamp": now_iso(),
                "sender_ip": sender[0],
                "sender_port": sender[1],
                "payload": payload.decode(
                    "utf-8",
                    errors="replace",
                ),
            }

            stream.write(
                json.dumps(
                    record,
                    sort_keys=True,
                    separators=(",", ":"),
                )
                + "\n"
            )

    receiver.close()

    print(
        "ISSUE7_UDP_OBSERVER_STOPPED=YES",
        flush=True,
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
