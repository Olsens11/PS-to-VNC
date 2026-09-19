#!/usr/bin/env python3
"""
File synopsis:
    Implements the Pi half of Q1-Q12 Proof 1A.

The server accepts one provisional TCP connection on the dedicated Wire port.
The PS2 must speak Wire framing first and send the minimal establishment HELLO.
Only after validating Wire/product versions does the Pi assign a non-zero
session ID and publish ACCEPT.

After ACCEPT the server deliberately starts no RFB, audio, MPEG, desktop,
configuration, or other rider. It observes an idle active Wire Session until
the PS2 closes it, proving that rider traffic is not a prerequisite.

This is experiment apparatus. It is not yet the persistent product service.
"""

from __future__ import annotations

import argparse
import secrets
import socket
import struct
import sys
import time

MAGIC = b"PSTV"
HEADER = struct.Struct(">4sBBBBII")
HEADER_BYTES = HEADER.size

WIRE_HEADER_VERSION = 1
FRAME_HELLO = 1
FRAME_ACCEPT = 12
FRAME_NOT_ACCEPTED = 13
CHANNEL_CONTROL = 0

WIRE_VERSION = 1
PRODUCT_VERSION = 1

HELLO = struct.Struct(">II")
ONE_WORD = struct.Struct(">I")

REJECT_WIRE_VERSION = 1
REJECT_PRODUCT_VERSION = 2
REJECT_MALFORMED = 3


class ProofError(RuntimeError):
    pass


def read_exact(sock: socket.socket, count: int) -> bytes:
    chunks: list[bytes] = []
    remaining = count

    while remaining:
        chunk = sock.recv(remaining)
        if not chunk:
            raise EOFError("peer closed during framed read")
        chunks.append(chunk)
        remaining -= len(chunk)

    return b"".join(chunks)


def encode_frame(kind: int, sequence: int, payload: bytes) -> bytes:
    return (
        HEADER.pack(
            MAGIC,
            WIRE_HEADER_VERSION,
            kind,
            CHANNEL_CONTROL,
            0,
            sequence,
            len(payload),
        )
        + payload
    )


def send_result(
    sock: socket.socket,
    kind: int,
    word: int,
) -> None:
    sock.sendall(encode_frame(kind, 1, ONE_WORD.pack(word)))


def receive_hello(sock: socket.socket) -> tuple[int, int]:
    raw_header = read_exact(sock, HEADER_BYTES)
    magic, version, kind, channel, flags, sequence, payload_length = (
        HEADER.unpack(raw_header)
    )

    if (
        magic != MAGIC
        or version != WIRE_HEADER_VERSION
        or kind != FRAME_HELLO
        or channel != CHANNEL_CONTROL
        or flags != 0
        or sequence != 1
        or payload_length != HELLO.size
    ):
        raise ProofError(
            "malformed provisional Wire HELLO "
            f"magic={magic!r} version={version} kind={kind} "
            f"channel={channel} flags={flags} sequence={sequence} "
            f"payload_length={payload_length}"
        )

    payload = read_exact(sock, payload_length)
    return HELLO.unpack(payload)


def nonzero_session_id() -> int:
    while True:
        value = secrets.randbits(32)
        if value != 0:
            return value


