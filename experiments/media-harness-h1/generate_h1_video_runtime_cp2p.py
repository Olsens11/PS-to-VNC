#!/usr/bin/env python3
"""Generate the CP2P MPEG runtime from the exact reviewed H1 runtime source.

The transform is intentionally narrow and fail-closed. It retains the current
H1 libmpeg/IPU/feed/timing/scheduler implementation, removes only the standalone
GIF/GS packet owner, and binds successful physical presentation to the calibrated
CP2P first-frame ownership contract.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

EXPECTED_INPUT_BLOB_SHA1 = "00156442504e752b8d8fdf0af55612f7faa92e4c"


def git_blob_sha1(data: bytes) -> str:
    header = f"blob {len(data)}\0".encode("ascii")
    return hashlib.sha1(header + data).hexdigest()


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def replace_function(text: str, anchor: str, replacement: str) -> str:
    start = text.find(anchor)
    if start < 0:
        raise RuntimeError(f"missing function anchor: {anchor}")

    brace = text.find("{", start)
    if brace < 0:
        raise RuntimeError(f"missing opening brace after: {anchor}")

    depth = 0
    end = None
    for index in range(brace, len(text)):
        char = text[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                end = index + 1
                break

    if end is None:
        raise RuntimeError(f"unbalanced function body after: {anchor}")

    return text[:start] + replacement.rstrip() + text[end:]


def generate(source: str) -> str:
    source = replace_once(
        source,
        '#include "h1_video_runtime.h"\n',
        '#include "h1_video_runtime.h"\n'
        '#include "h1_video_runtime_cp2p.h"\n'
        '#include "h1_cumulative39_graphics.h"\n',
        "CP2P include injection",
    )

    source = replace_once(
        source,
        "    pstvnc_h1_video_result_t *result;\n\n"
        "    MPEGSequenceInfo *sequence_info;",
        "    pstvnc_h1_video_result_t *result;\n"
        "    pstvnc_h1_mpeg_start_handoff_t *handoff;\n"
        "    pstvnc_h1_mpeg_start_contract_t start_contract;\n"
        "    const volatile int *stop_requested;\n"
        "    unsigned cancelled : 1;\n\n"
        "    MPEGSequenceInfo *sequence_info;",
        "session ownership fields",
    )

    source = replace_function(
        source,
        "static void h1_video_setup_environment(void)",
        """static void h1_video_setup_environment(void)
{
    /* GIF/GS environment belongs exclusively to the shared compositor. */
}""",
    )

    source = replace_function(
        source,
        "static void h1_video_show_color(int red, int green, int blue)",
        """static void h1_video_show_color(int red, int green, int blue)
{
    /* Historical MPEG stage colors must never overwrite shared RFB graphics. */
    (void)red;
    (void)green;
    (void)blue;
}""",
    )

    source = replace_once(
        source,
        "    if (!pstvnc_h1_transport_mpeg_read(\n"
        "            session->transport,\n"
        "            session->feed_buffer,\n"
        "            session->config->mpeg_feed_bytes,\n"
        "            &payload_size))\n"
        "        return 0;",
        "    if (!pstvnc_h1_transport_mpeg_read_cancellable(\n"
        "            session->transport,\n"
        "            session->feed_buffer,\n"
        "            session->config->mpeg_feed_bytes,\n"
        "            &payload_size,\n"
        "            session->stop_requested)) {\n"
        "        if (session->stop_requested != NULL && *session->stop_requested)\n"
        "            session->cancelled = 1;\n"
        "        return 0;\n"
        "    }",
        "CP2P cancellable MPEG feed",
    )

    source = replace_once(
        source,
        "    for (;;) {\n"
        "        size_t current = pstvnc_h1_transport_mpeg_queue_size(session->transport);",
        "    for (;;) {\n"
        "        size_t current;\n\n"
        "        if (session->stop_requested != NULL && *session->stop_requested) {\n"
        "            session->cancelled = 1;\n"
        "            return 0;\n"
        "        }\n\n"
        "        current = pstvnc_h1_transport_mpeg_queue_size(session->transport);",
        "CP2P cancellable prefill",
    )

    source = replace_function(
        source,
        "static void *h1_video_sequence_init(",
        """static void *h1_video_sequence_init(
    void *user_data,
    MPEGSequenceInfo *sequence)
{
    h1_video_session_t *session = (h1_video_session_t *)user_data;
    uint32_t mb_width;
    uint32_t mb_height;
    uint64_t required_bytes;

    if (session == NULL || sequence == NULL)
        return NULL;

    session->sequence_info = sequence;
    session->result->sequence_callbacks += 1u;
    session->result->sequence_width = sequence->m_Width;
    session->result->sequence_height = sequence->m_Height;

    /* The shared compositor is intentionally the proven RGB16 path only. */
    if (session->config->video_pixel_mode != PSTVNC_H1_VIDEO_RGB16 ||
        sequence->m_Width == 0u || sequence->m_Height == 0u ||
        sequence->m_Width > session->config->video_max_width ||
        sequence->m_Height > session->config->video_max_height ||
        (sequence->m_Width & 15u) != 0u ||
        (sequence->m_Height & 15u) != 0u)
        return NULL;

    mb_width = sequence->m_Width >> 4;
    mb_height = sequence->m_Height >> 4;
    required_bytes =
        (uint64_t)mb_width * (uint64_t)mb_height * 256ull * 2ull;

    if (required_bytes == 0u ||
        required_bytes > (uint64_t)session->picture_capacity)
        return NULL;

    SyncDCache(
        session->picture_buffer,
        session->picture_buffer + (size_t)required_bytes);

    printf(
        "H1_VIDEO_SEQUENCE width=%u height=%u bpp=2 mbw=%u mbh=%u "
        "shared_compositor=1\\n",
        (unsigned int)sequence->m_Width,
        (unsigned int)sequence->m_Height,
        (unsigned int)mb_width,
        (unsigned int)mb_height);

    return session->picture_buffer;
}""",
    )

    source = replace_function(
        source,
        "static void h1_video_upload(h1_video_session_t *session)",
        """static void h1_video_upload(h1_video_session_t *session)
{
    /* libmpeg already decoded into EE memory; compositor owns GS upload. */
    (void)session;
}""",
    )

    source = replace_function(
        source,
        "static void h1_video_draw(h1_video_session_t *session)",
        """static int h1_video_draw(h1_video_session_t *session)
{
    pstvnc_h1_mpeg_presentation_owner_state_t state;

    if (session == NULL || session->sequence_info == NULL ||
        session->handoff == NULL)
        return 0;

    state = pstvnc_h1_mpeg_presentation_owner_state(
        &session->handoff->owner);
    if (state != PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME &&
        state != PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED) {
        h1_video_record_error(session, PSTVNC_H1_VIDEO_ERROR_DISPLAY);
        return 0;
    }

    if (pstvnc_h1_graphics_present_video_macroblocks(
            (const uint16_t *)session->picture_buffer,
            session->sequence_info->m_Width,
            session->sequence_info->m_Height,
            &session->start_contract) < 0) {
        h1_video_record_error(session, PSTVNC_H1_VIDEO_ERROR_DISPLAY);
        return 0;
    }

    /*
     * The compositor returns only after gsKit_sync_flip(), so promotion here is
     * the first physical frame boundary rather than merely a decoded-buffer fact.
     */
    if (state == PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME &&
        !pstvnc_h1_mpeg_start_handoff_first_frame_presented(
            session->handoff,
            session->start_contract.generation)) {
        h1_video_record_error(session, PSTVNC_H1_VIDEO_ERROR_DISPLAY);
        return 0;
    }

    return 1;
}""",
    )

    source = replace_function(
        source,
        "static void h1_video_release_session(h1_video_session_t *session)",
        """static void h1_video_release_session(h1_video_session_t *session)
{
    pstvnc_h1_mpeg_presentation_owner_state_t state;

    dma_channel_wait(DMA_CHANNEL_toIPU, 0);

    if (session->mpeg_initialized) {
        MPEG_Destroy();
        session->mpeg_initialized = 0;
    }

    free(session->picture_allocation);
    free(session->feed_allocation);
    session->picture_allocation = NULL;
    session->feed_allocation = NULL;
    session->picture_buffer = NULL;
    session->feed_buffer = NULL;

    if (session->handoff == NULL)
        return;

    state = pstvnc_h1_mpeg_presentation_owner_state(
        &session->handoff->owner);

    /*
     * A start that never crossed the physical first-frame boundary must fail
     * back to full RFB immediately. Once MPEG is visible, keep its last frame
     * owned until the concurrent RFB worker is proven dormant and the caller
     * explicitly retires presentation.
     */
    if (state == PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME &&
        !session->cancelled) {
        (void)pstvnc_h1_graphics_clear_video();
        (void)pstvnc_h1_mpeg_start_handoff_abort_start(
            session->handoff,
            session->start_contract.generation);
    }
}""",
    )

    source = replace_function(
        source,
        "int pstvnc_h1_video_chassis_init(void)",
        """int pstvnc_h1_video_chassis_init(void)
{
    if (s_h1_chassis.initialized)
        return 0;

    memset(&s_h1_chassis, 0, sizeof(s_h1_chassis));

    /* MPEG keeps TO_IPU; GIF/GS are initialized and owned by ps2_graphics. */
    dma_channel_initialize(DMA_CHANNEL_toIPU, NULL, 0);
    s_h1_chassis.initialized = 1;

    printf("H1_VIDEO_CHASSIS=READY shared_compositor=1\\n");
    return 0;
}""",
    )

    source = replace_once(
        source,
        "int pstvnc_h1_video_run_session(\n"
        "    pstvnc_h1_transport_runtime_t *transport,\n"
        "    pstvnc_h1_media_clock_t *clock,\n"
        "    pstvnc_h1_video_result_t *result)",
        "int pstvnc_h1_video_run_cp2p_session(\n"
        "    pstvnc_h1_transport_runtime_t *transport,\n"
        "    pstvnc_h1_media_clock_t *clock,\n"
        "    pstvnc_h1_video_result_t *result,\n"
        "    pstvnc_h1_mpeg_start_handoff_t *handoff,\n"
        "    const pstvnc_h1_mpeg_start_contract_t *start_contract,\n"
        "    const volatile int *stop_requested)",
        "CP2P run signature",
    )

    source = replace_once(
        source,
        "    if (transport == NULL || clock == NULL || result == NULL)\n"
        "        return -1;",
        "    if (transport == NULL || clock == NULL || result == NULL ||\n"
        "        handoff == NULL || start_contract == NULL)\n"
        "        return -1;\n\n"
        "    if (pstvnc_h1_mpeg_presentation_owner_state(&handoff->owner) !=\n"
        "            PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||\n"
        "        handoff->owner.generation != start_contract->generation)\n"
        "        return -1;",
        "CP2P run ownership precondition",
    )

    source = replace_once(
        source,
        "    session.result = result;\n",
        "    session.result = result;\n"
        "    session.handoff = handoff;\n"
        "    session.start_contract = *start_contract;\n"
        "    session.stop_requested = stop_requested;\n",
        "CP2P session ownership assignment",
    )

    source = replace_once(
        source,
        "    if (!h1_video_wait_prefill(&session)) {\n"
        "        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_PREFILL);\n"
        "        goto done;\n"
        "    }",
        "    if (!h1_video_wait_prefill(&session)) {\n"
        "        if (session.cancelled) {\n"
        "            success = 1;\n"
        "            goto done;\n"
        "        }\n"
        "        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_PREFILL);\n"
        "        goto done;\n"
        "    }",
        "CP2P clean cancellation during prefill",
    )

    source = replace_once(
        source,
        "    if (picture_result == 0 ||\n"
        "        session.transfer_packet == NULL ||\n"
        "        session.draw_packet == NULL) {\n"
        "        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_DECODE);\n"
        "        goto done;\n"
        "    }",
        "    if (picture_result == 0) {\n"
        "        if (session.cancelled) {\n"
        "            success = 1;\n"
        "            goto done;\n"
        "        }\n"
        "        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_DECODE);\n"
        "        goto done;\n"
        "    }",
        "CP2P shared-compositor first-picture guard",
    )

    source = replace_once(
        source,
        "        if (picture_result == 0)\n"
        "            break;",
        "        if (picture_result == 0) {\n"
        "            if (session.cancelled) {\n"
        "                success = 1;\n"
        "                goto done;\n"
        "            }\n"
        "            break;\n"
        "        }",
        "CP2P clean cancellation during decode loop",
    )

    draw_call = "        h1_video_draw(&session);"
    if source.count(draw_call) != 2:
        raise RuntimeError(
            f"CP2P draw call count: expected 2, found {source.count(draw_call)}"
        )
    source = source.replace(
        draw_call,
        "        if (!h1_video_draw(&session))\n            goto done;",
    )

    source += """

