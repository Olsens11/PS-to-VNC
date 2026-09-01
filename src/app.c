/*
 * File synopsis:
 * Runs the application coordinator: ordered startup, the live RFB/presentation
 * loop, failure policy, and owned cleanup. Subsystems retain their own private
 * mechanisms.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Complete application
 * coordinator"; docs/CLEAN_ARCHITECTURE.md, "Application coordinator" and
 * "Live loop".
 */

#include "app.h"

#include <stdint.h>

#include "diagnostics.h"
#include "display.h"
#include "framebuffer.h"
#include "platform/ps2_graphics.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb_session.h"

/*
 * These buffers have deliberately different authority. remote_pixels backs the
 * one CPU-side image trusted to represent the complete VNC desktop. gs_pixels
 * is disposable presentation data with the GS alpha bit added; display
 * conversion must never modify remote authority in place. The 128-byte
 * alignment satisfies the PS2 DMA/cache-facing presentation path.
 */
static uint16_t remote_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));
static uint16_t gs_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));

static void send_diagnostic_literal(
    int diagnostics_ready,
    const char *text,
    size_t length)
{
    if (diagnostics_ready)
        (void)pstvnc_diagnostics_send(text, length);
}

int pstvnc_app_run(void)
{
    static const char net_ready[] = "PSTVNC_STAGE NET_READY";
    static const char gs_ready[] = "PSTVNC_STAGE GS_READY";
    static const char desktop_ready[] = "PSTVNC_STAGE DESKTOP_READY";
    static const char fatal[] = "PSTVNC_STAGE FATAL";

    pstvnc_framebuffer_t framebuffer;
    pstvnc_rfb_session_t session;
    int socket_fd = -1;
    int graphics_ready = 0;
    int diagnostics_ready = 0;

    /*
     * The coordinator owns ordering and failure policy. Each subsystem owns its
     * mechanism, but none may independently advance the product to READY or
     * decide how a fatal startup failure converges.
     */
    if (pstvnc_ps2_system_prepare_iop() < 0)
        goto fail;

    if (pstvnc_ps2_network_init() < 0)
        goto fail;

    if (pstvnc_ps2_network_wait_link() < 0)
        goto fail;

    if (pstvnc_diagnostics_init() == 0) {
        diagnostics_ready = 1;
        send_diagnostic_literal(
            diagnostics_ready,
            net_ready,
            sizeof(net_ready) - 1u);
    }

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
    send_diagnostic_literal(
        diagnostics_ready,
        gs_ready,
        sizeof(gs_ready) - 1u);

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

    /*
     * Session startup has requested a non-incremental desktop. The receive call
     * publishes framebuffer.valid only after proving exact coverage of every
     * pixel; presenting before that point could expose unwritten or stale data.
     */
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

    send_diagnostic_literal(
        diagnostics_ready,
        desktop_ready,
        sizeof(desktop_ready) - 1u);

    /*
     * Issue #7 intentionally has one blocking owner for both halves of the RFB
     * exchange: request one update, consume one complete server update, then
     * present coherent state. Input concurrency and safe-boundary yielding are
     * later responsibilities, not hidden behavior in this baseline loop.
     */
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
    /*
     * Every fatal path converges here. Diagnostics is observational and
     * best-effort; resource ownership flags determine cleanup, and main() owns
     * the final transition back to OSDSYS.
     */
    send_diagnostic_literal(
        diagnostics_ready,
        fatal,
        sizeof(fatal) - 1u);

    if (socket_fd >= 0)
        pstvnc_ps2_network_close(socket_fd);

    if (graphics_ready)
        pstvnc_ps2_graphics_shutdown();

    if (diagnostics_ready)
        pstvnc_diagnostics_shutdown();

    return -1;
}
