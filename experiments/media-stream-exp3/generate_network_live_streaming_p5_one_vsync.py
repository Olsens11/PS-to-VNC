#!/usr/bin/env python3
"""
File synopsis:
    Generates the EXP3 O3 P5 diagnostic harness by changing only the two
    per-picture VSYNC waits in the hardware-qualified O3 live-stream harness
    to exactly one per-picture VSYNC wait.

Purpose:
    P3 (two waits) was throughput-limited and visually slow/clunky. P4 (zero
    waits) exceeded the exact source's real-time byte-rate requirement but ran
    visibly fast/unpaced. P5 keeps transport, queue, SMS decoder, TO_IPU feed,
    GS upload, GS draw, EOF behavior, and source authority unchanged while
    using exactly one graph_wait_vsync() at the end of each picture draw.

This is a hardware timing bracket, not the final product timing policy.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess

EXPECTED_O3_BLOB = "4e34795823cbb386f7ac958c43a2ddbce4e255e5"


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

    print(f"P5_INPUT_BLOB={actual_blob}")

    if actual_blob != EXPECTED_O3_BLOB:
        raise SystemExit(
            "O3 harness authority mismatch: "
            f"expected={EXPECTED_O3_BLOB} actual={actual_blob}"
        )

    text = source.read_text()

    old = '''    graph_wait_vsync();
    graph_wait_vsync();
}

int main(void)
'''

    new = '''    /*
     * P5 timing bracket: exactly one presentation VSYNC wait per picture.
     * GIF DMA completion above remains the ownership boundary. Decode,
     * upload, draw, transport, and EOF behavior are otherwise unchanged.
     */
    graph_wait_vsync();
}

int main(void)
'''

    count = text.count(old)
    if count != 1:
        raise SystemExit(
            "P5 draw-tail target is not unique: "
            f"expected=1 actual={count}"
        )

    text = text.replace(old, new, 1)

    old_synopsis = (
        " * upload, GS draw path, and two-VSYNC presentation cadence are unchanged.\n"
    )
    new_synopsis = (
        " * upload and GS draw path are unchanged. P5 alone changes the two per-picture\n"
        " * VSYNC waits to exactly one as a controlled timing bracket.\n"
    )

    if text.count(old_synopsis) != 1:
        raise SystemExit("P5 synopsis target missing or non-unique")

    text = text.replace(old_synopsis, new_synopsis, 1)

    if text.count("P5 timing bracket") != 1:
        raise SystemExit("P5 diagnostic marker missing after transform")

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(text)

    generated_blob = subprocess.check_output(
        ["git", "hash-object", str(output)],
        text=True,
    ).strip()

    print(f"P5_OUTPUT={output}")
    print(f"P5_OUTPUT_BLOB={generated_blob}")
    print("P5_GENERATION=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
