#!/usr/bin/env python3
"""File synopsis:
Defines the Raspberry Pi product-side PSTV Wire framing and provisional
establishment representation shared conceptually with src/transport/protocol.*.

This module owns bytes only: fixed headers, HELLO, ACCEPT, NOT_ACCEPTED, exact
RFB DATA/CREDIT framing, envelope classification, and unsigned-field
validation. It owns no sockets, listener/session lifecycle, rider dispatch, MPEG
producer state, RFB provider lifecycle, reconnect policy, or systemd behavior.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-WIRE-SERVER-ESTABLISHMENT-R8.
"""

from __future__ import annotations

from dataclasses import dataclass
import struct

# Every PSTV frame starts with the same 16-byte network-order header:
# magic, header version, kind, logical channel, flags, sequence, payload length.
# The header version describes framing itself; WIRE_VERSION below is the
# separately negotiated product Wire compatibility version carried by HELLO.
MAGIC = b"PSTV"
HEADER = struct.Struct(">4sBBBBII")
HEADER_BYTES = HEADER.size
WIRE_HEADER_VERSION = 1
MAX_PAYLOAD_BYTES = 8192

# Q4 establishment frame identities are now product protocol authority. They
# deliberately occupy the control channel and do not imply any rider exists.
FRAME_HELLO = 1
FRAME_DATA = 3
FRAME_CREDIT = 4
FRAME_ACCEPT = 12
FRAME_NOT_ACCEPTED = 13

CHANNEL_CONTROL = 0
CHANNEL_RFB = 1

# HELLO carries two independent compatibility words. WIRE_VERSION covers the
# current PSTV Wire contract; PRODUCT_ESTABLISHMENT_VERSION lets establishment
# semantics evolve without changing the fixed frame-header representation.
WIRE_VERSION = 1
PRODUCT_ESTABLISHMENT_VERSION = 1

# HELLO is exactly two uint32 values. ACCEPT and NOT_ACCEPTED each carry one
# uint32 value: a nonzero Pi-owned session ID or a bounded rejection reason.
HELLO = struct.Struct(">II")
ONE_WORD = struct.Struct(">I")
CREDIT = ONE_WORD

REJECT_WIRE_VERSION = 1
REJECT_PRODUCT_VERSION = 2
REJECT_MALFORMED = 3

UINT32_MAX = 0xFFFFFFFF


class WireProtocolError(ValueError):
    """Reject malformed or unrepresentable product Wire bytes."""


@dataclass(frozen=True)
class WireHeader:
    version: int
    kind: int
    channel: int
    flags: int
    sequence: int
    payload_length: int


def _require_u32(value: int, name: str) -> None:
    if not isinstance(value, int) or value < 0 or value > UINT32_MAX:
        raise WireProtocolError(f"{name} is outside uint32 range")


def encode_header(header: WireHeader) -> bytes:
    _require_u32(header.sequence, "sequence")
    _require_u32(header.payload_length, "payload_length")
    if header.version != WIRE_HEADER_VERSION:
        raise WireProtocolError("unsupported Wire header version")
    if header.payload_length > MAX_PAYLOAD_BYTES:
        raise WireProtocolError("payload exceeds product Wire maximum")
    for name, value in (
        ("kind", header.kind),
        ("channel", header.channel),
        ("flags", header.flags),
    ):
        if not isinstance(value, int) or value < 0 or value > 0xFF:
            raise WireProtocolError(f"{name} is outside uint8 range")
    return HEADER.pack(
        MAGIC,
        header.version,
        header.kind,
        header.channel,
        header.flags,
        header.sequence,
        header.payload_length,
    )


def decode_header(data: bytes) -> WireHeader:
    if len(data) != HEADER_BYTES:
        raise WireProtocolError("Wire header must be exactly 16 bytes")
    magic, version, kind, channel, flags, sequence, payload_length = HEADER.unpack(data)
    if magic != MAGIC:
        raise WireProtocolError("Wire magic mismatch")
    if version != WIRE_HEADER_VERSION:
        raise WireProtocolError("unsupported Wire header version")
    if payload_length > MAX_PAYLOAD_BYTES:
        raise WireProtocolError("payload exceeds product Wire maximum")
    return WireHeader(
        version=version,
        kind=kind,
        channel=channel,
        flags=flags,
        sequence=sequence,
        payload_length=payload_length,
    )


def encode_channel_frame(
    kind: int,
    channel: int,
    sequence: int,
    payload: bytes,
) -> bytes:
    """Encode one exact zero-flags PSTV frame for a selected logical channel."""

    header = WireHeader(
        version=WIRE_HEADER_VERSION,
        kind=kind,
        channel=channel,
        flags=0,
        sequence=sequence,
        payload_length=len(payload),
    )
    return encode_header(header) + payload


def encode_frame(kind: int, sequence: int, payload: bytes) -> bytes:
    # Establishment helpers use the control channel so Q4 identity cannot drift
    # independently between frame kinds.
    return encode_channel_frame(
        kind,
        CHANNEL_CONTROL,
        sequence,
        payload,
    )


def encode_hello_payload(
    wire_version: int = WIRE_VERSION,
    product_version: int = PRODUCT_ESTABLISHMENT_VERSION,
) -> bytes:
    _require_u32(wire_version, "wire_version")
    _require_u32(product_version, "product_establishment_version")
    return HELLO.pack(wire_version, product_version)


