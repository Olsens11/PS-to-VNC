/*
 * File synopsis:
 * Implements CP2M visible-RFB mouse plus transient L1+D-pad arrow-key input.
 *
 * Physical controller acquisition and mouse interpretation remain owned by the
 * through-Issue-39 input runtime. This main-thread service consumes typed facts,
 * establishes an explicit mouse-suspension boundary while L1 owns the D-pad,
 * and serializes pointer/key messages through the already-synchronized RFB
 * session. No second socket, RFB reader, GS owner, OSK, local UI, AUDIO, or MPEG
 * path is introduced.
 */

#include "h1_rfb_keyboard_input_service.h"

#include "display.h"
#include "h1_rfb_keyboard_chord.h"
#include "keyboard.h"
#include "mouse.h"
#include "rfb.h"

#include <stddef.h>
#include <string.h>

#define H1_RFB_INPUT_CONTROLLER_PORT 0
#define H1_RFB_INPUT_CONTROLLER_SLOT 0

static int h1_rfb_input_map_clicks(
    unsigned char semantic_clicks,
    uint8_t *rfb_buttons)
{
    uint8_t mapped = 0;

    if (rfb_buttons == NULL ||
        (semantic_clicks &
         (unsigned char)~PSTVNC_MOUSE_BUTTON_MASK) != 0)
        return 0;

    if (semantic_clicks & PSTVNC_MOUSE_BUTTON_LCLICK)
        mapped |= PSTVNC_RFB_POINTER_BUTTON_LEFT;

    if (semantic_clicks & PSTVNC_MOUSE_BUTTON_RCLICK)
        mapped |= PSTVNC_RFB_POINTER_BUTTON_RIGHT;

    *rfb_buttons = mapped;
    return 1;
}

static int h1_rfb_input_map_wheel(
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

static int h1_rfb_input_publish_mouse_update(
    pstvnc_h1_rfb_keyboard_input_service_t *service,
    pstvnc_rfb_session_t *session,
    const pstvnc_mouse_update_t *mouse_update)
{
    uint8_t ordinary_rfb_buttons;
    uint8_t wheel_rfb_button;

    if (service == NULL ||
        session == NULL ||
        mouse_update == NULL ||
        mouse_update->cursor_x > UINT16_MAX ||
        mouse_update->cursor_y > UINT16_MAX)
        return 0;

    if (service->l1_keyboard_chord_active)
        return 0;

    if (!h1_rfb_input_map_clicks(
            mouse_update->click_buttons,
            &ordinary_rfb_buttons))
        return 0;

    if (!h1_rfb_input_map_wheel(
            mouse_update->wheel_direction,
            &wheel_rfb_button))
        return 0;

    if (mouse_update->pointer_changed) {
        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                ordinary_rfb_buttons,
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y))
            return 0;

        service->published_cursor_x = mouse_update->cursor_x;
        service->published_cursor_y = mouse_update->cursor_y;
        service->published_click_buttons = mouse_update->click_buttons;

        if (service->pointer_messages_sent == UINT32_MAX)
            return 0;
        service->pointer_messages_sent++;
    }

    if (wheel_rfb_button != 0) {
        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                (uint8_t)(ordinary_rfb_buttons | wheel_rfb_button),
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y))
            return 0;

        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                ordinary_rfb_buttons,
                (uint16_t)mouse_update->cursor_x,
                (uint16_t)mouse_update->cursor_y))
            return 0;

        service->published_cursor_x = mouse_update->cursor_x;
        service->published_cursor_y = mouse_update->cursor_y;
        service->published_click_buttons = mouse_update->click_buttons;

        if (service->pointer_messages_sent > UINT32_MAX - 2u ||
            service->wheel_pulses_sent == UINT32_MAX)
            return 0;

        service->pointer_messages_sent += 2u;
        service->wheel_pulses_sent++;
    }

    return 1;
}

