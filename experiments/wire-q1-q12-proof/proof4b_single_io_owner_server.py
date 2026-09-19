#!/usr/bin/env python3
"""
Pi peer for Proof 4B.

It reproduces the same physical frame sequence surrounding the Proof 4 stall,
but makes no claim about PCM failure or logical rider recovery. The only
question is whether one PS2 execution context can carry the complete bidirectional
transaction over one physical Wire socket.
"""

from __future__ import annotations

import argparse
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
CHANNEL_AUDIO = 2

RFB_FIRST = bytes(range(0x10, 0x30))
RFB_SECOND = bytes(range(0x40, 0x60))
AUDIO_TRIGGER = bytes((0xA1, 0xA2, 0xA3, 0xA4))
AUDIO_SECOND = bytes(range(0xB0, 0xC0))
RFB_OUTBOUND = bytes(range(0x80, 0xA5))


class ProofError(RuntimeError):
    pass


def recv_exact(connection: socket.socket, count: int) -> bytes:
    data = bytearray()

    while len(data) < count:
        chunk = connection.recv(count - len(data))

        if not chunk:
            raise EOFError(
                f"peer closed with {count - len(data)} bytes outstanding"
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
        raise ProofError(f"bad version {version}")
    if sequence != expected_sequence:
        raise ProofError(
            f"sequence got={sequence} expected={expected_sequence}"
        )
    if length > 65536:
        raise ProofError(f"payload too large {length}")

    return kind, channel, flags, recv_exact(connection, length)


def send_frame(
    connection: socket.socket,
    sequence: int,
    kind: int,
    channel: int,
    payload: bytes = b"",
) -> None:
    connection.sendall(
        HEADER.pack(
            MAGIC,
            WIRE_VERSION,
            kind,
            channel,
            0,
            sequence,
            len(payload),
        )
    )

    if payload:
        connection.sendall(payload)


def credit_value(payload: bytes) -> int:
    if len(payload) != 4:
        raise ProofError(f"bad credit size {len(payload)}")

    return struct.unpack("!I", payload)[0]


def expect_credit(
    connection: socket.socket,
    sequence: int,
    channel: int,
    amount: int,
) -> None:
    kind, got_channel, flags, payload = recv_frame(
        connection,
        sequence,
    )

    if (
        kind != FRAME_CREDIT
        or got_channel != channel
        or flags != 0
        or credit_value(payload) != amount
    ):
        raise ProofError(
            f"bad CREDIT seq={sequence} "
            f"kind={kind} channel={got_channel} "
            f"flags={flags} amount="
            f"{credit_value(payload) if len(payload) == 4 else 'invalid'}"
        )


def expect_heartbeat(
    connection: socket.socket,
    sequence: int,
) -> None:
    kind, channel, flags, payload = recv_frame(
        connection,
        sequence,
    )

    if (
        kind != FRAME_HEARTBEAT
        or channel != CHANNEL_CONTROL
        or flags != 0
        or payload
    ):
        raise ProofError(
            f"bad heartbeat seq={sequence}"
        )


def receive_hello(connection: socket.socket) -> None:
    kind, channel, flags, payload = recv_frame(connection, 1)

    if (
        kind != FRAME_HELLO
        or channel != CHANNEL_CONTROL
        or flags != 0
        or len(payload) != 8
    ):
        raise ProofError("first frame is not HELLO")

    wire_version, product_version = struct.unpack("!II", payload)

    if (
        wire_version != WIRE_VERSION
        or product_version != PRODUCT_VERSION
    ):
        raise ProofError(
            f"HELLO versions wire={wire_version} "
            f"product={product_version}"
        )


def new_session_id() -> int:
    while True:
        value = secrets.randbits(32)

        if value:
            return value


def self_test() -> int:
    assert HEADER.size == 16
    assert len(RFB_FIRST) == 32
    assert len(RFB_SECOND) == 32
    assert len(AUDIO_TRIGGER) == 4
    assert len(AUDIO_SECOND) == 16
    assert len(RFB_OUTBOUND) == 37
    assert [
        len(RFB_OUTBOUND[:16]),
        len(RFB_OUTBOUND[16:32]),
        len(RFB_OUTBOUND[32:]),
    ] == [16, 16, 5]

    print("WIRE_PROOF4B_SERVER_SELF_TEST=PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--listen", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5902)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        return self_test()

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(1)

    print(
        f"WIRE_PROOF4B_LISTENING={args.listen}:{args.port}",
        flush=True,
    )

    connection: socket.socket | None = None

    try:
        connection, peer = listener.accept()
        listener.close()

        connection.setsockopt(
            socket.IPPROTO_TCP,
            socket.TCP_NODELAY,
            1,
        )
        connection.settimeout(15.0)

        print(
            f"WIRE_PROOF4B_PROVISIONAL peer={peer[0]}:{peer[1]}",
            flush=True,
        )

        receive_hello(connection)

        print(
            "WIRE_PROOF4B_HELLO=PASS ps2_sequence=1",
            flush=True,
        )

        session_id = new_session_id()

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

        expect_credit(
            connection,
            2,
            CHANNEL_RFB,
            32,
        )
        expect_credit(
            connection,
            3,
            CHANNEL_AUDIO,
            16,
        )

        print(
            "INITIAL_CREDITS=PASS ps2_sequences=2,3",
            flush=True,
        )

        send_frame(
            connection,
            2,
            FRAME_DATA,
            CHANNEL_RFB,
            RFB_FIRST[:16],
        )
        send_frame(
            connection,
            3,
            FRAME_DATA,
            CHANNEL_RFB,
            RFB_FIRST[16:],
        )

        print(
            "FIRST_RFB_WINDOW_SENT=PASS pi_sequences=2,3",
            flush=True,
        )

        send_frame(
            connection,
            4,
            FRAME_DATA,
            CHANNEL_AUDIO,
            AUDIO_TRIGGER,
        )

        expect_credit(
            connection,
            4,
            CHANNEL_AUDIO,
            4,
        )

        print(
            "AUDIO_TRIGGER_ROUNDTRIP=PASS ps2_sequence=4",
            flush=True,
        )

        send_frame(
            connection,
            5,
            FRAME_DATA,
            CHANNEL_AUDIO,
            AUDIO_SECOND,
        )

        expect_heartbeat(connection, 5)

        print(
            "ORDER_MARKER=PASS ps2_sequence=5",
            flush=True,
        )

        expect_credit(
            connection,
            6,
            CHANNEL_RFB,
            32,
        )

        print(
            "FIRST_RFB_CREDIT=PASS ps2_sequence=6",
            flush=True,
        )

        send_frame(
            connection,
            6,
            FRAME_DATA,
            CHANNEL_RFB,
            RFB_SECOND[:16],
        )
        send_frame(
            connection,
            7,
            FRAME_DATA,
            CHANNEL_RFB,
            RFB_SECOND[16:],
        )

        print(
            "SECOND_RFB_WINDOW_SENT=PASS pi_sequences=6,7",
            flush=True,
        )

        # This is the exact wire point where P4-HW1 emitted only the
        # 16-byte PSTV header of CREDIT sequence 7.
        expect_credit(
            connection,
            7,
            CHANNEL_RFB,
            32,
        )

        print(
            "SEQ7_CREDIT_HEADER_AND_PAYLOAD=PASS "
            "amount=32 ps2_sequence=7",
            flush=True,
        )

        outbound = bytearray()

        for sequence, length in (
            (8, 16),
            (9, 16),
            (10, 5),
        ):
            kind, channel, flags, payload = recv_frame(
                connection,
                sequence,
            )

            if (
                kind != FRAME_DATA
                or channel != CHANNEL_RFB
                or flags != 0
                or len(payload) != length
            ):
                raise ProofError(
                    f"bad outbound frame seq={sequence} "
                    f"kind={kind} channel={channel} "
                    f"flags={flags} length={len(payload)}"
                )

            outbound.extend(payload)

        if bytes(outbound) != RFB_OUTBOUND:
            raise ProofError("outbound RFB bytes changed")

        print(
            "OUTBOUND_RFB=PASS "
            "bytes=37 fragments=16+16+5 "
            "ps2_sequences=8,9,10",
            flush=True,
        )

        connection.settimeout(5.0)

        trailing = connection.recv(1)

        if trailing:
            raise ProofError(
                f"unexpected trailing bytes {trailing.hex()}"
            )

        print(
            f"WIRE_SESSION=INACTIVE former_session_id={session_id}",
            flush=True,
        )

        print(
            "WIRE_PROOF4B_SINGLE_IO_OWNER=PASS",
            flush=True,
        )

        return 0

    except (EOFError, OSError, ProofError) as exc:
        print(
            f"WIRE_PROOF4B_SINGLE_IO_OWNER=FAIL detail={exc}",
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