def decode_hello_payload(payload: bytes) -> tuple[int, int]:
    if len(payload) != HELLO.size:
        raise WireProtocolError("HELLO payload must be exactly 8 bytes")
    return HELLO.unpack(payload)


def encode_accept_payload(session_id: int) -> bytes:
    _require_u32(session_id, "session_id")
    # Zero is reserved to mean "no active Wire Session"; ACCEPT may therefore
    # publish only a Pi-authoritative nonzero identity.
    if session_id == 0:
        raise WireProtocolError("ACCEPT session_id must be nonzero")
    return ONE_WORD.pack(session_id)


def decode_accept_payload(payload: bytes) -> int:
    if len(payload) != ONE_WORD.size:
        raise WireProtocolError("ACCEPT payload must be exactly 4 bytes")
    session_id = ONE_WORD.unpack(payload)[0]
    if session_id == 0:
        raise WireProtocolError("ACCEPT session_id must be nonzero")
    return session_id


def _require_rejection_reason(reason: int) -> None:
    if reason not in (
        REJECT_WIRE_VERSION,
        REJECT_PRODUCT_VERSION,
        REJECT_MALFORMED,
    ):
        raise WireProtocolError("unknown NOT_ACCEPTED reason")


def encode_not_accepted_payload(reason: int) -> bytes:
    _require_rejection_reason(reason)
    return ONE_WORD.pack(reason)


def decode_not_accepted_payload(payload: bytes) -> int:
    if len(payload) != ONE_WORD.size:
        raise WireProtocolError("NOT_ACCEPTED payload must be exactly 4 bytes")
    reason = ONE_WORD.unpack(payload)[0]
    _require_rejection_reason(reason)
    return reason


def encode_hello_frame(
    wire_version: int = WIRE_VERSION,
    product_version: int = PRODUCT_ESTABLISHMENT_VERSION,
    sequence: int = 1,
) -> bytes:
    # Sequence 1 is the provisional transaction's first client->server frame.
    # Later ordinary Wire traffic, once implemented, begins from the next
    # direction-local sequence rather than reusing this establishment slot.
    return encode_frame(
        FRAME_HELLO,
        sequence,
        encode_hello_payload(wire_version, product_version),
    )


def encode_accept_frame(session_id: int, sequence: int = 1) -> bytes:
    return encode_frame(FRAME_ACCEPT, sequence, encode_accept_payload(session_id))


def encode_not_accepted_frame(reason: int, sequence: int = 1) -> bytes:
    return encode_frame(
        FRAME_NOT_ACCEPTED,
        sequence,
        encode_not_accepted_payload(reason),
    )


def encode_rfb_credit_payload(amount: int) -> bytes:
    _require_u32(amount, "RFB credit")
    if amount == 0:
        raise WireProtocolError("RFB credit must be nonzero")
    return CREDIT.pack(amount)


def decode_rfb_credit_payload(payload: bytes) -> int:
    if len(payload) != CREDIT.size:
        raise WireProtocolError("RFB CREDIT payload must be exactly 4 bytes")
    amount = CREDIT.unpack(payload)[0]
    if amount == 0:
        raise WireProtocolError("RFB credit must be nonzero")
    return amount


def encode_rfb_credit_frame(amount: int, sequence: int) -> bytes:
    return encode_channel_frame(
        FRAME_CREDIT,
        CHANNEL_RFB,
        sequence,
        encode_rfb_credit_payload(amount),
    )


def encode_rfb_data_frame(payload: bytes, sequence: int) -> bytes:
    if len(payload) > MAX_PAYLOAD_BYTES:
        raise WireProtocolError("RFB DATA payload exceeds product Wire maximum")
    return encode_channel_frame(
        FRAME_DATA,
        CHANNEL_RFB,
        sequence,
        payload,
    )


def is_rfb_credit_header(header: WireHeader) -> bool:
    return (
        header.kind == FRAME_CREDIT
        and header.channel == CHANNEL_RFB
        and header.flags == 0
        and header.payload_length == CREDIT.size
    )


def is_rfb_data_header(header: WireHeader) -> bool:
    # Zero-length DATA is intentionally valid framing but reserved for the
    # existing RFB quiesce lifecycle rather than raw provider bytes.
    return (
        header.kind == FRAME_DATA
        and header.channel == CHANNEL_RFB
        and header.flags == 0
        and header.payload_length <= MAX_PAYLOAD_BYTES
    )


def is_hello_header(header: WireHeader) -> bool:
    return (
        header.kind == FRAME_HELLO
        and header.channel == CHANNEL_CONTROL
        and header.flags == 0
        and header.payload_length == HELLO.size
    )


def is_accept_header(header: WireHeader) -> bool:
    return (
        header.kind == FRAME_ACCEPT
        and header.channel == CHANNEL_CONTROL
        and header.flags == 0
        and header.payload_length == ONE_WORD.size
    )


def is_not_accepted_header(header: WireHeader) -> bool:
    return (
        header.kind == FRAME_NOT_ACCEPTED
        and header.channel == CHANNEL_CONTROL
        and header.flags == 0
        and header.payload_length == ONE_WORD.size
    )
