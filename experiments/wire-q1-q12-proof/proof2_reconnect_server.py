#!/usr/bin/env python3
"""
Wire Q9/Q12 Proof 2 foreground development server.

Accept Session A, establish it normally, deliberately terminate that physical
connection while retaining the listening socket, then accept a fresh
provisional connection and establish Session B with a different authoritative
session ID.

This is proof apparatus only. It does not implement the final product daemon,
rider orchestration, resumption, or production recovery policy.
"""

from __future__ import annotations

import argparse
import socket
import sys
import time

import proof1_wire_server as base


class ReconnectProofError(RuntimeError):
    pass


def fresh_session_id(excluded: int) -> int:
    """Return a non-zero authoritative ID different from excluded."""

    while True:
        value = base.nonzero_session_id()
        if value != excluded:
            return value


def establish(
    listener: socket.socket,
    label: str,
    excluded_session_id: int,
) -> tuple[socket.socket, int, tuple[str, int]]:
    connection, peer = listener.accept()
    connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    print(
        f"WIRE_PROOF_{label}_PROVISIONAL peer={peer[0]}:{peer[1]}",
        flush=True,
    )

    try:
        wire_version, product_version = base.receive_hello(connection)
    except (EOFError, base.ProofError) as exc:
        connection.close()
        raise ReconnectProofError(
            f"{label} malformed HELLO: {exc}"
        ) from exc

    print(
        f"WIRE_PROOF_{label}_HELLO "
        f"wire_version={wire_version} "
        f"product_version={product_version}",
        flush=True,
    )

    rejection_reason = base.compatibility_rejection_reason(
        wire_version,
        product_version,
        base.WIRE_VERSION,
        base.PRODUCT_VERSION,
    )

    if rejection_reason != 0:
        base.send_result(
            connection,
            base.FRAME_NOT_ACCEPTED,
            rejection_reason,
        )
        connection.close()
        raise ReconnectProofError(
            f"{label} unexpectedly incompatible reason={rejection_reason}"
        )

    session_id = fresh_session_id(excluded_session_id)

    base.send_result(
        connection,
        base.FRAME_ACCEPT,
        session_id,
    )

    print(
        f"WIRE_SESSION_{label}=ACTIVE session_id={session_id}",
        flush=True,
    )
    print(
        f"WIRE_RIDERS_{label}=0 "
        "rfb=0 audio=0 mpeg=0 config=0 desktop=0",
        flush=True,
    )

    return connection, session_id, peer


def self_test() -> int:
    for excluded in (1, 0x12345678, 0xFFFFFFFF):
        candidate = fresh_session_id(excluded)
        assert candidate != 0
        assert candidate != excluded

    print("WIRE_PROOF2_RECONNECT_SERVER_SELF_TEST=PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--listen", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5902)
    parser.add_argument(
        "--session-a-hold-seconds",
        type=float,
        default=3.0,
    )
    parser.add_argument(
        "--session-b-minimum-seconds",
        type=float,
        default=2.5,
    )
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        return self_test()

    if args.session_a_hold_seconds <= 0:
        raise SystemExit("--session-a-hold-seconds must be positive")

    if args.session_b_minimum_seconds <= 0:
        raise SystemExit("--session-b-minimum-seconds must be positive")

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.listen, args.port))
    listener.listen(2)

    print(
        f"WIRE_PROOF2_LISTENING={args.listen}:{args.port}",
        flush=True,
    )

    connection_a: socket.socket | None = None
    connection_b: socket.socket | None = None

    try:
        connection_a, session_a, peer_a = establish(
            listener,
            "A",
            0,
        )

        time.sleep(args.session_a_hold_seconds)

        print(
            "WIRE_PROOF_FORCED_LOSS "
            f"session=A session_id={session_a} "
            f"after_seconds={args.session_a_hold_seconds:.6f}",
            flush=True,
        )

        try:
            connection_a.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass

        connection_a.close()
        connection_a = None

        print(
            "WIRE_SESSION_A=INACTIVE "
            f"former_session_id={session_a} "
            "reason=PI_FORCED_CLOSE",
            flush=True,
        )

        print("WIRE_PROOF_WAITING_FOR_RECONNECT=YES", flush=True)

        connection_b, session_b, peer_b = establish(
            listener,
            "B",
            session_a,
        )

        if session_b == session_a:
            raise ReconnectProofError("Session B reused Session A ID")

        print(
            "WIRE_SESSION_IDS_DISTINCT=YES "
            f"session_a={session_a} session_b={session_b}",
            flush=True,
        )

        print(
            "WIRE_RECONNECT_PEER "
            f"session_a={peer_a[0]}:{peer_a[1]} "
            f"session_b={peer_b[0]}:{peer_b[1]}",
            flush=True,
        )

        active_b_at = time.monotonic()

        connection_b.settimeout(
            max(30.0, args.session_b_minimum_seconds + 10.0)
        )

        try:
            unexpected = connection_b.recv(1)
        except socket.timeout:
            print(
                "WIRE_PROOF_SESSION_B=FAIL "
                "reason=PEER_DID_NOT_CLOSE",
                flush=True,
            )
            return 5

        elapsed_b = time.monotonic() - active_b_at

        if unexpected:
            print(
                "WIRE_PROOF_SESSION_B=FAIL "
                f"reason=UNEXPECTED_DATA value={unexpected.hex()} "
                f"elapsed={elapsed_b:.6f}",
                flush=True,
            )
            return 6

        if elapsed_b < args.session_b_minimum_seconds:
            print(
                "WIRE_PROOF_SESSION_B=FAIL "
                f"reason=EARLY_CLOSE elapsed={elapsed_b:.6f} "
                f"minimum={args.session_b_minimum_seconds:.6f}",
                flush=True,
            )
            return 7

        print(
            "WIRE_PROOF_SESSION_B=PASS "
            f"elapsed={elapsed_b:.6f} "
            "ordinary_post_establishment_bytes=0",
            flush=True,
        )

        print(
            "WIRE_SESSION_B=INACTIVE "
            f"former_session_id={session_b} "
            "reason=CONSOLE_CLOSE",
            flush=True,
        )

        print("WIRE_PROOF2_RECONNECT=PASS", flush=True)
        return 0

    except ReconnectProofError as exc:
        print(f"WIRE_PROOF2_RECONNECT=FAIL detail={exc}", flush=True)
        return 2

    finally:
        if connection_a is not None:
            try:
                connection_a.close()
            except OSError:
                pass

        if connection_b is not None:
            try:
                connection_b.close()
            except OSError:
                pass

        listener.close()


if __name__ == "__main__":
    sys.exit(main())
