#!/usr/bin/env python3
"""
File synopsis:
    Composes the cumulative through-Issue-39 H1 Pi runner with the verified raw
    RFB channel-1 session adapter, while leaving the PS2 RFB activation gate
    closed at this checkpoint.

This wrapper intentionally does not copy H1Session.reader(), media scheduling,
telemetry, capture geometry, or result validation.  It imports the current
cumulative runner first, subclasses its already-composed H1Session, and adds one
optional upstream VNC attachment during construction.

RFB-OFF behavior remains inert: no upstream VNC socket is opened and the RFB
receive shim is not installed.  RFB-ON opens exactly one upstream ordinary VNC
connection (default 127.0.0.1:5900) before H1Session.run() starts the sole PSTV
reader.  The PS2 still rejects RFB-ON today, so this is repository/host-tested
Pi orchestration preparation rather than a hardware-operational runner.

Temporary preparation-time upstream selection uses environment variables:
    H1_RFB_UPSTREAM_HOST  default 127.0.0.1
    H1_RFB_UPSTREAM_PORT  default 5900

The canonical h1_tool.py control surface is deliberately not redirected to this
wrapper until the PS2 activation checkpoint is ready.
"""

from __future__ import annotations

import os
import socket
from typing import Callable

import h1_mux_server_cumulative39_thread_census as cumulative
from h1_rfb_session_adapter import H1RfbSessionAdapter, open_rfb_session_adapter

base = cumulative.base
_ParentSession = base.H1Session

# Host tests may inject a socketpair connector.  Normal runner execution leaves
# this unset and uses the adapter's ordinary socket.create_connection path.
RFB_CONNECTOR: Callable[[str, int], socket.socket] | None = None


def _rfb_upstream() -> tuple[str, int]:
    host = os.environ.get("H1_RFB_UPSTREAM_HOST", "127.0.0.1")
    raw_port = os.environ.get("H1_RFB_UPSTREAM_PORT", "5900")
    try:
        port = int(raw_port, 0)
    except ValueError as exc:
        raise base.ProtocolError(
            f"invalid H1_RFB_UPSTREAM_PORT {raw_port!r}"
        ) from exc
    return host, port


class H1Cumulative39RfbBridgeSession(_ParentSession):
    """Current cumulative H1Session plus optional Pi-local RFB upstream."""

    def __init__(
        self,
        sock: socket.socket,
        profile: dict[str, int],
        evidence,
        duration: float,
        display: str,
    ) -> None:
        super().__init__(sock, profile, evidence, duration, display)
        self.rfb_session_adapter: H1RfbSessionAdapter | None = None

        if int(profile["rfb_mode"]) == 0:
            return

        host, port = _rfb_upstream()
        kwargs = {"host": host, "port": port}
        if RFB_CONNECTOR is not None:
            kwargs["connector"] = RFB_CONNECTOR

        self.rfb_session_adapter = open_rfb_session_adapter(self, **kwargs)
        if self.rfb_session_adapter is None:
            raise base.ProtocolError("RFB-ON profile failed to attach Pi bridge")

        print(
            f"H1_RFB_UPSTREAM_ATTACHED={host}:{port}",
            flush=True,
        )

    def cleanup(self) -> None:
        # Preserve lifecycle order: physical PSTV owner closes first through the
        # inherited cleanup, then the logical/upstream RFB bridge is detached.
        try:
            super().cleanup()
        finally:
            if self.rfb_session_adapter is not None:
                self.rfb_session_adapter.stop()
                self.rfb_session_adapter = None


base.H1Session = H1Cumulative39RfbBridgeSession


if __name__ == "__main__":
    raise SystemExit(base.main())
