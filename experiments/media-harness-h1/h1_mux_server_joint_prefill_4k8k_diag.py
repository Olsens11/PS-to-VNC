#!/usr/bin/env python3
"""
File synopsis:
    Pi-only H1 combined-media diagnostic that preserves joint AUDIO+MPEG
    startup prefill, then alternates 4 KiB AUDIO and 8 KiB MPEG DATA frames.

Purpose:
    The 1 AUDIO : 2 MPEG slot scheduler reproduced the hard PS2 stall, while
    1:1 service survived the media phase. Both used 8 KiB DATA opportunities.
    This diagnostic preserves approximately the same 1:2 byte allocation as
    the failing slot-weighted test while halving the maximum MPEG burst between
    AUDIO service opportunities.

Mechanism:
    * Startup remains AUDIO,MPEG 1:1 until both configured targets are sent.
    * Startup DATA quantum remains the base 8 KiB behavior.
    * Sustained streaming alternates AUDIO,MPEG.
    * AUDIO sustained quantum is 4096 bytes, matching the PS2 audio chunk and
      credit batch.
    * MPEG sustained quantum remains the 8192-byte PSTV maximum.
    * Empty/no-credit turns are skipped exactly as in the existing schedulers.
    * Existing HEARTBEAT telemetry is requested once per second.

No PS2 source, queue capacity, credit policy, codec, AUDSRV behavior, thread
priority, MPEG feed size, or physical transport topology changes.
"""

from __future__ import annotations

import threading

import h1_mux_server_post_result_eof as eof_fix

base = eof_fix.base

AUDIO_STEADY_QUANTUM = 4096
MPEG_STEADY_QUANTUM = base.MAX_PAYLOAD


def _telemetry_loop(session: base.H1Session, stop: threading.Event) -> None:
    sample = 0
    while not stop.wait(1.0):
        if session.stop_event.is_set():
            return
        try:
            session.send_frame(base.FRAME_HEARTBEAT, base.CHANNEL_CONTROL)
            sample += 1
            print(f"H1_4K8K_HEARTBEAT_SENT={sample}", flush=True)
        except BaseException as exc:
            print(
                "H1_4K8K_HEARTBEAT_STOP="
                f"{type(exc).__name__}:{exc}",
                flush=True,
            )
            return


def _send_startup_channel(self: base.H1Session, channel: int) -> bool:
    if channel == base.CHANNEL_AUDIO:
        if self.audio_producer is None:
            return False
        return self._send_from(
            self.audio_producer,
            self.audio,
            base.MAX_PAYLOAD,
            4,
        )

    return self._send_from(
        self.video_producer,
        self.mpeg,
        base.MAX_PAYLOAD,
        1,
    )


def _send_steady_channel(self: base.H1Session, channel: int) -> bool:
    if channel == base.CHANNEL_AUDIO:
        if self.audio_producer is None:
            return False
        return self._send_from(
            self.audio_producer,
            self.audio,
            AUDIO_STEADY_QUANTUM,
            4,
        )

    return self._send_from(
        self.video_producer,
        self.mpeg,
        MPEG_STEADY_QUANTUM,
        1,
    )


def _schedule_media_4k8k(self: base.H1Session) -> None:
    if self.video_producer is None:
        raise RuntimeError("video producer not started")
    if self.audio_producer is None:
        raise RuntimeError("4K8K diagnostic requires PCM audio")

    audio_target = self.profile["audio_start_target_bytes"]
    mpeg_target = self.profile["mpeg_start_target_bytes"]

    if self.profile["audio_start_mode"] != 1:
        raise base.ProtocolError(
            "4K8K diagnostic requires audio_start_mode=TARGET"
        )
    if audio_target == 0 or mpeg_target == 0:
        raise base.ProtocolError("4K8K startup targets must be nonzero")
    if audio_target != mpeg_target:
        raise base.ProtocolError(
            "4K8K diagnostic requires equal AUDIO/MPEG startup targets"
        )

    schedule = (base.CHANNEL_AUDIO, base.CHANNEL_MPEG2)
    schedule_index = 0
    startup_complete = False

    telemetry_stop = threading.Event()
    telemetry = threading.Thread(
        target=_telemetry_loop,
        args=(self, telemetry_stop),
        name="h1-4k8k-telemetry",
        daemon=True,
    )
    telemetry.start()

    try:
        while True:
            self.check_reader()
            self.video_producer.check()
            self.audio_producer.check()

            if self.video_producer.eof and not self.audio_stop_requested:
                self.audio_stop_requested = True
                self.audio_producer.stop()
                print("H1_AUDIO_STOP_REQUEST=VIDEO_EOF", flush=True)

            video_done = self.video_producer.done_and_empty()
            audio_done = self.audio_producer.done_and_empty()
            if video_done and audio_done:
                return

            if (
                not startup_complete
                and self.audio.bytes_sent >= audio_target
                and self.mpeg.bytes_sent >= mpeg_target
            ):
                startup_complete = True
                print(
                    "H1_4K8K_PREFILL_COMPLETE "
                    f"audio_sent={self.audio.bytes_sent} "
                    f"mpeg_sent={self.mpeg.bytes_sent} "
                    f"target={audio_target}",
                    flush=True,
                )

            sent = False
            for _ in range(len(schedule)):
                channel = schedule[schedule_index]
                schedule_index = (schedule_index + 1) % len(schedule)

                if startup_complete:
                    sent = _send_steady_channel(self, channel)
                else:
                    sent = _send_startup_channel(self, channel)

                if sent:
                    break

            if not sent:
                with self.condition:
                    self.condition.wait(timeout=0.001)
    finally:
        telemetry_stop.set()
        telemetry.join(timeout=2.0)


base.H1Session.schedule_media = _schedule_media_4k8k


if __name__ == "__main__":
    raise SystemExit(base.main())
