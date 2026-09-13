#!/usr/bin/env python3
"""
File synopsis:
    Disposable CP2P RFB-transaction diagnostic runner layered on the existing
    all-guns MPEG-stall diagnostic runner.

Purpose:
    Preserve one PSTV socket, one PS2 receive owner, the exact CP2P media policy,
    and the existing generation/suppression behavior while adding observation at
    the RFB transaction boundary.

Pi-side evidence records:
    * suppression install/activation/retirement;
    * each post-activation FramebufferUpdate message and rectangle header;
    * cumulative input/output rectangle and pixel-byte deltas after filtering;
    * filtered payload send begin/end and available RFB credit;
    * periodic suppression/credit/stat snapshots.

PS2-side evidence comes from the diagnostic h1_rfb_mux_io.c on this branch. It
reuses otherwise-live-unused producer telemetry slots before MEDIA_END to report
whether the clean RFB parser is polling at a complete message boundary, blocked
inside one exact read (including requested byte count), or publishing one RFB
client message. MEDIA_END remains authoritative and stops those temporary writes.

Console output is intentionally throttled relative to the prior stall runner so
terminal scrollback/QuickEdit selection is less likely to backpressure SSH.
Detailed RFB evidence is written to JSONL files in the run evidence directory.
"""

from __future__ import annotations

import json
from pathlib import Path
import socket
import struct
import threading
import time
from typing import Callable

import h1_mux_server_cp2p_stall_diag as stall
import h1_mux_server_cp2p_start_receiver as cp2p
import h1_cp2p_rfb_suppression as suppression
import h1_rfb_session_adapter as adapter_base

base = stall.base

# Keep liveness probes useful but reduce terminal-output pressure. These globals
# are read by the inherited diagnostic methods at runtime.
stall._HEARTBEAT_PERIOD_S = 1.0
stall._WATCHDOG_PERIOD_S = 2.0

_RFB_SNAPSHOT_PERIOD_S = 1.0
_RFB_CONSOLE_PERIOD_S = 5.0

_RFB_IO_STAGE_NAMES = {
    0xE1010001: "RFB_READ_EXACT_ENTER",
    0xE1010002: "RFB_READ_EXACT_RETURN_OK",
    0xE10100FF: "RFB_READ_EXACT_RETURN_FAIL",
    0xE1020001: "RFB_POLL_ENTER",
    0xE1020002: "RFB_POLL_IDLE",
    0xE1020003: "RFB_POLL_READY",
    0xE10200FF: "RFB_POLL_FAIL",
}


def _decode_rfb_io_stage(word: int) -> str:
    word &= 0xFFFFFFFF
    if (word & 0xFFFF0000) == 0xE1030000:
        return f"RFB_WRITE_ENTER[type={word & 0xFF}]"
    if (word & 0xFFFF0000) == 0xE1040000:
        return f"RFB_WRITE_OK[type={word & 0xFF}]"
    if (word & 0xFFFF0000) == 0xE1FF0000:
        return f"RFB_WRITE_FAIL[type={word & 0xFF}]"
    return _RFB_IO_STAGE_NAMES.get(word, f"RFB_IO_OTHER_0x{word:08X}")


