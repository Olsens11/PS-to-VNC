/*
 * File synopsis:
 * EXP3 one-picture MPEG-2 decode + GS presentation smoke test.
 *
 * Goal:
 * Starting from the hardware-qualified SMS first-picture decoder result,
 * transfer that single decoded macroblock-ordered RGBA32 picture into GS
 * texture memory and draw it once.
 *
 * Continuous playback, networking, audio synchronization, and PS2VNC
 * integration remain intentionally out of scope.
 *
 * Evidence used to derive this test:
 *
 *   1. PS2SDK libmpeg internals establish the actual MPEG_Initialize and
 *      MPEG_Picture contracts.
 *
 *   2. SMS, Eugene Plotnikov's mature PS2 media player, establishes the
 *      production-style pattern around that decoder:
 *        - explicitly establish clean IPU/DMAC ownership;
 *        - feed TO_IPU through ordinary aligned DMA;
 *        - use physical DMA source addresses;
 *        - avoid the PS2SDK dma_wait_fast helper in the MPEG feed path;
 *        - use deliberately managed output buffers.
 *
 *   3. Public retail-game reconstruction evidence independently shows Sony
 *      MPEG paths using 16-byte-rounded bitstream input and uncached aliases
 *      for decoded-picture destinations.
 *
 * This is an independently written experiment based on those behavioral
 * requirements. It is not copied retail-game source.
 *
 * Visible stage contract:
 *
 *   CYAN:
 *       graphics initialized and experiment reached the known-good chassis.
 *
 *   YELLOW:
 *       IPU/DMAC quiesce/reset completed.
 *
 *   ORANGE:
 *       MPEG_Initialize returned successfully.
 *       The test waits here briefly and then requests exactly one picture.
 *
 *   GREEN:
 *       MPEG_Picture returned nonzero: one picture decoded successfully.
 *       The test pauses here before attempting GS upload.
 *
 *   BLUE:
 *       the decoded 16x16 macroblocks were transferred to GS texture memory.
 *       The test pauses here immediately before drawing the texture.
 *
 *   PICTURE:
 *       successful final state: the decoded first MPEG picture itself is
 *       displayed and the test sleeps.
 *
 *   RED:
 *       MPEG_Picture returned zero instead of a picture.
 *
 *   ORANGE FOREVER:
 *       first-picture decoding did not return.
 *
 * Important:
 * This is deliberately a one-picture presentation test. It does not call
 * MPEG_Picture a second time.
 *
 * PS2SDK's old MPEG sample uses dma_wait_fast() before GIF presentation.
 * That helper is hardware-proven to hang in this EXP3 environment, so this
 * test uses ordinary DMA_CHANNEL_GIF waits instead.
 */

#include "libmpeg.h"

#include <dma.h>
#include <dma_tags.h>
#include <draw.h>
#include <gif_tags.h>
#include <gs_gp.h>
#include <ee_regs.h>
#include <graph.h>
#include <gs_psm.h>
#include <kernel.h>
#include <malloc.h>
#include <packet.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <tamtypes.h>

#define EXP3_SCREEN_WIDTH   640
#define EXP3_SCREEN_HEIGHT  512

#define EXP3_VIDEO_MAX_WIDTH   704
#define EXP3_VIDEO_MAX_HEIGHT  480

#define EXP3_PICTURE_BYTES \
    (EXP3_VIDEO_MAX_WIDTH * EXP3_VIDEO_MAX_HEIGHT * 4)

#define EXP3_FEED_BYTES 2048

#define EXP3_DMAC_START 0x00000100u

#define EXP3_IPU_CTRL_BUSY 0x80000000u
#define EXP3_IPU_CTRL_RST  0x40000000u
#define EXP3_IPU_CMD_BCLR  0x00000000u

