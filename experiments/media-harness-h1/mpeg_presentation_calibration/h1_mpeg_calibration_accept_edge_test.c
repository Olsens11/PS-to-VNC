/*
 * File synopsis:
 * Strict host contract test proving the existing calibration REVIEW acceptance
 * edge reaches the CP2P-facing interaction-binding result exactly once while the
 * committed region remains available through the binding-owned runtime.
 */
#include "h1_mpeg_calibration_interaction_binding.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int pstvnc_input_runtime_suspend_mouse_interpretation(
    pstvnc_input_runtime_t *runtime)
{
    return runtime != NULL ? 0 : -1;
}

int pstvnc_input_runtime_rebase_suspended_mouse_state(
    pstvnc_input_runtime_t *runtime,
    unsigned int x,
    unsigned int y,
    unsigned char buttons)
{
    (void)x;
    (void)y;
    (void)buttons;
    return runtime != NULL ? 0 : -1;
}

int pstvnc_input_runtime_resume_mouse_interpretation(
    pstvnc_input_runtime_t *runtime)
{
    return runtime != NULL ? 0 : -1;
}

int pstvnc_rfb_session_send_pointer_event(
    pstvnc_rfb_session_t *session,
    uint8_t button_mask,
    uint16_t x,
    uint16_t y)
{
    (void)button_mask;
    (void)x;
    (void)y;
    return session != NULL;
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

static void service(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    pstvnc_h1_mpeg_calibration_interaction_context_t *context,
    pstvnc_controller_state_t state,
    int expect_accepted)
{
    pstvnc_h1_mpeg_calibration_interaction_result_t result;

    assert(pstvnc_h1_mpeg_calibration_interaction_binding_service_controller_result(
        binding, context, &state, &result));
    assert(result.consume_controller_state);
    assert((int)result.accepted == expect_accepted);
}

int main(void)
{
    pstvnc_h1_mpeg_calibration_interaction_binding_t binding;
    pstvnc_h1_mpeg_calibration_interaction_context_t context;
    pstvnc_input_runtime_t input_runtime;
    pstvnc_rfb_session_t rfb_session;
    const pstvnc_mpeg_cal_region_t *committed;
    unsigned int cursor_x = 101u;
    unsigned int cursor_y = 202u;
    unsigned char buttons = 0u;
    uint32_t pointer_messages = 0u;
    int suspended = 0;

    memset(&input_runtime, 0, sizeof(input_runtime));
    memset(&rfb_session, 0, sizeof(rfb_session));
    memset(&context, 0, sizeof(context));

    context.input_runtime = &input_runtime;
    context.rfb_session = &rfb_session;
    context.published_cursor_x = &cursor_x;
    context.published_cursor_y = &cursor_y;
    context.published_click_buttons = &buttons;
    context.pointer_messages_sent = &pointer_messages;
    context.mouse_interpretation_suspended = &suspended;

    pstvnc_h1_mpeg_calibration_interaction_binding_init(
        &binding, 704, 462);

    service(
        &binding,
        &context,
        controller(
            PSTVNC_CONTROLLER_BUTTON_START |
                PSTVNC_CONTROLLER_BUTTON_SELECT,
            PSTVNC_CONTROLLER_BUTTON_START |
                PSTVNC_CONTROLLER_BUTTON_SELECT,
            0),
        0);

    service(
        &binding,
        &context,
        controller(
            PSTVNC_CONTROLLER_BUTTON_CROSS,
            PSTVNC_CONTROLLER_BUTTON_CROSS,
            PSTVNC_CONTROLLER_BUTTON_START |
                PSTVNC_CONTROLLER_BUTTON_SELECT),
        0);

    service(
        &binding,
        &context,
        controller(0, 0, PSTVNC_CONTROLLER_BUTTON_CROSS),
        0);

    service(
        &binding,
        &context,
        controller(
            PSTVNC_CONTROLLER_BUTTON_CROSS,
            PSTVNC_CONTROLLER_BUTTON_CROSS,
            0),
        1);

    committed =
        pstvnc_h1_mpeg_calibration_interaction_binding_committed_region(
            &binding);
    assert(committed != NULL);
    assert(committed->x == 176);
    assert(committed->y == 119);
    assert(committed->width == 352);
    assert(committed->height == 224);

    /* The same committed region remains available, but accepted is not sticky. */
    service(
        &binding,
        &context,
        controller(PSTVNC_CONTROLLER_BUTTON_CROSS, 0, 0),
        0);
    assert(
        pstvnc_h1_mpeg_calibration_interaction_binding_committed_region(
            &binding) == committed);

    puts("MPEG_CALIBRATION_ACCEPT_EDGE_HOST_TEST=PASS");
    return 0;
}
