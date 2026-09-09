/*
 * File synopsis:
 * Runs the unchanged through-Issue-39 RFB session/parser headlessly against H1
 * logical channel 1.
 *
 * This checkpoint deliberately stops at CPU framebuffer authority.  It does not
 * initialize gsKit, the H1 MPEG presenter, controller input, pointer routing,
 * keyboard, OSK, or local UI.  Its purpose is to make the RFB 3.8 handshake,
 * complete initial Raw framebuffer, incremental requests/updates, parser byte
 * consumption, and mux credit behavior observable before presentation ownership
 * is changed.
 */

#include "h1_rfb_session_runtime.h"

#include "display.h"
#include "h1_config.h"
#include "h1_rfb_transport_live.h"
#include "h1_transport_runtime.h"

#include <delaythread.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define H1_RFB_IDLE_DELAY_US 1000u
#define H1_RFB_DIAGNOSTIC_MARKER 0xA0000000u

static uint32_t h1_rfb_diagnostic_word(
    const pstvnc_h1_rfb_session_runtime_t *runtime)
{
    uint32_t state;
    uint32_t error;
    uint32_t updates;

    if (runtime == NULL)
        return H1_RFB_DIAGNOSTIC_MARKER;

    state = ((uint32_t)runtime->session.state & 0x0fu) << 24;
    error = ((uint32_t)runtime->session.error & 0xffu) << 16;
    updates = runtime->stats.incremental_updates_complete & 0xffffu;

    return H1_RFB_DIAGNOSTIC_MARKER | state | error | updates;
}

static void h1_rfb_publish_diagnostic(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport)
{
    pstvnc_h1_transport_set_diagnostic_word(
        transport,
        h1_rfb_diagnostic_word(runtime));
}

void pstvnc_h1_rfb_session_runtime_init(
    pstvnc_h1_rfb_session_runtime_t *runtime)
{
    if (runtime == NULL)
        return;

    memset(runtime, 0, sizeof(*runtime));
    pstvnc_rfb_session_init(&runtime->session);
}

static int h1_rfb_prepare_framebuffer(
    pstvnc_h1_rfb_session_runtime_t *runtime)
{
    size_t bytes;

    runtime->pixel_capacity = PSTVNC_DISPLAY_PIXEL_COUNT;

    if (runtime->pixel_capacity > ((size_t)-1) / sizeof(uint16_t))
        return 0;

    bytes = runtime->pixel_capacity * sizeof(uint16_t);
    runtime->pixels = (uint16_t *)malloc(bytes);
    if (runtime->pixels == NULL)
        return 0;

    if (!pstvnc_framebuffer_init(
            &runtime->framebuffer,
            runtime->pixels,
            runtime->pixel_capacity) ||
        !pstvnc_framebuffer_set_geometry(
            &runtime->framebuffer,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT))
        return 0;

    runtime->initialized = 1;
    return 1;
}

static int h1_rfb_terminal_state(
    pstvnc_h1_transport_runtime_t *transport,
    int *clean_terminal)
{
    pstvnc_h1_rfb_transport_snapshot_t snapshot;

    *clean_terminal = 0;

    if (pstvnc_h1_transport_last_error(transport) !=
        PSTVNC_H1_ERROR_NONE)
        return 1;

    if (!transport->receiver_done)
        return 0;

    if (!transport->end_received)
        return 1;

    if (!pstvnc_h1_rfb_transport_snapshot(
            transport,
            &snapshot))
        return 1;

    /*
     * A finite harness end is clean only after every channel-1 byte already
     * received before MEDIA_END has been consumed.  If a partial RFB message is
     * stranded, the parser will enter exact-read and fail rather than allowing
     * the terminal marker to hide a framing cut.
     */
    if (snapshot.queue_current != 0u)
        return 0;

    *clean_terminal = 1;
    return 1;
}

int pstvnc_h1_rfb_session_runtime_run(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport)
{
    int clean_terminal;

    if (runtime == NULL || transport == NULL ||
        transport->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        transport->config.audio_mode != PSTVNC_H1_AUDIO_OFF ||
        transport->config.video_mode != PSTVNC_H1_VIDEO_OFF ||
        !transport->rfb_resources.active)
        return -1;

    pstvnc_h1_rfb_session_runtime_init(runtime);

    if (!h1_rfb_prepare_framebuffer(runtime))
        goto fail;

    h1_rfb_publish_diagnostic(runtime, transport);

    /*
     * The integer handle remains only the existing clean-session identity token.
     * In this cumulative build rfb_session39.o resolves all three rfb_io calls
     * to the H1 mux adapter, which verifies this exact socket identity and then
     * delegates to logical channel 1.  No second physical recv/send occurs here.
     */
    if (!pstvnc_rfb_session_start(
            &runtime->session,
            transport->socket_fd,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT))
        goto fail;

    runtime->stats.handshake_complete = 1u;
    h1_rfb_publish_diagnostic(runtime, transport);

    if (!pstvnc_rfb_session_receive_initial_frame(
            &runtime->session,
            &runtime->framebuffer))
        goto fail;

    runtime->stats.initial_frame_complete = 1u;
    h1_rfb_publish_diagnostic(runtime, transport);

    if (!pstvnc_rfb_session_request_update(
            &runtime->session,
            1))
        goto fail;

    runtime->stats.incremental_requests_sent = 1u;

    for (;;) {
        pstvnc_rfb_session_receive_result_t receive_result;

        if (h1_rfb_terminal_state(
                transport,
                &clean_terminal)) {
            if (clean_terminal) {
                h1_rfb_publish_diagnostic(runtime, transport);
                return 0;
            }
            goto fail;
        }

        receive_result = pstvnc_rfb_session_try_receive_update(
            &runtime->session,
            &runtime->framebuffer);

        if (receive_result == PSTVNC_RFB_SESSION_RECEIVE_FAILED)
            goto fail;

        if (receive_result == PSTVNC_RFB_SESSION_RECEIVE_IDLE) {
            if (runtime->stats.idle_polls != UINT32_MAX)
                runtime->stats.idle_polls++;

            h1_rfb_publish_diagnostic(runtime, transport);

            if (DelayThread(H1_RFB_IDLE_DELAY_US) < 0)
                goto fail;
            continue;
        }

        if (receive_result != PSTVNC_RFB_SESSION_RECEIVE_UPDATE ||
            !runtime->framebuffer.valid)
            goto fail;

        if (runtime->stats.incremental_updates_complete == UINT32_MAX)
            goto fail;

        runtime->stats.incremental_updates_complete++;
        h1_rfb_publish_diagnostic(runtime, transport);

        /*
         * Re-check the transport before publishing another incremental request.
         * A finite harness end can arrive immediately after the update bytes.
         */
        if (h1_rfb_terminal_state(
                transport,
                &clean_terminal)) {
            if (clean_terminal)
                return 0;
            goto fail;
        }

        if (!pstvnc_rfb_session_request_update(
                &runtime->session,
                1))
            goto fail;

        if (runtime->stats.incremental_requests_sent == UINT32_MAX)
            goto fail;

        runtime->stats.incremental_requests_sent++;
    }

fail:
    h1_rfb_publish_diagnostic(runtime, transport);
    return -1;
}

void pstvnc_h1_rfb_session_runtime_shutdown(
    pstvnc_h1_rfb_session_runtime_t *runtime)
{
    if (runtime == NULL)
        return;

    free(runtime->pixels);
    runtime->pixels = NULL;
    runtime->pixel_capacity = 0u;
    runtime->initialized = 0;
}
