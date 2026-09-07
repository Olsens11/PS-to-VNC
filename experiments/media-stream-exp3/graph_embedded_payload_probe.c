/*
 * File synopsis:
 * Media Stream EXP3 large embedded-payload hardware bisection probe.
 *
 * Purpose:
 * Add the exact MPEG-2 payload used by the failed decoder smoke test to the
 * already-proven graph-only chassis, without invoking libmpeg or the IPU.
 *
 * This isolates:
 *   - large ELF loading;
 *   - static EE memory occupied by the embedded MPEG stream;
 *   - linked binary section layout.
 *
 * It deliberately does NOT use:
 *   - libmpeg;
 *   - IPU DMA;
 *   - MPEG parsing or decoding;
 *   - networking;
 *   - audio.
 *
 * Expected hardware result:
 *   solid magenta screen indefinitely.
 */

#include <dma.h>
#include <draw.h>
#include <graph.h>
#include <gs_psm.h>
#include <kernel.h>
#include <packet.h>
#include <stdio.h>
#include <stddef.h>

extern const unsigned char _binary_test_bin_start[];
extern const unsigned char _binary_test_bin_end[];

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

    /*
     * Read actual payload bytes so this is not merely an unused link section.
     */
    volatile unsigned int payload_witness =
        (unsigned int)_binary_test_bin_start[0] +
        (unsigned int)_binary_test_bin_start[payload_size - 1];

    printf(
        "EXP3_PAYLOAD_PROBE_START bytes=%u witness=%u\n",
        (unsigned int)payload_size,
        payload_witness);

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
     * Magenta distinguishes this probe from the prior blue graph-only probe.
     */
    q = draw_clear(
        q,
        0,
        0,
        0,
        640.0f,
        512.0f,
        160,
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

    printf(
        "EXP3_PAYLOAD_PROBE_MAGENTA_STABLE "
        "bytes=%u witness=%u\n",
        (unsigned int)payload_size,
        payload_witness);

    SleepThread();

    return 0;
}
