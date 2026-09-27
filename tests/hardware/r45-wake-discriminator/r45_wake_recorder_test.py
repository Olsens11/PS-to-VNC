#!/usr/bin/env python3
"""Host-only regression for the R45 silent recorder contract."""

from __future__ import annotations

import ast
import io
from pathlib import Path
import socket
import sys
import threading

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import r45_wake_recorder as recorder  # noqa: E402


def record_bytes(variant_name: str, cycle: int) -> bytes:
    variant = recorder.VARIANTS[variant_name]
    return recorder.RECORD.pack(
        recorder.MAGIC,
        recorder.VERSION,
        variant["code"],
        recorder.KIND_COMPLETION,
        0,
        cycle,
        cycle,
        variant["timeout_us"],
        recorder.checksum(
            cycle,
            cycle,
            variant["code"],
            variant["timeout_us"],
        ),
    )


def run_pass_case(variant_name: str) -> None:
    receiver, peer = socket.socketpair()
    receiver.settimeout(0.25)
    records = io.BytesIO()

    def writer() -> None:
        try:
            for cycle in range(1, 5):
                peer.sendall(record_bytes(variant_name, cycle))
        finally:
            peer.close()

    thread = threading.Thread(target=writer)
    thread.start()
    try:
        result = recorder.consume_connection(
            receiver,
            variant_name,
            4,
            records,
        )
    finally:
        receiver.close()
    thread.join()

    assert result["status"] == "PASS", result
    assert result["completed_cycles"] == 4, result
    assert result["last_sequence"] == 4, result
    assert result["bytes_received"] == 4 * recorder.RECORD.size, result
    assert result["peer_application_bytes_sent"] == 0, result
    assert result["completion_authority"] == "ACTUAL_SERIALIZED_OUTBOUND_SEQUENCE"
    assert len(records.getvalue().splitlines()) == 4


def run_stall_case() -> None:
    receiver, peer = socket.socketpair()
    receiver.settimeout(0.01)
    try:
        result = recorder.consume_connection(receiver, "baseline", 1)
    finally:
        receiver.close()
        peer.close()

    assert result["status"] == "STALL", result
    assert result["completed_cycles"] == 0, result
    assert result["stalled_cycle"] == 1, result
    assert result["peer_application_bytes_sent"] == 0, result


def run_invalid_sequence_case() -> None:
    receiver, peer = socket.socketpair()
    receiver.settimeout(0.25)

    def writer() -> None:
        try:
            peer.sendall(record_bytes("baseline", 2))
        finally:
            peer.close()

    thread = threading.Thread(target=writer)
    thread.start()
    try:
        result = recorder.consume_connection(receiver, "baseline", 2)
    finally:
        receiver.close()
    thread.join()

    assert result["status"] == "APPARATUS_INVALID", result
    assert result["failed_cycle"] == 1, result
    assert result["peer_application_bytes_sent"] == 0, result


def prove_recorder_never_sends_application_bytes() -> None:
    source_path = HERE / "r45_wake_recorder.py"
    tree = ast.parse(source_path.read_text())

    forbidden = []
    for node in ast.walk(tree):
        if not isinstance(node, ast.Call):
            continue
        function = node.func
        if isinstance(function, ast.Attribute) and function.attr in {
            "send",
            "sendall",
            "sendto",
        }:
            forbidden.append((function.attr, node.lineno))

    assert forbidden == [], forbidden


def main() -> int:
    run_pass_case("baseline")
    run_pass_case("control")
    run_stall_case()
    run_invalid_sequence_case()
    prove_recorder_never_sends_application_bytes()
    print("R45_WAKE_RECORDER_TEST=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
