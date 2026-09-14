/*
 * File synopsis:
 * Implements H1's experiment-local composition of the real through-Issue-39
 * interaction modules over the already-qualified mux-backed RFB session.
 *
 * The implementation deliberately mirrors only application-level orchestration
 * that cannot be reused unchanged while H1 owns the physical PSTV connection and
 * finite-session quiesce. Domain behavior remains in the clean input, mouse,
 * local-controller, local-UI, OSK, keyboard, display, graphics, and RFB modules.
 *
 * MPEG presentation calibration remains experiment-local. This coordinator owns
 * only the live composition seam: a monotonic held-entry policy, the already-
 * proven calibration interaction binding, and the borrowed RFB flow policy.
 */
#include "h1_interaction_coordinator.h"

#include "keyboard.h"
#include "local_ui_presentation.h"
#include "mouse.h"
#include "platform/ps2_graphics.h"
#include "rfb.h"

#include <stddef.h>
#include <string.h>
#include <timer.h>

#define PSTVNC_H1_INTERACTION_CONTROLLER_PORT 0
#define PSTVNC_H1_INTERACTION_CONTROLLER_SLOT 0

/*
 * Temporary hardware-test input adapter only. The session/MPEG lifecycle is
 * deliberately trigger-agnostic so a later local-UI button can replace this
 * chord without changing retirement, RFB restoration, START, or ownership.
 */
#define PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD \
    (PSTVNC_CONTROLLER_BUTTON_START | PSTVNC_CONTROLLER_BUTTON_SELECT)

static int h1_interaction_increment_counter(uint32_t *counter)
{
    if (counter == NULL || *counter == UINT32_MAX)
        return 0;

    (*counter)++;
    return 1;
}

static uint64_t h1_interaction_monotonic_us(void)
{
    unsigned int seconds = 0;
    unsigned int microseconds = 0;

    TimerBusClock2USec(
        GetTimerSystemTime(),
        &seconds,
        &microseconds);

    return
        (uint64_t)seconds * UINT64_C(1000000) +
        (uint64_t)microseconds;
}

static int h1_interaction_map_semantic_clicks(
    unsigned char semantic_clicks,
    uint8_t *rfb_buttons)
{
    uint8_t mapped_buttons = 0;

    if (rfb_buttons == NULL ||
        (semantic_clicks &
         (unsigned char)~PSTVNC_MOUSE_BUTTON_MASK) != 0)
        return 0;

    if (semantic_clicks & PSTVNC_MOUSE_BUTTON_LCLICK)
        mapped_buttons |= PSTVNC_RFB_POINTER_BUTTON_LEFT;

    if (semantic_clicks & PSTVNC_MOUSE_BUTTON_RCLICK)
        mapped_buttons |= PSTVNC_RFB_POINTER_BUTTON_RIGHT;

    *rfb_buttons = mapped_buttons;
    return 1;
}

static int h1_interaction_map_wheel_direction(
    pstvnc_mouse_wheel_direction_t direction,
    uint8_t *rfb_button)
{
    if (rfb_button == NULL)
        return 0;

    switch (direction) {
        case PSTVNC_MOUSE_WHEEL_NONE:
            *rfb_button = 0;
            return 1;

        case PSTVNC_MOUSE_WHEEL_UP:
            *rfb_button = PSTVNC_RFB_POINTER_WHEEL_UP;
            return 1;

        case PSTVNC_MOUSE_WHEEL_DOWN:
            *rfb_button = PSTVNC_RFB_POINTER_WHEEL_DOWN;
            return 1;

        case PSTVNC_MOUSE_WHEEL_LEFT:
            *rfb_button = PSTVNC_RFB_POINTER_WHEEL_LEFT;
            return 1;

        case PSTVNC_MOUSE_WHEEL_RIGHT:
            *rfb_button = PSTVNC_RFB_POINTER_WHEEL_RIGHT;
            return 1;

        default:
            return 0;
    }
}