class H1Cp2pRfbTransactionBridge(suppression.H1Cp2pRfbPiBridge):
    """Exact CP2P suppression bridge plus observation-only transaction witnesses."""

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self._rfb_diag_lock = threading.Lock()
        self._rfb_diag_path: Path | None = None
        self._rfb_diag_sequence = 0
        self._rfb_diag_phase = "RAW_BRIDGE"
        self._rfb_diag_last_event = "INIT"
        self._rfb_diag_message_type = -1
        self._rfb_diag_rectangles_remaining = 0
        self._rfb_diag_rectangle_index = 0
        self._rfb_diag_current_rectangle: dict[str, int] | None = None
        self._rfb_diag_filtered_send_bytes = 0

    def attach_diagnostic_evidence(self, evidence: Path) -> None:
        self._rfb_diag_path = Path(evidence) / "rfb_transaction_events.jsonl"
        self._diag_event("EVIDENCE_ATTACHED")

    def _diag_event(self, event: str, **fields) -> None:
        now = time.monotonic()
        with self._rfb_diag_lock:
            self._rfb_diag_sequence += 1
            self._rfb_diag_last_event = event
            record = {
                "pi_monotonic": now,
                "sequence": self._rfb_diag_sequence,
                "event": event,
                "phase": self._rfb_diag_phase,
            }
            record.update(fields)
            path = self._rfb_diag_path

        if path is not None:
            with path.open("a", encoding="utf-8") as handle:
                handle.write(json.dumps(record, sort_keys=True) + "\n")

    def diagnostic_snapshot(self) -> dict[str, object]:
        pending, active, rect = self._suppression_state()
        with self.condition:
            credit = int(self.credit)
        with self._rfb_diag_lock:
            snapshot = {
                "event_sequence": self._rfb_diag_sequence,
                "last_event": self._rfb_diag_last_event,
                "phase": self._rfb_diag_phase,
                "message_type": self._rfb_diag_message_type,
                "rectangles_remaining": self._rfb_diag_rectangles_remaining,
                "rectangle_index": self._rfb_diag_rectangle_index,
                "current_rectangle": self._rfb_diag_current_rectangle,
                "filtered_send_bytes": self._rfb_diag_filtered_send_bytes,
            }

        snapshot.update(
            {
                "generation": int(self.suppression_generation),
                "suppression_pending": bool(pending),
                "suppression_active": bool(active),
                "suppression_rect": (
                    {
                        "x": rect.x,
                        "y": rect.y,
                        "width": rect.width,
                        "height": rect.height,
                    }
                    if rect is not None
                    else None
                ),
                "rfb_credit": credit,
                "filtered_updates": int(self.suppression_stats.filtered_updates),
                "input_rectangles": int(self.suppression_stats.input_rectangles),
                "output_rectangles": int(self.suppression_stats.output_rectangles),
                "input_pixel_bytes": int(self.suppression_stats.input_pixel_bytes),
                "output_pixel_bytes": int(self.suppression_stats.output_pixel_bytes),
                "channel_frames_sent": int(self.stats.channel_frames_sent),
                "channel_bytes_sent": int(self.stats.channel_bytes_sent),
                "channel_frames_received": int(self.stats.channel_frames_received),
                "channel_bytes_received": int(self.stats.channel_bytes_received),
                "server_read_calls": int(self.stats.server_read_calls),
                "server_bytes_received": int(self.stats.server_bytes_received),
                "server_write_calls": int(self.stats.server_write_calls),
                "server_bytes_sent": int(self.stats.server_bytes_sent),
            }
        )
        return snapshot

    def install_suppression(self, request) -> None:
        self._diag_event(
            "SUPPRESSION_INSTALL_BEGIN",
            generation=int(request.generation),
            x=int(request.suppression_x),
            y=int(request.suppression_y),
            width=int(request.suppression_width),
            height=int(request.suppression_height),
        )
        super().install_suppression(request)
        self._diag_event("SUPPRESSION_INSTALL_COMPLETE", generation=int(request.generation))

    def retire_suppression_exact(self, generation: int) -> None:
        self._diag_event("SUPPRESSION_RETIRE_BEGIN", generation=int(generation))
        super().retire_suppression_exact(generation)
        self._diag_event("SUPPRESSION_RETIRE_COMPLETE", generation=int(generation))

    def _activate_pending_for_update_request(self) -> None:
        generation = int(self.suppression_generation)
        self._diag_event("SUPPRESSION_ACTIVATE_BEGIN", generation=generation)
        super()._activate_pending_for_update_request()
        self._rfb_diag_phase = "FILTERED_SERVER_MESSAGES"
        self._diag_event("SUPPRESSION_ACTIVATE_COMPLETE", generation=generation)

    def accept_client_data(self, payload: bytes) -> None:
        if payload:
            pending, active, _ = self._suppression_state()
            if payload[0] == suppression.RFB_CLIENT_FRAMEBUFFER_UPDATE_REQUEST or pending or active:
                self._diag_event(
                    "CLIENT_DATA",
                    first_message_type=int(payload[0]),
                    payload_bytes=len(payload),
                    suppression_pending=bool(pending),
                    suppression_active=bool(active),
                )
        super().accept_client_data(payload)

    def _read_one_filtered_server_message(
        self,
        suppression_rect: suppression.H1Cp2pSuppressionRect,
    ) -> bytes:
        self._rfb_diag_phase = "SERVER_MESSAGE_TYPE"
        self._rfb_diag_message_type = -1
        self._rfb_diag_rectangles_remaining = 0
        self._rfb_diag_rectangle_index = 0
        self._rfb_diag_current_rectangle = None
        self._diag_event("FILTERED_SERVER_MESSAGE_BEGIN")
        payload = super()._read_one_filtered_server_message(suppression_rect)
        self._rfb_diag_phase = "FILTERED_MESSAGE_READY_TO_SEND"
        self._diag_event(
            "FILTERED_SERVER_MESSAGE_COMPLETE",
            filtered_message_bytes=len(payload),
            message_type=self._rfb_diag_message_type,
        )
        return payload

    def _recv_exact_upstream(self, count: int) -> bytes:
        phase_before = self._rfb_diag_phase
        self._diag_event("UPSTREAM_EXACT_READ_BEGIN", requested_bytes=int(count))
        data = super()._recv_exact_upstream(count)

        if phase_before == "SERVER_MESSAGE_TYPE" and count == 1:
            self._rfb_diag_message_type = int(data[0])
            if data[0] == suppression.RFB_SERVER_FRAMEBUFFER_UPDATE:
                self._rfb_diag_phase = "UPDATE_HEADER"
            else:
                self._rfb_diag_phase = f"SERVER_MESSAGE_{int(data[0])}"
            self._diag_event("SERVER_MESSAGE_TYPE", message_type=int(data[0]))

        elif self._rfb_diag_phase == "UPDATE_HEADER" and count == 3:
            rectangle_count = struct.unpack(">H", data[1:3])[0]
            self._rfb_diag_rectangles_remaining = int(rectangle_count)
            self._rfb_diag_rectangle_index = 0
            self._rfb_diag_phase = (
                "RECTANGLE_HEADER" if rectangle_count else "UPDATE_EMPTY"
            )
            self._diag_event("UPDATE_HEADER", rectangle_count=int(rectangle_count))

        elif self._rfb_diag_phase == "RECTANGLE_HEADER" and count == 12:
            x, y, width, height, encoding = struct.unpack(">HHHHi", data)
            raw_bytes = int(width) * int(height) * suppression.RFB_RAW_BYTES_PER_PIXEL
            self._rfb_diag_current_rectangle = {
                "index": self._rfb_diag_rectangle_index,
                "x": int(x),
                "y": int(y),
                "width": int(width),
                "height": int(height),
                "encoding": int(encoding),
                "raw_bytes": raw_bytes,
            }
            self._rfb_diag_phase = "RECTANGLE_RAW"
            self._diag_event("RECTANGLE_HEADER", **self._rfb_diag_current_rectangle)

        elif self._rfb_diag_phase == "RECTANGLE_RAW":
            rectangle = self._rfb_diag_current_rectangle or {}
            expected = int(rectangle.get("raw_bytes", -1))
            self._diag_event(
                "RECTANGLE_RAW_COMPLETE",
                rectangle_index=self._rfb_diag_rectangle_index,
                expected_bytes=expected,
                actual_bytes=len(data),
            )
            self._rfb_diag_rectangles_remaining = max(
                0, self._rfb_diag_rectangles_remaining - 1
            )
            self._rfb_diag_rectangle_index += 1
            self._rfb_diag_current_rectangle = None
            self._rfb_diag_phase = (
                "RECTANGLE_HEADER"
                if self._rfb_diag_rectangles_remaining
                else "UPDATE_RECTANGLES_COMPLETE"
            )

        self._diag_event(
            "UPSTREAM_EXACT_READ_COMPLETE",
            requested_bytes=int(count),
            returned_bytes=len(data),
        )
        return data

    def _read_filtered_framebuffer_update(
        self,
        first_byte: bytes,
        suppression_rect: suppression.H1Cp2pSuppressionRect,
    ) -> bytes:
        before = {
            "filtered_updates": int(self.suppression_stats.filtered_updates),
            "input_rectangles": int(self.suppression_stats.input_rectangles),
            "output_rectangles": int(self.suppression_stats.output_rectangles),
            "input_pixel_bytes": int(self.suppression_stats.input_pixel_bytes),
            "output_pixel_bytes": int(self.suppression_stats.output_pixel_bytes),
        }
        self._diag_event("FILTER_UPDATE_BEGIN")
        payload = super()._read_filtered_framebuffer_update(first_byte, suppression_rect)
        after = {
            "filtered_updates": int(self.suppression_stats.filtered_updates),
            "input_rectangles": int(self.suppression_stats.input_rectangles),
            "output_rectangles": int(self.suppression_stats.output_rectangles),
            "input_pixel_bytes": int(self.suppression_stats.input_pixel_bytes),
            "output_pixel_bytes": int(self.suppression_stats.output_pixel_bytes),
        }
        self._diag_event(
            "FILTER_UPDATE_COMPLETE",
            payload_bytes=len(payload),
            input_rectangles_delta=after["input_rectangles"] - before["input_rectangles"],
            output_rectangles_delta=after["output_rectangles"] - before["output_rectangles"],
            input_pixel_bytes_delta=after["input_pixel_bytes"] - before["input_pixel_bytes"],
            output_pixel_bytes_delta=after["output_pixel_bytes"] - before["output_pixel_bytes"],
        )
        return payload

    def _send_with_credit(self, payload: bytes) -> None:
        with self.condition:
            credit_before = int(self.credit)
        self._rfb_diag_phase = "FILTERED_SEND"
        self._rfb_diag_filtered_send_bytes = len(payload)
        self._diag_event(
            "FILTERED_SEND_BEGIN",
            payload_bytes=len(payload),
            credit_before=credit_before,
        )
        super()._send_with_credit(payload)
        with self.condition:
            credit_after = int(self.credit)
        self._rfb_diag_filtered_send_bytes = 0
        self._rfb_diag_phase = "FILTERED_SEND_COMPLETE"
        self._diag_event(
            "FILTERED_SEND_COMPLETE",
            payload_bytes=len(payload),
            credit_after=credit_after,
        )


