/*
 * File synopsis:
 * Implements H1's P11-derived MPEG-2 video session consumer.
 *
 * Qualified mechanisms retained from P11:
 *   - mature SMS libmpeg;
 *   - explicit IPU/DMAC quiesce before MPEG_Initialize;
 *   - normal TO_IPU DMA with an aligned staging buffer;
 *   - RGB16 compatibility path through _MPEG_Set16(1);
 *   - macroblock-oriented GIF source-chain upload;
 *   - ordinary GIF-channel ownership waits;
 *   - absolute 30000/1001-style presentation scheduling;
 *   - P9 decode/upload/draw timing.
 *
 * H1 changes ownership around those mechanisms: MPEG bytes come from the H1
 * mux queue, parameters are connection-scoped, the first video presentation
 * boundary arms the shared A/V epoch, and MPEG_Destroy()/packet_free() retire
 * all session-owned decoder/display resources before the next connection.
 */

#include "h1_video_runtime.h"

#include "libmpeg.h"
#include "libmpeg_internal.h"

#include <dma.h>
#include <dma_tags.h>
#include <draw.h>
#include <ee_regs.h>
#include <gif_tags.h>
#include <graph.h>
#include <gs_gp.h>
#include <gs_psm.h>
#include <kernel.h>
#include <malloc.h>
#include <packet.h>
#include <timer.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tamtypes.h>

#define H1_SCREEN_WIDTH 640
#define H1_SCREEN_HEIGHT 512

#define H1_P11_MAX_WIDTH 704u
#define H1_P11_MAX_HEIGHT 480u
#define H1_P11_RGB16_BYTES \
    (H1_P11_MAX_WIDTH * H1_P11_MAX_HEIGHT * 2u)
#define H1_P11_FEED_BYTES 2048u

#define H1_DMAC_START 0x00000100u
#define H1_IPU_CTRL_BUSY 0x80000000u
#define H1_IPU_CTRL_RST 0x40000000u
#define H1_IPU_CMD_BCLR 0x00000000u

static unsigned char s_h1_p11_picture_buffer[H1_P11_RGB16_BYTES]
    __attribute__((aligned(64)));

static unsigned char s_h1_p11_feed_buffer[H1_P11_FEED_BYTES]
    __attribute__((aligned(64)));

typedef struct h1_video_chassis {
    int initialized;
    framebuffer_t frame;
    zbuffer_t z;
    packet_t *stage_packet;
    int texture_vram_address;
} h1_video_chassis_t;

typedef struct h1_video_session {
    pstvnc_h1_transport_runtime_t *transport;
    pstvnc_h1_media_clock_t *clock;
    const pstvnc_h1_config_t *config;
    pstvnc_h1_video_result_t *result;

    MPEGSequenceInfo *sequence_info;
    s64 current_stream_pts;

    packet_t *transfer_packet;
    packet_t *draw_packet;

    unsigned char *picture_buffer;
    void *picture_allocation;
    size_t picture_capacity;

    unsigned char *feed_buffer;
    void *feed_allocation;
    size_t feed_capacity;

    int mpeg_initialized;
} h1_video_session_t;

static h1_video_chassis_t s_h1_chassis;

static void h1_video_record_error(
    h1_video_session_t *session,
    pstvnc_h1_video_error_t error)
{
    if (session != NULL &&
        session->result != NULL &&
        session->result->error == PSTVNC_H1_VIDEO_ERROR_NONE)
        session->result->error = error;
}

static void *h1_video_allocate_aligned64(
    size_t byte_count,
    void **allocation)
{
    uintptr_t address;
    void *raw;

    if (allocation == NULL ||
        byte_count == 0u ||
        byte_count > ((size_t)-1 - 63u))
        return NULL;

    raw = malloc(byte_count + 63u);
    if (raw == NULL)
        return NULL;

    address = ((uintptr_t)raw + 63u) & ~(uintptr_t)63u;
    *allocation = raw;
    return (void *)address;
}

/*
 * Establish the GS drawing environment required by the MPEG presentation
 * packets. This is renderer initialization, not diagnostic presentation, so
 * it must run regardless of whether visible stage markers are enabled.
 */
