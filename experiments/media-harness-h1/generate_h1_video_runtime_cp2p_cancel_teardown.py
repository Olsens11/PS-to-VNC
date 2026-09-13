#!/usr/bin/env python3
"""Generate the persistent CP2P MPEG witness with cancellation-safe teardown.

This is a one-variable derivative of the priority-67 persistent-witness runtime.
All ordinary decode, queue, scheduler, RFB, PCM, and non-cancelled teardown policy
is preserved. The only behavioral change is the teardown path entered after the
MPEG worker's explicit stop request has made the feed callback report
``session->cancelled``.

The inherited runtime waits for DMA_CHANNEL_toIPU with ``timeout == 0`` before
MPEG_Destroy(). PS2SDK defines that form as an unbounded tight poll of the DMA
channel's STR bit. For an intentionally cancelled generation, the Pi producer
has already been retired and no further MPEG feed is expected. This derivative
therefore uses the already-existing CP2P IPU reset helper on that cancellation
path to stop/reset the IPU DMA channels before MPEG_Destroy(), rather than
waiting forever for an in-flight transfer to quiesce naturally.

Normal/non-cancelled teardown keeps the original infinite wait byte-for-byte.
Additional D81 markers identify entry/return of the cancellation reset branch.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import generate_h1_video_runtime_cp2p as basegen
import generate_h1_video_runtime_cp2p_persistent_diag as parent


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def generate(source: str) -> str:
    out = parent.generate(source)

    old = """    h1_diag_stage(session, 0xD8000001u); /* before teardown TO_IPU wait */
    dma_channel_wait(DMA_CHANNEL_toIPU, 0);
    h1_diag_stage(session, 0xD8000002u); /* teardown TO_IPU wait returned */

    if (session->mpeg_initialized) {
"""

    new = """    if (session->cancelled) {
        /*
         * Explicit cancellation is not an end-of-stream drain. The producer is
         * already retired, so an in-flight TO_IPU transfer cannot rely on future
         * feed activity to make progress. Abort/reset the IPU DMA machinery with
         * the same helper already used to establish a clean decoder session.
         */
        h1_diag_stage(session, 0xD8100001u); /* cancelled teardown reset enter */
        h1_video_reference_ipu_reset(session);
        h1_diag_stage(session, 0xD8100002u); /* cancelled teardown reset return */
    } else {
        h1_diag_stage(session, 0xD8000001u); /* before teardown TO_IPU wait */
        dma_channel_wait(DMA_CHANNEL_toIPU, 0);
        h1_diag_stage(session, 0xD8000002u); /* teardown TO_IPU wait returned */
    }

    if (session->mpeg_initialized) {
"""

    out = replace_once(out, old, new, "cancel-safe teardown branch")

    required = (
        "if (session->cancelled)",
        "0xD8100001u",
        "h1_video_reference_ipu_reset(session);",
        "0xD8100002u",
        "dma_channel_wait(DMA_CHANNEL_toIPU, 0);",
        "0xD8000003u",
        "MPEG_Destroy();",
    )
    for marker in required:
        if marker not in out:
            raise RuntimeError(f"cancel teardown generation missing marker: {marker}")

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
    print("H1_CP2P_CANCEL_TEARDOWN_IPU_RESET=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
