/*
 * File synopsis:
 * Implements the generic RFB safe-boundary freeze/request/full-refresh policy.
 *
 * The policy is intentionally ignorant of why a caller freezes RFB. It retains
 * no MPEG, calibration, decoder, producer, GS, presentation-owner, socket,
 * parser, or framebuffer state.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md, A004-RFB-FREEZE-REFRESH-R2;
 *   docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md, Q7.
 */

#include "flow_policy.h"

#include <stddef.h>
#include <string.h>

void pstvnc_rfb_flow_policy_init(
    pstvnc_rfb_flow_policy_t *policy)
{
    if (policy == NULL)
        return;

    memset(policy, 0, sizeof(*policy));
}

int pstvnc_rfb_flow_policy_set_frozen(
    pstvnc_rfb_flow_policy_t *policy,
    int frozen)
{
    unsigned int new_frozen;

    if (policy == NULL || (frozen != 0 && frozen != 1))
        return 0;

    new_frozen = (unsigned int)frozen;

    if (policy->frozen == new_frozen)
        return 1;

    if (policy->frozen && !new_frozen)
        policy->full_refresh_pending = 1u;

    policy->frozen = new_frozen;
    return 1;
}

pstvnc_rfb_flow_request_t
pstvnc_rfb_flow_policy_next_request(
    const pstvnc_rfb_flow_policy_t *policy)
{
    if (policy == NULL ||
        policy->frozen ||
        policy->request_outstanding)
        return PSTVNC_RFB_FLOW_REQUEST_HOLD;

    if (policy->full_refresh_pending)
        return PSTVNC_RFB_FLOW_REQUEST_FULL;

    return PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL;
}

int pstvnc_rfb_flow_policy_record_request_sent(
    pstvnc_rfb_flow_policy_t *policy,
    pstvnc_rfb_flow_request_t request)
{
    pstvnc_rfb_flow_request_t expected;

    if (policy == NULL)
        return 0;

    expected = pstvnc_rfb_flow_policy_next_request(policy);

    if (request == PSTVNC_RFB_FLOW_REQUEST_HOLD ||
        request != expected)
        return 0;

    if (request != PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL &&
        request != PSTVNC_RFB_FLOW_REQUEST_FULL)
        return 0;

    if (request == PSTVNC_RFB_FLOW_REQUEST_FULL)
        policy->full_refresh_pending = 0u;

    policy->request_outstanding = 1u;
    return 1;
}

int pstvnc_rfb_flow_policy_record_update_complete(
    pstvnc_rfb_flow_policy_t *policy)
{
    if (policy == NULL || !policy->request_outstanding)
        return 0;

    policy->request_outstanding = 0u;
    return 1;
}

int pstvnc_rfb_flow_policy_has_outstanding_request(
    const pstvnc_rfb_flow_policy_t *policy)
{
    return policy != NULL && policy->request_outstanding;
}

int pstvnc_rfb_flow_policy_allows_remote_publication(
    const pstvnc_rfb_flow_policy_t *policy)
{
    return policy != NULL && !policy->frozen;
}
