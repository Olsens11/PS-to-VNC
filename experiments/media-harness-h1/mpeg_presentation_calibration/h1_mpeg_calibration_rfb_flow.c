/*
 * File synopsis:
 * Implements the narrow adapter between MPEG calibration request/presentation
 * policy and the generic H1 RFB session runtime flow-policy interface.
 */
#include "h1_mpeg_calibration_rfb_flow.h"

#include <string.h>

static pstvnc_h1_rfb_request_policy_decision_t flow_next_request(
    void *context)
{
    pstvnc_h1_mpeg_calibration_rfb_flow_t *flow =
        (pstvnc_h1_mpeg_calibration_rfb_flow_t *)context;
    pstvnc_h1_mpeg_calibration_rfb_request_t request;

    if (flow == NULL || flow->calibration_runtime == NULL)
        return PSTVNC_H1_RFB_REQUEST_POLICY_HOLD;

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &flow->schedule,
        &flow->calibration_runtime->rfb_gate);

    if (request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_INCREMENTAL)
        return PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL;
    if (request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_FULL)
        return PSTVNC_H1_RFB_REQUEST_POLICY_FULL;
    return PSTVNC_H1_RFB_REQUEST_POLICY_HOLD;
}

static int flow_request_sent(
    void *context,
    pstvnc_h1_rfb_request_policy_decision_t decision)
{
    pstvnc_h1_mpeg_calibration_rfb_flow_t *flow =
        (pstvnc_h1_mpeg_calibration_rfb_flow_t *)context;
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

static int flow_update_complete(void *context)
{
    pstvnc_h1_mpeg_calibration_rfb_flow_t *flow =
        (pstvnc_h1_mpeg_calibration_rfb_flow_t *)context;

    if (flow == NULL)
        return 0;

    return pstvnc_h1_mpeg_calibration_rfb_schedule_update_complete(
        &flow->schedule);
}

static int flow_allow_present(void *context)
{
    const pstvnc_h1_mpeg_calibration_rfb_flow_t *flow =
        (const pstvnc_h1_mpeg_calibration_rfb_flow_t *)context;

    if (flow == NULL || flow->calibration_runtime == NULL)
        return 0;

    return pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(
        flow->calibration_runtime);
}

void pstvnc_h1_mpeg_calibration_rfb_flow_init(
    pstvnc_h1_mpeg_calibration_rfb_flow_t *flow,
    pstvnc_h1_mpeg_calibration_runtime_t *calibration_runtime)
{
    if (flow == NULL)
        return;

    memset(flow, 0, sizeof(*flow));
    flow->calibration_runtime = calibration_runtime;
    pstvnc_h1_mpeg_calibration_rfb_schedule_init(&flow->schedule);
}

int pstvnc_h1_mpeg_calibration_rfb_flow_prepare_policy(
    pstvnc_h1_mpeg_calibration_rfb_flow_t *flow,
    pstvnc_h1_rfb_flow_policy_t *policy)
{
    if (flow == NULL || flow->calibration_runtime == NULL || policy == NULL)
        return 0;

    memset(policy, 0, sizeof(*policy));
    policy->next_request = flow_next_request;
    policy->request_sent = flow_request_sent;
    policy->update_complete = flow_update_complete;
    policy->allow_present = flow_allow_present;
    policy->context = flow;
    return 1;
}
