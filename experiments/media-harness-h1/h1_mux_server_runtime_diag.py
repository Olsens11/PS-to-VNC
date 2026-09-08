#!/usr/bin/env python3
"""
File synopsis:
    Narrow Pi-side diagnostic wrapper for H1 combined AUDIO + MPEG sessions.

This wrapper preserves CONFIG and media scheduling. While the base scheduler is
running, one daemon thread sends the already-supported PSTV HEARTBEAT control
frame once per second. The resident PS2 responds with its existing telemetry
snapshot, exposing queue depths and consumption/playback counters without an
ELF rebuild.

The post-result EOF correction is imported first so normal resident-session
socket retirement remains classified correctly.
"""

from __future__ import annotations

import threading
import time

import h1_mux_server_post_result_eof as eof_fix

base = eof_fix.base

_ORIGINAL_SCHEDULE_MEDIA = base.H1Session.schedule_media


def _telemetry_loop(session: base.H1Session, stop: threading.Event) -> None:
    sample = 0

    while not stop.wait(1.0):
        if session.stop_event.is_set():
            return

        try:
            session.send_frame(
                base.FRAME_HEARTBEAT,
                base.CHANNEL_CONTROL,
            )
            sample += 1
            print(f"H1_DIAG_HEARTBEAT_SENT={sample}", flush=True)
        except BaseException as exc:
            # The reader owns authoritative transport failure classification.
            # A send racing with a peer reset is useful evidence but should not
            # replace the original exception path.
            print(
                "H1_DIAG_HEARTBEAT_STOP="
                f"{type(exc).__name__}:{exc}",
                flush=True,
            )
            return


def _schedule_media_with_telemetry(self: base.H1Session) -> None:
    stop = threading.Event()
    thread = threading.Thread(
        target=_telemetry_loop,
        args=(self, stop),
        name="h1-runtime-telemetry",
        daemon=True,
    )

    thread.start()
    try:
        _ORIGINAL_SCHEDULE_MEDIA(self)
    finally:
        stop.set()
        thread.join(timeout=2.0)


base.H1Session.schedule_media = _schedule_media_with_telemetry


if __name__ == "__main__":
    raise SystemExit(base.main())
