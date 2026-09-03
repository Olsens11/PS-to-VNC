#!/usr/bin/env python3

"""File synopsis:
Prove successor identity tooling reproduces sealed historical stamps exactly.
"""

from __future__ import annotations

import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

MAGIC = b"PS2VNCIDv1!BLOB!\x00"
TEST_SIZE = 64
DIGEST_SIZE = 65
PRISTINE_TEST = b"UNSTAMPED\x00" + b"\x00" * (TEST_SIZE - 10)
ZERO_DIGEST = b"0" * 64 + b"\x00"
TOOL = Path("scripts/testkit/elf-identity.py")

FIXTURES = [
    Path("evidence/m4/m4i-c2-hw1-identity-prep/PS2VNC-M4I-C2-HW1.ELF"),
    Path("evidence/m4/m4i-c2-hw2-identity-prep/PS2VNC-M4I-C2-HW2.ELF"),
    Path("evidence/m4/m4i-final-hw1-identity-prep/PS2VNC-M4I-FINAL-HW1.ELF"),
]


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def fields(data: bytes) -> tuple[int, int, int, str, str]:
    if data.count(MAGIC) != 1:
        raise SystemExit("fixture identity magic is not unique")
    offset = data.find(MAGIC)
    test_start = offset + len(MAGIC)
    digest_start = test_start + TEST_SIZE
    end = digest_start + DIGEST_SIZE
    raw_test = data[test_start:digest_start]
    raw_digest = data[digest_start:end]
    test_id = raw_test.split(b"\x00", 1)[0].decode("ascii")
    digest = raw_digest[:-1].decode("ascii")
    return test_start, digest_start, end, test_id, digest


def run(*args: str) -> str:
    result = subprocess.run(
        [sys.executable, str(TOOL), *args],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        sys.stderr.write(result.stderr)
        raise SystemExit(f"identity tool failed: {' '.join(args)}")
    return result.stdout


def main() -> None:
    if not TOOL.is_file():
        raise SystemExit(f"missing successor identity tool: {TOOL}")

    with tempfile.TemporaryDirectory() as temp_name:
        temp = Path(temp_name)

        for index, fixture in enumerate(FIXTURES, 1):
            stamped = fixture.read_bytes()
            test_start, digest_start, end, test_id, digest = fields(stamped)

            pristine = bytearray(stamped)
            pristine[test_start:digest_start] = PRISTINE_TEST
            pristine[digest_start:end] = ZERO_DIGEST

            candidate = temp / f"fixture-{index}.ELF"
            candidate.write_bytes(bytes(pristine))

            before_sha = sha(candidate.read_bytes())
            output = run("stamp", str(candidate), test_id)
            if candidate.read_bytes() != stamped:
                raise SystemExit(
                    f"fixture {index} successor stamp is not byte-exact"
                )
            if f"ELF_IDENTITY_SHA256={digest}" not in output:
                raise SystemExit(
                    f"fixture {index} successor stamp reported wrong digest"
                )

            verify_output = run("verify", str(candidate), test_id, digest)
            if "TESTKIT_VERIFY_ELF_IDENTITY=PASS" not in verify_output:
                raise SystemExit(f"fixture {index} successor verify did not pass")

            print(f"FIXTURE_{index}_TEST_ID={test_id}")
            print(f"FIXTURE_{index}_NORMALIZED_PRISTINE_SHA256={before_sha}")
            print(f"FIXTURE_{index}_STAMPED_SHA256={sha(stamped)}")
            print(f"FIXTURE_{index}_IDENTITY_SHA256={digest}")
            print(f"FIXTURE_{index}_BYTE_EXACT_REPRODUCTION=PASS")

        # Fail-closed checks: already-stamped input must not be stamped again,
        # and an overlong test ID must not mutate a pristine candidate.
        stamped_copy = temp / "already-stamped.ELF"
        shutil.copyfile(FIXTURES[0], stamped_copy)
        before = stamped_copy.read_bytes()
        result = subprocess.run(
            [sys.executable, str(TOOL), "stamp", str(stamped_copy), "RESTAMP"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if result.returncode == 0 or stamped_copy.read_bytes() != before:
            raise SystemExit("already-stamped fail-closed check failed")

        pristine_copy = temp / "overlong.ELF"
        data = bytearray(FIXTURES[0].read_bytes())
        test_start, digest_start, end, _, _ = fields(bytes(data))
        data[test_start:digest_start] = PRISTINE_TEST
        data[digest_start:end] = ZERO_DIGEST
        pristine_copy.write_bytes(bytes(data))
        before = pristine_copy.read_bytes()
        result = subprocess.run(
            [
                sys.executable,
                str(TOOL),
                "stamp",
                str(pristine_copy),
                "X" * 64,
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if result.returncode == 0 or pristine_copy.read_bytes() != before:
            raise SystemExit("overlong TEST_ID fail-closed check failed")

    print("HISTORICAL_FIXTURE_COUNT=3")
    print("IDENTITY_WHOLE_ELF_ZERO_DIGEST_CONTRACT=PROVEN")
    print("ALREADY_STAMPED_FAIL_CLOSED=PASS")
    print("OVERLONG_TEST_ID_FAIL_CLOSED=PASS")
    print("SUCCESSOR_IDENTITY_COMPATIBILITY=PASS")


if __name__ == "__main__":
    main()
