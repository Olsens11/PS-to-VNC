#!/usr/bin/env python3
"""File synopsis:
Evaluate the successor Issue #7 deployment, diagnostics, and capture apparatus.

This evaluator proves DUT readback identity, ordered startup diagnostics, and
presence of the required PS2-facing traffic. Physical presentation and detailed
RFB frame semantics remain separate qualification authorities.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


PS2_HOST = "192.168.50.2"
UDP_PORT = 5999
RFB_PORT = 5900
SHA256_RE = re.compile(r"[0-9a-f]{64}")


class ResultError(RuntimeError):
    pass


def fail(message: str) -> "NoReturn":
    raise ResultError(message)


def parse_key_values(path: Path) -> dict[str, str]:
    if not path.is_file():
        fail(f"required file missing: {path}")

    values: dict[str, str] = {}

    for number, raw in enumerate(
        path.read_text(encoding="utf-8").splitlines(),
        start=1,
    ):
        line = raw.strip()

        if not line or line.startswith("#"):
            continue

        if "=" not in raw:
            fail(f"malformed key/value line {number}: {path}")

        key, value = raw.split("=", 1)

        if not key or not value:
            fail(f"empty key/value line {number}: {path}")

        if key in values:
            fail(f"duplicate key {key}: {path}")

        values[key] = value

    return values


def read_payloads(path: Path) -> list[str]:
    if not path.is_file():
        fail(f"UDP JSONL missing: {path}")

    payloads: list[str] = []

    for number, raw in enumerate(
        path.read_text(encoding="utf-8").splitlines(),
        start=1,
    ):
        if not raw.strip():
            continue

        try:
            record = json.loads(raw)
        except json.JSONDecodeError as exc:
            fail(f"malformed UDP JSON line {number}: {exc}")

        payload = record.get("payload")

        if not isinstance(payload, str):
            fail(f"UDP record lacks string payload line {number}")

        payloads.append(payload)

    return payloads


def evaluate_payloads(
    manifest: dict[str, str],
    payloads: list[str],
) -> tuple[bool, dict[str, int]]:
    identity = (
        "PS2VNC_ID version=1 "
        f"test={manifest['HARDWARE_TEST_ID']} "
        f"digest={manifest['HARDWARE_ELF_IDENTITY_SHA256']}"
    )

    required = (
        identity,
        "PSTVNC_STAGE NET_READY",
        "PSTVNC_STAGE GS_READY",
        "PSTVNC_STAGE DESKTOP_READY",
    )

    counts = {
        payload: payloads.count(payload)
        for payload in required
    }

    counts["PSTVNC_STAGE FATAL"] = payloads.count(
        "PSTVNC_STAGE FATAL"
    )

    if any(counts[payload] != 1 for payload in required):
        return False, counts

    if counts["PSTVNC_STAGE FATAL"] != 0:
        return False, counts

    indexes = [
        payloads.index(payload)
        for payload in required
    ]

    return indexes == sorted(indexes), counts


def count_packets(
    pcap: Path,
    expression: list[str],
) -> int:
    if shutil.which("tcpdump") is None:
        fail("tcpdump is required for packet-capture analysis")

    if not pcap.is_file() or pcap.stat().st_size <= 24:
        return 0

    result = subprocess.run(
        [
            "tcpdump",
            "-nn",
            "-r",
            str(pcap),
            *expression,
        ],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )

    if result.returncode not in (0, 1):
        fail(
            "tcpdump could not read capture "
            f"rc={result.returncode}: {result.stderr.strip()}"
        )

    return sum(
        1
        for line in result.stdout.splitlines()
        if line.strip()
    )


def verify_raw_ledger(run: Path) -> None:
    ledger = run / "RAW-SHA256.txt"

    if not ledger.is_file():
        fail(f"raw evidence ledger missing: {ledger}")

    result = subprocess.run(
        [
            "sha256sum",
            "-c",
            "RAW-SHA256.txt",
        ],
        cwd=run,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )

    if result.returncode != 0:
        fail(
            "raw evidence SHA256 verification failed: "
            + result.stdout
            + result.stderr
        )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("run", type=Path)
    args = parser.parse_args()

    run = args.run.expanduser().resolve()

    if not run.is_dir():
        fail(f"run directory not found: {run}")

    verify_raw_ledger(run)

    manifest = parse_key_values(
        run / "manifest.env"
    )

    deployment_path = run / "deployment.json"

    if not deployment_path.is_file():
        fail("deployment record is missing")

    deployment = json.loads(
        deployment_path.read_text(encoding="utf-8")
    )

    expected_sha = manifest[
        "HARDWARE_STAMPED_ELF_SHA256"
    ]

    if SHA256_RE.fullmatch(expected_sha) is None:
        fail("manifest stamped ELF SHA256 is malformed")

    for key in (
        "elf_sha256",
        "unique_readback_sha256",
        "rolling_readback_sha256",
    ):
        if deployment.get(key) != expected_sha:
            fail(f"deployment identity mismatch: {key}")

    payloads = read_payloads(
        run / "observers/udp5999.jsonl"
    )

    sequence_pass, counts = evaluate_payloads(
        manifest,
        payloads,
    )

    pcap = run / "observers/ps2-facing.pcap"

    rfb_packets = count_packets(
        pcap,
        [
            "host",
            PS2_HOST,
            "and",
            "tcp",
            "port",
            str(RFB_PORT),
        ],
    )

    diagnostic_packets = count_packets(
        pcap,
        [
            "host",
            PS2_HOST,
            "and",
            "udp",
            "port",
            str(UDP_PORT),
        ],
    )

    capture_pass = (
        rfb_packets > 0
        and diagnostic_packets >= 4
    )

    identity = (
        "PS2VNC_ID version=1 "
        f"test={manifest['HARDWARE_TEST_ID']} "
        f"digest={manifest['HARDWARE_ELF_IDENTITY_SHA256']}"
    )

    apparatus_pass = sequence_pass and capture_pass

    lines = [
        "ISSUE7_APPARATUS_RESULT_VERSION=1",
        f"TEST_ID={manifest['HARDWARE_TEST_ID']}",
        "DEPLOYMENT_READBACK=PASS",
        (
            "RUNTIME_IDENTITY_COUNT="
            f"{counts[identity]}"
        ),
        (
            "NET_READY_COUNT="
            f"{counts['PSTVNC_STAGE NET_READY']}"
        ),
        (
            "GS_READY_COUNT="
            f"{counts['PSTVNC_STAGE GS_READY']}"
        ),
        (
            "DESKTOP_READY_COUNT="
            f"{counts['PSTVNC_STAGE DESKTOP_READY']}"
        ),
        (
            "FATAL_COUNT="
            f"{counts['PSTVNC_STAGE FATAL']}"
        ),
        (
            "ORDERED_STARTUP_DIAGNOSTICS="
            + ("PASS" if sequence_pass else "FAIL")
        ),
        f"PCAP_RFB_PACKET_COUNT={rfb_packets}",
        (
            "PCAP_DIAGNOSTIC_PACKET_COUNT="
            f"{diagnostic_packets}"
        ),
        (
            "PCAP_BASIC_TRAFFIC="
            + ("PASS" if capture_pass else "FAIL")
        ),
        (
            "ISSUE7_APPARATUS_RESULT="
            + ("PASS" if apparatus_pass else "FAIL")
        ),
        "PHYSICAL_OPERATOR_RESULT=SEPARATE_AUTHORITY",
        "RFB_FRAME_SEMANTIC_ANALYSIS=SEPARATE_AUTHORITY",
    ]

    text = "\n".join(lines) + "\n"

    sys.stdout.write(text)

    (run / "ANALYSIS-RESULT.txt").write_text(
        text,
        encoding="utf-8",
    )

    return 0 if apparatus_pass else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ResultError as exc:
        print(
            f"ISSUE7_RESULT_ERROR={exc}",
            file=sys.stderr,
        )
        raise SystemExit(1)
