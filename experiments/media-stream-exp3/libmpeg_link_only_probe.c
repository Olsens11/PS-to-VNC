/*
 * File synopsis:
 * Media Stream EXP3 libmpeg-link-only hardware bisection probe.
 *
 * Proven starting point:
 *   - wLaunchELF launch works;
 *   - roughly 9 MB ELF works;
 *   - exact 8.8 MB MPEG payload works when embedded;
 *   - EE reads of that payload work;
 *   - GIF/GS presentation works;
 *   - DMA_CHANNEL_toIPU initialization works.
 *
 * This probe adds one property:
 *
 *   libmpeg is forced into the linked executable by retaining a live
 *   reference to MPEG_Initialize.
 *
 * MPEG_Initialize is NOT called.
 * MPEG_Picture is NOT called.
 * No data is transferred to the IPU.
 *
 * Expected result:
 *   solid cyan screen indefinitely.
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
 * Volatile retained function reference:
 *
 * This creates an actual undefined MPEG_Initialize symbol in this translation
 * unit. The final link must therefore pull the defining libmpeg object from
 * libmpeg.a even though the function is never executed.
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

    /*
     * Read the retained libmpeg function pointer at runtime as well.
     * It must be non-NULL, but it is deliberately never invoked.
     */
    if (g_mpeg_initialize_witness == NULL) {
        SleepThread();
    }

    printf(
        "EXP3_LIBMPEG_LINK_PROBE_START "
        "bytes=%u witness=%u\n",
        (unsigned int)payload_size,
        payload_witness);

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

    q = packet->data;

    q = draw_setup_environment(
        q,
        0,
        &frame,
        &z);

    /*
     * Cyan = libmpeg is linked and addressable, but no libmpeg code has run.
     */
    q = draw_clear(
        q,
        0,
        0,
        0,
        640.0f,
        512.0f,
        0,
        160,
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

    printf(
        "EXP3_LIBMPEG_LINK_PROBE_CYAN_STABLE "
        "bytes=%u witness=%u\n",
        (unsigned int)payload_size,
        payload_witness);

    SleepThread();

    return 0;
}
