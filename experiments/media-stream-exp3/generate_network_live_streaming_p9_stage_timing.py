#!/usr/bin/env python3
"""
File synopsis:
    Generates EXP3 P9 from the exact hardware-qualified P8 RGB16 harness.

Purpose:
    The deterministic 608x416 / 2 Mb/s stress reel now runs with zero network
    feed waits and zero presentation deadline misses on P8. That creates a
    clean opportunity to measure the PS2 pipeline itself instead of inferring
    decoder cost from end-to-end throughput.

    P9 is instrumentation-only. It preserves P8 RGB16, the P7A 256 KiB queue
    and 224 KiB prefill, P6 absolute presentation scheduling, no-drop policy,
    MPEG feed, source, and GS behavior. It measures wall-clock ticks around:

      * MPEG_Picture() -- decode/IPU/libmpeg call duration;
      * GS texture upload -- the existing source-chain upload ownership span;
      * GS draw -- the existing GIF draw span, excluding scheduler waits.

    Totals, maxima, and sample counts are appended to the existing post-
    playback UDP clock datagram. No timing telemetry is transmitted during
    playback.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys


EXPECTED_P8_GENERATOR_BLOB = "63ba31bcd553777cbadd524bcd91eebef46ab422"
EXPECTED_P8_OUTPUT_BLOB = "26ab27a361961b350e549f8ac28027053b41ebc3"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected one target, found {count}")
    return text.replace(old, new, 1)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    source = pathlib.Path(args.input)
    output = pathlib.Path(args.output)

    p8_generator = source.parent / "generate_network_live_streaming_p8_rgb16.py"

    actual_generator_blob = subprocess.check_output(
        ["git", "hash-object", str(p8_generator)],
        text=True,
    ).strip()

    print(f"P9_P8_GENERATOR_BLOB={actual_generator_blob}")

    if actual_generator_blob != EXPECTED_P8_GENERATOR_BLOB:
        raise SystemExit(
            "P8 generator authority mismatch: "
            f"expected={EXPECTED_P8_GENERATOR_BLOB} actual={actual_generator_blob}"
        )

    base = output.with_name(output.name + ".p8-base.tmp")

    try:
        subprocess.run(
            [
                sys.executable,
                str(p8_generator),
                "--input",
                str(source),
                "--output",
                str(base),
            ],
            check=True,
        )

        actual_base_blob = subprocess.check_output(
            ["git", "hash-object", str(base)],
            text=True,
        ).strip()

        print(f"P9_P8_BASE_BLOB={actual_base_blob}")

        if actual_base_blob != EXPECTED_P8_OUTPUT_BLOB:
            raise SystemExit(
                "P8 generated-source authority mismatch: "
                f"expected={EXPECTED_P8_OUTPUT_BLOB} actual={actual_base_blob}"
            )

        text = base.read_text()

        old_fields = '''    unsigned int p7a_prefill_target;
    unsigned int p7a_prefill_observed;
} Exp3DecoderState;
'''

        new_fields = '''    unsigned int p7a_prefill_target;
    unsigned int p7a_prefill_observed;

    /* P9 instrumentation-only stage timing. */
    unsigned int p9_decode_samples;
    u64 p9_decode_ticks_total;
    u64 p9_decode_ticks_max;

    unsigned int p9_upload_samples;
    u64 p9_upload_ticks_total;
    u64 p9_upload_ticks_max;

    unsigned int p9_draw_samples;
    u64 p9_draw_ticks_total;
    u64 p9_draw_ticks_max;
} Exp3DecoderState;
'''

        text = replace_once(
            text,
            old_fields,
            new_fields,
            "P9 timing fields",
        )

        helper = r'''
static void exp3_p9_accumulate_timing(
    u64 elapsed,
    unsigned int *samples,
    u64 *total,
    u64 *maximum)
{
    if (samples == NULL || total == NULL || maximum == NULL)
        return;

    *samples += 1u;
    *total += elapsed;

    if (elapsed > *maximum)
        *maximum = elapsed;
}

'''

        text = replace_once(
            text,
            "int main(void)\n",
            helper + "int main(void)\n",
            "P9 accumulation helper",
        )

        old_main_vars = '''    s64 picture_pts;
    s64 last_picture_pts;

    int stream_exhausted;
'''

        new_main_vars = '''    s64 picture_pts;
    s64 last_picture_pts;

    u64 p9_stage_start;
    u64 p9_stage_elapsed;

    int stream_exhausted;
'''

        text = replace_once(
            text,
            old_main_vars,
            new_main_vars,
            "P9 main timing temporaries",
        )

        first_decode = '''    picture_result =
        MPEG_Picture(
            NULL,
            &picture_pts);
'''

        first_decode_timed = '''    p9_stage_start = GetTimerSystemTime();

    picture_result =
        MPEG_Picture(
            NULL,
            &picture_pts);

    p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

    exp3_p9_accumulate_timing(
        p9_stage_elapsed,
        &decoder.p9_decode_samples,
        &decoder.p9_decode_ticks_total,
        &decoder.p9_decode_ticks_max);
'''

        text = replace_once(
            text,
            first_decode,
            first_decode_timed,
            "P9 first decode timing",
        )

        loop_decode = '''            picture_result =
                MPEG_Picture(
                    s_picture_buffer,
                    &picture_pts);
'''

        loop_decode_timed = '''            p9_stage_start = GetTimerSystemTime();

            picture_result =
                MPEG_Picture(
                    s_picture_buffer,
                    &picture_pts);

            p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

            exp3_p9_accumulate_timing(
                p9_stage_elapsed,
                &decoder.p9_decode_samples,
                &decoder.p9_decode_ticks_total,
                &decoder.p9_decode_ticks_max);
'''

        text = replace_once(
            text,
            loop_decode,
            loop_decode_timed,
            "P9 loop decode timing",
        )

        first_upload = '''        exp3_upload_picture_texture(
            &decoder);

        /*
         * BLUE:
'''

        first_upload_timed = '''        p9_stage_start = GetTimerSystemTime();

        exp3_upload_picture_texture(
            &decoder);

        p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

        exp3_p9_accumulate_timing(
            p9_stage_elapsed,
            &decoder.p9_upload_samples,
            &decoder.p9_upload_ticks_total,
            &decoder.p9_upload_ticks_max);

        /*
         * BLUE:
'''

        text = replace_once(
            text,
            first_upload,
            first_upload_timed,
            "P9 first upload timing",
        )

        loop_upload = '''            exp3_upload_picture_texture(
                &decoder);

            exp3_p6_wait_for_picture(
'''

        loop_upload_timed = '''            p9_stage_start = GetTimerSystemTime();

            exp3_upload_picture_texture(
                &decoder);

            p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

            exp3_p9_accumulate_timing(
                p9_stage_elapsed,
                &decoder.p9_upload_samples,
                &decoder.p9_upload_ticks_total,
                &decoder.p9_upload_ticks_max);

            exp3_p6_wait_for_picture(
'''

        text = replace_once(
            text,
            loop_upload,
            loop_upload_timed,
            "P9 loop upload timing",
        )

        first_draw = '''        exp3_draw_picture_texture(
            &decoder);

        pictures_decoded = 1;
'''

        first_draw_timed = '''        p9_stage_start = GetTimerSystemTime();

        exp3_draw_picture_texture(
            &decoder);

        p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

        exp3_p9_accumulate_timing(
            p9_stage_elapsed,
            &decoder.p9_draw_samples,
            &decoder.p9_draw_ticks_total,
            &decoder.p9_draw_ticks_max);

        pictures_decoded = 1;
'''

        text = replace_once(
            text,
            first_draw,
            first_draw_timed,
            "P9 first draw timing",
        )

        loop_draw = '''            exp3_draw_picture_texture(
                &decoder);

            pictures_displayed += 1;
'''

        loop_draw_timed = '''            p9_stage_start = GetTimerSystemTime();

            exp3_draw_picture_texture(
                &decoder);

            p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

            exp3_p9_accumulate_timing(
                p9_stage_elapsed,
                &decoder.p9_draw_samples,
                &decoder.p9_draw_ticks_total,
                &decoder.p9_draw_ticks_max);

            pictures_displayed += 1;
'''

        text = replace_once(
            text,
            loop_draw,
            loop_draw_timed,
            "P9 loop draw timing",
        )

        text = replace_once(
            text,
            "    char payload[256];\n",
            "    char payload[640];\n",
            "P9 telemetry payload capacity",
        )

        old_payload_fields = '''        "prefill_target=%u "
        "prefill_observed=%u",
'''

        new_payload_fields = '''        "prefill_target=%u "
        "prefill_observed=%u "
        "decode_n=%u decode_total=%llu decode_max=%llu "
        "upload_n=%u upload_total=%llu upload_max=%llu "
        "draw_n=%u draw_total=%llu draw_max=%llu",
'''

        text = replace_once(
            text,
            old_payload_fields,
            new_payload_fields,
            "P9 UDP timing fields",
        )

        old_payload_args = '''        state->p7a_prefill_target,
        state->p7a_prefill_observed);
'''

        new_payload_args = '''        state->p7a_prefill_target,
        state->p7a_prefill_observed,
        state->p9_decode_samples,
        (unsigned long long)state->p9_decode_ticks_total,
        (unsigned long long)state->p9_decode_ticks_max,
        state->p9_upload_samples,
        (unsigned long long)state->p9_upload_ticks_total,
        (unsigned long long)state->p9_upload_ticks_max,
        state->p9_draw_samples,
        (unsigned long long)state->p9_draw_ticks_total,
        (unsigned long long)state->p9_draw_ticks_max);
'''

        text = replace_once(
            text,
            old_payload_args,
            new_payload_args,
            "P9 UDP timing arguments",
        )

        text = text.replace(
            "EXP3_P7A_CLOCK ",
            "EXP3_P9_TIMING ",
        )

        if text.count("EXP3_P9_TIMING ") != 1:
            raise SystemExit("P9 timing marker replacement failed")

        if text.count("_MPEG_Set16(1);") != 1:
            raise SystemExit("P9 must retain P8 RGB16")

        if text.count("exp3_p9_accumulate_timing(") != 7:
            raise SystemExit(
                "P9 expected one helper definition plus six measured stage calls"
            )

        if text.count("p9_stage_start = GetTimerSystemTime();") != 6:
            raise SystemExit("P9 expected six timing starts")

        if text.count("p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;") != 6:
            raise SystemExit("P9 expected six timing stops")

        if text.count("exp3_p6_wait_for_picture(") != 4:
            raise SystemExit("P9 unexpectedly changed P6 scheduler structure")

        if text.count("exp3_p7a_wait_for_prefill(") != 2:
            raise SystemExit("P9 unexpectedly changed P7A prefill structure")

        if "frames_dropped" in text:
            raise SystemExit("P9 must not add frame dropping")

        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text)

        generated_blob = subprocess.check_output(
            ["git", "hash-object", str(output)],
            text=True,
        ).strip()

        print(f"P9_OUTPUT={output}")
        print(f"P9_OUTPUT_BLOB={generated_blob}")
        print("P9_BASE=P8_RGB16")
        print("P9_INSTRUMENTS=MPEG_PICTURE,GS_UPLOAD,GS_DRAW")
        print("P9_GENERATION=PASS")

    finally:
        try:
            base.unlink()
        except FileNotFoundError:
            pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
