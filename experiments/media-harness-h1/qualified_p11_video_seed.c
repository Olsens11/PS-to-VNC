/*
 * File synopsis:
 * EXP3 O3 finite live X11 -> ffmpeg -> Ethernet -> SMS MPEG hardware test.
 *
 * This file is mechanically derived from the exact hardware-qualified O2
 * harness. The MPEG decoder, TO_IPU feed, picture buffer, GIF macroblock
 * upload and GS draw path are unchanged. P6 alone replaces the relative
 * two-VSYNC presentation delay with an absolute 30000/1001-fps clock.
 *
 * O3 changes only EOF/source ownership: DATA/channel 4 arrives while ffmpeg
 * captures X11, and a terminal producer-metadata frame supplies the exact
 * live byte count, CRC32, and coded-picture count. A temporarily empty queue
 * remains starvation rather than EOF until that terminal frame arrives.
 */

#include "libmpeg.h"
#include "libmpeg_internal.h"
#include "network_live_stream_runtime.h"
#include "ps2_network.h"
#include "ps2_system.h"
#include "transport_protocol.h"

#include <arpa/inet.h>
#include <dma.h>
#include <dma_tags.h>
#include <draw.h>
#include <gif_tags.h>
#include <gs_gp.h>
#include <ee_regs.h>
#include <graph.h>
#include <gs_psm.h>
#include <kernel.h>
#include <timer.h>
#include <delaythread.h>
#include <malloc.h>
#include <packet.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <tamtypes.h>

#define EXP3_SCREEN_WIDTH   640
#define EXP3_SCREEN_HEIGHT  512

#define EXP3_VIDEO_MAX_WIDTH   704
#define EXP3_VIDEO_MAX_HEIGHT  480

#define EXP3_PICTURE_BYTES \
    (EXP3_VIDEO_MAX_WIDTH * EXP3_VIDEO_MAX_HEIGHT * 2)

#define EXP3_FEED_BYTES 2048

#define EXP3_DMAC_START 0x00000100u

#define EXP3_IPU_CTRL_BUSY 0x80000000u
#define EXP3_IPU_CTRL_RST  0x40000000u
#define EXP3_IPU_CMD_BCLR  0x00000000u

