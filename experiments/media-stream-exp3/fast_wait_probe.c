/*
 * File synopsis:
 * Media Stream EXP3 dma_wait_fast hardware bisection probe.
 *
 * Proven before this probe:
 *   - large embedded MPEG payload;
 *   - EE payload access;
 *   - GIF/GS presentation;
 *   - DMA_CHANNEL_toIPU initialization;
 *   - libmpeg linkage.
 *
 * This probe does NOT call MPEG_Initialize or MPEG_Picture.
 *
 * The single runtime mechanism under test is:
 *
 *     dma_wait_fast();
 *
 * immediately before the first GIF clear.
 *
 * Expected result:
 *     purple indefinitely.
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

/*
 * Force the same libmpeg linkage proven by the preceding cyan probe,
 * without executing MPEG_Initialize.
 */
static void (*volatile g_mpeg_initialize_witness)(
    int (*)(void *),
    void *,
    void *(*)(void *, MPEGSequenceInfo *),
    void *,
    s64 *) = MPEG_Initialize;

int main(void)
{
    framebuffer_t frame;
    zbuffer_t z;
    packet_t *packet;
    qword_t *q;

    size_t payload_size =
        (size_t)(
            _binary_test_bin_end -
            _binary_test_bin_start);

    volatile unsigned int payload_witness =
        (unsigned int)_binary_test_bin_start[0] +
        (unsigned int)_binary_test_bin_start[payload_size - 1];

    if (g_mpeg_initialize_witness == NULL) {
        SleepThread();
    }

    printf(
        "EXP3_FAST_WAIT_START bytes=%u witness=%u\n",
        (unsigned int)payload_size,
        payload_witness);

    /*
     * Already proven in the yellow and cyan probes.
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

    printf("EXP3_FAST_WAIT_BEFORE\n");

    /*
     * This is the ONE operation under test.
     */
    dma_wait_fast();

    printf("EXP3_FAST_WAIT_RETURNED\n");

    q = packet->data;

    q = draw_setup_environment(
        q,
        0,
        &frame,
        &z);

    q = draw_clear(
        q,
        0,
        0,
        0,
        640.0f,
        512.0f,
        128,
        0,
        160);

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

    printf("EXP3_FAST_WAIT_PURPLE_STABLE\n");

    SleepThread();

    return 0;
}
