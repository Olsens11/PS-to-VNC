/*
 * File synopsis:
 * Strict host contract test for the experiment-local MPEG calibration
 * interaction-coordinator binding.
 */
#include "h1_mpeg_calibration_interaction_binding.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned int suspend_calls;
static unsigned int rebase_calls;
static unsigned int resume_calls;
static unsigned int pointer_calls;
static uint8_t last_pointer_buttons;
static uint16_t last_pointer_x;
static uint16_t last_pointer_y;

int pstvnc_input_runtime_suspend_mouse_interpretation(
    pstvnc_input_runtime_t *runtime)
{
    assert(runtime != NULL);
    suspend_calls++;
    return 0;
}

int pstvnc_input_runtime_rebase_suspended_mouse_state(
    pstvnc_input_runtime_t *runtime,
    unsigned int x,
    unsigned int y,
    unsigned char buttons)
{
    assert(runtime != NULL);
    assert(x == 101u);
    assert(y == 202u);
    assert(buttons == 0);
    rebase_calls++;
    return 0;
}

int pstvnc_input_runtime_resume_mouse_interpretation(
    pstvnc_input_runtime_t *runtime)
{
    assert(runtime != NULL);
    resume_calls++;
    return 0;
}

int pstvnc_rfb_session_send_pointer_event(
    pstvnc_rfb_session_t *session,
    uint8_t button_mask,
    uint16_t x,
    uint16_t y)
{
    assert(session != NULL);
    pointer_calls++;
    last_pointer_buttons = button_mask;
    last_pointer_x = x;
    last_pointer_y = y;
    return 1;
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

static void reset_spies(void)
{
    suspend_calls = 0;
    rebase_calls = 0;
    resume_calls = 0;
    pointer_calls = 0;
    last_pointer_buttons = 0xffu;
    last_pointer_x = 0;
    last_pointer_y = 0;
}

static void test_entry_uses_existing_ownership_sequence_and_consumes_chord(void)
{
    pstvnc_h1_mpeg_calibration_interaction_binding_t binding;
    pstvnc_h1_mpeg_calibration_interaction_context_t context;
    pstvnc_input_runtime_t input_runtime;
    pstvnc_rfb_session_t rfb_session;
    pstvnc_controller_state_t state;
    unsigned int cursor_x = 101u;
    unsigned int cursor_y = 202u;
    unsigned char buttons = 1u;
    uint32_t pointer_messages = 7u;
    int suspended = 0;
    int consume = 0;

    memset(&input_runtime, 0, sizeof(input_runtime));
    memset(&rfb_session, 0, sizeof(rfb_session));
    reset_spies();

    context.input_runtime = &input_runtime;
    context.rfb_session = &rfb_session;
    context.published_cursor_x = &cursor_x;
    context.published_cursor_y = &cursor_y;
    context.published_click_buttons = &buttons;
    context.pointer_messages_sent = &pointer_messages;
    context.mouse_interpretation_suspended = &suspended;

    pstvnc_h1_mpeg_calibration_interaction_binding_init(
        &binding, 704, 462);

    assert(!pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
        &binding));
    assert(!pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
        &binding));

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);

    assert(pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
        &binding, &context, &state, &consume));
    assert(consume);
    assert(suspended);
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
        &binding));
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
        &binding));
    assert(suspend_calls == 1u);
    assert(pointer_calls == 1u);
    assert(last_pointer_buttons == 0);
    assert(last_pointer_x == 101u);
    assert(last_pointer_y == 202u);
    assert(buttons == 0);
    assert(pointer_messages == 8u);
    assert(rebase_calls == 1u);
    assert(resume_calls == 0u);
    assert(!pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(
        &binding.runtime));
    assert(!pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(
        &binding.runtime));
}

