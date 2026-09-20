#!/usr/bin/env python3
"""
Q12 stale-access hardware proof server.

The server creates proof-local Wire Sessions A and B around the real PS2
product Transport bridge.  A must emit one valid RFB marker before being
forcibly closed.  B then receives one known RFB payload.  The PS2 is required
to reject stale A outbound and inbound operations locally, preserve B's payload,
return B credit, and emit exactly one fresh B acknowledgement.

Receiving BOK! therefore machine-proves that stale A work neither escaped on the
outbound path nor consumed the known B payload on the return path.
"""

from __future__ import annotations

import argparse
import socket
import struct
import sys

import proof1_wire_server as base

FRAME_DATA = 3
FRAME_CREDIT = 4
CHANNEL_RFB = 1

A_MARKER = b"AOK!"
STALE_MARKER = b"OLD!"
B_MARKER = b"BOK!"
B_PAYLOAD = bytes((0xB1, 0xB2, 0xB3, 0xB4))
CREDIT = struct.Struct(">I")


class Q12ProofError(RuntimeError):
    pass


def encode_frame(
    kind: int,
    channel: int,
    sequence: int,
    payload: bytes,
) -> bytes:
    return (
        base.HEADER.pack(
            base.MAGIC,
            base.WIRE_HEADER_VERSION,
            kind,
            channel,
            0,
            sequence,
            len(payload),
        )
        + payload
    )


def receive_frame(sock: socket.socket) -> tuple[int, int, int, bytes]:
    raw = base.read_exact(sock, base.HEADER_BYTES)
    magic, version, kind, channel, flags, sequence, length = base.HEADER.unpack(raw)

    if magic != base.MAGIC or version != base.WIRE_HEADER_VERSION or flags != 0:
        raise Q12ProofError(
            "invalid frame header "
            f"magic={magic!r} version={version} flags={flags}"
        )

    return kind, channel, sequence, base.read_exact(sock, length)


def fresh_session_id(excluded: int) -> int:
    while True:
        candidate = base.nonzero_session_id()
        if candidate != excluded:
            return candidate


def establish(
    listener: socket.socket,
    label: str,
    excluded: int,
) -> tuple[socket.socket, int]:
    connection, peer = listener.accept()
    connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    connection.settimeout(15.0)

    print(
        f"Q12_SESSION_{label}_PROVISIONAL peer={peer[0]}:{peer[1]}",
        flush=True,
    )

    wire_version, product_version = base.receive_hello(connection)
    if (wire_version, product_version) != (base.WIRE_VERSION, base.PRODUCT_VERSION):
        raise Q12ProofError(
            f"{label} incompatible "
            f"wire={wire_version} product={product_version}"
        )

    session_id = fresh_session_id(excluded)
    base.send_result(connection, base.FRAME_ACCEPT, session_id)

    print(
        f"Q12_SESSION_{label}=ACTIVE proof_session_id={session_id}",
        flush=True,
    )
    return connection, session_id


def expect_initial_credit(sock: socket.socket, label: str) -> None:
    kind, channel, sequence, payload = receive_frame(sock)
    if (
        kind != FRAME_CREDIT
        or channel != CHANNEL_RFB
        or sequence != 1
        or payload != CREDIT.pack(16)
    ):
        raise Q12ProofError(
            f"{label} initial credit mismatch "
            f"kind={kind} channel={channel} sequence={sequence} "
            f"payload={payload.hex()}"
        )

    print(f"Q12_{label}_INITIAL_RFB_CREDIT=PASS amount=16", flush=True)


