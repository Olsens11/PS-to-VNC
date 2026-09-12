#!/usr/bin/env python3
"""
File synopsis:
    Receives and validates the CP2P MPEG START control message on the existing
    Pi-side PSTV reader path without creating another socket or reader thread.

The PS2 wire authority is the fixed 44-byte, eleven-big-endian-word START
payload carried as PSTV DATA on logical MPEG2 channel 4. This module validates
that representation against the active H1 session id and Pi-owned desktop
geometry, then retains exactly one immutable prepared START request for a later
MPEG activation step.

This item-#5 seam is intentionally inert beyond receive/validation/state:
    - it does not suppress RFB;
    - it does not alter capture geometry;
    - it does not launch ffmpeg or emit MPEG bytes;
    - it does not create a second physical PSTV receive owner.

Installation wraps whatever receive_frame owner is already active. In CP2P the
RFB adapter is attached first, so channel-1 frames continue to be consumed by
that adapter and channel-4 START reaches this wrapper on the same H1 reader
thread. Non-START frames pass through unchanged to H1Session.reader().
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
import json
from pathlib import Path
import struct
import threading

import h1_mux_server as base

MPEG_START_WIRE_VERSION = 1
MPEG_START_WIRE_WORDS = 11
MPEG_START_WIRE_BYTES = MPEG_START_WIRE_WORDS * 4
INT_MAX = (1 << 31) - 1


@dataclass(frozen=True)
class H1Cp2pPreparedStart:
    """Immutable Pi-side representation of one validated PS2 START request."""

    session_id: int
    generation: int
    draw_x: int
    draw_y: int
    draw_width: int
    draw_height: int
    suppression_x: int
    suppression_y: int
    suppression_width: int
    suppression_height: int

    def to_dict(self) -> dict[str, int]:
        return {key: int(value) for key, value in asdict(self).items()}


class H1Cp2pPreparedStartState:
    """Retain one prepared generation and reject duplicate/stale replacement."""

    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._prepared: H1Cp2pPreparedStart | None = None
        self._highest_generation = 0
        self.prepared_event = threading.Event()

    def prepare(self, request: H1Cp2pPreparedStart) -> None:
        with self._lock:
            if self._prepared is not None:
                raise base.ProtocolError(
                    "MPEG START arrived while generation "
                    f"{self._prepared.generation} remains prepared"
                )
            if request.generation <= self._highest_generation:
                raise base.ProtocolError(
                    "stale MPEG START generation "
                    f"{request.generation} <= {self._highest_generation}"
                )
            self._prepared = request
            self._highest_generation = request.generation
            self.prepared_event.set()

    def peek(self) -> H1Cp2pPreparedStart | None:
        with self._lock:
            return self._prepared

    def release_exact(self, generation: int) -> H1Cp2pPreparedStart:
        """Future activation/lifecycle seam; release only the exact generation."""

        with self._lock:
            if self._prepared is None:
                raise base.ProtocolError("no MPEG START generation is prepared")
            if self._prepared.generation != int(generation):
                raise base.ProtocolError(
                    "prepared MPEG START generation mismatch "
                    f"prepared={self._prepared.generation} requested={generation}"
                )
            request = self._prepared
            self._prepared = None
            self.prepared_event.clear()
            return request


def _decode_nonnegative_int(raw: int, field: str) -> int:
    if raw > INT_MAX:
        raise base.ProtocolError(f"MPEG START {field} exceeds signed-int range")
    return int(raw)


def decode_mpeg_start_payload(
    payload: bytes,
    *,
    expected_session_id: int,
    desktop_width: int,
    desktop_height: int,
) -> H1Cp2pPreparedStart:
    """Decode the PS2's fixed START payload and apply Pi-side session/bounds checks."""

    if len(payload) != MPEG_START_WIRE_BYTES:
        raise base.ProtocolError(
            f"MPEG START length {len(payload)} != {MPEG_START_WIRE_BYTES}"
        )
    if desktop_width <= 0 or desktop_height <= 0:
        raise base.ProtocolError("MPEG START validation requires active desktop geometry")

    words = struct.unpack(">11I", payload)
    if words[0] != MPEG_START_WIRE_VERSION:
        raise base.ProtocolError(
            f"unsupported MPEG START version {words[0]}"
        )
    if words[1] != int(expected_session_id):
        raise base.ProtocolError(
            "MPEG START session mismatch "
            f"actual={words[1]} expected={int(expected_session_id)}"
        )

    generation = int(words[2])
    if generation == 0:
        raise base.ProtocolError("MPEG START generation must be nonzero")

    draw_x = _decode_nonnegative_int(words[3], "draw_x")
    draw_y = _decode_nonnegative_int(words[4], "draw_y")
    draw_width = _decode_nonnegative_int(words[5], "draw_width")
    draw_height = _decode_nonnegative_int(words[6], "draw_height")
    suppression_x = _decode_nonnegative_int(words[7], "suppression_x")
    suppression_y = _decode_nonnegative_int(words[8], "suppression_y")
    suppression_width = _decode_nonnegative_int(words[9], "suppression_width")
    suppression_height = _decode_nonnegative_int(words[10], "suppression_height")

    if (
        draw_width < 16
        or draw_height < 16
        or (draw_width & 15) != 0
        or (draw_height & 15) != 0
    ):
        raise base.ProtocolError(
            "MPEG START base rectangle must be at least 16x16 and 16-pixel aligned"
        )
    if suppression_width <= 0 or suppression_height <= 0:
        raise base.ProtocolError("MPEG START suppression rectangle must be non-empty")

    draw_right = draw_x + draw_width
    draw_bottom = draw_y + draw_height
    suppression_right = suppression_x + suppression_width
    suppression_bottom = suppression_y + suppression_height

    if not (
        suppression_x <= draw_x
        and suppression_y <= draw_y
        and suppression_right >= draw_right
        and suppression_bottom >= draw_bottom
    ):
        raise base.ProtocolError(
            "MPEG START suppression rectangle must contain the exact base rectangle"
        )

    if draw_right > desktop_width or draw_bottom > desktop_height:
        raise base.ProtocolError("MPEG START base rectangle exceeds active desktop")
    if suppression_right > desktop_width or suppression_bottom > desktop_height:
        raise base.ProtocolError("MPEG START suppression rectangle exceeds active desktop")

    return H1Cp2pPreparedStart(
        session_id=int(words[1]),
        generation=generation,
        draw_x=draw_x,
        draw_y=draw_y,
        draw_width=draw_width,
        draw_height=draw_height,
        suppression_x=suppression_x,
        suppression_y=suppression_y,
        suppression_width=suppression_width,
        suppression_height=suppression_height,
    )


