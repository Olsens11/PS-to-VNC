#!/usr/bin/env python3
"""
File synopsis:
    Generates the EXP3 O3 P4 diagnostic harness by changing only the two
    per-picture VSYNC waits in the hardware-qualified O3 live-stream harness.

Purpose:
    P3 showed that the exact 600-picture P2 archive remains throughput-limited
    when replayed through the unchanged PS2 path. P4 keeps the transport,
    queue, SMS decoder, TO_IPU feed, GS upload, GS draw, terminal telemetry,
    and EOF behavior unchanged while removing only the two graph_wait_vsync()
    calls at the end of exp3_draw_picture_texture().

This is a throughput bisection, not a proposed product presentation policy.
The resulting stream is expected to run as fast as decode/upload/draw allow.
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

    print(f"P4_INPUT_BLOB={actual_blob}")

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
     * P4 throughput diagnostic: intentionally no presentation VSYNC waits.
     * GIF DMA completion above remains the ownership boundary. This changes
     * only display pacing; decode, upload, draw, transport, and EOF behavior
     * are otherwise identical to O3.
     */
}

int main(void)
'''

    count = text.count(old)
    if count != 1:
        raise SystemExit(
            "P4 draw-tail target is not unique: "
            f"expected=1 actual={count}"
        )

    text = text.replace(old, new, 1)

    old_synopsis = (
        " * upload, GS draw path, and two-VSYNC presentation cadence are unchanged.\n"
    )
    new_synopsis = (
        " * upload and GS draw path are unchanged. P4 alone removes the two per-picture\n"
        " * VSYNC waits as a controlled throughput diagnostic.\n"
    )

    if text.count(old_synopsis) != 1:
        raise SystemExit("P4 synopsis target missing or non-unique")

    text = text.replace(old_synopsis, new_synopsis, 1)

    if text.count("P4 throughput diagnostic") != 1:
        raise SystemExit("P4 diagnostic marker missing after transform")

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(text)

    generated_blob = subprocess.check_output(
        ["git", "hash-object", str(output)],
        text=True,
    ).strip()

    print(f"P4_OUTPUT={output}")
    print(f"P4_OUTPUT_BLOB={generated_blob}")
    print("P4_GENERATION=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
