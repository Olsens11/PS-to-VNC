#!/usr/bin/env python3
"""
File synopsis:
    Implements CP2P item #6: generation-scoped Pi-side RFB suppression without
    creating another VNC/PSTV connection, reader, framebuffer authority, or
    unbounded queue.

The ordinary CP2O bridge remains byte-transparent. CP2P substitutes this bridge
only in the CP2P runner. START installs a *pending* suppression rectangle. The
bridge activates that rectangle immediately before forwarding the first new RFB
FramebufferUpdateRequest after START. This deliberately lets any older request
already outstanding at calibration acceptance complete unchanged while the PS2
presentation owner is frozen in WAIT_FIRST_FRAME.

Once active, one upstream Raw FramebufferUpdate transaction at a time is parsed,
clipped around the suppression rectangle, and emitted as one correctly-counted
FramebufferUpdate. The transaction's Raw pixel budget is derived from the
CURRENT active desktop geometry:

    desktop_width * desktop_height * 2 bytes

There is no hard-coded 704x462/704x480 desktop assumption. Future display modes
receive their own bound from the active H1 profile. The fixed factor of two is
not a resolution assumption; it is the already-negotiated 16-bit GS555 RFB
pixel format.

Only Raw encoding is accepted while suppression is active, matching the clean
PS2 client's Raw-only SetEncodings contract. Non-framebuffer server messages are
preserved. Normal CP2O credit-driven byte forwarding is used until suppression
becomes active.
"""

from __future__ import annotations

from dataclasses import dataclass
import select
import socket
import struct
import threading
from typing import Callable

import h1_mux_server as mux_base
import h1_rfb_session_adapter as adapter_base
from h1_rfb_pi_bridge import H1RfbPiBridge, RfbBridgeError

RFB_SERVER_FRAMEBUFFER_UPDATE = 0
RFB_SERVER_SET_COLOR_MAP_ENTRIES = 1
RFB_SERVER_BELL = 2
RFB_SERVER_CUT_TEXT = 3

RFB_CLIENT_SET_PIXEL_FORMAT = 0
RFB_CLIENT_SET_ENCODINGS = 2
RFB_CLIENT_FRAMEBUFFER_UPDATE_REQUEST = 3
RFB_CLIENT_KEY_EVENT = 4
RFB_CLIENT_POINTER_EVENT = 5
RFB_CLIENT_CUT_TEXT = 6

RFB_ENCODING_RAW = 0
RFB_RAW_BYTES_PER_PIXEL = 2


@dataclass(frozen=True)
class H1Cp2pSuppressionRect:
    x: int
    y: int
    width: int
    height: int

    @property
    def right(self) -> int:
        return self.x + self.width

    @property
    def bottom(self) -> int:
        return self.y + self.height


@dataclass
class H1Cp2pSuppressionStats:
    installs: int = 0
    activations: int = 0
    filtered_updates: int = 0
    input_rectangles: int = 0
    output_rectangles: int = 0
    input_pixel_bytes: int = 0
    output_pixel_bytes: int = 0