def run(args: argparse.Namespace) -> int:
    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(2)

    print(f"Q12_PROOF_LISTENING={args.listen}:{args.port}", flush=True)

    connection_a: socket.socket | None = None
    connection_b: socket.socket | None = None

    try:
        connection_a, session_a = establish(listener, "A", 0)
        expect_initial_credit(connection_a, "A")

        kind, channel, sequence, payload = receive_frame(connection_a)
        if (
            kind != FRAME_DATA
            or channel != CHANNEL_RFB
            or sequence != 2
            or payload != A_MARKER
        ):
            raise Q12ProofError(
                "A valid-access marker mismatch "
                f"kind={kind} channel={channel} sequence={sequence} "
                f"payload={payload!r}"
            )

        print("Q12_ACCESS_A_VALID_WRITE=PASS", flush=True)
        print(
            f"Q12_FORCE_SESSION_A_LOSS proof_session_id={session_a}",
            flush=True,
        )

        try:
            connection_a.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        connection_a.close()
        connection_a = None

        print(
            f"Q12_SESSION_A=INACTIVE former_proof_session_id={session_a}",
            flush=True,
        )

        connection_b, session_b = establish(listener, "B", session_a)
        if session_b == session_a:
            raise Q12ProofError("proof Session B reused Session A identity")

        expect_initial_credit(connection_b, "B")

        connection_b.sendall(
            encode_frame(
                FRAME_DATA,
                CHANNEL_RFB,
                1,
                B_PAYLOAD,
            )
        )
        print(
            "Q12_B_INBOUND_SENT=YES payload=b1b2b3b4 sequence=1",
            flush=True,
        )

        # The next PS2 frame must be the credit generated only by the fresh B
        # read.  A stale outbound OLD! escaping through B would appear here and
        # fail before any B acknowledgement can be accepted.
        kind, channel, sequence, payload = receive_frame(connection_b)
        if (
            kind != FRAME_CREDIT
            or channel != CHANNEL_RFB
            or sequence != 2
            or payload != CREDIT.pack(4)
        ):
            if payload == STALE_MARKER:
                detail = "STALE_A_OUTBOUND_ESCAPED"
            else:
                detail = "UNEXPECTED_FIRST_B_FRAME"
            raise Q12ProofError(
                f"{detail} kind={kind} channel={channel} "
                f"sequence={sequence} payload={payload.hex()}"
            )

        print(
            "Q12_B_FRESH_READ_CREDIT=PASS amount=4 sequence=2",
            flush=True,
        )

        kind, channel, sequence, payload = receive_frame(connection_b)
        if (
            kind != FRAME_DATA
            or channel != CHANNEL_RFB
            or sequence != 3
            or payload != B_MARKER
        ):
            if payload == STALE_MARKER:
                detail = "STALE_A_OUTBOUND_ESCAPED"
            else:
                detail = "B_ACK_MISMATCH"
            raise Q12ProofError(
                f"{detail} kind={kind} channel={channel} "
                f"sequence={sequence} payload={payload!r}"
            )

        print("Q12_STALE_A_OUTBOUND_ABSENT=PASS", flush=True)
        print("Q12_STALE_A_RETURN_PATH_PRESERVED_B_PAYLOAD=PASS", flush=True)
        print("Q12_FRESH_B_ACK=PASS payload=BOK!", flush=True)

        try:
            connection_b.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        connection_b.close()
        connection_b = None

        print(
            f"Q12_SESSION_B=INACTIVE former_proof_session_id={session_b}",
            flush=True,
        )
        print("Q12_STALE_ACCESS_HARDWARE_PROOF=PASS", flush=True)
        return 0

    except (EOFError, OSError, base.ProofError, Q12ProofError) as exc:
        print(f"Q12_STALE_ACCESS_HARDWARE_PROOF=FAIL detail={exc}", flush=True)
        return 2

    finally:
        for connection in (connection_a, connection_b):
            if connection is not None:
                try:
                    connection.close()
                except OSError:
                    pass
        listener.close()


def self_test() -> int:
    frame = encode_frame(FRAME_DATA, CHANNEL_RFB, 7, B_PAYLOAD)
    values = base.HEADER.unpack(frame[: base.HEADER_BYTES])

    assert values[0] == base.MAGIC
    assert values[1] == base.WIRE_HEADER_VERSION
    assert values[2] == FRAME_DATA
    assert values[3] == CHANNEL_RFB
    assert values[4] == 0
    assert values[5] == 7
    assert values[6] == len(B_PAYLOAD)
    assert frame[base.HEADER_BYTES :] == B_PAYLOAD
    assert CREDIT.unpack(CREDIT.pack(16))[0] == 16
    assert A_MARKER != STALE_MARKER != B_MARKER

    print("Q12_STALE_ACCESS_SERVER_SELF_TEST=PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--listen", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5902)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        return self_test()

    return run(args)


if __name__ == "__main__":
    sys.exit(main())
