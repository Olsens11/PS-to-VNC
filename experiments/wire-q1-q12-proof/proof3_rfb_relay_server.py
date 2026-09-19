#!/usr/bin/env python3
"""
Wire Q3 Proof 3 foreground development server.

This apparatus verifies one continuous PSTV sequence space from Q4
establishment into a bounded channel-1 Relay exercise.

It deliberately does not run a VNC provider. Channel-1 payload is opaque test
data so the hardware proof isolates Relay ownership, bounded capacity, credit
timing, sibling/control progress, and outbound fragmentation.
"""

from __future__ import annotations

import argparse
import select
import secrets
import socket
import struct
import sys

MAGIC = b"PSTV"
WIRE_VERSION = 1
PRODUCT_VERSION = 1
HEADER = struct.Struct("!4sBBBBII")

FRAME_HELLO = 1
FRAME_DATA = 3
FRAME_CREDIT = 4
FRAME_HEARTBEAT = 6
FRAME_ACCEPT = 12

CHANNEL_CONTROL = 0
CHANNEL_RFB = 1

INBOUND = bytes(range(0x10, 0x30))
OUTBOUND = bytes(range(0x80, 0xA5))

QUEUE_CAPACITY = 32
MAX_DATA_PAYLOAD = 16


class ProofError(RuntimeError):
    pass


def recv_exact(connection: socket.socket, count: int) -> bytes:
    data = bytearray()

    while len(data) < count:
        chunk = connection.recv(count - len(data))
        if not chunk:
            raise EOFError(
                f"peer closed with {count - len(data)} bytes still required"
            )
        data.extend(chunk)

    return bytes(data)


def recv_frame(
    connection: socket.socket,
    expected_sequence: int,
) -> tuple[int, int, int, bytes]:
    raw = recv_exact(connection, HEADER.size)

    magic, version, kind, channel, flags, sequence, length = HEADER.unpack(raw)

    if magic != MAGIC:
        raise ProofError(f"bad magic {magic!r}")
    if version != WIRE_VERSION:
        raise ProofError(f"bad frame version {version}")
    if sequence != expected_sequence:
        raise ProofError(
            f"bad sequence got={sequence} expected={expected_sequence}"
        )
    if length > 65536:
        raise ProofError(f"unreasonable payload length {length}")

    payload = recv_exact(connection, length)

    return kind, channel, flags, payload


def send_frame(
    connection: socket.socket,
    sequence: int,
    kind: int,
    channel: int,
    payload: bytes = b"",
) -> None:
    header = HEADER.pack(
        MAGIC,
        WIRE_VERSION,
        kind,
        channel,
        0,
        sequence,
        len(payload),
    )

    connection.sendall(header)
    if payload:
        connection.sendall(payload)


def decode_u32(payload: bytes) -> int:
    if len(payload) != 4:
        raise ProofError(f"expected four-byte integer, got {len(payload)}")
    return struct.unpack("!I", payload)[0]


def receive_hello(connection: socket.socket) -> tuple[int, int]:
    kind, channel, flags, payload = recv_frame(connection, 1)

    if (
        kind != FRAME_HELLO
        or channel != CHANNEL_CONTROL
        or flags != 0
        or len(payload) != 8
    ):
        raise ProofError(
            "first application frame was not exact establishment HELLO"
        )

    wire_version, product_version = struct.unpack("!II", payload)

    if wire_version != WIRE_VERSION or product_version != PRODUCT_VERSION:
        raise ProofError(
            "unexpected compatibility identity "
            f"wire={wire_version} product={product_version}"
        )

    return wire_version, product_version


def nonzero_session_id() -> int:
    while True:
        value = secrets.randbits(32)
        if value != 0:
            return value


