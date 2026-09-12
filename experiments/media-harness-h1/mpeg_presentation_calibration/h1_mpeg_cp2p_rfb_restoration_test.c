/*
 * File synopsis:
 * Strict host contract for CP2P's post-retirement full-RFB restoration watch.
 * The watch must ignore a FULL request already in flight when it is armed and
 * complete only after a NEW FULL request is sent and completed afterward.
 */
#include "h1_mpeg_cp2p_rfb_flow.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t calibration;
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;

    pstvnc_h1_mpeg_calibration_runtime_init(&calibration, 704, 462);
    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(&flow, &calibration, &owner);
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(&flow, &policy));

    /* Manufacture a legitimate FULL and leave it outstanding. */
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
        &calibration.rfb_gate, 1));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
        &calibration.rfb_gate, 0));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(policy.request_sent(
        policy.context, PSTVNC_H1_RFB_REQUEST_POLICY_FULL));

    /* START+SELECT retires MPEG here: this older FULL is not restoration. */
    pstvnc_h1_mpeg_cp2p_rfb_flow_arm_restoration_watch(&flow);
    assert(policy.update_complete(policy.context));
    assert(!pstvnc_h1_mpeg_cp2p_rfb_flow_take_restoration_complete(&flow));

    /* A NEW full request after the watch is armed is the eligible restoration. */
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
        &calibration.rfb_gate, 1));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
        &calibration.rfb_gate, 0));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(policy.request_sent(
        policy.context, PSTVNC_H1_RFB_REQUEST_POLICY_FULL));
    assert(!pstvnc_h1_mpeg_cp2p_rfb_flow_take_restoration_complete(&flow));
    assert(policy.update_complete(policy.context));
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_take_restoration_complete(&flow));
    assert(!pstvnc_h1_mpeg_cp2p_rfb_flow_take_restoration_complete(&flow));

    puts("H1_MPEG_CP2P_RFB_RESTORATION_HOST_TEST=PASS");
    return 0;
}