static int h1_rfb_keyboard_publish_tap(
    pstvnc_h1_rfb_keyboard_input_service_t *service,
    pstvnc_rfb_session_t *session,
    uint32_t keysym)
{
    pstvnc_keyboard_sequence_t sequence;
    unsigned int event_index;

    if (service == NULL || session == NULL || keysym == 0)
        return 0;

    if (!pstvnc_keyboard_build_tap_sequence(
            keysym,
            0u,
            &sequence))
        return 0;

    if (sequence.event_count == 0 ||
        sequence.event_count > PSTVNC_KEYBOARD_SEQUENCE_MAX_EVENTS)
        return 0;

    for (event_index = 0;
         event_index < sequence.event_count;
         event_index++) {

        if (!pstvnc_rfb_session_send_key_event(
                session,
                sequence.events[event_index].down,
                sequence.events[event_index].keysym))
            return 0;
    }

    if (service->keyboard_taps_sent == UINT32_MAX ||
        service->keyboard_messages_sent >
            UINT32_MAX - sequence.event_count)
        return 0;

    service->keyboard_taps_sent++;
    service->keyboard_messages_sent += sequence.event_count;
    return 1;
}

static int h1_rfb_keyboard_enter_chord(
    pstvnc_h1_rfb_keyboard_input_service_t *service,
    pstvnc_rfb_session_t *session)
{
    if (service == NULL ||
        session == NULL ||
        service->l1_keyboard_chord_active)
        return 0;

    if (pstvnc_input_runtime_suspend_mouse_interpretation(
            &service->input_runtime) < 0)
        return 0;

    if (service->published_click_buttons != 0) {
        if (!pstvnc_rfb_session_send_pointer_event(
                session,
                0,
                (uint16_t)service->published_cursor_x,
                (uint16_t)service->published_cursor_y))
            return 0;

        service->published_click_buttons = 0;

        if (service->pointer_messages_sent == UINT32_MAX)
            return 0;
        service->pointer_messages_sent++;
    }

    if (pstvnc_input_runtime_rebase_suspended_mouse_state(
            &service->input_runtime,
            service->published_cursor_x,
            service->published_cursor_y,
            service->published_click_buttons) < 0)
        return 0;

    service->l1_keyboard_chord_active = 1;

    if (service->keyboard_chord_entries == UINT32_MAX)
        return 0;
    service->keyboard_chord_entries++;

    return 1;
}

static int h1_rfb_keyboard_exit_chord(
    pstvnc_h1_rfb_keyboard_input_service_t *service)
{
    if (service == NULL || !service->l1_keyboard_chord_active)
        return 0;

    if (pstvnc_input_runtime_resume_mouse_interpretation(
            &service->input_runtime) < 0)
        return 0;

    service->l1_keyboard_chord_active = 0;

    if (service->keyboard_chord_exits == UINT32_MAX)
        return 0;
    service->keyboard_chord_exits++;

    return 1;
}

static int h1_rfb_keyboard_handle_controller_state(
    pstvnc_h1_rfb_keyboard_input_service_t *service,
    pstvnc_rfb_session_t *session,
    const pstvnc_controller_state_t *state)
{
    pstvnc_h1_rfb_keyboard_chord_result_t routed;
    unsigned int key_index;

    if (service == NULL || session == NULL || state == NULL)
        return 0;

    if (!pstvnc_h1_rfb_keyboard_chord_route(
            service->l1_keyboard_chord_active,
            state,
            &routed))
        return 0;

    if (routed.enter_chord) {
        if (!h1_rfb_keyboard_enter_chord(service, session))
            return 0;
    }

    for (key_index = 0;
         key_index < routed.keysym_count;
         key_index++) {
        if (!h1_rfb_keyboard_publish_tap(
                service,
                session,
                routed.keysyms[key_index]))
            return 0;
    }

    if (routed.exit_chord) {
        if (!h1_rfb_keyboard_exit_chord(service))
            return 0;
    }

    return 1;
}