int pstvnc_h1_video_cp2p_retire_presentation(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    uint32_t generation)
{
    pstvnc_h1_mpeg_presentation_owner_state_t state;

    if (handoff == NULL || handoff->owner.generation != generation)
        return -1;

    state = pstvnc_h1_mpeg_presentation_owner_state(&handoff->owner);

    if (state == PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY)
        return 0;

    /* Caller guarantees the concurrent RFB worker is already dormant. */
    if (pstvnc_h1_graphics_clear_video() < 0)
        return -1;

    if (state == PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME)
        return pstvnc_h1_mpeg_start_handoff_abort_start(
            handoff, generation) ? 0 : -1;

    if (state == PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED)
        return pstvnc_h1_mpeg_start_handoff_stop(
            handoff, generation) ? 0 : -1;

    return -1;
}
"""

    forbidden = (
        "DMA_CHANNEL_GIF",
        "session.transfer_packet == NULL",
        "session.draw_packet == NULL",
        "graph_vram_allocate(",
        "dma_channel_send_chain(DMA_CHANNEL_GIF",
        "dma_channel_send_normal(\n        DMA_CHANNEL_GIF",
    )
    for marker in forbidden:
        if marker in source:
            raise RuntimeError(f"generated runtime retained forbidden GS owner: {marker}")

    required = (
        "pstvnc_h1_transport_mpeg_read_cancellable(",
        "pstvnc_h1_graphics_present_video_macroblocks(",
        "pstvnc_h1_mpeg_start_handoff_first_frame_presented(",
        "pstvnc_h1_video_cp2p_retire_presentation(",
        "shared_compositor=1",
    )
    for marker in required:
        if marker not in source:
            raise RuntimeError(f"generated runtime missing contract marker: {marker}")

    return source


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)
    data = input_path.read_bytes()
    actual_sha = git_blob_sha1(data)
    if actual_sha != EXPECTED_INPUT_BLOB_SHA1:
        raise SystemExit(
            "H1_CP2P_VIDEO_SOURCE_MISMATCH="
            f"expected:{EXPECTED_INPUT_BLOB_SHA1}:actual:{actual_sha}"
        )

    generated = generate(data.decode("utf-8"))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(generated, encoding="utf-8")

    print(f"H1_CP2P_VIDEO_SOURCE_BLOB={actual_sha}")
    print("H1_CP2P_VIDEO_RUNTIME_GENERATION=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
