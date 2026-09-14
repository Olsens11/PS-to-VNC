/*
 * File synopsis:
 * Runs the unchanged through-Issue-39 RFB session/parser against H1 logical
 * channel 1 and owns the PS2 half of clean finite-session quiescence.
 *
 * CP2J qualified this coordinator headlessly and CP2K qualified its complete-
 * framebuffer presentation callback. Optional service and flow-policy seams let
 * later experiment layers act only at complete RFB message boundaries without
 * moving RFB serialization into the controller worker or parser.
 */

#include "h1_rfb_session_runtime.h"

#include "display.h"
#include "h1_config.h"
#include "h1_rfb_transport_live.h"
#include "h1_rfb_mux_io.h"
#include "h1_transport_runtime.h"


#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define H1_RFB_IDLE_DELAY_US 1000u
#define H1_RFB_QUIESCE_WAIT_US 1000u
#define H1_RFB_DIAGNOSTIC_MARKER 0xA0000000u

/*
 * Observation-only post-IDLE control-flow witnesses.
 *
 * If the RFB thread stops making progress, the last marker tells us which
 * call was entered but did not return.
 */
#define H1_RFB_DIAG_APP_SERVICE_ENTER    0xE1050001u
#define H1_RFB_DIAG_APP_SERVICE_RETURN   0xE1050002u
#define H1_RFB_DIAG_REQUEST_ENTER        0xE1050003u
#define H1_RFB_DIAG_REQUEST_RETURN       0xE1050004u
#define H1_RFB_DIAG_IDLE_DELAY_ENTER     0xE1050005u
#define H1_RFB_DIAG_IDLE_DELAY_RETURN    0xE1050006u

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
    uint32_t activity_sequence;

    if (!pstvnc_h1_rfb_transport_activity_snapshot(
            transport,
            &activity_sequence))
        return 0;

    while (transport->rfb_quiesce_commit_received == 0u) {
        if (pstvnc_h1_transport_last_error(transport) !=
                PSTVNC_H1_ERROR_NONE ||
            transport->receiver_done ||
            transport->stop_requested)
            return 0;

        h1_rfb_publish_diagnostic(runtime, transport);

        if (!pstvnc_h1_rfb_transport_wait_for_activity(
                transport,
                &activity_sequence))
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

    if (!pstvnc_h1_rfb_transport_send_quiesce_boundary(transport))
        return -1;

    runtime->stats.quiesce_boundary_sent = 1u;
    h1_rfb_publish_diagnostic(runtime, transport);

    if (!h1_rfb_wait_for_commit(runtime, transport))
        return -1;

    if (!pstvnc_h1_rfb_transport_snapshot(
            transport,
            &snapshot))
        return -1;

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

    if (!service(service_context, &runtime->session))
        return 0;

    if (runtime->stats.application_service_calls == UINT32_MAX)
        return 0;

    runtime->stats.application_service_calls++;
    return 1;
}

static int h1_rfb_flow_policy_valid(
    const pstvnc_h1_rfb_flow_policy_t *flow_policy)
{
    if (flow_policy == NULL)
        return 1;

    return flow_policy->next_request != NULL &&
        flow_policy->request_sent != NULL &&
        flow_policy->update_complete != NULL &&
        flow_policy->allow_present != NULL;
}

static int h1_rfb_request_at_boundary(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    const pstvnc_h1_rfb_flow_policy_t *flow_policy)
{
    pstvnc_h1_rfb_request_policy_decision_t decision;
    int incremental;

    if (runtime == NULL)
        return 0;

    decision = PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL;
    if (flow_policy != NULL)
        decision = flow_policy->next_request(flow_policy->context);

    if (decision == PSTVNC_H1_RFB_REQUEST_POLICY_HOLD) {
        if (runtime->stats.held_request_boundaries != UINT32_MAX)
            runtime->stats.held_request_boundaries++;
        return 1;
    }

    if (decision == PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL) {
        incremental = 1;
    } else if (decision == PSTVNC_H1_RFB_REQUEST_POLICY_FULL) {
        incremental = 0;
    } else {
        return 0;
    }

    if (!pstvnc_rfb_session_request_update(
            &runtime->session,
            incremental))
        return 0;

    if (flow_policy != NULL &&
        !flow_policy->request_sent(
            flow_policy->context,
            decision))
        return 0;

    if (decision == PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL) {
        if (runtime->stats.incremental_requests_sent == UINT32_MAX)
            return 0;
        runtime->stats.incremental_requests_sent++;
    } else {
        if (runtime->stats.full_requests_sent == UINT32_MAX)
            return 0;
        runtime->stats.full_requests_sent++;
    }

    return 1;
}

static int h1_rfb_allow_present(
    const pstvnc_h1_rfb_flow_policy_t *flow_policy)
{
    if (flow_policy == NULL)
        return 1;

    return flow_policy->allow_present(flow_policy->context) ? 1 : 0;
}