static int h1_rfb_input_start(
    pstvnc_h1_rfb_keyboard_input_service_t *service,
    pstvnc_rfb_session_t *session)
{
    if (service == NULL || session == NULL || service->input_initialized)
        return 0;

    if (pstvnc_input_runtime_init(
            &service->input_runtime,
            PSTVNC_DISPLAY_WIDTH,
            PSTVNC_DISPLAY_HEIGHT,
            H1_RFB_INPUT_CONTROLLER_PORT,
            H1_RFB_INPUT_CONTROLLER_SLOT) < 0)
        return 0;

    service->input_initialized = 1;
    service->published_cursor_x = PSTVNC_DISPLAY_WIDTH / 2u;
    service->published_cursor_y = PSTVNC_DISPLAY_HEIGHT / 2u;
    service->published_click_buttons = 0;

    if (!pstvnc_rfb_session_send_pointer_event(
            session,
            0,
            (uint16_t)service->published_cursor_x,
            (uint16_t)service->published_cursor_y))
        return 0;

    service->pointer_messages_sent = 1u;

    if (pstvnc_input_runtime_start(
            &service->input_runtime) < 0)
        return 0;

    service->input_started = 1;
    return 1;
}

void pstvnc_h1_rfb_keyboard_input_service_init(
    pstvnc_h1_rfb_keyboard_input_service_t *service)
{
    if (service == NULL)
        return;

    memset(service, 0, sizeof(*service));
}

int pstvnc_h1_rfb_keyboard_input_service_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_rfb_keyboard_input_service_t *service =
        (pstvnc_h1_rfb_keyboard_input_service_t *)context;

    if (service == NULL ||
        session == NULL ||
        session->state != PSTVNC_RFB_SESSION_READY)
        return 0;

    if (!service->input_started) {
        if (!h1_rfb_input_start(service, session))
            return 0;
    }

    if (pstvnc_input_runtime_last_error(
            &service->input_runtime) !=
        PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        return 0;

    for (;;) {
        pstvnc_input_event_t event;
        int pop_result =
            pstvnc_input_runtime_pop_event(
                &service->input_runtime,
                &event);

        if (pop_result < 0)
            return 0;

        if (pop_result == 0)
            break;

        switch (event.type) {
            case PSTVNC_INPUT_EVENT_CONTROLLER_STATE:
                if (!h1_rfb_keyboard_handle_controller_state(
                        service,
                        session,
                        &event.payload.controller_state))
                    return 0;

                if (service->controller_state_events_consumed == UINT32_MAX)
                    return 0;
                service->controller_state_events_consumed++;
                break;

            case PSTVNC_INPUT_EVENT_MOUSE_UPDATE:
                if (!h1_rfb_input_publish_mouse_update(
                        service,
                        session,
                        &event.payload.mouse_update))
                    return 0;

                if (service->mouse_update_events_consumed == UINT32_MAX)
                    return 0;
                service->mouse_update_events_consumed++;
                break;

            case PSTVNC_INPUT_EVENT_KEYBOARD_TAP:
            case PSTVNC_INPUT_EVENT_NONE:
            default:
                return 0;
        }
    }

    return
        pstvnc_input_runtime_last_error(
            &service->input_runtime) ==
        PSTVNC_INPUT_RUNTIME_ERROR_NONE;
}

int pstvnc_h1_rfb_keyboard_input_service_shutdown(
    pstvnc_h1_rfb_keyboard_input_service_t *service)
{
    if (service == NULL)
        return -1;

    if (!service->input_initialized)
        return 0;

    if (pstvnc_input_runtime_shutdown(
            &service->input_runtime) < 0)
        return -1;

    service->input_initialized = 0;
    service->input_started = 0;
    service->l1_keyboard_chord_active = 0;
    return 0;
}
