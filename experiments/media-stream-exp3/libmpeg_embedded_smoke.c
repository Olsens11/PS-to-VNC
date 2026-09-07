/*
 * File synopsis:
 * Standalone Media Stream EXP3 hardware smoke test for the PlayStation 2
 * MPEG-2 decoding/display path.
 *
 * The test deliberately embeds a known MPEG-2 elementary stream in the ELF.
 * No PS2 networking, RFB, audio, USB filesystem, or mux code participates.
 *
 * Visible states:
 *   blue  = graphics initialized and decoder setup is about to begin;
 *   video = libmpeg/IPU is decoding and the GS is presenting pictures;
 *   green = at least EXP3_MIN_SUCCESS_PICTURES pictures decoded;
 *   red   = decoding ended before the success threshold.
 *
 * The MPEG decode/display structure is derived from the PS2SDK libmpeg sample:
 *   ee/mpeg/samples/mpeg.c
 * Copyright (c) 2006-2007 Eugene Plotnikov.
 * Licensed under Academic Free License version 2.0.
 */

#include "libmpeg.h"

#include <dma.h>
#include <dma_tags.h>
#include <draw.h>
#include <gif_tags.h>
#include <graph.h>
#include <gs_gp.h>
#include <gs_psm.h>
#include <kernel.h>
#include <malloc.h>
#include <packet.h>
#include <stdio.h>
#include <string.h>

#define EXP3_DMA_BLOCK_BYTES       2048u
#define EXP3_MIN_SUCCESS_PICTURES   550u

typedef struct exp3_mpeg_display {
    MPEGSequenceInfo *sequence;
    void *picture_data;
    packet_t *transfer_packet;
    packet_t *draw_packet;
    int texture_address;
} exp3_mpeg_display_t;

/*
 * These symbols are produced by:
 *
 *   mips64r5900el-ps2-elf-ld -r -b binary test.bin
 *
 * The input filename is intentionally exactly "test.bin" so the linker
 * symbol names remain deterministic.
 */
extern const unsigned char _binary_test_bin_start[];
extern const unsigned char _binary_test_bin_end[];

static const unsigned char *g_mpeg_data;
static size_t g_mpeg_size;
static size_t g_mpeg_offset;

/*
 * libmpeg ultimately feeds the IPU through DMA. Using an explicitly aligned
 * staging block avoids relying on the alignment of the linked binary section
 * and also gives the final partial input block safe zero padding.
 */
static unsigned char g_dma_block[EXP3_DMA_BLOCK_BYTES]
    __attribute__((aligned(64)));

static int exp3_supply_mpeg_dma(void *user_data);
static void *exp3_initialize_sequence(
    void *user_data,
    MPEGSequenceInfo *sequence);

static void exp3_show_solid(
    packet_t *packet,
    framebuffer_t *frame,
    zbuffer_t *z,
    int red,
    int green,
    int blue)
{
    qword_t *q;

    dma_wait_fast();

    q = packet->data;
    q = draw_setup_environment(q, 0, frame, z);
    q = draw_clear(
        q,
        0,
        0,
        0,
        640.0f,
        512.0f,
        red,
        green,
        blue);

    dma_channel_send_normal(
        DMA_CHANNEL_GIF,
        packet->data,
        q - packet->data,
        0,
        0);

    graph_wait_vsync();
    graph_wait_vsync();
}

