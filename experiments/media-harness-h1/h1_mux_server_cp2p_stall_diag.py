#!/usr/bin/env python3
"""
File synopsis:
    Disposable CP2P all-guns stall diagnostic runner.

It imports the exact item-10 CP2P runner and changes no media policy. The only
additions are:
  * low-rate PSTV HEARTBEAT probes that request the transport's already-existing
    160-byte telemetry snapshot;
  * an independent Pi watchdog thread that keeps printing even if the ordinary
    media scheduler blocks in socket.sendall() or behind the shared send lock;
  * diagnostic-only send-path state so a hang can be classified as lock wait,
    header send, payload send, or ordinary scheduler inactivity;
  * JSONL archives for watchdog and PS2 telemetry snapshots.

The PS2 diagnostic_word is decoded as the stage markers injected by
`generate_h1_video_runtime_cp2p_diag.py` and `h1_cp2p_mpeg_worker.c`.
"""

from __future__ import annotations

import json
import socket
import struct
import threading
import time
from pathlib import Path

import h1_mux_server_cp2p_start_receiver as cp2p

base = cp2p.base
_ParentSession = cp2p.H1Cp2pStartReceiveSession

_HEARTBEAT_PERIOD_S = 0.25
_WATCHDOG_PERIOD_S = 1.0

_STAGE_NAMES = {
    0xD1000001: "MPEG_WORKER_ENTER",
    0xD1000002: "MPEG_VIDEO_RUNTIME_RETURN",
    0xD1000003: "MPEG_WORKER_FINISHED",
    0xD1000010: "MPEG_WORKER_INITIALIZED",
    0xD1000011: "MPEG_ARM_ACCEPTED",
    0xD1000012: "MPEG_BEFORE_CHASSIS_INIT",
    0xD1000013: "MPEG_CHASSIS_INIT_RETURN",
    0xD1000014: "MPEG_BEFORE_START_THREAD",
    0xD1FF0001: "MPEG_WORKER_ARM_FAIL",
    0xD2000001: "BEFORE_MPEG_INITIALIZE",
    0xD2000002: "MPEG_INITIALIZE_RETURN",
    0xD2000003: "BEFORE_MPEG_PREFILL",
    0xD2000004: "MPEG_PREFILL_COMPLETE",
    0xD5000001: "MPEG_FEED_ENTER",
    0xD5000002: "MPEG_FEED_BEFORE_QUEUE_READ",
    0xD5000003: "MPEG_FEED_QUEUE_READ_RETURN",
    0xD5000004: "MPEG_FEED_BEFORE_TO_IPU_DMA_WAIT",
    0xD5000005: "MPEG_FEED_TO_IPU_DMA_WAIT_RETURN",
    0xD5000006: "MPEG_FEED_TO_IPU_DMA_SUBMITTED",
    0xD5000007: "MPEG_FEED_RETURN_SUCCESS",
    0xD5FF0001: "MPEG_FEED_READ_FALSE",
    0xD5FF0002: "MPEG_FEED_BAD_PAYLOAD_SIZE",
    0xD5FF0003: "MPEG_FEED_CAPACITY_FAIL",
    0xD5FF0004: "MPEG_FEED_DMA_WAIT_FAIL",
    0xD6000001: "MPEG_DRAW_ENTER",
    0xD6000002: "MPEG_DRAW_BEFORE_COMPOSITOR_PRESENT",
    0xD6000003: "MPEG_DRAW_COMPOSITOR_PRESENT_RETURN",
    0xD6000004: "MPEG_DRAW_RETURN_SUCCESS",
    0xD6FF0001: "MPEG_DRAW_COMPOSITOR_FAIL",
    0xD6FF0002: "MPEG_DRAW_FIRST_FRAME_HANDOFF_FAIL",
    0xD7000001: "IPU_RESET_ENTER",
    0xD7000002: "IPU_RESET_WAIT_RST_BUSY_CLEAR",
    0xD7000003: "IPU_RESET_RST_BUSY_CLEARED",
    0xD7000004: "IPU_RESET_WAIT_BCLR_BUSY_CLEAR",
    0xD7000005: "IPU_RESET_BCLR_BUSY_CLEARED",
    0xD7000006: "IPU_RESET_RETURN",
    0xD8000001: "TEARDOWN_BEFORE_TO_IPU_DMA_WAIT",
    0xD8000002: "TEARDOWN_TO_IPU_DMA_WAIT_RETURN",
    0xD8000003: "TEARDOWN_BEFORE_MPEG_DESTROY",
    0xD8000004: "TEARDOWN_MPEG_DESTROY_RETURN",
}


