#!/usr/bin/env python3
"""
File synopsis:
    Attaches the credit-driven Pi RFB byte bridge to the existing H1Session
    receive path without replacing or duplicating the qualified media reader.

The adapter is deliberately experiment-local.  H1Session remains the sole
owner of the physical PSTV socket and its existing reader thread remains the
only code path that calls receive_frame() for that socket.  This module installs
one narrow receive-frame shim which consumes only logical channel-1 RFB frames;
all other frames are returned unchanged to H1Session.reader().

For intercepted channel-1 frames the shim performs the same global PSTV receive
sequence check/increment that H1Session.reader() would otherwise perform before
routing the frame to the raw-byte bridge.  This lets RFB CREDIT/DATA interleave
with AUDIO/MPEG/control frames without copying the monolithic reader logic.

RFB-OFF is genuinely inert: open_rfb_session_adapter() returns None before
opening an upstream VNC socket or installing the shim.  RFB-ON is still blocked
by the PS2 CONFIG validator at the current checkpoint, so this component is
host-testable preparation only and does not activate hardware behavior.
"""

from __future__ import annotations

import socket
import struct
import threading
from typing import Callable

import h1_mux_server as base
from h1_rfb_pi_bridge import H1RfbPiBridge

CHANNEL_RFB = 1
RFB_OFF = 0
RFB_ON_RESERVED = 1

_RFB_REGISTRY_LOCK = threading.Lock()
_RFB_BY_PSTV_SOCKET: dict[socket.socket, "H1RfbSessionAdapter"] = {}
_SHIM_INSTALL_LOCK = threading.Lock()
_SHIM_INSTALLED = False
_ORIGINAL_RECEIVE_FRAME = base.receive_frame


class H1RfbSessionAdapter:
    """Bind one H1Session to one upstream VNC socket for logical channel 1."""

    def __init__(self, session: base.H1Session, upstream: socket.socket) -> None:
        self.session = session
        self.upstream = upstream
        self.started = False

        if int(session.profile["rfb_mode"]) != RFB_ON_RESERVED:
            raise base.ProtocolError("RFB session adapter requires rfb_mode=1")

        queue_capacity = int(session.profile["rfb_queue_capacity"])
        max_payload = int(session.profile["max_data_payload"])
        if queue_capacity <= 0:
            raise base.ProtocolError("RFB session requires a positive queue capacity")
        if max_payload <= 0:
            raise base.ProtocolError("RFB session requires a positive max_data_payload")

        self.bridge = H1RfbPiBridge(
            upstream,
            queue_capacity=queue_capacity,
            max_payload=max_payload,
            send_data=self._send_server_data_to_ps2,
        )

    def _send_server_data_to_ps2(self, payload: bytes) -> None:
        """Serialize one credited VNC-server fragment as PSTV DATA channel 1."""

        self.session.send_frame(base.FRAME_DATA, CHANNEL_RFB, payload)

    def handle_rfb_frame(self, frame: base.Frame) -> None:
        """Handle one already-sequence-validated PS2->Pi channel-1 frame."""

        if frame.channel != CHANNEL_RFB:
            raise base.ProtocolError("RFB adapter received a non-RFB channel")
        if frame.flags != 0:
            raise base.ProtocolError("RFB DATA/CREDIT flags must be zero")

        if frame.kind == base.FRAME_CREDIT:
            if len(frame.payload) != base.CREDIT_BYTES:
                raise base.ProtocolError("invalid RFB CREDIT payload")
            amount = struct.unpack(">I", frame.payload)[0]
            self.bridge.add_credit(amount)
            return

        if frame.kind == base.FRAME_DATA:
            if not frame.payload:
                raise base.ProtocolError("empty RFB DATA payload")
            if len(frame.payload) > int(self.session.profile["max_data_payload"]):
                raise base.ProtocolError("RFB DATA exceeds configured max_data_payload")
            self.bridge.accept_client_data(frame.payload)
            return

        raise base.ProtocolError(
            f"unsupported RFB frame kind={frame.kind} channel={frame.channel}"
        )

    def start(self) -> None:
        """Register before the bridge worker starts so initial CREDIT cannot race."""

        if self.started:
            raise base.ProtocolError("RFB session adapter already started")

        _install_receive_shim()
        with _RFB_REGISTRY_LOCK:
            if self.session.sock in _RFB_BY_PSTV_SOCKET:
                raise base.ProtocolError("RFB adapter already bound to PSTV socket")
            _RFB_BY_PSTV_SOCKET[self.session.sock] = self

        try:
            self.bridge.start()
        except BaseException:
            with _RFB_REGISTRY_LOCK:
                if _RFB_BY_PSTV_SOCKET.get(self.session.sock) is self:
                    del _RFB_BY_PSTV_SOCKET[self.session.sock]
            raise

        self.started = True

    def check(self) -> None:
        self.bridge.check()

    def stop(self) -> None:
        """Detach after the owning H1 reader has stopped, then close upstream VNC."""

        if not self.started:
            try:
                self.upstream.close()
            except OSError:
                pass
            return

        with _RFB_REGISTRY_LOCK:
            if _RFB_BY_PSTV_SOCKET.get(self.session.sock) is self:
                del _RFB_BY_PSTV_SOCKET[self.session.sock]

        self.bridge.stop()
        self.started = False


