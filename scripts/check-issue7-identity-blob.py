#!/usr/bin/env python3

import hashlib
from pathlib import Path
import sys

MAGIC = b"PS2VNCIDv1!BLOB!\x00"
TEST_ID_SIZE = 64
DIGEST_SIZE = 65


def fail(message: str) -> None:
    print(f"ISSUE7_IDENTITY_BLOB_CHECK=FAIL reason={message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        print("usage: check-issue7-identity-blob.py ELF", file=sys.stderr)
        raise SystemExit(2)

    path = Path(sys.argv[1])
    if not path.is_file():
        fail(f"missing-elf:{path}")

    data = path.read_bytes()
    offsets = []
    start = 0

    while True:
        offset = data.find(MAGIC, start)
        if offset < 0:
            break
        offsets.append(offset)
        start = offset + 1

    if len(offsets) != 1:
        fail(f"magic-count:{len(offsets)}")

    offset = offsets[0]
    test_start = offset + len(MAGIC)
    digest_start = test_start + TEST_ID_SIZE
    end = digest_start + DIGEST_SIZE

    if end > len(data):
        fail("truncated-blob")

    test_id = data[test_start:digest_start]
    digest = data[digest_start:end]

    expected_test = b"UNSTAMPED\x00" + (b"\x00" * (TEST_ID_SIZE - 10))
    expected_digest = (b"0" * 64) + b"\x00"

    if test_id != expected_test:
        fail("unexpected-pristine-test-id")

    if digest != expected_digest:
        fail("unexpected-pristine-digest")

    print("TESTKIT_ELF_IDENTITY_VERSION=1")
    print(f"ELF={path}")
    print(f"ELF_IDENTITY_BLOB_OFFSET={offset}")
    print("ELF_IDENTITY_TEST_ID=UNSTAMPED")
    print("ELF_IDENTITY_DIGEST=" + ("0" * 64))
    print(f"ELF_PRISTINE_SHA256={hashlib.sha256(data).hexdigest()}")
    print("ISSUE7_IDENTITY_BLOB_CHECK=PASS")


if __name__ == "__main__":
    main()
