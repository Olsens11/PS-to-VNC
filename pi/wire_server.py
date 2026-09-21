#!/usr/bin/env python3
"""File synopsis:
Implements the maintained Raspberry Pi product PSTV Wire listener and provisional
Q4 establishment owner.

One long-lived server process owns the listening socket and, sequentially, the
accepted socket, all physical recv/frame parsing, and all physical send for one
Wire Session. TCP accept begins PROVISIONAL; only an exact HELLO followed by a
successfully sent Pi-owned ACCEPT creates ACTIVE. EOF or protocol failure fully
retires that connection before the persistent owner accepts another.

R8 deliberately supports no ordinary rider traffic. An ACTIVE session may stay
idle indefinitely, but any post-establishment application byte currently fails
that session closed. The server owns no RFB provider, AUDIO/MPEG/CONFIG rider,
MPEG producer, heartbeat, Application policy, or custom restart loop.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-WIRE-SERVER-ESTABLISHMENT-R8.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from enum import Enum
import socket
import sys

import wire_protocol as protocol

DEFAULT_LISTEN_ADDRESS = "192.168.50.1"
DEFAULT_LISTEN_PORT = 5902
LISTEN_BACKLOG = 1


class WireSessionState(Enum):
    INACTIVE = "INACTIVE"
    PROVISIONAL = "PROVISIONAL"
    ACTIVE = "ACTIVE"


class SessionIdExhausted(RuntimeError):
    """Fail closed rather than wrap a process-local Wire Session identity."""


@dataclass(frozen=True)
class WireSessionOutcome:
    accepted: bool
    session_id: int | None
    rejection_reason: int | None
    protocol_failed: bool
    next_receive_sequence: int
    next_send_sequence: int


class SessionIdAllocator:
    """Allocate monotonically increasing nonzero uint32 IDs for this process."""

    def __init__(self, next_session_id: int = 1) -> None:
        if (
            not isinstance(next_session_id, int)
            or next_session_id <= 0
            or next_session_id > protocol.UINT32_MAX
        ):
            raise ValueError("next_session_id must be a nonzero uint32")
        self._next_session_id = next_session_id
        self._exhausted = False

    def allocate(self) -> int:
        if self._exhausted:
            raise SessionIdExhausted("Wire Session ID space exhausted")

        session_id = self._next_session_id
        if session_id == protocol.UINT32_MAX:
            self._exhausted = True
        else:
            self._next_session_id += 1
        return session_id


def read_exact(connection: socket.socket, byte_count: int) -> bytes:
    chunks: list[bytes] = []
    remaining = byte_count

    while remaining:
        chunk = connection.recv(remaining)
        if not chunk:
            raise EOFError("peer closed during framed read")
        chunks.append(chunk)
        remaining -= len(chunk)

    return b"".join(chunks)


def compatibility_rejection_reason(
    wire_version: int,
    product_version: int,
) -> int | None:
    if wire_version != protocol.WIRE_VERSION:
        return protocol.REJECT_WIRE_VERSION
    if product_version != protocol.PRODUCT_ESTABLISHMENT_VERSION:
        return protocol.REJECT_PRODUCT_VERSION
    return None


class WireConnectionOwner:
    """Own one accepted physical connection until it is completely retired."""

    def __init__(
        self,
        connection: socket.socket,
        session_ids: SessionIdAllocator,
    ) -> None:
        self._connection = connection
        self._session_ids = session_ids
        self.state = WireSessionState.PROVISIONAL
        self.session_id: int | None = None
        self.next_receive_sequence = 1
        self.next_send_sequence = 1

    def _send_rejection(self, reason: int) -> None:
        self._connection.sendall(
            protocol.encode_not_accepted_frame(
                reason,
                sequence=self.next_send_sequence,
            )
        )
        self.next_send_sequence += 1

    def _read_provisional_hello(self) -> tuple[int, int]:
        raw_header = read_exact(self._connection, protocol.HEADER_BYTES)
        header = protocol.decode_header(raw_header)
        payload = read_exact(self._connection, header.payload_length)

        if (
            self.next_receive_sequence != 1
            or header.sequence != self.next_receive_sequence
            or not protocol.is_hello_header(header)
        ):
            raise protocol.WireProtocolError(
                "first application frame is not exact sequence-1 HELLO"
            )

        wire_version, product_version = protocol.decode_hello_payload(payload)
        self.next_receive_sequence = 2
        return wire_version, product_version

    def establish(self) -> WireSessionOutcome:
        try:
            wire_version, product_version = self._read_provisional_hello()
        except EOFError:
            return self._finish(
                accepted=False,
                rejection_reason=None,
                protocol_failed=True,
            )
        except protocol.WireProtocolError:
            try:
                self._send_rejection(protocol.REJECT_MALFORMED)
            except OSError:
                pass
            return self._finish(
                accepted=False,
                rejection_reason=protocol.REJECT_MALFORMED,
                protocol_failed=True,
            )

        rejection = compatibility_rejection_reason(
            wire_version,
            product_version,
        )
        if rejection is not None:
            try:
                self._send_rejection(rejection)
            except OSError:
                return self._finish(
                    accepted=False,
                    rejection_reason=rejection,
                    protocol_failed=True,
                )
            return self._finish(
                accepted=False,
                rejection_reason=rejection,
                protocol_failed=False,
            )

        session_id = self._session_ids.allocate()
        try:
            self._connection.sendall(
                protocol.encode_accept_frame(
                    session_id,
                    sequence=self.next_send_sequence,
                )
            )
        except OSError:
            return self._finish(
                accepted=False,
                rejection_reason=None,
                protocol_failed=True,
            )

        self.next_send_sequence = 2
        self.session_id = session_id
        self.state = WireSessionState.ACTIVE

        return WireSessionOutcome(
            accepted=True,
            session_id=session_id,
            rejection_reason=None,
            protocol_failed=False,
            next_receive_sequence=self.next_receive_sequence,
            next_send_sequence=self.next_send_sequence,
        )

    def wait_until_session_end(self) -> WireSessionOutcome:
        if self.state is not WireSessionState.ACTIVE or self.session_id is None:
            raise RuntimeError("Wire Session is not ACTIVE")

        try:
            unsupported = self._connection.recv(1)
        except OSError:
            unsupported = b""

        if unsupported:
            return self._finish(
                accepted=True,
                rejection_reason=None,
                protocol_failed=True,
            )

        return self._finish(
            accepted=True,
            rejection_reason=None,
            protocol_failed=False,
        )

    def _finish(
        self,
        *,
        accepted: bool,
        rejection_reason: int | None,
        protocol_failed: bool,
    ) -> WireSessionOutcome:
        session_id = self.session_id
        self.state = WireSessionState.INACTIVE
        return WireSessionOutcome(
            accepted=accepted,
            session_id=session_id,
            rejection_reason=rejection_reason,
            protocol_failed=protocol_failed,
            next_receive_sequence=self.next_receive_sequence,
            next_send_sequence=self.next_send_sequence,
        )


class WireServer:
    """Own the persistent listener and all sequential physical Wire I/O."""

    def __init__(
        self,
        listen_address: str = DEFAULT_LISTEN_ADDRESS,
        port: int = DEFAULT_LISTEN_PORT,
        session_ids: SessionIdAllocator | None = None,
    ) -> None:
        if not listen_address:
            raise ValueError("listen_address must be non-empty")
        if not isinstance(port, int) or port <= 0 or port > 65535:
            raise ValueError("port must be in 1..65535")
        self.listen_address = listen_address
        self.port = port
        self.session_ids = session_ids or SessionIdAllocator()

    def serve_connection(self, connection: socket.socket) -> WireSessionOutcome:
        owner = WireConnectionOwner(connection, self.session_ids)
        try:
            outcome = owner.establish()
            if outcome.accepted:
                outcome = owner.wait_until_session_end()
            return outcome
        finally:
            try:
                connection.close()
            except OSError:
                pass

    def serve_forever(self) -> None:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
            listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            listener.bind((self.listen_address, self.port))
            listener.listen(LISTEN_BACKLOG)

            while True:
                connection, _peer = listener.accept()
                self.serve_connection(connection)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="PS-to-VNC product Wire server"
    )
    parser.add_argument("--listen", default=DEFAULT_LISTEN_ADDRESS)
    parser.add_argument("--port", type=int, default=DEFAULT_LISTEN_PORT)
    args = parser.parse_args()

    server = WireServer(args.listen, args.port)
    server.serve_forever()
    return 0


if __name__ == "__main__":
    sys.exit(main())
