#!/usr/bin/env python3
"""Generate CP2P MPEG stall stages plus a persistent live last-stage witness.

This wrapper preserves the existing observation-only CP2P stall instrumentation
from generate_h1_video_runtime_cp2p_diag.py and adds one independent witness:
while MEDIA_END has not arrived, every MPEG stage write is also mirrored into
transport producer_stop_reason. That telemetry slot is otherwise live-only in
this diagnostic derivative and MEDIA_END later overwrites it authoritatively.

The point is to retain the last MPEG stage even when diagnostic_word is later
reused by RFB or thread-census diagnostics. No queue size, thread priority,
timeout, scheduling policy, decoder policy, or transport ownership is changed.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import generate_h1_video_runtime_cp2p as basegen
import generate_h1_video_runtime_cp2p_diag as diaggen


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def generate(source: str) -> str:
    out = diaggen.generate(source)

    old = """static void h1_diag_stage(h1_video_session_t *session, uint32_t stage)
{
    if (session != NULL && session->transport != NULL)
        pstvnc_h1_transport_set_diagnostic_word(session->transport, stage);
}
"""
    new = """static void h1_diag_stage(h1_video_session_t *session, uint32_t stage)
{
    if (session != NULL && session->transport != NULL) {
        pstvnc_h1_transport_set_diagnostic_word(session->transport, stage);

        /*
         * Dedicated live MPEG witness. The RFB transaction diagnostic leaves
         * producer_stop_reason untouched; MEDIA_END later replaces this value
         * with the authoritative producer stop reason.
         */
        if (session->transport->end_received == 0u)
            session->transport->producer_stop_reason = stage;
    }
}
"""

    out = replace_once(out, old, new, "persistent MPEG stage helper")

    if "session->transport->producer_stop_reason = stage;" not in out:
        raise RuntimeError("persistent MPEG stage witness was not generated")

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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
