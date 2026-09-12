/*
 * File synopsis:
 * Strict host contract for wiring the real 750 ms entry hold to CP2P's clean
 * retire -> new full RFB -> fresh calibration transaction lifecycle.
 */
#include "h1_mpeg_recalibration_entry_bridge.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct clear_probe {
    unsigned calls;
    uint32_t generation;
} clear_probe_t;

static int clear_mpeg(void *context, uint32_t generation)
{
    clear_probe_t *probe = (clear_probe_t *)context;

    if (probe == NULL || generation == 0u)
        return 0;

    probe->calls++;
    probe->generation = generation;
    return 1;
}

static pstvnc_mpeg_cal_region_t test_region(void)
{
    pstvnc_mpeg_cal_region_t region;

    memset(&region, 0, sizeof(region));
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

static void init_fixture(
    pstvnc_h1_mpeg_calibration_runtime_t *runtime,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow,
    pstvnc_h1_rfb_flow_policy_t *policy,
    pstvnc_h1_mpeg_recalibration_t *recalibration,
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    pstvnc_h1_mpeg_recalibration_entry_bridge_t *bridge,
    clear_probe_t *clear_probe)
{
    pstvnc_h1_mpeg_calibration_runtime_init(runtime, 704, 462);
    pstvnc_h1_mpeg_start_handoff_init(handoff, 704, 462);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(flow, runtime, &handoff->owner);
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(flow, policy));
    pstvnc_h1_mpeg_recalibration_init(recalibration);
    pstvnc_h1_mpeg_calibration_entry_hold_init(hold);
    pstvnc_h1_mpeg_recalibration_entry_bridge_init(bridge);
    memset(clear_probe, 0, sizeof(*clear_probe));

    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_attach(
        bridge,
        hold,
        handoff,
        &runtime->foreground.adapter.calibration,
        flow,
        recalibration,
        clear_mpeg,
        clear_probe));
}

static void test_rfb_only_hold_enters_without_deferral(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t runtime;
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;
    pstvnc_h1_mpeg_recalibration_t recalibration;
    pstvnc_h1_mpeg_calibration_entry_hold_t hold;
    pstvnc_h1_mpeg_recalibration_entry_bridge_t bridge;
    clear_probe_t clear_probe;
    int consume = 0;
    int activate = 0;

    init_fixture(
        &runtime, &handoff, &flow, &policy, &recalibration,
        &hold, &bridge, &clear_probe);

    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_observe(
        &hold, 100u, 1, 1, &consume));
    assert(consume);

    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_poll(
        &hold,
        100u + PSTVNC_H1_MPEG_CALIBRATION_ENTRY_HOLD_US,
        1,
        &activate));
    assert(activate);
    assert(!recalibration.awaiting_restored_full_rfb);
    assert(clear_probe.calls == 0u);

    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_detach(&bridge));
}

static void test_mpeg_hold_retires_then_reissues_activation_after_new_full(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t runtime;
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;
    pstvnc_h1_mpeg_recalibration_t recalibration;
    pstvnc_h1_mpeg_calibration_entry_hold_t hold;
    pstvnc_h1_mpeg_recalibration_entry_bridge_t bridge;
    pstvnc_h1_mpeg_start_contract_t start;
    pstvnc_mpeg_cal_region_t region = test_region();
    clear_probe_t clear_probe;
    int consume = 0;
    int activate = 0;

    init_fixture(
        &runtime, &handoff, &flow, &policy, &recalibration,
        &hold, &bridge, &clear_probe);

    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &region, &start));
    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(
        &handoff, start.generation));

    /* Consume the first-frame refresh so recalibration must create a new one. */
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(policy.request_sent(
        policy.context, PSTVNC_H1_RFB_REQUEST_POLICY_FULL));
    assert(policy.update_complete(policy.context));

    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_observe(
        &hold, 1000u, 1, 1, &consume));
    assert(consume);
    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_poll(
        &hold,
        1000u + PSTVNC_H1_MPEG_CALIBRATION_ENTRY_HOLD_US,
        1,
        &activate));

    assert(!activate);
    assert(recalibration.awaiting_restored_full_rfb);
    assert(clear_probe.calls == 1u);
    assert(clear_probe.generation == start.generation);
    assert(pstvnc_h1_mpeg_presentation_owner_state(&handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(runtime.foreground.adapter.calibration.has_committed);
    assert(memcmp(
        &runtime.foreground.adapter.calibration.committed,
        &region,
        sizeof(region)) == 0);

    /* Controller release/other samples remain owned during restoration. */
    consume = 0;
    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_observe(
        &hold, 900000u, 1, 0, &consume));
    assert(consume);

    /* A new post-retirement FULL must complete before activation is re-issued. */
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(policy.request_sent(
        policy.context, PSTVNC_H1_RFB_REQUEST_POLICY_FULL));
    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_poll(
        &hold, 900001u, 1, &activate));
    assert(!activate);
    assert(policy.update_complete(policy.context));

    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_poll(
        &hold, 900002u, 1, &activate));
    assert(activate);
    assert(!recalibration.awaiting_restored_full_rfb);
    assert(!pstvnc_h1_mpeg_recalibration_entry_bridge_poll(
        NULL, 0u, 0, NULL));

    assert(pstvnc_h1_mpeg_recalibration_entry_bridge_detach(&bridge));
}

int main(void)
{
    test_rfb_only_hold_enters_without_deferral();
    test_mpeg_hold_retires_then_reissues_activation_after_new_full();

    puts("H1_MPEG_RECALIBRATION_ENTRY_BRIDGE_HOST_TEST=PASS");
    return 0;
}