static int h1_interaction_publish_mouse_update(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session,
    const pstvnc_mouse_update_t *mouse_update)
{
    uint8_t ordinary_rfb_buttons;
    uint8_t wheel_rfb_button;

    if (coordinator == NULL ||
        session == NULL ||
        mouse_update == NULL ||
        mouse_update->cursor_x > UINT16_MAX ||
        mouse_update->cursor_y > UINT16_MAX)
        return 0;

    if (!h1_interaction_map_semantic_clicks(
            mouse_update->click_buttons,
            &ordinary_rfb_buttons) ||
        !h1_interaction_map_wheel_direction(
            mouse_update->wheel_direction,
            &wheel_rfb_button))
        return 0;

    if (mouse_update->pointer_changed) {
        if (coordinator->stats.pointer_messages_sent == UINT32_MAX)
            return 0;

        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                ordinary_rfb_buttons,
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y))
            return 0;

        coordinator->published_cursor_x = mouse_update->cursor_x;
        coordinator->published_cursor_y = mouse_update->cursor_y;
        coordinator->published_click_buttons = mouse_update->click_buttons;
        coordinator->stats.pointer_messages_sent++;
    }

    if (wheel_rfb_button != 0) {
        if (coordinator->stats.pointer_messages_sent > UINT32_MAX - 2u ||
            coordinator->stats.wheel_pulses_sent == UINT32_MAX)
            return 0;

        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                (uint8_t)(ordinary_rfb_buttons | wheel_rfb_button),
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y) ||
            !pstvnc_rfb_session_send_pointer_event(
                session,
                ordinary_rfb_buttons,
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y))
            return 0;

        coordinator->published_cursor_x = mouse_update->cursor_x;
        coordinator->published_cursor_y = mouse_update->cursor_y;
        coordinator->published_click_buttons = mouse_update->click_buttons;
        coordinator->stats.pointer_messages_sent += 2u;
        coordinator->stats.wheel_pulses_sent++;
    }

    return 1;
}

static int h1_interaction_publish_keyboard_tap(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session,
    const pstvnc_keyboard_tap_t *keyboard_tap)
{
    pstvnc_keyboard_sequence_t sequence;
    unsigned int event_index;

    if (coordinator == NULL || session == NULL || keyboard_tap == NULL)
        return 0;

    if (!pstvnc_keyboard_build_tap_sequence(
            keyboard_tap->keysym,
            keyboard_tap->modifiers,
            &sequence) ||
        sequence.event_count == 0 ||
        sequence.event_count > PSTVNC_KEYBOARD_SEQUENCE_MAX_EVENTS ||
        coordinator->stats.keyboard_taps_published == UINT32_MAX ||
        coordinator->stats.key_messages_sent >
            UINT32_MAX - sequence.event_count)
        return 0;

    for (event_index = 0;
         event_index < sequence.event_count;
         event_index++) {

        if (!pstvnc_rfb_session_send_key_event(
                session,
                sequence.events[event_index].down,
                sequence.events[event_index].keysym))
            return 0;

        coordinator->stats.key_messages_sent++;
    }

    coordinator->stats.keyboard_taps_published++;
    return 1;
}

static int h1_interaction_present_current_frame(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    int remote_frame_changed)
{
    pstvnc_local_ui_presentation_t local_presentation;
    pstvnc_ps2_graphics_overlay_t platform_overlay;
    const pstvnc_ps2_graphics_overlay_t *platform_overlay_ptr = NULL;

    if (coordinator == NULL ||
        coordinator->current_framebuffer == NULL ||
        !coordinator->current_framebuffer->valid)
        return 0;

    if (remote_frame_changed) {
        if (!pstvnc_display_prepare_gs16(
                coordinator->current_framebuffer,
                coordinator->gs_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT))
            return 0;
    }

    if (!pstvnc_local_ui_prepare_presentation(
            &coordinator->local_ui,
            &coordinator->osk,
            coordinator->local_overlay_pixels,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT,
            &local_presentation))
        return 0;

    if (local_presentation.overlay_visible) {
        platform_overlay.pixels = local_presentation.overlay_pixels;
        platform_overlay.pixel_count = local_presentation.overlay_pixel_count;
        platform_overlay.width = local_presentation.overlay_width;
        platform_overlay.height = local_presentation.overlay_height;
        platform_overlay.x = local_presentation.overlay_x;
        platform_overlay.y = local_presentation.overlay_y;
        platform_overlay_ptr = &platform_overlay;
    }

    if (pstvnc_ps2_graphics_present(
            coordinator->gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT,
            platform_overlay_ptr) < 0)
        return 0;

    if (pstvnc_local_ui_needs_present(&coordinator->local_ui)) {
        if (!pstvnc_local_ui_mark_presented(
                &coordinator->local_ui,
                local_presentation.generation))
            return 0;
    }

    return h1_interaction_increment_counter(
        &coordinator->stats.local_presentations);
}

