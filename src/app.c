#include "app.h"

#include <stdint.h>

#include "display.h"
#include "framebuffer.h"
#include "platform/ps2_graphics.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb_session.h"

static uint16_t remote_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));
static uint16_t gs_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));

int pstvnc_app_run(void)
{
    pstvnc_framebuffer_t framebuffer;
    pstvnc_rfb_session_t session;
    int socket_fd = -1;
    int graphics_ready = 0;

    if (pstvnc_ps2_system_prepare_iop() < 0)
        goto fail;

    if (pstvnc_ps2_network_init() < 0)
        goto fail;

    if (pstvnc_ps2_network_wait_link() < 0)
        goto fail;

    if (!pstvnc_framebuffer_init(
            &framebuffer,
            remote_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT))
        goto fail;

    if (!pstvnc_framebuffer_set_geometry(
            &framebuffer,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT))
        goto fail;

    if (pstvnc_ps2_graphics_init() < 0)
        goto fail;

    graphics_ready = 1;

    socket_fd = pstvnc_ps2_network_connect_vnc();
    if (socket_fd < 0)
        goto fail;

    pstvnc_rfb_session_init(&session);

    if (!pstvnc_rfb_session_start(
            &session,
            socket_fd,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT))
        goto fail;

    if (!pstvnc_rfb_session_receive_initial_frame(
            &session,
            &framebuffer))
        goto fail;

    if (!pstvnc_display_prepare_gs16(
            &framebuffer,
            gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT))
        goto fail;

    if (pstvnc_ps2_graphics_present(
            gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT) < 0)
        goto fail;

    for (;;) {
        if (!pstvnc_rfb_session_request_update(&session, 1))
            goto fail;

        if (!pstvnc_rfb_session_receive_update(
                &session,
                &framebuffer))
            goto fail;

        if (!framebuffer.valid)
            goto fail;

        if (!framebuffer.dirty)
            continue;

        if (!pstvnc_display_prepare_gs16(
                &framebuffer,
                gs_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT))
            goto fail;

        if (pstvnc_ps2_graphics_present(
                gs_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT) < 0)
            goto fail;
    }

fail:
    if (socket_fd >= 0)
        pstvnc_ps2_network_close(socket_fd);

    if (graphics_ready)
        pstvnc_ps2_graphics_shutdown();

    return -1;
}
