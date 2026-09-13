#!/usr/bin/env python3
"""Generate the CP2P runtime plus disposable stall-stage instrumentation.

This wrapper first invokes the authoritative CP2P mechanical transform, then
adds observation-only writes to H1's existing 32-bit diagnostic_word around the
places capable of monopolizing the EE: IPU reset busy waits, MPEG_Initialize,
MPEG_Picture, TO_IPU DMA wait/feed, compositor presentation, and teardown.

No queue size, thread priority, timeout, scheduling rule, decoder policy, or
transport ownership is changed. A stage write is only a volatile 32-bit store.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import generate_h1_video_runtime_cp2p as basegen


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def generate(source: str) -> str:
    out = basegen.generate(source)

    # h1_video_session_t is already defined immediately before this anchor.
    out = replace_once(
        out,
        "static h1_video_chassis_t s_h1_chassis;\n",
        "static h1_video_chassis_t s_h1_chassis;\n\n"
        "/* Disposable CP2P stall-stage witness. High byte identifies stage family. */\n"
        "static void h1_diag_stage(h1_video_session_t *session, uint32_t stage)\n"
        "{\n"
        "    if (session != NULL && session->transport != NULL)\n"
        "        pstvnc_h1_transport_set_diagnostic_word(session->transport, stage);\n"
        "}\n\n"
        "static int h1_diag_mpeg_picture(\n"
        "    h1_video_session_t *session,\n"
        "    void *picture,\n"
        "    s64 *pts,\n"
        "    uint32_t picture_index)\n"
        "{\n"
        "    int result;\n"
        "    h1_diag_stage(session, 0xD3000000u | (picture_index & 0x00ffffffu));\n"
        "    result = MPEG_Picture(picture, pts);\n"
        "    h1_diag_stage(session, 0xD4000000u | (picture_index & 0x00ffffffu));\n"
        "    return result;\n"
        "}\n",
        "diagnostic helper injection",
    )

    out = basegen.replace_function(
        out,
        "static void h1_video_reference_ipu_reset(void)",
        """static void h1_video_reference_ipu_reset(h1_video_session_t *session)
{
    int interrupt_was_enabled;
    u32 dmac_enable_state;
    u32 old_ipu_control;

    h1_diag_stage(session, 0xD7000001u); /* IPU reset enter */
    interrupt_was_enabled = DIntr();
    dmac_enable_state = *R_EE_D_ENABLER;

    *R_EE_D_ENABLEW = dmac_enable_state | 0x00010000u;
    EE_SYNCL();

    *R_EE_D3_CHCR &= ~H1_DMAC_START;
    *R_EE_D4_CHCR &= ~H1_DMAC_START;
    *R_EE_D3_MADR = 0;
    *R_EE_D4_MADR = 0;
    *R_EE_D3_QWC = 0;
    *R_EE_D4_QWC = 0;

    EE_SYNCL();
    *R_EE_D_ENABLEW = dmac_enable_state;

    if (interrupt_was_enabled)
        EIntr();

    old_ipu_control =
        *R_EE_IPU_CTRL & ~(H1_IPU_CTRL_BUSY | H1_IPU_CTRL_RST);

    *R_EE_IPU_CTRL = H1_IPU_CTRL_RST;
    h1_diag_stage(session, 0xD7000002u); /* waiting for RST busy clear */
    while (*R_EE_IPU_CTRL & H1_IPU_CTRL_BUSY) {
    }
    h1_diag_stage(session, 0xD7000003u); /* RST busy cleared */

    *R_EE_IPU_CMD = H1_IPU_CMD_BCLR;
    h1_diag_stage(session, 0xD7000004u); /* waiting for BCLR busy clear */
    while (*R_EE_IPU_CTRL & H1_IPU_CTRL_BUSY) {
    }
    h1_diag_stage(session, 0xD7000005u); /* BCLR busy cleared */

    *R_EE_IPU_CTRL = old_ipu_control;
    h1_diag_stage(session, 0xD7000006u); /* IPU reset return */
}""",
    )

    out = replace_once(
        out,
        "        h1_video_reference_ipu_reset();",
        "        h1_video_reference_ipu_reset(&session);",
        "instrumented IPU reset call",
    )

    out = basegen.replace_function(
        out,
        "static int h1_video_feed_ipu(void *user_data)",
        """static int h1_video_feed_ipu(void *user_data)
{
    h1_video_session_t *session = (h1_video_session_t *)user_data;
    size_t payload_size = 0u;
    uint32_t payload_bytes;
    uint32_t dma_bytes;
    uint32_t dma_qwc;

    if (session == NULL ||
        session->transport == NULL ||
        session->feed_buffer == NULL)
        return 0;

    h1_diag_stage(session, 0xD5000001u); /* feed enter */
    h1_diag_stage(session, 0xD5000002u); /* before cancellable queue read */
    if (!pstvnc_h1_transport_mpeg_read_cancellable(
            session->transport,
            session->feed_buffer,
            session->config->mpeg_feed_bytes,
            &payload_size,
            session->stop_requested)) {
        if (session->stop_requested != NULL && *session->stop_requested)
            session->cancelled = 1;
        h1_diag_stage(session, 0xD5FF0001u); /* read returned false */
        return 0;
    }
    h1_diag_stage(session, 0xD5000003u); /* queue read returned */

    if (payload_size == 0u ||
        payload_size > session->config->mpeg_feed_bytes) {
        h1_diag_stage(session, 0xD5FF0002u);
        return 0;
    }

    payload_bytes = (uint32_t)payload_size;
    dma_bytes = (payload_bytes + 15u) & ~15u;

    if ((size_t)dma_bytes > session->feed_capacity) {
        h1_diag_stage(session, 0xD5FF0003u);
        return 0;
    }

    if (dma_bytes > payload_bytes) {
        memset(
            session->feed_buffer + payload_bytes,
            0,
            dma_bytes - payload_bytes);
    }

    h1_diag_stage(session, 0xD5000004u); /* before TO_IPU DMA wait */
    if (dma_channel_wait(DMA_CHANNEL_toIPU, 0) != 0) {
        h1_diag_stage(session, 0xD5FF0004u);
        return 0;
    }
    h1_diag_stage(session, 0xD5000005u); /* TO_IPU DMA wait returned */

    dma_qwc = dma_bytes >> 4;
    dma_channel_send_normal(
        DMA_CHANNEL_toIPU,
        session->feed_buffer,
        dma_qwc,
        0,
        0);
    h1_diag_stage(session, 0xD5000006u); /* TO_IPU DMA submitted */

    session->result->feed_calls += 1u;
    session->result->payload_bytes_submitted += payload_bytes;
    session->result->dma_bytes_submitted += dma_bytes;
    h1_diag_stage(session, 0xD5000007u); /* feed return success */
    return 1;
}""",
    )

    # Preserve the generated shared-compositor owner, adding only stage stores.
    out = basegen.replace_function(
        out,
        "static int h1_video_draw(h1_video_session_t *session)",
        """static int h1_video_draw(h1_video_session_t *session)
{
    pstvnc_h1_mpeg_presentation_owner_state_t state;

    h1_diag_stage(session, 0xD6000001u); /* draw enter */
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

    h1_diag_stage(session, 0xD6000002u); /* before compositor present */
    if (pstvnc_h1_graphics_present_video_macroblocks(
            (const uint16_t *)session->picture_buffer,
            session->sequence_info->m_Width,
            session->sequence_info->m_Height,
            &session->start_contract) < 0) {
        h1_video_record_error(session, PSTVNC_H1_VIDEO_ERROR_DISPLAY);
        h1_diag_stage(session, 0xD6FF0001u);
        return 0;
    }
    h1_diag_stage(session, 0xD6000003u); /* compositor present returned */

    if (state == PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME &&
        !pstvnc_h1_mpeg_start_handoff_first_frame_presented(
            session->handoff,
            session->start_contract.generation)) {
        h1_video_record_error(session, PSTVNC_H1_VIDEO_ERROR_DISPLAY);
        h1_diag_stage(session, 0xD6FF0002u);
        return 0;
    }

    h1_diag_stage(session, 0xD6000004u); /* draw return success */
    return 1;
}""",
    )

    out = replace_once(
        out,
        "    MPEG_Initialize(\n"
        "        h1_video_feed_ipu,\n"
        "        &session,\n"
        "        h1_video_sequence_init,\n"
        "        &session,\n"
        "        &session.current_stream_pts);",
        "    h1_diag_stage(&session, 0xD2000001u); /* before MPEG_Initialize */\n"
        "    MPEG_Initialize(\n"
        "        h1_video_feed_ipu,\n"
        "        &session,\n"
        "        h1_video_sequence_init,\n"
        "        &session,\n"
        "        &session.current_stream_pts);\n"
        "    h1_diag_stage(&session, 0xD2000002u); /* MPEG_Initialize returned */",
        "MPEG initialize stages",
    )

    out = replace_once(
        out,
        "    if (!h1_video_wait_prefill(&session)) {",
        "    h1_diag_stage(&session, 0xD2000003u); /* before prefill */\n"
        "    if (!h1_video_wait_prefill(&session)) {",
        "prefill entry stage",
    )
    out = replace_once(
        out,
        "    picture_pts = 0;\n    stage_start = GetTimerSystemTime();",
        "    h1_diag_stage(&session, 0xD2000004u); /* prefill complete */\n"
        "    picture_pts = 0;\n    stage_start = GetTimerSystemTime();",
        "prefill completion stage",
    )

    out = replace_once(
        out,
        "picture_result = MPEG_Picture(NULL, &picture_pts);",
        "picture_result = h1_diag_mpeg_picture(&session, NULL, &picture_pts, 1u);",
        "first MPEG_Picture wrapper",
    )
    out = replace_once(
        out,
        "picture_result = MPEG_Picture(session.picture_buffer, &picture_pts);",
        "picture_result = h1_diag_mpeg_picture(\n"
        "            &session, session.picture_buffer, &picture_pts, picture_index);",
        "loop MPEG_Picture wrapper",
    )

    # Teardown can also monopolize the worker if TO_IPU never quiesces.
    out = replace_once(
        out,
        "    dma_channel_wait(DMA_CHANNEL_toIPU, 0);\n\n"
        "    if (session->mpeg_initialized) {",
        "    h1_diag_stage(session, 0xD8000001u); /* before teardown TO_IPU wait */\n"
        "    dma_channel_wait(DMA_CHANNEL_toIPU, 0);\n"
        "    h1_diag_stage(session, 0xD8000002u); /* teardown TO_IPU wait returned */\n\n"
        "    if (session->mpeg_initialized) {",
        "teardown DMA stages",
    )
    out = replace_once(
        out,
        "        MPEG_Destroy();\n        session->mpeg_initialized = 0;",
        "        h1_diag_stage(session, 0xD8000003u); /* before MPEG_Destroy */\n"
        "        MPEG_Destroy();\n"
        "        h1_diag_stage(session, 0xD8000004u); /* MPEG_Destroy returned */\n"
        "        session->mpeg_initialized = 0;",
        "MPEG destroy stages",
    )

    required = (
        "0xD7000002u",
        "0xD5000004u",
        "h1_diag_mpeg_picture",
        "0xD6000002u",
        "0xD8000001u",
    )
    for marker in required:
        if marker not in out:
            raise RuntimeError(f"diagnostic generation missing marker: {marker}")

    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)
    data = input_path.read_bytes()
    actual_sha = basegen.git_blob_sha1(data)
    if actual_sha != basegen.EXPECTED_INPUT_BLOB_SHA1:
        raise SystemExit(
            "H1_CP2P_VIDEO_SOURCE_MISMATCH="
            f"expected:{basegen.EXPECTED_INPUT_BLOB_SHA1}:actual:{actual_sha}"
        )

    generated = generate(data.decode("utf-8"))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(generated, encoding="utf-8")

    print(f"H1_CP2P_VIDEO_SOURCE_BLOB={actual_sha}")
    print("H1_CP2P_MPEG_STALL_DIAGNOSTICS=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
