/*
 * File synopsis:
 * Strict host contract for the CP2P START+SELECT lifecycle:
 * - an existing MPEG generation is never edited in place;
 * - its immutable calibration settings survive as the next transaction seed;
 * - that generation is retired before calibration starts;
 * - calibration waits for a NEW post-retirement full RFB refresh;
 * - confirm creates a fresh generation, while cancel leaves MPEG absent.
 */
#include "h1_mpeg_recalibration.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static pstvnc_mpeg_cal_region_t first_region(void)
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

static pstvnc_mpeg_cal_input_t input(uint32_t down, uint32_t pressed)
{
    pstvnc_mpeg_cal_input_t value;

    memset(&value, 0, sizeof(value));
    value.down = down;
    value.pressed = pressed;
    return value;
}

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

static int region_equal(
    const pstvnc_mpeg_cal_region_t *left,
    const pstvnc_mpeg_cal_region_t *right)
{
    return left != NULL && right != NULL &&
        memcmp(left, right, sizeof(*left)) == 0;
}

static void complete_policy_request(
    const pstvnc_h1_rfb_flow_policy_t *policy,
    pstvnc_h1_rfb_request_policy_decision_t decision)
{
    assert(policy != NULL);
    assert(policy->request_sent(policy->context, decision));
    assert(policy->update_complete(policy->context));
}

static void test_rfb_only_enters_immediately(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t runtime;
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_mpeg_recalibration_t recalibration;
    pstvnc_h1_mpeg_recalibration_begin_result_t result;

    pstvnc_h1_mpeg_calibration_runtime_init(&runtime, 704, 462);
    pstvnc_h1_mpeg_start_handoff_init(&handoff, 704, 462);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(
        &flow, &runtime, &handoff.owner);
    pstvnc_h1_mpeg_recalibration_init(&recalibration);

    assert(pstvnc_h1_mpeg_recalibration_begin(
        &recalibration,
        &handoff,
        &runtime.foreground.adapter.calibration,
        &flow,
        NULL,
        NULL,
        &result));
    assert(result == PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW);
    assert(!recalibration.awaiting_restored_full_rfb);
}

static void test_active_generation_retires_and_seeds_fresh_calibration(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t runtime;
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;
    pstvnc_h1_mpeg_recalibration_t recalibration;
    pstvnc_h1_mpeg_recalibration_begin_result_t begin_result;
    pstvnc_h1_mpeg_start_contract_t first_start;
    pstvnc_h1_mpeg_start_contract_t second_start;
    pstvnc_mpeg_calibration_t *calibration;
    pstvnc_mpeg_cal_region_t first = first_region();
    pstvnc_mpeg_cal_region_t second = first;
    pstvnc_mpeg_cal_effects_t effects;
    pstvnc_mpeg_cal_input_t sample;
    clear_probe_t clear_probe;

    memset(&clear_probe, 0, sizeof(clear_probe));
    pstvnc_h1_mpeg_calibration_runtime_init(&runtime, 704, 462);
    calibration = &runtime.foreground.adapter.calibration;
    pstvnc_h1_mpeg_start_handoff_init(&handoff, 704, 462);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(
        &flow, &runtime, &handoff.owner);
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(&flow, &policy));
    pstvnc_h1_mpeg_recalibration_init(&recalibration);

    /* Make calibration's prior durable value intentionally different. */
    calibration->committed = calibration->defaults;
    calibration->has_committed = 1;

    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &first, &first_start));
    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(
        &handoff, first_start.generation));

    /* Consume the first-frame restoration so it cannot satisfy recalibration. */
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    complete_policy_request(&policy, PSTVNC_H1_RFB_REQUEST_POLICY_FULL);

    assert(pstvnc_h1_mpeg_recalibration_begin(
        &recalibration,
        &handoff,
        calibration,
        &flow,
        clear_mpeg,
        &clear_probe,
        &begin_result));
    assert(begin_result == PSTVNC_H1_MPEG_RECALIBRATION_WAIT_FOR_FULL_RFB);
    assert(clear_probe.calls == 1u);
    assert(clear_probe.generation == first_start.generation);
    assert(pstvnc_h1_mpeg_presentation_owner_state(&handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(region_equal(&calibration->committed, &first));
    assert(calibration->has_committed);
    assert(recalibration.awaiting_restored_full_rfb);
    assert(!pstvnc_h1_mpeg_recalibration_take_entry_ready(
        &recalibration, &flow));

    /* Retirement creates the required NEW full RFB restoration. */
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(policy.request_sent(
        policy.context, PSTVNC_H1_RFB_REQUEST_POLICY_FULL));
    assert(!pstvnc_h1_mpeg_recalibration_take_entry_ready(
        &recalibration, &flow));
    assert(policy.update_complete(policy.context));
    assert(pstvnc_h1_mpeg_recalibration_take_entry_ready(
        &recalibration, &flow));
    assert(!pstvnc_h1_mpeg_recalibration_take_entry_ready(
        &recalibration, &flow));

    /* Entering the new transaction reuses the retired generation's settings. */
    sample = input(
        PSTVNC_MPEG_CAL_START | PSTVNC_MPEG_CAL_SELECT,
        0);
    effects = pstvnc_mpeg_calibration_update(calibration, &sample);
    assert(effects.enter_foreground);
    assert(region_equal(&calibration->current, &first));

    /* Change geometry/mattes and confirm: this becomes a new generation. */
    second.x += 16;
    second.y += 8;
    second.width -= 16;
    second.height -= 16;
    second.inner_matte_x = 4;
    second.inner_matte_y = 2;
    second.outer_matte_x = 6;
    second.outer_matte_y = 5;
    calibration->current = second;

    sample = input(PSTVNC_MPEG_CAL_CROSS, PSTVNC_MPEG_CAL_CROSS);
    effects = pstvnc_mpeg_calibration_update(calibration, &sample);
    assert(!effects.accepted);
    assert(calibration->screen == PSTVNC_MPEG_CAL_REVIEW);

    sample = input(0, 0);
    (void)pstvnc_mpeg_calibration_update(calibration, &sample);
    assert(calibration->review_cross_armed);

    sample = input(PSTVNC_MPEG_CAL_CROSS, PSTVNC_MPEG_CAL_CROSS);
    effects = pstvnc_mpeg_calibration_update(calibration, &sample);
    assert(effects.accepted);
    assert(region_equal(&calibration->committed, &second));

    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &calibration->committed, &second_start));
    assert(second_start.generation != first_start.generation);
    assert(second_start.draw_x == second.x);
    assert(second_start.draw_y == second.y);
    assert(second_start.draw_width == second.width);
    assert(second_start.draw_height == second.height);
    assert(second_start.inner_matte_x == second.inner_matte_x);
    assert(second_start.inner_matte_y == second.inner_matte_y);
}

