#!/usr/bin/env python3
"""File synopsis:
Implements the maintained Raspberry Pi product PSTV Wire listener and provisional
Q4 establishment owner.

One long-lived server process owns the listening socket and, sequentially, the
accepted socket, all physical recv/frame parsing, and all physical send for one
Wire Session. TCP accept begins PROVISIONAL; only an exact HELLO followed by a
successfully sent Pi-owned ACCEPT creates ACTIVE. EOF or protocol failure fully
retires that connection before the persistent owner accepts another.

R10 optionally composes one explicit session-scoped RFB provider relay while
preserving this object as the sole PS2-facing Wire recv/send owner. The relay
never receives from or sends to the Wire socket; this owner multiplexes Wire
readiness with provider readiness and moves exact channel-1 DATA/CREDIT between
them. With no explicit relay attachment the installed service remains the R8
establishment-only server.

The server still owns no concrete RFB provider selection, AUDIO/MPEG/CONFIG
rider, MPEG producer, heartbeat, Application policy, or custom restart loop.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-RFB-WIRE-RELAY-R10.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from enum import Enum
import select
import socket
import sys

import rfb_relay as rfb
import wire_protocol as protocol

DEFAULT_LISTEN_ADDRESS = "192.168.50.1"
DEFAULT_LISTEN_PORT = 5902
LISTEN_BACKLOG = 1


class WireSessionState(Enum):
    # INACTIVE: no accepted connection currently owns Wire Session authority.
    # PROVISIONAL: TCP exists, but HELLO/compatibility/ACCEPT are not complete.
    # ACTIVE: the Pi successfully sent ACCEPT for a nonzero session identity.
    INACTIVE = "INACTIVE"
    PROVISIONAL = "PROVISIONAL"
    ACTIVE = "ACTIVE"


class SessionIdExhausted(RuntimeError):
    """Fail closed rather than wrap a process-local Wire Session identity."""


class UnsafeProvisionalFraming(protocol.WireProtocolError):
    """Header bytes are not valid enough to send a Wire rejection safely."""


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

        # IDs are never reused within this long-lived process. Importantly, an
        # ID remains consumed even if the subsequent ACCEPT send fails: sendall
        # may have exposed a prefix of that ACCEPT to the peer, so recycling the
        # same identity would make the next physical connection ambiguous.
        session_id = self._next_session_id
        if session_id == protocol.UINT32_MAX:
            self._exhausted = True
        else:
            self._next_session_id += 1
        return session_id


def read_exact(connection: socket.socket, byte_count: int) -> bytes:
    # TCP is a byte stream, so one recv() is not assumed to return one complete
    # Wire field. EOF before the requested byte count makes the provisional or
    # active physical connection terminal.
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
        rfb_attachment: rfb.RfbRelay | None = None,
    ) -> None:
        self._connection = connection
        self._session_ids = session_ids
        self._rfb_relay = rfb_attachment
        self.state = WireSessionState.PROVISIONAL
        self.session_id: int | None = None
        self.next_receive_sequence = 1
        self.next_send_sequence = 1

    def _send_rejection(self, reason: int) -> None:
        # Rejections are themselves framed Wire traffic and therefore consume
        # the server->client sequence exactly like ACCEPT.
        self._connection.sendall(
            protocol.encode_not_accepted_frame(
                reason,
                sequence=self.next_send_sequence,
            )
        )
        self.next_send_sequence += 1

    def _read_provisional_hello(self) -> tuple[int, int]:
        raw_header = read_exact(self._connection, protocol.HEADER_BYTES)
        try:
            header = protocol.decode_header(raw_header)
        except protocol.WireProtocolError as exc:
            # Bad magic/header-version/length means we cannot safely assume the
            # peer understands enough PSTV framing to interpret NOT_ACCEPTED.
            # Close instead of manufacturing a response on an untrusted frame.
            raise UnsafeProvisionalFraming(str(exc)) from exc

        # Once the fixed header is structurally valid, consuming its declared
        # payload lets us distinguish a well-framed but semantically malformed
        # establishment request from unsafe framing.
        payload = read_exact(self._connection, header.payload_length)

        if (
            self.next_receive_sequence != 1
            or header.sequence != self.next_receive_sequence
            or not protocol.is_hello_header(header)
        ):
            # Q4 has exactly one legal first application frame. A valid PSTV
            # header with the wrong sequence/kind/channel/flags/length is safe
            # to reject as MALFORMED, but it never creates ACTIVE authority.
            raise protocol.WireProtocolError(
                "first application frame is not exact sequence-1 HELLO"
            )

        wire_version, product_version = protocol.decode_hello_payload(payload)
        self.next_receive_sequence = 2
        return wire_version, product_version

    def establish(self) -> WireSessionOutcome:
        try:
            wire_version, product_version = self._read_provisional_hello()
        except (EOFError, UnsafeProvisionalFraming):
            # Nothing ACTIVE exists yet. Unsafe framing or premature EOF simply
            # retires this provisional TCP connection; the persistent listener
            # survives and may accept a completely fresh connection afterward.
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
            # Compatibility failure is a normal, well-framed Q4 outcome. Send
            # the exact bounded reason, remain PROVISIONAL/never ACTIVE, then
            # retire only this connection.

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

        # Allocate before ACCEPT so the Pi is authoritative for identity.
        # ACTIVE is intentionally *not* published yet: successful ACCEPT send is
        # the final establishment fence.
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

        # Only after sendall succeeds may local state claim ACTIVE. The next
        # sequence values are direction-local and begin ordinary post-Q4 traffic
        # at 2, preserving sequence 1 for HELLO/ACCEPT establishment.
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

    def _send_active_frame(
        self,
        kind: int,
        channel: int,
        payload: bytes,
    ) -> None:
        """Serialize one post-Q4 frame through the sole Wire send owner."""

        self._connection.sendall(
            protocol.encode_channel_frame(
                kind,
                channel,
                self.next_send_sequence,
                payload,
            )
        )
        self.next_send_sequence += 1

    def _read_active_frame(self) -> tuple[protocol.WireHeader, bytes]:
        """Read one complete next-sequence frame through the sole Wire owner."""

        raw_header = read_exact(self._connection, protocol.HEADER_BYTES)
        header = protocol.decode_header(raw_header)
        payload = read_exact(self._connection, header.payload_length)

        if header.sequence != self.next_receive_sequence:
            raise protocol.WireProtocolError(
                "active Wire receive sequence mismatch"
            )

        self.next_receive_sequence += 1
        return header, payload

    def _handle_rfb_frame(
        self,
        header: protocol.WireHeader,
        payload: bytes,
    ) -> bool:
        if self._rfb_relay is None:
            return False

        if protocol.is_rfb_credit_header(header):
            amount = protocol.decode_rfb_credit_payload(payload)
            # A terminal provider stops accepting new read authority but does
            # not redefine the containing Wire Session.
            self._rfb_relay.add_provider_read_credit(amount)
            return True

        if protocol.is_rfb_data_header(header):
            if not payload:
                # Zero-length channel-1 DATA belongs to the existing quiesce
                # lifecycle. R10 preserves the reservation but deliberately
                # does not implement REQUEST/BOUNDARY/COMMIT/COMPLETE here.
                self._rfb_relay.note_reserved_zero_length_marker()
                return True

            self._rfb_relay.accept_ps2_data(payload)
            return True

        return False

    def _wait_with_rfb_relay(self) -> WireSessionOutcome:
        if self._rfb_relay is None:
            raise RuntimeError("RFB relay is not attached")

        try:
            initial_credit = self._rfb_relay.activate()
            self._send_active_frame(
                protocol.FRAME_CREDIT,
                protocol.CHANNEL_RFB,
                protocol.encode_rfb_credit_payload(initial_credit),
            )
        except (OSError, rfb.RfbRelayError):
            return self._finish(
                accepted=True,
                rejection_reason=None,
                protocol_failed=True,
            )

        while True:
            provider = self._rfb_relay.provider_socket
            read_wait = [self._connection]
            write_wait: list[socket.socket] = []

            if self._rfb_relay.wants_provider_read:
                read_wait.append(provider)
            if self._rfb_relay.wants_provider_write:
                write_wait.append(provider)

            try:
                readable, writable, _exceptional = select.select(
                    read_wait,
                    write_wait,
                    [],
                )
            except (OSError, ValueError):
                return self._finish(
                    accepted=True,
                    rejection_reason=None,
                    protocol_failed=True,
                )

            if self._connection in readable:
                try:
                    header, payload = self._read_active_frame()
                    if not self._handle_rfb_frame(header, payload):
                        raise protocol.WireProtocolError(
                            "unsupported active Wire frame"
                        )
                except EOFError:
                    return self._finish(
                        accepted=True,
                        rejection_reason=None,
                        protocol_failed=False,
                    )
                except (OSError, protocol.WireProtocolError, rfb.RfbRelayError):
                    return self._finish(
                        accepted=True,
                        rejection_reason=None,
                        protocol_failed=True,
                    )

            # Provider writes are nonblocking and readiness-driven. Queue drain
            # earns replacement Pi CREDIT only after provider send() actually
            # releases bytes from the finite queue.
            if provider in writable and not self._rfb_relay.terminal:
                drained = self._rfb_relay.drain_provider_write_ready()
                if drained:
                    try:
                        self._send_active_frame(
                            protocol.FRAME_CREDIT,
                            protocol.CHANNEL_RFB,
                            protocol.encode_rfb_credit_payload(drained),
                        )
                        self._rfb_relay.confirm_credit_sent(drained)
                    except (OSError, rfb.RfbRelayError):
                        return self._finish(
                            accepted=True,
                            rejection_reason=None,
                            protocol_failed=True,
                        )

            # Provider reads occur only while PS2-granted credit exists. The
            # relay returns raw bytes; only this Wire owner frames/sends them.
            if provider in readable and not self._rfb_relay.terminal:
                provider_payload = self._rfb_relay.read_provider_ready()
                if provider_payload:
                    try:
                        self._send_active_frame(
                            protocol.FRAME_DATA,
                            protocol.CHANNEL_RFB,
                            provider_payload,
                        )
                    except OSError:
                        return self._finish(
                            accepted=True,
                            rejection_reason=None,
                            protocol_failed=True,
                        )

    def wait_until_session_end(self) -> WireSessionOutcome:
        if self.state is not WireSessionState.ACTIVE or self.session_id is None:
            raise RuntimeError("Wire Session is not ACTIVE")

        if self._rfb_relay is not None:
            return self._wait_with_rfb_relay()

        # With no explicitly injected rider the installed service remains
        # establishment-only. An ACTIVE session may be completely idle forever;
        # any ordinary post-Q4 byte is still unsupported in that composition.
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
        # Retiring the connection removes ACTIVE authority before the outer
        # server closes the socket or accepts another peer. The historical
        # session_id is returned only as evidence; it is not reusable authority.
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

    def serve_connection(
        self,
        connection: socket.socket,
        rfb_attachment: rfb.RfbRelay | None = None,
    ) -> WireSessionOutcome:
        # One connection owner contains every physical Wire read/write. An
        # optional relay owns only its separately injected provider socket.
        owner = WireConnectionOwner(
            connection,
            self.session_ids,
            rfb_attachment=rfb_attachment,
        )
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
            if rfb_attachment is not None:
                rfb_attachment.close()

    def serve_forever(self) -> None:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
            listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            listener.bind((self.listen_address, self.port))
            listener.listen(LISTEN_BACKLOG)

            while True:
                # Sessions remain deliberately sequential. The installed
                # service passes no provider attachment here, so R10 does not
                # silently select/migrate a concrete RFB endpoint.
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