static void test_ordinary_select_is_not_consumed(void)
{
    pstvnc_h1_mpeg_calibration_interaction_binding_t binding;
    pstvnc_h1_mpeg_calibration_interaction_context_t context;
    pstvnc_input_runtime_t input_runtime;
    pstvnc_rfb_session_t rfb_session;
    pstvnc_controller_state_t state;
    unsigned int cursor_x = 101u;
    unsigned int cursor_y = 202u;
    unsigned char buttons = 0;
    uint32_t pointer_messages = 0;
    int suspended = 0;
    int consume = 1;

    memset(&input_runtime, 0, sizeof(input_runtime));
    memset(&rfb_session, 0, sizeof(rfb_session));
    reset_spies();

    context.input_runtime = &input_runtime;
    context.rfb_session = &rfb_session;
    context.published_cursor_x = &cursor_x;
    context.published_cursor_y = &cursor_y;
    context.published_click_buttons = &buttons;
    context.pointer_messages_sent = &pointer_messages;
    context.mouse_interpretation_suspended = &suspended;

    pstvnc_h1_mpeg_calibration_interaction_binding_init(
        &binding, 704, 462);

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);

    assert(pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
        &binding, &context, &state, &consume));
    assert(!consume);
    assert(!suspended);
    assert(!pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
        &binding));
    assert(!pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
        &binding));
    assert(suspend_calls == 0u);
    assert(pointer_calls == 0u);
    assert(rebase_calls == 0u);
    assert(resume_calls == 0u);
}

static void test_cancel_holds_ownership_until_full_release_then_resumes(void)
{
    pstvnc_h1_mpeg_calibration_interaction_binding_t binding;
    pstvnc_h1_mpeg_calibration_interaction_context_t context;
    pstvnc_input_runtime_t input_runtime;
    pstvnc_rfb_session_t rfb_session;
    pstvnc_controller_state_t state;
    unsigned int cursor_x = 101u;
    unsigned int cursor_y = 202u;
    unsigned char buttons = 0;
    uint32_t pointer_messages = 0;
    int suspended = 0;
    int consume = 0;

    memset(&input_runtime, 0, sizeof(input_runtime));
    memset(&rfb_session, 0, sizeof(rfb_session));
    reset_spies();

    context.input_runtime = &input_runtime;
    context.rfb_session = &rfb_session;
    context.published_cursor_x = &cursor_x;
    context.published_cursor_y = &cursor_y;
    context.published_click_buttons = &buttons;
    context.pointer_messages_sent = &pointer_messages;
    context.mouse_interpretation_suspended = &suspended;

    pstvnc_h1_mpeg_calibration_interaction_binding_init(
        &binding, 704, 462);

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
        &binding, &context, &state, &consume));
    assert(consume && suspended);
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
        &binding));
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
        &binding));

    state = controller(
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_START |
            PSTVNC_CONTROLLER_BUTTON_SELECT);
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
        &binding, &context, &state, &consume));
    assert(consume && suspended);
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
        &binding));
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
        &binding));
    assert(resume_calls == 0u);

    state = controller(0, 0, PSTVNC_CONTROLLER_BUTTON_CIRCLE);
    assert(pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
        &binding, &context, &state, &consume));
    assert(consume);
    assert(!suspended);
    assert(!pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
        &binding));
    assert(!pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
        &binding));
    assert(resume_calls == 1u);
    assert(pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(
        &binding.runtime));
    assert(pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(
        &binding.runtime));
}

static void test_binding_exposes_prepared_rfb_policy(void)
{
    pstvnc_h1_mpeg_calibration_interaction_binding_t binding;
    const pstvnc_h1_rfb_flow_policy_t *policy;

    pstvnc_h1_mpeg_calibration_interaction_binding_init(
        &binding, 704, 462);
    policy = pstvnc_h1_mpeg_calibration_interaction_binding_rfb_policy(
        &binding);

    assert(policy != NULL);
    assert(policy->next_request != NULL);
    assert(policy->request_sent != NULL);
    assert(policy->update_complete != NULL);
    assert(policy->allow_present != NULL);
    assert(policy->context == &binding.rfb_flow);
}

int main(void)
{
    test_entry_uses_existing_ownership_sequence_and_consumes_chord();
    test_ordinary_select_is_not_consumed();
    test_cancel_holds_ownership_until_full_release_then_resumes();
    test_binding_exposes_prepared_rfb_policy();

    puts("MPEG_CALIBRATION_INTERACTION_BINDING_HOST_TEST=PASS");
    return 0;
}