static void test_cancel_after_retirement_leaves_no_mpeg_generation(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t runtime;
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t flow;
    pstvnc_h1_rfb_flow_policy_t policy;
    pstvnc_h1_mpeg_recalibration_t recalibration;
    pstvnc_h1_mpeg_recalibration_begin_result_t begin_result;
    pstvnc_h1_mpeg_start_contract_t start;
    pstvnc_mpeg_calibration_t *calibration;
    pstvnc_mpeg_cal_region_t region = first_region();
    pstvnc_mpeg_cal_effects_t effects;
    pstvnc_mpeg_cal_input_t sample;
    clear_probe_t clear_probe;

    memset(&clear_probe, 0, sizeof(clear_probe));
    pstvnc_h1_mpeg_calibration_runtime_init(&runtime, 704, 462);
    calibration = &runtime.foreground.adapter.calibration;
    pstvnc_h1_mpeg_start_handoff_init(&handoff, 704, 462);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(
        &flow, &runtime, &handoff.owner);
    assert(pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(&flow, &policy));
    pstvnc_h1_mpeg_recalibration_init(&recalibration);

    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &region, &start));
    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(
        &handoff, start.generation));
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    complete_policy_request(&policy, PSTVNC_H1_RFB_REQUEST_POLICY_FULL);

    assert(pstvnc_h1_mpeg_recalibration_begin(
        &recalibration,
        &handoff,
        calibration,
        &flow,
        clear_mpeg,
        &clear_probe,
        &begin_result));
    assert(begin_result == PSTVNC_H1_MPEG_RECALIBRATION_WAIT_FOR_FULL_RFB);
    assert(policy.next_request(policy.context) ==
        PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    complete_policy_request(&policy, PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(pstvnc_h1_mpeg_recalibration_take_entry_ready(
        &recalibration, &flow));

    sample = input(
        PSTVNC_MPEG_CAL_START | PSTVNC_MPEG_CAL_SELECT,
        0);
    effects = pstvnc_mpeg_calibration_update(calibration, &sample);
    assert(effects.enter_foreground);
    assert(region_equal(&calibration->current, &region));

    sample = input(PSTVNC_MPEG_CAL_CIRCLE, PSTVNC_MPEG_CAL_CIRCLE);
    effects = pstvnc_mpeg_calibration_update(calibration, &sample);
    assert(effects.cancelled);
    assert(calibration->screen == PSTVNC_MPEG_CAL_INACTIVE);
    assert(pstvnc_h1_mpeg_presentation_owner_state(&handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(region_equal(&calibration->committed, &region));
}

int main(void)
{
    test_rfb_only_enters_immediately();
    test_active_generation_retires_and_seeds_fresh_calibration();
    test_cancel_after_retirement_leaves_no_mpeg_generation();

    puts("H1_MPEG_RECALIBRATION_HOST_TEST=PASS");
    return 0;
}