def run_session(
    connection: socket.socket,
    peer: tuple[str, int],
    minimum_idle_seconds: float,
) -> int:
    connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    print(
        f"WIRE_PROOF_PROVISIONAL peer={peer[0]}:{peer[1]}",
        flush=True,
    )

    try:
        wire_version, product_version = receive_hello(connection)
    except (EOFError, ProofError) as exc:
        try:
            send_result(
                connection,
                FRAME_NOT_ACCEPTED,
                REJECT_MALFORMED,
            )
        except OSError:
            pass
        print(f"WIRE_PROOF_NOT_ACCEPTED reason=MALFORMED detail={exc}", flush=True)
        return 2

    print(
        "WIRE_PROOF_HELLO "
        f"wire_version={wire_version} "
        f"product_version={product_version}",
        flush=True,
    )

    if wire_version != WIRE_VERSION:
        send_result(
            connection,
            FRAME_NOT_ACCEPTED,
            REJECT_WIRE_VERSION,
        )
        print("WIRE_PROOF_NOT_ACCEPTED reason=WIRE_VERSION", flush=True)
        return 3

    if product_version != PRODUCT_VERSION:
        send_result(
            connection,
            FRAME_NOT_ACCEPTED,
            REJECT_PRODUCT_VERSION,
        )
        print("WIRE_PROOF_NOT_ACCEPTED reason=PRODUCT_VERSION", flush=True)
        return 4

    session_id = nonzero_session_id()
    send_result(connection, FRAME_ACCEPT, session_id)

    print(
        f"WIRE_SESSION=ACTIVE session_id={session_id}",
        flush=True,
    )
    print(
        "WIRE_RIDERS_STARTED=0 "
        "rfb=0 audio=0 mpeg=0 config=0 desktop=0",
        flush=True,
    )

    active_at = time.monotonic()

    connection.settimeout(max(30.0, minimum_idle_seconds + 10.0))

    try:
        unexpected = connection.recv(1)
    except socket.timeout:
        print("WIRE_PROOF_IDLE=FAIL reason=PEER_DID_NOT_CLOSE", flush=True)
        return 5

    elapsed = time.monotonic() - active_at

    if unexpected:
        print(
            "WIRE_PROOF_IDLE=FAIL "
            f"reason=UNEXPECTED_POST_ESTABLISH_BYTE value={unexpected.hex()} "
            f"elapsed={elapsed:.6f}",
            flush=True,
        )
        return 6

    if elapsed < minimum_idle_seconds:
        print(
            "WIRE_PROOF_IDLE=FAIL "
            f"reason=EARLY_CLOSE elapsed={elapsed:.6f} "
            f"minimum={minimum_idle_seconds:.6f}",
            flush=True,
        )
        return 7

    print(
        "WIRE_PROOF_IDLE=PASS "
        f"elapsed={elapsed:.6f} "
        "ordinary_post_establishment_bytes=0",
        flush=True,
    )
    print(
        f"WIRE_SESSION=INACTIVE former_session_id={session_id}",
        flush=True,
    )
    return 0


def self_test() -> int:
    hello = HELLO.pack(WIRE_VERSION, PRODUCT_VERSION)
    frame = encode_frame(FRAME_HELLO, 1, hello)

    assert len(frame) == HEADER_BYTES + HELLO.size

    (
        magic,
        header_version,
        kind,
        channel,
        flags,
        sequence,
        payload_length,
    ) = HEADER.unpack(frame[:HEADER_BYTES])

    assert magic == MAGIC
    assert header_version == WIRE_HEADER_VERSION
    assert kind == FRAME_HELLO
    assert channel == CHANNEL_CONTROL
    assert flags == 0
    assert sequence == 1
    assert payload_length == HELLO.size
    assert HELLO.unpack(frame[HEADER_BYTES:]) == (
        WIRE_VERSION,
        PRODUCT_VERSION,
    )

    accept = encode_frame(
        FRAME_ACCEPT,
        1,
        ONE_WORD.pack(0x12345678),
    )
    assert len(accept) == HEADER_BYTES + ONE_WORD.size

    print("WIRE_PROOF_PI_SERVER_SELF_TEST=PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--listen", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5902)
    parser.add_argument("--minimum-idle-seconds", type=float, default=9.0)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        return self_test()

    if args.minimum_idle_seconds <= 0:
        raise SystemExit("--minimum-idle-seconds must be positive")

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(1)

    print(
        f"WIRE_PROOF_LISTENING={args.listen}:{args.port}",
        flush=True,
    )

    connection, peer = listener.accept()
    listener.close()

    try:
        return run_session(
            connection,
            peer,
            args.minimum_idle_seconds,
        )
    finally:
        try:
            connection.close()
        except OSError:
            pass


if __name__ == "__main__":
    sys.exit(main())
