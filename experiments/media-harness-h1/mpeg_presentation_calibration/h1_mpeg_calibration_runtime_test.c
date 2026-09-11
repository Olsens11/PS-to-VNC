/*
 * File synopsis:
 * Host contract test for the experiment-local MPEG calibration runtime seam.
 */
#include "h1_mpeg_calibration_runtime.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct runtime_test_ops_state {
    char calls[16];
    unsigned int call_count;
} runtime_test_ops_state_t;

static int record(runtime_test_ops_state_t *state, char call)
{
    assert(state != NULL);
    assert(state->call_count < sizeof(state->calls));
    state->calls[state->call_count++] = call;
    return 1;
}

static int test_suspend(void *context)
{
    return record((runtime_test_ops_state_t *)context, 'S');
}

static int test_neutralize(void *context)
{
    return record((runtime_test_ops_state_t *)context, 'N');
}

static int test_rebase(void *context)
{
    return record((runtime_test_ops_state_t *)context, 'B');
}

static int test_resume(void *context)
{
    return record((runtime_test_ops_state_t *)context, 'R');
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

static void test_entry_freezes_gate_and_exposes_render_plan(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t runtime;
    pstvnc_h1_mpeg_calibration_runtime_result_t result;
    pstvnc_h1_mpeg_calibration_render_plan_t plan;
    pstvnc_h1_mpeg_calibration_foreground_ops_t ops = test_ops();
    runtime_test_ops_state_t ops_state = {{0}, 0};
    pstvnc_controller_state_t state;

    pstvnc_h1_mpeg_calibration_runtime_init(&runtime, 704, 462);

    assert(pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(&runtime));
    assert(pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(&runtime));
    assert(!pstvnc_h1_mpeg_calibration_runtime_take_full_refresh(&runtime));

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);

    assert(pstvnc_h1_mpeg_calibration_runtime_service_controller(
        &runtime, &state, &ops, &ops_state, &result));
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);
    assert(result.calibration_visible);
    assert(!pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(&runtime));
    assert(!pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(&runtime));
    assert(!pstvnc_h1_mpeg_calibration_runtime_take_full_refresh(&runtime));
    assert(ops_state.call_count == 3);
    assert(ops_state.calls[0] == 'S');
    assert(ops_state.calls[1] == 'N');
    assert(ops_state.calls[2] == 'B');

    assert(pstvnc_h1_mpeg_calibration_runtime_prepare_render_plan(
        &runtime, &plan));
    assert(plan.kind == PSTVNC_H1_MPEG_CAL_RENDER_EDIT);
}

static void test_cancel_thaws_only_after_release_and_requests_full_refresh(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t runtime;
    pstvnc_h1_mpeg_calibration_runtime_result_t result;
    pstvnc_h1_mpeg_calibration_foreground_ops_t ops = test_ops();
    runtime_test_ops_state_t ops_state = {{0}, 0};
    pstvnc_controller_state_t state;

    pstvnc_h1_mpeg_calibration_runtime_init(&runtime, 704, 462);

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);
    assert(pstvnc_h1_mpeg_calibration_runtime_service_controller(
        &runtime, &state, &ops, &ops_state, &result));

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT);
    assert(pstvnc_h1_mpeg_calibration_runtime_service_controller(
        &runtime, &state, &ops, &ops_state, &result));
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);
    assert(!result.calibration_visible);
    assert(!pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(&runtime));

    state = controller(0, 0, PSTVNC_CONTROLLER_BUTTON_CIRCLE);
    assert(pstvnc_h1_mpeg_calibration_runtime_service_controller(
        &runtime, &state, &ops, &ops_state, &result));
    assert(result.consume_controller_state);
    assert(!result.freeze_rfb_visuals);
    assert(pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(&runtime));
    assert(pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(&runtime));
    assert(ops_state.call_count == 4);
    assert(ops_state.calls[3] == 'R');

    assert(pstvnc_h1_mpeg_calibration_runtime_take_full_refresh(&runtime));
    assert(!pstvnc_h1_mpeg_calibration_runtime_take_full_refresh(&runtime));
}

static void test_no_committed_geometry_before_accept(void)
{
    pstvnc_h1_mpeg_calibration_runtime_t runtime;

    pstvnc_h1_mpeg_calibration_runtime_init(&runtime, 704, 462);
    assert(pstvnc_h1_mpeg_calibration_runtime_committed_region(&runtime) == NULL);
}

int main(void)
{
    test_entry_freezes_gate_and_exposes_render_plan();
    test_cancel_thaws_only_after_release_and_requests_full_refresh();
    test_no_committed_geometry_before_accept();

    puts("MPEG_CALIBRATION_RUNTIME_HOST_TEST=PASS");
    return 0;
}
