/*
 * File synopsis:
 * Strict host contract for CP2P composition of calibration RFB gating with the
 * accepted-region/first-frame MPEG presentation owner.
 */
#include "h1_mpeg_cp2p_rfb_flow.h"

#include <assert.h>
#include <stdio.h>

static pstvnc_mpeg_cal_region_t test_region(void)
{
    pstvnc_mpeg_cal_region_t region;

    region.x = 160;
    region.y = 96;
    region.width = 352;
    region.height = 224;
    region.inner_matte_x = 8;
    region.inner_matte_y = 4;
    region.outer_matte_x = 12;
    region.outer_matte_y = 10;
    return region;
}

static pstvnc_mpeg_cal_rect_t test_suppression(void)
{
    pstvnc_mpeg_cal_rect_t rect;

    rect.x = 148;
    rect.y = 86;
    rect.width = 376;
    rect.height = 244;
    return rect;
}

static void mark_request_round_trip(
    const pstvnc_h1_rfb_flow_policy_t *policy,
    pstvnc_h1_rfb_request_policy_decision_t decision)
{
    assert(policy != NULL);
    assert(policy->request_sent(policy->context, decision));
    assert(policy->next_request(policy->context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_HOLD);
    assert(policy->update_complete(policy->context));
}

static void test_ordinary_rfb_uses_one_outstanding_request(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t calibration;
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;

    pstvnc_h1_mpeg_calibration_runtime_init(&calibration, 704, 462);
    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(&flow, &calibration, &owner);
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(&flow, &policy));

    assert(policy.allow_present(policy.context));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL);
    mark_request_round_trip(
        &policy,
        PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL);
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL);
}

static void test_accept_wait_first_frame_collapses_refresh_obligations(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t calibration;
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;
    pstvnc_mpeg_cal_region_t region = test_region();
    pstvnc_mpeg_cal_rect_t suppression = test_suppression();
    uint32_t generation = 0;

    pstvnc_h1_mpeg_calibration_runtime_init(&calibration, 704, 462);
    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(&flow, &calibration, &owner);
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(&flow, &policy));

    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
        &calibration.rfb_gate, 1));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_HOLD);
    assert(!policy.allow_present(policy.context));

    assert(pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &region, &suppression, &generation));
    assert(generation != 0u);
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
        &calibration.rfb_gate, 0));
    assert(calibration.rfb_gate.full_refresh_required);
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_HOLD);
    assert(calibration.rfb_gate.full_refresh_required);
    assert(!policy.allow_present(policy.context));

    assert(pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
        &owner, generation));
    assert(owner.full_refresh_pending);
    assert(policy.allow_present(policy.context));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(!calibration.rfb_gate.full_refresh_required);
    assert(!owner.full_refresh_pending);
    mark_request_round_trip(&policy, PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL);
}

static void test_stop_and_abort_each_require_one_full_refresh(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t calibration;
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;
    pstvnc_mpeg_cal_region_t region = test_region();
    pstvnc_mpeg_cal_rect_t suppression = test_suppression();
    uint32_t generation = 0;

    pstvnc_h1_mpeg_calibration_runtime_init(&calibration, 704, 462);
    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(&flow, &calibration, &owner);
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(&flow, &policy));

    assert(pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &region, &suppression, &generation));
    assert(pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
        &owner, generation));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    mark_request_round_trip(&policy, PSTVNC_H1_RFB_REQUEST_POLICY_FULL);

    assert(pstvnc_h1_mpeg_presentation_owner_stop(&owner, generation));
    assert(policy.allow_present(policy.context));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    mark_request_round_trip(&policy, PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_INCREMENTAL);

    assert(pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &region, &suppression, &generation));
    assert(!policy.allow_present(policy.context));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_HOLD);
    assert(pstvnc_h1_mpeg_presentation_owner_abort_start(
        &owner, generation));
    assert(policy.allow_present(policy.context));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    mark_request_round_trip(&policy, PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
}

static void test_stale_first_frame_cannot_thaw_owner(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t calibration;
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;
    pstvnc_mpeg_cal_region_t region = test_region();
    pstvnc_mpeg_cal_rect_t suppression = test_suppression();
    uint32_t generation = 0;

    pstvnc_h1_mpeg_calibration_runtime_init(&calibration, 704, 462);
    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(&flow, &calibration, &owner);
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(&flow, &policy));

    assert(pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &region, &suppression, &generation));
    assert(!pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
        &owner, generation + 1u));
    assert(!policy.allow_present(policy.context));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_HOLD);
}

int main(void)
{
    test_ordinary_rfb_uses_one_outstanding_request();
    test_accept_wait_first_frame_collapses_refresh_obligations();
    test_stop_and_abort_each_require_one_full_refresh();
    test_stale_first_frame_cannot_thaw_owner();

    puts("MPEG_CP2P_RFB_FLOW_HOST_TEST=PASS");
    return 0;
}
