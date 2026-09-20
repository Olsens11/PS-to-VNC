/*
 * File synopsis:
 * Implements Proof 6's PS2 libmpeg/IPU/GS adapter. The adapter reuses the
 * qualified mechanisms established by Media Stream EXP3 and the later H1 MPEG
 * campaign: explicit IPU/DMAC known-state preparation, aligned TO_IPU DMA,
 * PS2SDK libmpeg, macroblock GIF upload, and visible GS presentation.
 *
 * Exact-generation policy, Wire ownership and safe-stop semantics do not live
 * here. src/mpeg/decoder.c owns decoder-call boundaries and consumes MPEG only
 * through product Transport.
 */

#include "proof6_mpeg_platform_ps2.h"

#include "libmpeg.h"

#include <dma.h>
#include <dma_tags.h>
#include <draw.h>
#include <ee_regs.h>
#include <gif_tags.h>
#include <graph.h>
#include <gs_gp.h>
#include <gs_psm.h>
#include <kernel.h>
#include <packet.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <tamtypes.h>

#define PROOF6_SCREEN_WIDTH 640
#define PROOF6_SCREEN_HEIGHT 512

#define PROOF6_DMAC_START 0x00000100u
#define PROOF6_IPU_CTRL_BUSY 0x80000000u
#define PROOF6_IPU_CTRL_RST 0x40000000u
#define PROOF6_IPU_CMD_BCLR 0x00000000u

typedef struct proof6_mpeg_platform_state {
    pstvnc_mpeg_feed_callback_t feed_callback;
    void *feed_context;
    pstvnc_mpeg_sequence_callback_t sequence_callback;
    void *sequence_context;

    MPEGSequenceInfo *sequence;
    void *picture_data;
    size_t picture_capacity;

    packet_t *stage_packet;
    packet_t *transfer_packet;
    packet_t *draw_packet;

    framebuffer_t frame;
    zbuffer_t z;

    int texture_vram_address;
    int decoder_initialized;
    s64 current_stream_pts;
} proof6_mpeg_platform_state_t;

static proof6_mpeg_platform_state_t *proof6_state(
    proof6_mpeg_platform_t *platform)
{
    return platform != NULL
        ? (proof6_mpeg_platform_state_t *)platform->state
        : NULL;
}

static void proof6_reference_ipu_reset(void)
{
    int interrupt_was_enabled;
    u32 dmac_enable_state;
    u32 old_ipu_control;

    interrupt_was_enabled = DIntr();
    dmac_enable_state = *R_EE_D_ENABLER;

    *R_EE_D_ENABLEW = dmac_enable_state | 0x00010000u;
    EE_SYNCL();

    *R_EE_D3_CHCR &= ~PROOF6_DMAC_START;
    *R_EE_D4_CHCR &= ~PROOF6_DMAC_START;
    *R_EE_D3_MADR = 0;
    *R_EE_D4_MADR = 0;
    *R_EE_D3_QWC = 0;
    *R_EE_D4_QWC = 0;

    EE_SYNCL();
    *R_EE_D_ENABLEW = dmac_enable_state;

    if (interrupt_was_enabled)
        EIntr();

    old_ipu_control =
        *R_EE_IPU_CTRL & ~(PROOF6_IPU_CTRL_BUSY | PROOF6_IPU_CTRL_RST);

    *R_EE_IPU_CTRL = PROOF6_IPU_CTRL_RST;
    while (*R_EE_IPU_CTRL & PROOF6_IPU_CTRL_BUSY) {
    }

    *R_EE_IPU_CMD = PROOF6_IPU_CMD_BCLR;
    while (*R_EE_IPU_CTRL & PROOF6_IPU_CTRL_BUSY) {
    }

    *R_EE_IPU_CTRL = old_ipu_control;
}

static void proof6_show_solid_internal(
    proof6_mpeg_platform_state_t *state,
    int red,
    int green,
    int blue)
{
    qword_t *q;

    if (state == NULL || state->stage_packet == NULL)
        return;

    dma_wait_fast();

    q = state->stage_packet->data;
    q = draw_setup_environment(q, 0, &state->frame, &state->z);
    q = draw_clear(
        q,
        0,
        0,
        0,
        (float)PROOF6_SCREEN_WIDTH,
        (float)PROOF6_SCREEN_HEIGHT,
        red,
        green,
        blue);

    dma_channel_send_normal(
        DMA_CHANNEL_GIF,
        state->stage_packet->data,
        q - state->stage_packet->data,
        0,
        0);

    graph_wait_vsync();
    graph_wait_vsync();
}

