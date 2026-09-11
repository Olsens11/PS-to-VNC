/*
 * File synopsis:
 * Host contract test for H1's experiment-local MPEG calibration foreground
 * ownership bridge.
 */

#include "h1_mpeg_calibration_foreground.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct foreground_test_ops_state {
    char calls[16];
    unsigned int call_count;
    int fail_suspend;
    int fail_neutralize;
    int fail_rebase;
    int fail_resume;
} foreground_test_ops_state_t;

static int record_call(
    foreground_test_ops_state_t *state,
    char call,
    int should_fail)
{
    assert(state != NULL);
    assert(state->call_count < sizeof(state->calls));

    state->calls[state->call_count++] = call;
    return should_fail ? 0 : 1;
}

static int test_suspend(void *context)
{
    foreground_test_ops_state_t *state = context;
    return record_call(state, 'S', state->fail_suspend);
}

static int test_neutralize(void *context)
{
    foreground_test_ops_state_t *state = context;
    return record_call(state, 'N', state->fail_neutralize);
}

static int test_rebase(void *context)
{
    foreground_test_ops_state_t *state = context;
    return record_call(state, 'B', state->fail_rebase);
}

static int test_resume(void *context)
{
    foreground_test_ops_state_t *state = context;
    return record_call(state, 'R', state->fail_resume);
}

static pstvnc_h1_mpeg_calibration_foreground_ops_t test_ops(void)
{
    pstvnc_h1_mpeg_calibration_foreground_ops_t ops;

    ops.suspend_mouse_interpretation = test_suspend;
    ops.neutralize_published_pointer = test_neutralize;
    ops.rebase_suspended_mouse_state = test_rebase;
    ops.resume_mouse_interpretation = test_resume;
    return ops;
}

static pstvnc_controller_state_t controller(
    uint16_t down,
    uint16_t pressed,
    uint16_t released)
{
    pstvnc_controller_state_t state;

    memset(&state, 0, sizeof(state));
    state.buttons_down = down;
    state.buttons_pressed = pressed;
    state.buttons_released = released;
    return state;
}

static void test_ordinary_select_does_not_claim_foreground(void)
{
    pstvnc_h1_mpeg_calibration_foreground_t foreground;
    pstvnc_h1_mpeg_calibration_foreground_result_t result;
    pstvnc_h1_mpeg_calibration_foreground_ops_t ops = test_ops();
    foreground_test_ops_state_t ops_state = {{0}, 0, 0, 0, 0, 0};
    pstvnc_controller_state_t state = controller(
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);

    pstvnc_h1_mpeg_calibration_foreground_init(&foreground, 704, 462);

    assert(pstvnc_h1_mpeg_calibration_foreground_service_controller(
        &foreground, &state, &ops, &ops_state, &result));
    assert(!result.consume_controller_state);
    assert(!result.freeze_rfb_visuals);
    assert(ops_state.call_count == 0);
}

static void test_entry_orders_qualified_ownership_sequence(void)
{
    pstvnc_h1_mpeg_calibration_foreground_t foreground;
    pstvnc_h1_mpeg_calibration_foreground_result_t result;
    pstvnc_h1_mpeg_calibration_foreground_ops_t ops = test_ops();
    foreground_test_ops_state_t ops_state = {{0}, 0, 0, 0, 0, 0};
    pstvnc_controller_state_t state = controller(
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);

    pstvnc_h1_mpeg_calibration_foreground_init(&foreground, 704, 462);

    assert(pstvnc_h1_mpeg_calibration_foreground_service_controller(
        &foreground, &state, &ops, &ops_state, &result));

    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);
    assert(foreground.mouse_interpretation_suspended);
    assert(foreground.rfb_visuals_frozen);
    assert(ops_state.call_count == 3);
    assert(ops_state.calls[0] == 'S');
    assert(ops_state.calls[1] == 'N');
    assert(ops_state.calls[2] == 'B');
}

static void test_cancel_stays_frozen_until_full_release(void)
{
    pstvnc_h1_mpeg_calibration_foreground_t foreground;
    pstvnc_h1_mpeg_calibration_foreground_result_t result;
    pstvnc_h1_mpeg_calibration_foreground_ops_t ops = test_ops();
    foreground_test_ops_state_t ops_state = {{0}, 0, 0, 0, 0, 0};
    pstvnc_controller_state_t state;

    pstvnc_h1_mpeg_calibration_foreground_init(&foreground, 704, 462);

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);
    assert(pstvnc_h1_mpeg_calibration_foreground_service_controller(
        &foreground, &state, &ops, &ops_state, &result));

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT);
    assert(pstvnc_h1_mpeg_calibration_foreground_service_controller(
        &foreground, &state, &ops, &ops_state, &result));
    assert(result.adapter_result.calibration_effects.cancelled);
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);
    assert(ops_state.call_count == 3);

    /* Held cancel remains quarantined and cannot resume desktop input. */
    state = controller(PSTVNC_CONTROLLER_BUTTON_CIRCLE, 0, 0);
    assert(pstvnc_h1_mpeg_calibration_foreground_service_controller(
        &foreground, &state, &ops, &ops_state, &result));
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);
    assert(ops_state.call_count == 3);

    /* Full physical release is the only resume boundary. */
    state = controller(0, 0, PSTVNC_CONTROLLER_BUTTON_CIRCLE);
    assert(pstvnc_h1_mpeg_calibration_foreground_service_controller(
        &foreground, &state, &ops, &ops_state, &result));
    assert(result.adapter_result.calibration_effects.release_quarantine_complete);
    assert(result.consume_controller_state);
    assert(!result.freeze_rfb_visuals);
    assert(!foreground.mouse_interpretation_suspended);
    assert(!foreground.rfb_visuals_frozen);
    assert(ops_state.call_count == 4);
    assert(ops_state.calls[3] == 'R');
}

static void test_entry_failure_fails_closed(void)
{
    pstvnc_h1_mpeg_calibration_foreground_t foreground;
    pstvnc_h1_mpeg_calibration_foreground_result_t result;
    pstvnc_h1_mpeg_calibration_foreground_ops_t ops = test_ops();
    foreground_test_ops_state_t ops_state = {{0}, 0, 0, 0, 1, 0};
    pstvnc_controller_state_t state = controller(
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);

    pstvnc_h1_mpeg_calibration_foreground_init(&foreground, 704, 462);

    assert(!pstvnc_h1_mpeg_calibration_foreground_service_controller(
        &foreground, &state, &ops, &ops_state, &result));

    assert(foreground.rfb_visuals_frozen);
    assert(foreground.mouse_interpretation_suspended);
    assert(ops_state.call_count == 3);
    assert(ops_state.calls[0] == 'S');
    assert(ops_state.calls[1] == 'N');
    assert(ops_state.calls[2] == 'B');
}

int main(void)
{
    test_ordinary_select_does_not_claim_foreground();
    test_entry_orders_qualified_ownership_sequence();
    test_cancel_stays_frozen_until_full_release();
    test_entry_failure_fails_closed();

    puts("MPEG_CALIBRATION_FOREGROUND_HOST_TEST=PASS");
    return 0;
}
