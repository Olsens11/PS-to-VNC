#!/usr/bin/env python3
"""File synopsis:
Implements the maintained Raspberry Pi product PSTV Wire listener and provisional
Q4 establishment owner.

One long-lived server process owns the listening socket and, sequentially, the
accepted socket, all physical recv/frame parsing, and all physical send for one
Wire Session. TCP accept begins PROVISIONAL; only an exact HELLO followed by a
successfully sent Pi-owned ACCEPT creates ACTIVE. EOF or protocol failure fully
retires that connection before the persistent owner accepts another.

R13 composes one explicit session-scoped RFB attachment mechanism while
preserving this object as the sole PS2-facing Wire recv/send and global sequence
owner. R17 optionally composes one exact-generation MPEG owner under the same
physical loop: START/RETIRE/CREDIT are received here, while MPEG DATA is emitted
only by this owner's ordinary global-sequence send path.

With no explicit rider factories the server remains establishment-only. Rider
mechanisms own their local lifecycle; this server still owns no selected profile
defaults, Application policy, AUDIO/CONFIG rider, heartbeat, or custom restart
loop.

Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from enum import Enum
import select
import socket
import sys
from typing import Callable

import mpeg_generation as mpeg
import rfb_attachment as rfb_attach
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
        rfb_attachment: rfb_attach.RfbAttachment | None = None,
        mpeg_generation: mpeg.MpegGenerationController | None = None,
    ) -> None:
        self._connection = connection
        self._session_ids = session_ids
        self._rfb_attachment = rfb_attachment
        self._mpeg_generation = mpeg_generation
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
        attachment = self._rfb_attachment
        if attachment is None:
            return False

        if protocol.is_rfb_credit_header(header):
            amount = protocol.decode_rfb_credit_payload(payload)
            attachment.add_ps2_credit(amount)
            return True

        if protocol.is_rfb_data_header(header):
            if not payload:
                attachment.accept_ps2_marker()
                return True
            attachment.accept_ps2_data(payload)
            return True

        return False

    def attach_mpeg_generation(
        self,
        generation: mpeg.MpegGenerationController,
    ) -> None:
        if (
            self.state is not WireSessionState.ACTIVE
            or self.session_id is None
            or self._mpeg_generation is not None
            or generation.session_id != self.session_id
        ):
            raise RuntimeError("MPEG generation owner does not match active Wire")
        self._mpeg_generation = generation

    def _handle_mpeg_frame(
        self,
        header: protocol.WireHeader,
        payload: bytes,
    ) -> bool:
        generation = self._mpeg_generation
        if generation is None:
            return False

        if protocol.is_mpeg_credit_header(header):
            generation.add_credit(protocol.decode_mpeg_credit_payload(payload))
            return True

        if protocol.is_mpeg_start_header(header):
            generation.start_exact(protocol.decode_mpeg_start_payload(payload))
            return True

        if protocol.is_mpeg_retire_header(header):
            control = protocol.decode_mpeg_retire_payload(payload)
            completion = generation.retire_exact(control)
            self._send_active_frame(
                protocol.FRAME_MPEG_RETIRE,
                protocol.CHANNEL_CONTROL,
                protocol.encode_mpeg_retire_payload(completion),
            )
            generation.confirm_retire_completion(completion)
            return True

        return False

    def _flush_mpeg_generation_output(self) -> None:
        generation = self._mpeg_generation
        if generation is None:
            return

        lease = generation.begin_emission(protocol.MAX_PAYLOAD_BYTES)
        if lease is None:
            return
        try:
            self._send_active_frame(
                protocol.FRAME_DATA,
                protocol.CHANNEL_MPEG2,
                lease.payload,
            )
        finally:
            # sendall() returning or raising ends this local physical-send call.
            # A failure makes the Wire Session terminal; finishing the local lease
            # is not a claim that failed physical bytes were delivered.
            generation.finish_emission(lease)

    def _flush_rfb_attachment_output(self) -> None:
        """Serialize attachment output through the sole physical Wire owner."""

        attachment = self._rfb_attachment
        if attachment is None:
            return

        initial_credit = attachment.take_initial_ps2_credit()
        if initial_credit:
            self._send_active_frame(
                protocol.FRAME_CREDIT,
                protocol.CHANNEL_RFB,
                protocol.encode_rfb_credit_payload(initial_credit),
            )

        if attachment.wants_request_marker:
            self._send_active_frame(
                protocol.FRAME_DATA,
                protocol.CHANNEL_RFB,
                b"",
            )
            attachment.confirm_request_sent()

        if attachment.wants_commit_marker:
            self._send_active_frame(
                protocol.FRAME_DATA,
                protocol.CHANNEL_RFB,
                b"",
            )
            attachment.confirm_commit_sent()

        if attachment.wants_provider_failure_report:
            # ERROR=7 on channel 1 is R16A's typed provider-terminal mechanism.
            # The attachment never sends it itself: this owner allocates the
            # ordinary Wire sequence and performs the physical send. Successful
            # reporting does not clear the attachment's FAILED/cause authority.
            reason = attachment.provider_failure
            if reason is None:
                raise RuntimeError("provider failure report has no reason")
            self._send_active_frame(
                protocol.FRAME_ERROR,
                protocol.CHANNEL_RFB,
                protocol.encode_rfb_provider_failure_payload(reason.value),
            )
            if not attachment.confirm_provider_failure_reported(reason):
                raise RuntimeError("provider failure report confirmation failed")

    def _wait_with_riders(self) -> WireSessionOutcome:
        """Drive configured riders while retaining sole physical Wire I/O."""

        attachment = self._rfb_attachment
        generation = self._mpeg_generation
        if attachment is None and generation is None:
            raise RuntimeError("no Wire rider is configured")

        while True:
            try:
                self._flush_rfb_attachment_output()
            except (OSError, RuntimeError):
                return self._finish(
                    accepted=True,
                    rejection_reason=None,
                    protocol_failed=True,
                )

            connecting = (
                attachment.connecting_socket if attachment is not None else None
            )
            provider = attachment.provider_socket if attachment is not None else None
            quiesce_wake = None
            if attachment is not None:
                # Keep the canonical R13 ownership expression visible to the
                # repository boundary guard while sharing the outer select loop.
                quiesce_wake = attachment.quiesce_wake_reader
            mpeg_wake = (
                generation.activity_reader if generation is not None else None
            )

            read_wait = [self._connection]
            write_wait: list[socket.socket] = []
            exception_wait: list[socket.socket] = []

            if quiesce_wake is not None:
                read_wait.append(quiesce_wake)
            if mpeg_wake is not None:
                read_wait.append(mpeg_wake)
            if connecting is not None:
                write_wait.append(connecting)
                exception_wait.append(connecting)
            if (
                attachment is not None
                and provider is not None
                and attachment.wants_provider_read
            ):
                read_wait.append(provider)
            if (
                attachment is not None
                and provider is not None
                and attachment.wants_provider_write
            ):
                write_wait.append(provider)

            try:
                readable, writable, exceptional = select.select(
                    read_wait,
                    write_wait,
                    exception_wait,
                )
            except (OSError, ValueError):
                return self._finish(
                    accepted=True,
                    rejection_reason=None,
                    protocol_failed=True,
                )

            if (
                attachment is not None
                and quiesce_wake is not None
                and quiesce_wake in readable
            ):
                attachment.acknowledge_quiesce_wake()
                try:
                    self._flush_rfb_attachment_output()
                except (OSError, RuntimeError):
                    return self._finish(
                        accepted=True,
                        rejection_reason=None,
                        protocol_failed=True,
                    )

            # Prefer an already-readable physical frame over producer output.
            # In particular, an exact RETIRE request closes generation emission
            # admission before a simultaneous local producer wake can send more.
            if self._connection in readable:
                try:
                    header, payload = self._read_active_frame()
                    handled = self._handle_rfb_frame(header, payload)
                    if not handled:
                        handled = self._handle_mpeg_frame(header, payload)
                    if not handled:
                        raise protocol.WireProtocolError(
                            "unsupported active Wire frame"
                        )
                except EOFError:
                    return self._finish(
                        accepted=True,
                        rejection_reason=None,
                        protocol_failed=False,
                    )
                except (
                    OSError,
                    RuntimeError,
                    mpeg.MpegGenerationError,
                    protocol.WireProtocolError,
                ):
                    return self._finish(
                        accepted=True,
                        rejection_reason=None,
                        protocol_failed=True,
                    )

            if (
                generation is not None
                and mpeg_wake is not None
                and mpeg_wake in readable
            ):
                generation.acknowledge_activity()
                try:
                    self._flush_mpeg_generation_output()
                except (OSError, mpeg.MpegGenerationError):
                    return self._finish(
                        accepted=True,
                        rejection_reason=None,
                        protocol_failed=True,
                    )

            if (
                attachment is not None
                and connecting is not None
                and (connecting in writable or connecting in exceptional)
            ):
                attachment.finish_connect_ready()

            if (
                attachment is not None
                and provider is not None
                and provider in writable
                and attachment.wants_provider_write
            ):
                drained = attachment.drain_provider_write_ready()
                if drained:
                    try:
                        self._send_active_frame(
                            protocol.FRAME_CREDIT,
                            protocol.CHANNEL_RFB,
                            protocol.encode_rfb_credit_payload(drained),
                        )
                    except OSError:
                        return self._finish(
                            accepted=True,
                            rejection_reason=None,
                            protocol_failed=True,
                        )
                    attachment.confirm_credit_sent(drained)

            if (
                attachment is not None
                and provider is not None
                and provider in readable
                and attachment.wants_provider_read
            ):
                provider_payload = attachment.read_provider_ready()
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

        if self._rfb_attachment is not None or self._mpeg_generation is not None:
            return self._wait_with_riders()

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
        if self._mpeg_generation is not None:
            if not self._mpeg_generation.close():
                protocol_failed = True
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
        rfb_attachment_factory: Callable[
            [], rfb_attach.RfbAttachment
        ] | None = None,
        mpeg_generation_factory: Callable[
            [int], mpeg.MpegGenerationController
        ] | None = None,
    ) -> None:
        if not listen_address:
            raise ValueError("listen_address must be non-empty")
        if not isinstance(port, int) or port <= 0 or port > 65535:
            raise ValueError("port must be in 1..65535")
        self.listen_address = listen_address
        self.port = port
        self.session_ids = session_ids or SessionIdAllocator()
        self.rfb_attachment_factory = rfb_attachment_factory
        self.mpeg_generation_factory = mpeg_generation_factory

    def serve_connection(
        self,
        connection: socket.socket,
        rfb_attachment: rfb_attach.RfbAttachment | None = None,
    ) -> WireSessionOutcome:
        attachment = rfb_attachment
        if attachment is None and self.rfb_attachment_factory is not None:
            attachment = self.rfb_attachment_factory()

        owner = WireConnectionOwner(
            connection,
            self.session_ids,
            rfb_attachment=attachment,
        )
        try:
            outcome = owner.establish()
            if outcome.accepted:
                if (
                    self.mpeg_generation_factory is not None
                    and outcome.session_id is not None
                ):
                    try:
                        generation = self.mpeg_generation_factory(outcome.session_id)
                        owner.attach_mpeg_generation(generation)
                    except (RuntimeError, ValueError, mpeg.MpegGenerationError):
                        return owner._finish(
                            accepted=True,
                            rejection_reason=None,
                            protocol_failed=True,
                        )
                outcome = owner.wait_until_session_end()
            return outcome
        finally:
            try:
                connection.close()
            except OSError:
                pass
            if attachment is not None:
                attachment.close()

    def serve_forever(self) -> None:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
            listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            listener.bind((self.listen_address, self.port))
            listener.listen(LISTEN_BACKLOG)

            while True:
                # Sessions remain deliberately sequential. The default
                # service supplies no attachment factory, so an idle or active
                # Wire Session cannot select/start RFB merely because the
                # physical connection exists.
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
