/*
 * File synopsis:
 * EXP3 O1 network MPEG-2 fixture-replay test.
 *
 * Goal:
 * Starting from the hardware-qualified SMS one-picture decode/presentation
 * result, decode and display exactly 60 MPEG pictures using the same
 * picture buffer, GIF macroblock upload packet, and GS textured draw.
 *
 * O1 changes only the source of the already-qualified MPEG byte stream:
 * the exact fixture is received from the Pi through PSTV DATA/channel 4,
 * validated completely, and buffered before MPEG_Initialize is called.
 * Audio synchronization and full PS2VNC mux integration remain out of
 * scope for this first network-boundary bisection.
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
 * This test performs exactly 60 successful MPEG_Picture()/presentation
 * iterations and then stops. It does not run to stream EOF.
 *
 * GIF upload completion is explicitly awaited before the next decode is
 * allowed to overwrite the shared picture buffer.
 *
 * PS2SDK's old MPEG sample uses dma_wait_fast() before GIF presentation.
 * That helper is hardware-proven to hang in this EXP3 environment, so this
 * test uses ordinary DMA_CHANNEL_GIF waits instead.
 */

#include "libmpeg.h"
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
    (EXP3_VIDEO_MAX_WIDTH * EXP3_VIDEO_MAX_HEIGHT * 4)

#define EXP3_FEED_BYTES 2048

#define EXP3_EXPECTED_PICTURES 599

#define EXP3_EXPECTED_STREAM_BYTES 8815372u
#define EXP3_EXPECTED_NETWORK_FRAMES 1077u
#define EXP3_NETWORK_REPLAY_PORT 5904
#define EXP3_NETWORK_REPLAY_SERVER_IP "192.168.50.1"
#define EXP3_EXPECTED_STREAM_CRC32 0x4bdc7859u

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


typedef struct Exp3NetworkReplayStats
{
    unsigned int frames_received;
    unsigned int payload_bytes_received;
    unsigned int last_sequence;
    uint32_t crc32;
} Exp3NetworkReplayStats;

/*
 * O1 deliberately reuses the existing PSTV wire-header implementation.
 *
 * TCP is a byte stream, so header and payload reads are exact-size loops and
 * make no assumption that one recv() corresponds to one send() or packet.
 */
static int exp3_socket_read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    unsigned char *destination =
        (unsigned char *)buffer;

    size_t done = 0;

    while (done < count) {
        int received =
            recv(
                socket_fd,
                destination + done,
                count - done,
                0);

        if (received <= 0)
            return 0;

        done +=
            (size_t)received;
    }

    return 1;
}

/*
 * Small table-free CRC32 is sufficient for the controlled O1 fixture.
 *
 * The Pi independently calculates the same IEEE CRC32 from its authoritative
 * fixture before listening. O1 therefore has three independent integrity
 * boundaries:
 *
 *   - authoritative SHA-256 on the Pi before transmission;
 *   - PSTV framing/sequence/byte-count validation on the PS2;
 *   - end-to-end CRC32 over the exact bytes received by the PS2.
 */
static uint32_t exp3_crc32_update(
    uint32_t crc,
    const unsigned char *data,
    size_t count)
{
    size_t index;

    for (index = 0; index < count; ++index) {
        int bit;

        crc ^=
            (uint32_t)data[index];

        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask =
                0u - (crc & 1u);

            crc =
                (crc >> 1) ^
                (0xEDB88320u & mask);
        }
    }

    return crc;
}

/*
 * Receive one exact replay of the qualified 8,815,372-byte MPEG fixture.
 *
 * O1 intentionally does NOT decode while the socket is active. The complete
 * stream is received and validated first, then the socket is closed. Only
 * after this function succeeds does main() initialize graphics/IPU/libmpeg.
 *
 * Required wire shape:
 *
 *   PSTV version 1
 *   DATA frame
 *   MPEG2 channel 4
 *   flags 0
 *   sequence 0..1076
 *   1076 x 8192-byte payloads
 *   1 x 780-byte final payload
 */
