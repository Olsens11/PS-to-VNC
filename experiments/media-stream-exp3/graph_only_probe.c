/*
 * File synopsis:
 * Media Stream EXP3 graph-only hardware bisection probe.
 *
 * Purpose:
 * Prove that an independently linked ELF launched through wLaunchELF can
 * initialize the PS2 GIF/GS path, present a framebuffer, and remain executing.
 *
 * This probe deliberately contains:
 *   - no MPEG data;
 *   - no libmpeg use;
 *   - no IPU DMA;
 *   - no networking;
 *   - no audio;
 *   - no controller runtime.
 *
 * Expected hardware result:
 *   solid blue screen indefinitely.
 */

#include <dma.h>
#include <draw.h>
#include <graph.h>
#include <gs_psm.h>
#include <kernel.h>
#include <packet.h>
#include <stdio.h>

int main(void)
{
    framebuffer_t frame;
    zbuffer_t z;
    packet_t *packet;
    qword_t *q;

    printf("EXP3_GRAPH_ONLY_START\n");

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

    q = draw_clear(
        q,
        0,
        0,
        0,
        640.0f,
        512.0f,
        0,
        0,
        192);

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

    printf("EXP3_GRAPH_ONLY_BLUE_STABLE\n");

    SleepThread();

    return 0;
}