typedef struct Exp3DecoderState
{
    const unsigned char *stream_begin;
    const unsigned char *stream_cursor;
    const unsigned char *stream_end;

    unsigned int feed_calls;
    unsigned int payload_bytes_submitted;
    unsigned int dma_bytes_submitted;

    unsigned int sequence_callbacks;
    unsigned int sequence_width;
    unsigned int sequence_height;

    MPEGSequenceInfo *sequence_info;

    /*
     * One-picture GS presentation state.
     *
     * texture_vram_address is the byte-address returned by the graph VRAM
     * allocator. The GIF BITBLT packet converts it to the GS 64-byte unit.
     */
    int texture_vram_address;
    packet_t *transfer_packet;
    packet_t *draw_packet;

    s64 current_stream_pts;
} Exp3DecoderState;

extern const unsigned char _binary_test_bin_start[];
extern const unsigned char _binary_test_bin_end[];

/*
 * The decoder output buffer is deliberately BSS rather than embedded file
 * content. The first smoke stream is known to be 704x480 or smaller.
 *
 * IMPORTANT:
 * Return the ordinary aligned EE pointer to PS2SDK libmpeg. This matches both
 * the PS2SDK libmpeg sample and SMS. The previous experiment incorrectly
 * imported the uncached-output convention from Sony's different sceMpeg API.
 */
static unsigned char s_picture_buffer[EXP3_PICTURE_BYTES]
    __attribute__((aligned(64)));

/*
 * PS2SDK's libmpeg sample feeds 2048-byte normal TO_IPU DMA blocks.
 *
 * Our embedded source section is not relied upon for DMA alignment. Instead,
 * each block is copied into this known 64-byte-aligned staging buffer and is
 * submitted through PS2SDK's own dma_channel_send_normal() helper.
 *
 * The final partial block is rounded only to the required 16-byte DMA
 * boundary and zero-padded.
 */
static unsigned char s_feed_buffer[EXP3_FEED_BYTES]
    __attribute__((aligned(64)));

static void exp3_show_color(
    packet_t *packet,
    framebuffer_t *frame,
    zbuffer_t *z,
    int red,
    int green,
    int blue)
{
    qword_t *q;

    /*
     * Deliberately use the ordinary per-channel wait.
     * dma_wait_fast() is prohibited in this experiment.
     */
    dma_channel_wait(
        DMA_CHANNEL_GIF,
        0);

    q = packet->data;

    q = draw_setup_environment(
        q,
        0,
        frame,
        z);

    q = draw_clear(
        q,
        0,
        0,
        0,
        (float)EXP3_SCREEN_WIDTH,
        (float)EXP3_SCREEN_HEIGHT,
        red,
        green,
        blue);

    dma_channel_send_normal(
        DMA_CHANNEL_GIF,
        packet->data,
        q - packet->data,
        0,
        0);

    dma_channel_wait(
        DMA_CHANNEL_GIF,
        0);

    graph_wait_vsync();
    graph_wait_vsync();
}

static void exp3_hold_vsyncs(int count)
{
    int index;

    for (index = 0; index < count; ++index) {
        graph_wait_vsync();
    }
}

/*
 * Establish an explicitly quiescent IPU DMA state before libmpeg takes
 * ownership.
 *
 * This follows the behavior demonstrated by mature PS2 media code:
 * stop TO/FROM-IPU transfers, clear their transfer registers, reset IPU,
 * wait for reset completion, clear the bitstream state, and wait again.
 *
 * MPEG_Initialize will then perform its own library reset/setup on top of a
 * known idle hardware state.
 */
