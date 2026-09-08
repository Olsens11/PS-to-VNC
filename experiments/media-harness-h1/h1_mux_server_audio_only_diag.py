#!/usr/bin/env python3
"""
File synopsis:
    Pi-only H1 bisection mode for PCM audio on the same physical PSTV mux.

Purpose:
    The first combined MPEG+PCM H1 run froze the PS2 deeply enough that the
    peer stopped completing TCP close/ACK progress. This wrapper removes only
    MPEG production/presentation while preserving the H1 mux transport and the
    qualified PCM CONFIG/runtime path. It therefore distinguishes an audio-side
    failure from one requiring concurrent audio+video activity.

The wrapper also sends one HEARTBEAT per second so the existing PS2 telemetry
counters remain visible. No PS2 source or ELF change is required.
"""

from __future__ import annotations

import json
import struct
import subprocess
import threading
import time

import h1_mux_server_post_result_eof as eof_fix

base = eof_fix.base


def _telemetry_loop(session: base.H1Session, stop: threading.Event) -> None:
    sample = 0
    while not stop.wait(1.0):
        if session.stop_event.is_set():
            return
        try:
            session.send_frame(base.FRAME_HEARTBEAT, base.CHANNEL_CONTROL)
            sample += 1
            print(f"H1_AUDIO_ONLY_HEARTBEAT_SENT={sample}", flush=True)
        except BaseException as exc:
            print(
                "H1_AUDIO_ONLY_HEARTBEAT_STOP="
                f"{type(exc).__name__}:{exc}",
                flush=True,
            )
            return


def _start_audio_only(self: base.H1Session) -> None:
    if self.profile["audio_mode"] != base.AUDIO_PCM:
        raise base.ProtocolError("audio-only diagnostic requires PCM audio")
    if self.profile["video_mode"] != 0:
        raise base.ProtocolError("audio-only diagnostic requires video_mode=0")

    audio_command = self.audio_command()
    print("H1_AUDIO_COMMAND=" + json.dumps(audio_command), flush=True)
    process = subprocess.Popen(
        audio_command,
        stdout=subprocess.PIPE,
        stderr=None,
        bufsize=0,
        start_new_session=True,
    )
    self.audio_producer = base.ProducerBuffer(
        "audio",
        process,
        8192,
        self.evidence / "audio.pcm",
    )
    self.video_producer = None


def _schedule_audio_only(self: base.H1Session) -> None:
    if self.audio_producer is None:
        raise RuntimeError("audio producer not started")

    telemetry_stop = threading.Event()
    telemetry = threading.Thread(
        target=_telemetry_loop,
        args=(self, telemetry_stop),
        name="h1-audio-only-telemetry",
        daemon=True,
    )
    telemetry.start()

    capture_deadline = time.monotonic() + self.duration

    try:
        while True:
            self.check_reader()
            self.audio_producer.check()

            if (
                not self.audio_stop_requested
                and time.monotonic() >= capture_deadline
            ):
                self.audio_stop_requested = True
                self.audio_producer.stop()
                print("H1_AUDIO_ONLY_STOP_REQUEST=DURATION", flush=True)

            if self.audio_producer.done_and_empty():
                return

            sent = self._send_from(
                self.audio_producer,
                self.audio,
                base.MAX_PAYLOAD,
                4,
            )

            if not sent:
                with self.condition:
                    self.condition.wait(timeout=0.001)
    finally:
        telemetry_stop.set()
        telemetry.join(timeout=2.0)


def _send_audio_only_media_end(self: base.H1Session) -> dict[str, int]:
    words = [
        base.MEDIA_END_VERSION,
        self.profile["session_id"],
        self.audio.bytes_sent,
        self.audio.frames_sent,
        self.audio.last_data_sequence,
        self.audio.crc32,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        base.STOP_REASON_FINITE_DURATION,
        0,
        0,
    ]
    payload = struct.pack(">16I", *words)

    metadata = {
        "session_id": self.profile["session_id"],
        "audio_bytes": self.audio.bytes_sent,
        "audio_frames": self.audio.frames_sent,
        "audio_last_sequence": self.audio.last_data_sequence,
        "audio_crc32": self.audio.crc32,
        "mpeg_bytes": 0,
        "mpeg_frames": 0,
        "mpeg_last_sequence": 0,
        "mpeg_crc32": 0,
        "picture_starts": 0,
        "sequence_headers": 0,
        "sequence_ends": 0,
        "stop_reason": base.STOP_REASON_FINITE_DURATION,
    }

    self.send_frame(base.FRAME_MEDIA_END, base.CHANNEL_CONTROL, payload)
    print("H1_MEDIA_END_SENT=" + json.dumps(metadata, sort_keys=True), flush=True)
    return metadata


base.H1Session.start_producers = _start_audio_only
base.H1Session.schedule_media = _schedule_audio_only
base.H1Session.send_media_end = _send_audio_only_media_end


if __name__ == "__main__":
    raise SystemExit(base.main())
