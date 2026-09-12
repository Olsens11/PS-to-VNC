/*
 * File synopsis:
 * Implements the experiment-local H1 coordinator binding for MPEG calibration
 * foreground ownership and RFB flow policy preparation.
 */
#include "h1_mpeg_calibration_interaction_binding.h"

#include <stddef.h>

static int binding_context_valid(
    const pstvnc_h1_mpeg_calibration_interaction_context_t *context)
{
    return context != NULL &&
        context->input_runtime != NULL &&
        context->rfb_session != NULL &&
        context->published_cursor_x != NULL &&
        context->published_cursor_y != NULL &&
        context->published_click_buttons != NULL &&
        context->pointer_messages_sent != NULL &&
        context->mouse_interpretation_suspended != NULL;
}

static int binding_suspend_mouse(void *opaque)
{
    pstvnc_h1_mpeg_calibration_interaction_context_t *context =
        (pstvnc_h1_mpeg_calibration_interaction_context_t *)opaque;

    if (!binding_context_valid(context))
        return 0;

    if (*context->mouse_interpretation_suspended)
        return 1;

    if (pstvnc_input_runtime_suspend_mouse_interpretation(
            context->input_runtime) < 0)
        return 0;

    *context->mouse_interpretation_suspended = 1;
    return 1;
}

static int binding_neutralize_pointer(void *opaque)
{
    pstvnc_h1_mpeg_calibration_interaction_context_t *context =
        (pstvnc_h1_mpeg_calibration_interaction_context_t *)opaque;

    if (!binding_context_valid(context) ||
        *context->published_cursor_x > UINT16_MAX ||
        *context->published_cursor_y > UINT16_MAX)
        return 0;

    if (*context->published_click_buttons == 0)
        return 1;

    if (*context->pointer_messages_sent == UINT32_MAX)
        return 0;

    if (!pstvnc_rfb_session_send_pointer_event(
            context->rfb_session,
            0,
            (uint16_t)*context->published_cursor_x,
            (uint16_t)*context->published_cursor_y))
        return 0;

    *context->published_click_buttons = 0;
    (*context->pointer_messages_sent)++;
    return 1;
}

static int binding_rebase_mouse(void *opaque)
{
    pstvnc_h1_mpeg_calibration_interaction_context_t *context =
        (pstvnc_h1_mpeg_calibration_interaction_context_t *)opaque;

    if (!binding_context_valid(context) ||
        !*context->mouse_interpretation_suspended)
        return 0;

    return pstvnc_input_runtime_rebase_suspended_mouse_state(
        context->input_runtime,
        *context->published_cursor_x,
        *context->published_cursor_y,
        *context->published_click_buttons) >= 0;
}

static int binding_resume_mouse(void *opaque)
{
    pstvnc_h1_mpeg_calibration_interaction_context_t *context =
        (pstvnc_h1_mpeg_calibration_interaction_context_t *)opaque;

    if (!binding_context_valid(context))
        return 0;

    if (!*context->mouse_interpretation_suspended)
        return 1;

    if (pstvnc_input_runtime_resume_mouse_interpretation(
            context->input_runtime) < 0)
        return 0;

    *context->mouse_interpretation_suspended = 0;
    return 1;
}

void pstvnc_h1_mpeg_calibration_interaction_binding_init(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    int canvas_width,
    int canvas_height)
{
    if (binding == NULL)
        return;

    pstvnc_h1_mpeg_calibration_runtime_init(
        &binding->runtime,
        canvas_width,
        canvas_height);

    pstvnc_h1_mpeg_calibration_rfb_flow_init(
        &binding->rfb_flow,
        &binding->runtime);

    (void)pstvnc_h1_mpeg_calibration_rfb_flow_prepare_policy(
        &binding->rfb_flow,
        &binding->rfb_policy);
}

int pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    pstvnc_h1_mpeg_calibration_interaction_context_t *context,
    const pstvnc_controller_state_t *controller_state,
    int *consume_controller_state)
{
    static const pstvnc_h1_mpeg_calibration_foreground_ops_t ops = {
        binding_suspend_mouse,
        binding_neutralize_pointer,
        binding_rebase_mouse,
        binding_resume_mouse
    };
    pstvnc_h1_mpeg_calibration_runtime_result_t result;

    if (binding == NULL ||
        !binding_context_valid(context) ||
        controller_state == NULL ||
        consume_controller_state == NULL)
        return 0;

    if (!pstvnc_h1_mpeg_calibration_runtime_service_controller(
            &binding->runtime,
            controller_state,
            &ops,
            context,
            &result))
        return 0;

    *consume_controller_state = result.consume_controller_state ? 1 : 0;
    return 1;
}

int pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding)
{
    if (binding == NULL)
        return 0;

    return pstvnc_mpeg_calibration_owns_foreground(
        &binding->runtime.foreground.adapter.calibration)
        ? 1
        : 0;
}

int pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding)
{
    return binding != NULL &&
        binding->runtime.foreground.mouse_interpretation_suspended != 0;
}

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_mpeg_calibration_interaction_binding_rfb_policy(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding)
{
    if (binding == NULL)
        return NULL;

    return &binding->rfb_policy;
}
