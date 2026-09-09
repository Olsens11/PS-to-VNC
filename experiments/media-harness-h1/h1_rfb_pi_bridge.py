#!/usr/bin/env python3
"""
File synopsis:
    Defines the Pi-side raw-byte bridge between one upstream VNC socket and H1
    logical RFB channel 1.

The bridge deliberately knows nothing about RFB message syntax. It preserves the
clean PS2 parser as protocol authority and moves bytes only:

    VNC server -> Pi bridge -> H1 DATA channel 1 -> PS2 RFB parser
    PS2 RFB client bytes -> H1 DATA channel 1 -> Pi bridge -> VNC server

The VNC-server receive loop is driven directly by PS2-granted receiver credit.
It does not maintain a second arbitrary user-space staging queue: when no RFB
credit is available, it simply stops reading the upstream VNC socket and lets
TCP backpressure apply. Each emitted DATA fragment is bounded by H1's negotiated
max_data_payload.

This module owns only the Pi-side VNC socket. It never opens another PS2-facing
socket and never reads from the physical PSTV connection; H1Session remains the
sole owner of that connection and supplies send_data/add_credit/client_data
integration points.

Context: RFB_MUX_CP2G_LIVE_BEHIND_GATE.md.
"""

from __future__ import annotations

from dataclasses import dataclass
import socket
import threading
from typing import Callable


class RfbBridgeError(RuntimeError):
    """Raised when logical RFB bridge flow-control or socket state is invalid."""


@dataclass
class RfbBridgeStats:
    credit_frames_received: int = 0
    credit_bytes_received: int = 0
    server_read_calls: int = 0
    server_bytes_received: int = 0
    channel_frames_sent: int = 0
    channel_bytes_sent: int = 0
    channel_frames_received: int = 0
    channel_bytes_received: int = 0
    server_write_calls: int = 0
    server_bytes_sent: int = 0


class H1RfbPiBridge:
    """Credit-driven raw VNC-byte bridge for H1 logical channel 1."""

    def __init__(
        self,
        upstream: socket.socket,
        *,
        queue_capacity: int,
        max_payload: int,
        send_data: Callable[[bytes], None],
        name: str = "h1-rfb-upstream-reader",
    ) -> None:
        if queue_capacity <= 0:
            raise ValueError("RFB queue_capacity must be positive")
        if max_payload <= 0:
            raise ValueError("RFB max_payload must be positive")
        if max_payload > queue_capacity:
            raise ValueError("RFB max_payload may not exceed queue_capacity")

        self.upstream = upstream
        self.queue_capacity = int(queue_capacity)
        self.max_payload = int(max_payload)
        self.send_data = send_data
        self.name = name

        self.condition = threading.Condition()
        self.upstream_send_lock = threading.Lock()
        self.stop_event = threading.Event()
        self.credit = 0
        self.error: BaseException | None = None
        self.upstream_eof = False
        self.stats = RfbBridgeStats()
        self.thread: threading.Thread | None = None

    def start(self) -> None:
        if self.thread is not None:
            raise RfbBridgeError("RFB bridge already started")

        self.thread = threading.Thread(
            target=self._server_reader,
            name=self.name,
            daemon=True,
        )
        self.thread.start()

    def add_credit(self, amount: int) -> None:
        """Add one PS2 receiver-credit grant for VNC-server -> PS2 bytes."""

        amount = int(amount)
        if amount <= 0 or amount > 0xFFFFFFFF:
            raise RfbBridgeError(f"invalid RFB credit amount {amount}")

        with self.condition:
            next_credit = self.credit + amount
            # The PS2 grants free queue space. Outstanding producer credit must
            # therefore never exceed that queue's configured capacity.
            if next_credit > self.queue_capacity:
                raise RfbBridgeError(
                    "RFB credit window exceeds configured PS2 queue capacity: "
                    f"current={self.credit} add={amount} "
                    f"capacity={self.queue_capacity}"
                )

            self.credit = next_credit
            self.stats.credit_frames_received += 1
            self.stats.credit_bytes_received += amount
            self.condition.notify_all()

    def accept_client_data(self, payload: bytes) -> None:
        """Forward PS2 RFB-client bytes to the upstream VNC server exactly."""

        if not payload:
            raise RfbBridgeError("empty RFB client DATA payload")
        if len(payload) > self.max_payload:
            raise RfbBridgeError(
                f"RFB client DATA payload {len(payload)} exceeds {self.max_payload}"
            )

        self.check()
        try:
            with self.upstream_send_lock:
                self.upstream.sendall(payload)
        except BaseException as exc:
            self._record_error(exc)
            raise RfbBridgeError("failed writing PS2 RFB bytes to VNC server") from exc

        self.stats.channel_frames_received += 1
        self.stats.channel_bytes_received += len(payload)
        self.stats.server_write_calls += 1
        self.stats.server_bytes_sent += len(payload)

    def _record_error(self, exc: BaseException) -> None:
        with self.condition:
            if self.error is None:
                self.error = exc
            self.condition.notify_all()

    def _reserve_read_budget(self) -> int:
        with self.condition:
            while (
                self.credit == 0
                and not self.stop_event.is_set()
                and self.error is None
            ):
                self.condition.wait(timeout=0.1)

            if self.stop_event.is_set() or self.error is not None:
                return 0

            budget = min(self.credit, self.max_payload)
            # Reserve the credit before blocking in recv. A simultaneous CREDIT
            # frame may then replenish the remaining window without double-use.
            self.credit -= budget
            return budget

    def _restore_unused_budget(self, reserved: int, used: int) -> None:
        unused = reserved - used
        if unused <= 0:
            return
        with self.condition:
            self.credit += unused
            if self.credit > self.queue_capacity:
                self._record_error(
                    RfbBridgeError("RFB credit restoration overflowed window")
                )
            self.condition.notify_all()

    def _server_reader(self) -> None:
        try:
            while not self.stop_event.is_set():
                reserved = self._reserve_read_budget()
                if reserved == 0:
                    return

                try:
                    payload = self.upstream.recv(reserved)
                except BaseException:
                    self._restore_unused_budget(reserved, 0)
                    raise

                self.stats.server_read_calls += 1

                if not payload:
                    self._restore_unused_budget(reserved, 0)
                    self.upstream_eof = True
                    return

                self._restore_unused_budget(reserved, len(payload))
                self.stats.server_bytes_received += len(payload)

                self.send_data(payload)
                self.stats.channel_frames_sent += 1
                self.stats.channel_bytes_sent += len(payload)

        except BaseException as exc:
            if not self.stop_event.is_set():
                self._record_error(exc)

    def check(self) -> None:
        if self.error is not None:
            raise RfbBridgeError("RFB bridge worker failed") from self.error

    def stop(self) -> None:
        self.stop_event.set()
        with self.condition:
            self.condition.notify_all()

        # Unblock a worker currently inside upstream recv(). The socket is owned
        # by this bridge, so shutdown is part of deterministic bridge teardown.
        try:
            self.upstream.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass

        if self.thread is not None:
            self.thread.join(timeout=2.0)
            if self.thread.is_alive():
                raise RfbBridgeError("RFB bridge worker did not stop")

        try:
            self.upstream.close()
        except OSError:
            pass