static void exp3_reference_ipu_reset(void)
{
    int interrupt_was_enabled;
    u32 dmac_enable_state;
    u32 old_ipu_control;

    interrupt_was_enabled = DIntr();

    dmac_enable_state = *R_EE_D_ENABLER;

    /*
     * Temporarily disable DMA execution while the IPU channels are cleared.
     */
    *R_EE_D_ENABLEW =
        dmac_enable_state |
        0x00010000u;

    EE_SYNCL();

    *R_EE_D3_CHCR &= ~EXP3_DMAC_START;
    *R_EE_D4_CHCR &= ~EXP3_DMAC_START;

    *R_EE_D3_MADR = 0;
    *R_EE_D4_MADR = 0;

    *R_EE_D3_QWC = 0;
    *R_EE_D4_QWC = 0;

    EE_SYNCL();

    *R_EE_D_ENABLEW =
        dmac_enable_state;

    if (interrupt_was_enabled) {
        EIntr();
    }

    old_ipu_control =
        *R_EE_IPU_CTRL &
        ~(EXP3_IPU_CTRL_BUSY | EXP3_IPU_CTRL_RST);

    *R_EE_IPU_CTRL =
        EXP3_IPU_CTRL_RST;

    while (
        *R_EE_IPU_CTRL &
        EXP3_IPU_CTRL_BUSY
    ) {
        /* wait for IPU reset */
    }

    *R_EE_IPU_CMD =
        EXP3_IPU_CMD_BCLR;

    while (
        *R_EE_IPU_CTRL &
        EXP3_IPU_CTRL_BUSY
    ) {
        /* wait for bitstream clear */
    }

    /*
     * Restore the non-busy, non-reset control bits that existed before this
     * explicit quiesce. MPEG_Initialize owns subsequent decoder-mode setup.
     */
    *R_EE_IPU_CTRL =
        old_ipu_control;
}

/*
 * libmpeg data callback.
 *
 * This intentionally follows the PS2SDK libmpeg sample's transport contract:
 *
 *   dma_channel_wait(DMA_CHANNEL_toIPU, 0)
 *   dma_channel_send_normal(DMA_CHANNEL_toIPU, ..., qwc, 0, 0)
 *
 * PS2SDK's helper performs the cache synchronization, clears channel status,
 * programs MADR/QWC, and starts the normal DMA transfer.
 *
 * The sample uses 2048-byte blocks. We retain that exact normal block size.
 * Only the final short block is rounded to a 16-byte DMA boundary.
 */
static int exp3_feed_ipu(void *user_data)
{
    Exp3DecoderState *state =
        (Exp3DecoderState *)user_data;

    unsigned int remaining;
    unsigned int payload_bytes;
    unsigned int dma_bytes;
    unsigned int dma_qwc;

    if (
        state->stream_cursor >=
        state->stream_end
    ) {
        return 0;
    }

    remaining =
        (unsigned int)(
            state->stream_end -
            state->stream_cursor);

    payload_bytes = remaining;

    if (
        payload_bytes >
        EXP3_FEED_BYTES
    ) {
        payload_bytes =
            EXP3_FEED_BYTES;
    }

    dma_bytes =
        (payload_bytes + 15u) &
        ~15u;

    if (
        dma_bytes >
        EXP3_FEED_BYTES
    ) {
        return 0;
    }

    memcpy(
        s_feed_buffer,
        state->stream_cursor,
        payload_bytes);

    if (
        dma_bytes >
        payload_bytes
    ) {
        memset(
            s_feed_buffer + payload_bytes,
            0,
            dma_bytes - payload_bytes);
    }

    /*
     * Exact PS2SDK libmpeg sample ownership model:
     * wait the TO_IPU channel and submit a normal DMA transfer.
     */
    if (
        dma_channel_wait(
            DMA_CHANNEL_toIPU,
            0) != 0
    ) {
        return 0;
    }

    dma_qwc =
        dma_bytes >> 4;

    dma_channel_send_normal(
        DMA_CHANNEL_toIPU,
        s_feed_buffer,
        dma_qwc,
        0,
        0);

    state->stream_cursor +=
        payload_bytes;

    state->feed_calls += 1;

    state->payload_bytes_submitted +=
        payload_bytes;

    state->dma_bytes_submitted +=
        dma_bytes;

    return 1;
}

/*
 * Called by libmpeg after it has parsed the sequence header.
 *
 * For this first proof we use one fixed, 64-byte-aligned BSS area sized for
 * the known 704x480 smoke stream. The ordinary aligned pointer is returned,
 * exactly as expected by the PS2SDK libmpeg sample and SMS.
 */
