#!/usr/bin/env python3
"""Derive the EXP3 O2 harness deterministically from the exact O1 source."""

from __future__ import annotations

import argparse
import pathlib
import subprocess

EXPECTED_O1_BLOB = "473d9c930c5e246b60a8685730ae11339fe1d6fd"


def once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected one target, found {count}")
    return text.replace(old, new, 1)


def region(text: str, start: str, end: str, new: str, label: str) -> str:
    a = text.find(start)
    if a < 0:
        raise SystemExit(f"{label}: start marker not found")
    b = text.find(end, a)
    if b < 0:
        raise SystemExit(f"{label}: end marker not found")
    if text.find(start, a + 1) >= 0:
        raise SystemExit(f"{label}: start marker not unique")
    return text[:a] + new + text[b:]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", required=True)
    ap.add_argument("--output", required=True)
    args = ap.parse_args()

    src = pathlib.Path(args.input)
    out = pathlib.Path(args.output)

    blob = subprocess.check_output(["git", "hash-object", str(src)], text=True).strip()
    if blob != EXPECTED_O1_BLOB:
        raise SystemExit(f"O1 source authority mismatch: expected={EXPECTED_O1_BLOB} actual={blob}")

    text = src.read_text()

    text = once(
        text,
        " * EXP3 O1 network MPEG-2 fixture-replay test.\n",
        " * EXP3 O2 concurrent network MPEG-2 fixture-streaming test.\n",
        "synopsis",
    )

    text = once(
        text,
        '#include "libmpeg.h"\n',
        '#include "libmpeg.h"\n#include "network_stream_runtime.h"\n',
        "runtime include",
    )

    text = once(
        text,
        "    const unsigned char *stream_begin;\n"
        "    const unsigned char *stream_cursor;\n"
        "    const unsigned char *stream_end;\n",
        "    Exp3MpegStreamRuntime *stream_runtime;\n",
        "decoder stream fields",
    )

    text = region(
        text,
        "typedef struct Exp3NetworkReplayStats\n",
        "static void exp3_show_color(\n",
        "",
        "remove O1 prebuffer helpers",
    )

    feed = r'''static int exp3_feed_ipu(void *user_data)
{
    Exp3DecoderState *state = (Exp3DecoderState *)user_data;
    size_t payload_size = 0;
    unsigned int payload_bytes;
    unsigned int dma_bytes;
    unsigned int dma_qwc;

    if (state == NULL || state->stream_runtime == NULL)
        return 0;

    if (!exp3_mpeg_stream_read(
            state->stream_runtime,
            s_feed_buffer,
            EXP3_FEED_BYTES,
            &payload_size))
        return 0;

    if (payload_size == 0 || payload_size > EXP3_FEED_BYTES)
        return 0;

    payload_bytes = (unsigned int)payload_size;
    dma_bytes = (payload_bytes + 15u) & ~15u;

    if (dma_bytes > EXP3_FEED_BYTES)
        return 0;

    if (dma_bytes > payload_bytes) {
        memset(
            s_feed_buffer + payload_bytes,
            0,
            dma_bytes - payload_bytes);
    }

    if (dma_channel_wait(DMA_CHANNEL_toIPU, 0) != 0)
        return 0;

    dma_qwc = dma_bytes >> 4;

    dma_channel_send_normal(
        DMA_CHANNEL_toIPU,
        s_feed_buffer,
        dma_qwc,
        0,
        0);

    state->feed_calls += 1;
    state->payload_bytes_submitted += payload_bytes;
    state->dma_bytes_submitted += dma_bytes;

    return 1;
}

'''

    text = region(
        text,
        "static int exp3_feed_ipu(void *user_data)\n",
        "/*\n * Called by libmpeg after it has parsed the sequence header.\n",
        feed,
        "streaming feed",
    )

    text = once(
        text,
        "    unsigned char *network_stream;\n"
        "    Exp3NetworkReplayStats network_stats;\n",
        "    Exp3MpegStreamRuntime stream_runtime;\n"
        "    int network_integrity;\n",
        "main runtime declarations",
    )

    text = once(
        text,
        "    memset(\n"
        "        &network_stats,\n"
        "        0,\n"
        "        sizeof(network_stats));\n",
        "    memset(\n"
        "        &stream_runtime,\n"
        "        0,\n"
        "        sizeof(stream_runtime));\n",
        "main runtime initialization",
    )

    startup = r'''    printf(
        "EXP3_NETWORK_STREAMING_FIXTURE_START expected_bytes=%u expected_crc32=%08x\n",
        EXP3_EXPECTED_STREAM_BYTES,
        EXP3_EXPECTED_STREAM_CRC32);

    printf("EXP3_NETWORK_STAGE=BEFORE_IOP_PREPARE\n");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        printf("EXP3_O2=IOP_PREPARE_FAIL\n");
        SleepThread();
        return 10;
    }

    printf("EXP3_NETWORK_STAGE=IOP_PREPARE_RETURNED\n");

    if (pstvnc_ps2_network_init() < 0) {
        printf("EXP3_O2=NETWORK_INIT_FAIL\n");
        SleepThread();
        return 11;
    }

    printf("EXP3_NETWORK_STAGE=NETWORK_INIT_RETURNED\n");

    if (pstvnc_ps2_network_wait_link() < 0) {
        printf("EXP3_O2=LINK_FAIL\n");
        SleepThread();
        return 12;
    }

    printf("EXP3_NETWORK_STAGE=LINK_UP\n");

    if (!exp3_mpeg_stream_start(&stream_runtime)) {
        printf("EXP3_O2_STREAM_START=FAIL error=%d\n", stream_runtime.error);
        SleepThread();
        return 14;
    }

    decoder.stream_runtime = &stream_runtime;

    printf(
        "EXP3_O2_STREAM_RUNTIME_READY queue_capacity=%u\n",
        (unsigned int)stream_runtime.queue_capacity);

'''

    text = region(
        text,
        "    printf(\n        \"EXP3_NETWORK_FIXTURE_REPLAY_START \"\n",
        "    frame.width =\n",
        startup,
        "main streaming startup",
    )

    text = once(
        text,
        "        stream_exhausted =\n"
        "            decoder.stream_cursor >=\n"
        "            decoder.stream_end;\n",
        "        stream_exhausted =\n"
        "            exp3_mpeg_stream_is_exhausted(\n"
        "                &stream_runtime);\n",
        "terminal exhaustion",
    )

    network_marker = r'''        /*
         * SECOND TERMINAL MARKER — concurrent network integrity.
         * CYAN = exact bytes/CRC, empty queue, receiver done, no stream error.
         * RED  = any mismatch.
         */
        network_integrity =
            exp3_mpeg_stream_integrity_pass(&stream_runtime);

        if (stream_exhausted && network_integrity) {
            exp3_show_color(packet, &frame, &z, 0, 160, 160);
            printf("EXP3_O2_STREAM_INTEGRITY=PASS\n");
        } else {
            exp3_show_color(packet, &frame, &z, 192, 0, 0);
            printf(
                "EXP3_O2_STREAM_INTEGRITY=FAIL exhausted=%d integrity=%d error=%d remaining=%u\n",
                stream_exhausted,
                network_integrity,
                stream_runtime.error,
                (unsigned int)exp3_mpeg_stream_remaining_bytes(&stream_runtime));
        }

        exp3_hold_vsyncs(60);

'''

    text = region(
        text,
        "        /*\n         * SECOND TERMINAL MARKER — input callback position\n",
        "        /*\n         * The last successfully uploaded picture remains resident",
        network_marker,
        "terminal network marker",
    )

    old_final = '''        exp3_draw_picture_texture(
            &decoder);

        printf(
            "EXP3_EOF_CHARACTERIZATION_FINAL_FRAME_HELD=PASS\\n");
'''

    new_final = '''        exp3_draw_picture_texture(
            &decoder);

        if (exp3_mpeg_stream_send_result(
                &stream_runtime,
                (uint32_t)pictures_decoded,
                (uint32_t)pictures_displayed,
                decoder.feed_calls,
                decoder.payload_bytes_submitted,
                decoder.dma_bytes_submitted)) {
            printf("EXP3_O2_RESULT_SEND=PASS\\n");
        } else {
            printf("EXP3_O2_RESULT_SEND=FAIL error=%d\\n", stream_runtime.error);
        }

        printf(
            "EXP3_EOF_CHARACTERIZATION_FINAL_FRAME_HELD=PASS\\n");
'''

    text = once(text, old_final, new_final, "terminal telemetry")

    for token in (
        "Exp3NetworkReplayStats",
        "exp3_receive_network_fixture",
        "network_stats",
        "stream_begin",
        "stream_cursor",
        "stream_end",
    ):
        if token in text:
            raise SystemExit(f"generated source still contains O1 token: {token}")

    if "dma_wait_fast();" in text:
        raise SystemExit("generated source reintroduced dma_wait_fast()")

    if "exp3_mpeg_stream_read(" not in text:
        raise SystemExit("generated source lacks streaming feed")

    if "exp3_mpeg_stream_send_result(" not in text:
        raise SystemExit("generated source lacks result telemetry")

    out.write_text(text)

    print(f"O2_GENERATED_SOURCE={out}")
    print("O2_GENERATION=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