class H1Cp2pRfbPiBridge(H1RfbPiBridge):
    """CP2P-only RFB bridge with one generation-scoped Raw update filter."""

    def __init__(
        self,
        upstream: socket.socket,
        *,
        queue_capacity: int,
        max_payload: int,
        send_data: Callable[[bytes], None],
        desktop_width: int,
        desktop_height: int,
        name: str = "h1-cp2p-rfb-upstream-reader",
    ) -> None:
        super().__init__(
            upstream,
            queue_capacity=queue_capacity,
            max_payload=max_payload,
            send_data=send_data,
            name=name,
        )

        self.desktop_width = int(desktop_width)
        self.desktop_height = int(desktop_height)
        if self.desktop_width <= 0 or self.desktop_height <= 0:
            raise ValueError("CP2P suppression requires positive active desktop geometry")
        if self.desktop_width > 0xFFFF or self.desktop_height > 0xFFFF:
            raise ValueError("CP2P suppression desktop exceeds RFB 16-bit geometry")

        self.desktop_pixel_budget = self.desktop_width * self.desktop_height
        self.desktop_raw_byte_budget = (
            self.desktop_pixel_budget * RFB_RAW_BYTES_PER_PIXEL
        )

        self.suppression_lock = threading.Lock()
        self.suppression_generation = 0
        self.suppression_rect: H1Cp2pSuppressionRect | None = None
        self.suppression_pending = False
        self.suppression_active = False
        self.client_parse_buffer = bytearray()
        self.suppression_stats = H1Cp2pSuppressionStats()

    def install_suppression(self, request) -> None:
        """Install one prepared START generation, pending the next new RFB request."""

        generation = int(request.generation)
        rect = H1Cp2pSuppressionRect(
            int(request.suppression_x),
            int(request.suppression_y),
            int(request.suppression_width),
            int(request.suppression_height),
        )

        if generation <= 0:
            raise mux_base.ProtocolError("RFB suppression generation must be nonzero")
        if rect.x < 0 or rect.y < 0 or rect.width <= 0 or rect.height <= 0:
            raise mux_base.ProtocolError("RFB suppression rectangle is invalid")
        if rect.right > self.desktop_width or rect.bottom > self.desktop_height:
            raise mux_base.ProtocolError(
                "RFB suppression rectangle exceeds current active desktop"
            )

        with self.suppression_lock:
            if self.suppression_generation != 0:
                raise mux_base.ProtocolError(
                    "RFB suppression already owns generation "
                    f"{self.suppression_generation}"
                )
            self.suppression_generation = generation
            self.suppression_rect = rect
            self.suppression_pending = True
            self.suppression_active = False
            self.client_parse_buffer.clear()
            self.suppression_stats.installs += 1

        with self.condition:
            self.condition.notify_all()

    def retire_suppression_exact(self, generation: int) -> None:
        """Generation-safe removal seam for later explicit retirement wiring."""

        with self.suppression_lock:
            if self.suppression_generation != int(generation):
                raise mux_base.ProtocolError(
                    "RFB suppression retirement generation mismatch "
                    f"active={self.suppression_generation} requested={int(generation)}"
                )
            self.suppression_generation = 0
            self.suppression_rect = None
            self.suppression_pending = False
            self.suppression_active = False
            self.client_parse_buffer.clear()

        with self.condition:
            self.condition.notify_all()

    def _suppression_state(self) -> tuple[bool, bool, H1Cp2pSuppressionRect | None]:
        with self.suppression_lock:
            return (
                self.suppression_pending,
                self.suppression_active,
                self.suppression_rect,
            )

    def _activate_pending_for_update_request(self) -> None:
        with self.suppression_lock:
            if not self.suppression_pending:
                return
            if self.suppression_rect is None or self.suppression_generation == 0:
                raise mux_base.ProtocolError("pending RFB suppression lacks generation state")
            self.suppression_pending = False
            self.suppression_active = True
            self.suppression_stats.activations += 1

        # Wake the server reader if it is waiting for raw-mode credit/data. It
        # will re-check active state before recv(), so the response to the request
        # being sent cannot escape the suppression parser.
        with self.condition:
            self.condition.notify_all()

    @staticmethod
    def _client_message_size(buffer: bytearray) -> int | None:
        if not buffer:
            return None

        message_type = buffer[0]
        if message_type == RFB_CLIENT_SET_PIXEL_FORMAT:
            return 20
        if message_type == RFB_CLIENT_SET_ENCODINGS:
            if len(buffer) < 4:
                return None
            count = struct.unpack(">H", buffer[2:4])[0]
            return 4 + count * 4
        if message_type == RFB_CLIENT_FRAMEBUFFER_UPDATE_REQUEST:
            return 10
        if message_type == RFB_CLIENT_KEY_EVENT:
            return 8
        if message_type == RFB_CLIENT_POINTER_EVENT:
            return 6
        if message_type == RFB_CLIENT_CUT_TEXT:
            if len(buffer) < 8:
                return None
            length = struct.unpack(">I", buffer[4:8])[0]
            return 8 + length
        raise RfbBridgeError(f"unsupported RFB client message type {message_type}")

    def accept_client_data(self, payload: bytes) -> None:
        """Forward client bytes; pending suppression activates on the next update request."""

        if not payload:
            raise RfbBridgeError("empty RFB client DATA payload")
        if len(payload) > self.max_payload:
            raise RfbBridgeError(
                f"RFB client DATA payload {len(payload)} exceeds {self.max_payload}"
            )

        self.check()
        pending, _, _ = self._suppression_state()
        if not pending:
            super().accept_client_data(payload)
            return

        # START is installed at a PS2 application-service boundary, therefore the
        # client stream is at a complete-message boundary when this buffer starts.
        self.stats.channel_frames_received += 1
        self.stats.channel_bytes_received += len(payload)
        self.client_parse_buffer.extend(payload)

        while self.client_parse_buffer:
            message_size = self._client_message_size(self.client_parse_buffer)
            if message_size is None or len(self.client_parse_buffer) < message_size:
                return

            message = bytes(self.client_parse_buffer[:message_size])
            del self.client_parse_buffer[:message_size]

            if message[0] == RFB_CLIENT_FRAMEBUFFER_UPDATE_REQUEST:
                # Set active BEFORE the request can reach the VNC server.
                self._activate_pending_for_update_request()

            try:
                with self.upstream_send_lock:
                    self.upstream.sendall(message)
            except BaseException as exc:
                self._record_error(exc)
                raise RfbBridgeError("failed writing PS2 RFB bytes to VNC server") from exc

            self.stats.server_write_calls += 1
            self.stats.server_bytes_sent += len(message)

            # Once activation occurs, bytes after this complete message can return
            # to the ordinary direct path; no partial client message is retained.
            pending, _, _ = self._suppression_state()
            if not pending and self.client_parse_buffer:
                remainder = bytes(self.client_parse_buffer)
                self.client_parse_buffer.clear()
                super().accept_client_data(remainder)
                return

    def _reserve_raw_read_budget_or_active(self) -> int:
        """Reserve raw-mode credit, or return -1 when suppression became active."""

        with self.condition:
            while (
                self.credit == 0
                and not self.stop_event.is_set()
                and self.error is None
            ):
                _, active, _ = self._suppression_state()
                if active:
                    return -1
                self.condition.wait(timeout=0.05)

            if self.stop_event.is_set() or self.error is not None:
                return 0

            _, active, _ = self._suppression_state()
            if active:
                return -1

            budget = min(self.credit, self.max_payload)
            self.credit -= budget
            return budget

    def _send_with_credit(self, payload: bytes) -> None:
        """Emit filtered bytes only as PS2 channel-1 receive credit permits."""

        offset = 0
        while offset < len(payload):
            with self.condition:
                while (
                    self.credit == 0
                    and not self.stop_event.is_set()
                    and self.error is None
                ):
                    self.condition.wait(timeout=0.1)

                if self.stop_event.is_set() or self.error is not None:
                    raise RfbBridgeError("RFB bridge stopped while emitting filtered update")

                chunk_size = min(self.credit, self.max_payload, len(payload) - offset)
                self.credit -= chunk_size

            chunk = payload[offset:offset + chunk_size]
            self.send_data(chunk)
            self.stats.channel_frames_sent += 1
            self.stats.channel_bytes_sent += len(chunk)
            offset += chunk_size

    def _recv_exact_upstream(self, count: int) -> bytes:
        data = bytearray()
        while len(data) < count:
            part = self.upstream.recv(count - len(data))
            self.stats.server_read_calls += 1
            if not part:
                self.upstream_eof = True
                raise RfbBridgeError("upstream VNC closed inside RFB server message")
            self.stats.server_bytes_received += len(part)
            data.extend(part)
        return bytes(data)

    @staticmethod
    def _subtract_rect(
        x: int,
        y: int,
        width: int,
        height: int,
        suppression: H1Cp2pSuppressionRect,
    ) -> list[tuple[int, int, int, int]]:
        right = x + width
        bottom = y + height
        ix0 = max(x, suppression.x)
        iy0 = max(y, suppression.y)
        ix1 = min(right, suppression.right)
        iy1 = min(bottom, suppression.bottom)

        if ix0 >= ix1 or iy0 >= iy1:
            return [(x, y, width, height)]

        pieces: list[tuple[int, int, int, int]] = []
        if y < iy0:
            pieces.append((x, y, width, iy0 - y))
        if iy1 < bottom:
            pieces.append((x, iy1, width, bottom - iy1))
        if x < ix0:
            pieces.append((x, iy0, ix0 - x, iy1 - iy0))
        if ix1 < right:
            pieces.append((ix1, iy0, right - ix1, iy1 - iy0))
        return pieces

    @staticmethod
    def _extract_raw_piece(
        raw: bytes,
        source_x: int,
        source_y: int,
        source_width: int,
        piece: tuple[int, int, int, int],
    ) -> bytes:
        piece_x, piece_y, piece_width, piece_height = piece
        row_stride = source_width * RFB_RAW_BYTES_PER_PIXEL
        column_offset = (piece_x - source_x) * RFB_RAW_BYTES_PER_PIXEL
        first_row = piece_y - source_y
        piece_row_bytes = piece_width * RFB_RAW_BYTES_PER_PIXEL
        out = bytearray(piece_row_bytes * piece_height)

        destination = 0
        for row in range(piece_height):
            source = (first_row + row) * row_stride + column_offset
            out[destination:destination + piece_row_bytes] = raw[
                source:source + piece_row_bytes
            ]
            destination += piece_row_bytes
        return bytes(out)

    def _read_filtered_framebuffer_update(
        self,
        first_byte: bytes,
        suppression: H1Cp2pSuppressionRect,
    ) -> bytes:
        update_tail = self._recv_exact_upstream(3)
        rectangle_count = struct.unpack(">H", update_tail[1:3])[0]
        output_rectangles: list[bytes] = []
        input_pixel_bytes = 0
        output_pixel_bytes = 0

        for _ in range(rectangle_count):
            header = self._recv_exact_upstream(12)
            x, y, width, height, encoding = struct.unpack(">HHHHi", header)
            if encoding != RFB_ENCODING_RAW:
                raise RfbBridgeError(
                    f"suppression filter received non-Raw encoding {encoding}"
                )
            if x + width > self.desktop_width or y + height > self.desktop_height:
                raise RfbBridgeError("RFB rectangle exceeds current active desktop")

            raw_bytes = width * height * RFB_RAW_BYTES_PER_PIXEL
            input_pixel_bytes += raw_bytes
            if input_pixel_bytes > self.desktop_raw_byte_budget:
                raise RfbBridgeError(
                    "one RFB update exceeds current-desktop Raw pixel budget "
                    f"budget={self.desktop_raw_byte_budget} bytes"
                )

            raw = self._recv_exact_upstream(raw_bytes)
            pieces = self._subtract_rect(x, y, width, height, suppression)
            for piece in pieces:
                piece_x, piece_y, piece_width, piece_height = piece
                piece_raw = self._extract_raw_piece(
                    raw,
                    x,
                    y,
                    width,
                    piece,
                )
                output_pixel_bytes += len(piece_raw)
                output_rectangles.append(
                    struct.pack(">HHHHi", piece_x, piece_y, piece_width, piece_height, 0)
                    + piece_raw
                )
                if len(output_rectangles) > 0xFFFF:
                    raise RfbBridgeError("filtered RFB update exceeds rectangle-count field")

        self.suppression_stats.filtered_updates += 1
        self.suppression_stats.input_rectangles += rectangle_count
        self.suppression_stats.output_rectangles += len(output_rectangles)
        self.suppression_stats.input_pixel_bytes += input_pixel_bytes
        self.suppression_stats.output_pixel_bytes += output_pixel_bytes

        return (
            first_byte
            + bytes((update_tail[0],))
            + struct.pack(">H", len(output_rectangles))
            + b"".join(output_rectangles)
        )

    def _read_one_filtered_server_message(
        self,
        suppression: H1Cp2pSuppressionRect,
    ) -> bytes:
        first = self._recv_exact_upstream(1)
        message_type = first[0]

        if message_type == RFB_SERVER_FRAMEBUFFER_UPDATE:
            return self._read_filtered_framebuffer_update(first, suppression)
        if message_type == RFB_SERVER_BELL:
            return first
        if message_type == RFB_SERVER_SET_COLOR_MAP_ENTRIES:
            header = self._recv_exact_upstream(5)
            color_count = struct.unpack(">H", header[3:5])[0]
            return first + header + self._recv_exact_upstream(color_count * 6)
        if message_type == RFB_SERVER_CUT_TEXT:
            header = self._recv_exact_upstream(7)
            text_length = struct.unpack(">I", header[3:7])[0]
            # Clipboard text is not framebuffer data, but keep its temporary
            # buffering bounded relative to the CURRENT desktop rather than a
            # hidden resolution constant.
            text_budget = max(65536, self.desktop_raw_byte_budget)
            if text_length > text_budget:
                raise RfbBridgeError(
                    f"RFB ServerCutText length {text_length} exceeds bounded budget"
                )
            return first + header + self._recv_exact_upstream(text_length)

        raise RfbBridgeError(f"unsupported RFB server message type {message_type}")

    def _server_reader(self) -> None:
        try:
            while not self.stop_event.is_set():
                _, active, suppression = self._suppression_state()
                if active:
                    if suppression is None:
                        raise RfbBridgeError("active RFB suppression lacks rectangle")
                    message = self._read_one_filtered_server_message(suppression)
                    self._send_with_credit(message)
                    continue

                reserved = self._reserve_raw_read_budget_or_active()
                if reserved == 0:
                    return
                if reserved < 0:
                    continue

                # Unlike the legacy bridge's permanently blocking recv(), use a
                # short readiness wait so a START-triggered activation can take
                # ownership before the response to its new update request arrives.
                readable, _, _ = select.select([self.upstream], [], [], 0.05)
                if not readable:
                    self._restore_unused_budget(reserved, 0)
                    continue

                _, active, _ = self._suppression_state()
                if active:
                    self._restore_unused_budget(reserved, 0)
                    continue

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


