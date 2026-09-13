#!/usr/bin/env python3
"""
Generate the authoritative safe-stop CP2P runtime plus MPEG_Picture boundary
witnesses.

The transport layer independently records the detailed MPEG callback stages.
This generator deliberately does not replace the feed callback and therefore
does not alter queue mechanics, stop semantics, or the ordinary
pstvnc_h1_transport_mpeg_read() callback.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import generate_h1_video_runtime_cp2p as basegen


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)

    if count != 1:
        raise RuntimeError(
            f"{label}: expected exactly one match, found {count}"
        )

    return text.replace(old, new, 1)


def generate(source: str) -> str:
    out = basegen.generate(source)

    out = replace_once(
        out,
        "static h1_video_chassis_t s_h1_chassis;\n",
        """static h1_video_chassis_t s_h1_chassis;

/*
 * Persistent MPEG_Picture boundary witness.
 *
 * Detailed queue/callback stages are written by h1_transport_runtime.c.
 */
static void h1_mpeg_picture_stage(
    h1_video_session_t *session,
    uint32_t stage)
{
    if (session != NULL && session->transport != NULL)
        session->transport->mpeg_diag_stage = stage;
}

static int h1_mpeg_picture_diag(
    h1_video_session_t *session,
    void *picture,
    s64 *pts,
    uint32_t picture_index)
{
    int result;

    h1_mpeg_picture_stage(
        session,
        0xD3000000u | (picture_index & 0x00ffffffu));

    result = MPEG_Picture(picture, pts);

    h1_mpeg_picture_stage(
        session,
        0xD4000000u | (picture_index & 0x00ffffffu));

    return result;
}
""",
        "PICTURE_HELPER",
    )

    out = replace_once(
        out,
        "picture_result = MPEG_Picture(NULL, &picture_pts);",
        "picture_result = h1_mpeg_picture_diag("
        "&session, NULL, &picture_pts, 1u);",
        "FIRST_PICTURE",
    )

    out = replace_once(
        out,
        "picture_result = MPEG_Picture(session.picture_buffer, &picture_pts);",
        "picture_result = h1_mpeg_picture_diag(\n"
        "            &session,\n"
        "            session.picture_buffer,\n"
        "            &picture_pts,\n"
        "            picture_index);",
        "LOOP_PICTURE",
    )

    feed_start = out.find(
        "static int h1_video_feed_ipu(void *user_data)"
    )
    feed_end = out.find(
        "static void *h1_video_sequence_init(",
        feed_start,
    )

    if feed_start < 0 or feed_end < 0:
        raise RuntimeError(
            "could not isolate generated MPEG feed callback"
        )

    feed = out[feed_start:feed_end]

    if "pstvnc_h1_transport_mpeg_read(" not in feed:
        raise RuntimeError(
            "ordinary MPEG read missing from feed callback"
        )

    if "pstvnc_h1_transport_mpeg_read_cancellable(" in feed:
        raise RuntimeError(
            "cancellable MPEG read reintroduced into feed callback"
        )

    if "stop_requested" in feed:
        raise RuntimeError(
            "feed callback observes asynchronous stop"
        )

    required = (
        "0xD3000000u",
        "0xD4000000u",
        "h1_mpeg_picture_diag",
        "pstvnc_h1_transport_mpeg_read(",
    )

    for marker in required:
        if marker not in out:
            raise RuntimeError(
                f"generated runtime missing marker: {marker}"
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
            f"expected:{basegen.EXPECTED_INPUT_BLOB_SHA1}:"
            f"actual:{actual_sha}"
        )

    generated = generate(data.decode("utf-8"))

    output_path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )
    output_path.write_text(
        generated,
        encoding="utf-8",
    )

    print(
        f"H1_CP2P_VIDEO_SOURCE_BLOB={actual_sha}"
    )
    print(
        "H1_CP2P_MPEG_INNER_STALL_GENERATION=PASS"
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
