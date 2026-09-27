#!/usr/bin/env python3
"""File synopsis:
Composes the ordinary Raspberry Pi product Wire process from accepted narrow
owners without taking Wire, RFB, or MPEG mechanism ownership.

The selected R14 RFB profile supplies one fresh R13 attachment factory. R25 adds
one fresh exact-session R17 MPEG-generation factory using the selected Pi MPEG
composition profile plus the existing Configuration-owned producer profile.
R42 adds one selected lazy R39 AUDIO factory. All remain inert until WireServer
crosses their accepted lifecycle edges: RFB provider contact begins only on
valid post-Q4 RFB CREDIT, FFmpeg creation begins only on exact R17 START, and
pw-record capture begins only on the first exact channel-2 CREDIT.

This composition layer owns no Wire Session identity, physical I/O, provider
retry/recovery policy, generation/capture mechanics, CONFIG delivery, or PS2
product-trigger policy.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15;
A003-PI-MPEG-ORDINARY-PRODUCT-COMPOSITION-R25;
A006-ORDINARY-CROSS-PLATFORM-AUDIO-COMPOSITION-R42.
"""

from __future__ import annotations

import argparse
import sys
from typing import Callable

import audio_product_profile
import mpeg_generation as mpeg
import mpeg_product_profile
import mpeg_runtime_profile
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


def _make_mpeg_generation(
    session_id: int,
    composition: mpeg_product_profile.MpegProductCompositionProfile,
    producer_profile: mpeg_runtime_profile.MpegProducerProfile,
) -> mpeg.MpegGenerationController:
    """Create one fresh R17 owner bound to exact accepted Wire authority."""

    return mpeg.MpegGenerationController(
        session_id=session_id,
        desktop_width=composition.desktop_width,
        desktop_height=composition.desktop_height,
        display=composition.display,
        retirement_timeout_seconds=composition.retirement_timeout_seconds,
        profile=producer_profile,
    )


def selected_mpeg_generation_factory() -> (
    Callable[[int], mpeg.MpegGenerationController]
):
    """Project selected product facts into a fresh exact-session R17 factory."""

    composition = mpeg_product_profile.selected_mpeg_product_profile()
    producer_profile = mpeg_runtime_profile.selected_mpeg_producer_profile()

    def make_generation(session_id: int) -> mpeg.MpegGenerationController:
        return _make_mpeg_generation(
            session_id,
            composition,
            producer_profile,
        )

    return make_generation


def build_product_wire_server(
    listen_address: str = wire_server.DEFAULT_LISTEN_ADDRESS,
    port: int = wire_server.DEFAULT_LISTEN_PORT,
) -> wire_server.WireServer:
    """Build the ordinary supervised product server before any listener opens."""

    attachment_factory = selected_rfb_attachment_factory()
    mpeg_generation_factory = selected_mpeg_generation_factory()
    audio_pcm_factory = audio_product_profile.selected_audio_pcm_factory()
    return wire_server.WireServer(
        listen_address,
        port,
        rfb_attachment_factory=attachment_factory,
        mpeg_generation_factory=mpeg_generation_factory,
        audio_pcm_factory=audio_pcm_factory,
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