static void h1_video_setup_environment(void)
{
    qword_t *q;

    if (!s_h1_chassis.initialized || s_h1_chassis.stage_packet == NULL)
        return;

    dma_channel_wait(DMA_CHANNEL_GIF, 0);

    q = s_h1_chassis.stage_packet->data;
    q = draw_setup_environment(
        q,
        0,
        &s_h1_chassis.frame,
        &s_h1_chassis.z);

    dma_channel_send_normal(
        DMA_CHANNEL_GIF,
        s_h1_chassis.stage_packet->data,
        q - s_h1_chassis.stage_packet->data,
        0,
        0);
    dma_channel_wait(DMA_CHANNEL_GIF, 0);
}

static void h1_video_show_color(int red, int green, int blue)
{
    qword_t *q;

    if (!s_h1_chassis.initialized || s_h1_chassis.stage_packet == NULL)
        return;

    dma_channel_wait(DMA_CHANNEL_GIF, 0);

    q = s_h1_chassis.stage_packet->data;
    q = draw_setup_environment(
        q,
        0,
        &s_h1_chassis.frame,
        &s_h1_chassis.z);
    q = draw_clear(
        q,
        0,
        0,
        0,
        (float)H1_SCREEN_WIDTH,
        (float)H1_SCREEN_HEIGHT,
        red,
        green,
        blue);

    dma_channel_send_normal(
        DMA_CHANNEL_GIF,
        s_h1_chassis.stage_packet->data,
        q - s_h1_chassis.stage_packet->data,
        0,
        0);
    dma_channel_wait(DMA_CHANNEL_GIF, 0);
    graph_wait_vsync();
    graph_wait_vsync();
}

static void h1_video_hold_vsyncs(uint32_t count)
{
    uint32_t index;

    for (index = 0u; index < count; ++index)
        graph_wait_vsync();
}

static void h1_video_stage(
    const pstvnc_h1_config_t *config,
    int red,
    int green,
    int blue,
    uint32_t hold_multiplier)
{
    uint64_t total;

    if (config == NULL || config->video_stage_markers == 0u)
        return;

    h1_video_show_color(red, green, blue);

    total =
        (uint64_t)config->video_stage_hold_vsyncs *
        (uint64_t)hold_multiplier;

    if (total > 0xffffffffull)
        total = 0xffffffffull;

    h1_video_hold_vsyncs((uint32_t)total);
}

static void h1_video_reference_ipu_reset(void)
{
    int interrupt_was_enabled;
    u32 dmac_enable_state;
    u32 old_ipu_control;

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
    while (*R_EE_IPU_CTRL & H1_IPU_CTRL_BUSY) {
    }

    *R_EE_IPU_CMD = H1_IPU_CMD_BCLR;
    while (*R_EE_IPU_CTRL & H1_IPU_CTRL_BUSY) {
    }

    *R_EE_IPU_CTRL = old_ipu_control;
}

static void h1_video_accumulate_timing(
    u64 elapsed,
    uint32_t *samples,
    uint64_t *total,
    uint64_t *maximum)
{
    if (samples == NULL || total == NULL || maximum == NULL)
        return;

    *samples += 1u;
    *total += (uint64_t)elapsed;

    if ((uint64_t)elapsed > *maximum)
        *maximum = (uint64_t)elapsed;
}

