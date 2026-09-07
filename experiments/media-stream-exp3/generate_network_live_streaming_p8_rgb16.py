#!/usr/bin/env python3
"""
File synopsis:
    Generates EXP3 P8 from the exact P7A deep-buffer/absolute-clock harness.

Purpose:
    The deterministic 608x416 stress reel is now smooth with zero decoder feed
    waits and zero presentation deadline misses. P8 changes only the decoded
    picture / GS texture pixel format from the current SMS RGB32 path to the
    mature SMS RGB16 path.

    The public SMS MPEG-1/2 codec selects _MPEG_Set16(1) when its IPU texture
    format is PSMCT16. SMS_FrameBufferInit then allocates two bytes per pixel,
    uses 32-qword 16x16 macroblocks, 512-byte macroblock increments, and
    PSMCT16 GS transfers. The vendored libmpeg_core.S contains the same
    _MPEG_Set16 implementation.

    P8 deliberately retains P7A's:
      * P6 absolute presentation scheduler,
      * 256 KiB compressed queue supplied by the replay apparatus,
      * 224 KiB prefill gate,
      * no-drop policy,
      * MPEG feed path,
      * one-picture serialized decode/upload/draw structure.

    Only the SMS CSC output format and matching GS texture-transfer contract
    change. This gives a clean hardware A/B against the fixed stress reel.
"""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys


EXPECTED_P7A_GENERATOR_BLOB = "1b7918575a66cb3a83bac9d78b24a252fffc50a7"


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

    p7a_generator = (
        source.parent /
        "generate_network_live_streaming_p7a_deep_buffer_prefill.py"
    )

    actual_generator_blob = subprocess.check_output(
        ["git", "hash-object", str(p7a_generator)],
        text=True,
    ).strip()

    print(f"P8_P7A_GENERATOR_BLOB={actual_generator_blob}")

    if actual_generator_blob != EXPECTED_P7A_GENERATOR_BLOB:
        raise SystemExit(
            "P7A generator authority mismatch: "
            f"expected={EXPECTED_P7A_GENERATOR_BLOB} "
            f"actual={actual_generator_blob}"
        )

    base = output.with_name(output.name + ".p7a-base.tmp")

    try:
        subprocess.run(
            [
                sys.executable,
                str(p7a_generator),
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
            '#include "libmpeg.h"\n',
            '#include "libmpeg.h"\n#include "libmpeg_internal.h"\n',
            "SMS internal MPEG declaration include",
        )

        text = replace_once(
            text,
            "#define EXP3_PICTURE_BYTES \\\n"
            "    (EXP3_VIDEO_MAX_WIDTH * EXP3_VIDEO_MAX_HEIGHT * 4)\n",
            "#define EXP3_PICTURE_BYTES \\\n"
            "    (EXP3_VIDEO_MAX_WIDTH * EXP3_VIDEO_MAX_HEIGHT * 2)\n",
            "RGB16 picture-buffer size",
        )

        text = replace_once(
            text,
            "        sequence->m_Height *\n"
            "        4u;\n",
            "        sequence->m_Height *\n"
            "        2u;\n",
            "RGB16 required decoded bytes",
        )

        text = replace_once(
            text,
            "     * libmpeg produces RGBA32 as consecutive 16x16 macroblocks. Each\n"
            "     * macroblock is therefore 16 * 16 * 4 = 1024 bytes = 64 qwords.\n",
            "     * P8 enables SMS _MPEG_Set16(1), so libmpeg produces RGB16 as\n"
            "     * consecutive 16x16 macroblocks. Each macroblock is therefore\n"
            "     * 16 * 16 * 2 = 512 bytes = 32 qwords.\n",
            "RGB16 macroblock contract comment",
        )

        text = replace_once(
            text,
            "            GS_PSM_32),\n"
            "        GS_REG_BITBLTBUF);\n",
            "            GS_PSM_16),\n"
            "        GS_REG_BITBLTBUF);\n",
            "RGB16 BITBLTBUF",
        )

        text = replace_once(
            text,
            "            image_ptr += 1024\n",
            "            image_ptr += 512\n",
            "RGB16 macroblock byte increment",
        )

        text = replace_once(
            text,
            "                    64,\n"
            "                    1,\n"
            "                    0,\n"
            "                    0,\n"
            "                    2,\n"
            "                    0),\n",
            "                    32,\n"
            "                    1,\n"
            "                    0,\n"
            "                    0,\n"
            "                    2,\n"
            "                    0),\n",
            "RGB16 IMAGE GIF qword count",
        )

        text = replace_once(
            text,
            "            DMATAG_REF(\n"
            "                q,\n"
            "                64,\n",
            "            DMATAG_REF(\n"
            "                q,\n"
            "                32,\n",
            "RGB16 macroblock DMA REF qword count",
        )

        text = replace_once(
            text,
            "            GS_PSM_32,\n"
            "            texture_width_log2,\n",
            "            GS_PSM_16,\n"
            "            texture_width_log2,\n",
            "RGB16 TEX0",
        )

        text = replace_once(
            text,
            "        graph_vram_allocate(\n"
            "            0,\n"
            "            0,\n"
            "            GS_PSM_32,\n"
            "            GRAPH_ALIGN_BLOCK);\n",
            "        graph_vram_allocate(\n"
            "            0,\n"
            "            0,\n"
            "            GS_PSM_16,\n"
            "            GRAPH_ALIGN_BLOCK);\n",
            "RGB16 texture VRAM format",
        )

        initialize_call = '''    MPEG_Initialize(
        exp3_feed_ipu,
        &decoder,
        exp3_sequence_init,
        &decoder,
        &decoder.current_stream_pts);
'''

        initialize_call_rgb16 = initialize_call + '''
    /*
     * Mature SMS selects this exact libmpeg core mode whenever its IPU/GS
     * texture format is PSMCT16. The call patches the IPU CSC/DMA output path
     * itself; this is not an EE-side post-conversion.
     */
    _MPEG_Set16(1);

    printf(
        "EXP3_P8_RGB16=ENABLED bytes_per_pixel=2 macroblock_bytes=512 macroblock_qwc=32\\n");
'''

        text = replace_once(
            text,
            initialize_call,
            initialize_call_rgb16,
            "SMS RGB16 mode enable",
        )

        if text.count("_MPEG_Set16(1);") != 1:
            raise SystemExit("P8 must enable SMS RGB16 exactly once")

        if text.count("GS_PSM_16") != 3:
            raise SystemExit(
                "P8 expected exactly three GS_PSM_16 uses "
                "(BITBLTBUF, TEX0, texture allocation)"
            )

        if "image_ptr += 1024" in text:
            raise SystemExit("P8 retained a 32-bit macroblock increment")

        if text.count("image_ptr += 512") != 1:
            raise SystemExit("P8 expected one 512-byte macroblock increment")

        if text.count("exp3_p6_wait_for_picture(") != 4:
            raise SystemExit("P8 unexpectedly changed P6 scheduler structure")

        if text.count("exp3_p7a_wait_for_prefill(") != 2:
            raise SystemExit("P8 unexpectedly changed P7A prefill structure")

        if text.count("graph_wait_vsync();") != 5:
            raise SystemExit("P8 unexpectedly changed P7A VSYNC structure")

        if "frames_dropped" in text:
            raise SystemExit("P8 must not add frame-drop behavior")

        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text)

        generated_blob = subprocess.check_output(
            ["git", "hash-object", str(output)],
            text=True,
        ).strip()

        print(f"P8_OUTPUT={output}")
        print(f"P8_OUTPUT_BLOB={generated_blob}")
        print("P8_PIXEL_FORMAT=RGB16_PSMCT16")
        print("P8_BYTES_PER_PIXEL=2")
        print("P8_MACROBLOCK_BYTES=512")
        print("P8_MACROBLOCK_QWC=32")
        print("P8_GENERATION=PASS")

    finally:
        try:
            base.unlink()
        except FileNotFoundError:
            pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