class H1Cp2pRfbTransactionAdapter(adapter_base.H1RfbSessionAdapter):
    """Existing adapter lifecycle with only the bridge implementation replaced."""

    def __init__(self, session: base.H1Session, upstream: socket.socket) -> None:
        super().__init__(session, upstream)
        self.bridge = H1Cp2pRfbTransactionBridge(
            upstream,
            queue_capacity=int(session.profile["rfb_queue_capacity"]),
            max_payload=int(session.profile["max_data_payload"]),
            send_data=self._send_server_data_to_ps2,
            desktop_width=int(session.profile["desktop_width"]),
            desktop_height=int(session.profile["desktop_height"]),
        )


def open_rfb_transaction_diag_adapter(
    session: base.H1Session,
    *,
    host: str = "127.0.0.1",
    port: int = 5900,
    connector: Callable[[str, int], socket.socket] = adapter_base._default_vnc_connector,
) -> H1Cp2pRfbTransactionAdapter | None:
    mode = int(session.profile["rfb_mode"])
    if mode == adapter_base.RFB_OFF:
        return None
    if not adapter_base._rfb_mode_is_enabled(mode):
        raise base.ProtocolError(f"unsupported Pi RFB mode {mode}")
    if not host:
        raise base.ProtocolError("RFB upstream host must be non-empty")
    if port <= 0 or port > 65535:
        raise base.ProtocolError(f"invalid RFB upstream port {port}")

    upstream = connector(host, port)
    try:
        upstream.settimeout(None)
        adapter = H1Cp2pRfbTransactionAdapter(session, upstream)
        adapter.start()
    except BaseException:
        try:
            upstream.close()
        except OSError:
            pass
        raise
    return adapter


