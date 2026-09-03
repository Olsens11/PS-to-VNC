#!/usr/bin/env python3

"""File synopsis:
Read-only forensics for the historical TestKit ELF identity digest.

This script deliberately does not stamp or modify repository files. It tests a
small set of mechanically plausible normalization/hash formulas against multiple
already-stamped historical ELF fixtures and reports which formulas reproduce the
embedded digest. A formula is not successor authority merely because it matches;
the result is evidence used to decide whether exact tool reconstruction is safe.
"""

from __future__ import annotations

import hashlib
from pathlib import Path
import subprocess
import sys

MAGIC = b"PS2VNCIDv1!BLOB!\x00"
TEST_SIZE = 64
DIGEST_SIZE = 65
ZERO_DIGEST = (b"0" * 64) + b"\x00"
PRISTINE_TEST = b"UNSTAMPED\x00" + (b"\x00" * (TEST_SIZE - 10))

FIXTURES = [
    Path("evidence/m4/m4i-c2-hw1-identity-prep/PS2VNC-M4I-C2-HW1.ELF"),
    Path("evidence/m4/m4i-c2-hw2-identity-prep/PS2VNC-M4I-C2-HW2.ELF"),
    Path("evidence/m4/m4i-final-hw1-identity-prep/PS2VNC-M4I-FINAL-HW1.ELF"),
]


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def load_segments(path: Path) -> list[tuple[int, int]]:
    text = subprocess.check_output(
        ["readelf", "-W", "-l", str(path)],
        text=True,
    )
    result: list[tuple[int, int]] = []
    for line in text.splitlines():
        fields = line.split()
        if fields and fields[0] == "LOAD":
            result.append((int(fields[1], 16), int(fields[4], 16)))
    if not result:
        raise SystemExit(f"no PT_LOAD segments: {path}")
    return result


def joined_load(data: bytes, segments: list[tuple[int, int]]) -> bytes:
    return b"".join(data[offset : offset + size] for offset, size in segments)


def analyze(path: Path) -> dict[str, str]:
    if not path.is_file():
        raise SystemExit(f"fixture missing: {path}")

    original = path.read_bytes()
    offsets = []
    start = 0
    while True:
        offset = original.find(MAGIC, start)
        if offset < 0:
            break
        offsets.append(offset)
        start = offset + 1

    if len(offsets) != 1:
        raise SystemExit(f"identity magic count {len(offsets)}: {path}")

    offset = offsets[0]
    test_start = offset + len(MAGIC)
    digest_start = test_start + TEST_SIZE
    end = digest_start + DIGEST_SIZE
    if end > len(original):
        raise SystemExit(f"truncated identity blob: {path}")

    test_raw = original[test_start:digest_start]
    digest_raw = original[digest_start:end]
    test_id = test_raw.split(b"\x00", 1)[0].decode("ascii")
    expected = digest_raw[:-1].decode("ascii")

    if len(expected) != 64 or digest_raw[-1:] != b"\x00":
        raise SystemExit(f"malformed embedded digest: {path}")

    zero_digest = bytearray(original)
    zero_digest[digest_start:end] = ZERO_DIGEST

    pristine_blob = bytearray(zero_digest)
    pristine_blob[test_start:digest_start] = PRISTINE_TEST

    segments = load_segments(path)

    variants = {
        "WHOLE_ZERO_DIGEST_KEEP_TEST": sha(bytes(zero_digest)),
        "WHOLE_PRISTINE_BLOB": sha(bytes(pristine_blob)),
        "PT_LOAD_ZERO_DIGEST_KEEP_TEST": sha(
            joined_load(bytes(zero_digest), segments)
        ),
        "PT_LOAD_PRISTINE_BLOB": sha(
            joined_load(bytes(pristine_blob), segments)
        ),
    }

    result = {
        "fixture": str(path),
        "test_id": test_id,
        "expected": expected,
        "offset": str(offset),
    }
    result.update(variants)
    return result


def main() -> None:
    analyses = [analyze(path) for path in FIXTURES]
    methods = [
        "WHOLE_ZERO_DIGEST_KEEP_TEST",
        "WHOLE_PRISTINE_BLOB",
        "PT_LOAD_ZERO_DIGEST_KEEP_TEST",
        "PT_LOAD_PRISTINE_BLOB",
    ]

    print("IDENTITY_DIGEST_FORENSICS_VERSION=1")

    for index, row in enumerate(analyses, 1):
        print(f"FIXTURE_{index}={row['fixture']}")
        print(f"FIXTURE_{index}_TEST_ID={row['test_id']}")
        print(f"FIXTURE_{index}_BLOB_OFFSET={row['offset']}")
        print(f"FIXTURE_{index}_EMBEDDED_DIGEST={row['expected']}")
        for method in methods:
            actual = row[method]
            match = "YES" if actual == row["expected"] else "NO"
            print(f"FIXTURE_{index}_{method}_SHA256={actual}")
            print(f"FIXTURE_{index}_{method}_MATCH={match}")

    consistent = [
        method
        for method in methods
        if all(row[method] == row["expected"] for row in analyses)
    ]

    print("CONSISTENT_METHOD_COUNT=" + str(len(consistent)))
    print(
        "CONSISTENT_METHODS="
        + (",".join(consistent) if consistent else "NONE")
    )
    print("IDENTITY_DIGEST_FORENSICS=PASS")


if __name__ == "__main__":
    main()