static int h1_interaction_neutralize_published_pointer(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session)
{
    if (coordinator == NULL || session == NULL ||
        coordinator->published_cursor_x > UINT16_MAX ||
        coordinator->published_cursor_y > UINT16_MAX)
        return 0;

    if (coordinator->published_click_buttons == 0)
        return 1;

    if (coordinator->stats.pointer_messages_sent == UINT32_MAX)
        return 0;

    if (!pstvnc_rfb_session_send_pointer_event(
            session,
            0,
            (uint16_t)coordinator->published_cursor_x,
            (uint16_t)coordinator->published_cursor_y))
        return 0;

    coordinator->published_click_buttons = 0;
    coordinator->stats.pointer_messages_sent++;
    return 1;
}

static int h1_interaction_apply_osk_activation(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session,
    const pstvnc_osk_activation_t *activation)
{
    if (coordinator == NULL || session == NULL || activation == NULL)
        return 0;

    if (activation->local_state_changed &&
        !pstvnc_local_ui_mark_local_change(&coordinator->local_ui))
        return 0;

    if (activation->produced_keyboard_tap &&
        !h1_interaction_publish_keyboard_tap(
            coordinator,
            session,
            &activation->keyboard_tap))
        return 0;

    return 1;
}

static int h1_interaction_open_osk(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session)
{
    if (coordinator == NULL || session == NULL ||
        coordinator->local_ui.foreground !=
            PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP ||
        pstvnc_local_ui_input_is_quarantined(&coordinator->local_ui) ||
        coordinator->mouse_interpretation_suspended)
        return 0;

    if (pstvnc_input_runtime_suspend_mouse_interpretation(
            &coordinator->input_runtime) < 0)
        return 0;

    coordinator->mouse_interpretation_suspended = 1;

    if (!h1_interaction_neutralize_published_pointer(
            coordinator,
            session) ||
        pstvnc_input_runtime_rebase_suspended_mouse_state(
            &coordinator->input_runtime,
            coordinator->published_cursor_x,
            coordinator->published_cursor_y,
            coordinator->published_click_buttons) < 0)
        return 0;

    pstvnc_osk_reset_for_open(&coordinator->osk);

    if (!pstvnc_local_ui_open_osk(&coordinator->local_ui) ||
        !h1_interaction_increment_counter(&coordinator->stats.osk_open_count))
        return 0;

    return 1;
}

