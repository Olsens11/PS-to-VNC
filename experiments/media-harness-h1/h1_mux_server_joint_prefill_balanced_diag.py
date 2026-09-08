#!/usr/bin/env python3
"""
File synopsis:
    Pi-only H1 combined-media diagnostic that preserves the successful joint
    AUDIO+MPEG startup prefill and then uses equal AUDIO/MPEG service
    opportunity during sustained streaming.

Purpose:
    The joint-prefill run proved both PS2 queues can start near 448 KiB, but the
    original steady 1 AUDIO : 3 MPEG scheduler then drained both reservoirs and
    left about 1 MiB of captured PCM unsent while all MPEG bytes were delivered.
    This diagnostic changes only steady-state Pi scheduling fairness.

Mechanism:
    * AUDIO and MPEG use equal configured startup targets.
    * Startup schedule is AUDIO,MPEG (1:1) until both ChannelState.bytes_sent
      counters reach the common target.
    * Steady-state schedule remains AUDIO,MPEG (1:1).
    * A turn whose producer lacks bytes or credit is skipped, so equal service
      opportunity does not impose equal bandwidth or prevent MPEG from using
      otherwise-idle AUDIO opportunities.
    * Existing HEARTBEAT telemetry is requested once per second.

No PS2 source, queue capacity, credit policy, codec, AUDSRV behavior, thread
priority, chunk size, MPEG feed size, or physical transport topology changes.
"""

from __future__ import annotations

import threading

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
            print(f"H1_BALANCED_HEARTBEAT_SENT={sample}", flush=True)
        except BaseException as exc:
            print(
                "H1_BALANCED_HEARTBEAT_STOP="
                f"{type(exc).__name__}:{exc}",
                flush=True,
            )
            return


def _send_channel(self: base.H1Session, channel: int) -> bool:
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


def _schedule_media_balanced(self: base.H1Session) -> None:
    if self.video_producer is None:
        raise RuntimeError("video producer not started")
    if self.audio_producer is None:
        raise RuntimeError("balanced diagnostic requires PCM audio")

    audio_target = self.profile["audio_start_target_bytes"]
    mpeg_target = self.profile["mpeg_start_target_bytes"]

    if self.profile["audio_start_mode"] != 1:
        raise base.ProtocolError(
            "balanced diagnostic requires audio_start_mode=TARGET"
        )
    if audio_target == 0 or mpeg_target == 0:
        raise base.ProtocolError("balanced startup targets must be nonzero")
    if audio_target != mpeg_target:
        raise base.ProtocolError(
            "balanced diagnostic requires equal AUDIO/MPEG startup targets"
        )

    schedule = (base.CHANNEL_AUDIO, base.CHANNEL_MPEG2)
    schedule_index = 0
    startup_complete = False

    telemetry_stop = threading.Event()
    telemetry = threading.Thread(
        target=_telemetry_loop,
        args=(self, telemetry_stop),
        name="h1-balanced-telemetry",
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
                    "H1_BALANCED_PREFILL_COMPLETE "
                    f"audio_sent={self.audio.bytes_sent} "
                    f"mpeg_sent={self.mpeg.bytes_sent} "
                    f"target={audio_target}",
                    flush=True,
                )

            sent = False
            for _ in range(len(schedule)):
                channel = schedule[schedule_index]
                schedule_index = (schedule_index + 1) % len(schedule)
                sent = _send_channel(self, channel)
                if sent:
                    break

            if not sent:
                with self.condition:
                    self.condition.wait(timeout=0.001)
    finally:
        telemetry_stop.set()
        telemetry.join(timeout=2.0)


base.H1Session.schedule_media = _schedule_media_balanced


if __name__ == "__main__":
    raise SystemExit(base.main())