static int proof6_prepare_known_state(void *context)
{
    proof6_mpeg_platform_t *platform =
        (proof6_mpeg_platform_t *)context;
    proof6_mpeg_platform_state_t *state = proof6_state(platform);

    if (platform == NULL || state == NULL)
        return -1;

    memset(&state->frame, 0, sizeof(state->frame));
    memset(&state->z, 0, sizeof(state->z));

    state->frame.width = PROOF6_SCREEN_WIDTH;
    state->frame.height = PROOF6_SCREEN_HEIGHT;
    state->frame.mask = 0;
    state->frame.psm = GS_PSM_32;
    state->frame.address = graph_vram_allocate(
        state->frame.width,
        state->frame.height,
        state->frame.psm,
        GRAPH_ALIGN_PAGE);

    state->z.enable = 0;
    state->z.mask = 0;
    state->z.method = 0;
    state->z.zsm = 0;
    state->z.address = 0;

    state->stage_packet = packet_init(100, PACKET_NORMAL);
    if (state->stage_packet == NULL)
        return -1;

    dma_channel_initialize(DMA_CHANNEL_toIPU, NULL, 0);
    dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
    dma_channel_fast_waits(DMA_CHANNEL_GIF);

    graph_initialize(
        0,
        PROOF6_SCREEN_WIDTH,
        PROOF6_SCREEN_HEIGHT,
        GS_PSM_32,
        0,
        0);

    state->texture_vram_address =
        graph_vram_allocate(0, 0, GS_PSM_32, GRAPH_ALIGN_BLOCK);

    proof6_reference_ipu_reset();

    platform->graphics_ready = 1;
    proof6_show_solid_internal(state, 0, 0, 96);
    return 0;
}

static int proof6_feed_trampoline(void *context)
{
    proof6_mpeg_platform_t *platform =
        (proof6_mpeg_platform_t *)context;
    proof6_mpeg_platform_state_t *state = proof6_state(platform);
    int result;

    if (state == NULL || state->feed_callback == NULL)
        return 0;

    result = state->feed_callback(state->feed_context);
    return result > 0 ? 1 : 0;
}