static int h1_interaction_apply_local_action(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session,
    pstvnc_local_controller_action_t action)
{
    pstvnc_osk_activation_t activation;

    if (coordinator == NULL || session == NULL)
        return 0;

    switch (action) {
        case PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK:
            return h1_interaction_open_osk(coordinator, session);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_LEFT:
            return
                pstvnc_osk_move_horizontal(&coordinator->osk, -1) &&
                pstvnc_local_ui_mark_local_change(&coordinator->local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_RIGHT:
            return
                pstvnc_osk_move_horizontal(&coordinator->osk, 1) &&
                pstvnc_local_ui_mark_local_change(&coordinator->local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_UP:
            return
                pstvnc_osk_move_vertical(&coordinator->osk, -1) &&
                pstvnc_local_ui_mark_local_change(&coordinator->local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_DOWN:
            return
                pstvnc_osk_move_vertical(&coordinator->osk, 1) &&
                pstvnc_local_ui_mark_local_change(&coordinator->local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_TOGGLE_SHIFT:
            return
                pstvnc_osk_toggle_shift_modifier(&coordinator->osk) &&
                pstvnc_local_ui_mark_local_change(&coordinator->local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_ACTIVATE_SELECTED:
            if (!pstvnc_osk_activate_selected(&coordinator->osk, &activation))
                return 0;
            return h1_interaction_apply_osk_activation(
                coordinator, session, &activation);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_BACKSPACE:
            if (!pstvnc_osk_activate_direct_key(
                    &coordinator->osk,
                    PSTVNC_KEYBOARD_KEYSYM_BACKSPACE,
                    &activation))
                return 0;
            return h1_interaction_apply_osk_activation(
                coordinator, session, &activation);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_ENTER:
            if (!pstvnc_osk_activate_direct_key(
                    &coordinator->osk,
                    PSTVNC_KEYBOARD_KEYSYM_ENTER,
                    &activation))
                return 0;
            return h1_interaction_apply_osk_activation(
                coordinator, session, &activation);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_TAB:
            if (!pstvnc_osk_activate_direct_key(
                    &coordinator->osk,
                    PSTVNC_KEYBOARD_KEYSYM_TAB,
                    &activation))
                return 0;
            return h1_interaction_apply_osk_activation(
                coordinator, session, &activation);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_CLOSE_OSK:
            if (coordinator->local_ui.foreground !=
                    PSTVNC_LOCAL_UI_FOREGROUND_OSK)
                return 0;

            (void)pstvnc_osk_clear_modifiers(&coordinator->osk);

            if (!pstvnc_local_ui_close_osk(&coordinator->local_ui) ||
                !h1_interaction_increment_counter(
                    &coordinator->stats.osk_close_count))
                return 0;

            return 1;

        default:
            return 0;
    }
}

static void h1_interaction_prepare_calibration_context(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session,
    pstvnc_h1_mpeg_calibration_interaction_context_t *context)
{
    memset(context, 0, sizeof(*context));
    context->input_runtime = &coordinator->input_runtime;
    context->rfb_session = session;
    context->published_cursor_x = &coordinator->published_cursor_x;
    context->published_cursor_y = &coordinator->published_cursor_y;
    context->published_click_buttons = &coordinator->published_click_buttons;
    context->pointer_messages_sent = &coordinator->stats.pointer_messages_sent;
    context->mouse_interpretation_suspended =
        &coordinator->mouse_interpretation_suspended;
}

static int h1_interaction_calibration_entry_available(
    const pstvnc_h1_interaction_coordinator_t *coordinator)
{
    if (coordinator == NULL)
        return 0;

    return
        coordinator->local_ui.foreground ==
            PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP &&
        !pstvnc_local_ui_input_is_quarantined(&coordinator->local_ui) &&
        !coordinator->mouse_interpretation_suspended &&
        !coordinator->calibration_entry_deferred &&
        !pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
            &coordinator->mpeg_calibration);
}

static int h1_interaction_record_calibration_result(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    const pstvnc_h1_mpeg_calibration_interaction_result_t *result)
{
    const pstvnc_mpeg_cal_region_t *committed_region;

    if (coordinator == NULL || result == NULL)
        return 0;

    if (!result->accepted)
        return 1;

    /*
     * CP2P consumes this snapshot from the same service cadence. Other H1
     * compositions may have no consumer; in that case retain the latest accepted
     * settings without changing their calibration behavior.
     */
    committed_region =
        pstvnc_h1_mpeg_calibration_interaction_binding_committed_region(
            &coordinator->mpeg_calibration);
    if (committed_region == NULL)
        return 0;

    coordinator->accepted_calibration_region = *committed_region;
    coordinator->accepted_calibration_pending = 1;
    return 1;
}

static int h1_interaction_service_active_calibration(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session,
    const pstvnc_controller_state_t *controller_state)
{
    pstvnc_h1_mpeg_calibration_interaction_context_t calibration_context;
    pstvnc_h1_mpeg_calibration_interaction_result_t result;

    h1_interaction_prepare_calibration_context(
        coordinator,
        session,
        &calibration_context);
    memset(&result, 0, sizeof(result));

    if (!pstvnc_h1_mpeg_calibration_interaction_binding_service_controller_result(
            &coordinator->mpeg_calibration,
            &calibration_context,
            controller_state,
            &result) ||
        !h1_interaction_record_calibration_result(coordinator, &result))
        return 0;

    /*
     * Once calibration owns foreground (including release quarantine), every
     * trustworthy controller observation belongs to it. Losing that consumption
     * fact would be an ownership violation, not a reason to fall through.
     */
    return result.consume_controller_state ? 1 : 0;
}

static int h1_interaction_service_controller_state(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session,
    const pstvnc_controller_state_t *controller_state)
{
    pstvnc_local_controller_result_t result;
    int consume_entry_sample = 0;
    int entry_available;
    int entry_chord_down;
    unsigned int action_index;

    if (coordinator == NULL || session == NULL || controller_state == NULL)
        return 0;

    /*
     * Calibration gets first refusal only after it has actually claimed
     * foreground. OSK and calibration never stack: OSK remains the ordinary
     * local-controller owner until the user closes it.
     */
    if (pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
            &coordinator->mpeg_calibration))
        return h1_interaction_service_active_calibration(
            coordinator,
            session,
            controller_state);

    /*
     * A matured CP2P entry request owns controller routing while its session
     * prerequisite is pending. This keeps START/SELECT from leaking into OSK or
     * desktop actions during the one-full-RFB restoration interval.
     */
    if (coordinator->calibration_entry_deferred)
        return 1;

    entry_available =
        h1_interaction_calibration_entry_available(coordinator);
    entry_chord_down =
        (controller_state->buttons_down &
         PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD) ==
        PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD;

    /*
     * Arm/cancel the 750 ms policy before ordinary routing. While a desktop
     * entry attempt is armed, consume its transition samples so SELECT cannot
     * also open OSK. If OSK already owns foreground, entry_available is false
     * and the same physical sample passes through unchanged to the OSK router.
     */
    if (!pstvnc_h1_mpeg_calibration_entry_hold_observe(
            &coordinator->mpeg_calibration_entry_hold,
            h1_interaction_monotonic_us(),
            entry_available,
            entry_chord_down,
            &consume_entry_sample))
        return 0;

    if (consume_entry_sample)
        return 1;

    if (!pstvnc_local_controller_route(
            &coordinator->local_controller,
            coordinator->local_ui.foreground,
            pstvnc_local_ui_input_is_quarantined(&coordinator->local_ui),
            controller_state,
            &result))
        return 0;

    for (action_index = 0;
         action_index < result.action_count;
         action_index++) {

        if (!h1_interaction_apply_local_action(
                coordinator,
                session,
                result.actions[action_index]))
            return 0;
    }

    if (pstvnc_local_ui_input_is_quarantined(&coordinator->local_ui) &&
        pstvnc_local_controller_release_is_proven(
            &coordinator->local_controller)) {

        if (!pstvnc_local_ui_complete_input_quarantine(
                &coordinator->local_ui))
            return 0;
    }

    return 1;
}

static int h1_interaction_query_calibration_entry_gate(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    int *enter_now)
{
    if (coordinator == NULL || enter_now == NULL)
        return 0;

    if (coordinator->calibration_entry_gate == NULL) {
        *enter_now = 1;
        return 1;
    }

    if (!coordinator->calibration_entry_gate(
            coordinator->calibration_entry_gate_context,
            enter_now))
        return 0;

    *enter_now = *enter_now ? 1 : 0;
    return 1;
}

static int h1_interaction_activate_calibration(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_mpeg_calibration_interaction_context_t calibration_context;
    pstvnc_h1_mpeg_calibration_interaction_result_t result;
    pstvnc_controller_state_t activation_state;

    if (coordinator == NULL || session == NULL)
        return 0;

    memset(&activation_state, 0, sizeof(activation_state));
    activation_state.buttons_down =
        PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD;
    memset(&result, 0, sizeof(result));

    h1_interaction_prepare_calibration_context(
        coordinator,
        session,
        &calibration_context);

    if (!pstvnc_h1_mpeg_calibration_interaction_binding_service_controller_result(
            &coordinator->mpeg_calibration,
            &calibration_context,
            &activation_state,
            &result) ||
        !h1_interaction_record_calibration_result(coordinator, &result))
        return 0;

    return
        result.consume_controller_state &&
        pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
            &coordinator->mpeg_calibration);
}

static int h1_interaction_service_calibration_entry_hold(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session)
{
    int activate_calibration = 0;
    int enter_now = 0;

    if (coordinator == NULL || session == NULL)
        return 0;

    if (coordinator->calibration_entry_deferred) {
        if (!h1_interaction_query_calibration_entry_gate(
                coordinator, &enter_now))
            return 0;

        if (!enter_now)
            return 1;

        coordinator->calibration_entry_deferred = 0;
        return h1_interaction_activate_calibration(coordinator, session);
    }

    if (!pstvnc_h1_mpeg_calibration_entry_hold_poll(
            &coordinator->mpeg_calibration_entry_hold,
            h1_interaction_monotonic_us(),
            h1_interaction_calibration_entry_available(coordinator),
            &activate_calibration))
        return 0;

    if (!activate_calibration)
        return 1;

    if (!h1_interaction_query_calibration_entry_gate(
            coordinator, &enter_now))
        return 0;

    if (!enter_now) {
        coordinator->calibration_entry_deferred = 1;
        return 1;
    }

    return h1_interaction_activate_calibration(coordinator, session);
}

static int h1_interaction_resume_desktop_mouse_if_ready(
    pstvnc_h1_interaction_coordinator_t *coordinator)
{
    if (coordinator == NULL)
        return 0;

    if (!coordinator->mouse_interpretation_suspended)
        return 1;

    /*
     * Calibration owns the same input-runtime suspension fact used by OSK.
     * Its binding alone may withdraw that suspension after full physical
     * release; ordinary desktop cleanup must not race it.
     */
    if (pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
            &coordinator->mpeg_calibration))
        return 1;

    if (coordinator->local_ui.foreground !=
            PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP ||
        pstvnc_local_ui_input_is_quarantined(&coordinator->local_ui) ||
        pstvnc_local_ui_needs_present(&coordinator->local_ui))
        return 1;

    if (pstvnc_input_runtime_resume_mouse_interpretation(
            &coordinator->input_runtime) < 0)
        return 0;

    coordinator->mouse_interpretation_suspended = 0;
    return 1;
}

int pstvnc_h1_interaction_coordinator_set_activity_notify(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_input_runtime_activity_notify_fn notify,
    void *notify_context)
{
    if (coordinator == NULL ||
        coordinator->input_initialized ||
        coordinator->input_started)
        return 0;

    coordinator->activity_notify = notify;
    coordinator->activity_notify_context =
        notify != NULL ? notify_context : NULL;

    return 1;
}

static int h1_interaction_start_input(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session)
{
    if (coordinator == NULL || session == NULL || coordinator->input_initialized)
        return 0;

    if (pstvnc_input_runtime_init(
            &coordinator->input_runtime,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT,
            PSTVNC_H1_INTERACTION_CONTROLLER_PORT,
            PSTVNC_H1_INTERACTION_CONTROLLER_SLOT) < 0)
        return 0;

    coordinator->input_initialized = 1;

    if (pstvnc_input_runtime_set_activity_notify(
            &coordinator->input_runtime,
            coordinator->activity_notify,
            coordinator->activity_notify_context) < 0)
        return 0;

    coordinator->published_cursor_x = PSTVNC_DISPLAY_WIDTH / 2u;
    coordinator->published_cursor_y = PSTVNC_DISPLAY_HEIGHT / 2u;
    coordinator->published_click_buttons = 0;

    if (!pstvnc_rfb_session_send_pointer_event(
            session,
            0,
            (uint16_t)coordinator->published_cursor_x,
            (uint16_t)coordinator->published_cursor_y))
        return 0;

    coordinator->stats.pointer_messages_sent = 1u;

    if (pstvnc_input_runtime_start(&coordinator->input_runtime) < 0)
        return 0;

    coordinator->input_started = 1;
    return 1;
}

void pstvnc_h1_interaction_coordinator_init(
    pstvnc_h1_interaction_coordinator_t *coordinator)
{
    if (coordinator == NULL)
        return;

    memset(coordinator, 0, sizeof(*coordinator));
    pstvnc_local_controller_init(&coordinator->local_controller);
    pstvnc_local_ui_init(&coordinator->local_ui);
    pstvnc_osk_reset_for_open(&coordinator->osk);
    pstvnc_h1_mpeg_calibration_interaction_binding_init(
        &coordinator->mpeg_calibration,
        PSTVNC_DISPLAY_WIDTH,
        PSTVNC_DISPLAY_HEIGHT);
    pstvnc_h1_mpeg_calibration_entry_hold_init(
        &coordinator->mpeg_calibration_entry_hold);
}

int pstvnc_h1_interaction_coordinator_set_calibration_entry_gate(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_h1_interaction_calibration_entry_gate_fn gate,
    void *gate_context)
{
    if (coordinator == NULL || coordinator->input_started ||
        coordinator->calibration_entry_deferred ||
        pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
            &coordinator->mpeg_calibration))
        return 0;

    coordinator->calibration_entry_gate = gate;
    coordinator->calibration_entry_gate_context =
        gate != NULL ? gate_context : NULL;
    return 1;
}

