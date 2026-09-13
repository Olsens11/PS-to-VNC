#!/usr/bin/env python3
"""Generate a CP2P MPEG runtime that stops only at a completed picture boundary.

This diagnostic is a clean-room behavioral adaptation of a mature decoder-control
pattern: request stop while the producer remains live, allow the currently-active
MPEG_Picture() call to complete with ordinary data, observe the stop between
picture calls, then destroy decoder ownership. It intentionally does not copy
SMS_MPEG12.c, Sony sceMpeg code, or any commercial player implementation.

The key A/B against the failed callback-cancellation path is that stop_requested
is NOT consulted by the libmpeg data callback. Therefore an externally-requested
stop can never be translated into a synthetic callback EOF in the middle of a
picture decode. The Pi producer remains live until the worker has returned; only
then does the coordinator perform exact-generation producer retirement.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import generate_h1_video_runtime_cp2p as basegen
import generate_h1_video_runtime_cp2p_mature_lifecycle as parent


def generate(source: str) -> str:
    out = parent.generate(source)

    # Preserve all existing feed mechanics and diagnostics, but make the feed
    # callback independent of the asynchronous worker stop flag. The producer
    # remains live until the worker has reached a completed-picture boundary.
    out = basegen.replace_function(
        out,
        "static int h1_video_feed_ipu(void *user_data)",
        """static int h1_video_feed_ipu(void *user_data)
{
    h1_video_session_t *session = (h1_video_session_t *)user_data;
    size_t payload_size = 0u;
    uint32_t payload_bytes;
    uint32_t dma_bytes;
    uint32_t dma_qwc;

    if (session == NULL ||
        session->transport == NULL ||
        session->feed_buffer == NULL)
        return 0;

    h1_diag_stage(session, 0xD5000001u); /* feed enter */
    h1_diag_stage(session, 0xD5000002u); /* before queue read */
    if (!pstvnc_h1_transport_mpeg_read(
            session->transport,
            session->feed_buffer,
            session->config->mpeg_feed_bytes,
            &payload_size)) {
        h1_diag_stage(session, 0xD5FF0001u); /* ordinary stream read ended */
        return 0;
    }
    h1_diag_stage(session, 0xD5000003u); /* queue read returned */

    if (payload_size == 0u ||
        payload_size > session->config->mpeg_feed_bytes) {
        h1_diag_stage(session, 0xD5FF0002u);
        return 0;
    }

    payload_bytes = (uint32_t)payload_size;
    dma_bytes = (payload_bytes + 15u) & ~15u;

    if ((size_t)dma_bytes > session->feed_capacity) {
        h1_diag_stage(session, 0xD5FF0003u);
        return 0;
    }

    if (dma_bytes > payload_bytes) {
        memset(
            session->feed_buffer + payload_bytes,
            0,
            dma_bytes - payload_bytes);
    }

    h1_diag_stage(session, 0xD5000004u); /* before TO_IPU DMA wait */
    if (dma_channel_wait(DMA_CHANNEL_toIPU, 0) != 0) {
        h1_diag_stage(session, 0xD5FF0004u);
        return 0;
    }
    h1_diag_stage(session, 0xD5000005u); /* TO_IPU DMA wait returned */

    dma_qwc = dma_bytes >> 4;
    dma_channel_send_normal(
        DMA_CHANNEL_toIPU,
        session->feed_buffer,
        dma_qwc,
        0,
        0);
    h1_diag_stage(session, 0xD5000006u); /* TO_IPU DMA submitted */

    session->result->feed_calls += 1u;
    session->result->payload_bytes_submitted += payload_bytes;
    session->result->dma_bytes_submitted += dma_bytes;
    h1_diag_stage(session, 0xD5000007u); /* feed return success */
    return 1;
}""",
    )

    # The mature control point is between completed MPEG_Picture() calls. The
    # first picture cannot carry a stop request because the two-second timer does
    # not start until MPEG_OWNED, so the loop boundary is the active stop point.
    old = """        result->pictures_decoded += 1u;

        if (!h1_video_wait_for_picture(
"""
    new = """        result->pictures_decoded += 1u;

        if (session.stop_requested != NULL && *session.stop_requested) {
            h1_diag_stage(&session, 0xD8300001u); /* stop at completed picture */
            session.cancelled = 1;
            success = 1;
            goto done;
        }

        if (!h1_video_wait_for_picture(
"""
    out = basegen.replace_once(
        out,
        old,
        new,
        "completed-picture stop boundary",
    )

    required = (
        "0xD8300001u",
        "pstvnc_h1_transport_mpeg_read(",
        "MPEG_Destroy();",
        "0xD8200002u",
        "h1_video_reference_ipu_reset(session);",
    )
    for marker in required:
        if marker not in out:
            raise RuntimeError(f"boundary-stop generation missing marker: {marker}")

    feed_start = out.find("static int h1_video_feed_ipu(void *user_data)")
    feed_end = out.find("static void *h1_video_sequence_init(", feed_start)
    if feed_start < 0 or feed_end < 0:
        raise RuntimeError("could not isolate generated MPEG feed callback")

    feed = out[feed_start:feed_end]
    if "pstvnc_h1_transport_mpeg_read_cancellable(" in feed:
        raise RuntimeError("boundary-stop feed still observes asynchronous stop")
    if "session->stop_requested" in feed:
        raise RuntimeError("boundary-stop feed still references stop_requested")

    run_start = out.find("int pstvnc_h1_video_run_cp2p_session(")
    run_end = out.find("int pstvnc_h1_video_cp2p_retire_presentation(", run_start)
    release_start = out.find("static void h1_video_release_session(")
    release_end = out.find("int pstvnc_h1_video_chassis_init", release_start)
    if run_start < 0 or run_end < 0 or release_start < 0 or release_end < 0:
        raise RuntimeError("could not isolate generated boundary-stop sections")

    run = out[run_start:run_end]
    release = out[release_start:release_end]
    if "0xD8300001u" not in run:
        raise RuntimeError("completed-picture stop marker missing from run loop")
    if "0xD8200001u" not in release or "MPEG_Destroy();" not in release:
        raise RuntimeError("mature decoder release missing from release_session")

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
    print("H1_CP2P_MATURE_LIFECYCLE=PASS")
    print("H1_CP2P_BOUNDARY_STOP=PASS")
    print("H1_CP2P_STOP_IN_DATA_CALLBACK=ABSENT")
    print("H1_CP2P_STOP_POINT=BETWEEN_COMPLETED_MPEG_PICTURE_CALLS")
    print("H1_CP2P_MATURE_ORDER=MPEG_DESTROY_THEN_LOCAL_IPU_RESET")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
