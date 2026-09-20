#!/usr/bin/env python3
"""
Q12 in-flight Transport admission/drain hardware proof server.

Session A receives one-byte DATA while the PS2 has requested four bytes through
one product Transport RFB read.  Observing the resulting one-byte returned
credit proves that the read crossed Transport admission, consumed valid A data,
and is still incomplete when A is forcibly terminated.

The PS2 emits DRAINOK! through fresh Session B only after it has machine-checked
that:
  * the first A close returned WOULD_BLOCK while that call had not returned;
  * a replacement session-open probe was rejected while the call remained
    admitted;
  * the old A Transport call then returned terminally;
  * the second A close released the runtime;
  * real B subsequently opened and acquired fresh access.

Q4 Wire establishment remains proof-local.
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
A_PARTIAL = b"\xA5"
B_DRAIN_MARKER = b"DRAINOK!"
CREDIT = struct.Struct(">I")


class Q12InflightProofError(RuntimeError):
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


def receive_frame(
    sock: socket.socket,
) -> tuple[int, int, int, bytes]:
    raw = base.read_exact(sock, base.HEADER_BYTES)
    magic, version, kind, channel, flags, sequence, length = (
        base.HEADER.unpack(raw)
    )

    if (
        magic != base.MAGIC
        or version != base.WIRE_HEADER_VERSION
        or flags != 0
    ):
        raise Q12InflightProofError(
            "invalid frame header "
            f"magic={magic!r} version={version} flags={flags}"
        )

    return (
        kind,
        channel,
        sequence,
        base.read_exact(sock, length),
    )


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
    connection.setsockopt(
        socket.IPPROTO_TCP,
        socket.TCP_NODELAY,
        1,
    )
    connection.settimeout(20.0)

    print(
        f"Q12_HW2_SESSION_{label}_PROVISIONAL "
        f"peer={peer[0]}:{peer[1]}",
        flush=True,
    )

    wire_version, product_version = base.receive_hello(
        connection
    )

    if (
        wire_version,
        product_version,
    ) != (
        base.WIRE_VERSION,
        base.PRODUCT_VERSION,
    ):
        raise Q12InflightProofError(
            f"{label} incompatible "
            f"wire={wire_version} product={product_version}"
        )

    session_id = fresh_session_id(excluded)
    base.send_result(
        connection,
        base.FRAME_ACCEPT,
        session_id,
    )

    print(
        f"Q12_HW2_SESSION_{label}=ACTIVE "
        f"proof_session_id={session_id}",
        flush=True,
    )

    return connection, session_id


def expect_initial_credit(
    sock: socket.socket,
    label: str,
) -> None:
    kind, channel, sequence, payload = receive_frame(sock)

    if (
        kind != FRAME_CREDIT
        or channel != CHANNEL_RFB
        or sequence != 1
        or payload != CREDIT.pack(16)
    ):
        raise Q12InflightProofError(
            f"{label} initial credit mismatch "
            f"kind={kind} channel={channel} "
            f"sequence={sequence} payload={payload.hex()}"
        )

    print(
        f"Q12_HW2_{label}_INITIAL_RFB_CREDIT=PASS amount=16",
        flush=True,
    )


def run(args: argparse.Namespace) -> int:
    listener = socket.socket(
        socket.AF_INET,
        socket.SOCK_STREAM,
    )
    listener.setsockopt(
        socket.SOL_SOCKET,
        socket.SO_REUSEADDR,
        1,
    )
    listener.bind((args.listen, args.port))
    listener.listen(2)

    print(
        f"Q12_HW2_PROOF_LISTENING={args.listen}:{args.port}",
        flush=True,
    )

    connection_a: socket.socket | None = None
    connection_b: socket.socket | None = None

    try:
        connection_a, session_a = establish(
            listener,
            "A",
            0,
        )

        expect_initial_credit(connection_a, "A")

        kind, channel, sequence, payload = receive_frame(
            connection_a
        )

        if (
            kind != FRAME_DATA
            or channel != CHANNEL_RFB
            or sequence != 2
            or payload != A_MARKER
        ):
            raise Q12InflightProofError(
                "A valid marker mismatch "
                f"kind={kind} channel={channel} "
                f"sequence={sequence} payload={payload!r}"
            )

        print(
            "Q12_HW2_ACCESS_A_VALID_WRITE=PASS",
            flush=True,
        )

        connection_a.sendall(
            encode_frame(
                FRAME_DATA,
                CHANNEL_RFB,
                1,
                A_PARTIAL,
            )
        )

        print(
            "Q12_HW2_A_PARTIAL_SENT=YES "
            "payload=a5 bytes=1 requested_read_bytes=4",
            flush=True,
        )

        kind, channel, sequence, payload = receive_frame(
            connection_a
        )

        if (
            kind != FRAME_CREDIT
            or channel != CHANNEL_RFB
            or sequence != 3
            or payload != CREDIT.pack(1)
        ):
            raise Q12InflightProofError(
                "A partial-read credit mismatch "
                f"kind={kind} channel={channel} "
                f"sequence={sequence} payload={payload.hex()}"
            )

        print(
            "Q12_HW2_A_PARTIAL_READ_CREDIT=PASS "
            "amount=1 sequence=3",
            flush=True,
        )
        print(
            "Q12_HW2_A_CALL_EXTERNALLY_PROVEN_IN_FLIGHT=YES "
            "consumed=1 requested=4 outstanding=3",
            flush=True,
        )

        print(
            f"Q12_HW2_FORCE_SESSION_A_LOSS "
            f"proof_session_id={session_a}",
            flush=True,
        )

        try:
            connection_a.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass

        connection_a.close()
        connection_a = None

        print(
            f"Q12_HW2_SESSION_A=INACTIVE "
            f"former_proof_session_id={session_a}",
            flush=True,
        )

        connection_b, session_b = establish(
            listener,
            "B",
            session_a,
        )

        if session_b == session_a:
            raise Q12InflightProofError(
                "proof Session B reused Session A identity"
            )

        expect_initial_credit(connection_b, "B")

        kind, channel, sequence, payload = receive_frame(
            connection_b
        )

        if (
            kind != FRAME_DATA
            or channel != CHANNEL_RFB
            or sequence != 2
            or payload != B_DRAIN_MARKER
        ):
            raise Q12InflightProofError(
                "B drain marker mismatch "
                f"kind={kind} channel={channel} "
                f"sequence={sequence} payload={payload!r}"
            )

        print(
            "Q12_HW2_DRAIN_MARKER=PASS payload=DRAINOK!",
            flush=True,
        )
        print(
            "Q12_HW2_IN_FLIGHT_A_OPERATION_DRAIN=PASS",
            flush=True,
        )
        print(
            "Q12_HW2_B_OPEN_AFTER_A_CALL_RETURN=PASS",
            flush=True,
        )

        try:
            connection_b.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass

        connection_b.close()
        connection_b = None

        print(
            f"Q12_HW2_SESSION_B=INACTIVE "
            f"former_proof_session_id={session_b}",
            flush=True,
        )
        print(
            "Q12_INFLIGHT_DRAIN_HARDWARE_PROOF=PASS",
            flush=True,
        )
        return 0

    except (
        EOFError,
        OSError,
        base.ProofError,
        Q12InflightProofError,
    ) as exc:
        print(
            "Q12_INFLIGHT_DRAIN_HARDWARE_PROOF=FAIL "
            f"detail={exc}",
            flush=True,
        )
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
    frame = encode_frame(
        FRAME_DATA,
        CHANNEL_RFB,
        7,
        A_PARTIAL,
    )
    values = base.HEADER.unpack(
        frame[: base.HEADER_BYTES]
    )

    assert values[0] == base.MAGIC
    assert values[1] == base.WIRE_HEADER_VERSION
    assert values[2] == FRAME_DATA
    assert values[3] == CHANNEL_RFB
    assert values[4] == 0
    assert values[5] == 7
    assert values[6] == 1
    assert frame[base.HEADER_BYTES :] == A_PARTIAL

    assert CREDIT.unpack(CREDIT.pack(1))[0] == 1
    assert len(A_MARKER) == 4
    assert B_DRAIN_MARKER == b"DRAINOK!"

    print("Q12_INFLIGHT_DRAIN_SERVER_SELF_TEST=PASS")
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
