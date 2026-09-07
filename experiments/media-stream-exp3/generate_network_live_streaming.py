#!/usr/bin/env python3
"""
File synopsis:
    Deterministically derives the EXP3 O3 live playback harness from the exact
    hardware-qualified O2 concurrent fixture harness.

O3 deliberately leaves the SMS/IPU/GS implementation untouched. This script
changes only the stream-runtime boundary and replaces fixed fixture EOF/count
expectations with metadata supplied by the finite live producer.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess

EXPECTED_O2_BLOB = "d53cce693fe684a29bc154d45a0e9b378221955f"


def once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected one target, found {count}")
    return text.replace(old, new, 1)


def region(text: str, start: str, end: str, replacement: str, label: str) -> str:
    first = text.find(start)
    if first < 0:
        raise SystemExit(f"{label}: start marker not found")
    last = text.find(end, first)
    if last < 0:
        raise SystemExit(f"{label}: end marker not found")
    if text.find(start, first + 1) >= 0:
        raise SystemExit(f"{label}: start marker not unique")
    return text[:first] + replacement + text[last:]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    source = pathlib.Path(args.input)
    output = pathlib.Path(args.output)

    actual_blob = subprocess.check_output(
        ["git", "hash-object", str(source)],
        text=True,
    ).strip()

    if actual_blob != EXPECTED_O2_BLOB:
        raise SystemExit(
            "O2 harness authority mismatch: "
            f"expected={EXPECTED_O2_BLOB} actual={actual_blob}"
        )

    text = source.read_text()

    new_synopsis = '''/*
 * File synopsis:
 * EXP3 O3 finite live X11 -> ffmpeg -> Ethernet -> SMS MPEG hardware test.
 *
 * This file is mechanically derived from the exact hardware-qualified O2
 * harness. The MPEG decoder, TO_IPU feed, picture buffer, GIF macroblock
 * upload, GS draw path, and two-VSYNC presentation cadence are unchanged.
 *
 * O3 changes only EOF/source ownership: DATA/channel 4 arrives while ffmpeg
 * captures X11, and a terminal producer-metadata frame supplies the exact
 * live byte count, CRC32, and coded-picture count. A temporarily empty queue
 * remains starvation rather than EOF until that terminal frame arrives.
 */

#include "libmpeg.h"
'''

    text = region(
        text,
        "/*\n * File synopsis:\n",
        '#include "libmpeg.h"\n',
        new_synopsis,
        "top synopsis",
    )

    text = once(
        text,
        '#include "network_stream_runtime.h"\n',
        '#include "network_live_stream_runtime.h"\n',
        "runtime include",
    )

    text = text.replace("Exp3MpegStreamRuntime", "Exp3LiveMpegStreamRuntime")
    text = text.replace("exp3_mpeg_stream_", "exp3_live_mpeg_stream_")
    text = text.replace("EXP3_O2_", "EXP3_O3_")

    fixed_defines = '''#define EXP3_EXPECTED_PICTURES 599

#define EXP3_EXPECTED_STREAM_BYTES 8815372u
#define EXP3_EXPECTED_NETWORK_FRAMES 1077u
#define EXP3_NETWORK_REPLAY_PORT 5904
#define EXP3_NETWORK_REPLAY_SERVER_IP "192.168.50.1"
#define EXP3_EXPECTED_STREAM_CRC32 0x4bdc7859u

'''

    text = once(text, fixed_defines, "", "remove fixed fixture authority")

    startup_old = '''    printf(
        "EXP3_NETWORK_STREAMING_FIXTURE_START expected_bytes=%u expected_crc32=%08x\\n",
        EXP3_EXPECTED_STREAM_BYTES,
        EXP3_EXPECTED_STREAM_CRC32);
'''

    startup_new = '''    printf("EXP3_O3_LIVE_STREAM_START\\n");
'''

    text = once(text, startup_old, startup_new, "live startup")

    guard_start = '''            if (
                picture_index >
                    EXP3_EXPECTED_PICTURES
            ) {
'''
    guard_end = '''            pictures_decoded += 1;
'''

    text = region(
        text,
        guard_start,
        guard_end,
        guard_end,
        "remove fixed picture ceiling",
    )

    text = once(
        text,
        '            "expected=%d "\n',
        '            "coded_producer=%u "\n',
        "EOF print label",
    )
    text = once(
        text,
        "            EXP3_EXPECTED_PICTURES,\n",
        "            (unsigned int)stream_runtime.producer_picture_starts,\n",
        "EOF producer count",
    )

    text = once(
        text,
        '            "coded_expected=%d "\n',
        '            "coded_producer=%u "\n',
        "characterization print label",
    )
    text = once(
        text,
        "            EXP3_EXPECTED_PICTURES,\n",
        "            (unsigned int)stream_runtime.producer_picture_starts,\n",
        "characterization producer count",
    )

    terminal_start = '''        /*
         * FIRST TERMINAL MARKER — returned/displayed count
'''
    terminal_end = '''        exp3_hold_vsyncs(60);
'''

    dynamic_terminal = '''        /*
         * FIRST TERMINAL MARKER — dynamic SMS EOF characterization.
         *
         * PURPLE = SMS returned/displayed one fewer picture than the live
         *          producer counted in the elementary stream.
         * WHITE  = SMS returned/displayed every coded picture.
         * RED    = any other relationship.
         */
        if (
            stream_runtime.producer_picture_starts != 0u &&
            (uint32_t)pictures_decoded + 1u ==
                stream_runtime.producer_picture_starts &&
            pictures_displayed == pictures_decoded
        ) {
            exp3_show_color(packet, &frame, &z, 160, 0, 160);
            printf(
                "EXP3_O3_DECODER_RETURN=CODED_MINUS_ONE "
                "coded=%u returned=%d displayed=%d\\n",
                (unsigned int)stream_runtime.producer_picture_starts,
                pictures_decoded,
                pictures_displayed);
        } else if (
            stream_runtime.producer_picture_starts != 0u &&
            (uint32_t)pictures_decoded ==
                stream_runtime.producer_picture_starts &&
            pictures_displayed == pictures_decoded
        ) {
            exp3_show_color(packet, &frame, &z, 224, 224, 224);
            printf(
                "EXP3_O3_DECODER_RETURN=CODED_ALL "
                "coded=%u returned=%d displayed=%d\\n",
                (unsigned int)stream_runtime.producer_picture_starts,
                pictures_decoded,
                pictures_displayed);
        } else {
            exp3_show_color(packet, &frame, &z, 192, 0, 0);
            printf(
                "EXP3_O3_DECODER_RETURN=OTHER "
                "coded=%u returned=%d displayed=%d\\n",
                (unsigned int)stream_runtime.producer_picture_starts,
                pictures_decoded,
                pictures_displayed);
        }

        exp3_hold_vsyncs(60);
'''

    text = region(
        text,
        terminal_start,
        terminal_end,
        dynamic_terminal,
        "dynamic EOF marker",
    )

    text = text.replace("EXP3_NETWORK_STREAMING_FIXTURE_START", "EXP3_O3_LIVE_STREAM_START")

    forbidden = (
        "EXP3_EXPECTED_PICTURES",
        "EXP3_EXPECTED_STREAM_BYTES",
        "EXP3_EXPECTED_STREAM_CRC32",
        "Exp3MpegStreamRuntime",
        "exp3_mpeg_stream_",
        'network_stream_runtime.h',
    )

    for token in forbidden:
        if token in text:
            raise SystemExit(f"generated O3 harness still contains O2 token: {token}")

    if "dma_wait_fast();" in text:
        raise SystemExit("generated O3 harness reintroduced dma_wait_fast()")

    if "producer_picture_starts" not in text:
        raise SystemExit("generated O3 harness lacks dynamic producer count")

    output.write_text(text)

    print(f"O3_GENERATED_SOURCE={output}")
    print("O3_GENERATION=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