static void *proof6_sequence_trampoline(
    void *context,
    MPEGSequenceInfo *sequence)
{
    proof6_mpeg_platform_t *platform =
        (proof6_mpeg_platform_t *)context;
    proof6_mpeg_platform_state_t *state = proof6_state(platform);
    void *picture_data;
    size_t picture_capacity = 0u;
    int macroblock_width;
    int macroblock_height;
    int texture_buffer_width;
    int texture_width;
    int texture_height;
    int texture_address;
    int x;
    int y;
    char *image;
    qword_t *q;

    if (state == NULL || sequence == NULL ||
        state->sequence_callback == NULL)
        return NULL;

    picture_data = state->sequence_callback(
        state->sequence_context,
        (uint32_t)sequence->m_Width,
        (uint32_t)sequence->m_Height,
        &picture_capacity);
    if (picture_data == NULL || picture_capacity == 0u)
        return NULL;

    if (state->transfer_packet != NULL) {
        packet_free(state->transfer_packet);
        state->transfer_packet = NULL;
    }
    if (state->draw_packet != NULL) {
        packet_free(state->draw_packet);
        state->draw_packet = NULL;
    }

    state->sequence = sequence;
    state->picture_data = picture_data;
    state->picture_capacity = picture_capacity;

    macroblock_width = (sequence->m_Width + 15) >> 4;
    macroblock_height = (sequence->m_Height + 15) >> 4;
    texture_buffer_width = (sequence->m_Width + 63) >> 6;
    texture_width = draw_log2(sequence->m_Width);
    texture_height = draw_log2(sequence->m_Height);
    texture_address = state->texture_vram_address >> 6;

    SyncDCache(
        picture_data,
        (char *)picture_data + picture_capacity);

    state->transfer_packet = packet_init(
        (10 + 12 * macroblock_width * macroblock_height) >> 1,
        PACKET_NORMAL);
    if (state->transfer_packet == NULL)
        return NULL;

    image = (char *)picture_data;
    q = state->transfer_packet->data;

    DMATAG_CNT(q, 3, 0, 0, 0);
    q++;

    PACK_GIFTAG(
        q,
        GIF_SET_TAG(2, 0, 0, 0, 0, 1),
        GIF_REG_AD);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_TRXREG(16, 16),
        GS_REG_TRXREG);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_BITBLTBUF(
            0,
            0,
            0,
            texture_address,
            texture_buffer_width,
            GS_PSM_32),
        GS_REG_BITBLTBUF);
    q++;

    for (y = 0; y < sequence->m_Height; y += 16) {
        for (x = 0; x < sequence->m_Width; x += 16, image += 1024) {
            DMATAG_CNT(q, 4, 0, 0, 0);
            q++;

            PACK_GIFTAG(
                q,
                GIF_SET_TAG(2, 0, 0, 0, 0, 1),
                GIF_REG_AD);
            q++;

            PACK_GIFTAG(
                q,
                GS_SET_TRXPOS(0, 0, x, y, 0),
                GS_REG_TRXPOS);
            q++;

            PACK_GIFTAG(
                q,
                GS_SET_TRXDIR(0),
                GS_REG_TRXDIR);
            q++;

            PACK_GIFTAG(
                q,
                GIF_SET_TAG(64, 1, 0, 0, 2, 0),
                0);
            q++;

            DMATAG_REF(
                q,
                64,
                (unsigned int)image,
                0,
                0,
                0);
            q++;
        }
    }

    state->transfer_packet->qwc =
        q - state->transfer_packet->data;

    state->draw_packet = packet_init(7, PACKET_NORMAL);
    if (state->draw_packet == NULL)
        return NULL;

    q = state->draw_packet->data;

    PACK_GIFTAG(
        q,
        GIF_SET_TAG(6, 1, 0, 0, 0, 1),
        GIF_REG_AD);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_TEX0(
            texture_address,
            texture_buffer_width,
            GS_PSM_32,
            texture_width,
            texture_height,
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
        GS_SET_XYZ(2048 << 4, 2048 << 4, 0),
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
            (PROOF6_SCREEN_WIDTH << 4) + (2048 << 4),
            (PROOF6_SCREEN_HEIGHT << 4) + (2048 << 4),
            0),
        GS_REG_XYZ2);
    q++;

    state->draw_packet->qwc =
        q - state->draw_packet->data;

    return picture_data;
}

static int proof6_initialize_decoder(
    void *context,
    pstvnc_mpeg_feed_callback_t feed_callback,
    void *feed_context,
    pstvnc_mpeg_sequence_callback_t sequence_callback,
    void *sequence_context)
{
    proof6_mpeg_platform_t *platform =
        (proof6_mpeg_platform_t *)context;
    proof6_mpeg_platform_state_t *state = proof6_state(platform);

    if (state == NULL || feed_callback == NULL ||
        sequence_callback == NULL)
        return -1;

    state->feed_callback = feed_callback;
    state->feed_context = feed_context;
    state->sequence_callback = sequence_callback;
    state->sequence_context = sequence_context;
    state->current_stream_pts = 0;

    MPEG_Initialize(
        proof6_feed_trampoline,
        platform,
        proof6_sequence_trampoline,
        platform,
        &state->current_stream_pts);

    state->decoder_initialized = 1;
    return 0;
}

static int proof6_picture(void *context)
{
    proof6_mpeg_platform_t *platform =
        (proof6_mpeg_platform_t *)context;
    proof6_mpeg_platform_state_t *state = proof6_state(platform);
    s64 picture_pts = 0;
    int result;

    if (platform == NULL || state == NULL ||
        !state->decoder_initialized)
        return -1;

    platform->in_mpeg_picture = 1;
    result = MPEG_Picture(state->picture_data, &picture_pts);
    platform->in_mpeg_picture = 0;

    if (result <= 0)
        return result;

    if (state->transfer_packet == NULL ||
        state->draw_packet == NULL ||
        state->picture_data == NULL)
        return -1;

    dma_wait_fast();

    dma_channel_send_chain(
        DMA_CHANNEL_GIF,
        state->transfer_packet->data,
        state->transfer_packet->qwc,
        0,
        0);

    graph_wait_vsync();
    graph_wait_vsync();

    dma_channel_send_normal(
        DMA_CHANNEL_GIF,
        state->draw_packet->data,
        state->draw_packet->qwc,
        0,
        0);

    platform->pictures_presented += 1u;
    return 1;
}