def _receive_frame_with_rfb_dispatch(sock: socket.socket) -> base.Frame:
    """Run inside H1Session.reader(); consume only registered channel-1 frames."""

    while True:
        frame = _ORIGINAL_RECEIVE_FRAME(sock)

        with _RFB_REGISTRY_LOCK:
            adapter = _RFB_BY_PSTV_SOCKET.get(sock)

        if adapter is None or frame.channel != CHANNEL_RFB:
            return frame

        session = adapter.session
        if frame.sequence != session.expected_rx_sequence:
            raise base.ProtocolError(
                "PS2 sequence mismatch "
                f"expected={session.expected_rx_sequence} actual={frame.sequence}"
            )
        session.expected_rx_sequence += 1

        adapter.handle_rfb_frame(frame)
        adapter.check()
        # The channel-1 frame is complete.  Stay on the same H1 reader thread
        # and receive the next physical PSTV frame for normal base dispatch.


def _install_receive_shim() -> None:
    global _SHIM_INSTALLED

    with _SHIM_INSTALL_LOCK:
        if _SHIM_INSTALLED:
            return
        if base.receive_frame is not _ORIGINAL_RECEIVE_FRAME:
            raise base.ProtocolError(
                "cannot install RFB receive shim over an unknown receive_frame owner"
            )
        base.receive_frame = _receive_frame_with_rfb_dispatch
        _SHIM_INSTALLED = True


def _default_vnc_connector(host: str, port: int) -> socket.socket:
    return socket.create_connection((host, port), timeout=5.0)


def open_rfb_session_adapter(
    session: base.H1Session,
    *,
    host: str = "127.0.0.1",
    port: int = 5900,
    connector: Callable[[str, int], socket.socket] = _default_vnc_connector,
) -> H1RfbSessionAdapter | None:
    """Open/attach upstream VNC only for an explicitly RFB-enabled profile."""

    mode = int(session.profile["rfb_mode"])
    if mode == RFB_OFF:
        return None
    if mode != RFB_ON_RESERVED:
        raise base.ProtocolError(f"unsupported Pi RFB mode {mode}")
    if not host:
        raise base.ProtocolError("RFB upstream host must be non-empty")
    if port <= 0 or port > 65535:
        raise base.ProtocolError(f"invalid RFB upstream port {port}")

    upstream = connector(host, port)
    adapter = H1RfbSessionAdapter(session, upstream)
    try:
        adapter.start()
    except BaseException:
        try:
            upstream.close()
        except OSError:
            pass
        raise
    return adapter
