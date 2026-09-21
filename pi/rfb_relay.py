#!/usr/bin/env python3
"""File synopsis:
Owns one session-scoped, provider-neutral Pi RFB relay behind the product Wire
server.

The relay owns only an explicitly supplied local RFB provider socket, two RFB
credit windows, and one bounded PS2->provider byte queue. It never receives from
or sends to the PS2-facing Wire socket. The Wire connection owner remains the
sole physical Wire I/O context and calls this object only after readiness has
already been observed.

Provider->PS2 reads are permitted only by PS2-granted channel-1 CREDIT.
PS2->provider DATA is admitted only by Pi-granted finite queue capacity, and
that capacity is re-granted only after bytes actually leave the queue through a
nonblocking provider send.

Non-empty DATA is opaque RFB byte-stream payload. Zero-length channel-1 DATA is
reserved for the existing quiesce lifecycle and is deliberately not accepted by
this raw-byte relay.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-RFB-WIRE-RELAY-R10.
"""

from __future__ import annotations

from dataclasses import dataclass
import socket

import wire_protocol as protocol


class RfbRelayError(RuntimeError):
    """Reject invalid relay state, credit, or provider-local mechanisms."""


@dataclass
class RfbRelayStats:
    provider_read_credit_frames: int = 0
    provider_read_credit_bytes: int = 0
    provider_read_calls: int = 0
    provider_bytes_read: int = 0
    wire_data_frames_received: int = 0
    wire_data_bytes_received: int = 0
    provider_write_calls: int = 0
    provider_bytes_written: int = 0
    credit_bytes_returned: int = 0
    reserved_zero_length_markers: int = 0
    terminal_discarded_bytes: int = 0


