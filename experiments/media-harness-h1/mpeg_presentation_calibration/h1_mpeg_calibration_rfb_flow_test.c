/*
 * File synopsis:
 * Host contract for the adapter between calibration RFB ownership and the
 * generic H1 RFB session flow-policy seam.
 */
#include "h1_mpeg_calibration_rfb_flow.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t calibration;
    pstvnc_h1_mpeg_calibration_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;

    memset(&calibration, 0, sizeof(calibration));
    pstvnc_h1_mpeg_calibration_rfb_gate_init(&calibration.rfb_gate);

    pstvnc_h1_mpeg_calibration_rfb_flow_init(&flow, &calibration);
    assert(pstvnc_h1_mpeg_calibration_rfb_flow_prepare_policy(
        &flow,
        &policy));

    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL);
    assert(policy.request_sent(
        policy.context,
        PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL));

    /* One request remains protocol-owned until its complete response. */
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_HOLD);

    /* Calibration may claim foreground while that old request is in flight. */
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
        &calibration.rfb_gate,
        1));
    assert(!policy.allow_present(policy.context));

    /* Completion clears protocol ownership even though visual publish is off. */
    assert(policy.update_complete(policy.context));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_HOLD);

    /* Thaw converts the next available request into exactly one full refresh. */
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
        &calibration.rfb_gate,
        0));
    assert(policy.allow_present(policy.context));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(policy.request_sent(
        policy.context,
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL));

    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_HOLD);
    assert(policy.update_complete(policy.context));

    /* After the full response, normal incremental cadence resumes. */
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL);

    puts("MPEG_CALIBRATION_RFB_FLOW_HOST_TEST=PASS");
    return 0;
}