typedef struct Exp3DecoderState
{
    Exp3LiveMpegStreamRuntime *stream_runtime;

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

    /* P6 absolute presentation-clock diagnostics. */
    u64 p6_presentation_epoch_tick;
    unsigned int p6_clock_started;
    unsigned int p6_vsync_waits;
    unsigned int p6_deadline_misses;
    u64 p6_max_deadline_late_ticks;

    /* P7A compressed-input prefill diagnostics. */
    unsigned int p7a_prefill_target;
    unsigned int p7a_prefill_observed;

    /* P9 instrumentation-only stage timing. */
    unsigned int p9_decode_samples;
    u64 p9_decode_ticks_total;
    u64 p9_decode_ticks_max;

    unsigned int p9_upload_samples;
    u64 p9_upload_ticks_total;
    u64 p9_upload_ticks_max;

    unsigned int p9_draw_samples;
    u64 p9_draw_ticks_total;
    u64 p9_draw_ticks_max;
} Exp3DecoderState;

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
 * The network-received source allocation is not relied upon for DMA
 * alignment. Instead,
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
    Exp3DecoderState *state = (Exp3DecoderState *)user_data;
    size_t payload_size = 0;
    unsigned int payload_bytes;
    unsigned int dma_bytes;
    unsigned int dma_qwc;

    if (state == NULL || state->stream_runtime == NULL)
        return 0;

    if (!exp3_live_mpeg_stream_read(
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
        2u;

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
     * P8 enables SMS _MPEG_Set16(1), so libmpeg produces RGB16 as
     * consecutive 16x16 macroblocks. Each macroblock is therefore
     * 16 * 16 * 2 = 512 bytes = 32 qwords.
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
            GS_PSM_16),
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
            image_ptr += 512
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
                    32,
                    1,
                    0,
                    0,
                    2,
                    0),
                0);
            q++;

            DMATAG_REF(
                q,
                32,
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
            GS_PSM_16,
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


/*
 * Upload the decoder's macroblock-ordered RGBA32 buffer into the GS texture.
 *
 * The source-chain REF tags point directly at s_picture_buffer. The ordinary
 * GIF-channel wait after send_chain() is therefore the ownership boundary:
 * only after this function returns may MPEG_Picture() overwrite that buffer
 * for the next picture.
 */
static void exp3_upload_picture_texture(
    Exp3DecoderState *state)
{
    dma_channel_wait(
        DMA_CHANNEL_GIF,
        0);

    dma_channel_send_chain(
        DMA_CHANNEL_GIF,
        state->transfer_packet->data,
        state->transfer_packet->qwc,
        0,
        0);

    dma_channel_wait(
        DMA_CHANNEL_GIF,
        0);
}

/*
 * Draw the already-uploaded GS texture as the same full-screen textured
 * sprite qualified by the one-picture presentation test.
 *
 * Two VSYNC waits retain the old PS2SDK sample's interlaced-frame cadence.
 */
static void exp3_draw_picture_texture(
    Exp3DecoderState *state)
{
    dma_channel_wait(
        DMA_CHANNEL_GIF,
        0);

    dma_channel_send_normal(
        DMA_CHANNEL_GIF,
        state->draw_packet->data,
        state->draw_packet->qwc,
        0,
        0);

    dma_channel_wait(
        DMA_CHANNEL_GIF,
        0);

    /*
     * P6 absolute-clock diagnostic: presentation waits are performed before
     * draw by exp3_p6_wait_for_picture(). The GIF DMA completion above remains
     * the decoded-buffer ownership boundary.
     */
}


#define EXP3_P7A_PREFILL_BYTES (448u * 1024u)
#define EXP3_P7A_PREFILL_WAIT_US 1000u
#define EXP3_P7A_PREFILL_MAX_LOOPS 10000u

static int exp3_p7a_wait_for_prefill(Exp3DecoderState *state)
{
    Exp3LiveMpegStreamRuntime *runtime;
    unsigned int loops = 0u;

    if (state == NULL || state->stream_runtime == NULL)
        return 0;

    runtime = state->stream_runtime;
    state->p7a_prefill_target = EXP3_P7A_PREFILL_BYTES;
    state->p7a_prefill_observed = 0u;

    if (runtime->queue_capacity < EXP3_P7A_PREFILL_BYTES)
        return 0;

    for (;;) {
        size_t current;
        int receiver_done;
        int error;

        if (WaitSema(runtime->queue_sema_id) < 0)
            return 0;

        current = pstvnc_transport_queue_size(&runtime->queue);
        receiver_done = runtime->receiver_done;
        error = runtime->error;

        if (current > state->p7a_prefill_observed)
            state->p7a_prefill_observed = (unsigned int)current;

        if (SignalSema(runtime->queue_sema_id) < 0)
            return 0;

        if (current >= EXP3_P7A_PREFILL_BYTES)
            return 1;

        if (
            error != EXP3_LIVE_STREAM_ERROR_NONE ||
            receiver_done ||
            loops >= EXP3_P7A_PREFILL_MAX_LOOPS
        ) {
            return 0;
        }

        loops += 1u;

        if (DelayThread(EXP3_P7A_PREFILL_WAIT_US) < 0)
            return 0;
    }
}

#define EXP3_P6_SOURCE_FPS_NUMERATOR   30000u
#define EXP3_P6_SOURCE_FPS_DENOMINATOR 1001u

static u64 exp3_p6_picture_deadline(
    const Exp3DecoderState *state,
    unsigned int picture_index)
{
    u64 picture_offset;

    if (picture_index <= 1u)
        return state->p6_presentation_epoch_tick;

    picture_offset =
        (u64)(picture_index - 1u) *
        (u64)kBUSCLK *
        (u64)EXP3_P6_SOURCE_FPS_DENOMINATOR;

    picture_offset /=
        (u64)EXP3_P6_SOURCE_FPS_NUMERATOR;

    return
        state->p6_presentation_epoch_tick +
        picture_offset;
}

static void exp3_p6_wait_for_picture(
    Exp3DecoderState *state,
    unsigned int picture_index)
{
    u64 deadline;
    u64 now;

    if (!state->p6_clock_started) {
        /*
         * Establish the epoch on a real display boundary. Subsequent picture
         * deadlines remain anchored to this one epoch and never rebase after
         * decode/network jitter.
         */
        graph_wait_vsync();
        state->p6_vsync_waits += 1u;

        state->p6_presentation_epoch_tick =
            GetTimerSystemTime();

        state->p6_clock_started = 1u;
    }

    deadline =
        exp3_p6_picture_deadline(
            state,
            picture_index);

    now =
        GetTimerSystemTime();

    if (picture_index > 1u && now >= deadline) {
        u64 lateness = now - deadline;

        state->p6_deadline_misses += 1u;

        if (lateness > state->p6_max_deadline_late_ticks)
            state->p6_max_deadline_late_ticks = lateness;

        return;
    }

    while (now < deadline) {
        /*
         * Wait on display boundaries rather than busy-spinning. This leaves
         * the receiver thread runnable while still presenting on a GS-safe
         * boundary. The absolute deadline, not the number of waits, owns
         * cadence.
         */
        graph_wait_vsync();
        state->p6_vsync_waits += 1u;

        now =
            GetTimerSystemTime();
    }
}


#define EXP3_P6B_CLOCK_TELEMETRY_IP   "192.168.50.1"
#define EXP3_P6B_CLOCK_TELEMETRY_PORT 5999

/*
 * The experiment makefile retains the product link contract
 * -Wl,--wrap=sendto, but this isolated harness intentionally does not link the
 * diagnostics identity object that normally defines __wrap_sendto(). Calling
 * sendto() here would therefore become an unresolved __wrap_sendto reference.
 * GNU ld's matching __real_sendto alias explicitly names the underlying SDK
 * socket function while preserving the existing linker contract unchanged.
 */
extern __typeof__(sendto) __real_sendto;

static void exp3_p6b_send_clock_telemetry(
    const Exp3DecoderState *state)
{
    struct sockaddr_in address;
    char payload[640];
    int socket_fd;
    int payload_length;
    int sent;

    if (state == NULL)
        return;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_fd < 0) {
        printf("EXP3_P6B_CLOCK_UDP=SOCKET_FAIL\n");
        return;
    }

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(EXP3_P6B_CLOCK_TELEMETRY_PORT);
    address.sin_addr.s_addr = inet_addr(EXP3_P6B_CLOCK_TELEMETRY_IP);

    payload_length = snprintf(
        payload,
        sizeof(payload),
        "EXP3_P9_TIMING "
        "fps_num=%u fps_den=%u "
        "frame_period_ticks=%llu "
        "vsync_waits=%u "
        "deadline_misses=%u "
        "max_deadline_late_ticks=%llu "
        "prefill_target=%u "
        "prefill_observed=%u "
        "decode_n=%u decode_total=%llu decode_max=%llu "
        "upload_n=%u upload_total=%llu upload_max=%llu "
        "draw_n=%u draw_total=%llu draw_max=%llu",
        EXP3_P6_SOURCE_FPS_NUMERATOR,
        EXP3_P6_SOURCE_FPS_DENOMINATOR,
        (unsigned long long)(
            ((u64)kBUSCLK *
             (u64)EXP3_P6_SOURCE_FPS_DENOMINATOR) /
            (u64)EXP3_P6_SOURCE_FPS_NUMERATOR),
        state->p6_vsync_waits,
        state->p6_deadline_misses,
        (unsigned long long)state->p6_max_deadline_late_ticks,
        state->p7a_prefill_target,
        state->p7a_prefill_observed,
        state->p9_decode_samples,
        (unsigned long long)state->p9_decode_ticks_total,
        (unsigned long long)state->p9_decode_ticks_max,
        state->p9_upload_samples,
        (unsigned long long)state->p9_upload_ticks_total,
        (unsigned long long)state->p9_upload_ticks_max,
        state->p9_draw_samples,
        (unsigned long long)state->p9_draw_ticks_total,
        (unsigned long long)state->p9_draw_ticks_max);

    if (
        payload_length <= 0 ||
        payload_length >= (int)sizeof(payload)
    ) {
        printf("EXP3_P6B_CLOCK_UDP=FORMAT_FAIL\n");
        close(socket_fd);
        return;
    }

    sent = __real_sendto(
        socket_fd,
        payload,
        (size_t)payload_length,
        0,
        (struct sockaddr *)&address,
        sizeof(address));

    if (sent == payload_length)
        printf("EXP3_P6B_CLOCK_UDP=PASS bytes=%d\n", sent);
    else
        printf("EXP3_P6B_CLOCK_UDP=SEND_FAIL sent=%d expected=%d\n", sent, payload_length);

    close(socket_fd);
}


