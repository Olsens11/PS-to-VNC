#!/usr/bin/env python3

"""Successor-owned TestKit ELF identity stamp/verify implementation.

The digest contract is recovered from sealed historical artifacts and is kept
small and explicit:

1. the fixed identity blob must occur exactly once;
2. stamp writes the requested test ID into its fixed 64-byte field;
3. the 65-byte digest field is set to 64 ASCII zeroes plus NUL;
4. identity digest = SHA-256 of the entire ELF in that normalized state;
5. the 64 lowercase hex digest plus NUL is written into the digest field.

No ELF layout, addresses, or file length are changed.
"""

from __future__ import annotations

import hashlib
from pathlib import Path
import string
import sys

MAGIC = b"PS2VNCIDv1!BLOB!\x00"
TEST_SIZE = 64
DIGEST_SIZE = 65
PRISTINE_TEST = b"UNSTAMPED\x00" + b"\x00" * (TEST_SIZE - 10)
ZERO_DIGEST = b"0" * 64 + b"\x00"
LOWER_HEX = set(string.hexdigits.lower()) & set(string.ascii_lowercase + string.digits)


class IdentityError(Exception):
    pass


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def encode_test_id(test_id: str) -> bytes:
    if not test_id:
        raise IdentityError("TEST_ID must not be empty")
    try:
        encoded = test_id.encode("ascii")
    except UnicodeEncodeError as exc:
        raise IdentityError("TEST_ID must be ASCII") from exc
    if len(encoded) > 63:
        raise IdentityError("TEST_ID exceeds 63 visible bytes")
    if b"\x00" in encoded:
        raise IdentityError("TEST_ID contains NUL")
    return encoded + b"\x00" + b"\x00" * (TEST_SIZE - len(encoded) - 1)


def locate_blob(data: bytes) -> tuple[int, int, int]:
    offsets: list[int] = []
    start = 0
    while True:
        offset = data.find(MAGIC, start)
        if offset < 0:
            break
        offsets.append(offset)
        start = offset + 1
    if len(offsets) != 1:
        raise IdentityError(f"identity magic count is {len(offsets)}, expected 1")
    offset = offsets[0]
    test_start = offset + len(MAGIC)
    digest_start = test_start + TEST_SIZE
    end = digest_start + DIGEST_SIZE
    if end > len(data):
        raise IdentityError("identity blob is truncated")
    return offset, test_start, digest_start


def decode_test_field(field: bytes) -> str:
    if len(field) != TEST_SIZE:
        raise IdentityError("test ID field has wrong size")
    try:
        end = field.index(0)
    except ValueError as exc:
        raise IdentityError("test ID field is not NUL-terminated") from exc
    if any(field[end + 1 :]):
        raise IdentityError("test ID field has nonzero bytes after NUL")
    if end == 0:
        raise IdentityError("test ID field is empty")
    try:
        return field[:end].decode("ascii")
    except UnicodeDecodeError as exc:
        raise IdentityError("test ID field is not ASCII") from exc


def decode_digest_field(field: bytes) -> str:
    if len(field) != DIGEST_SIZE or field[-1:] != b"\x00":
        raise IdentityError("digest field has wrong size or terminator")
    try:
        digest = field[:-1].decode("ascii")
    except UnicodeDecodeError as exc:
        raise IdentityError("digest field is not ASCII") from exc
    if len(digest) != 64 or any(c not in "0123456789abcdef" for c in digest):
        raise IdentityError("digest field is not 64 lowercase hex characters")
    return digest


def stamp(path: Path, test_id: str) -> tuple[int, str, str]:
    original = path.read_bytes()
    offset, test_start, digest_start = locate_blob(original)
    end = digest_start + DIGEST_SIZE

    if original[test_start:digest_start] != PRISTINE_TEST:
        raise IdentityError("ELF test ID field is not pristine UNSTAMPED state")
    if original[digest_start:end] != ZERO_DIGEST:
        raise IdentityError("ELF digest field is not pristine zero state")

    staged = bytearray(original)
    staged[test_start:digest_start] = encode_test_id(test_id)
    staged[digest_start:end] = ZERO_DIGEST
    identity_digest = sha256(bytes(staged))
    staged[digest_start:end] = identity_digest.encode("ascii") + b"\x00"

    final = bytes(staged)
    if len(final) != len(original):
        raise IdentityError("identity stamp changed ELF size")

    path.write_bytes(final)
    return offset, identity_digest, sha256(final)


def verify(path: Path, expected_test_id: str, expected_digest: str) -> tuple[int, str]:
    data = path.read_bytes()
    offset, test_start, digest_start = locate_blob(data)
    end = digest_start + DIGEST_SIZE

    actual_test_id = decode_test_field(data[test_start:digest_start])
    actual_digest = decode_digest_field(data[digest_start:end])

    if actual_test_id != expected_test_id:
        raise IdentityError(
            f"test ID mismatch expected={expected_test_id} actual={actual_test_id}"
        )
    if len(expected_digest) != 64 or any(
        c not in "0123456789abcdef" for c in expected_digest
    ):
        raise IdentityError("expected digest must be 64 lowercase hex characters")
    if actual_digest != expected_digest:
        raise IdentityError(
            f"embedded digest mismatch expected={expected_digest} actual={actual_digest}"
        )

    normalized = bytearray(data)
    normalized[digest_start:end] = ZERO_DIGEST
    recomputed = sha256(bytes(normalized))
    if recomputed != actual_digest:
        raise IdentityError(
            f"identity digest verification failed embedded={actual_digest} recomputed={recomputed}"
        )

    return offset, sha256(data)


def print_common(path: Path, offset: int, test_id: str, digest: str, final_sha: str) -> None:
    print("TESTKIT_ELF_IDENTITY_VERSION=1")
    print(f"ELF={path}")
    print(f"ELF_IDENTITY_BLOB_OFFSET={offset}")
    print(f"ELF_IDENTITY_TEST_ID={test_id}")
    print(f"ELF_IDENTITY_SHA256={digest}")
    print(f"ELF_FINAL_SHA256={final_sha}")


def usage() -> None:
    print(
        "usage: elf-identity.py stamp ELF TEST_ID | verify ELF TEST_ID IDENTITY_SHA256",
        file=sys.stderr,
    )
    raise SystemExit(2)


def main() -> None:
    if len(sys.argv) < 2:
        usage()

    command = sys.argv[1]
    try:
        if command == "stamp":
            if len(sys.argv) != 4:
                usage()
            path = Path(sys.argv[2])
            test_id = sys.argv[3]
            if not path.is_file():
                raise IdentityError(f"ELF not found: {path}")
            offset, digest, final_sha = stamp(path, test_id)
            print_common(path, offset, test_id, digest, final_sha)
            print("TESTKIT_STAMP_ELF_IDENTITY=PASS")
            return

        if command == "verify":
            if len(sys.argv) != 5:
                usage()
            path = Path(sys.argv[2])
            test_id = sys.argv[3]
            digest = sys.argv[4]
            if not path.is_file():
                raise IdentityError(f"ELF not found: {path}")
            offset, final_sha = verify(path, test_id, digest)
            print_common(path, offset, test_id, digest, final_sha)
            print("TESTKIT_VERIFY_ELF_IDENTITY=PASS")
            return

        usage()
    except IdentityError as exc:
        print(f"TESTKIT_ELF_IDENTITY_ERROR={exc}", file=sys.stderr)
        raise SystemExit(1)


if __name__ == "__main__":
    main()