static int h1_video_prepare_buffers(h1_video_session_t *session)
{
    uint32_t bytes_per_pixel;
    uint64_t macroblocks;
    uint64_t picture_bytes;
    uint64_t rounded_feed;

    bytes_per_pixel =
        session->config->video_pixel_mode == PSTVNC_H1_VIDEO_RGB16 ? 2u : 4u;

    macroblocks =
        ((uint64_t)session->config->video_max_width / 16ull) *
        ((uint64_t)session->config->video_max_height / 16ull);

    picture_bytes = macroblocks * 256ull * (uint64_t)bytes_per_pixel;

    if (picture_bytes == 0u ||
        picture_bytes > (uint64_t)(size_t)-1)
        return 0;

    if (session->config->video_pixel_mode == PSTVNC_H1_VIDEO_RGB16 &&
        session->config->video_max_width <= H1_P11_MAX_WIDTH &&
        session->config->video_max_height <= H1_P11_MAX_HEIGHT) {
        session->picture_buffer = s_h1_p11_picture_buffer;
        session->picture_capacity = sizeof(s_h1_p11_picture_buffer);
    } else {
        session->picture_buffer =
            (unsigned char *)h1_video_allocate_aligned64(
                (size_t)picture_bytes,
                &session->picture_allocation);
        session->picture_capacity = (size_t)picture_bytes;

        if (session->picture_buffer == NULL)
            return 0;
    }

    rounded_feed =
        ((uint64_t)session->config->mpeg_feed_bytes + 15ull) & ~15ull;

    if (rounded_feed == 0u || rounded_feed > (uint64_t)(size_t)-1)
        return 0;

    if (session->config->mpeg_feed_bytes == H1_P11_FEED_BYTES) {
        session->feed_buffer = s_h1_p11_feed_buffer;
        session->feed_capacity = sizeof(s_h1_p11_feed_buffer);
    } else {
        session->feed_buffer =
            (unsigned char *)h1_video_allocate_aligned64(
                (size_t)rounded_feed,
                &session->feed_allocation);
        session->feed_capacity = (size_t)rounded_feed;

        if (session->feed_buffer == NULL)
            return 0;
    }

    return 1;
}

static int h1_video_feed_ipu(void *user_data)
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

    if (!pstvnc_h1_transport_mpeg_read(
            session->transport,
            session->feed_buffer,
            session->config->mpeg_feed_bytes,
            &payload_size))
        return 0;

    if (payload_size == 0u ||
        payload_size > session->config->mpeg_feed_bytes)
        return 0;

    payload_bytes = (uint32_t)payload_size;
    dma_bytes = (payload_bytes + 15u) & ~15u;

    if ((size_t)dma_bytes > session->feed_capacity)
        return 0;

    if (dma_bytes > payload_bytes) {
        memset(
            session->feed_buffer + payload_bytes,
            0,
            dma_bytes - payload_bytes);
    }

    if (dma_channel_wait(DMA_CHANNEL_toIPU, 0) != 0)
        return 0;

    dma_qwc = dma_bytes >> 4;

    dma_channel_send_normal(
        DMA_CHANNEL_toIPU,
        session->feed_buffer,
        dma_qwc,
        0,
        0);

    session->result->feed_calls += 1u;
    session->result->payload_bytes_submitted += payload_bytes;
    session->result->dma_bytes_submitted += dma_bytes;
    return 1;
}