static void exp3_p9_accumulate_timing(
    u64 elapsed,
    unsigned int *samples,
    u64 *total,
    u64 *maximum)
{
    if (samples == NULL || total == NULL || maximum == NULL)
        return;

    *samples += 1u;
    *total += elapsed;

    if (elapsed > *maximum)
        *maximum = elapsed;
}

int main(void)
{
    framebuffer_t frame;
    zbuffer_t z;
    packet_t *packet;

    Exp3DecoderState decoder;

    int picture_result;
    int picture_index;
    int pictures_decoded;
    int pictures_displayed;

    s64 picture_pts;
    s64 last_picture_pts;

    u64 p9_stage_start;
    u64 p9_stage_elapsed;

    int stream_exhausted;

    Exp3LiveMpegStreamRuntime stream_runtime;
    int network_integrity;

    memset(
        &decoder,
        0,
        sizeof(decoder));

    memset(
        &stream_runtime,
        0,
        sizeof(stream_runtime));

    printf("EXP3_O3_LIVE_STREAM_START\n");

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

    if (!exp3_live_mpeg_stream_start(&stream_runtime)) {
        printf("EXP3_O3_STREAM_START=FAIL error=%d\n", stream_runtime.error);
        SleepThread();
        return 14;
    }

    decoder.stream_runtime = &stream_runtime;

    printf(
        "EXP3_O3_STREAM_RUNTIME_READY queue_capacity=%u\n",
        (unsigned int)stream_runtime.queue_capacity);

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
            GS_PSM_16,
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

    /*
     * Mature SMS selects this exact libmpeg core mode whenever its IPU/GS
     * texture format is PSMCT16. The call patches the IPU CSC/DMA output path
     * itself; this is not an EE-side post-conversion.
     */
    _MPEG_Set16(1);

    printf(
        "EXP3_P8_RGB16=ENABLED bytes_per_pixel=2 macroblock_bytes=512 macroblock_qwc=32\n");

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

    if (!exp3_p7a_wait_for_prefill(&decoder)) {
        printf(
            "EXP3_P7A_PREFILL=FAIL target=%u observed=%u queue_capacity=%u error=%d\n",
            decoder.p7a_prefill_target,
            decoder.p7a_prefill_observed,
            (unsigned int)stream_runtime.queue_capacity,
            stream_runtime.error);
        SleepThread();
        return 15;
    }

    printf(
        "EXP3_P7A_PREFILL=PASS target=%u observed=%u queue_capacity=%u\n",
        decoder.p7a_prefill_target,
        decoder.p7a_prefill_observed,
        (unsigned int)stream_runtime.queue_capacity);

    printf(
        "EXP3_STAGE=BEFORE_FIRST_MPEG_PICTURE\n");

    picture_pts = 0;

    /*
     * On the first call libmpeg parses the sequence, invokes
     * exp3_sequence_init(), substitutes that callback's returned picture
     * buffer, and then performs the first picture decode.
     */
    p9_stage_start = GetTimerSystemTime();

    picture_result =
        MPEG_Picture(
            NULL,
            &picture_pts);

    p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

    exp3_p9_accumulate_timing(
        p9_stage_elapsed,
        &decoder.p9_decode_samples,
        &decoder.p9_decode_ticks_total,
        &decoder.p9_decode_ticks_max);

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
         * picture 1 decoded successfully.
         *
         * This preserves the already-qualified decoder-success boundary
         * before bounded playback begins.
         */
        exp3_show_color(
            packet,
            &frame,
            &z,
            0,
            192,
            0);

        printf(
            "EXP3_FIRST_PICTURE_DECODE=PASS\n");

        exp3_hold_vsyncs(30);

        if (
            decoder.transfer_packet == NULL ||
            decoder.draw_packet == NULL
        ) {
            /*
             * MAGENTA:
             * sequence callback did not leave usable display packets.
             */
            exp3_show_color(
                packet,
                &frame,
                &z,
                192,
                0,
                192);

            printf(
                "EXP3_FULL_STREAM_EOF="
                "PACKET_SETUP_FAIL\n");

            SleepThread();
            return 2;
        }

        /*
         * Present picture 1 through the already-qualified GS path.
         */
        printf(
            "EXP3_STAGE=BEFORE_FIRST_GS_UPLOAD\n");

        p9_stage_start = GetTimerSystemTime();

        exp3_upload_picture_texture(
            &decoder);

        p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

        exp3_p9_accumulate_timing(
            p9_stage_elapsed,
            &decoder.p9_upload_samples,
            &decoder.p9_upload_ticks_total,
            &decoder.p9_upload_ticks_max);

        /*
         * BLUE:
         * first source-chain macroblock upload returned.
         */
        exp3_show_color(
            packet,
            &frame,
            &z,
            0,
            0,
            192);

        exp3_hold_vsyncs(30);

        exp3_p6_wait_for_picture(
            &decoder,
            1u);

        p9_stage_start = GetTimerSystemTime();

        exp3_draw_picture_texture(
            &decoder);

        p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

        exp3_p9_accumulate_timing(
            p9_stage_elapsed,
            &decoder.p9_draw_samples,
            &decoder.p9_draw_ticks_total,
            &decoder.p9_draw_ticks_max);

        pictures_decoded = 1;
        pictures_displayed = 1;
        last_picture_pts = picture_pts;

        printf(
            "EXP3_FRAME "
            "index=1 "
            "pts=%lld "
            "decoded=%d "
            "displayed=%d "
            "feed_calls=%u "
            "payload_bytes=%u "
            "dma_bytes=%u\n",
            picture_pts,
            pictures_decoded,
            pictures_displayed,
            decoder.feed_calls,
            decoder.payload_bytes_submitted,
            decoder.dma_bytes_submitted);

        /*
         * Pictures 2..EOF are the only new behavior in this experiment.
         *
         * Ownership remains identical to the qualified 60-picture test:
         *
         *   MPEG_Picture
         *   -> source-chain texture upload
         *   -> ordinary GIF wait
         *   -> textured draw
         *   -> two VSYNCs
         *   -> next MPEG_Picture
         *
         * The loop ends only when MPEG_Picture() returns zero.
         */
        for (
            picture_index = 2;
            ;
            ++picture_index
        ) {
            picture_pts = 0;

            printf(
                "EXP3_STAGE=BEFORE_MPEG_PICTURE "
                "index=%d\n",
                picture_index);

            p9_stage_start = GetTimerSystemTime();

            picture_result =
                MPEG_Picture(
                    s_picture_buffer,
                    &picture_pts);

            p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

            exp3_p9_accumulate_timing(
                p9_stage_elapsed,
                &decoder.p9_decode_samples,
                &decoder.p9_decode_ticks_total,
                &decoder.p9_decode_ticks_max);

            if (
                picture_result == 0
            ) {
                printf(
                    "EXP3_STAGE=NATURAL_DECODER_RETURN "
                    "requested_index=%d "
                    "decoded=%d "
                    "displayed=%d "
                    "eof_flag=%d\n",
                    picture_index,
                    pictures_decoded,
                    pictures_displayed,
                    decoder.sequence_info
                        ? decoder.sequence_info->m_fEOF
                        : -1);

                break;
            }

            pictures_decoded += 1;
            last_picture_pts =
                picture_pts;

            p9_stage_start = GetTimerSystemTime();

            exp3_upload_picture_texture(
                &decoder);

            p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

            exp3_p9_accumulate_timing(
                p9_stage_elapsed,
                &decoder.p9_upload_samples,
                &decoder.p9_upload_ticks_total,
                &decoder.p9_upload_ticks_max);

            exp3_p6_wait_for_picture(
                &decoder,
                (unsigned int)picture_index);

            p9_stage_start = GetTimerSystemTime();

            exp3_draw_picture_texture(
                &decoder);

            p9_stage_elapsed = GetTimerSystemTime() - p9_stage_start;

            exp3_p9_accumulate_timing(
                p9_stage_elapsed,
                &decoder.p9_draw_samples,
                &decoder.p9_draw_ticks_total,
                &decoder.p9_draw_ticks_max);

            pictures_displayed += 1;

            printf(
                "EXP3_FRAME "
                "index=%d "
                "pts=%lld "
                "decoded=%d "
                "displayed=%d "
                "feed_calls=%u "
                "payload_bytes=%u "
                "dma_bytes=%u\n",
                picture_index,
                picture_pts,
                pictures_decoded,
                pictures_displayed,
                decoder.feed_calls,
                decoder.payload_bytes_submitted,
                decoder.dma_bytes_submitted);
        }

        stream_exhausted =
            exp3_live_mpeg_stream_is_exhausted(
                &stream_runtime);

        printf(
            "EXP3_FULL_STREAM_EOF_RETURN "
            "coded_producer=%u "
            "decoded=%d "
            "displayed=%d "
            "last_pts=%lld "
            "stream_exhausted=%d "
            "eof_flag=%d "
            "feed_calls=%u "
            "payload_bytes=%u "
            "dma_bytes=%u "
            "sequence_callbacks=%u "
            "width=%u "
            "height=%u\n",
            (unsigned int)stream_runtime.producer_picture_starts,
            pictures_decoded,
            pictures_displayed,
            last_picture_pts,
            stream_exhausted,
            decoder.sequence_info
                ? decoder.sequence_info->m_fEOF
                : -1,
            decoder.feed_calls,
            decoder.payload_bytes_submitted,
            decoder.dma_bytes_submitted,
            decoder.sequence_callbacks,
            decoder.sequence_width,
            decoder.sequence_height);

        /*
         * Test M2 deliberately does not assume that the number of coded
         * MPEG pictures must equal the number returned by MPEG_Picture().
         *
         * SMS keeps reference pictures delayed for display reordering. Its
         * sequence-end path returns from _get_next_picture() immediately,
         * so this experiment records exactly what the public API exposes.
         */

        printf(
            "EXP3_EOF_CHARACTERIZATION "
            "coded_producer=%u "
            "returned=%d "
            "displayed=%d "
            "last_pts=%lld "
            "stream_exhausted=%d "
            "eof_flag=%d "
            "feed_calls=%u "
            "payload_bytes=%u "
            "dma_bytes=%u\n",
            (unsigned int)stream_runtime.producer_picture_starts,
            pictures_decoded,
            pictures_displayed,
            last_picture_pts,
            stream_exhausted,
            decoder.sequence_info
                ? decoder.sequence_info->m_fEOF
                : -1,
            decoder.feed_calls,
            decoder.payload_bytes_submitted,
            decoder.dma_bytes_submitted);

        printf(
            "EXP3_P6_CLOCK "
            "fps_num=%u fps_den=%u "
            "frame_period_ticks=%llu "
            "vsync_waits=%u "
            "deadline_misses=%u "
            "max_deadline_late_ticks=%llu\n",
            EXP3_P6_SOURCE_FPS_NUMERATOR,
            EXP3_P6_SOURCE_FPS_DENOMINATOR,
            (unsigned long long)(
                ((u64)kBUSCLK *
                 (u64)EXP3_P6_SOURCE_FPS_DENOMINATOR) /
                (u64)EXP3_P6_SOURCE_FPS_NUMERATOR),
            decoder.p6_vsync_waits,
            decoder.p6_deadline_misses,
            (unsigned long long)decoder.p6_max_deadline_late_ticks);

        /*
         * P6B instrumentation-only side channel. Playback is already over,
         * so this datagram cannot perturb the measured presentation cadence.
         */
        exp3_p6b_send_clock_telemetry(
            &decoder);

        /*
         * FIRST TERMINAL MARKER — dynamic SMS EOF characterization.
         *
         * PURPLE = SMS returned/displayed one fewer picture than the live
         *          producer counted in the elementary stream.
         * WHITE  = SMS returned/displayed every coded picture.
         * RED    = any other relationship.
         */
        if (
            stream_runtime.producer_picture_starts != 0u &&
            (uint32_t)pictures_decoded + 1u ==
                stream_runtime.producer_picture_starts &&
            pictures_displayed == pictures_decoded
        ) {
            exp3_show_color(packet, &frame, &z, 160, 0, 160);
            printf(
                "EXP3_O3_DECODER_RETURN=CODED_MINUS_ONE "
                "coded=%u returned=%d displayed=%d\n",
                (unsigned int)stream_runtime.producer_picture_starts,
                pictures_decoded,
                pictures_displayed);
        } else if (
            stream_runtime.producer_picture_starts != 0u &&
            (uint32_t)pictures_decoded ==
                stream_runtime.producer_picture_starts &&
            pictures_displayed == pictures_decoded
        ) {
            exp3_show_color(packet, &frame, &z, 224, 224, 224);
            printf(
                "EXP3_O3_DECODER_RETURN=CODED_ALL "
                "coded=%u returned=%d displayed=%d\n",
                (unsigned int)stream_runtime.producer_picture_starts,
                pictures_decoded,
                pictures_displayed);
        } else {
            exp3_show_color(packet, &frame, &z, 192, 0, 0);
            printf(
                "EXP3_O3_DECODER_RETURN=OTHER "
                "coded=%u returned=%d displayed=%d\n",
                (unsigned int)stream_runtime.producer_picture_starts,
                pictures_decoded,
                pictures_displayed);
        }

        exp3_hold_vsyncs(60);

        /*
         * SECOND TERMINAL MARKER — concurrent network integrity.
         * CYAN = exact bytes/CRC, empty queue, receiver done, no stream error.
         * RED  = any mismatch.
         */
        network_integrity =
            exp3_live_mpeg_stream_integrity_pass(&stream_runtime);

        if (stream_exhausted && network_integrity) {
            exp3_show_color(packet, &frame, &z, 0, 160, 160);
            printf("EXP3_O3_STREAM_INTEGRITY=PASS\n");
        } else {
            exp3_show_color(packet, &frame, &z, 192, 0, 0);
            printf(
                "EXP3_O3_STREAM_INTEGRITY=FAIL exhausted=%d integrity=%d error=%d remaining=%u\n",
                stream_exhausted,
                network_integrity,
                stream_runtime.error,
                (unsigned int)exp3_live_mpeg_stream_remaining_bytes(&stream_runtime));
        }

        exp3_hold_vsyncs(60);

        /*
         * The last successfully uploaded picture remains resident in GS
         * texture memory. Redraw it without touching the EE picture buffer.
         *
         * Its visible test-pattern time/frame gives an independent reading
         * of the last picture actually returned by SMS.
         */
        exp3_draw_picture_texture(
            &decoder);

        if (exp3_live_mpeg_stream_send_result(
                &stream_runtime,
                (uint32_t)pictures_decoded,
                (uint32_t)pictures_displayed,
                decoder.feed_calls,
                decoder.payload_bytes_submitted,
                decoder.dma_bytes_submitted)) {
            printf("EXP3_O3_RESULT_SEND=PASS\n");
        } else {
            printf("EXP3_O3_RESULT_SEND=FAIL error=%d\n", stream_runtime.error);
        }

        printf(
            "EXP3_EOF_CHARACTERIZATION_FINAL_FRAME_HELD=PASS\n");

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