static void *exp3_sequence_init(
    void *user_data,
    MPEGSequenceInfo *sequence)
{
    Exp3DecoderState *state =
        (Exp3DecoderState *)user_data;

    unsigned int required_bytes;

    int mb_width;
    int mb_height;
    int texture_buffer_width;
    int texture_width_log2;
    int texture_height_log2;
    int x;
    int y;
    int texture_address_64;

    unsigned char *image_ptr;
    qword_t *q;

    state->sequence_callbacks += 1;
    state->sequence_info =
        sequence;

    state->sequence_width =
        sequence->m_Width;

    state->sequence_height =
        sequence->m_Height;

    required_bytes =
        sequence->m_Width *
        sequence->m_Height *
        4u;

    printf(
        "EXP3_SEQUENCE width=%u height=%u bytes=%u\n",
        state->sequence_width,
        state->sequence_height,
        required_bytes);

    if (
        sequence->m_Width >
            EXP3_VIDEO_MAX_WIDTH ||
        sequence->m_Height >
            EXP3_VIDEO_MAX_HEIGHT ||
        required_bytes >
            sizeof(s_picture_buffer)
    ) {
        printf(
            "EXP3_SEQUENCE_REJECT dimensions exceed smoke buffer\n");

        return NULL;
    }

    /*
     * Flush any cached initialization state before libmpeg/IPU writes decoded
     * output. The actual pointer handed to libmpeg remains the normal aligned
     * EE address; libmpeg owns the required DMA operations internally.
     */
    SyncDCache(
        s_picture_buffer,
        s_picture_buffer +
            required_bytes);

    /*
     * Build the exact macroblock-oriented GS transfer shape demonstrated by
     * the PS2SDK MPEG sample.
     *
     * libmpeg produces RGBA32 as consecutive 16x16 macroblocks. Each
     * macroblock is therefore 16 * 16 * 4 = 1024 bytes = 64 qwords.
     */
    mb_width =
        sequence->m_Width >> 4;

    mb_height =
        sequence->m_Height >> 4;

    texture_buffer_width =
        (sequence->m_Width + 63) >> 6;

    texture_width_log2 =
        draw_log2(sequence->m_Width);

    texture_height_log2 =
        draw_log2(sequence->m_Height);

    texture_address_64 =
        state->texture_vram_address >> 6;

    state->transfer_packet =
        packet_init(
            (10 + 12 * mb_width * mb_height) >> 1,
            PACKET_NORMAL);

    state->draw_packet =
        packet_init(
            7,
            PACKET_NORMAL);

    if (
        state->transfer_packet == NULL ||
        state->draw_packet == NULL
    ) {
        printf(
            "EXP3_DISPLAY_PACKET_ALLOCATION=FAIL\n");

        return NULL;
    }

    image_ptr =
        s_picture_buffer;

    q =
        state->transfer_packet->data;

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
            texture_address_64,
            texture_buffer_width,
            GS_PSM_32),
        GS_REG_BITBLTBUF);
    q++;

    for (
        y = 0;
        y < sequence->m_Height;
        y += 16
    ) {
        for (
            x = 0;
            x < sequence->m_Width;
            x += 16,
            image_ptr += 1024
        ) {
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
                (unsigned int)image_ptr,
                0,
                0,
                0);
            q++;
        }
    }

    state->transfer_packet->qwc =
        q -
        state->transfer_packet->data;

    q =
        state->draw_packet->data;

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
            texture_address_64,
            texture_buffer_width,
            GS_PSM_32,
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
        GS_SET_UV(
            0,
            0),
        GS_REG_UV);
    q++;

    PACK_GIFTAG(
        q,
        GS_SET_XYZ(
            (2048 << 4),
            (2048 << 4),
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
            (EXP3_SCREEN_WIDTH << 4) +
                (2048 << 4),
            (EXP3_SCREEN_HEIGHT << 4) +
                (2048 << 4),
            0),
        GS_REG_XYZ2);
    q++;

    state->draw_packet->qwc =
        q -
        state->draw_packet->data;

    printf(
        "EXP3_DISPLAY_PACKETS_READY "
        "mbw=%d mbh=%d "
        "transfer_qwc=%u draw_qwc=%u "
        "texture_vram=%d\n",
        mb_width,
        mb_height,
        state->transfer_packet->qwc,
        state->draw_packet->qwc,
        state->texture_vram_address);

    return s_picture_buffer;
}

