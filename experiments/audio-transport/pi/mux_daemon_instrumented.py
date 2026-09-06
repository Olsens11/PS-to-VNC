#!/usr/bin/env python3
"""
File synopsis:
    Experiment-only diagnostic wrapper for the Pi mux policy daemon.

This wrapper preserves the existing mux protocol and scheduling behavior while
adding timestamped observations around the raw-PCM path:

    PipeWire -> audio reader -> Pi host buffer -> scheduler/credit gate
             -> TCP send -> PS2 receiver queue.

The base mux_daemon.py remains unchanged.  This file subclasses its session
implementation and installs that subclass only for this diagnostic process.

No recovery behavior, watchdog, packet dropping, or policy changes are added.
"""

from __future__ import annotations

import json
import sys
import threading
import time
from pathlib import Path

# Keep this experiment companion directly runnable and also importable by
# validation harnesses that load it by absolute path rather than as a script.
THIS_DIRECTORY = Path(__file__).resolve().parent

if str(THIS_DIRECTORY) not in sys.path:
    sys.path.insert(0, str(THIS_DIRECTORY))

import mux_daemon as base


DIAG_PERIOD_SECONDS = 0.250
SLOW_STAGE_SECONDS = 0.100


class InstrumentedMuxSession(base.MuxSession):
    """Mux session with observation-only audio-path instrumentation."""

    def __init__(self, ps2_socket, config):
        super().__init__(ps2_socket, config)

        self.diag_lock = threading.Lock()

        self.last_pipe_read_complete = None
        self.last_audio_enqueue_complete = None
        self.last_audio_send_complete = None

        self.last_pipe_read_seconds = 0.0
        self.last_audio_enqueue_seconds = 0.0
        self.last_audio_send_seconds = 0.0

        self.last_pipe_read_bytes = 0

        self.audio_source_bytes_total = 0
        self.audio_send_bytes_total = 0

        self.pipe_gap_events = 0
        self.enqueue_block_events = 0
        self.send_block_events = 0

        self.last_audio_credit_received = None
        self.last_audio_credit_gap_seconds = 0.0
        self.max_audio_credit_gap_seconds = 0.0

        self.audio_credit_receipts = 0
        self.audio_credit_bytes_received_total = 0
        self.audio_credit_gap_events = 0

        audio_channel = self.channels[
            base.CHANNEL_AUDIO
        ]

        original_audio_add_credit = (
            audio_channel.add_credit
        )

        def add_audio_credit_with_timing(
            amount: int,
        ) -> None:
            now = time.monotonic()

            original_audio_add_credit(amount)

            with self.diag_lock:
                previous = (
                    self.last_audio_credit_received
                )

                if previous is None:
                    gap = 0.0
                else:
                    gap = now - previous

                self.last_audio_credit_received = now
                self.last_audio_credit_gap_seconds = gap

                if (
                    gap
                    > self.max_audio_credit_gap_seconds
                ):
                    self.max_audio_credit_gap_seconds = (
                        gap
                    )

                self.audio_credit_receipts += 1
                self.audio_credit_bytes_received_total += (
                    amount
                )

                if (
                    previous is not None
                    and gap >= 0.100
                ):
                    self.audio_credit_gap_events += 1
                    event_number = (
                        self.audio_credit_gap_events
                    )
                else:
                    event_number = 0

                total_receipts = (
                    self.audio_credit_receipts
                )

                total_bytes = (
                    self.audio_credit_bytes_received_total
                )

            if event_number:
                telemetry = self.last_telemetry

                self.diag_event(
                    "AUDIO_CREDIT_GAP",
                    event_number=event_number,
                    duration_ms=round(
                        gap * 1000.0,
                        3,
                    ),
                    credit_bytes=amount,
                    credit_receipts=total_receipts,
                    credit_bytes_total=total_bytes,
                    available_credit=(
                        audio_channel.available_credit
                    ),
                    outstanding=(
                        audio_channel.outstanding
                    ),
                    ps2_credit_bytes_sent=(
                        telemetry.get(
                            "audio_credit_bytes_sent"
                        )
                        if telemetry is not None
                        else None
                    ),
                    ps2_audio_queue=(
                        telemetry.get(
                            "audio_queue_current"
                        )
                        if telemetry is not None
                        else None
                    ),
                )

        audio_channel.add_credit = (
            add_audio_credit_with_timing
        )

    def diag_event(self, name: str, **fields) -> None:
        event = {
            "event": name,
            "monotonic": round(
                time.monotonic(),
                6,
            ),
        }

        event.update(fields)

        print(
            "AUDIO_EVENT="
            + json.dumps(
                event,
                sort_keys=True,
            ),
            flush=True,
        )

    def send_frame(
        self,
        kind: int,
        channel: int,
        payload: bytes = b"",
        flags: int = 0,
    ) -> None:
        is_audio_data = (
            kind == base.FRAME_DATA
            and channel == base.CHANNEL_AUDIO
        )

        started = time.monotonic()

        super().send_frame(
            kind,
            channel,
            payload,
            flags,
        )

        finished = time.monotonic()

        if not is_audio_data:
            return

        elapsed = finished - started

        with self.diag_lock:
            self.last_audio_send_complete = finished
            self.last_audio_send_seconds = elapsed
            self.audio_send_bytes_total += len(payload)

            if elapsed >= SLOW_STAGE_SECONDS:
                self.send_block_events += 1
                event_number = self.send_block_events
            else:
                event_number = 0

        if event_number:
            self.diag_event(
                "AUDIO_SEND_BLOCK",
                event_number=event_number,
                duration_ms=round(
                    elapsed * 1000.0,
                    3,
                ),
                payload_bytes=len(payload),
            )

    def audio_reader(self) -> None:
        assert self.audio_process is not None
        assert self.audio_process.stdout is not None

        mirror = base.VolumeMirror(
            bool(
                self.config[
                    "mirror_sink_volume"
                ]
            )
        )

        quantum = int(
            self.config[
                "audio_quantum"
            ]
        )

        while not self.stop_event.is_set():
            read_started = time.monotonic()

            payload = self.audio_process.stdout.read(
                quantum
            )

            read_finished = time.monotonic()
            read_elapsed = (
                read_finished - read_started
            )

            with self.diag_lock:
                self.last_pipe_read_complete = (
                    read_finished
                )
                self.last_pipe_read_seconds = (
                    read_elapsed
                )
                self.last_pipe_read_bytes = len(
                    payload
                )
                self.audio_source_bytes_total += len(
                    payload
                )

                if (
                    read_elapsed
                    >= SLOW_STAGE_SECONDS
                ):
                    self.pipe_gap_events += 1
                    event_number = (
                        self.pipe_gap_events
                    )
                else:
                    event_number = 0

            if event_number:
                self.diag_event(
                    "PIPE_READ_GAP",
                    event_number=event_number,
                    duration_ms=round(
                        read_elapsed * 1000.0,
                        3,
                    ),
                    payload_bytes=len(payload),
                )

            if not payload:
                raise EOFError(
                    "pw-record stopped"
                )

            remainder = (
                len(payload)
                % base.AUDIO_FRAME_BYTES
            )

            if remainder:
                payload = payload[:-remainder]

            if not payload:
                continue

            payload = mirror.apply(payload)

            enqueue_started = time.monotonic()

            self.enqueue(
                base.CHANNEL_AUDIO,
                payload,
            )

            enqueue_finished = time.monotonic()
            enqueue_elapsed = (
                enqueue_finished
                - enqueue_started
            )

            with self.diag_lock:
                self.last_audio_enqueue_complete = (
                    enqueue_finished
                )
                self.last_audio_enqueue_seconds = (
                    enqueue_elapsed
                )

                if (
                    enqueue_elapsed
                    >= SLOW_STAGE_SECONDS
                ):
                    self.enqueue_block_events += 1
                    enqueue_event_number = (
                        self.enqueue_block_events
                    )
                else:
                    enqueue_event_number = 0

            if enqueue_event_number:
                self.diag_event(
                    "AUDIO_HOST_BUFFER_BLOCK",
                    event_number=(
                        enqueue_event_number
                    ),
                    duration_ms=round(
                        enqueue_elapsed * 1000.0,
                        3,
                    ),
                    payload_bytes=len(payload),
                )

    @staticmethod
    def age_ms(now, timestamp):
        if timestamp is None:
            return None

        return round(
            (now - timestamp) * 1000.0,
            3,
        )

    def audio_diag_loop(self) -> None:
        while not self.stop_event.wait(
            DIAG_PERIOD_SECONDS
        ):
            now = time.monotonic()

            audio = self.channels[
                base.CHANNEL_AUDIO
            ]

            with self.condition:
                host_bytes = len(audio.data)
                host_high_water = (
                    audio.high_water
                )
                available_credit = (
                    audio.available_credit
                )
                outstanding = (
                    audio.outstanding
                )
                physical_capacity = (
                    audio.physical_capacity
                )

                policy_room = max(
                    0,
                    audio.policy_window
                    - audio.outstanding,
                )

                send_budget = (
                    audio.send_budget()
                )

            if host_bytes == 0:
                block_reason = (
                    "NO_SOURCE_DATA"
                )

            elif available_credit <= 0:
                block_reason = "NO_CREDIT"

            elif policy_room <= 0:
                block_reason = (
                    "POLICY_WINDOW"
                )

            elif send_budget == 0:
                block_reason = (
                    "OTHER_ZERO_BUDGET"
                )

            else:
                block_reason = "READY"

            with self.diag_lock:
                last_pipe = (
                    self.last_pipe_read_complete
                )
                last_enqueue = (
                    self.last_audio_enqueue_complete
                )
                last_send = (
                    self.last_audio_send_complete
                )

                pipe_duration = (
                    self.last_pipe_read_seconds
                )
                enqueue_duration = (
                    self.last_audio_enqueue_seconds
                )
                send_duration = (
                    self.last_audio_send_seconds
                )

                read_bytes = (
                    self.last_pipe_read_bytes
                )

                source_total = (
                    self.audio_source_bytes_total
                )
                send_total = (
                    self.audio_send_bytes_total
                )

                pipe_events = (
                    self.pipe_gap_events
                )
                enqueue_events = (
                    self.enqueue_block_events
                )
                send_events = (
                    self.send_block_events
                )

            telemetry = self.last_telemetry

            if telemetry is None:
                ps2_audio_queue = None
                ps2_audio_enqueued = None
                ps2_audio_played = None
                ps2_frames = None
            else:
                ps2_audio_queue = telemetry.get(
                    "audio_queue_current"
                )
                ps2_audio_enqueued = telemetry.get(
                    "audio_bytes_enqueued"
                )
                ps2_audio_played = telemetry.get(
                    "audio_bytes_played"
                )
                ps2_frames = telemetry.get(
                    "frames_received"
                )

            snapshot = {
                "monotonic": round(
                    now,
                    6,
                ),
                "block_reason": block_reason,
                "host_buffer": host_bytes,
                "host_high_water": host_high_water,
                "available_credit": (
                    available_credit
                ),
                "outstanding": outstanding,
                "physical_capacity": (
                    physical_capacity
                ),
                "policy_room": policy_room,
                "send_budget": send_budget,
                "ms_since_pipe_read": (
                    self.age_ms(
                        now,
                        last_pipe,
                    )
                ),
                "ms_since_enqueue": (
                    self.age_ms(
                        now,
                        last_enqueue,
                    )
                ),
                "ms_since_audio_send": (
                    self.age_ms(
                        now,
                        last_send,
                    )
                ),
                "last_pipe_read_ms": round(
                    pipe_duration * 1000.0,
                    3,
                ),
                "last_enqueue_ms": round(
                    enqueue_duration * 1000.0,
                    3,
                ),
                "last_audio_send_ms": round(
                    send_duration * 1000.0,
                    3,
                ),
                "last_pipe_read_bytes": (
                    read_bytes
                ),
                "source_bytes_total": (
                    source_total
                ),
                "audio_send_bytes_total": (
                    send_total
                ),
                "pipe_gap_events": (
                    pipe_events
                ),
                "enqueue_block_events": (
                    enqueue_events
                ),
                "send_block_events": (
                    send_events
                ),
                "ms_since_audio_credit": (
                    self.age_ms(
                        now,
                        self.last_audio_credit_received,
                    )
                ),
                "last_audio_credit_gap_ms": round(
                    self.last_audio_credit_gap_seconds
                    * 1000.0,
                    3,
                ),
                "max_audio_credit_gap_ms": round(
                    self.max_audio_credit_gap_seconds
                    * 1000.0,
                    3,
                ),
                "audio_credit_receipts": (
                    self.audio_credit_receipts
                ),
                "audio_credit_bytes_received_total": (
                    self.audio_credit_bytes_received_total
                ),
                "audio_credit_gap_events": (
                    self.audio_credit_gap_events
                ),
                "ps2_audio_queue": (
                    ps2_audio_queue
                ),
                "ps2_audio_enqueued": (
                    ps2_audio_enqueued
                ),
                "ps2_audio_played": (
                    ps2_audio_played
                ),
                "ps2_frames_received": (
                    ps2_frames
                ),
            }

            print(
                "AUDIO_DIAG="
                + json.dumps(
                    snapshot,
                    sort_keys=True,
                ),
                flush=True,
            )

    def run(self) -> None:
        diag_thread = threading.Thread(
            name="audio-boundary-diag",
            target=self.audio_diag_loop,
            daemon=True,
        )

        diag_thread.start()

        try:
            super().run()
        finally:
            self.stop_event.set()
            diag_thread.join(timeout=1.0)


def main() -> int:
    # base.serve() resolves this global when it accepts a session.
    # Replacing it here leaves mux_daemon.py itself untouched.
    base.MuxSession = InstrumentedMuxSession

    return base.main()


if __name__ == "__main__":
    raise SystemExit(main())
