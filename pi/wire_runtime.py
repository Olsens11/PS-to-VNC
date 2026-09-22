#!/usr/bin/env python3
"""File synopsis:
Composes the ordinary Raspberry Pi product Wire process from accepted narrow
owners without taking Wire or RFB mechanism ownership.

The selected R14 RFB profile is resolved before the listener is constructed.
Semantic ON supplies WireServer with a factory that creates a fresh R13
RfbAttachment for each sequential Wire connection. Semantic OFF supplies no
factory, leaving Wire establishment-only. RfbAttachment construction is inert:
the R12 internal provider at 127.0.0.1:5900 is still contacted only when the
first valid post-Q4 nonzero channel-1 CREDIT reaches that attachment.

This composition layer owns no selected numeric tuning literals, Wire Session
identity, physical I/O, provider retry/recovery policy, CONFIG delivery, AUDIO,
or MPEG lifecycle.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15.
"""

from __future__ import annotations

import argparse
import sys
from typing import Callable

import rfb_attachment as rfb_attach
import rfb_runtime_profile
import wire_server


def _make_attachment(
    flow: rfb_attach.RfbFlowConfig,
) -> rfb_attach.RfbAttachment:
    """Create one fresh session-scoped R13 attachment from selected values."""

    return rfb_attach.RfbAttachment(flow)


def selected_rfb_attachment_factory() -> (
    Callable[[], rfb_attach.RfbAttachment] | None
):
    """Project R14 ON into a fresh-attachment factory; OFF remains absent."""

    flow = rfb_runtime_profile.selected_rfb_flow_config()
    if flow is None:
        return None

    def make_attachment() -> rfb_attach.RfbAttachment:
        return _make_attachment(flow)

    return make_attachment


def build_product_wire_server(
    listen_address: str = wire_server.DEFAULT_LISTEN_ADDRESS,
    port: int = wire_server.DEFAULT_LISTEN_PORT,
) -> wire_server.WireServer:
    """Build the ordinary supervised product server before any listener opens."""

    attachment_factory = selected_rfb_attachment_factory()
    return wire_server.WireServer(
        listen_address,
        port,
        rfb_attachment_factory=attachment_factory,
    )


def main() -> int:
    parser = argparse.ArgumentParser(
        description="PS-to-VNC composed product Wire runtime"
    )
    parser.add_argument(
        "--listen",
        default=wire_server.DEFAULT_LISTEN_ADDRESS,
    )
    parser.add_argument(
        "--port",
        type=int,
        default=wire_server.DEFAULT_LISTEN_PORT,
    )
    args = parser.parse_args()

    server = build_product_wire_server(args.listen, args.port)
    server.serve_forever()
    return 0


if __name__ == "__main__":
    sys.exit(main())