_START_REGISTRY_LOCK = threading.Lock()
_START_BY_PSTV_SOCKET: dict[object, "H1Cp2pStartReceiver"] = {}
_SHIM_INSTALL_LOCK = threading.Lock()
_SHIM_INSTALLED = False
_DOWNSTREAM_RECEIVE_FRAME = None


class H1Cp2pStartReceiver:
    """Bind one H1 session to the channel-4 START receive/validation seam."""

    def __init__(self, session: base.H1Session) -> None:
        self.session = session
        self.state = H1Cp2pPreparedStartState()
        self.started = False

    def _handle_start_frame(self, frame: base.Frame) -> None:
        if frame.kind != base.FRAME_DATA or frame.channel != base.CHANNEL_MPEG2:
            raise base.ProtocolError("CP2P START receiver received a non-START frame")
        if frame.flags != 0:
            raise base.ProtocolError("MPEG START DATA flags must be zero")

        request = decode_mpeg_start_payload(
            frame.payload,
            expected_session_id=int(self.session.profile["session_id"]),
            desktop_width=int(self.session.profile["desktop_width"]),
            desktop_height=int(self.session.profile["desktop_height"]),
        )
        self.state.prepare(request)

        evidence_path = Path(self.session.evidence) / "mpeg_start_prepared.json"
        evidence_path.write_text(
            json.dumps(request.to_dict(), indent=2, sort_keys=True) + "\n"
        )
        print(
            "H1_CP2P_MPEG_START_PREPARED="
            + json.dumps(request.to_dict(), sort_keys=True),
            flush=True,
        )

    def start(self) -> None:
        if self.started:
            raise base.ProtocolError("CP2P START receiver already started")

        _install_receive_shim()
        with _START_REGISTRY_LOCK:
            if self.session.sock in _START_BY_PSTV_SOCKET:
                raise base.ProtocolError("CP2P START receiver already bound to PSTV socket")
            _START_BY_PSTV_SOCKET[self.session.sock] = self
        self.started = True

    def stop(self) -> None:
        if not self.started:
            return
        with _START_REGISTRY_LOCK:
            if _START_BY_PSTV_SOCKET.get(self.session.sock) is self:
                del _START_BY_PSTV_SOCKET[self.session.sock]
        self.started = False

    def peek_prepared(self) -> H1Cp2pPreparedStart | None:
        return self.state.peek()

    def release_prepared_exact(self, generation: int) -> H1Cp2pPreparedStart:
        return self.state.release_exact(generation)


def _receive_frame_with_start_dispatch(sock):
    """Consume START on the existing reader call chain; return all other frames."""

    while True:
        if _DOWNSTREAM_RECEIVE_FRAME is None:
            raise base.ProtocolError("CP2P START receive shim has no downstream owner")
        frame = _DOWNSTREAM_RECEIVE_FRAME(sock)

        with _START_REGISTRY_LOCK:
            receiver = _START_BY_PSTV_SOCKET.get(sock)

        if (
            receiver is None
            or frame.kind != base.FRAME_DATA
            or frame.channel != base.CHANNEL_MPEG2
        ):
            return frame

        session = receiver.session
        if frame.sequence != session.expected_rx_sequence:
            raise base.ProtocolError(
                "PS2 sequence mismatch "
                f"expected={session.expected_rx_sequence} actual={frame.sequence}"
            )
        session.expected_rx_sequence += 1

        receiver._handle_start_frame(frame)
        # START is complete. Stay in this same H1 reader call and receive the next
        # physical PSTV frame for the pre-existing dispatch chain/base reader.


def _install_receive_shim() -> None:
    """Wrap the already-active receive owner; CP2P calls this after RFB attach."""

    global _SHIM_INSTALLED, _DOWNSTREAM_RECEIVE_FRAME

    with _SHIM_INSTALL_LOCK:
        if _SHIM_INSTALLED:
            return
        _DOWNSTREAM_RECEIVE_FRAME = base.receive_frame
        base.receive_frame = _receive_frame_with_start_dispatch
        _SHIM_INSTALLED = True


def open_cp2p_start_receiver(session: base.H1Session) -> H1Cp2pStartReceiver:
    receiver = H1Cp2pStartReceiver(session)
    receiver.start()
    return receiver
