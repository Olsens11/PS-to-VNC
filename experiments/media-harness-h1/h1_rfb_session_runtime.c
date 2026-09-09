/*
 * File synopsis:
 * Runs the unchanged through-Issue-39 RFB session/parser against H1 logical
 * channel 1 and owns the PS2 half of clean finite-session quiescence.
 *
 * CP2J qualified this coordinator headlessly and CP2K qualified its complete-
 * framebuffer presentation callback. A second optional callback now lets the
 * application/main thread service semantic input only at complete RFB message
 * boundaries without moving RFB serialization into the controller worker.
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
#define H1_RFB_QUIESCE_WAIT_US 1000u
#define H1_RFB_DIAGNOSTIC_MARKER 0xA0000000u

static uint32_t h1_rfb_diagnostic_word(
    const pstvnc_h1_rfb_session_runtime_t *runtime)
{
    uint32_t state;
    uint32_t error;
    uint32_t phase;
    uint32_t updates;

    if (runtime == NULL)
        return H1_RFB_DIAGNOSTIC_MARKER;

    state = ((uint32_t)runtime->session.state & 0x0fu) << 24;
    error = ((uint32_t)runtime->session.error & 0xffu) << 16;
    phase =
        ((runtime->stats.quiesce_boundary_sent != 0u) ? 1u : 0u) << 15 |
        ((runtime->stats.quiesce_commit_observed != 0u) ? 1u : 0u) << 14 |
        ((runtime->stats.quiesce_complete_sent != 0u) ? 1u : 0u) << 13;
    updates = runtime->stats.incremental_updates_complete & 0x1fffu;

    return H1_RFB_DIAGNOSTIC_MARKER | state | error | phase | updates;
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

static int h1_rfb_wait_for_commit(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport)
{
    while (transport->rfb_quiesce_commit_received == 0u) {
        if (pstvnc_h1_transport_last_error(transport) !=
                PSTVNC_H1_ERROR_NONE ||
            transport->receiver_done ||
            transport->stop_requested)
            return 0;

        h1_rfb_publish_diagnostic(runtime, transport);

        if (DelayThread(H1_RFB_QUIESCE_WAIT_US) < 0)
            return 0;
    }

    runtime->stats.quiesce_commit_observed = 1u;
    h1_rfb_publish_diagnostic(runtime, transport);
    return 1;
}

static int h1_rfb_complete_quiesce_at_boundary(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport)
{
    pstvnc_h1_rfb_transport_snapshot_t snapshot;

    if (transport->rfb_quiesce_request_received == 0u)
        return 0;

    /*
     * We are called only between complete RFB server messages. The coordinator
     * stops issuing framebuffer requests before publishing BOUNDARY, so every
     * RFB client write preceding this marker is already ordered ahead of it on
     * the same PSTV send sequence.
     */
    if (!pstvnc_h1_rfb_transport_send_quiesce_boundary(transport))
        return -1;

    runtime->stats.quiesce_boundary_sent = 1u;
    h1_rfb_publish_diagnostic(runtime, transport);

    /*
     * On BOUNDARY the Pi shuts down and joins its raw VNC reader before sending
     * COMMIT. Because all Pi->PS2 PSTV frames share one send lock/sequence, any
     * final DATA already read by the bridge must appear before COMMIT.
     */
    if (!h1_rfb_wait_for_commit(runtime, transport))
        return -1;

    if (!pstvnc_h1_rfb_transport_snapshot(
            transport,
            &snapshot))
        return -1;

    /*
     * COMMIT is accepted as a clean RFB boundary only if no channel-1 bytes
     * remain after every pre-COMMIT DATA frame has been received. Residual bytes
     * mean the raw bridge had already crossed into another server message.
     */
    if (!snapshot.active || snapshot.queue_current != 0u)
        return -1;

    if (!pstvnc_h1_rfb_transport_send_quiesce_complete(transport))
        return -1;

    runtime->stats.quiesce_complete_sent = 1u;
    h1_rfb_publish_diagnostic(runtime, transport);
    return 1;
}

static int h1_rfb_service_application(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_rfb_service_callback_t service,
    void *service_context)
{
    if (service == NULL)
        return 1;

    if (runtime == NULL ||
        runtime->session.state != PSTVNC_RFB_SESSION_READY)
        return 0;

    /*
     * This call site exists only at server-message boundaries. The service owns
     * no receive work; it may publish already-routed RFB input writes through
     * the same synchronized session before parsing resumes.
     */
    if (!service(service_context, &runtime->session))
        return 0;

    if (runtime->stats.application_service_calls == UINT32_MAX)
        return 0;

    runtime->stats.application_service_calls++;
    return 1;
}