static int exp3_receive_network_fixture(
    unsigned char *stream,
    size_t stream_capacity,
    Exp3NetworkReplayStats *stats)
{
    int socket_fd;
    struct sockaddr_in server;

    unsigned int expected_sequence = 0;
    size_t total_bytes = 0;

    uint32_t crc =
        0xFFFFFFFFu;

    if (
        stream == NULL ||
        stats == NULL ||
        stream_capacity !=
            EXP3_EXPECTED_STREAM_BYTES
    ) {
        printf(
            "EXP3_NETWORK_REPLAY=ARGUMENT_FAIL\n");

        return 0;
    }

    memset(
        stats,
        0,
        sizeof(*stats));

    socket_fd =
        socket(
            AF_INET,
            SOCK_STREAM,
            0);

    if (socket_fd < 0) {
        printf(
            "EXP3_NETWORK_REPLAY=SOCKET_FAIL\n");

        return 0;
    }

    memset(
        &server,
        0,
        sizeof(server));

    server.sin_len =
        sizeof(server);

    server.sin_family =
        AF_INET;

    server.sin_port =
        htons(
            EXP3_NETWORK_REPLAY_PORT);

    server.sin_addr.s_addr =
        inet_addr(
            EXP3_NETWORK_REPLAY_SERVER_IP);

    printf(
        "EXP3_NETWORK_STAGE=CONNECT "
        "server=%s port=%u\n",
        EXP3_NETWORK_REPLAY_SERVER_IP,
        (unsigned int)
            EXP3_NETWORK_REPLAY_PORT);

    if (
        connect(
            socket_fd,
            (struct sockaddr *)&server,
            sizeof(server)) < 0
    ) {
        printf(
            "EXP3_NETWORK_REPLAY=CONNECT_FAIL\n");

        close(socket_fd);
        return 0;
    }

    printf(
        "EXP3_NETWORK_STAGE=CONNECTED\n");

    while (
        total_bytes <
        EXP3_EXPECTED_STREAM_BYTES
    ) {
        unsigned char wire_header[
            PSTVNC_TRANSPORT_HEADER_SIZE
        ];

        pstvnc_transport_header_t header;

        size_t remaining;
        unsigned int required_payload;

        if (
            !exp3_socket_read_exact(
                socket_fd,
                wire_header,
                sizeof(wire_header))
        ) {
            printf(
                "EXP3_NETWORK_REPLAY=HEADER_READ_FAIL "
                "sequence=%u bytes=%u\n",
                expected_sequence,
                (unsigned int)total_bytes);

            close(socket_fd);
            return 0;
        }

        if (
            !pstvnc_transport_header_decode(
                &header,
                wire_header)
        ) {
            printf(
                "EXP3_NETWORK_REPLAY=HEADER_DECODE_FAIL "
                "sequence=%u\n",
                expected_sequence);

            close(socket_fd);
            return 0;
        }

        if (
            header.kind !=
                PSTVNC_TRANSPORT_FRAME_DATA ||
            header.channel !=
                PSTVNC_TRANSPORT_CHANNEL_MPEG2 ||
            header.flags != 0u
        ) {
            printf(
                "EXP3_NETWORK_REPLAY=FRAME_CONTRACT_FAIL "
                "kind=%u channel=%u flags=%u\n",
                header.kind,
                header.channel,
                header.flags);

            close(socket_fd);
            return 0;
        }

        if (
            header.sequence !=
            expected_sequence
        ) {
            printf(
                "EXP3_NETWORK_REPLAY=SEQUENCE_FAIL "
                "expected=%u actual=%u\n",
                expected_sequence,
                (unsigned int)header.sequence);

            close(socket_fd);
            return 0;
        }

        remaining =
            EXP3_EXPECTED_STREAM_BYTES -
            total_bytes;

        required_payload =
            remaining >
                PSTVNC_TRANSPORT_MAX_PAYLOAD
            ? PSTVNC_TRANSPORT_MAX_PAYLOAD
            : (unsigned int)remaining;

        if (
            header.payload_length !=
            required_payload
        ) {
            printf(
                "EXP3_NETWORK_REPLAY=PAYLOAD_SHAPE_FAIL "
                "sequence=%u expected=%u actual=%u\n",
                expected_sequence,
                required_payload,
                (unsigned int)header.payload_length);

            close(socket_fd);
            return 0;
        }

        if (
            !exp3_socket_read_exact(
                socket_fd,
                stream + total_bytes,
                header.payload_length)
        ) {
            printf(
                "EXP3_NETWORK_REPLAY=PAYLOAD_READ_FAIL "
                "sequence=%u length=%u\n",
                expected_sequence,
                (unsigned int)header.payload_length);

            close(socket_fd);
            return 0;
        }

        crc =
            exp3_crc32_update(
                crc,
                stream + total_bytes,
                header.payload_length);

        total_bytes +=
            header.payload_length;

        stats->frames_received += 1;
        stats->payload_bytes_received +=
            header.payload_length;

        stats->last_sequence =
            header.sequence;

        expected_sequence += 1;
    }

    close(socket_fd);

    crc ^=
        0xFFFFFFFFu;

    stats->crc32 =
        crc;

    printf(
        "EXP3_NETWORK_REPLAY_RETURN "
        "frames=%u "
        "payload_bytes=%u "
        "last_sequence=%u "
        "crc32=%08x "
        "expected_crc32=%08x\n",
        stats->frames_received,
        stats->payload_bytes_received,
        stats->last_sequence,
        (unsigned int)stats->crc32,
        EXP3_EXPECTED_STREAM_CRC32);

    if (
        stats->frames_received !=
            EXP3_EXPECTED_NETWORK_FRAMES ||
        stats->payload_bytes_received !=
            EXP3_EXPECTED_STREAM_BYTES ||
        stats->last_sequence !=
            EXP3_EXPECTED_NETWORK_FRAMES - 1u ||
        stats->crc32 !=
            EXP3_EXPECTED_STREAM_CRC32
    ) {
        printf(
            "EXP3_NETWORK_REPLAY=INTEGRITY_FAIL\n");

        return 0;
    }

    printf(
        "EXP3_NETWORK_REPLAY=PASS "
        "frames=%u bytes=%u crc32=%08x\n",
        stats->frames_received,
        stats->payload_bytes_received,
        (unsigned int)stats->crc32);

    return 1;
}

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

    graph_wait_vsync();
    graph_wait_vsync();
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

    int stream_exhausted;

    unsigned char *network_stream;
    Exp3NetworkReplayStats network_stats;

    memset(
        &decoder,
        0,
        sizeof(decoder));

    memset(
        &network_stats,
        0,
        sizeof(network_stats));

    printf(
        "EXP3_NETWORK_FIXTURE_REPLAY_START "
        "expected_bytes=%u "
        "expected_frames=%u "
        "expected_crc32=%08x\n",
        EXP3_EXPECTED_STREAM_BYTES,
        EXP3_EXPECTED_NETWORK_FRAMES,
        EXP3_EXPECTED_STREAM_CRC32);

    /*
     * Reuse the qualified product platform seams themselves:
     *
     *   deterministic IOP reset/bootstrap
     *   DEV9 -> NETMAN -> SMAP
     *   fixed PS2 192.168.50.2 / Pi 192.168.50.1 private link
     */
    printf(
        "EXP3_NETWORK_STAGE=BEFORE_IOP_PREPARE\n");

    if (
        pstvnc_ps2_system_prepare_iop() < 0
    ) {
        printf(
            "EXP3_NETWORK_REPLAY=IOP_PREPARE_FAIL\n");

        SleepThread();
        return 10;
    }

    printf(
        "EXP3_NETWORK_STAGE=IOP_PREPARE_RETURNED\n");

    if (
        pstvnc_ps2_network_init() < 0
    ) {
        printf(
            "EXP3_NETWORK_REPLAY=NETWORK_INIT_FAIL\n");

        SleepThread();
        return 11;
    }

    printf(
        "EXP3_NETWORK_STAGE=NETWORK_INIT_RETURNED\n");

    if (
        pstvnc_ps2_network_wait_link() < 0
    ) {
        printf(
            "EXP3_NETWORK_REPLAY=LINK_FAIL\n");

        SleepThread();
        return 12;
    }

    printf(
        "EXP3_NETWORK_STAGE=LINK_UP\n");

    network_stream =
        (unsigned char *)malloc(
            EXP3_EXPECTED_STREAM_BYTES);

    if (network_stream == NULL) {
        printf(
            "EXP3_NETWORK_REPLAY=STREAM_ALLOCATION_FAIL "
            "bytes=%u\n",
            EXP3_EXPECTED_STREAM_BYTES);

        SleepThread();
        return 13;
    }

    printf(
        "EXP3_NETWORK_STREAM_ALLOCATION=PASS "
        "bytes=%u address=%p\n",
        EXP3_EXPECTED_STREAM_BYTES,
        network_stream);

    if (
        !exp3_receive_network_fixture(
            network_stream,
            EXP3_EXPECTED_STREAM_BYTES,
            &network_stats)
    ) {
        printf(
            "EXP3_NETWORK_FIXTURE_REPLAY=FAIL\n");

        free(network_stream);
        SleepThread();
        return 14;
    }

    /*
     * Critical O1 boundary:
     *
     * The replay socket is now CLOSED. From this point onward the existing
     * decoder sees an ordinary contiguous memory range just as it did with the
     * embedded fixture. No network producer runs concurrently with decoding.
     */
    decoder.stream_begin =
        network_stream;

    decoder.stream_cursor =
        network_stream;

    decoder.stream_end =
        network_stream +
        EXP3_EXPECTED_STREAM_BYTES;

    printf(
        "EXP3_NETWORK_FIXTURE_REPLAY=QUALIFIED_FOR_DECODE "
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

        exp3_upload_picture_texture(
            &decoder);

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

        exp3_draw_picture_texture(
            &decoder);

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

            picture_result =
                MPEG_Picture(
                    s_picture_buffer,
                    &picture_pts);

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

            if (
                picture_index >
                    EXP3_EXPECTED_PICTURES
            ) {
                exp3_show_color(
                    packet,
                    &frame,
                    &z,
                    192,
                    0,
                    0);

                printf(
                    "EXP3_FULL_STREAM_EOF="
                    "TOO_MANY_PICTURES "
                    "index=%d "
                    "expected=%d\n",
                    picture_index,
                    EXP3_EXPECTED_PICTURES);

                SleepThread();
                return 4;
            }

            pictures_decoded += 1;
            last_picture_pts =
                picture_pts;

            exp3_upload_picture_texture(
                &decoder);

            exp3_draw_picture_texture(
                &decoder);

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
            decoder.stream_cursor >=
            decoder.stream_end;

        printf(
            "EXP3_FULL_STREAM_EOF_RETURN "
            "expected=%d "
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
            EXP3_EXPECTED_PICTURES,
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
            "coded_expected=%d "
            "returned=%d "
            "displayed=%d "
            "last_pts=%lld "
            "stream_exhausted=%d "
            "eof_flag=%d "
            "feed_calls=%u "
            "payload_bytes=%u "
            "dma_bytes=%u\n",
            EXP3_EXPECTED_PICTURES,
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

        /*
         * FIRST TERMINAL MARKER — returned/displayed count
         *
         * PURPLE = exactly 598 pictures returned/displayed.
         * WHITE  = exactly 599 pictures returned/displayed.
         * RED    = any other count.
         */
        if (
            pictures_decoded == 598 &&
            pictures_displayed == 598
        ) {
            exp3_show_color(
                packet,
                &frame,
                &z,
                160,
                0,
                160);

            printf(
                "EXP3_EOF_RETURN_COUNT=598\n");
        } else if (
            pictures_decoded == 599 &&
            pictures_displayed == 599
        ) {
            exp3_show_color(
                packet,
                &frame,
                &z,
                224,
                224,
                224);

            printf(
                "EXP3_EOF_RETURN_COUNT=599\n");
        } else {
            exp3_show_color(
                packet,
                &frame,
                &z,
                192,
                0,
                0);

            printf(
                "EXP3_EOF_RETURN_COUNT=OTHER "
                "decoded=%d displayed=%d\n",
                pictures_decoded,
                pictures_displayed);
        }

        exp3_hold_vsyncs(60);

        /*
         * SECOND TERMINAL MARKER — input callback position
         *
         * CYAN   = callback cursor reached the exact end of test.bin.
         * YELLOW = sequence end was recognized before the callback cursor
         *          reached the end of the embedded fixture.
         */
        if (
            stream_exhausted
        ) {
            exp3_show_color(
                packet,
                &frame,
                &z,
                0,
                160,
                160);

            printf(
                "EXP3_EOF_STREAM_CURSOR=EXHAUSTED\n");
        } else {
            exp3_show_color(
                packet,
                &frame,
                &z,
                160,
                160,
                0);

            printf(
                "EXP3_EOF_STREAM_CURSOR=NOT_EXHAUSTED "
                "remaining=%u\n",
                (unsigned int)(
                    decoder.stream_end -
                    decoder.stream_cursor));
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
