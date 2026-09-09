/*
 * File synopsis:
 * Implements CP2L's mouse-only application-side input service over the existing
 * through-Issue-39 semantic controller runtime.
 *
 * The controller thread produces typed semantic events. This module consumes
 * them only when called by the RFB-owning main thread, maps mouse semantics to
 * native RFB pointer bits, and publishes through the already-synchronized RFB
 * session. It deliberately ignores physical controller-state meaning beyond
 * consuming those facts, because keyboard/OSK/local UI remain outside CP2L.
 */

#include "h1_rfb_input_service.h"

#include "display.h"
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
    pstvnc_h1_rfb_input_service_t *service,
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
        /*
         * Native RFB wheel motion is a momentary button pulse at the current
         * cursor while preserving ordinary click state through press/release.
         * Any partial write fails the synchronized RFB session closed.
         */
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

static int h1_rfb_input_start(
    pstvnc_h1_rfb_input_service_t *service,
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

    /*
     * Establish exact remote pointer authority before the worker can produce a
     * newer local semantic state. This matches the through-Issue-39 application
     * ordering and prevents the first controller sample from racing an unknown
     * server-side pointer state.
     */
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

void pstvnc_h1_rfb_input_service_init(
    pstvnc_h1_rfb_input_service_t *service)
{
    if (service == NULL)
        return;

    memset(service, 0, sizeof(*service));
}

int pstvnc_h1_rfb_input_service_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_rfb_input_service_t *service =
        (pstvnc_h1_rfb_input_service_t *)context;

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
                /*
                 * CP2L has no local foreground or chord action owner. Consume
                 * the physical fact so the bounded FIFO remains healthy, but do
                 * not assign it product meaning in this checkpoint.
                 */
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

            /*
             * Keyboard/OSK production is not part of this checkpoint. Seeing a
             * keyboard event would therefore indicate an unexpected ownership
             * expansion and fails closed rather than silently broadening CP2L.
             */
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

int pstvnc_h1_rfb_input_service_shutdown(
    pstvnc_h1_rfb_input_service_t *service)
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
    return 0;
}