static void *h1_video_sequence_init(
    void *user_data,
    MPEGSequenceInfo *sequence)
{
    h1_video_session_t *session = (h1_video_session_t *)user_data;
    uint32_t bytes_per_pixel;
    uint32_t macroblock_bytes;
    uint32_t macroblock_qwc;
    uint32_t gs_psm;
    uint32_t mb_width;
    uint32_t mb_height;
    uint64_t required_bytes;
    uint64_t packet_qwords;
    int texture_buffer_width;
    int texture_width_log2;
    int texture_height_log2;
    int texture_address_64;
    int x;
    int y;
    unsigned char *image_ptr;
    qword_t *q;

    if (session == NULL || sequence == NULL)
        return NULL;

    session->sequence_info = sequence;
    session->result->sequence_callbacks += 1u;
    session->result->sequence_width = sequence->m_Width;
    session->result->sequence_height = sequence->m_Height;

    if (sequence->m_Width == 0u ||
        sequence->m_Height == 0u ||
        sequence->m_Width > session->config->video_max_width ||
        sequence->m_Height > session->config->video_max_height)
        return NULL;

    bytes_per_pixel =
        session->config->video_pixel_mode == PSTVNC_H1_VIDEO_RGB16 ? 2u : 4u;
    macroblock_bytes = 256u * bytes_per_pixel;
    macroblock_qwc = macroblock_bytes >> 4;
    gs_psm =
        session->config->video_pixel_mode == PSTVNC_H1_VIDEO_RGB16 ?
            GS_PSM_16 : GS_PSM_32;

    mb_width = (sequence->m_Width + 15u) >> 4;
    mb_height = (sequence->m_Height + 15u) >> 4;
    required_bytes =
        (uint64_t)mb_width *
        (uint64_t)mb_height *
        (uint64_t)macroblock_bytes;

    if (required_bytes > (uint64_t)session->picture_capacity)
        return NULL;

    SyncDCache(
        session->picture_buffer,
        session->picture_buffer + (size_t)required_bytes);

    texture_buffer_width = (sequence->m_Width + 63u) >> 6;
    texture_width_log2 = draw_log2(sequence->m_Width);
    texture_height_log2 = draw_log2(sequence->m_Height);
    texture_address_64 = s_h1_chassis.texture_vram_address >> 6;

    packet_qwords =
        (10ull + 12ull * (uint64_t)mb_width * (uint64_t)mb_height) >> 1;

    if (packet_qwords == 0u || packet_qwords > 0x7fffffffull)
        return NULL;

    session->transfer_packet =
        packet_init((int)packet_qwords, PACKET_NORMAL);
    session->draw_packet = packet_init(7, PACKET_NORMAL);

    if (session->transfer_packet == NULL || session->draw_packet == NULL)
        return NULL;

    image_ptr = session->picture_buffer;
    q = session->transfer_packet->data;

    DMATAG_CNT(q, 3, 0, 0, 0);
    q++;

    PACK_GIFTAG(
        q,
        GIF_SET_TAG(2, 0, 0, 0, 0, 1),
        GIF_REG_AD);
    q++;

    PACK_GIFTAG(q, GS_SET_TRXREG(16, 16), GS_REG_TRXREG);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_BITBLTBUF(
            0,
            0,
            0,
            texture_address_64,
            texture_buffer_width,
            gs_psm),
        GS_REG_BITBLTBUF);
    q++;

    for (y = 0; y < (int)sequence->m_Height; y += 16) {
        for (x = 0; x < (int)sequence->m_Width; x += 16) {
            DMATAG_CNT(q, 4, 0, 0, 0);
            q++;

            PACK_GIFTAG(
                q,
                GIF_SET_TAG(2, 0, 0, 0, 0, 1),
                GIF_REG_AD);
            q++;

            PACK_GIFTAG(q, GS_SET_TRXPOS(0, 0, x, y, 0), GS_REG_TRXPOS);
            q++;

            PACK_GIFTAG(q, GS_SET_TRXDIR(0), GS_REG_TRXDIR);
            q++;

            PACK_GIFTAG(
                q,
                GIF_SET_TAG(macroblock_qwc, 1, 0, 0, 2, 0),
                0);
            q++;

            DMATAG_REF(
                q,
                macroblock_qwc,
                (unsigned int)image_ptr,
                0,
                0,
                0);
            q++;

            image_ptr += macroblock_bytes;
        }
    }

    session->transfer_packet->qwc =
        q - session->transfer_packet->data;

    q = session->draw_packet->data;

    PACK_GIFTAG(
        q,
        GIF_SET_TAG(6, 1, 0, 0, 0, 1),
        GIF_REG_AD);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_TEX0(
            texture_address_64,
            texture_buffer_width,
            gs_psm,
            texture_width_log2,
            texture_height_log2,
            1,
            1,
            0,
            0,
            0,
            0,
            0),
        GS_REG_TEX0_1);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0),
        GS_REG_PRIM);
    q++;

    PACK_GIFTAG(q, GS_SET_UV(0, 0), GS_REG_UV);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_XYZ(
            ((int)session->config->video_draw_x << 4) + (2048 << 4),
            ((int)session->config->video_draw_y << 4) + (2048 << 4),
            0),
        GS_REG_XYZ2);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_UV(
            sequence->m_Width << 4,
            sequence->m_Height << 4),
        GS_REG_UV);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_XYZ(
            (((int)session->config->video_draw_x +
              (int)session->config->video_draw_width) << 4) +
                (2048 << 4),
            (((int)session->config->video_draw_y +
              (int)session->config->video_draw_height) << 4) +
                (2048 << 4),
            0),
        GS_REG_XYZ2);
    q++;

    session->draw_packet->qwc = q - session->draw_packet->data;

    printf(
        "H1_VIDEO_SEQUENCE width=%u height=%u bpp=%u mbw=%u mbh=%u transfer_qwc=%u\n",
        (unsigned int)sequence->m_Width,
        (unsigned int)sequence->m_Height,
        (unsigned int)bytes_per_pixel,
        (unsigned int)mb_width,
        (unsigned int)mb_height,
        (unsigned int)session->transfer_packet->qwc);

    return session->picture_buffer;
}