int main(void)
{
    packet_t *packet;
    framebuffer_t frame;
    zbuffer_t z;
    exp3_mpeg_display_t display = {0};
    s64 picture_pts;
    s64 current_pts;
    unsigned int decoded_pictures = 0;

    g_mpeg_data = _binary_test_bin_start;
    g_mpeg_size = (size_t)(
        _binary_test_bin_end
        - _binary_test_bin_start);
    g_mpeg_offset = 0;

    printf(
        "EXP3_MPEG_SMOKE_START bytes=%u\n",
        (unsigned int)g_mpeg_size);

    frame.width = 640;
    frame.height = 512;
    frame.mask = 0;
    frame.psm = GS_PSM_32;
    frame.address = graph_vram_allocate(
        frame.width,
        frame.height,
        frame.psm,
        GRAPH_ALIGN_PAGE);

    z.enable = 0;
    z.mask = 0;
    z.method = 0;
    z.zsm = 0;
    z.address = 0;

    packet = packet_init(100, PACKET_NORMAL);

    /*
     * Match the official libmpeg sample's DMA ownership.
     */
    dma_channel_initialize(
        DMA_CHANNEL_toIPU,
        NULL,
        0);

    dma_channel_initialize(
        DMA_CHANNEL_GIF,
        NULL,
        0);

    dma_channel_fast_waits(
        DMA_CHANNEL_GIF);

    /*
     * graph_initialize() automatically selects the console's default
     * NTSC/PAL region mode.
     */
    graph_initialize(
        0,
        640,
        512,
        GS_PSM_32,
        0,
        0);

    display.texture_address =
        graph_vram_allocate(
            0,
            0,
            GS_PSM_32,
            GRAPH_ALIGN_BLOCK);

    /*
     * Blue means the ELF reached graphics + DMA initialization.
     */
    exp3_show_solid(
        packet,
        &frame,
        &z,
        0,
        0,
        96);

    MPEG_Initialize(
        exp3_supply_mpeg_dma,
        NULL,
        exp3_initialize_sequence,
        &display,
        &current_pts);

    while (1) {
        if (!MPEG_Picture(
                display.picture_data,
                &picture_pts)) {
            break;
        }

        decoded_pictures++;

        dma_wait_fast();

        dma_channel_send_chain(
            DMA_CHANNEL_GIF,
            display.transfer_packet->data,
            display.transfer_packet->qwc,
            0,
            0);

        /*
         * The official sample waits two field vsyncs per picture.
         * On an NTSC console this gives approximately 29.97 pictures/sec.
         */
        graph_wait_vsync();
        graph_wait_vsync();

        dma_channel_send_normal(
            DMA_CHANNEL_GIF,
            display.draw_packet->data,
            display.draw_packet->qwc,
            0,
            0);
    }

    MPEG_Destroy();

    printf(
        "EXP3_MPEG_SMOKE_END decoded_pictures=%u "
        "input_bytes=%u consumed_bytes=%u\n",
        decoded_pictures,
        (unsigned int)g_mpeg_size,
        (unsigned int)g_mpeg_offset);

    if (decoded_pictures >=
        EXP3_MIN_SUCCESS_PICTURES) {
        /*
         * Green is the hardware PASS state.
         */
        exp3_show_solid(
            packet,
            &frame,
            &z,
            0,
            96,
            0);
    } else {
        /*
         * Red means the decoder ended too early.
         */
        exp3_show_solid(
            packet,
            &frame,
            &z,
            128,
            0,
            0);
    }

    SleepThread();
    return 0;
}

static int exp3_supply_mpeg_dma(void *user_data)
{
    size_t remaining;
    size_t payload_bytes;

    (void)user_data;

    if (g_mpeg_offset >= g_mpeg_size) {
        return 0;
    }

    remaining =
        g_mpeg_size - g_mpeg_offset;

    payload_bytes =
        remaining > EXP3_DMA_BLOCK_BYTES
            ? EXP3_DMA_BLOCK_BYTES
            : remaining;

    memset(
        g_dma_block,
        0,
        sizeof(g_dma_block));

    memcpy(
        g_dma_block,
        g_mpeg_data + g_mpeg_offset,
        payload_bytes);

    /*
     * The IPU DMA always receives a complete 2048-byte aligned staging
     * block. Only payload_bytes are counted as source consumption.
     */
    SyncDCache(
        g_dma_block,
        g_dma_block
            + sizeof(g_dma_block));

    dma_channel_wait(
        DMA_CHANNEL_toIPU,
        0);

    dma_channel_send_normal(
        DMA_CHANNEL_toIPU,
        g_dma_block,
        EXP3_DMA_BLOCK_BYTES >> 4,
        0,
        0);

    g_mpeg_offset += payload_bytes;

    return 1;
}

