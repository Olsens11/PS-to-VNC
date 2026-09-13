#!/usr/bin/env python3
"""
File synopsis:
    Disposable CP2P all-guns runner layered on the RFB-transaction diagnostic.

Purpose:
    Preserve the exact Pi transport/RFB/suppression behavior while decoding one
    dedicated PS2 live telemetry witness: before MEDIA_END, telemetry word 39
    (producer_stop_reason) contains the last MPEG D-stage written by the decoder
    runtime. MEDIA_END later restores the normal authoritative producer stop
    reason semantics.

The underlying RFB diagnostic still owns producer_audio_bytes,
producer_mpeg_bytes, and producer_picture_starts. Its former live-only detail
word is intentionally relinquished so RFB observation cannot overwrite the
MPEG last-stage witness.
"""

from __future__ import annotations

import json
from pathlib import Path
import struct
import time

import h1_mux_server_cp2p_rfb_transaction_diag as tx

base = tx.base
_ParentSession = tx.H1Cp2pRfbTransactionDiagnosticSession


class H1Cp2pMpegPersistentWitnessSession(_ParentSession):
    """RFB transaction diagnostics plus one persistent MPEG last-stage field."""

    def _parse_telemetry(self, payload: bytes) -> dict[str, object]:
        # Bypass the transaction layer's old word-39 RFB-detail interpretation;
        # the stall parent still performs its normal telemetry/archive handling.
        parsed = tx.stall.H1Cp2pStallDiagnosticSession._parse_telemetry(self, payload)
        words = struct.unpack(">40I", payload)

        rfb_stage = int(words[27])
        end_received = int(parsed.get("end_received", 0))
        mpeg_stage = int(words[39]) if end_received == 0 else 0

        parsed.update(
            {
                "rfb_io_stage": rfb_stage,
                "rfb_io_stage_name": tx._decode_rfb_io_stage(rfb_stage),
                "rfb_io_count": int(words[28]),
                "rfb_io_sequence": int(words[29]),
                "rfb_io_detail": None,
                "mpeg_last_stage": mpeg_stage,
                "mpeg_last_stage_hex": (
                    f"0x{mpeg_stage:08X}" if end_received == 0 else None
                ),
                "mpeg_last_stage_name": (
                    tx.stall._decode_stage(mpeg_stage)
                    if end_received == 0
                    else "MEDIA_END_AUTHORITATIVE"
                ),
            }
        )

        record = {
            "pi_monotonic": time.monotonic(),
            "end_received": end_received,
            "mpeg_last_stage": parsed["mpeg_last_stage_hex"],
            "mpeg_last_stage_name": parsed["mpeg_last_stage_name"],
            "rfb_io_stage": f"0x{rfb_stage:08X}",
            "rfb_io_stage_name": parsed["rfb_io_stage_name"],
            "rfb_io_count": parsed["rfb_io_count"],
            "rfb_io_sequence": parsed["rfb_io_sequence"],
            "frames_received": parsed.get("frames_received"),
            "receiver_loop_count": parsed.get("receiver_loop_count"),
            "audio_bytes_consumed": parsed.get("audio_bytes_consumed"),
            "audio_bytes_played": parsed.get("audio_bytes_played"),
            "mpeg_bytes_consumed": parsed.get("mpeg_bytes_consumed"),
            "mpeg_read_calls": parsed.get("mpeg_read_calls"),
        }
        with (Path(self.evidence) / "mpeg_persistent_witness_telemetry.jsonl").open(
            "a", encoding="utf-8"
        ) as handle:
            handle.write(json.dumps(record, sort_keys=True) + "\n")

        return parsed


base.H1Session = H1Cp2pMpegPersistentWitnessSession


if __name__ == "__main__":
    raise SystemExit(base.main())
