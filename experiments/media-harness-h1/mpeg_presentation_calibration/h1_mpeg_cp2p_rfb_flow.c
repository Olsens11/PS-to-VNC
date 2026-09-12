/*
 * File synopsis:
 * Implements the CP2P-only composition of calibration RFB gating and MPEG
 * presentation ownership over one framebuffer-request scheduler.
 *
 * One scheduler remains authoritative for outstanding-request bookkeeping.
 * Calibration thaw and first-frame/stop owner refresh obligations are consumed
 * only when both owners permit a request; coincident obligations collapse into
 * one nonincremental refresh.
 */
#include "h1_mpeg_cp2p_rfb_flow.h"

#include <stddef.h>
#include <string.h>

static pstvnc_h1_rfb_request_policy_decision_t cp2p_next_request(
    void *context)
{
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow =
        (pstvnc_h1_mpeg_cp2p_rfb_flow_t *)context;
    int need_full_refresh;

    if (flow == NULL ||
        flow->calibration_runtime == NULL ||
        flow->presentation_owner == NULL)
        return PSTVNC_H1_RFB_REQUEST_POLICY_HOLD;

    if (pstvnc_h1_mpeg_calibration_rfb_schedule_has_outstanding_request(
            &flow->schedule))
        return PSTVNC_H1_RFB_REQUEST_POLICY_HOLD;

    /*
     * Do not consume either owner's one-shot refresh while the other owner is
     * still frozen. In particular, calibration may have thawed after Accept
     * while WAIT_FIRST_FRAME intentionally keeps the old desktop visible.
     */
    if (!pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(
            flow->calibration_runtime) ||
        !pstvnc_h1_mpeg_presentation_owner_allows_rfb_request(
            flow->presentation_owner))
        return PSTVNC_H1_RFB_REQUEST_POLICY_HOLD;

    need_full_refresh =
        pstvnc_h1_mpeg_calibration_runtime_take_full_refresh(
            flow->calibration_runtime);

    if (pstvnc_h1_mpeg_presentation_owner_take_full_refresh(
            flow->presentation_owner))
        need_full_refresh = 1;

    return need_full_refresh
        ? PSTVNC_H1_RFB_REQUEST_POLICY_FULL
        : PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL;
}

static int cp2p_request_sent(
    void *context,
    pstvnc_h1_rfb_request_policy_decision_t decision)
{
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow =
        (pstvnc_h1_mpeg_cp2p_rfb_flow_t *)context;
    pstvnc_h1_mpeg_calibration_rfb_request_t request;

    if (flow == NULL)
        return 0;

    if (decision == PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL) {
        request = PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_INCREMENTAL;
    } else if (decision == PSTVNC_H1_RFB_REQUEST_POLICY_FULL) {
        request = PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_FULL;
    } else {
        return 0;
    }

    return pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
        &flow->schedule,
        request);
}

static int cp2p_update_complete(void *context)
{
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow =
        (pstvnc_h1_mpeg_cp2p_rfb_flow_t *)context;

    if (flow == NULL)
        return 0;

    return pstvnc_h1_mpeg_calibration_rfb_schedule_update_complete(
        &flow->schedule);
}

static int cp2p_allow_present(void *context)
{
    const pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow =
        (const pstvnc_h1_mpeg_cp2p_rfb_flow_t *)context;

    if (flow == NULL ||
        flow->calibration_runtime == NULL ||
        flow->presentation_owner == NULL ||
        !pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(
            flow->calibration_runtime))
        return 0;

    return pstvnc_h1_mpeg_presentation_owner_remote_present_mode(
        flow->presentation_owner) !=
        PSTVNC_H1_MPEG_REMOTE_PRESENT_FROZEN_DESKTOP;
}

void pstvnc_h1_mpeg_cp2p_rfb_flow_init(
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow,
    pstvnc_h1_mpeg_calibration_runtime_t *calibration_runtime,
    pstvnc_h1_mpeg_presentation_owner_t *presentation_owner)
{
    if (flow == NULL)
        return;

    memset(flow, 0, sizeof(*flow));
    flow->calibration_runtime = calibration_runtime;
    flow->presentation_owner = presentation_owner;
    pstvnc_h1_mpeg_calibration_rfb_schedule_init(&flow->schedule);
}

int pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow,
    pstvnc_h1_rfb_flow_policy_t *policy)
{
    if (flow == NULL ||
        flow->calibration_runtime == NULL ||
        flow->presentation_owner == NULL ||
        policy == NULL)
        return 0;

    memset(policy, 0, sizeof(*policy));
    policy->next_request = cp2p_next_request;
    policy->request_sent = cp2p_request_sent;
    policy->update_complete = cp2p_update_complete;
    policy->allow_present = cp2p_allow_present;
    policy->context = flow;
    return 1;
}