static void *exp3_initialize_sequence(
    void *user_data,
    MPEGSequenceInfo *sequence)
{
    exp3_mpeg_display_t *display =
        (exp3_mpeg_display_t *)user_data;

    int data_size =
        sequence->m_Width
        * sequence->m_Height
        * 4;

    char *picture_data =
        (char *)memalign(
            64,
            data_size);

    int macroblock_width =
        sequence->m_Width >> 4;

    int macroblock_height =
        sequence->m_Height >> 4;

    int texture_buffer_width =
        (sequence->m_Width + 63) >> 6;

    int texture_width =
        draw_log2(sequence->m_Width);

    int texture_height =
        draw_log2(sequence->m_Height);

    int x;
    int y;
    char *image;
    qword_t *q;

    printf(
        "EXP3_MPEG_SEQUENCE width=%d height=%d "
        "ms_per_frame=%d profile=%d level=%d chroma=%d\n",
        sequence->m_Width,
        sequence->m_Height,
        sequence->m_MSPerFrame,
        sequence->m_Profile,
        sequence->m_Level,
        sequence->m_ChromaFmt);

    display->texture_address >>= 6;

    display->picture_data =
        image = picture_data;

    display->sequence = sequence;

    SyncDCache(
        picture_data,
        picture_data + data_size);

    display->transfer_packet =
        packet_init(
            (
                10
                + 12
                * macroblock_width
                * macroblock_height
            ) >> 1,
            PACKET_NORMAL);

    q = display->transfer_packet->data;

    DMATAG_CNT(
        q,
        3,
        0,
        0,
        0);
    q++;

    PACK_GIFTAG(
        q,
        GIF_SET_TAG(
            2,
            0,
            0,
            0,
            0,
            1),
        GIF_REG_AD);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_TRXREG(
            16,
            16),
        GS_REG_TRXREG);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_BITBLTBUF(
            0,
            0,
            0,
            display->texture_address,
            texture_buffer_width,
            GS_PSM_32),
        GS_REG_BITBLTBUF);
    q++;

    for (
        y = 0;
        y < sequence->m_Height;
        y += 16) {

        for (
            x = 0;
            x < sequence->m_Width;
            x += 16,
            image += 1024) {

            DMATAG_CNT(
                q,
                4,
                0,
                0,
                0);
            q++;

            PACK_GIFTAG(
                q,
                GIF_SET_TAG(
                    2,
                    0,
                    0,
                    0,
                    0,
                    1),
                GIF_REG_AD);
            q++;

            PACK_GIFTAG(
                q,
                GS_SET_TRXPOS(
                    0,
                    0,
                    x,
                    y,
                    0),
                GS_REG_TRXPOS);
            q++;

            PACK_GIFTAG(
                q,
                GS_SET_TRXDIR(0),
                GS_REG_TRXDIR);
            q++;

            PACK_GIFTAG(
                q,
                GIF_SET_TAG(
                    64,
                    1,
                    0,
                    0,
                    2,
                    0),
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

    display->transfer_packet->qwc =
        q - display->transfer_packet->data;

    display->draw_packet =
        packet_init(
            7,
            PACKET_NORMAL);

    q = display->draw_packet->data;

    PACK_GIFTAG(
        q,
        GIF_SET_TAG(
            6,
            1,
            0,
            0,
            0,
            1),
        GIF_REG_AD);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_TEX0(
            display->texture_address,
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
        GS_SET_PRIM(
            6,
            0,
            1,
            0,
            0,
            0,
            1,
            0,
            0),
        GS_REG_PRIM);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_UV(0, 0),
        GS_REG_UV);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_XYZ(
            2048 << 4,
            2048 << 4,
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
            (640 << 4)
                + (2048 << 4),
            (512 << 4)
                + (2048 << 4),
            0),
        GS_REG_XYZ2);
    q++;

    display->draw_packet->qwc =
        q - display->draw_packet->data;

    return picture_data;
}