static int proof6_submit_feed(
    void *context,
    const uint8_t *bytes,
    size_t payload_bytes,
    size_t transfer_bytes)
{
    (void)context;

    if (bytes == NULL || payload_bytes == 0u ||
        transfer_bytes < payload_bytes ||
        (transfer_bytes & 15u) != 0u)
        return -1;

    SyncDCache(
        (void *)bytes,
        (void *)(bytes + transfer_bytes));

    if (dma_channel_wait(DMA_CHANNEL_toIPU, 0) != 0)
        return -1;

    dma_channel_send_normal(
        DMA_CHANNEL_toIPU,
        (void *)bytes,
        transfer_bytes >> 4,
        0,
        0);

    return 0;
}

static int proof6_destroy_decoder(void *context)
{
    proof6_mpeg_platform_t *platform =
        (proof6_mpeg_platform_t *)context;
    proof6_mpeg_platform_state_t *state = proof6_state(platform);

    if (state == NULL)
        return -1;

    dma_channel_wait(DMA_CHANNEL_GIF, 0);
    dma_channel_wait(DMA_CHANNEL_toIPU, 0);

    if (state->decoder_initialized) {
        MPEG_Destroy();
        state->decoder_initialized = 0;
    }

    if (state->transfer_packet != NULL) {
        packet_free(state->transfer_packet);
        state->transfer_packet = NULL;
    }

    if (state->draw_packet != NULL) {
        packet_free(state->draw_packet);
        state->draw_packet = NULL;
    }

    state->sequence = NULL;
    state->picture_data = NULL;
    state->picture_capacity = 0u;
    return 0;
}

static int proof6_release_known_state(void *context)
{
    proof6_mpeg_platform_t *platform =
        (proof6_mpeg_platform_t *)context;

    if (platform == NULL || proof6_state(platform) == NULL)
        return -1;

    dma_channel_wait(DMA_CHANNEL_GIF, 0);
    dma_channel_wait(DMA_CHANNEL_toIPU, 0);
    proof6_reference_ipu_reset();
    return 0;
}

int proof6_mpeg_platform_context_initialize(
    proof6_mpeg_platform_t *platform,
    pstvnc_mpeg_decoder_t *decoder)
{
    proof6_mpeg_platform_state_t *state;

    if (platform == NULL || decoder == NULL)
        return 0;

    memset(platform, 0, sizeof(*platform));

    state = (proof6_mpeg_platform_state_t *)calloc(1, sizeof(*state));
    if (state == NULL)
        return 0;

    platform->decoder = decoder;
    platform->state = state;
    return 1;
}

void proof6_mpeg_platform_make_ops(
    proof6_mpeg_platform_t *platform,
    pstvnc_mpeg_decoder_platform_ops_t *ops)
{
    if (ops == NULL)
        return;

    memset(ops, 0, sizeof(*ops));
    ops->prepare_known_state = proof6_prepare_known_state;
    ops->initialize = proof6_initialize_decoder;
    ops->picture = proof6_picture;
    ops->submit_feed = proof6_submit_feed;
    ops->destroy = proof6_destroy_decoder;
    ops->release_known_state = proof6_release_known_state;
    ops->context = platform;
}

void proof6_mpeg_platform_show_solid(
    proof6_mpeg_platform_t *platform,
    int red,
    int green,
    int blue)
{
    proof6_show_solid_internal(proof6_state(platform), red, green, blue);
}

void proof6_mpeg_platform_context_release(
    proof6_mpeg_platform_t *platform)
{
    proof6_mpeg_platform_state_t *state;

    if (platform == NULL)
        return;

    state = proof6_state(platform);
    if (state != NULL) {
        if (state->decoder_initialized)
            MPEG_Destroy();
        if (state->transfer_packet != NULL)
            packet_free(state->transfer_packet);
        if (state->draw_packet != NULL)
            packet_free(state->draw_packet);
        if (state->stage_packet != NULL)
            packet_free(state->stage_packet);
        free(state);
    }

    memset(platform, 0, sizeof(*platform));
}