def self_test() -> int:
    assert HEADER.size == 16
    assert len(INBOUND) == 32
    assert len(OUTBOUND) == 37

    fragments = [
        OUTBOUND[0:16],
        OUTBOUND[16:32],
        OUTBOUND[32:37],
    ]

    assert [len(item) for item in fragments] == [16, 16, 5]
    assert b"".join(fragments) == OUTBOUND

    packed = HEADER.pack(
        MAGIC,
        WIRE_VERSION,
        FRAME_DATA,
        CHANNEL_RFB,
        0,
        7,
        16,
    )

    assert HEADER.unpack(packed) == (
        MAGIC,
        WIRE_VERSION,
        FRAME_DATA,
        CHANNEL_RFB,
        0,
        7,
        16,
    )

    print("WIRE_PROOF3_RFB_RELAY_SERVER_SELF_TEST=PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--listen", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5902)
    parser.add_argument(
        "--no-early-credit-seconds",
        type=float,
        default=2.0,
    )
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        return self_test()

    if args.no_early_credit_seconds <= 0:
        raise SystemExit("--no-early-credit-seconds must be positive")

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(1)

    print(
        f"WIRE_PROOF3_LISTENING={args.listen}:{args.port}",
        flush=True,
    )

    connection: socket.socket | None = None

    try:
        connection, peer = listener.accept()
        listener.close()

        connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        connection.settimeout(10.0)

        print(
            f"WIRE_PROOF3_PROVISIONAL peer={peer[0]}:{peer[1]}",
            flush=True,
        )

        wire_version, product_version = receive_hello(connection)

        print(
            "WIRE_PROOF3_HELLO "
            f"wire_version={wire_version} "
            f"product_version={product_version} "
            "ps2_sequence=1",
            flush=True,
        )

        session_id = nonzero_session_id()

        send_frame(
            connection,
            1,
            FRAME_ACCEPT,
            CHANNEL_CONTROL,
            struct.pack("!I", session_id),
        )

        print(
            f"WIRE_SESSION=ACTIVE session_id={session_id}",
            flush=True,
        )

        # PS2 sequence 2: initial RFB credit after HELLO sequence 1.
        kind, channel, flags, payload = recv_frame(connection, 2)

        if (
            kind != FRAME_CREDIT
            or channel != CHANNEL_RFB
            or flags != 0
        ):
            raise ProofError("expected initial channel-1 CREDIT at sequence 2")

        initial_credit = decode_u32(payload)

        if initial_credit != QUEUE_CAPACITY:
            raise ProofError(
                f"initial credit got={initial_credit} "
                f"expected={QUEUE_CAPACITY}"
            )

        print(
            "RFB_INITIAL_CREDIT=PASS "
            f"amount={initial_credit} ps2_sequence=2",
            flush=True,
        )

        # Pi sequences 2 and 3: exactly fill the advertised window.
        send_frame(
            connection,
            2,
            FRAME_DATA,
            CHANNEL_RFB,
            INBOUND[:16],
        )
        send_frame(
            connection,
            3,
            FRAME_DATA,
            CHANNEL_RFB,
            INBOUND[16:],
        )

        print(
            "RFB_WINDOW_FILLED "
            "bytes=32 fragments=16+16 "
            "pi_sequences=2,3",
            flush=True,
        )

        # Pi sequence 4: CONTROL progress while channel-1 capacity is exhausted.
        send_frame(
            connection,
            4,
            FRAME_HEARTBEAT,
            CHANNEL_CONTROL,
        )

        # PS2 sequence 3 must be heartbeat reply, not returned RFB credit.
        kind, channel, flags, payload = recv_frame(connection, 3)

        if (
            kind != FRAME_HEARTBEAT
            or channel != CHANNEL_CONTROL
            or flags != 0
            or payload
        ):
            raise ProofError(
                "expected heartbeat reply before any returned RFB credit"
            )

        print(
            "WIRE_PROGRESS_WHILE_RFB_STALLED=PASS "
            "ps2_sequence=3",
            flush=True,
        )

        readable, _, _ = select.select(
            [connection],
            [],
            [],
            args.no_early_credit_seconds,
        )

        if readable:
            kind, channel, flags, payload = recv_frame(connection, 4)
            raise ProofError(
                "frame arrived before domain acceptance "
                f"kind={kind} channel={channel} "
                f"flags={flags} bytes={len(payload)}"
            )

        print(
            "RFB_NO_EARLY_RETURN_CREDIT=PASS "
            f"window_seconds={args.no_early_credit_seconds:.3f}",
            flush=True,
        )

        # PS2 sequence 4: credit returned only after domain accepted 32 bytes.
        kind, channel, flags, payload = recv_frame(connection, 4)

        if (
            kind != FRAME_CREDIT
            or channel != CHANNEL_RFB
            or flags != 0
        ):
            raise ProofError(
                "expected returned channel-1 CREDIT after domain acceptance"
            )

        returned_credit = decode_u32(payload)

        if returned_credit != QUEUE_CAPACITY:
            raise ProofError(
                f"returned credit got={returned_credit} "
                f"expected={QUEUE_CAPACITY}"
            )

        print(
            "RFB_RETURN_CREDIT_AFTER_DOMAIN_ACCEPT=PASS "
            f"amount={returned_credit} ps2_sequence=4",
            flush=True,
        )

        # PS2 sequences 5,6,7: outbound 37 bytes fragmented 16+16+5.
        outbound_parts: list[bytes] = []

        for expected_sequence, expected_length in (
            (5, 16),
            (6, 16),
            (7, 5),
        ):
            kind, channel, flags, payload = recv_frame(
                connection,
                expected_sequence,
            )

            if (
                kind != FRAME_DATA
                or channel != CHANNEL_RFB
                or flags != 0
                or len(payload) != expected_length
            ):
                raise ProofError(
                    "bad outbound RFB fragment "
                    f"sequence={expected_sequence} "
                    f"kind={kind} channel={channel} "
                    f"flags={flags} length={len(payload)} "
                    f"expected_length={expected_length}"
                )

            outbound_parts.append(payload)

        outbound = b"".join(outbound_parts)

        if outbound != OUTBOUND:
            raise ProofError(
                "outbound opaque bytes did not survive Relay unchanged"
            )

        print(
            "RFB_OUTBOUND_REASSEMBLY=PASS "
            "bytes=37 fragments=16+16+5 "
            "ps2_sequences=5,6,7",
            flush=True,
        )

        # Pi sequence 5 proves the same Wire remains live after Relay service.
        send_frame(
            connection,
            5,
            FRAME_HEARTBEAT,
            CHANNEL_CONTROL,
        )

        print(
            "WIRE_FINAL_HEARTBEAT_SENT pi_sequence=5",
            flush=True,
        )

        connection.settimeout(10.0)
        trailing = connection.recv(1)

        if trailing:
            raise ProofError(
                f"unexpected trailing byte after final heartbeat: {trailing.hex()}"
            )

        print(
            "WIRE_SESSION=INACTIVE "
            f"former_session_id={session_id}",
            flush=True,
        )
        print("WIRE_PROOF3_RFB_RELAY=PASS", flush=True)
        return 0

    except (EOFError, OSError, ProofError) as exc:
        print(
            f"WIRE_PROOF3_RFB_RELAY=FAIL detail={exc}",
            flush=True,
        )
        return 2

    finally:
        if connection is not None:
            try:
                connection.close()
            except OSError:
                pass

        try:
            listener.close()
        except OSError:
            pass


if __name__ == "__main__":
    sys.exit(main())
