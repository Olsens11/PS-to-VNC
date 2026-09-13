#!/usr/bin/env python3
"""Generate a CP2P cancellation-boundary diagnostic.

This derivative starts from the existing persistent MPEG witness runtime and
changes exactly one cancellation behavior inside the libmpeg data callback.
When the transport's cancellable MPEG read observes stop_requested and returns
false, the callback records a dedicated stage and deliberately parks the MPEG
worker with SleepThread() instead of returning 0 to libmpeg.

Purpose:
    distinguish the transport-read cancellation path itself from the act of
    returning 0 through SMS libmpeg's data callback. If RFB/PCM/receiver service
    continues while the worker is parked at D5C00001, then stop_requested and
    pstvnc_h1_transport_mpeg_read_cancellable() are safe, and the global freeze
    begins only when the callback returns 0 into libmpeg. If the system still
    globally freezes before the parked marker is observable, the defect is
    earlier inside the cancellable-read path.

This is intentionally a disposable diagnostic. It does not attempt teardown,
worker completion, graphics clear, queue finalization, or owner transition.
No queue size, priority, pacing, decoder setup, DMA policy, RFB policy, or PCM
policy is changed.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import generate_h1_video_runtime_cp2p as basegen
import generate_h1_video_runtime_cp2p_persistent_diag as persistentgen


PARK_MARKER = "0xD5C00001u"
UNEXPECTED_WAKE_MARKER = "0xD5C0FF01u"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def generate(source: str) -> str:
    out = persistentgen.generate(source)

    old = """    if (!pstvnc_h1_transport_mpeg_read_cancellable(
            session->transport,
            session->feed_buffer,
            session->config->mpeg_feed_bytes,
            &payload_size,
            session->stop_requested)) {
        if (session->stop_requested != NULL && *session->stop_requested)
            session->cancelled = 1;
        h1_diag_stage(session, 0xD5FF0001u); /* read returned false */
        return 0;
    }
"""

    new = """    if (!pstvnc_h1_transport_mpeg_read_cancellable(
            session->transport,
            session->feed_buffer,
            session->config->mpeg_feed_bytes,
            &payload_size,
            session->stop_requested)) {
        if (session->stop_requested != NULL && *session->stop_requested) {
            session->cancelled = 1;

            /*
             * Experimental boundary: prove the cancellable read can return
             * safely without handing a zero return value back into libmpeg.
             * SleepThread yields the EE completely and is intentionally never
             * woken in this disposable diagnostic.
             */
            h1_diag_stage(session, 0xD5C00001u);
            (void)SleepThread();

            /* Reaching here would mean an unexpected external wakeup. */
            h1_diag_stage(session, 0xD5C0FF01u);
        }

        h1_diag_stage(session, 0xD5FF0001u); /* read returned false */
        return 0;
    }
"""

    out = replace_once(
        out,
        old,
        new,
        "cancelled feed callback park",
    )

    required = (
        PARK_MARKER,
        UNEXPECTED_WAKE_MARKER,
        "(void)SleepThread();",
        "pstvnc_h1_transport_mpeg_read_cancellable(",
        "0xD5000002u",
    )
    for marker in required:
        if marker not in out:
            raise RuntimeError(
                f"cancel-callback-park generation missing marker: {marker}"
            )

    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)
    data = input_path.read_bytes()
    actual_sha = basegen.git_blob_sha1(data)
    if actual_sha != basegen.EXPECTED_INPUT_BLOB_SHA1:
        raise SystemExit(
            "H1_CP2P_VIDEO_SOURCE_MISMATCH="
            f"expected:{basegen.EXPECTED_INPUT_BLOB_SHA1}:actual:{actual_sha}"
        )

    generated = generate(data.decode("utf-8"))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(generated, encoding="utf-8")

    print(f"H1_CP2P_VIDEO_SOURCE_BLOB={actual_sha}")
    print("H1_CP2P_MPEG_STALL_DIAGNOSTICS=PASS")
    print("H1_CP2P_MPEG_PERSISTENT_WITNESS=PASS")
    print("H1_CP2P_CANCEL_CALLBACK_PARK=PASS")
    print("H1_CP2P_CANCEL_CALLBACK_PARK_STAGE=0xD5C00001")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