# CP2P's parent resolves this construction seam when each session is created.
cp2p.cp2o.open_rfb_session_adapter = open_rfb_transaction_diag_adapter


class H1Cp2pRfbTransactionDiagnosticSession(stall.H1Cp2pStallDiagnosticSession):
    """Existing stall diagnostics plus dedicated RFB transaction evidence."""

    def __init__(self, sock, profile, evidence, duration, display) -> None:
        self._rfb_transaction_stop = threading.Event()
        super().__init__(sock, profile, evidence, duration, display)

        adapter = self.rfb_session_adapter
        if adapter is None or not isinstance(
            adapter.bridge, H1Cp2pRfbTransactionBridge
        ):
            super().cleanup()
            raise base.ProtocolError("RFB transaction diagnostic bridge was not installed")

        self._rfb_transaction_bridge = adapter.bridge
        self._rfb_transaction_bridge.attach_diagnostic_evidence(Path(self.evidence))
        self._rfb_transaction_watchdog_thread = threading.Thread(
            target=self._rfb_transaction_watchdog,
            name="h1-cp2p-rfb-transaction-watchdog",
            daemon=True,
        )
        self._rfb_transaction_watchdog_thread.start()

    def _parse_telemetry(self, payload: bytes) -> dict[str, int]:
        parsed = super()._parse_telemetry(payload)
        words = struct.unpack(">40I", payload)

        # Diagnostic h1_rfb_mux_io.c uses these four live-only producer slots.
        rfb_stage = int(words[27])
        parsed.update(
            {
                "rfb_io_stage": rfb_stage,
                "rfb_io_stage_name": _decode_rfb_io_stage(rfb_stage),
                "rfb_io_count": int(words[28]),
                "rfb_io_sequence": int(words[29]),
                "rfb_io_detail": int(words[39]),
            }
        )

        record = {
            "pi_monotonic": time.monotonic(),
            "rfb_io_stage": f"0x{rfb_stage:08X}",
            "rfb_io_stage_name": parsed["rfb_io_stage_name"],
            "rfb_io_count": parsed["rfb_io_count"],
            "rfb_io_sequence": parsed["rfb_io_sequence"],
            "rfb_io_detail": parsed["rfb_io_detail"],
            "frames_received": parsed.get("frames_received"),
            "receiver_loop_count": parsed.get("receiver_loop_count"),
        }
        with (Path(self.evidence) / "rfb_transaction_telemetry.jsonl").open(
            "a", encoding="utf-8"
        ) as handle:
            handle.write(json.dumps(record, sort_keys=True) + "\n")

        return parsed

    def _rfb_transaction_watchdog(self) -> None:
        path = Path(self.evidence) / "rfb_transaction_watchdog.jsonl"
        last_console = 0.0
        while not self._rfb_transaction_stop.is_set():
            bridge = getattr(self, "_rfb_transaction_bridge", None)
            if bridge is not None:
                snapshot = bridge.diagnostic_snapshot()
                now = time.monotonic()
                telemetry = self._diag_last_telemetry or {}
                snapshot.update(
                    {
                        "elapsed_s": round(now - self._diag_started, 3),
                        "ps2_rfb_io_stage": telemetry.get("rfb_io_stage_name"),
                        "ps2_rfb_io_count": telemetry.get("rfb_io_count"),
                        "ps2_rfb_io_sequence": telemetry.get("rfb_io_sequence"),
                        "ps2_rfb_io_detail": telemetry.get("rfb_io_detail"),
                    }
                )
                with path.open("a", encoding="utf-8") as handle:
                    handle.write(json.dumps(snapshot, sort_keys=True) + "\n")

                if now - last_console >= _RFB_CONSOLE_PERIOD_S:
                    compact = {
                        "elapsed_s": snapshot["elapsed_s"],
                        "pi_phase": snapshot["phase"],
                        "pi_last_event": snapshot["last_event"],
                        "rfb_credit": snapshot["rfb_credit"],
                        "filtered_updates": snapshot["filtered_updates"],
                        "ps2_stage": snapshot["ps2_rfb_io_stage"],
                        "ps2_count": snapshot["ps2_rfb_io_count"],
                    }
                    print(
                        "H1_CP2P_RFB_TX_WATCHDOG="
                        + json.dumps(compact, sort_keys=True),
                        flush=True,
                    )
                    last_console = now

            self._rfb_transaction_stop.wait(_RFB_SNAPSHOT_PERIOD_S)

    def cleanup(self) -> None:
        self._rfb_transaction_stop.set()
        super().cleanup()


base.H1Session = H1Cp2pRfbTransactionDiagnosticSession


if __name__ == "__main__":
    raise SystemExit(base.main())
