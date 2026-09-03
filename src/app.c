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

#define PSTVNC_RFB_RECONNECT_DELAY_MS 500u

static void send_diagnostic_literal(
    int diagnostics_ready,
    const char *text,
    size_t length)
{
    if (diagnostics_ready)
        (void)pstvnc_diagnostics_send(text, length);
}

static int reconnect_rfb_after_io_loss(
    int *socket_fd,
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer)
{
    /*
     * A dead TCP/RFB connection is replaceable transport, not application
     * lifetime. Keep the initialized Ethernet and GS/display state alive and
     * reconnect only the RFB session to the same stable Pi endpoint.
     *
     * Only explicit I/O loss is retryable here. Wrong geometry, malformed RFB,
     * unsupported encoding, bad full-frame coverage, and every other protocol
     * failure remain fatal so reconnect cannot hide real bugs.
     */
    if (*socket_fd >= 0) {
        pstvnc_ps2_network_close(*socket_fd);
        *socket_fd = -1;
    }

    for (;;) {
        /*
         * Try immediately once. If no provider is ready behind the durable
         * endpoint, pause before trying again rather than hammering connect().
         */
        *socket_fd = pstvnc_ps2_network_connect_vnc();

        if (*socket_fd < 0) {
            pstvnc_ps2_system_delay_ms(
                PSTVNC_RFB_RECONNECT_DELAY_MS);
            continue;
        }

        /*
         * A replacement TCP connection is a completely new RFB session.
         * Repeat the handshake and require a new non-incremental full frame
         * before ordinary incremental operation is allowed to resume.
         */
        if (pstvnc_rfb_session_start(
                session,
                *socket_fd,
                PSTVNC_DISPLAY_WIDTH,
                PSTVNC_DISPLAY_HEIGHT) &&
            pstvnc_rfb_session_receive_initial_frame(
                session,
                framebuffer))
            return 1;

        /*
         * A provider that accepted TCP and then disappeared is still ordinary
         * connection loss. A live connection that fails an RFB semantic check
         * is not recoverable here and remains fail-closed.
         */
        if (session->error != PSTVNC_RFB_SESSION_ERROR_IO)
            return 0;

        pstvnc_ps2_network_close(*socket_fd);
        *socket_fd = -1;

        pstvnc_ps2_system_delay_ms(
            PSTVNC_RFB_RECONNECT_DELAY_MS);
    }
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

    if (pstvnc_diagnostics_init() == 0)
        diagnostics_ready = 1;

    /*
     * Connect to the Pi before startup diagnostics.  The frozen PS2IP stack can
     * replace pending UDP packets while ARP is unresolved.
     * See Issue #7 startup-order hardware qualification evidence.
     */
    socket_fd = pstvnc_ps2_network_connect_vnc();
    if (socket_fd < 0)
        goto fail;

    send_diagnostic_literal(
        diagnostics_ready,
        net_ready,
        sizeof(net_ready) - 1u);

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
     * One blocking owner still serializes RFB requests and responses. The only
     * added recovery policy is for an explicitly dead TCP/RFB connection.
     *
     * A silent stall is deliberately unchanged: without an I/O error this
     * thread remains blocked exactly as before, preserving the failure for
     * diagnosis.
     */
    for (;;) {
        if (!pstvnc_rfb_session_request_update(&session, 1) ||
            !pstvnc_rfb_session_receive_update(
                &session,
                &framebuffer)) {
            /*
             * Connection loss must not tear down the whole application. All
             * other RFB failures still use the existing fatal path.
             */
            if (session.error != PSTVNC_RFB_SESSION_ERROR_IO)
                goto fail;

            if (!reconnect_rfb_after_io_loss(
                    &socket_fd,
                    &session,
                    &framebuffer))
                goto fail;

            /*
             * Reconnection produced a new authoritative full frame. Present it
             * while keeping the existing GS/display mode alive, then resume
             * ordinary incremental requests.
             */
            if (!pstvnc_display_prepare_gs16(
                    &framebuffer,
                    gs_pixels,
                    PSTVNC_DISPLAY_PIXEL_COUNT))
                goto fail;

            if (pstvnc_ps2_graphics_present(
                    gs_pixels,
                    PSTVNC_DISPLAY_PIXEL_COUNT) < 0)
                goto fail;

            continue;
        }

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
