#!/usr/bin/env python3
"""
File synopsis:
    Pi-only H1 combined-media diagnostic that gives AUDIO and MPEG equal
    startup service until both have each transmitted their configured startup
    target, then restores the qualified steady-state 1 AUDIO : 3 MPEG scheduler.

Purpose:
    AUDIO and MPEG share one PS2 receiver thread. The normal H1 Pi scheduler is
    byte-weighted 1:3 (AUDIO:MPEG), so MPEG can reach its 448 KiB prefill while
    AUDIO is still building its reservoir. This diagnostic tests whether that
    startup competition is responsible for the combined AUDSRV+MPEG failure.

Required CONFIG for the intended test:
    audio_start_mode=TARGET
    audio_start_target_bytes == mpeg_start_target_bytes

Mechanism:
    * During startup, schedule AUDIO,MPEG 1:1.
    * Declare startup complete only after both ChannelState.bytes_sent values
      have reached their configured startup targets.
    * Thereafter use the original AUDIO,MPEG,MPEG,MPEG schedule unchanged.
    * Send one existing HEARTBEAT per second for PS2 queue telemetry.

No PS2 source, queue capacity, credit policy, codec, AUDSRV behavior, thread
priority, or steady-state transport weighting is changed.
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
            print(f"H1_JOINT_PREFILL_HEARTBEAT_SENT={sample}", flush=True)
        except BaseException as exc:
            print(
                "H1_JOINT_PREFILL_HEARTBEAT_STOP="
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


def _schedule_media_joint_prefill(self: base.H1Session) -> None:
    if self.video_producer is None:
        raise RuntimeError("video producer not started")
    if self.audio_producer is None:
        raise RuntimeError("joint-prefill diagnostic requires PCM audio")

    audio_target = self.profile["audio_start_target_bytes"]
    mpeg_target = self.profile["mpeg_start_target_bytes"]

    if self.profile["audio_start_mode"] != 1:
        raise base.ProtocolError(
            "joint-prefill diagnostic requires audio_start_mode=TARGET"
        )
    if audio_target == 0 or mpeg_target == 0:
        raise base.ProtocolError("joint-prefill targets must be nonzero")
    if audio_target != mpeg_target:
        raise base.ProtocolError(
            "joint-prefill diagnostic requires equal AUDIO/MPEG startup targets"
        )

    startup_schedule = (base.CHANNEL_AUDIO, base.CHANNEL_MPEG2)
    steady_schedule = (
        base.CHANNEL_AUDIO,
        base.CHANNEL_MPEG2,
        base.CHANNEL_MPEG2,
        base.CHANNEL_MPEG2,
    )
    startup_index = 0
    steady_index = 0
    startup_complete = False

    telemetry_stop = threading.Event()
    telemetry = threading.Thread(
        target=_telemetry_loop,
        args=(self, telemetry_stop),
        name="h1-joint-prefill-telemetry",
        daemon=True,
    )
    telemetry.start()

    try:
        while True:
            self.check_reader()
            self.video_producer.check()
            self.audio_producer.check()

            if (
                self.video_producer.eof
                and not self.audio_stop_requested
            ):
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
                    "H1_JOINT_PREFILL_COMPLETE "
                    f"audio_sent={self.audio.bytes_sent} "
                    f"mpeg_sent={self.mpeg.bytes_sent} "
                    f"target={audio_target}",
                    flush=True,
                )

            schedule = steady_schedule if startup_complete else startup_schedule
            if startup_complete:
                schedule_index = steady_index
            else:
                schedule_index = startup_index

            sent = False
            for _ in range(len(schedule)):
                channel = schedule[schedule_index]
                schedule_index = (schedule_index + 1) % len(schedule)

                sent = _send_channel(self, channel)
                if sent:
                    break

            if startup_complete:
                steady_index = schedule_index
            else:
                startup_index = schedule_index

            if not sent:
                with self.condition:
                    self.condition.wait(timeout=0.001)
    finally:
        telemetry_stop.set()
        telemetry.join(timeout=2.0)


base.H1Session.schedule_media = _schedule_media_joint_prefill


if __name__ == "__main__":
    raise SystemExit(base.main())