class RfbRelay:
    """Bounded raw-byte courier for one Wire Session and one provider socket."""

    def __init__(
        self,
        provider_socket: socket.socket,
        *,
        provider_read_credit_limit: int,
        provider_write_capacity: int,
        max_data_payload: int,
    ) -> None:
        if provider_socket is None:
            raise ValueError("provider_socket is required")
        for name, value in (
            ("provider_read_credit_limit", provider_read_credit_limit),
            ("provider_write_capacity", provider_write_capacity),
            ("max_data_payload", max_data_payload),
        ):
            if not isinstance(value, int) or value <= 0:
                raise ValueError(f"{name} must be positive")
        if provider_read_credit_limit > protocol.UINT32_MAX:
            raise ValueError("provider_read_credit_limit exceeds uint32")
        if provider_write_capacity > protocol.UINT32_MAX:
            raise ValueError("provider_write_capacity exceeds uint32")
        if max_data_payload > protocol.MAX_PAYLOAD_BYTES:
            raise ValueError("max_data_payload exceeds Wire maximum")
        if max_data_payload > provider_read_credit_limit:
            raise ValueError(
                "max_data_payload may not exceed provider_read_credit_limit"
            )
        if max_data_payload > provider_write_capacity:
            raise ValueError(
                "max_data_payload may not exceed provider_write_capacity"
            )

        # Ownership of this local provider attachment transfers to the relay.
        # Nonblocking provider I/O is essential: a stalled VNC peer may consume
        # all advertised Pi queue capacity, but must never suspend the sole Wire
        # receive/send owner.
        self.provider_socket = provider_socket
        self.provider_socket.setblocking(False)

        self.provider_read_credit_limit = provider_read_credit_limit
        self.provider_write_capacity = provider_write_capacity
        self.max_data_payload = max_data_payload

        # PS2 CREDIT authorizes provider->PS2 reads. This is deliberately not a
        # staging-buffer budget: when it reaches zero we stop reading provider
        # bytes and let provider TCP backpressure apply.
        self.provider_read_credit = 0

        # Pi CREDIT authorizes PS2->provider DATA into the finite local queue.
        # It increases only when a CREDIT frame has actually been sent by the
        # Wire owner, never merely because local queue storage became free.
        self.ps2_write_credit = 0
        self.pending_credit_return = 0
        self.provider_write_queue = bytearray()

        self.activated = False
        self.terminal = False
        self.closed = False
        self.provider_eof = False
        self.stats = RfbRelayStats()

    @property
    def queued_provider_write_bytes(self) -> int:
        return len(self.provider_write_queue)

    @property
    def wants_provider_read(self) -> bool:
        return not self.terminal and self.provider_read_credit > 0

    @property
    def wants_provider_write(self) -> bool:
        return not self.terminal and bool(self.provider_write_queue)

    def activate(self) -> int:
        """Publish exactly the finite initial PS2->provider queue capacity."""

        if self.activated or self.terminal or self.closed:
            raise RfbRelayError("RFB relay cannot be activated in current state")

        self.activated = True
        self.ps2_write_credit = self.provider_write_capacity
        return self.provider_write_capacity

    def add_provider_read_credit(self, amount: int) -> bool:
        """Add PS2 receiver credit that permits provider->PS2 socket reads."""

        if not isinstance(amount, int) or amount <= 0 or amount > protocol.UINT32_MAX:
            raise RfbRelayError("RFB provider-read credit must be nonzero uint32")

        # A provider-local terminal condition is rider-local. Validate framing
        # but do not let a late CREDIT frame redefine the containing Wire
        # Session; simply stop increasing provider-read authority.
        if self.terminal:
            return False

        next_credit = self.provider_read_credit + amount
        if next_credit > self.provider_read_credit_limit:
            raise RfbRelayError(
                "RFB provider-read credit exceeds configured PS2 receive window"
            )

        self.provider_read_credit = next_credit
        self.stats.provider_read_credit_frames += 1
        self.stats.provider_read_credit_bytes += amount
        return True

    def note_reserved_zero_length_marker(self) -> None:
        """Record, but never forward, one reserved RFB lifecycle marker."""

        self.stats.reserved_zero_length_markers += 1

    def accept_ps2_data(self, payload: bytes) -> bool:
        """Admit one credited non-empty PS2 RFB fragment into bounded storage."""

        if not payload:
            raise RfbRelayError("zero-length RFB DATA is lifecycle-reserved")
        if len(payload) > self.max_data_payload:
            raise RfbRelayError("RFB DATA exceeds configured Wire DATA maximum")
        if len(payload) > self.ps2_write_credit:
            raise RfbRelayError("PS2 RFB DATA exceeds Pi-granted credit")

        # Consume peer-visible credit at admission. Queue capacity and peer
        # credit therefore describe the same finite free-space authority.
        self.ps2_write_credit -= len(payload)

        if self.terminal:
            # Provider failure is rider-local. Bytes still covered by credit
            # granted before failure may arrive afterward; consume/drop those
            # bytes but issue no replacement credit and keep Wire alive.
            self.stats.terminal_discarded_bytes += len(payload)
            return False

        if len(self.provider_write_queue) + len(payload) > self.provider_write_capacity:
            raise RfbRelayError("RFB provider-write queue capacity exceeded")

        self.provider_write_queue.extend(payload)
        self.stats.wire_data_frames_received += 1
        self.stats.wire_data_bytes_received += len(payload)
        return True

    def read_provider_ready(self) -> bytes | None:
        """Read at most currently credited bytes from an already-ready provider."""

        if not self.wants_provider_read:
            return None

        count = min(
            self.provider_read_credit,
            self.max_data_payload,
        )

        try:
            payload = self.provider_socket.recv(count)
        except BlockingIOError:
            return None
        except OSError:
            self.mark_terminal()
            return None

        self.stats.provider_read_calls += 1

        if not payload:
            self.provider_eof = True
            self.mark_terminal()
            return None

        self.provider_read_credit -= len(payload)
        self.stats.provider_bytes_read += len(payload)
        return payload

    def drain_provider_write_ready(self) -> int:
        """Nonblockingly drain queued PS2 bytes into an already-ready provider."""

        if not self.wants_provider_write:
            return 0

        try:
            sent = self.provider_socket.send(self.provider_write_queue)
        except BlockingIOError:
            return 0
        except OSError:
            self.mark_terminal()
            return 0

        self.stats.provider_write_calls += 1

        if sent <= 0:
            self.mark_terminal()
            return 0

        del self.provider_write_queue[:sent]
        self.pending_credit_return += sent
        self.stats.provider_bytes_written += sent
        return sent

    def confirm_credit_sent(self, amount: int) -> None:
        """Publish queue capacity only after the Wire owner sent matching CREDIT."""

        if amount <= 0 or amount > self.pending_credit_return:
            raise RfbRelayError("invalid returned-credit confirmation")
        if self.ps2_write_credit > protocol.UINT32_MAX - amount:
            raise RfbRelayError("RFB PS2-write credit overflow")

        self.pending_credit_return -= amount
        self.ps2_write_credit += amount
        self.stats.credit_bytes_returned += amount

    def mark_terminal(self) -> None:
        """Stop provider I/O without changing containing Wire-session validity."""

        if self.terminal:
            return

        self.terminal = True

        # Queued bytes cannot be delivered after provider failure. Discard them
        # locally and, critically, do not turn their freed storage into CREDIT.
        # Any peer-visible credit already issued remains consumable once; late
        # bytes covered by that old grant are dropped by accept_ps2_data().
        if self.provider_write_queue:
            self.stats.terminal_discarded_bytes += len(self.provider_write_queue)
            self.provider_write_queue.clear()

    def close(self) -> None:
        """Terminally retire this session-scoped relay and provider attachment."""

        if self.closed:
            return

        self.mark_terminal()
        self.closed = True
        try:
            self.provider_socket.close()
        except OSError:
            pass
