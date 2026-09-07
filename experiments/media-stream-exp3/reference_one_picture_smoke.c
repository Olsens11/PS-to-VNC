/*
 * File synopsis:
 * EXP3 reference-derived one-picture MPEG-2 hardware smoke test.
 *
 * Goal:
 * Prove one complete MPEG-2 picture can be decoded by the PS2 IPU/libmpeg
 * path before attempting decoded-picture presentation, continuous playback,
 * networking, audio synchronization, or integration with PS2VNC.
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
 *
 *   RED:
 *       MPEG_Picture returned zero instead of a picture.
 *
 *   ORANGE FOREVER:
 *       first-picture decoding did not return.
 *
 * Important:
 * This test does NOT upload the decoded picture to GS. That is the next
 * increment after decoder success is independently proven.
 */

#include "libmpeg.h"

#include <dma.h>
#include <draw.h>
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