static int h1_rfb_run(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_rfb_present_callback_t present,
    void *present_context,
    pstvnc_h1_rfb_service_callback_t service,
    void *service_context)
{
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
     * delegates to logical channel 1. No second physical recv/send occurs here.
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

    /*
     * Publication is permitted only after the parser has proven complete full
     * coverage and marked the authoritative framebuffer valid. The callback is
     * outside parser/transport ownership and may only observe this complete
     * state.
     */
    if (present != NULL) {
        if (!present(present_context, &runtime->framebuffer))
            goto fail;
        runtime->stats.initial_presentations = 1u;
    }

    h1_rfb_publish_diagnostic(runtime, transport);

    /*
     * Initial-frame completion itself is a protocol boundary. A very early Pi
     * quiesce request may therefore terminate cleanly here without manufacturing
     * an unnecessary incremental request or starting application-side work.
     */
    if (transport->rfb_quiesce_request_received != 0u) {
        if (h1_rfb_complete_quiesce_at_boundary(runtime, transport) == 1)
            return 0;
        goto fail;
    }

    /*
     * Ordinary application work begins only after a coherent initial desktop is
     * visible and only if shutdown has not already been requested. CP2L uses
     * this first call to establish the neutral published pointer and then start
     * its controller producer.
     */
    if (!h1_rfb_service_application(
            runtime,
            service,
            service_context))
        goto fail;

    if (!pstvnc_rfb_session_request_update(
            &runtime->session,
            1))
        goto fail;

    runtime->stats.incremental_requests_sent = 1u;

    for (;;) {
        pstvnc_rfb_session_receive_result_t receive_result;

        if (pstvnc_h1_transport_last_error(transport) !=
                PSTVNC_H1_ERROR_NONE ||
            transport->receiver_done ||
            transport->stop_requested)
            goto fail;

        receive_result = pstvnc_rfb_session_try_receive_update(
            &runtime->session,
            &runtime->framebuffer);

        if (receive_result == PSTVNC_RFB_SESSION_RECEIVE_FAILED)
            goto fail;

        if (receive_result == PSTVNC_RFB_SESSION_RECEIVE_IDLE) {
            if (runtime->stats.idle_polls != UINT32_MAX)
                runtime->stats.idle_polls++;

            h1_rfb_publish_diagnostic(runtime, transport);

            /*
             * IDLE is returned only before the parser consumes the next server
             * message, so it is already a clean RFB protocol boundary. This is
             * essential for a static desktop: an outstanding incremental
             * FramebufferUpdateRequest may legitimately have no response yet,
             * and shutdown must not wait forever for damage that never occurs.
             */
            if (transport->rfb_quiesce_request_received != 0u) {
                if (h1_rfb_complete_quiesce_at_boundary(
                        runtime,
                        transport) == 1)
                    return 0;
                goto fail;
            }

            /*
             * Drain ordinary application work while the server is idle. The
             * callback still runs on the RFB-owning main thread; the controller
             * producer only fills its semantic FIFO.
             */
            if (!h1_rfb_service_application(
                    runtime,
                    service,
                    service_context))
                goto fail;

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

        /*
         * Match the through-Issue-39 application presentation contract: a
         * completed incremental server message advances presentation only when
         * it actually dirtied the authoritative desktop. The parser remains the
         * sole authority for that dirty bit.
         */
        if (present != NULL && runtime->framebuffer.dirty) {
            if (!present(present_context, &runtime->framebuffer))
                goto fail;
            if (runtime->stats.incremental_presentations == UINT32_MAX)
                goto fail;
            runtime->stats.incremental_presentations++;
        }

        h1_rfb_publish_diagnostic(runtime, transport);

        /*
         * This is the decisive safe boundary after a completed update. If the Pi
         * requested shutdown at any point while that update was outstanding,
         * stop here and do not publish later input or send another framebuffer
         * request.
         */
        if (transport->rfb_quiesce_request_received != 0u) {
            if (h1_rfb_complete_quiesce_at_boundary(runtime, transport) == 1)
                return 0;
            goto fail;
        }

        if (!h1_rfb_service_application(
                runtime,
                service,
                service_context))
            goto fail;

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

int pstvnc_h1_rfb_session_runtime_run(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport)
{
    return h1_rfb_run(
        runtime,
        transport,
        NULL,
        NULL,
        NULL,
        NULL);
}

int pstvnc_h1_rfb_session_runtime_run_with_presenter(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_rfb_present_callback_t present,
    void *present_context)
{
    if (present == NULL)
        return -1;

    return h1_rfb_run(
        runtime,
        transport,
        present,
        present_context,
        NULL,
        NULL);
}

int pstvnc_h1_rfb_session_runtime_run_with_presenter_and_service(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_rfb_present_callback_t present,
    void *present_context,
    pstvnc_h1_rfb_service_callback_t service,
    void *service_context)
{
    if (present == NULL || service == NULL)
        return -1;

    return h1_rfb_run(
        runtime,
        transport,
        present,
        present_context,
        service,
        service_context);
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
