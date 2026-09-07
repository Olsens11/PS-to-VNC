#!/usr/bin/env python3
"""
File synopsis:
    Generates EXP3 P7 from the exact P6B absolute-clock harness.

Purpose:
    P6B hardware telemetry showed 289 deadline misses out of 598 SMS-returned
    pictures and a worst-case lateness of about 39.97 source-frame periods.
    P6 deliberately draws every late picture, which visibly produces fast
    catch-up bursts after stalls.

    P7 changes one presentation behavior only: every MPEG picture is still
    decoded and uploaded exactly as in P6, but the GS draw is suppressed when
    the picture reaches the presentation scheduler at least one complete
    30000/1001 source-frame period late. Pictures less than one frame late are
    still presented. This avoids treating normal VBlank quantization as an
    obsolete picture.

    Decode dropping is forbidden because MPEG reference-picture state must
    continue to advance. Texture upload is deliberately retained in P7 so this
    first experiment isolates visible stale-frame presentation from upload
    workload. A later experiment may separately test skipping stale uploads.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys


EXPECTED_P6B_GENERATOR_BLOB = "2f72d89a585cb5829de5fe7d9f27cd584479499f"


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
    p6b_generator = source.parent / "generate_network_live_streaming_p6b_clock_telemetry.py"

    actual_generator_blob = subprocess.check_output(
        ["git", "hash-object", str(p6b_generator)],
        text=True,
    ).strip()

    print(f"P7_P6B_GENERATOR_BLOB={actual_generator_blob}")

    if actual_generator_blob != EXPECTED_P6B_GENERATOR_BLOB:
        raise SystemExit(
            "P6B generator authority mismatch: "
            f"expected={EXPECTED_P6B_GENERATOR_BLOB} actual={actual_generator_blob}"
        )

    base = output.with_name(output.name + ".p6b-base.tmp")

    try:
        subprocess.run(
            [
                sys.executable,
                str(p6b_generator),
                "--input",
                str(source),
                "--output",
                str(base),
            ],
            check=True,
        )

        text = base.read_text()

        text = replace_once(
            text,
            "    u64 p6_max_deadline_late_ticks;\n} Exp3DecoderState;\n",
            "    u64 p6_max_deadline_late_ticks;\n\n"
            "    /* P7 presentation-drop diagnostic. */\n"
            "    unsigned int p7_frames_dropped;\n"
            "} Exp3DecoderState;\n",
            "P7 drop counter",
        )

        old_scheduler = r'''static void exp3_p6_wait_for_picture(
    Exp3DecoderState *state,
    unsigned int picture_index)
{
    u64 deadline;
    u64 now;

    if (!state->p6_clock_started) {
        /*
         * Establish the epoch on a real display boundary. Subsequent picture
         * deadlines remain anchored to this one epoch and never rebase after
         * decode/network jitter.
         */
        graph_wait_vsync();
        state->p6_vsync_waits += 1u;

        state->p6_presentation_epoch_tick =
            GetTimerSystemTime();

        state->p6_clock_started = 1u;
    }

    deadline =
        exp3_p6_picture_deadline(
            state,
            picture_index);

    now =
        GetTimerSystemTime();

    if (picture_index > 1u && now >= deadline) {
        u64 lateness = now - deadline;

        state->p6_deadline_misses += 1u;

        if (lateness > state->p6_max_deadline_late_ticks)
            state->p6_max_deadline_late_ticks = lateness;

        return;
    }

    while (now < deadline) {
        /*
         * Wait on display boundaries rather than busy-spinning. This leaves
         * the receiver thread runnable while still presenting on a GS-safe
         * boundary. The absolute deadline, not the number of waits, owns
         * cadence.
         */
        graph_wait_vsync();
        state->p6_vsync_waits += 1u;

        now =
            GetTimerSystemTime();
    }
}
'''

        new_scheduler = r'''static u64 exp3_p7_frame_period_ticks(void)
{
    return
        ((u64)kBUSCLK *
         (u64)EXP3_P6_SOURCE_FPS_DENOMINATOR) /
        (u64)EXP3_P6_SOURCE_FPS_NUMERATOR;
}

static int exp3_p7_wait_for_picture(
    Exp3DecoderState *state,
    unsigned int picture_index)
{
    u64 deadline;
    u64 now;

    if (!state->p6_clock_started) {
        /*
         * Keep P6's exact epoch rule: picture 1 establishes the absolute
         * presentation clock on a real display boundary.
         */
        graph_wait_vsync();
        state->p6_vsync_waits += 1u;

        state->p6_presentation_epoch_tick =
            GetTimerSystemTime();

        state->p6_clock_started = 1u;
    }

    deadline =
        exp3_p6_picture_deadline(
            state,
            picture_index);

    now =
        GetTimerSystemTime();

    if (picture_index > 1u && now >= deadline) {
        u64 lateness = now - deadline;

        state->p6_deadline_misses += 1u;

        if (lateness > state->p6_max_deadline_late_ticks)
            state->p6_max_deadline_late_ticks = lateness;

        /*
         * P7's sole playback change: once the next source-frame deadline has
         * already arrived, this picture is obsolete for display. MPEG decode
         * and GS upload have still occurred; only the stale draw is omitted.
         */
        if (lateness >= exp3_p7_frame_period_ticks()) {
            state->p7_frames_dropped += 1u;
            return 0;
        }

        return 1;
    }

    while (now < deadline) {
        graph_wait_vsync();
        state->p6_vsync_waits += 1u;

        now =
            GetTimerSystemTime();
    }

    return 1;
}
'''

        text = replace_once(
            text,
            old_scheduler,
            new_scheduler,
            "P7 scheduler",
        )

        text = replace_once(
            text,
            '''        exp3_p6_wait_for_picture(
            &decoder,
            1u);

        exp3_draw_picture_texture(
            &decoder);
''',
            '''        (void)exp3_p7_wait_for_picture(
            &decoder,
            1u);

        exp3_draw_picture_texture(
            &decoder);
''',
            "P7 first-picture scheduler call",
        )

        text = replace_once(
            text,
            '''            exp3_p6_wait_for_picture(
                &decoder,
                (unsigned int)picture_index);

            exp3_draw_picture_texture(
                &decoder);

            pictures_displayed += 1;
''',
            '''            if (exp3_p7_wait_for_picture(
                    &decoder,
                    (unsigned int)picture_index)) {
                exp3_draw_picture_texture(
                    &decoder);

                pictures_displayed += 1;
            }
''',
            "P7 loop presentation gate",
        )

        text = text.replace(
            "exp3_p6b_send_clock_telemetry",
            "exp3_p7_send_clock_telemetry",
        )
        text = text.replace(
            "EXP3_P6B_CLOCK_TELEMETRY",
            "EXP3_P7_CLOCK_TELEMETRY",
        )
        text = text.replace(
            "EXP3_P6B_CLOCK_UDP",
            "EXP3_P7_CLOCK_UDP",
        )

        text = replace_once(
            text,
            '''        "EXP3_P6B_CLOCK "
        "fps_num=%u fps_den=%u "
        "frame_period_ticks=%llu "
        "vsync_waits=%u "
        "deadline_misses=%u "
        "max_deadline_late_ticks=%llu",
''',
            '''        "EXP3_P7_CLOCK "
        "fps_num=%u fps_den=%u "
        "frame_period_ticks=%llu "
        "vsync_waits=%u "
        "deadline_misses=%u "
        "max_deadline_late_ticks=%llu "
        "frames_dropped=%u",
''',
            "P7 telemetry format",
        )

        text = replace_once(
            text,
            '''        state->p6_deadline_misses,
        (unsigned long long)state->p6_max_deadline_late_ticks);
''',
            '''        state->p6_deadline_misses,
        (unsigned long long)state->p6_max_deadline_late_ticks,
        state->p7_frames_dropped);
''',
            "P7 telemetry arguments",
        )

        if "exp3_p6_wait_for_picture(" in text:
            raise SystemExit("P7 left a P6 scheduler call or definition behind")

        if text.count("exp3_p7_wait_for_picture(") != 3:
            raise SystemExit("P7 expected scheduler definition plus two call sites")

        if text.count("state->p7_frames_dropped += 1u;") != 1:
            raise SystemExit("P7 expected exactly one stale-frame drop action")

        if text.count("exp3_upload_picture_texture(") != 3:
            raise SystemExit("P7 unexpectedly changed upload call structure")

        if text.count("graph_wait_vsync();") != 5:
            raise SystemExit("P7 unexpectedly changed VSYNC call structure")

        if text.count("EXP3_P7_CLOCK ") != 1:
            raise SystemExit("P7 telemetry marker missing")

        if text.count("frames_dropped=%u") != 1:
            raise SystemExit("P7 drop telemetry missing")

        if text.count("exp3_p7_send_clock_telemetry(") != 2:
            raise SystemExit("P7 expected telemetry helper plus one call")

        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text)

        generated_blob = subprocess.check_output(
            ["git", "hash-object", str(output)],
            text=True,
        ).strip()

        print(f"P7_OUTPUT={output}")
        print(f"P7_OUTPUT_BLOB={generated_blob}")
        print("P7_DROP_THRESHOLD=ONE_SOURCE_FRAME_PERIOD")
        print("P7_DECODE_DROPPING=NO")
        print("P7_UPLOAD_DROPPING=NO")
        print("P7_STALE_GS_DRAW_DROPPING=YES")
        print("P7_GENERATION=PASS")

    finally:
        try:
            base.unlink()
        except FileNotFoundError:
            pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
