#!/usr/bin/env python3
"""File synopsis:
Owns one Wire-Session-scoped Raspberry Pi RFB provider attachment and its finite
lifecycle around the provider-neutral R10 relay.

An attachment starts inert. The first valid nonzero PS2 channel-1 CREDIT is the
only R13 lazy-start edge: it opens one nonblocking connection attempt to the
selected Pi-local provider endpoint at 127.0.0.1:5900. Wire establishment alone
does not touch the provider. Flow limits are injected explicitly through
RfbFlowConfig; this module intentionally defines no product tuning defaults.

After provider connection succeeds, ownership of that socket transfers exactly
once into RfbRelay. The Wire connection owner remains the only PS2-facing
recv/send and global sequence owner. This object owns only provider attachment
state, RFB-local failure containment, and the ordered zero-marker lifecycle:

    REQUEST -> BOUNDARY -> COMMIT -> COMPLETE

At BOUNDARY new provider reads stop immediately. Already accepted
PS2-to-provider bytes must drain before the provider is closed and COMMIT becomes
sendable. COMPLETE stops only this RFB attachment; it does not end Wire.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-RFB-ATTACHMENT-QUIESCE-R13.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
import errno
import socket
import threading
from typing import Callable

import rfb_relay as rfb
import wire_protocol as protocol

INTERNAL_PROVIDER_HOST = "127.0.0.1"
INTERNAL_PROVIDER_PORT = 5900
_PENDING_CONNECT_ERRORS = {
    errno.EINPROGRESS,
    errno.EWOULDBLOCK,
    errno.EALREADY,
    errno.EINTR,
}


class RfbAttachmentState(Enum):
    """Finite lifecycle for one provider attachment inside one Wire Session."""

    IDLE = "IDLE"
    CONNECTING = "CONNECTING"
    RUNNING = "RUNNING"
    REQUEST_PENDING = "REQUEST_PENDING"
    WAIT_BOUNDARY = "WAIT_BOUNDARY"
    DRAINING = "DRAINING"
    COMMIT_PENDING = "COMMIT_PENDING"
    WAIT_COMPLETE = "WAIT_COMPLETE"
    STOPPED = "STOPPED"
    FAILED = "FAILED"


@dataclass(frozen=True)
class RfbFlowConfig:
    """Explicit finite flow authority supplied by later session composition."""

    provider_read_credit_limit: int
    provider_write_capacity: int
    max_data_payload: int

    def __post_init__(self) -> None:
        for name, value in (
            ("provider_read_credit_limit", self.provider_read_credit_limit),
            ("provider_write_capacity", self.provider_write_capacity),
            ("max_data_payload", self.max_data_payload),
        ):
            if not isinstance(value, int) or value <= 0:
                raise ValueError(f"{name} must be positive")

        if self.provider_read_credit_limit > protocol.UINT32_MAX:
            raise ValueError("provider_read_credit_limit exceeds uint32")
        if self.provider_write_capacity > protocol.UINT32_MAX:
            raise ValueError("provider_write_capacity exceeds uint32")
        if self.max_data_payload > protocol.MAX_PAYLOAD_BYTES:
            raise ValueError("max_data_payload exceeds Wire maximum")
        if self.max_data_payload > self.provider_read_credit_limit:
            raise ValueError(
                "max_data_payload may not exceed provider_read_credit_limit"
            )
        if self.max_data_payload > self.provider_write_capacity:
            raise ValueError(
                "max_data_payload may not exceed provider_write_capacity"
            )


@dataclass
class RfbAttachmentStats:
    """Observable R13 mechanism counters; none are synchronization authority."""

    connect_attempts: int = 0
    connect_successes: int = 0
    connect_failures: int = 0
    request_markers_sent: int = 0
    boundary_markers_received: int = 0
    provider_retirements: int = 0
    commit_markers_sent: int = 0
    complete_markers_received: int = 0
    lifecycle_failures: int = 0


def _new_provider_socket() -> socket.socket:
    """Create one ordinary nonblocking IPv4 provider socket."""

    return socket.socket(socket.AF_INET, socket.SOCK_STREAM)


class RfbAttachment:
    """Bind one selected local provider to one Wire Session, at most once."""

    def __init__(
        self,
        flow: RfbFlowConfig,
        *,
        socket_factory: Callable[[], socket.socket] = _new_provider_socket,
    ) -> None:
        if not isinstance(flow, RfbFlowConfig):
            raise ValueError("flow must be an RfbFlowConfig")
        if not callable(socket_factory):
            raise ValueError("socket_factory must be callable")

        self.flow = flow
        self._socket_factory = socket_factory
        self.state = RfbAttachmentState.IDLE
        self.stats = RfbAttachmentStats()

        self._connecting_socket: socket.socket | None = None
        self.relay: rfb.RfbRelay | None = None
        self._pending_provider_read_credit = 0
        self._pending_initial_ps2_credit = 0
        self._closed = False

        # request_quiesce() is the one intentional cross-thread seam. The
        # caller publishes RFB-local intent only; this private socketpair wakes
        # the sole Wire owner so that owner can serialize REQUEST itself.
        #
        # A per-attachment pair prevents Session-A wake state from becoming
        # Session-B authority. Both ends are nonblocking because the notification
        # is a one-bit edge, never a byte queue or alternate Wire path.
        self._quiesce_lock = threading.RLock()
        wake_reader, wake_writer = socket.socketpair()
        wake_reader.setblocking(False)
        wake_writer.setblocking(False)
        self._quiesce_wake_reader: socket.socket | None = wake_reader
        self._quiesce_wake_writer: socket.socket | None = wake_writer

    @property
    def quiesce_wake_reader(self) -> socket.socket | None:
        """Expose only the local readiness descriptor to the sole Wire owner."""

        with self._quiesce_lock:
            return self._quiesce_wake_reader

    def acknowledge_quiesce_wake(self) -> bool:
        """Drain the local wake edge; never serialize or mutate Wire traffic."""

        with self._quiesce_lock:
            reader = self._quiesce_wake_reader
            if reader is None:
                return False

            saw_wake = False
            while True:
                try:
                    payload = reader.recv(64)
                except BlockingIOError:
                    break
                except OSError:
                    return False

                if not payload:
                    break
                saw_wake = True

            return saw_wake

    @property
    def connecting_socket(self) -> socket.socket | None:
        """Expose only readiness identity for an in-progress provider connect."""

        if self.state is RfbAttachmentState.CONNECTING:
            return self._connecting_socket
        return None

    @property
    def provider_socket(self) -> socket.socket | None:
        """Expose provider readiness identity only after Relay composition."""

        if self.relay is None or self.relay.closed:
            return None
        return self.relay.provider_socket

    @property
    def wants_provider_read(self) -> bool:
        """Allow reads through BOUNDARY wait, never after BOUNDARY."""

        return (
            self.state
            in (
                RfbAttachmentState.RUNNING,
                RfbAttachmentState.REQUEST_PENDING,
                RfbAttachmentState.WAIT_BOUNDARY,
            )
            and self.relay is not None
            and self.relay.wants_provider_read
        )

    @property
    def wants_provider_write(self) -> bool:
        """Drain accepted writes during running and BOUNDARY retirement."""

        return (
            self.state
            in (
                RfbAttachmentState.RUNNING,
                RfbAttachmentState.REQUEST_PENDING,
                RfbAttachmentState.WAIT_BOUNDARY,
                RfbAttachmentState.DRAINING,
            )
            and self.relay is not None
            and self.relay.wants_provider_write
        )

    @property
    def wants_request_marker(self) -> bool:
        with self._quiesce_lock:
            return self.state is RfbAttachmentState.REQUEST_PENDING

    @property
    def wants_commit_marker(self) -> bool:
        return self.state is RfbAttachmentState.COMMIT_PENDING

    @property
    def fully_stopped(self) -> bool:
        return self.state is RfbAttachmentState.STOPPED

    def _close_connecting_socket(self) -> None:
        provider = self._connecting_socket
        self._connecting_socket = None
        if provider is None:
            return
        try:
            provider.close()
        except OSError:
            pass

    def _close_quiesce_wake(self) -> None:
        """Retire the Session-local wake descriptors exactly once."""

        with self._quiesce_lock:
            reader = self._quiesce_wake_reader
            writer = self._quiesce_wake_writer
            self._quiesce_wake_reader = None
            self._quiesce_wake_writer = None

            for endpoint in (reader, writer):
                if endpoint is None:
                    continue
                try:
                    endpoint.close()
                except OSError:
                    pass

    def _fail_local(self, *, connect_failure: bool = False) -> None:
        """Terminalize only this attachment and retire all provider I/O."""

        with self._quiesce_lock:
            if self.state is RfbAttachmentState.FAILED:
                return

            if connect_failure:
                self.stats.connect_failures += 1
            self.stats.lifecycle_failures += 1
            self.state = RfbAttachmentState.FAILED
            self._pending_initial_ps2_credit = 0
            self._pending_provider_read_credit = 0
            self._close_connecting_socket()

            if self.relay is not None:
                self.relay.close()

            # A failed attachment cannot publish a future quiesce intent. Close
            # both local wake endpoints so no stale Session-A readiness survives.
            self._close_quiesce_wake()

    def _begin_connect(self) -> None:
        """Start exactly one nonblocking connection to the selected endpoint."""

        if self.state is not RfbAttachmentState.IDLE:
            self._fail_local()
            return

        self.stats.connect_attempts += 1
        provider: socket.socket | None = None
        try:
            provider = self._socket_factory()
            provider.setblocking(False)
            result = provider.connect_ex(
                (INTERNAL_PROVIDER_HOST, INTERNAL_PROVIDER_PORT)
            )
        except (OSError, ValueError):
            if provider is not None:
                try:
                    provider.close()
                except OSError:
                    pass
            self._fail_local(connect_failure=True)
            return

        if result in (0, errno.EISCONN):
            self._compose_relay(provider)
            return

        if result in _PENDING_CONNECT_ERRORS:
            self._connecting_socket = provider
            self.state = RfbAttachmentState.CONNECTING
            return

        try:
            provider.close()
        except OSError:
            pass
        self._fail_local(connect_failure=True)

    def _compose_relay(self, provider: socket.socket) -> None:
        """Transfer a connected socket into exactly one configured R10 Relay."""

        if self.relay is not None:
            try:
                provider.close()
            except OSError:
                pass
            self._fail_local()
            return

        try:
            relay = rfb.RfbRelay(
                provider,
                provider_read_credit_limit=self.flow.provider_read_credit_limit,
                provider_write_capacity=self.flow.provider_write_capacity,
                max_data_payload=self.flow.max_data_payload,
            )
            if self._pending_provider_read_credit:
                relay.add_provider_read_credit(
                    self._pending_provider_read_credit
                )
            initial_credit = relay.activate()
        except (OSError, ValueError, rfb.RfbRelayError):
            try:
                provider.close()
            except OSError:
                pass
            self._fail_local(connect_failure=True)
            return

        self._connecting_socket = None
        self._pending_provider_read_credit = 0
        self.relay = relay
        self._pending_initial_ps2_credit = initial_credit
        self.state = RfbAttachmentState.RUNNING
        self.stats.connect_successes += 1

    def add_ps2_credit(self, amount: int) -> bool:
        """Accept PS2 receiver credit and lazily start provider attachment."""

        if (
            not isinstance(amount, int)
            or amount <= 0
            or amount > protocol.UINT32_MAX
        ):
            self._fail_local()
            return False

        if self.state in (
            RfbAttachmentState.IDLE,
            RfbAttachmentState.CONNECTING,
        ):
            next_credit = self._pending_provider_read_credit + amount
            if next_credit > self.flow.provider_read_credit_limit:
                self._fail_local()
                return False
            self._pending_provider_read_credit = next_credit

            if self.state is RfbAttachmentState.IDLE:
                self._begin_connect()
            return self.state not in (
                RfbAttachmentState.FAILED,
                RfbAttachmentState.STOPPED,
            )

        if self.state in (
            RfbAttachmentState.RUNNING,
            RfbAttachmentState.REQUEST_PENDING,
            RfbAttachmentState.WAIT_BOUNDARY,
        ):
            if self.relay is None:
                self._fail_local()
                return False
            try:
                return self.relay.add_provider_read_credit(amount)
            except rfb.RfbRelayError:
                self._fail_local()
                return False

        # BOUNDARY closes admission of new provider-read authority. A late
        # CREDIT is channel-local lifecycle misuse, never a Wire failure.
        self._fail_local()
        return False

    def finish_connect_ready(self) -> bool:
        """Resolve one readiness-signaled nonblocking provider connect."""

        if self.state is not RfbAttachmentState.CONNECTING:
            self._fail_local()
            return False

        provider = self._connecting_socket
        if provider is None:
            self._fail_local(connect_failure=True)
            return False

        try:
            error = provider.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR)
        except OSError:
            self._fail_local(connect_failure=True)
            return False

        if error in _PENDING_CONNECT_ERRORS:
            return False
        if error not in (0, errno.EISCONN):
            self._fail_local(connect_failure=True)
            return False

        self._compose_relay(provider)
        return self.state is RfbAttachmentState.RUNNING

    def take_initial_ps2_credit(self) -> int:
        """Return the one finite reverse-capacity grant after connect success."""

        amount = self._pending_initial_ps2_credit
        self._pending_initial_ps2_credit = 0
        return amount

    def accept_ps2_data(self, payload: bytes) -> bool:
        """Forward only credited nonempty client bytes to the configured Relay."""

        if not payload:
            self._fail_local()
            return False

        # After provider failure, consume/drop any bytes still covered by credit
        # already granted before failure. This contains the failure to RFB while
        # preventing stale peer authority from escaping into a replacement.
        if self.state is RfbAttachmentState.FAILED and self.relay is not None:
            try:
                self.relay.accept_ps2_data(payload)
            except rfb.RfbRelayError:
                pass
            return False

        if self.state not in (
            RfbAttachmentState.RUNNING,
            RfbAttachmentState.REQUEST_PENDING,
            RfbAttachmentState.WAIT_BOUNDARY,
        ):
            self._fail_local()
            return False

        if self.relay is None:
            self._fail_local()
            return False

        try:
            accepted = self.relay.accept_ps2_data(payload)
        except rfb.RfbRelayError:
            self._fail_local()
            return False

        if self.relay.terminal:
            self._fail_local()
            return False
        return accepted

    def read_provider_ready(self) -> bytes | None:
        """Read opaque provider bytes only while pre-BOUNDARY authority exists."""

        if not self.wants_provider_read or self.relay is None:
            return None

        payload = self.relay.read_provider_ready()
        if self.relay.terminal:
            self._fail_local()
            return None
        return payload

    def drain_provider_write_ready(self) -> int:
        """Drain accepted client bytes; return CREDIT only before BOUNDARY."""

        if not self.wants_provider_write or self.relay is None:
            return 0

        drained = self.relay.drain_provider_write_ready()
        if self.relay.terminal:
            self._fail_local()
            return 0

        if self.state is RfbAttachmentState.DRAINING:
            if self.relay.queued_provider_write_bytes == 0:
                self._retire_provider_for_commit()
            # Capacity freed during retirement is never re-advertised.
            return 0

        return drained

    def confirm_credit_sent(self, amount: int) -> None:
        """Confirm one running-state replacement CREDIT serialized by Wire."""

        if self.relay is None or self.state not in (
            RfbAttachmentState.RUNNING,
            RfbAttachmentState.REQUEST_PENDING,
            RfbAttachmentState.WAIT_BOUNDARY,
        ):
            self._fail_local()
            return

        try:
            self.relay.confirm_credit_sent(amount)
        except rfb.RfbRelayError:
            self._fail_local()

    def request_quiesce(self) -> bool:
        """Publish one REQUEST intent and wake, but never send, the Wire owner."""

        with self._quiesce_lock:
            if self.state is not RfbAttachmentState.RUNNING:
                self._fail_local()
                return False

            writer = self._quiesce_wake_writer
            if writer is None:
                self._fail_local()
                return False

            # State becomes observable before the wake edge. Once select() sees
            # this byte, the Wire owner can deterministically find exactly one
            # pending REQUEST and serialize it through _send_active_frame().
            self.state = RfbAttachmentState.REQUEST_PENDING
            try:
                writer.send(b"Q")
            except (BlockingIOError, OSError):
                self._fail_local()
                return False

            return True

    def confirm_request_sent(self) -> None:
        """Advance only after the sole Wire owner serialized REQUEST."""

        with self._quiesce_lock:
            if self.state is not RfbAttachmentState.REQUEST_PENDING:
                self._fail_local()
                return
            self.stats.request_markers_sent += 1
            self.state = RfbAttachmentState.WAIT_BOUNDARY

    def accept_ps2_marker(self) -> str | None:
        """Interpret one zero DATA marker strictly by the local ordered phase."""

        if self.state is RfbAttachmentState.WAIT_BOUNDARY:
            if self.relay is None:
                self._fail_local()
                return None
            self.relay.note_reserved_zero_length_marker()
            self.stats.boundary_markers_received += 1
            self.state = RfbAttachmentState.DRAINING

            if self.relay.terminal:
                self._fail_local()
                return None
            if self.relay.queued_provider_write_bytes == 0:
                self._retire_provider_for_commit()
            return "BOUNDARY"

        if self.state is RfbAttachmentState.WAIT_COMPLETE:
            self.stats.complete_markers_received += 1
            self.state = RfbAttachmentState.STOPPED
            self._close_quiesce_wake()
            return "COMPLETE"

        self._fail_local()
        return None

    def _retire_provider_for_commit(self) -> None:
        """Close provider I/O only after BOUNDARY and full accepted-write drain."""

        if self.state is not RfbAttachmentState.DRAINING or self.relay is None:
            self._fail_local()
            return
        if self.relay.queued_provider_write_bytes != 0:
            return

        self.relay.close()
        self.stats.provider_retirements += 1
        self.state = RfbAttachmentState.COMMIT_PENDING

    def confirm_commit_sent(self) -> None:
        """Advance only after provider retirement and Wire serialization."""

        if (
            self.state is not RfbAttachmentState.COMMIT_PENDING
            or self.relay is None
            or not self.relay.closed
        ):
            self._fail_local()
            return

        self.stats.commit_markers_sent += 1
        self.state = RfbAttachmentState.WAIT_COMPLETE

    def close(self) -> None:
        """Retire all Session-owned provider/wake state; never rebind it."""

        with self._quiesce_lock:
            if self._closed:
                return
            self._closed = True
            self._pending_initial_ps2_credit = 0
            self._pending_provider_read_credit = 0
            self._close_connecting_socket()

            if self.relay is not None:
                self.relay.close()

            if self.state is not RfbAttachmentState.FAILED:
                self.state = RfbAttachmentState.STOPPED

            self._close_quiesce_wake()