static int h1_rfb_run(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_rfb_present_callback_t present,
    void *present_context,
    pstvnc_h1_rfb_service_callback_t service,
    void *service_context,
    const pstvnc_h1_rfb_flow_policy_t *flow_policy)
{
    uint32_t activity_sequence;

    if (runtime == NULL || transport == NULL ||
        transport->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        !transport->rfb_resources.active ||
        !h1_rfb_flow_policy_valid(flow_policy))
        return -1;

    pstvnc_h1_rfb_session_runtime_init(runtime);

    if (!h1_rfb_prepare_framebuffer(runtime))
        goto fail;

    h1_rfb_publish_diagnostic(runtime, transport);

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
     * Calibration cannot own foreground until application service begins below,
     * so the initial authoritative desktop remains the qualified unconditional
     * publication point. Flow policy begins with the first subsequent request.
     */
    if (present != NULL) {
        if (!present(present_context, &runtime->framebuffer))
            goto fail;
        runtime->stats.initial_presentations = 1u;
    }

    h1_rfb_publish_diagnostic(runtime, transport);

    if (transport->rfb_quiesce_request_received != 0u) {
        if (h1_rfb_complete_quiesce_at_boundary(runtime, transport) == 1)
            return 0;
        goto fail;
    }

    /*
     * Snapshot before application service. Input produced during or after
     * service therefore changes the sequence and prevents the following idle
     * wait from sleeping through semantic work.
     */
    if (!pstvnc_h1_rfb_transport_activity_snapshot(
            transport,
            &activity_sequence))
        goto fail;

    if (!h1_rfb_service_application(
            runtime,
            service,
            service_context))
        goto fail;

    if (!h1_rfb_request_at_boundary(runtime, flow_policy))
        goto fail;

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

            if (transport->rfb_quiesce_request_received != 0u) {
                if (h1_rfb_complete_quiesce_at_boundary(
                        runtime,
                        transport) == 1)
                    return 0;
                goto fail;
            }

            pstvnc_h1_rfb_mux_io_diag_stage(
                transport->socket_fd,
                H1_RFB_DIAG_APP_SERVICE_ENTER,
                0u);

            if (!h1_rfb_service_application(
                    runtime,
                    service,
                    service_context))
                goto fail;

            pstvnc_h1_rfb_mux_io_diag_stage(
                transport->socket_fd,
                H1_RFB_DIAG_APP_SERVICE_RETURN,
                0u);

            /*
             * Default/qualified callers do not issue requests from IDLE because
             * their one incremental request may still be outstanding. A flow
             * policy explicitly owns that fact and may therefore safely HOLD or
             * issue the first post-thaw FULL request here.
             */
            pstvnc_h1_rfb_mux_io_diag_stage(
                transport->socket_fd,
                H1_RFB_DIAG_REQUEST_ENTER,
                0u);

            if (flow_policy != NULL &&
                !h1_rfb_request_at_boundary(runtime, flow_policy))
                goto fail;

            pstvnc_h1_rfb_mux_io_diag_stage(
                transport->socket_fd,
                H1_RFB_DIAG_REQUEST_RETURN,
                0u);

            /*
             * Keep the established marker ids for witness compatibility. The
             * old timer delay between them is now a producer-driven event wait.
             */
            pstvnc_h1_rfb_mux_io_diag_stage(
                transport->socket_fd,
                H1_RFB_DIAG_IDLE_DELAY_ENTER,
                0u);

            if (!pstvnc_h1_rfb_transport_wait_for_activity(
                    transport,
                    &activity_sequence))
                goto fail;

            pstvnc_h1_rfb_mux_io_diag_stage(
                transport->socket_fd,
                H1_RFB_DIAG_IDLE_DELAY_RETURN,
                0u);

            continue;
        }

        if (receive_result != PSTVNC_RFB_SESSION_RECEIVE_UPDATE ||
            !runtime->framebuffer.valid)
            goto fail;

        if (runtime->stats.incremental_updates_complete == UINT32_MAX)
            goto fail;

        runtime->stats.incremental_updates_complete++;

        /*
         * Protocol ownership is discharged before visual publication policy is
         * consulted. An update requested before calibration entry therefore
         * still advances parser/framebuffer state and clears request ownership
         * even when its dirty pixels are deliberately not presented.
         */
        if (flow_policy != NULL &&
            !flow_policy->update_complete(flow_policy->context))
            goto fail;

        if (present != NULL && runtime->framebuffer.dirty) {
            if (h1_rfb_allow_present(flow_policy)) {
                if (!present(present_context, &runtime->framebuffer))
                    goto fail;
                if (runtime->stats.incremental_presentations == UINT32_MAX)
                    goto fail;
                runtime->stats.incremental_presentations++;
            } else {
                if (runtime->stats.suppressed_presentations == UINT32_MAX)
                    goto fail;
                runtime->stats.suppressed_presentations++;
            }
        }

        h1_rfb_publish_diagnostic(runtime, transport);

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

        if (!h1_rfb_request_at_boundary(runtime, flow_policy))
            goto fail;
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
        service_context,
        NULL);
}

int pstvnc_h1_rfb_session_runtime_run_with_flow_policy(
    pstvnc_h1_rfb_session_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_rfb_present_callback_t present,
    void *present_context,
    pstvnc_h1_rfb_service_callback_t service,
    void *service_context,
    const pstvnc_h1_rfb_flow_policy_t *flow_policy)
{
    if (present == NULL || service == NULL || flow_policy == NULL ||
        !h1_rfb_flow_policy_valid(flow_policy))
        return -1;

    return h1_rfb_run(
        runtime,
        transport,
        present,
        present_context,
        service,
        service_context,
        flow_policy);
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
