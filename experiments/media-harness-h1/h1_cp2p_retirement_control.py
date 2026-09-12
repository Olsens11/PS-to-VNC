#!/usr/bin/env python3
"""
File synopsis:
    Defines CP2P item #11A's exact-generation retirement control payload.

RETIRE is one H1 experiment-local PSTV frame kind on control channel 0. The
payload is exactly three big-endian 32-bit words:

    version, session_id, generation

The PS2 sends this payload as a request. The Pi sends the identical payload back
only after exact-generation runtime cleanup succeeds. Direction plus each side's
strict pending-generation state distinguish request from completion; MPEG2
channel-4 DATA remains pure media.
"""

from __future__ import annotations

from dataclasses import dataclass
import struct

import h1_mux_server as base

MPEG_RETIRE_FRAME_KIND = 10
MPEG_RETIRE_WIRE_VERSION = 1
MPEG_RETIRE_WIRE_WORDS = 3
MPEG_RETIRE_WIRE_BYTES = MPEG_RETIRE_WIRE_WORDS * 4


@dataclass(frozen=True)
class H1Cp2pRetireControl:
    session_id: int
    generation: int


def decode_mpeg_retire_payload(
    payload: bytes,
    *,
    expected_session_id: int,
) -> H1Cp2pRetireControl:
    if len(payload) != MPEG_RETIRE_WIRE_BYTES:
        raise base.ProtocolError(
            f"MPEG RETIRE length {len(payload)} != {MPEG_RETIRE_WIRE_BYTES}"
        )

    version, session_id, generation = struct.unpack(">3I", payload)
    if version != MPEG_RETIRE_WIRE_VERSION:
        raise base.ProtocolError(f"unsupported MPEG RETIRE version {version}")
    if session_id != int(expected_session_id):
        raise base.ProtocolError(
            "MPEG RETIRE session mismatch "
            f"actual={session_id} expected={int(expected_session_id)}"
        )
    if generation == 0:
        raise base.ProtocolError("MPEG RETIRE generation must be nonzero")

    return H1Cp2pRetireControl(
        session_id=int(session_id),
        generation=int(generation),
    )


def encode_mpeg_retire_payload(session_id: int, generation: int) -> bytes:
    session_id = int(session_id)
    generation = int(generation)
    if session_id < 0 or session_id > 0xFFFFFFFF:
        raise base.ProtocolError("MPEG RETIRE session id is outside uint32 range")
    if generation <= 0 or generation > 0xFFFFFFFF:
        raise base.ProtocolError("MPEG RETIRE generation must be nonzero uint32")
    return struct.pack(
        ">3I",
        MPEG_RETIRE_WIRE_VERSION,
        session_id,
        generation,
    )