def _decode_stage(word: int) -> str:
    word &= 0xFFFFFFFF
    if (word & 0xFF000000) == 0xD3000000:
        return f"MPEG_PICTURE_ENTER[{word & 0x00FFFFFF}]"
    if (word & 0xFF000000) == 0xD4000000:
        return f"MPEG_PICTURE_RETURN[{word & 0x00FFFFFF}]"
    return _STAGE_NAMES.get(word, f"OTHER_0x{word:08X}")


class H1Cp2pStallDiagnosticSession(_ParentSession):
    """Exact CP2P session plus observation-only liveness instrumentation."""

    def __init__(self, sock, profile, evidence, duration, display) -> None:
        self._diag_started = time.monotonic()
        self._diag_stop = threading.Event()

        # The heartbeat is an active PSTV writer, while the watchdog is
        # observation-only.  Give the writer its own lifetime so MEDIA_END can
        # become the final Pi->PS2 application frame without sacrificing
        # shutdown telemetry.
        self._diag_heartbeat_stop = threading.Event()

        self._diag_send_attempts = 0
        self._diag_send_completions = 0
        self._diag_send_failures = 0
        self._diag_waiting_senders = 0
        self._diag_send_lock_holder = ""
        self._diag_send_stage = "idle"
        self._diag_last_send_thread = ""
        self._diag_last_send_kind = -1
        self._diag_last_send_channel = -1
        self._diag_last_send_bytes = 0
        self._diag_last_send_started = 0.0
        self._diag_last_send_completed = 0.0
        self._diag_heartbeat_attempts = 0
        self._diag_heartbeat_completions = 0
        self._diag_heartbeat_skips_locked = 0
        self._diag_heartbeat_error = ""
        self._diag_last_telemetry_monotonic = 0.0
        self._diag_last_telemetry: dict[str, int] | None = None

        super().__init__(sock, profile, evidence, duration, display)

        self._diag_watchdog_thread = threading.Thread(
            target=self._diag_watchdog,
            name="h1-cp2p-stall-watchdog",
            daemon=True,
        )
        self._diag_heartbeat_thread = threading.Thread(
            target=self._diag_heartbeat,
            name="h1-cp2p-stall-heartbeat",
            daemon=True,
        )
        self._diag_watchdog_thread.start()
        self._diag_heartbeat_thread.start()

    def send_frame(
        self,
        kind: int,
        channel: int,
        payload: bytes = b"",
        flags: int = 0,
    ) -> int:
        """Base send_frame semantics with diagnostic-only lock/socket witnesses."""

        if len(payload) > base.MAX_PAYLOAD:
            raise base.ProtocolError("attempted oversized PSTV payload")

        thread_name = threading.current_thread().name
        self._diag_send_attempts += 1
        self._diag_waiting_senders += 1
        self._diag_last_send_thread = thread_name
        self._diag_last_send_kind = int(kind)
        self._diag_last_send_channel = int(channel)
        self._diag_last_send_bytes = len(payload)
        self._diag_last_send_started = time.monotonic()
        self._diag_send_stage = "WAIT_SEND_LOCK"

        self.send_lock.acquire()
        self._diag_waiting_senders -= 1
        self._diag_send_lock_holder = thread_name

        try:
            sequence = self.next_tx_sequence
            self.next_tx_sequence += 1

            self._diag_send_stage = "SEND_HEADER"
            self.sock.sendall(
                base.encode_header(kind, channel, flags, sequence, len(payload))
            )

            if payload:
                self._diag_send_stage = "SEND_PAYLOAD"
                self.sock.sendall(payload)

            self._diag_send_stage = "SEND_COMPLETE"
            self._diag_send_completions += 1
            self._diag_last_send_completed = time.monotonic()
            return sequence
        except BaseException:
            self._diag_send_failures += 1
            self._diag_send_stage = "SEND_EXCEPTION"
            raise
        finally:
            self._diag_send_lock_holder = ""
            self.send_lock.release()

    def _parse_telemetry(self, payload: bytes) -> dict[str, int]:
        parsed = super()._parse_telemetry(payload)
        words = struct.unpack(">40I", payload)
        parsed.update(
            {
                "frames_received": words[4],
                "payload_bytes_received": words[5],
                "last_received_sequence": words[6],
                "last_sent_sequence": words[7],
                "audio_credit_bytes_sent": words[12],
                "audio_chunks_played": words[13],
                "audio_bytes_played": words[14],
                "audio_read_calls": words[15],
                "mpeg_credit_bytes_sent": words[20],
                "mpeg_read_calls": words[21],
                "receiver_loop_count": words[38],
            }
        )
        self._diag_last_telemetry_monotonic = time.monotonic()
        self._diag_last_telemetry = dict(parsed)

        record = {
            "pi_monotonic": self._diag_last_telemetry_monotonic,
            "stage_hex": f"0x{parsed['diagnostic_word']:08X}",
            "stage": _decode_stage(parsed["diagnostic_word"]),
            "telemetry": parsed,
        }
        with (Path(self.evidence) / "mpeg_stall_telemetry.jsonl").open(
            "a", encoding="utf-8"
        ) as handle:
            handle.write(json.dumps(record, sort_keys=True) + "\n")
        return parsed

    def _diag_heartbeat(self) -> None:
        # Do not add any extra PSTV traffic before the exact CONFIG ACK.
        while (
            not self._diag_stop.is_set()
            and not self._diag_heartbeat_stop.is_set()
            and not self.config_ack_event.wait(0.1)
        ):
            pass

        while (
            not self._diag_stop.is_set()
            and not self._diag_heartbeat_stop.is_set()
        ):
            # If another sender already owns the serialized path, do not queue a
            # probe behind it. The watchdog will report the occupied lock instead.
            if self.send_lock.locked():
                self._diag_heartbeat_skips_locked += 1
            else:
                self._diag_heartbeat_attempts += 1
                try:
                    self.send_frame(base.FRAME_HEARTBEAT, base.CHANNEL_CONTROL)
                    self._diag_heartbeat_completions += 1
                except BaseException as exc:
                    self._diag_heartbeat_error = repr(exc)
                    return

            self._diag_heartbeat_stop.wait(_HEARTBEAT_PERIOD_S)

    def _stop_diag_heartbeat_before_media_end(self) -> None:
        """Make ordinary MEDIA_END the final Pi->PS2 diagnostic write."""

        if self._diag_heartbeat_stop.is_set():
            if self._diag_heartbeat_thread.is_alive():
                self._diag_heartbeat_thread.join(timeout=2.0)

            if self._diag_heartbeat_thread.is_alive():
                raise base.ProtocolError(
                    "diagnostic heartbeat did not stop before MEDIA_END"
                )

            return

        attempts_before = self._diag_heartbeat_attempts
        completions_before = self._diag_heartbeat_completions

        # Close admission before waiting. If the heartbeat already owns
        # send_lock, join waits for that exact send to finish before MEDIA_END
        # can enter the serialized path.
        self._diag_heartbeat_stop.set()
        self._diag_heartbeat_thread.join(timeout=2.0)

        if self._diag_heartbeat_thread.is_alive():
            raise base.ProtocolError(
                "diagnostic heartbeat did not stop before MEDIA_END"
            )

        print(
            "H1_PI_HEARTBEAT_STOP_BEFORE_MEDIA_END=PASS "
            f"attempts_before={attempts_before} "
            f"attempts_after={self._diag_heartbeat_attempts} "
            f"completions_before={completions_before} "
            f"completions_after={self._diag_heartbeat_completions} "
            f"send_lock_locked={int(self.send_lock.locked())}",
            flush=True,
        )

    def _send_rfb_pcm_media_end(self) -> dict[str, int]:
        """Stop diagnostic writes, then delegate to the real MEDIA_END path."""

        self._stop_diag_heartbeat_before_media_end()

        metadata = super()._send_rfb_pcm_media_end()

        print(
            "H1_PI_MEDIA_END_FINAL_APPLICATION_WRITE=PASS "
            "half_close=0",
            flush=True,
        )

        return metadata

    def _diag_watchdog(self) -> None:
        path = Path(self.evidence) / "mpeg_stall_watchdog.jsonl"
        while not self._diag_stop.is_set():
            now = time.monotonic()
            telemetry = self._diag_last_telemetry or {}
            telemetry_age = (
                now - self._diag_last_telemetry_monotonic
                if self._diag_last_telemetry_monotonic > 0.0
                else None
            )
            word = int(telemetry.get("diagnostic_word", 0))

            snapshot = {
                "elapsed_s": round(now - self._diag_started, 3),
                "telemetry_age_s": (
                    round(telemetry_age, 3) if telemetry_age is not None else None
                ),
                "stage_hex": f"0x{word:08X}",
                "stage": _decode_stage(word),
                "send_lock_locked": bool(self.send_lock.locked()),
                "send_lock_holder": self._diag_send_lock_holder,
                "send_stage": self._diag_send_stage,
                "waiting_senders": self._diag_waiting_senders,
                "send_attempts": self._diag_send_attempts,
                "send_completions": self._diag_send_completions,
                "send_failures": self._diag_send_failures,
                "last_send_thread": self._diag_last_send_thread,
                "last_send_kind": self._diag_last_send_kind,
                "last_send_channel": self._diag_last_send_channel,
                "last_send_payload_bytes": self._diag_last_send_bytes,
                "last_send_age_s": (
                    round(now - self._diag_last_send_started, 3)
                    if self._diag_last_send_started > 0.0
                    else None
                ),
                "heartbeat_attempts": self._diag_heartbeat_attempts,
                "heartbeat_completions": self._diag_heartbeat_completions,
                "heartbeat_skips_locked": self._diag_heartbeat_skips_locked,
                "heartbeat_error": self._diag_heartbeat_error,
                "audio_credit": int(getattr(self.audio, "credit", 0)),
                "audio_bytes_sent": int(getattr(self.audio, "bytes_sent", 0)),
                "mpeg_credit": int(getattr(self.mpeg, "credit", 0)),
                "mpeg_bytes_sent": int(getattr(self.mpeg, "bytes_sent", 0)),
                "receiver_loop_count": telemetry.get("receiver_loop_count"),
                "frames_received": telemetry.get("frames_received"),
                "last_received_sequence": telemetry.get("last_received_sequence"),
                "last_sent_sequence": telemetry.get("last_sent_sequence"),
                "audio_bytes_enqueued": telemetry.get("audio_bytes_enqueued"),
                "audio_bytes_consumed": telemetry.get("audio_bytes_consumed"),
                "audio_bytes_played": telemetry.get("audio_bytes_played"),
                "audio_chunks_played": telemetry.get("audio_chunks_played"),
                "mpeg_bytes_enqueued": telemetry.get("mpeg_bytes_enqueued"),
                "mpeg_bytes_consumed": telemetry.get("mpeg_bytes_consumed"),
                "mpeg_read_calls": telemetry.get("mpeg_read_calls"),
                "mpeg_wait_events": telemetry.get("mpeg_wait_events"),
            }

            print(
                "H1_CP2P_STALL_WATCHDOG="
                + json.dumps(snapshot, sort_keys=True),
                flush=True,
            )
            with path.open("a", encoding="utf-8") as handle:
                handle.write(json.dumps(snapshot, sort_keys=True) + "\n")

            self._diag_stop.wait(_WATCHDOG_PERIOD_S)

    def cleanup(self) -> None:
        self._diag_heartbeat_stop.set()
        self._diag_stop.set()
        super().cleanup()


base.H1Session = H1Cp2pStallDiagnosticSession


if __name__ == "__main__":
    raise SystemExit(base.main())
