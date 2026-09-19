#!/usr/bin/env python3
"""
Wire Proof 4 foreground development server.

The run combines two bounded hardware discriminators:

1. AUDIO makes real logical-rider progress while the RFB Relay is completely
   full and its domain is not consuming.

2. A deterministic PCM service failure occurs above Transport. After that local
   failure, AUDIO is filled to its bounded capacity with no consumer, while RFB
   continues through fresh inbound and outbound traffic on the same Wire.

The proof uses no malformed Transport frames and deliberately does not test a
Transport/channel corruption failure.
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

RFB_CAPACITY = 32
AUDIO_CAPACITY = 16
MAX_DATA = 16

RFB_FIRST = bytes(range(0x10, 0x30))
RFB_SECOND = bytes(range(0x40, 0x60))
AUDIO_TRIGGER = bytes((0xA1, 0xA2, 0xA3, 0xA4))
AUDIO_AFTER_FAILURE = bytes(range(0xB0, 0xC0))
RFB_OUTBOUND = bytes(range(0x80, 0xA5))


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
        raise ProofError(f"bad version {version}")
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


def decode_credit(payload: bytes) -> int:
    if len(payload) != 4:
        raise ProofError(f"credit payload length {len(payload)}")
    return struct.unpack("!I", payload)[0]


def expect_credit(
    connection: socket.socket,
    sequence: int,
    channel: int,
    amount: int,
) -> None:
    kind, got_channel, flags, payload = recv_frame(connection, sequence)

    if kind != FRAME_CREDIT or got_channel != channel or flags != 0:
        raise ProofError(
            "expected CREDIT "
            f"seq={sequence} channel={channel}; "
            f"got kind={kind} channel={got_channel} flags={flags}"
        )

    got_amount = decode_credit(payload)

    if got_amount != amount:
        raise ProofError(
            f"credit amount got={got_amount} expected={amount}"
        )


def expect_heartbeat(
    connection: socket.socket,
    sequence: int,
) -> None:
    kind, channel, flags, payload = recv_frame(connection, sequence)

    if (
        kind != FRAME_HEARTBEAT
        or channel != CHANNEL_CONTROL
        or flags != 0
        or payload
    ):
        raise ProofError(
            "expected proof heartbeat "
            f"seq={sequence}; got kind={kind} channel={channel} "
            f"flags={flags} bytes={len(payload)}"
        )


def receive_hello(connection: socket.socket) -> tuple[int, int]:
    kind, channel, flags, payload = recv_frame(connection, 1)

    if (
        kind != FRAME_HELLO
        or channel != CHANNEL_CONTROL
        or flags != 0
        or len(payload) != 8
    ):
        raise ProofError("first PS2 frame was not exact HELLO")

    wire_version, product_version = struct.unpack("!II", payload)

    if wire_version != WIRE_VERSION or product_version != PRODUCT_VERSION:
        raise ProofError(
            "unexpected HELLO identity "
            f"wire={wire_version} product={product_version}"
        )

    return wire_version, product_version


def new_session_id() -> int:
    while True:
        value = secrets.randbits(32)
        if value:
            return value


def self_test() -> int:
    assert HEADER.size == 16
    assert len(RFB_FIRST) == RFB_CAPACITY
    assert len(RFB_SECOND) == RFB_CAPACITY
    assert len(AUDIO_TRIGGER) == 4
    assert len(AUDIO_AFTER_FAILURE) == AUDIO_CAPACITY
    assert len(RFB_OUTBOUND) == 37

    assert [
        len(RFB_OUTBOUND[0:16]),
        len(RFB_OUTBOUND[16:32]),
        len(RFB_OUTBOUND[32:]),
    ] == [16, 16, 5]

    print("WIRE_PROOF4_SERVER_SELF_TEST=PASS")
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
        f"WIRE_PROOF4_LISTENING={args.listen}:{args.port}",
        flush=True,
    )

    connection: socket.socket | None = None

    try:
        connection, peer = listener.accept()
        listener.close()

        connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        connection.settimeout(60.0)

        print(
            f"WIRE_PROOF4_PROVISIONAL peer={peer[0]}:{peer[1]}",
            flush=True,
        )

        wire_version, product_version = receive_hello(connection)

        print(
            "WIRE_PROOF4_HELLO "
            f"wire_version={wire_version} "
            f"product_version={product_version} "
            "ps2_sequence=1",
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

        # Real Transport runtime start_receiver() emits these in this order.
        expect_credit(
            connection,
            2,
            CHANNEL_RFB,
            RFB_CAPACITY,
        )
        expect_credit(
            connection,
            3,
            CHANNEL_AUDIO,
            AUDIO_CAPACITY,
        )

        print(
            "INITIAL_CREDITS=PASS "
            "rfb=32 audio=16 ps2_sequences=2,3",
            flush=True,
        )

        # Fill RFB completely and deliberately leave its domain stalled.
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
            "RFB_RELAY_FILLED "
            "bytes=32 fragments=16+16 pi_sequences=2,3",
            flush=True,
        )

        # AUDIO must still progress through the same sole receiver.
        send_frame(
            connection,
            4,
            FRAME_DATA,
            CHANNEL_AUDIO,
            AUDIO_TRIGGER,
        )

        # Playback dequeues the 4 AUDIO bytes before injected service failure.
        expect_credit(
            connection,
            4,
            CHANNEL_AUDIO,
            len(AUDIO_TRIGGER),
        )

        print(
            "AUDIO_PROGRESS_WHILE_RFB_FULL=PASS "
            "bytes=4 returned_credit=4 ps2_sequence=4",
            flush=True,
        )

        # Use restored AUDIO credit to fill the Relay after the domain fails.
        send_frame(
            connection,
            5,
            FRAME_DATA,
            CHANNEL_AUDIO,
            AUDIO_AFTER_FAILURE,
        )

        print(
            "AUDIO_RELAY_REFILLED_AFTER_PLAYBACK_READ "
            "bytes=16 pi_sequence=5",
            flush=True,
        )

        # PS2 sends this only after:
        #   - playback returned SERVICE_PLAY_FAILED,
        #   - playback cleanup succeeded,
        #   - Transport is still healthy,
        #   - AUDIO queue is observed completely full.
        expect_heartbeat(connection, 5)

        print(
            "AUDIO_LOCAL_FAILURE_AND_FULL_RELAY_MARKER=PASS "
            "ps2_sequence=5",
            flush=True,
        )

        # Initial stalled RFB bytes may now cross the RFB domain boundary.
        expect_credit(
            connection,
            6,
            CHANNEL_RFB,
            RFB_CAPACITY,
        )

        print(
            "RFB_FIRST_DRAIN_AFTER_AUDIO_FAILURE=PASS "
            "credit=32 ps2_sequence=6",
            flush=True,
        )

        # Fresh RFB work arrives while AUDIO is failed and its Relay is full.
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

        expect_credit(
            connection,
            7,
            CHANNEL_RFB,
            RFB_CAPACITY,
        )

        print(
            "RFB_PROGRESS_WHILE_AUDIO_FAILED_AND_FULL=PASS "
            "bytes=32 pi_sequences=6,7 ps2_credit_sequence=7",
            flush=True,
        )

        # Outbound RFB must still work on the same healthy Transport.
        outbound_parts: list[bytes] = []

        for expected_sequence, expected_length in (
            (8, 16),
            (9, 16),
            (10, 5),
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
                    f"seq={expected_sequence} "
                    f"kind={kind} channel={channel} "
                    f"flags={flags} length={len(payload)}"
                )

            outbound_parts.append(payload)

        if b"".join(outbound_parts) != RFB_OUTBOUND:
            raise ProofError("outbound RFB bytes changed")

        print(
            "RFB_OUTBOUND_AFTER_AUDIO_FAILURE=PASS "
            "bytes=37 fragments=16+16+5 ps2_sequences=8,9,10",
            flush=True,
        )

        # Normal runtime shutdown should produce EOF, not an application crash.
        trailing = connection.recv(1)

        if trailing:
            raise ProofError(
                f"unexpected trailing byte after shutdown: {trailing.hex()}"
            )

        print(
            f"WIRE_SESSION=INACTIVE former_session_id={session_id}",
            flush=True,
        )
        print("WIRE_PROOF4_RIDER_ISOLATION=PASS", flush=True)
        return 0

    except (EOFError, OSError, ProofError) as exc:
        print(
            f"WIRE_PROOF4_RIDER_ISOLATION=FAIL detail={exc}",
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
