#!/usr/bin/env python3
"""
File synopsis:
    CP2P all-guns recalibration/retirement persistent-witness runner.

Purpose:
    Preserve the exact priority-67 Pi transport/RFB behavior while decoding
    telemetry word 39 as the observation-only recalibration witness introduced
    by the matching diagnostic build. The parent MPEG witness archive is retained
    for provenance; this runner additionally writes a focused recalibration log.

No Pi scheduling, producer, RFB suppression, credit, or transport behavior is
changed.
"""

from __future__ import annotations

import json
from pathlib import Path
import time

import h1_mux_server_cp2p_mpeg_persistent_witness as parent

base = parent.base
_ParentSession = parent.H1Cp2pMpegPersistentWitnessSession


_FIXED_STAGES = {
    0xE0000001: "CHORD_DETECTED",
    0xE0000002: "HOLD_MATURED",
    0xE0000003: "CALIBRATION_ENTRY_GATE_CALL_ENTER",
    0xE0000004: "CALIBRATION_ENTRY_GATE_CALL_RETURN",
    0xE0000005: "DEFERRED_FLAG_SET_ENTER",
    0xE0000006: "DEFERRED_FLAG_SET_RETURN",
    0xE0000007: "DEFERRED_POLL_ENTER",
    0xE0000008: "DEFERRED_GATE_RETURN",
    0xE0000009: "DEFERRED_FLAG_CLEARED",
    0xE000000A: "CALIBRATION_REENTRY_ENTER",
    0xE000000B: "CALIBRATION_REENTRY_RETURN",
    0xE1000001: "CALIBRATION_ENTRY_GATE_ENTER",
    0xE1000002: "RETIRE_CONTROL_SEND_BEGIN",
    0xE1000003: "RETIRE_CONTROL_SEND_RETURN",
    0xE1000004: "RETIRE_ACK_POLL_ENTER",
    0xE1000005: "RETIRE_ACK_STILL_PENDING",
    0xE1000006: "RETIRE_ACK_OBSERVED",
    0xE1000007: "RECALIBRATION_BEGIN_ENTER",
    0xE1000008: "RECALIBRATION_BEGIN_RETURN",
    0xE1000009: "WAIT_FOR_FULL_RFB",
    0xE100000A: "RFB_RESTORATION_POLL_ENTER",
    0xE100000B: "RFB_RESTORATION_COMPLETE",
    0xE100000C: "CALIBRATION_ENTRY_READY",
    0xE2000001: "CLEAR_AFTER_PI_RETIRE_ENTER",
    0xE2000002: "LOCAL_WORKER_CLEAR_ENTER",
    0xE2000003: "LOCAL_WORKER_CLEAR_RETURN",
    0xE2000004: "MPEG_QUEUE_FINALIZE_ENTER",
    0xE2000005: "MPEG_QUEUE_FINALIZE_RETURN",
    0xE3000000: "WORKER_JOIN_NOT_STARTED",
    0xE3000001: "WORKER_CLEAR_ENTER",
    0xE3000002: "WORKER_JOIN_ENTER",
    0xE3000003: "WORKER_STOP_REQUESTED",
    0xE3000004: "WORKER_WAIT_ENTER",
    0xE3000005: "WORKER_DORMANT_OBSERVED",
    0xE3000006: "WORKER_FINISHED_OBSERVED",
    0xE3000007: "DELETE_THREAD_ENTER",
    0xE3000008: "DELETE_THREAD_RETURN",
    0xE3000009: "MPEG_PIXEL_CLEAR_ENTER",
    0xE300000A: "MPEG_PIXEL_CLEAR_RETURN",
    0xE300000B: "WORKER_CLEAR_RETURN",
    0xE3FF0001: "WORKER_JOIN_TIMEOUT",
    0xE3FF0002: "REFER_THREAD_STATUS_FAIL",
    0xE3FF0003: "WORKER_JOIN_DELAY_FAIL",
    0xE3FF0004: "DELETE_THREAD_FAIL",
    0xE3FF0005: "MPEG_PIXEL_CLEAR_FAIL",
}


def _decode_recalibration_stage(stage: int) -> str:
    if (stage & 0xFFFF0000) == 0xE3100000:
        return f"WORKER_WAIT_LOOP[{stage & 0xFFFF}]"
    return _FIXED_STAGES.get(stage, f"OTHER_0x{stage:08X}")


class H1Cp2pRecalibrationWitnessSession(_ParentSession):
    """Parent all-guns diagnostics plus persistent recalibration-stage decoding."""

    def _parse_telemetry(self, payload: bytes) -> dict[str, object]:
        parsed = super()._parse_telemetry(payload)
        end_received = int(parsed.get("end_received", 0))
        stage = int(parsed.get("mpeg_last_stage", 0)) if end_received == 0 else 0
        stage_name = (
            _decode_recalibration_stage(stage)
            if end_received == 0
            else "MEDIA_END_AUTHORITATIVE"
        )

        parsed.update(
            {
                "recalibration_stage": stage,
                "recalibration_stage_hex": (
                    f"0x{stage:08X}" if end_received == 0 else None
                ),
                "recalibration_stage_name": stage_name,
            }
        )

        record = {
            "pi_monotonic": time.monotonic(),
            "end_received": end_received,
            "recalibration_stage": parsed["recalibration_stage_hex"],
            "recalibration_stage_name": stage_name,
            "frames_received": parsed.get("frames_received"),
            "receiver_loop_count": parsed.get("receiver_loop_count"),
            "audio_bytes_enqueued": parsed.get("audio_bytes_enqueued"),
            "audio_bytes_consumed": parsed.get("audio_bytes_consumed"),
            "audio_bytes_played": parsed.get("audio_bytes_played"),
            "mpeg_bytes_enqueued": parsed.get("mpeg_bytes_enqueued"),
            "mpeg_bytes_consumed": parsed.get("mpeg_bytes_consumed"),
            "mpeg_read_calls": parsed.get("mpeg_read_calls"),
            "rfb_io_stage": parsed.get("rfb_io_stage"),
            "rfb_io_stage_name": parsed.get("rfb_io_stage_name"),
        }

        with (
            Path(self.evidence) / "recalibration_persistent_witness_telemetry.jsonl"
        ).open("a", encoding="utf-8") as handle:
            handle.write(json.dumps(record, sort_keys=True) + "\n")

        return parsed


base.H1Session = H1Cp2pRecalibrationWitnessSession


if __name__ == "__main__":
    raise SystemExit(base.main())