static int h1_video_wait_prefill(h1_video_session_t *session)
{
    uint32_t loops = 0u;
    uint32_t target = session->config->mpeg_start_target_bytes;

    session->result->prefill_target = target;
    session->result->prefill_observed = 0u;

    if (target == 0u)
        return 1;

    for (;;) {
        size_t current = pstvnc_h1_transport_mpeg_queue_size(session->transport);

        if (current > session->result->prefill_observed)
            session->result->prefill_observed = (uint32_t)current;

        if (current >= target)
            return 1;

        if (pstvnc_h1_transport_last_error(session->transport) !=
            PSTVNC_H1_ERROR_NONE)
            return 0;

        if (session->transport->end_received)
            return 0;

        if (session->config->mpeg_prefill_max_loops != 0u &&
            loops >= session->config->mpeg_prefill_max_loops)
            return 0;

        loops += 1u;

        if (DelayThread(session->config->mpeg_prefill_wait_us) < 0)
            return 0;
    }
}

static void h1_video_upload(h1_video_session_t *session)
{
    dma_channel_wait(DMA_CHANNEL_GIF, 0);
    dma_channel_send_chain(
        DMA_CHANNEL_GIF,
        session->transfer_packet->data,
        session->transfer_packet->qwc,
        0,
        0);
    dma_channel_wait(DMA_CHANNEL_GIF, 0);
}

static void h1_video_draw(h1_video_session_t *session)
{
    dma_channel_wait(DMA_CHANNEL_GIF, 0);
    dma_channel_send_normal(
        DMA_CHANNEL_GIF,
        session->draw_packet->data,
        session->draw_packet->qwc,
        0,
        0);
    dma_channel_wait(DMA_CHANNEL_GIF, 0);
}

static u64 h1_video_frame_period_ticks(const h1_video_session_t *session)
{
    return
        ((u64)kBUSCLK * (u64)session->config->video_fps_den) /
        (u64)session->config->video_fps_num;
}

static int h1_video_wait_for_picture(
    h1_video_session_t *session,
    uint32_t picture_index,
    int *drop_picture)
{
    u64 now;
    u64 deadline;
    u64 picture_offset;
    u64 frame_period;

    *drop_picture = 0;

    switch (session->config->video_scheduler_mode) {
        case PSTVNC_H1_VIDEO_SCHED_NONE:
            if (!pstvnc_h1_media_clock_is_armed(session->clock))
                pstvnc_h1_media_clock_arm_now(session->clock);
            return 1;

        case PSTVNC_H1_VIDEO_SCHED_ONE_VSYNC:
            if (!pstvnc_h1_media_clock_is_armed(session->clock))
                pstvnc_h1_media_clock_arm_now(session->clock);
            graph_wait_vsync();
            session->result->vsync_waits += 1u;
            return 1;

        case PSTVNC_H1_VIDEO_SCHED_TWO_VSYNC:
            if (!pstvnc_h1_media_clock_is_armed(session->clock))
                pstvnc_h1_media_clock_arm_now(session->clock);
            graph_wait_vsync();
            graph_wait_vsync();
            session->result->vsync_waits += 2u;
            return 1;

        case PSTVNC_H1_VIDEO_SCHED_ABSOLUTE:
        default:
            break;
    }

    if (!pstvnc_h1_media_clock_is_armed(session->clock)) {
        /* Exact P11 authority point: first real display boundary owns epoch. */
        graph_wait_vsync();
        session->result->vsync_waits += 1u;
        pstvnc_h1_media_clock_arm_now(session->clock);
    }

    frame_period = h1_video_frame_period_ticks(session);
    picture_offset =
        picture_index <= 1u ? 0u :
        (u64)(picture_index - 1u) * frame_period;

    deadline = pstvnc_h1_media_clock_deadline(
        session->clock,
        pstvnc_h1_config_video_offset_us(session->config),
        picture_offset);

    now = GetTimerSystemTime();

    if (now >= deadline) {
        u64 lateness = now - deadline;

        if (picture_index > 1u) {
            session->result->deadline_misses += 1u;

            if ((uint64_t)lateness >
                session->result->max_deadline_late_ticks)
                session->result->max_deadline_late_ticks =
                    (uint64_t)lateness;
        }

        if (session->config->video_drop_enabled != 0u) {
            u64 threshold =
                (frame_period *
                 (u64)session->config->video_drop_threshold_milliframes) /
                1000u;

            if (lateness >= threshold)
                *drop_picture = 1;
        }

        return 1;
    }

    while (now < deadline) {
        graph_wait_vsync();
        session->result->vsync_waits += 1u;
        now = GetTimerSystemTime();
    }

    return 1;
}