class H1Cp2pRfbSessionAdapter(adapter_base.H1RfbSessionAdapter):
    """Existing H1 RFB adapter lifecycle with the CP2P-only bridge substituted."""

    def __init__(self, session: mux_base.H1Session, upstream: socket.socket) -> None:
        # Reuse the proven adapter's validation/state construction first. Its
        # ordinary bridge has not been started yet, so replacing it here has no
        # socket/thread side effect.
        super().__init__(session, upstream)
        self.bridge = H1Cp2pRfbPiBridge(
            upstream,
            queue_capacity=int(session.profile["rfb_queue_capacity"]),
            max_payload=int(session.profile["max_data_payload"]),
            send_data=self._send_server_data_to_ps2,
            desktop_width=int(session.profile["desktop_width"]),
            desktop_height=int(session.profile["desktop_height"]),
        )


def open_cp2p_rfb_session_adapter(
    session: mux_base.H1Session,
    *,
    host: str = "127.0.0.1",
    port: int = 5900,
    connector: Callable[[str, int], socket.socket] = adapter_base._default_vnc_connector,
) -> H1Cp2pRfbSessionAdapter | None:
    """Open the normal single upstream VNC connection with CP2P suppression support."""

    mode = int(session.profile["rfb_mode"])
    if mode == adapter_base.RFB_OFF:
        return None
    if not adapter_base._rfb_mode_is_enabled(mode):
        raise mux_base.ProtocolError(f"unsupported Pi RFB mode {mode}")
    if not host:
        raise mux_base.ProtocolError("RFB upstream host must be non-empty")
    if port <= 0 or port > 65535:
        raise mux_base.ProtocolError(f"invalid RFB upstream port {port}")

    upstream = connector(host, port)
    try:
        upstream.settimeout(None)
        adapter = H1Cp2pRfbSessionAdapter(session, upstream)
        adapter.start()
    except BaseException:
        try:
            upstream.close()
        except OSError:
            pass
        raise
    return adapter
