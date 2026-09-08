#!/usr/bin/env python3
"""
File synopsis:
    Pi-only H1 combined-media diagnostic that preserves joint AUDIO+MPEG
    startup prefill and then schedules the more-depleted PS2 transport reservoir
    first instead of using a fixed AUDIO:MPEG service ratio.

Purpose:
    Fixed scheduler experiments bracketed the combined-media behavior:
      * 1 AUDIO : 1 MPEG kept the AUDIO reservoir healthy and completed media.
      * 1 AUDIO : 2 MPEG with 8 KiB slots hard-froze the PS2.
      * 4 KiB AUDIO / 8 KiB MPEG alternating avoided the hard runtime freeze,
        but AUDIO repeatedly drained to zero and was audibly choppy.

    H1 credit accounting already exposes a local estimate of queue depletion.
    Each channel starts with credit equal to its PS2 queue capacity. Sending DATA
    spends credit; PS2 consumption returns credit. Therefore, ignoring small
    network in-flight effects, capacity - current_credit is a conservative
    estimate of bytes still outstanding toward that PS2 channel.

Mechanism:
    * Startup remains AUDIO,MPEG 1:1 until both configured startup byte targets
      have been transmitted, matching the previous joint-prefill diagnostics.
    * Steady state computes normalized outstanding fill for AUDIO and MPEG from
      current channel credit.
    * The channel with the lower normalized fill is serviced first.
    * AUDIO uses a 4096-byte quantum, matching its PS2 playback chunk and credit
      batch. MPEG uses an 8192-byte quantum.
    * If the preferred channel lacks producer bytes or credit, the other channel
      gets an immediate opportunity.
    * One existing HEARTBEAT per second records PS2 queue telemetry.

No PS2 source, physical TCP topology, queue capacity, credit policy, codec,
AUDSRV behavior, thread priority, MPEG feed size, or playback configuration is
changed.
"""

from __future__ import annotations

import threading

import h1_mux_server_post_result_eof as eof_fix

base = eof_fix.base

AUDIO_QUANTUM = 4096
MPEG_QUANTUM = 8192


def _telemetry_loop(session: base.H1Session, stop: threading.Event) -> None:
    sample = 0
    while not stop.wait(1.0):
        if session.stop_event.is_set():
            return
        try:
            session.send_frame(base.FRAME_HEARTBEAT, base.CHANNEL_CONTROL)
            sample += 1
            print(f"H1_RESERVOIR_HEARTBEAT_SENT={sample}", flush=True)
        except BaseException as exc:
            print(
                "H1_RESERVOIR_HEARTBEAT_STOP="
                f"{type(exc).__name__}:{exc}",
                flush=True,
            )
            return


def _send_audio(self: base.H1Session, startup: bool = False) -> bool:
    if self.audio_producer is None:
        return False
    return self._send_from(
        self.audio_producer,
        self.audio,
        base.MAX_PAYLOAD if startup else AUDIO_QUANTUM,
        4,
    )


def _send_mpeg(self: base.H1Session, startup: bool = False) -> bool:
    return self._send_from(
        self.video_producer,
        self.mpeg,
        base.MAX_PAYLOAD if startup else MPEG_QUANTUM,
        1,
    )


def _outstanding_bytes(
    self: base.H1Session,
    state: base.ChannelState,
    capacity: int,
) -> int:
    with self.condition:
        credit = state.credit

    # Defensive clamping keeps a transient accounting anomaly from becoming a
    # scheduler inversion. Normal H1 credit flow should remain within bounds.
    if credit < 0:
        credit = 0
    if credit > capacity:
        credit = capacity
    return capacity - credit


def _preferred_channels(self: base.H1Session) -> tuple[int, int]:
    audio_capacity = self.profile["audio_queue_capacity"]
    mpeg_capacity = self.profile["mpeg_queue_capacity"]

    audio_outstanding = _outstanding_bytes(
        self,
        self.audio,
        audio_capacity,
    )
    mpeg_outstanding = _outstanding_bytes(
        self,
        self.mpeg,
        mpeg_capacity,
    )

    # Compare normalized fill without floating point:
    # audio_outstanding/audio_capacity <= mpeg_outstanding/mpeg_capacity
    # means AUDIO is at least as depleted as MPEG and should go first.
    if (
        audio_outstanding * mpeg_capacity
        <= mpeg_outstanding * audio_capacity
    ):
        return (base.CHANNEL_AUDIO, base.CHANNEL_MPEG2)

    return (base.CHANNEL_MPEG2, base.CHANNEL_AUDIO)


def _schedule_media_reservoir(self: base.H1Session) -> None:
    if self.video_producer is None:
        raise RuntimeError("video producer not started")
    if self.audio_producer is None:
        raise RuntimeError("reservoir diagnostic requires PCM audio")

    audio_target = self.profile["audio_start_target_bytes"]
    mpeg_target = self.profile["mpeg_start_target_bytes"]

    if self.profile["audio_start_mode"] != 1:
        raise base.ProtocolError(
            "reservoir diagnostic requires audio_start_mode=TARGET"
        )
    if audio_target == 0 or mpeg_target == 0:
        raise base.ProtocolError("reservoir startup targets must be nonzero")
    if audio_target != mpeg_target:
        raise base.ProtocolError(
            "reservoir diagnostic requires equal AUDIO/MPEG startup targets"
        )

    startup_schedule = (base.CHANNEL_AUDIO, base.CHANNEL_MPEG2)
    startup_index = 0
    startup_complete = False

    telemetry_stop = threading.Event()
    telemetry = threading.Thread(
        target=_telemetry_loop,
        args=(self, telemetry_stop),
        name="h1-reservoir-telemetry",
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
                    "H1_RESERVOIR_PREFILL_COMPLETE "
                    f"audio_sent={self.audio.bytes_sent} "
                    f"mpeg_sent={self.mpeg.bytes_sent} "
                    f"target={audio_target}",
                    flush=True,
                )

            sent = False

            if not startup_complete:
                for _ in range(len(startup_schedule)):
                    channel = startup_schedule[startup_index]
                    startup_index = (
                        startup_index + 1
                    ) % len(startup_schedule)

                    if channel == base.CHANNEL_AUDIO:
                        sent = _send_audio(self, startup=True)
                    else:
                        sent = _send_mpeg(self, startup=True)

                    if sent:
                        break
            else:
                preferred = _preferred_channels(self)
                for channel in preferred:
                    if channel == base.CHANNEL_AUDIO:
                        sent = _send_audio(self)
                    else:
                        sent = _send_mpeg(self)

                    if sent:
                        break

            if not sent:
                with self.condition:
                    self.condition.wait(timeout=0.001)
    finally:
        telemetry_stop.set()
        telemetry.join(timeout=2.0)


base.H1Session.schedule_media = _schedule_media_reservoir


if __name__ == "__main__":
    raise SystemExit(base.main())