static void h1_video_release_session(h1_video_session_t *session)
{
    dma_channel_wait(DMA_CHANNEL_GIF, 0);
    dma_channel_wait(DMA_CHANNEL_toIPU, 0);

    if (session->mpeg_initialized) {
        MPEG_Destroy();
        session->mpeg_initialized = 0;
    }

    if (session->transfer_packet != NULL) {
        packet_free(session->transfer_packet);
        session->transfer_packet = NULL;
    }

    if (session->draw_packet != NULL) {
        packet_free(session->draw_packet);
        session->draw_packet = NULL;
    }

    free(session->picture_allocation);
    free(session->feed_allocation);
    session->picture_allocation = NULL;
    session->feed_allocation = NULL;
    session->picture_buffer = NULL;
    session->feed_buffer = NULL;
}

int pstvnc_h1_video_chassis_init(void)
{
    if (s_h1_chassis.initialized)
        return 0;

    memset(&s_h1_chassis, 0, sizeof(s_h1_chassis));

    s_h1_chassis.frame.width = H1_SCREEN_WIDTH;
    s_h1_chassis.frame.height = H1_SCREEN_HEIGHT;
    s_h1_chassis.frame.mask = 0;
    s_h1_chassis.frame.psm = GS_PSM_32;
    s_h1_chassis.frame.address = graph_vram_allocate(
        H1_SCREEN_WIDTH,
        H1_SCREEN_HEIGHT,
        GS_PSM_32,
        GRAPH_ALIGN_PAGE);

    s_h1_chassis.z.enable = 0;
    s_h1_chassis.z.mask = 0;
    s_h1_chassis.z.method = 0;
    s_h1_chassis.z.zsm = 0;
    s_h1_chassis.z.address = 0;

    s_h1_chassis.stage_packet = packet_init(100, PACKET_NORMAL);
    if (s_h1_chassis.stage_packet == NULL)
        return -1;

    dma_channel_initialize(DMA_CHANNEL_toIPU, NULL, 0);
    dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);

    graph_initialize(
        0,
        H1_SCREEN_WIDTH,
        H1_SCREEN_HEIGHT,
        GS_PSM_32,
        0,
        0);

    s_h1_chassis.texture_vram_address = graph_vram_allocate(
        0,
        0,
        GS_PSM_16,
        GRAPH_ALIGN_BLOCK);

    s_h1_chassis.initialized = 1;

    printf(
        "H1_VIDEO_CHASSIS=READY texture_vram=%d\n",
        s_h1_chassis.texture_vram_address);

    return 0;
}

