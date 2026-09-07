#!/usr/bin/env python3
"""
File synopsis:
    Generates EXP3 P11 from the exact P9 RGB16 stage-timing harness.

Purpose:
    P10 increased only MPEG-2 bitrate from the clean 2 Mb/s benchmark to a
    visually excellent 4 Mb/s stream. Average decode/upload/draw cost remained
    comfortably below the 33.37 ms frame budget, but the run developed 409
    decoder feed-wait events, 56 presentation deadline misses, and one
    MPEG_Picture() call lasting ~451 ms. The maximum feed-wait span (~436 ms)
    nearly explains that maximum decoder-call duration, so the worst P10 timing
    is input-starvation-contaminated rather than evidence of intrinsic IPU
    decode cost.

    P11 changes only the compressed-input reservoir policy relative to P10:

      * replay queue: 256 KiB -> 512 KiB (Pi-side apparatus setting),
      * mandatory startup prefill: 224 KiB -> 448 KiB (this generator).

    P11 deliberately retains P9/P10:
      * exact 608x416 4 Mb/s source supplied by the replay apparatus,
      * SMS _MPEG_Set16(1) / PSMCT16 output,
      * P6 absolute 30000/1001 presentation clock,
      * no-drop policy,
      * MPEG feed callback behavior,
      * GS upload/draw path,
      * post-playback P9 stage timing instrumentation.

    The purpose is to remove compressed-input starvation before attributing
    any remaining MPEG_Picture() cost to the decoder itself.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys


EXPECTED_P9_GENERATOR_BLOB = "240b735d4452351729a9a8e82d889ea9ce3831fd"
EXPECTED_P9_OUTPUT_BLOB = "5567605e2068596cb943911c2c4fa20de9b9a056"
P11_PREFILL_BYTES = 448 * 1024


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

    p9_generator = source.parent / "generate_network_live_streaming_p9_stage_timing.py"

    actual_generator_blob = subprocess.check_output(
        ["git", "hash-object", str(p9_generator)],
        text=True,
    ).strip()

    print(f"P11_P9_GENERATOR_BLOB={actual_generator_blob}")

    if actual_generator_blob != EXPECTED_P9_GENERATOR_BLOB:
        raise SystemExit(
            "P9 generator authority mismatch: "
            f"expected={EXPECTED_P9_GENERATOR_BLOB} actual={actual_generator_blob}"
        )

    base = output.with_name(output.name + ".p9-base.tmp")

    try:
        subprocess.run(
            [
                sys.executable,
                str(p9_generator),
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

        print(f"P11_P9_BASE_BLOB={actual_base_blob}")

        if actual_base_blob != EXPECTED_P9_OUTPUT_BLOB:
            raise SystemExit(
                "P9 generated-source authority mismatch: "
                f"expected={EXPECTED_P9_OUTPUT_BLOB} actual={actual_base_blob}"
            )

        text = base.read_text()

        text = replace_once(
            text,
            "#define EXP3_P7A_PREFILL_BYTES (224u * 1024u)\n",
            "#define EXP3_P7A_PREFILL_BYTES (448u * 1024u)\n",
            "P11 deeper prefill",
        )

        if text.count("_MPEG_Set16(1);") != 1:
            raise SystemExit("P11 must retain P9 RGB16 exactly once")

        if text.count("EXP3_P9_TIMING ") != 1:
            raise SystemExit("P11 must retain P9 post-playback timing telemetry")

        if text.count("#define EXP3_P7A_PREFILL_BYTES (448u * 1024u)") != 1:
            raise SystemExit("P11 448 KiB prefill constant missing")

        if "#define EXP3_P7A_PREFILL_BYTES (224u * 1024u)" in text:
            raise SystemExit("P11 retained the old 224 KiB prefill")

        if text.count("exp3_p6_wait_for_picture(") != 4:
            raise SystemExit("P11 unexpectedly changed P6 scheduler structure")

        if text.count("exp3_p7a_wait_for_prefill(") != 2:
            raise SystemExit("P11 unexpectedly changed prefill helper structure")

        if text.count("exp3_p9_accumulate_timing(") != 7:
            raise SystemExit("P11 unexpectedly changed P9 timing structure")

        if "frames_dropped" in text:
            raise SystemExit("P11 must not add frame dropping")

        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text)

        generated_blob = subprocess.check_output(
            ["git", "hash-object", str(output)],
            text=True,
        ).strip()

        print(f"P11_OUTPUT={output}")
        print(f"P11_OUTPUT_BLOB={generated_blob}")
        print("P11_BASE=P9_RGB16_STAGE_TIMING")
        print(f"P11_PREFILL_BYTES={P11_PREFILL_BYTES}")
        print("P11_EXPECTED_QUEUE_BYTES=524288")
        print("P11_ONLY_PLAYBACK_CHANGE=COMPRESSED_RESERVOIR_POLICY")
        print("P11_GENERATION=PASS")

    finally:
        try:
            base.unlink()
        except FileNotFoundError:
            pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