int pstvnc_h1_interaction_coordinator_take_calibration_accept(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_mpeg_cal_region_t *committed_region)
{
    if (coordinator == NULL || committed_region == NULL ||
        !coordinator->accepted_calibration_pending)
        return 0;

    *committed_region = coordinator->accepted_calibration_region;
    coordinator->accepted_calibration_pending = 0;
    return 1;
}

int pstvnc_h1_interaction_coordinator_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer)
{
    pstvnc_h1_interaction_coordinator_t *coordinator =
        (pstvnc_h1_interaction_coordinator_t *)context;

    if (coordinator == NULL || framebuffer == NULL || !framebuffer->valid)
        return 0;

    coordinator->current_framebuffer = framebuffer;
    return h1_interaction_present_current_frame(coordinator, 1);
}

int pstvnc_h1_interaction_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_interaction_coordinator_t *coordinator =
        (pstvnc_h1_interaction_coordinator_t *)context;

    if (coordinator == NULL ||
        session == NULL ||
        session->state != PSTVNC_RFB_SESSION_READY ||
        coordinator->current_framebuffer == NULL)
        return 0;

    if (!coordinator->input_started &&
        !h1_interaction_start_input(coordinator, session))
        return 0;

    if (pstvnc_input_runtime_last_error(&coordinator->input_runtime) !=
        PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        return 0;

    for (;;) {
        pstvnc_input_event_t event;
        int pop_result = pstvnc_input_runtime_pop_event(
            &coordinator->input_runtime,
            &event);

        if (pop_result < 0)
            return 0;

        if (pop_result == 0)
            break;

        switch (event.type) {
            case PSTVNC_INPUT_EVENT_CONTROLLER_STATE:
                if (!h1_interaction_increment_counter(
                        &coordinator->stats.controller_state_events_consumed) ||
                    !h1_interaction_service_controller_state(
                        coordinator,
                        session,
                        &event.payload.controller_state))
                    return 0;
                break;

            case PSTVNC_INPUT_EVENT_MOUSE_UPDATE:
                if (coordinator->mouse_interpretation_suspended ||
                    !h1_interaction_publish_mouse_update(
                        coordinator,
                        session,
                        &event.payload.mouse_update) ||
                    !h1_interaction_increment_counter(
                        &coordinator->stats.mouse_update_events_consumed))
                    return 0;
                break;

            case PSTVNC_INPUT_EVENT_KEYBOARD_TAP:
                if (!h1_interaction_publish_keyboard_tap(
                        coordinator,
                        session,
                        &event.payload.keyboard_tap))
                    return 0;
                break;

            case PSTVNC_INPUT_EVENT_NONE:
            default:
                return 0;
        }
    }

    if (pstvnc_input_runtime_last_error(&coordinator->input_runtime) !=
        PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        return 0;

    /*
     * Promote a still-held desktop entry chord from the ordinary RFB service
     * cadence. IDLE RFB polls also invoke this service callback, so a static
     * desktop does not stall the monotonic 750 ms deadline.
     */
    if (!h1_interaction_service_calibration_entry_hold(
            coordinator,
            session))
        return 0;

    if (pstvnc_local_ui_needs_present(&coordinator->local_ui) &&
        !h1_interaction_present_current_frame(coordinator, 0))
        return 0;

    return h1_interaction_resume_desktop_mouse_if_ready(coordinator);
}

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_interaction_coordinator_rfb_policy(
    const pstvnc_h1_interaction_coordinator_t *coordinator)
{
    if (coordinator == NULL)
        return NULL;

    return pstvnc_h1_mpeg_calibration_interaction_binding_rfb_policy(
        &coordinator->mpeg_calibration);
}

int pstvnc_h1_interaction_coordinator_shutdown(
    pstvnc_h1_interaction_coordinator_t *coordinator)
{
    if (coordinator == NULL)
        return -1;

    if (!coordinator->input_initialized)
        return 0;

    if (pstvnc_input_runtime_shutdown(&coordinator->input_runtime) < 0)
        return -1;

    coordinator->input_initialized = 0;
    coordinator->input_started = 0;
    coordinator->mouse_interpretation_suspended = 0;
    return 0;
}
