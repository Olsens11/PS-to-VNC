#!/usr/bin/env python3
"""
File synopsis:
    Generates EXP3 P7A from the exact P6B absolute-clock harness.

Purpose:
    P6/P6B proved that average video rate can be near real time while the
    serialized playback loop still suffers large deadline misses and visible
    fast/slow bursts. P7A tests the compressed-input reservoir hypothesis
    before changing decode or presentation behavior.

    P7A keeps the exact P6 absolute presentation scheduler, MPEG decoder, GS
    upload/draw path, source archive, and no-drop policy. The only playback
    change is to require a deep compressed-byte prefill before the first MPEG
    picture decode. The Pi-side replay config separately requests a 256 KiB
    queue; this harness requires at least 224 KiB to be physically present in
    that queue before picture 1 starts.

    P7A retains P6B's post-playback UDP clock telemetry and extends it with the
    requested/observed prefill byte counts. Existing O3 result telemetry still
    reports feed_wait_events, so a deep-buffer run can directly show whether
    decoder input starvation collapses without adding per-feed instrumentation.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys


EXPECTED_P6B_GENERATOR_BLOB = "2f72d89a585cb5829de5fe7d9f27cd584479499f"
PREFILL_BYTES = 224 * 1024


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

    print(f"P7A_P6B_GENERATOR_BLOB={actual_generator_blob}")

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
            "#include <kernel.h>\n#include <timer.h>\n",
            "#include <kernel.h>\n#include <timer.h>\n#include <delaythread.h>\n",
            "DelayThread include",
        )

        text = replace_once(
            text,
            "    u64 p6_max_deadline_late_ticks;\n} Exp3DecoderState;\n",
            "    u64 p6_max_deadline_late_ticks;\n\n"
            "    /* P7A compressed-input prefill diagnostics. */\n"
            "    unsigned int p7a_prefill_target;\n"
            "    unsigned int p7a_prefill_observed;\n"
            "} Exp3DecoderState;\n",
            "P7A decoder fields",
        )

        helper = r'''
#define EXP3_P7A_PREFILL_BYTES (224u * 1024u)
#define EXP3_P7A_PREFILL_WAIT_US 1000u
#define EXP3_P7A_PREFILL_MAX_LOOPS 10000u

static int exp3_p7a_wait_for_prefill(Exp3DecoderState *state)
{
    Exp3LiveMpegStreamRuntime *runtime;
    unsigned int loops = 0u;

    if (state == NULL || state->stream_runtime == NULL)
        return 0;

    runtime = state->stream_runtime;
    state->p7a_prefill_target = EXP3_P7A_PREFILL_BYTES;
    state->p7a_prefill_observed = 0u;

    if (runtime->queue_capacity < EXP3_P7A_PREFILL_BYTES)
        return 0;

    for (;;) {
        size_t current;
        int receiver_done;
        int error;

        if (WaitSema(runtime->queue_sema_id) < 0)
            return 0;

        current = pstvnc_transport_queue_size(&runtime->queue);
        receiver_done = runtime->receiver_done;
        error = runtime->error;

        if (current > state->p7a_prefill_observed)
            state->p7a_prefill_observed = (unsigned int)current;

        if (SignalSema(runtime->queue_sema_id) < 0)
            return 0;

        if (current >= EXP3_P7A_PREFILL_BYTES)
            return 1;

        if (
            error != EXP3_LIVE_STREAM_ERROR_NONE ||
            receiver_done ||
            loops >= EXP3_P7A_PREFILL_MAX_LOOPS
        ) {
            return 0;
        }

        loops += 1u;

        if (DelayThread(EXP3_P7A_PREFILL_WAIT_US) < 0)
            return 0;
    }
}

'''

        text = replace_once(
            text,
            "#define EXP3_P6_SOURCE_FPS_NUMERATOR   30000u\n",
            helper + "#define EXP3_P6_SOURCE_FPS_NUMERATOR   30000u\n",
            "P7A prefill helper",
        )

        before_first = '''    printf(
        "EXP3_STAGE=BEFORE_FIRST_MPEG_PICTURE\\n");

    picture_pts = 0;
'''

        before_first_new = '''    if (!exp3_p7a_wait_for_prefill(&decoder)) {
        printf(
            "EXP3_P7A_PREFILL=FAIL target=%u observed=%u queue_capacity=%u error=%d\\n",
            decoder.p7a_prefill_target,
            decoder.p7a_prefill_observed,
            (unsigned int)stream_runtime.queue_capacity,
            stream_runtime.error);
        SleepThread();
        return 15;
    }

    printf(
        "EXP3_P7A_PREFILL=PASS target=%u observed=%u queue_capacity=%u\\n",
        decoder.p7a_prefill_target,
        decoder.p7a_prefill_observed,
        (unsigned int)stream_runtime.queue_capacity);

    printf(
        "EXP3_STAGE=BEFORE_FIRST_MPEG_PICTURE\\n");

    picture_pts = 0;
'''

        text = replace_once(
            text,
            before_first,
            before_first_new,
            "P7A pre-first-picture prefill",
        )

        old_payload_tail = '''        "deadline_misses=%u "
        "max_deadline_late_ticks=%llu",
'''

        new_payload_tail = '''        "deadline_misses=%u "
        "max_deadline_late_ticks=%llu "
        "prefill_target=%u "
        "prefill_observed=%u",
'''

        text = replace_once(
            text,
            old_payload_tail,
            new_payload_tail,
            "P7A UDP payload fields",
        )

        old_payload_args = '''        state->p6_deadline_misses,
        (unsigned long long)state->p6_max_deadline_late_ticks);
'''

        new_payload_args = '''        state->p6_deadline_misses,
        (unsigned long long)state->p6_max_deadline_late_ticks,
        state->p7a_prefill_target,
        state->p7a_prefill_observed);
'''

        text = replace_once(
            text,
            old_payload_args,
            new_payload_args,
            "P7A UDP payload arguments",
        )

        text = text.replace(
            "EXP3_P6B_CLOCK ",
            "EXP3_P7A_CLOCK ",
        )

        if text.count("EXP3_P7A_CLOCK ") != 1:
            raise SystemExit("P7A clock marker replacement failed")

        if text.count("exp3_p7a_wait_for_prefill(") != 2:
            raise SystemExit("P7A expected one helper definition plus one call")

        if text.count("EXP3_P7A_PREFILL_BYTES") < 4:
            raise SystemExit("P7A prefill constant use unexpectedly sparse")

        if text.count("exp3_p6_wait_for_picture(") != 4:
            raise SystemExit("P7A unexpectedly changed P6 scheduler structure")

        if text.count("graph_wait_vsync();") != 5:
            raise SystemExit("P7A unexpectedly changed P6 VSYNC structure")

        if "frames_dropped" in text:
            raise SystemExit("P7A must not contain frame-drop behavior")

        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text)

        generated_blob = subprocess.check_output(
            ["git", "hash-object", str(output)],
            text=True,
        ).strip()

        print(f"P7A_OUTPUT={output}")
        print(f"P7A_OUTPUT_BLOB={generated_blob}")
        print(f"P7A_PREFILL_BYTES={PREFILL_BYTES}")
        print("P7A_GENERATION=PASS")

    finally:
        try:
            base.unlink()
        except FileNotFoundError:
            pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
