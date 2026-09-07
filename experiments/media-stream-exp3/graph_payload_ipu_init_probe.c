/*
 * File synopsis:
 * Media Stream EXP3 IPU-DMA-initialization hardware bisection probe.
 *
 * Starting authority:
 * The preceding payload-only probe proved that wLaunchELF can load a roughly
 * 9 MB ELF containing the exact 8.8 MB MPEG-2 elementary stream, access that
 * linked payload from EE code, initialize the GIF/GS path, and hold a stable
 * magenta framebuffer.
 *
 * This probe adds exactly one MPEG-path mechanism:
 *
 *     dma_channel_initialize(DMA_CHANNEL_toIPU, NULL, 0);
 *
 * It does NOT:
 *   - link or call libmpeg;
 *   - parse MPEG;
 *   - transfer MPEG data to the IPU;
 *   - call MPEG_Initialize();
 *   - call MPEG_Picture();
 *   - use networking or audio.
 *
 * Expected hardware result:
 *   solid yellow screen indefinitely.
 *
 * Interpretation:
 *   yellow = IPU DMA-channel initialization returned and ordinary GS
 *            presentation still works;
 *   black  = adding IPU DMA-channel initialization is sufficient to cross the
 *            failure boundary relative to the proven payload-only probe.
 */

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
     * Force genuine EE reads from the embedded payload, as in the preceding
     * passing payload-only probe.
     */
    volatile unsigned int payload_witness =
        (unsigned int)_binary_test_bin_start[0] +
        (unsigned int)_binary_test_bin_start[payload_size - 1];

    printf(
        "EXP3_IPU_INIT_PROBE_START bytes=%u witness=%u\n",
        (unsigned int)payload_size,
        payload_witness);

    /*
     * This is the ONE newly introduced operation under test.
     */
    dma_channel_initialize(
        DMA_CHANNEL_toIPU,
        NULL,
        0);

    printf("EXP3_IPU_INIT_RETURNED\n");

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
     * Yellow distinguishes this stage from:
     *   blue    = graph-only probe;
     *   magenta = embedded-payload-only probe.
     */
    q = draw_clear(
        q,
        0,
        0,
        0,
        640.0f,
        512.0f,
        160,
        160,
        0);

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
        "EXP3_IPU_INIT_PROBE_YELLOW_STABLE "
        "bytes=%u witness=%u\n",
        (unsigned int)payload_size,
        payload_witness);

    SleepThread();

    return 0;
}