int pstvnc_h1_video_run_session(
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_media_clock_t *clock,
    pstvnc_h1_video_result_t *result)
{
    h1_video_session_t session;
    const pstvnc_h1_config_t *config;
    int picture_result;
    uint32_t picture_index;
    s64 picture_pts;
    u64 stage_start;
    u64 elapsed;
    int drop_picture;
    int success = 0;

    if (transport == NULL || clock == NULL || result == NULL)
        return -1;

    config = pstvnc_h1_transport_config(transport);
    if (config == NULL || config->video_mode != PSTVNC_H1_VIDEO_MPEG2_ES)
        return -1;

    if (!s_h1_chassis.initialized)
        return -1;

    memset(&session, 0, sizeof(session));
    memset(result, 0, sizeof(*result));

    session.transport = transport;
    session.clock = clock;
    session.config = config;
    session.result = result;

    if (!h1_video_prepare_buffers(&session)) {
        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_ALLOCATION);
        goto done;
    }

    /*
     * MPEG draw packets depend on this GS state. Stage markers may add
     * diagnostic clears, but enabling diagnostics must never be required for
     * ordinary presentation.
     */
    h1_video_setup_environment();

    h1_video_stage(config, 0, 160, 160, 1u);

    if (config->video_ipu_reset_each_session != 0u)
        h1_video_reference_ipu_reset();

    h1_video_stage(config, 160, 160, 0, 1u);

    MPEG_Initialize(
        h1_video_feed_ipu,
        &session,
        h1_video_sequence_init,
        &session,
        &session.current_stream_pts);
    session.mpeg_initialized = 1;

    _MPEG_Set16(
        config->video_pixel_mode == PSTVNC_H1_VIDEO_RGB16 ? 1 : 0);

    h1_video_stage(config, 192, 64, 0, 2u);

    if (!h1_video_wait_prefill(&session)) {
        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_PREFILL);
        goto done;
    }

    picture_pts = 0;
    stage_start = GetTimerSystemTime();
    picture_result = MPEG_Picture(NULL, &picture_pts);
    elapsed = GetTimerSystemTime() - stage_start;

    h1_video_accumulate_timing(
        elapsed,
        &result->decode_samples,
        &result->decode_ticks_total,
        &result->decode_ticks_max);

    if (picture_result == 0 ||
        session.transfer_packet == NULL ||
        session.draw_packet == NULL) {
        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_DECODE);
        goto done;
    }

    result->pictures_decoded = 1u;
    h1_video_stage(config, 0, 192, 0, 1u);

    stage_start = GetTimerSystemTime();
    h1_video_upload(&session);
    elapsed = GetTimerSystemTime() - stage_start;

    h1_video_accumulate_timing(
        elapsed,
        &result->upload_samples,
        &result->upload_ticks_total,
        &result->upload_ticks_max);

    h1_video_stage(config, 0, 0, 192, 1u);

    if (!h1_video_wait_for_picture(&session, 1u, &drop_picture)) {
        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_DISPLAY);
        goto done;
    }

    if (!drop_picture) {
        stage_start = GetTimerSystemTime();
        h1_video_draw(&session);
        elapsed = GetTimerSystemTime() - stage_start;

        h1_video_accumulate_timing(
            elapsed,
            &result->draw_samples,
            &result->draw_ticks_total,
            &result->draw_ticks_max);
        result->pictures_displayed = 1u;
    } else {
        result->pictures_dropped = 1u;
    }

    for (picture_index = 2u; ; ++picture_index) {
        picture_pts = 0;
        stage_start = GetTimerSystemTime();
        picture_result = MPEG_Picture(session.picture_buffer, &picture_pts);
        elapsed = GetTimerSystemTime() - stage_start;

        h1_video_accumulate_timing(
            elapsed,
            &result->decode_samples,
            &result->decode_ticks_total,
            &result->decode_ticks_max);

        if (picture_result == 0)
            break;

        result->pictures_decoded += 1u;

        if (!h1_video_wait_for_picture(
                &session,
                picture_index,
                &drop_picture)) {
            h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_DISPLAY);
            goto done;
        }

        if (drop_picture) {
            result->pictures_dropped += 1u;
            continue;
        }

        stage_start = GetTimerSystemTime();
        h1_video_upload(&session);
        elapsed = GetTimerSystemTime() - stage_start;

        h1_video_accumulate_timing(
            elapsed,
            &result->upload_samples,
            &result->upload_ticks_total,
            &result->upload_ticks_max);

        stage_start = GetTimerSystemTime();
        h1_video_draw(&session);
        elapsed = GetTimerSystemTime() - stage_start;

        h1_video_accumulate_timing(
            elapsed,
            &result->draw_samples,
            &result->draw_ticks_total,
            &result->draw_ticks_max);

        result->pictures_displayed += 1u;
    }

    if (!pstvnc_h1_transport_mpeg_exhausted(transport)) {
        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_DECODE);
        goto done;
    }

    success = 1;

done:
    h1_video_release_session(&session);
    return success ? 0 : -1;
}
