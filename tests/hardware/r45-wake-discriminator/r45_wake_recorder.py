#!/usr/bin/env python3
"""R45 Pi-side silent recorder for the sole-owner wake discriminator.

The recorder receives fixed-size completion records and never sends application
bytes to the PS2. A timeout is a measurement/classification fact only; PASS
requires every expected owner-serialized record in exact monotonic order.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import socket
import struct
import time
from typing import BinaryIO

MAGIC = b"R45W"
VERSION = 1
KIND_COMPLETION = 1
RECORD = struct.Struct("!4sBBBBIIII")

VARIANTS = {
    "baseline": {"code": 1, "name": "BASELINE_1000US", "timeout_us": 1000},
    "control": {"code": 2, "name": "CONTROL_ZERO_TIMEOUT", "timeout_us": 0},
}


def checksum(cycle: int, sequence: int, variant_code: int, timeout_us: int) -> int:
    return (
        0x52343557
        ^ 0x01000000
        ^ (variant_code << 16)
        ^ (KIND_COMPLETION << 8)
        ^ cycle
        ^ sequence
        ^ timeout_us
    ) & 0xFFFFFFFF


def decode_record(data: bytes, expected_variant: str, expected_cycle: int) -> dict:
    if len(data) != RECORD.size:
        raise ValueError(f"record-size:{len(data)}")

    magic, version, variant, kind, reserved, cycle, sequence, timeout_us, observed = (
        RECORD.unpack(data)
    )
    expected = VARIANTS[expected_variant]

    if magic != MAGIC:
        raise ValueError("magic")
    if version != VERSION:
        raise ValueError("version")
    if variant != expected["code"]:
        raise ValueError("variant")
    if kind != KIND_COMPLETION:
        raise ValueError("kind")
    if reserved != 0:
        raise ValueError("reserved")
    if cycle != expected_cycle:
        raise ValueError(f"cycle:{cycle}:expected:{expected_cycle}")
    if sequence != expected_cycle:
        raise ValueError(f"sequence:{sequence}:expected:{expected_cycle}")
    if timeout_us != expected["timeout_us"]:
        raise ValueError(f"timeout:{timeout_us}:expected:{expected['timeout_us']}")
    if observed != checksum(cycle, sequence, variant, timeout_us):
        raise ValueError("checksum")

    return {
        "variant": expected["name"],
        "cycle": cycle,
        "sequence": sequence,
        "readiness_timeout_us": timeout_us,
        "checksum": observed,
    }


def _recv_exact(connection: socket.socket, count: int) -> bytes | None:
    pieces: list[bytes] = []
    remaining = count

    while remaining:
        chunk = connection.recv(remaining)
        if not chunk:
            return None if not pieces else b"".join(pieces)
        pieces.append(chunk)
        remaining -= len(chunk)

    return b"".join(pieces)


def consume_connection(
    connection: socket.socket,
    expected_variant: str,
    cycles: int,
    records_file: BinaryIO | None = None,
) -> dict:
    started = time.monotonic()
    completed = 0
    bytes_received = 0

    try:
        for expected_cycle in range(1, cycles + 1):
            try:
                data = _recv_exact(connection, RECORD.size)
            except socket.timeout:
                return {
                    "status": "STALL",
                    "variant": VARIANTS[expected_variant]["name"],
                    "completed_cycles": completed,
                    "stalled_cycle": expected_cycle,
                    "last_sequence": completed,
                    "owner_phase": "POST_FIRST_POLL_READINESS_OR_NEXT_OWNER_PASS",
                    "bytes_received": bytes_received,
                    "peer_application_bytes_sent": 0,
                    "elapsed_seconds": time.monotonic() - started,
                }

            if data is None or len(data) != RECORD.size:
                return {
                    "status": "APPARATUS_INVALID",
                    "reason": "peer-eof-or-partial-record",
                    "variant": VARIANTS[expected_variant]["name"],
                    "completed_cycles": completed,
                    "failed_cycle": expected_cycle,
                    "last_sequence": completed,
                    "bytes_received": bytes_received + (0 if data is None else len(data)),
                    "peer_application_bytes_sent": 0,
                    "elapsed_seconds": time.monotonic() - started,
                }

            bytes_received += len(data)

            try:
                decoded = decode_record(data, expected_variant, expected_cycle)
            except ValueError as error:
                return {
                    "status": "APPARATUS_INVALID",
                    "reason": str(error),
                    "variant": VARIANTS[expected_variant]["name"],
                    "completed_cycles": completed,
                    "failed_cycle": expected_cycle,
                    "last_sequence": completed,
                    "bytes_received": bytes_received,
                    "peer_application_bytes_sent": 0,
                    "elapsed_seconds": time.monotonic() - started,
                }

            completed = expected_cycle
            if records_file is not None:
                records_file.write((json.dumps(decoded, sort_keys=True) + "\n").encode())
                records_file.flush()
    except OSError as error:
        return {
            "status": "APPARATUS_INVALID",
            "reason": f"socket:{error}",
            "variant": VARIANTS[expected_variant]["name"],
            "completed_cycles": completed,
            "failed_cycle": completed + 1,
            "last_sequence": completed,
            "bytes_received": bytes_received,
            "peer_application_bytes_sent": 0,
            "elapsed_seconds": time.monotonic() - started,
        }

    return {
        "status": "PASS",
        "variant": VARIANTS[expected_variant]["name"],
        "completed_cycles": completed,
        "stalled_cycle": None,
        "last_sequence": completed,
        "bytes_received": bytes_received,
        "peer_application_bytes_sent": 0,
        "completion_authority": "ACTUAL_SERIALIZED_OUTBOUND_SEQUENCE",
        "elapsed_seconds": time.monotonic() - started,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Silent Pi recorder for the R45 PS2 wake discriminator."
    )
    parser.add_argument("--bind", default="192.168.50.1")
    parser.add_argument("--port", type=int, default=5961)
    parser.add_argument("--variant", choices=sorted(VARIANTS), required=True)
    parser.add_argument("--cycles", type=int, default=4096)
    parser.add_argument(
        "--idle-timeout",
        type=float,
        default=10.0,
        help="measurement timeout only; never completion authority",
    )
    parser.add_argument("--records", type=Path, required=True)
    parser.add_argument("--summary", type=Path, required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if args.cycles <= 0 or args.port <= 0 or args.port > 65535 or args.idle_timeout <= 0:
        raise SystemExit("invalid recorder configuration")

    args.records.parent.mkdir(parents=True, exist_ok=True)
    args.summary.parent.mkdir(parents=True, exist_ok=True)

    result: dict
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind((args.bind, args.port))
        listener.listen(1)

        connection, peer = listener.accept()
        with connection:
            connection.settimeout(args.idle_timeout)
            with args.records.open("wb") as records_file:
                result = consume_connection(
                    connection,
                    args.variant,
                    args.cycles,
                    records_file,
                )
            result["peer"] = peer[0]
            result["listen_port"] = args.port
            result["expected_cycles"] = args.cycles

    args.summary.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))

    if result["status"] == "PASS":
        return 0
    if result["status"] == "STALL":
        return 3
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
