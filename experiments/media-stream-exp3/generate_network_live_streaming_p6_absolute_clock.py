#!/usr/bin/env python3
"""
File synopsis:
    Generates the EXP3 O3 P6 absolute-clock presentation diagnostic from the
    exact hardware-qualified O3 live-stream harness.

Purpose:
    P3/P4/P5 hardware testing established that two relative VSYNC waits are too
    slow, zero waits are too fast, and one relative wait gives near-real-time
    average throughput but visibly unstable cadence. P6 changes only
    presentation timing: decoded pictures are still decoded, uploaded, and
    drawn serially, but presentation is gated against an absolute monotonic
    30000/1001-fps deadline schedule instead of waiting a fixed number of
    VSYNCs after every draw.

    P6 deliberately does NOT drop late pictures. It records deadline misses so
    a later experiment can distinguish whether frame dropping or deeper
    buffering is needed. This is a timing diagnostic, not yet the product
    scheduler.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess


EXPECTED_O3_BLOB = "4e34795823cbb386f7ac958c43a2ddbce4e255e5"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(
            f"{label}: expected one target, found {count}"
        )
    return text.replace(old, new, 1)


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

    print(f"P6_INPUT_BLOB={actual_blob}")

    if actual_blob != EXPECTED_O3_BLOB:
        raise SystemExit(
            "O3 harness authority mismatch: "
            f"expected={EXPECTED_O3_BLOB} actual={actual_blob}"
        )

    text = source.read_text()

    text = replace_once(
        text,
        " * upload, GS draw path, and two-VSYNC presentation cadence are unchanged.\n",
        " * upload and GS draw path are unchanged. P6 alone replaces the relative\n"
        " * two-VSYNC presentation delay with an absolute 30000/1001-fps clock.\n",
        "synopsis",
    )

    text = replace_once(
        text,
        "#include <kernel.h>\n#include <malloc.h>\n",
        "#include <kernel.h>\n#include <timer.h>\n#include <malloc.h>\n",
        "timer include",
    )

    text = replace_once(
        text,
        "    s64 current_stream_pts;\n} Exp3DecoderState;\n",
        "    s64 current_stream_pts;\n\n"
        "    /* P6 absolute presentation-clock diagnostics. */\n"
        "    u64 p6_presentation_epoch_tick;\n"
        "    unsigned int p6_clock_started;\n"
        "    unsigned int p6_vsync_waits;\n"
        "    unsigned int p6_deadline_misses;\n"
        "    u64 p6_max_deadline_late_ticks;\n"
        "} Exp3DecoderState;\n",
        "decoder timing fields",
    )

    old_draw_tail = '''    graph_wait_vsync();
    graph_wait_vsync();
}

int main(void)
'''

    scheduler = '''    /*
     * P6 absolute-clock diagnostic: presentation waits are performed before
     * draw by exp3_p6_wait_for_picture(). The GIF DMA completion above remains
     * the decoded-buffer ownership boundary.
     */
}

#define EXP3_P6_SOURCE_FPS_NUMERATOR   30000u
#define EXP3_P6_SOURCE_FPS_DENOMINATOR 1001u

static u64 exp3_p6_picture_deadline(
    const Exp3DecoderState *state,
    unsigned int picture_index)
{
    u64 picture_offset;

    if (picture_index <= 1u)
        return state->p6_presentation_epoch_tick;

    picture_offset =
        (u64)(picture_index - 1u) *
        (u64)kBUSCLK *
        (u64)EXP3_P6_SOURCE_FPS_DENOMINATOR;

    picture_offset /=
        (u64)EXP3_P6_SOURCE_FPS_NUMERATOR;

    return
        state->p6_presentation_epoch_tick +
        picture_offset;
}

static void exp3_p6_wait_for_picture(
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

int main(void)
'''

    text = replace_once(
        text,
        old_draw_tail,
        scheduler,
        "draw-tail scheduler",
    )

    first_draw = '''        exp3_draw_picture_texture(
            &decoder);

        pictures_decoded = 1;
'''

    first_draw_new = '''        exp3_p6_wait_for_picture(
            &decoder,
            1u);

        exp3_draw_picture_texture(
            &decoder);

        pictures_decoded = 1;
'''

    text = replace_once(
        text,
        first_draw,
        first_draw_new,
        "first picture scheduler",
    )

    loop_draw = '''            exp3_draw_picture_texture(
                &decoder);

            pictures_displayed += 1;
'''

    loop_draw_new = '''            exp3_p6_wait_for_picture(
                &decoder,
                (unsigned int)picture_index);

            exp3_draw_picture_texture(
                &decoder);

            pictures_displayed += 1;
'''

    text = replace_once(
        text,
        loop_draw,
        loop_draw_new,
        "loop picture scheduler",
    )

    marker = '''        /*
         * FIRST TERMINAL MARKER — dynamic SMS EOF characterization.
'''

    timing_report = '''        printf(
            "EXP3_P6_CLOCK "
            "fps_num=%u fps_den=%u "
            "frame_period_ticks=%llu "
            "vsync_waits=%u "
            "deadline_misses=%u "
            "max_deadline_late_ticks=%llu\\n",
            EXP3_P6_SOURCE_FPS_NUMERATOR,
            EXP3_P6_SOURCE_FPS_DENOMINATOR,
            (unsigned long long)(
                ((u64)kBUSCLK *
                 (u64)EXP3_P6_SOURCE_FPS_DENOMINATOR) /
                (u64)EXP3_P6_SOURCE_FPS_NUMERATOR),
            decoder.p6_vsync_waits,
            decoder.p6_deadline_misses,
            (unsigned long long)decoder.p6_max_deadline_late_ticks);

        /*
         * FIRST TERMINAL MARKER — dynamic SMS EOF characterization.
'''

    text = replace_once(
        text,
        marker,
        timing_report,
        "P6 timing report",
    )

    if text.count("exp3_p6_wait_for_picture(") != 3:
        raise SystemExit(
            "P6 expected scheduler definition plus two call sites"
        )

    if text.count("P6 absolute-clock diagnostic") != 1:
        raise SystemExit("P6 draw diagnostic marker missing")

    if text.count("graph_wait_vsync();") != 6:
        raise SystemExit(
            "P6 unexpected graph_wait_vsync call count; "
            "expected original non-presentation waits plus scheduler waits"
        )

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(text)

    generated_blob = subprocess.check_output(
        ["git", "hash-object", str(output)],
        text=True,
    ).strip()

    print(f"P6_OUTPUT={output}")
    print(f"P6_OUTPUT_BLOB={generated_blob}")
    print("P6_GENERATION=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
