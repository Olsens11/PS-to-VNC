/*
 * File synopsis:
 * Media Stream EXP3 MPEG_Initialize-only hardware bisection probe.
 *
 * Proven before this test:
 *   - standalone wLaunchELF execution;
 *   - GIF/GS presentation;
 *   - roughly 9 MB ELF loading;
 *   - exact embedded 8.8 MB MPEG payload;
 *   - EE access to that payload;
 *   - DMA_CHANNEL_toIPU initialization;
 *   - libmpeg linkage and runtime addressability.
 *
 * This probe adds exactly one real libmpeg operation:
 *
 *     MPEG_Initialize(...)
 *
 * It deliberately does NOT:
 *   - feed MPEG bytes to the IPU;
 *   - call MPEG_Picture();
 *   - parse a sequence header;
 *   - invoke the sequence/display callback;
 *   - allocate MPEG frame buffers;
 *   - call MPEG_Destroy();
 *
 * Visible contract:
 *
 *   cyan for approximately two seconds
 *       = execution reached immediately before MPEG_Initialize();
 *
 *   orange indefinitely
 *       = MPEG_Initialize() returned successfully and GS presentation
 *         continued to work.
 *
 *   cyan indefinitely
 *       = MPEG_Initialize() did not return.
 *
 *   black/reset/etc.
 *       = initialization crossed a lower-level failure boundary.
 */

#include "libmpeg.h"

#include <dma.h>
#include <draw.h>
#include <graph.h>
#include <gs_psm.h>
#include <kernel.h>
#include <packet.h>
#include <stddef.h>
#include <stdio.h>

extern const unsigned char _binary_test_bin_start[];
extern const unsigned char _binary_test_bin_end[];

static int exp3_no_data(void *user_data)
{
    (void)user_data;

    /*
     * MPEG_Initialize() should only store this callback.
     * It is not expected to invoke it during this probe.
     */
    return 0;
}

static void *exp3_no_sequence(
    void *user_data,
    MPEGSequenceInfo *sequence)
{
    (void)user_data;
    (void)sequence;

    /*
     * Likewise, this callback should not be reached until actual stream
     * parsing discovers a sequence header.
     */
    return NULL;
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

    dma_wait_fast();

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

    s64 current_pts = 0;

    size_t payload_size =
        (size_t)(
            _binary_test_bin_end -
            _binary_test_bin_start);

    volatile unsigned int payload_witness =
        (unsigned int)_binary_test_bin_start[0] +
        (unsigned int)_binary_test_bin_start[payload_size - 1];

    printf(
        "EXP3_MPEG_INIT_PROBE_START "
        "bytes=%u witness=%u\n",
        (unsigned int)payload_size,
        payload_witness);

    /*
     * Preserve the already-proven IPU DMA channel initialization.
     */
    dma_channel_initialize(
        DMA_CHANNEL_toIPU,
        NULL,
        0);

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

    packet = packet_init(
        100,
        PACKET_NORMAL);

    dma_channel_initialize(
        DMA_CHANNEL_GIF,
        NULL,
        0);

    dma_channel_fast_waits(
        DMA_CHANNEL_GIF);

    graph_initialize(
        0,
        640,
        512,
        GS_PSM_32,
        0,
        0);

    /*
     * Persistent-enough pre-call breadcrumb.
     *
     * Cyan was also the preceding passing link-only probe, so seeing this
     * confirms we reached the same known-good state before the new operation.
     */
    exp3_show_color(
        packet,
        &frame,
        &z,
        0,
        160,
        160);

    printf("EXP3_MPEG_INIT_BEFORE_CALL\n");

    /*
     * Hold the pre-call cyan breadcrumb for about two seconds using the
     * already-proven video timing primitive rather than introducing another
     * EE timing API. NTSC has approximately 60 field vsyncs per second.
     */
    {
        int hold_vsync;

        for (hold_vsync = 0; hold_vsync < 120; ++hold_vsync) {
            graph_wait_vsync();
        }
    }

    /*
     * This is the ONE newly introduced operation under test.
     *
     * Source inspection of PS2SDK libmpeg shows this initializes/reset
     * decoder/IPU state, creates synchronization state, installs the DMAC
     * handler, and stores callbacks. It should not consume our stream yet.
     */
    MPEG_Initialize(
        exp3_no_data,
        NULL,
        exp3_no_sequence,
        NULL,
        &current_pts);

    printf("EXP3_MPEG_INIT_RETURNED\n");

    /*
     * Orange is the PASS state for this probe.
     */
    exp3_show_color(
        packet,
        &frame,
        &z,
        192,
        64,
        0);

    printf(
        "EXP3_MPEG_INIT_ORANGE_STABLE "
        "current_pts=%lld\n",
        current_pts);

    /*
     * Deliberately do not call MPEG_Destroy(): that would introduce another
     * libmpeg operation into this bisection stage.
     */
    SleepThread();

    return 0;
}