int main(void)
{
    framebuffer_t frame;
    zbuffer_t z;
    packet_t *packet;

    Exp3DecoderState decoder;

    int picture_result;
    s64 picture_pts;

    memset(
        &decoder,
        0,
        sizeof(decoder));

    decoder.stream_begin =
        _binary_test_bin_start;

    decoder.stream_cursor =
        _binary_test_bin_start;

    decoder.stream_end =
        _binary_test_bin_end;

    printf(
        "EXP3_REFERENCE_ONE_PICTURE_START "
        "stream_bytes=%u\n",
        (unsigned int)(
            decoder.stream_end -
            decoder.stream_begin));

    frame.width =
        EXP3_SCREEN_WIDTH;

    frame.height =
        EXP3_SCREEN_HEIGHT;

    frame.mask = 0;
    frame.psm =
        GS_PSM_32;

    frame.address =
        graph_vram_allocate(
            frame.width,
            frame.height,
            frame.psm,
            GRAPH_ALIGN_PAGE);

    z.enable = 0;
    z.mask = 0;
    z.method = 0;
    z.zsm = 0;
    z.address = 0;

    packet =
        packet_init(
            100,
            PACKET_NORMAL);

    /*
     * PS2SDK libmpeg sample initializes TO_IPU before decoding.
     */
    dma_channel_initialize(
        DMA_CHANNEL_toIPU,
        NULL,
        0);

    dma_channel_initialize(
        DMA_CHANNEL_GIF,
        NULL,
        0);

    /*
     * We intentionally do NOT call dma_channel_fast_waits().
     */

    graph_initialize(
        0,
        EXP3_SCREEN_WIDTH,
        EXP3_SCREEN_HEIGHT,
        GS_PSM_32,
        0,
        0);

    /*
     * Reserve the next free GS VRAM block as the texture base.
     *
     * This follows the old PS2SDK MPEG sample. No later VRAM allocations are
     * performed by this smoke test.
     */
    decoder.texture_vram_address =
        graph_vram_allocate(
            0,
            0,
            GS_PSM_32,
            GRAPH_ALIGN_BLOCK);

    printf(
        "EXP3_TEXTURE_VRAM_ADDRESS=%d\n",
        decoder.texture_vram_address);

    /*
     * CYAN:
     * proven graph/display chassis reached.
     */
    exp3_show_color(
        packet,
        &frame,
        &z,
        0,
        160,
        160);

    exp3_hold_vsyncs(30);

    printf(
        "EXP3_STAGE=BEFORE_REFERENCE_IPU_RESET\n");

    exp3_reference_ipu_reset();

    printf(
        "EXP3_STAGE=REFERENCE_IPU_RESET_RETURNED\n");

    /*
     * YELLOW:
     * explicit IPU/DMAC quiesce/reset returned.
     */
    exp3_show_color(
        packet,
        &frame,
        &z,
        160,
        160,
        0);

    exp3_hold_vsyncs(30);

    printf(
        "EXP3_STAGE=BEFORE_MPEG_INITIALIZE\n");

    MPEG_Initialize(
        exp3_feed_ipu,
        &decoder,
        exp3_sequence_init,
        &decoder,
        &decoder.current_stream_pts);

    printf(
        "EXP3_STAGE=MPEG_INITIALIZE_RETURNED\n");

    /*
     * ORANGE:
     * MPEG_Initialize returned.
     */
    exp3_show_color(
        packet,
        &frame,
        &z,
        192,
        64,
        0);

    /*
     * Give the operator enough time to identify orange before the first
     * decode request starts.
     */
    exp3_hold_vsyncs(60);

    printf(
        "EXP3_STAGE=BEFORE_FIRST_MPEG_PICTURE\n");

    picture_pts = 0;

    /*
     * On the first call libmpeg parses the sequence, invokes
     * exp3_sequence_init(), substitutes that callback's returned picture
     * buffer, and then performs the first picture decode.
     */
    picture_result =
        MPEG_Picture(
            NULL,
            &picture_pts);

    printf(
        "EXP3_FIRST_PICTURE_RETURNED "
        "result=%d "
        "pts=%lld "
        "feed_calls=%u "
        "payload_bytes=%u "
        "dma_bytes=%u "
        "sequence_callbacks=%u "
        "width=%u "
        "height=%u\n",
        picture_result,
        picture_pts,
        decoder.feed_calls,
        decoder.payload_bytes_submitted,
        decoder.dma_bytes_submitted,
        decoder.sequence_callbacks,
        decoder.sequence_width,
        decoder.sequence_height);

    if (
        picture_result != 0
    ) {
        /*
         * GREEN:
         * one complete MPEG picture was decoded.
         *
         * Preserve the already-qualified success marker before beginning the
         * new GS-presentation increment.
         */
        exp3_show_color(
            packet,
            &frame,
            &z,
            0,
            192,
            0);

        printf(
            "EXP3_REFERENCE_ONE_PICTURE=PASS\n");

        exp3_hold_vsyncs(60);

        if (
            decoder.transfer_packet == NULL ||
            decoder.draw_packet == NULL
        ) {
            /*
             * MAGENTA:
             * sequence callback returned but failed to create display
             * packets.
             */
            exp3_show_color(
                packet,
                &frame,
                &z,
                192,
                0,
                192);

            printf(
                "EXP3_FIRST_PICTURE_DISPLAY="
                "PACKET_SETUP_FAIL\n");

            SleepThread();
            return 2;
        }

        printf(
            "EXP3_STAGE=BEFORE_GS_TEXTURE_UPLOAD\n");

        /*
         * Do NOT use dma_wait_fast().
         *
         * The decoded picture is no longer owned by active IPU decode at
         * this point. Transfer the 16x16 macroblocks into GS texture memory
         * with the source-chain packet built by exp3_sequence_init().
         */
        dma_channel_wait(
            DMA_CHANNEL_GIF,
            0);

        dma_channel_send_chain(
            DMA_CHANNEL_GIF,
            decoder.transfer_packet->data,
            decoder.transfer_packet->qwc,
            0,
            0);

        dma_channel_wait(
            DMA_CHANNEL_GIF,
            0);

        printf(
            "EXP3_STAGE=GS_TEXTURE_UPLOAD_RETURNED\n");

        /*
         * BLUE:
         * macroblock upload completed and ordinary GIF wait returned.
         */
        exp3_show_color(
            packet,
            &frame,
            &z,
            0,
            0,
            192);

        exp3_hold_vsyncs(30);

        printf(
            "EXP3_STAGE=BEFORE_GS_TEXTURE_DRAW\n");

        dma_channel_wait(
            DMA_CHANNEL_GIF,
            0);

        dma_channel_send_normal(
            DMA_CHANNEL_GIF,
            decoder.draw_packet->data,
            decoder.draw_packet->qwc,
            0,
            0);

        dma_channel_wait(
            DMA_CHANNEL_GIF,
            0);

        graph_wait_vsync();
        graph_wait_vsync();

        printf(
            "EXP3_FIRST_PICTURE_DISPLAY=PASS\n");

        /*
         * Successful terminal state is the decoded picture itself.
         */
        SleepThread();

        return 0;
    }

    /*
     * RED:
     * decoder returned without producing a picture.
     */
    exp3_show_color(
        packet,
        &frame,
        &z,
        192,
        0,
        0);

    printf(
        "EXP3_REFERENCE_ONE_PICTURE=NO_PICTURE "
        "eof=%d\n",
        decoder.sequence_info
            ? decoder.sequence_info->m_fEOF
            : -1);

    SleepThread();

    return 1;
}
