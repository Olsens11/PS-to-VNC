/*
 * File synopsis:
 * Runs the application coordinator: ordered startup, Transport descriptor
 * adoption, logical RFB session service, P2-governed live request/publication
 * flow, semantic controller-input routing, presentation, typed RFB-provider
 * recovery policy, and ownership-safe cleanup.
 *
 * Concrete Transport queue/credit/thread/payload values are never chosen here;
 * the configured entry point accepts one already-validated value from its
 * eventual configuration/composition owner.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Complete application
 * coordinator"; docs/CLEAN_ARCHITECTURE.md, "Application coordinator", "Live
 * loop", and input/controller concurrency model; docs/ledge/
 * LEDGE_ARCHITECTURE_OVERLAY.md; GitHub Issue #38.
 */

#include "app.h"
#include "app_product_bindings.h"
#include "app_mpeg_product.h"

#include <stdint.h>

#include "config/media_clock_profile.h"
#include "config/mpeg_runtime_profile.h"
#include "config/rfb_runtime_profile.h"
#include "diagnostics.h"
#include "display.h"
#include "framebuffer.h"
#include "input_runtime.h"
#include "local_controller.h"
#include "local_ui_presentation.h"
#include "keyboard.h"
#include "media/clock.h"
#include "platform/ps2_graphics.h"
#include "platform/ps2_media_clock.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb_session.h"
#include "rfb/flow_policy.h"
#include "transport/bridge.h"

#define PSTVNC_APP_CONTROLLER_PORT       0
#define PSTVNC_APP_CONTROLLER_SLOT       0
#define PSTVNC_APP_IDLE_POLL_DELAY_US    1000u

/*
 * These buffers have deliberately different authority. remote_pixels backs the
 * one CPU-side image trusted to represent the complete VNC desktop. gs_pixels
 * is disposable presentation data with the GS alpha bit added; display
 * conversion must never modify remote authority in place. The 128-byte
 * alignment satisfies the PS2 DMA/cache-facing presentation path.
 */
static uint16_t remote_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));
static uint16_t gs_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));
static uint16_t mpeg_calibration_frozen_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));
static uint16_t mpeg_calibration_work_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
    __attribute__((aligned(128)));
static uint16_t local_overlay_pixels[
    PSTVNC_OSK_SURFACE_PIXEL_COUNT]
    __attribute__((aligned(128)));

/*
 * Application-owned pointer state that is known to have completed publication
 * to the remote RFB peer.
 *
 * click_buttons deliberately remains in semantic mouse vocabulary because this
 * state is the authority a later libpad handoff/rebase must feed back into the
 * input interpreter. Native RFB button bits exist only during serialization.
 */
typedef struct app_published_pointer_state {
    unsigned int cursor_x;
    unsigned int cursor_y;
    unsigned char click_buttons;
} app_published_pointer_state_t;

static void send_diagnostic_literal(
    int diagnostics_ready,
    const char *text,
    size_t length)
{
    if (diagnostics_ready)
        (void)pstvnc_diagnostics_send(text, length);
}

static int map_semantic_clicks_to_rfb_buttons(
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

static int map_wheel_direction_to_rfb_button(
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

static int publish_semantic_mouse_update(
    pstvnc_rfb_session_t *session,
    const pstvnc_mouse_update_t *mouse_update,
    app_published_pointer_state_t *published_pointer)
{
    uint8_t ordinary_rfb_buttons;
    uint8_t wheel_rfb_button;

    if (session == NULL ||
        mouse_update == NULL ||
        published_pointer == NULL ||
        mouse_update->cursor_x > UINT16_MAX ||
        mouse_update->cursor_y > UINT16_MAX)
        return 0;

    if (!map_semantic_clicks_to_rfb_buttons(
            mouse_update->click_buttons,
            &ordinary_rfb_buttons))
        return 0;

    if (!map_wheel_direction_to_rfb_button(
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

        published_pointer->cursor_x = mouse_update->cursor_x;
        published_pointer->cursor_y = mouse_update->cursor_y;
        published_pointer->click_buttons = mouse_update->click_buttons;
    }

    if (wheel_rfb_button != 0) {
        /*
         * RFB wheel motion is a momentary button pulse at the current pointer
         * position. Preserve any ordinary held buttons through both messages.
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

        published_pointer->cursor_x = mouse_update->cursor_x;
        published_pointer->cursor_y = mouse_update->cursor_y;
        published_pointer->click_buttons = mouse_update->click_buttons;
    }

    return 1;
}

static int publish_semantic_keyboard_tap(
    pstvnc_rfb_session_t *session,
    const pstvnc_keyboard_tap_t *keyboard_tap)
{
    pstvnc_keyboard_sequence_t sequence;
    unsigned int event_index;

    if (session == NULL || keyboard_tap == NULL)
        return 0;

    if (!pstvnc_keyboard_build_tap_sequence(
            keyboard_tap->keysym,
            keyboard_tap->modifiers,
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

    return 1;
}

/*
 * Present the current authoritative remote desktop with the current PS2-local
 * foreground surface, if any.
 */
static int present_current_application_frame(
    const pstvnc_framebuffer_t *framebuffer,
    int remote_frame_changed,
    pstvnc_local_ui_t *local_ui,
    const pstvnc_osk_t *osk)
{
    pstvnc_local_ui_presentation_t local_presentation;
    pstvnc_ps2_graphics_overlay_t platform_overlay;
    const pstvnc_ps2_graphics_overlay_t *platform_overlay_ptr = NULL;

    if (framebuffer == NULL ||
        local_ui == NULL ||
        osk == NULL ||
        !framebuffer->valid)
        return 0;

    if (remote_frame_changed) {
        if (!pstvnc_display_prepare_gs16(
                framebuffer,
                gs_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT))
            return 0;
    }

    if (!pstvnc_local_ui_prepare_presentation(
            local_ui,
            osk,
            local_overlay_pixels,
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
            gs_pixels,
            PSTVNC_DISPLAY_PIXEL_COUNT,
            platform_overlay_ptr) < 0)
        return 0;

    if (pstvnc_local_ui_needs_present(local_ui)) {
        if (!pstvnc_local_ui_mark_presented(
                local_ui,
                local_presentation.generation))
            return 0;
    }

    return 1;
}

/*
 * Service exactly the request P2 currently permits at the ordinary live
 * scheduling boundary. Inspection is non-mutating; policy accounting advances
 * only after the RFB owner confirms serialization succeeded.
 *
 * HOLD is a successful no-op. A send failure preserves the RFB session's typed
 * error for the existing Application recovery classifier. If policy accounting
 * rejects a request after serialization, fail closed rather than inventing
 * repaired debt: the wire request is real and the local authority is invalid.
 */
static int service_rfb_flow_request(
    pstvnc_rfb_session_t *session,
    pstvnc_rfb_flow_policy_t *flow_policy)
{
    pstvnc_rfb_flow_request_t request;
    int incremental;

    if (session == NULL || flow_policy == NULL)
        return 0;

    request = pstvnc_rfb_flow_policy_next_request(flow_policy);

    switch (request) {
        case PSTVNC_RFB_FLOW_REQUEST_HOLD:
            return 1;

        case PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL:
            incremental = 1;
            break;

        case PSTVNC_RFB_FLOW_REQUEST_FULL:
            incremental = 0;
            break;

        default:
            return 0;
    }

    if (!pstvnc_rfb_session_request_update(session, incremental))
        return 0;

    return pstvnc_rfb_flow_policy_record_request_sent(
        flow_policy,
        request);
}

static int neutralize_published_pointer_for_local_foreground(
    pstvnc_rfb_session_t *session,
    app_published_pointer_state_t *published_pointer)
{
    if (session == NULL ||
        published_pointer == NULL ||
        published_pointer->cursor_x > UINT16_MAX ||
        published_pointer->cursor_y > UINT16_MAX)
        return 0;

    if (published_pointer->click_buttons == 0)
        return 1;

    if (!pstvnc_rfb_session_send_pointer_event(
            session,
            0,
            (uint16_t)published_pointer->cursor_x,
            (uint16_t)published_pointer->cursor_y))
        return 0;

    published_pointer->click_buttons = 0;
    return 1;
}

static int apply_osk_activation_result(
    pstvnc_rfb_session_t *session,
    pstvnc_local_ui_t *local_ui,
    const pstvnc_osk_activation_t *activation)
{
    if (session == NULL || local_ui == NULL || activation == NULL)
        return 0;

    if (activation->local_state_changed) {
        if (!pstvnc_local_ui_mark_local_change(local_ui))
            return 0;
    }

    if (activation->produced_keyboard_tap) {
        if (!publish_semantic_keyboard_tap(
                session,
                &activation->keyboard_tap))
            return 0;
    }

    return 1;
}

static int open_osk_foreground(
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_rfb_session_t *session,
    app_published_pointer_state_t *published_pointer,
    pstvnc_local_ui_t *local_ui,
    pstvnc_osk_t *osk,
    int *mouse_interpretation_suspended)
{
    if (input_runtime == NULL ||
        session == NULL ||
        published_pointer == NULL ||
        local_ui == NULL ||
        osk == NULL ||
        mouse_interpretation_suspended == NULL)
        return 0;

    if (local_ui->foreground != PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP ||
        pstvnc_local_ui_input_is_quarantined(local_ui) ||
        *mouse_interpretation_suspended)
        return 0;

    if (pstvnc_input_runtime_suspend_mouse_interpretation(input_runtime) < 0)
        return 0;

    *mouse_interpretation_suspended = 1;

    if (!neutralize_published_pointer_for_local_foreground(
            session,
            published_pointer))
        return 0;

    if (pstvnc_input_runtime_rebase_suspended_mouse_state(
            input_runtime,
            published_pointer->cursor_x,
            published_pointer->cursor_y,
            published_pointer->click_buttons) < 0)
        return 0;

    pstvnc_osk_reset_for_open(osk);

    if (!pstvnc_local_ui_open_osk(local_ui))
        return 0;

    return 1;
}

static int apply_local_controller_action(
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_rfb_session_t *session,
    app_published_pointer_state_t *published_pointer,
    pstvnc_local_ui_t *local_ui,
    pstvnc_osk_t *osk,
    int *mouse_interpretation_suspended,
    pstvnc_local_controller_action_t action)
{
    pstvnc_osk_activation_t activation;

    if (input_runtime == NULL ||
        session == NULL ||
        published_pointer == NULL ||
        local_ui == NULL ||
        osk == NULL ||
        mouse_interpretation_suspended == NULL)
        return 0;

    switch (action) {
        case PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK:
            return open_osk_foreground(
                input_runtime,
                session,
                published_pointer,
                local_ui,
                osk,
                mouse_interpretation_suspended);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_LEFT:
            if (!pstvnc_osk_move_horizontal(osk, -1))
                return 0;
            return pstvnc_local_ui_mark_local_change(local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_RIGHT:
            if (!pstvnc_osk_move_horizontal(osk, 1))
                return 0;
            return pstvnc_local_ui_mark_local_change(local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_UP:
            if (!pstvnc_osk_move_vertical(osk, -1))
                return 0;
            return pstvnc_local_ui_mark_local_change(local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_DOWN:
            if (!pstvnc_osk_move_vertical(osk, 1))
                return 0;
            return pstvnc_local_ui_mark_local_change(local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_TOGGLE_SHIFT:
            if (!pstvnc_osk_toggle_shift_modifier(osk))
                return 0;
            return pstvnc_local_ui_mark_local_change(local_ui);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_ACTIVATE_SELECTED:
            if (!pstvnc_osk_activate_selected(osk, &activation))
                return 0;
            return apply_osk_activation_result(session, local_ui, &activation);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_BACKSPACE:
            if (!pstvnc_osk_activate_direct_key(
                    osk,
                    PSTVNC_KEYBOARD_KEYSYM_BACKSPACE,
                    &activation))
                return 0;
            return apply_osk_activation_result(session, local_ui, &activation);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_ENTER:
            if (!pstvnc_osk_activate_direct_key(
                    osk,
                    PSTVNC_KEYBOARD_KEYSYM_ENTER,
                    &activation))
                return 0;
            return apply_osk_activation_result(session, local_ui, &activation);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_TAB:
            if (!pstvnc_osk_activate_direct_key(
                    osk,
                    PSTVNC_KEYBOARD_KEYSYM_TAB,
                    &activation))
                return 0;
            return apply_osk_activation_result(session, local_ui, &activation);

        case PSTVNC_LOCAL_CONTROLLER_ACTION_CLOSE_OSK:
            if (local_ui->foreground != PSTVNC_LOCAL_UI_FOREGROUND_OSK)
                return 0;
            (void)pstvnc_osk_clear_modifiers(osk);
            return pstvnc_local_ui_close_osk(local_ui);

        default:
            return 0;
    }
}

static int service_controller_state(
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_app_mpeg_product_t *mpeg_product,
    pstvnc_rfb_session_t *session,
    app_published_pointer_state_t *published_pointer,
    pstvnc_local_controller_t *local_controller,
    pstvnc_local_ui_t *local_ui,
    pstvnc_osk_t *osk,
    int *mouse_interpretation_suspended,
    const pstvnc_controller_state_t *controller_state,
    int *mpeg_session_failure)
{
    pstvnc_local_controller_result_t result;
    unsigned int action_index;
    int product_consumed = 0;

    if (input_runtime == NULL ||
        mpeg_product == NULL ||
        session == NULL ||
        published_pointer == NULL ||
        local_controller == NULL ||
        local_ui == NULL ||
        osk == NULL ||
        mouse_interpretation_suspended == NULL ||
        controller_state == NULL ||
        mpeg_session_failure == NULL)
        return 0;

    if (pstvnc_app_mpeg_product_service_controller(
            mpeg_product,
            controller_state,
            &product_consumed) != PSTVNC_APP_MPEG_PRODUCT_OK) {
        *mpeg_session_failure = 1;
        return 0;
    }

    if (product_consumed)
        return 1;

    if (!pstvnc_local_controller_route(
            local_controller,
            local_ui->foreground,
            pstvnc_local_ui_input_is_quarantined(local_ui),
            controller_state,
            &result))
        return 0;

    for (action_index = 0;
         action_index < result.action_count;
         action_index++) {
        if (result.actions[action_index] ==
                PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK &&
            pstvnc_input_runtime_set_product_action_desktop_eligible(
                input_runtime,
                0) < 0)
            return 0;

        if (!apply_local_controller_action(
                input_runtime,
                session,
                published_pointer,
                local_ui,
                osk,
                mouse_interpretation_suspended,
                result.actions[action_index]))
            return 0;
    }

    if (pstvnc_local_ui_input_is_quarantined(local_ui) &&
        pstvnc_local_controller_release_is_proven(local_controller)) {
        if (!pstvnc_local_ui_complete_input_quarantine(local_ui))
            return 0;
    }

    return 1;
}

static int resume_desktop_mouse_if_ready(
    pstvnc_input_runtime_t *input_runtime,
    const pstvnc_local_ui_t *local_ui,
    int *mouse_interpretation_suspended)
{
    if (input_runtime == NULL ||
        local_ui == NULL ||
        mouse_interpretation_suspended == NULL)
        return 0;

    if (!*mouse_interpretation_suspended)
        return 1;

    if (local_ui->foreground != PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP)
        return 1;

    if (pstvnc_local_ui_input_is_quarantined(local_ui))
        return 1;

    if (pstvnc_local_ui_needs_present(local_ui))
        return 1;

    if (pstvnc_input_runtime_resume_mouse_interpretation(input_runtime) < 0)
        return 0;

    *mouse_interpretation_suspended = 0;
    return 1;
}

static int service_semantic_input_events(
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_app_mpeg_product_t *mpeg_product,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    pstvnc_rfb_session_t *session,
    app_published_pointer_state_t *published_pointer,
    pstvnc_local_controller_t *local_controller,
    pstvnc_local_ui_t *local_ui,
    pstvnc_osk_t *osk,
    int *mouse_interpretation_suspended,
    int *mpeg_session_failure)
{
    if (input_runtime == NULL ||
        mpeg_product == NULL ||
        rfb_flow_policy == NULL ||
        session == NULL ||
        published_pointer == NULL ||
        local_controller == NULL ||
        local_ui == NULL ||
        osk == NULL ||
        mouse_interpretation_suspended == NULL ||
        mpeg_session_failure == NULL)
        return 0;

    *mpeg_session_failure = 0;

    if (pstvnc_input_runtime_last_error(input_runtime) !=
        PSTVNC_INPUT_RUNTIME_ERROR_NONE)
        return 0;

    for (;;) {
        pstvnc_input_event_t event;
        int pop_result = pstvnc_input_runtime_pop_event(input_runtime, &event);

        if (pop_result < 0)
            return 0;
        if (pop_result == 0)
            break;

        switch (event.type) {
            case PSTVNC_INPUT_EVENT_CONTROLLER_STATE:
                if (!service_controller_state(
                        input_runtime,
                        mpeg_product,
                        session,
                        published_pointer,
                        local_controller,
                        local_ui,
                        osk,
                        mouse_interpretation_suspended,
                        &event.payload.controller_state,
                        mpeg_session_failure))
                    return 0;
                break;

            case PSTVNC_INPUT_EVENT_MOUSE_UPDATE:
                if (*mouse_interpretation_suspended)
                    return 0;

                if (!publish_semantic_mouse_update(
                        session,
                        &event.payload.mouse_update,
                        published_pointer))
                    return 0;
                break;

            case PSTVNC_INPUT_EVENT_KEYBOARD_TAP:
                if (!publish_semantic_keyboard_tap(
                        session,
                        &event.payload.keyboard_tap))
                    return 0;
                break;

            case PSTVNC_INPUT_EVENT_PRODUCT_ACTION: {
                pstvnc_app_mpeg_product_result_t product_result;

                if (pstvnc_input_runtime_set_product_action_desktop_eligible(
                        input_runtime,
                        0) < 0)
                    return 0;

                product_result = pstvnc_app_mpeg_product_route_action(
                    mpeg_product,
                    event.payload.product_action,
                    rfb_flow_policy,
                    input_runtime,
                    session,
                    local_ui,
                    published_pointer->cursor_x,
                    published_pointer->cursor_y,
                    &published_pointer->click_buttons,
                    gs_pixels,
                    PSTVNC_DISPLAY_PIXEL_COUNT);

                if (product_result ==
                        PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE) {
                    *mpeg_session_failure = 1;
                    return 0;
                }

                if (product_result != PSTVNC_APP_MPEG_PRODUCT_OK)
                    return 0;
                break;
            }

            case PSTVNC_INPUT_EVENT_NONE:
            default:
                return 0;
        }
    }

    return pstvnc_input_runtime_last_error(input_runtime) ==
        PSTVNC_INPUT_RUNTIME_ERROR_NONE;
}

static int sync_product_action_desktop_eligibility(
    pstvnc_input_runtime_t *input_runtime,
    const pstvnc_app_mpeg_product_t *mpeg_product,
    const pstvnc_local_ui_t *local_ui)
{
    if (input_runtime == NULL || mpeg_product == NULL || local_ui == NULL)
        return 0;

    return pstvnc_input_runtime_set_product_action_desktop_eligible(
        input_runtime,
        pstvnc_app_mpeg_product_desktop_action_eligible(
            mpeg_product,
            local_ui)) == 0;
}

static int retire_attempt_owners(
    pstvnc_input_runtime_t *input_runtime,
    int *input_runtime_ready,
    pstvnc_app_mpeg_product_t *mpeg_product,
    int mpeg_product_ready,
    pstvnc_ps2_media_clock_binding_t *media_clock_binding,
    int *media_clock_binding_active,
    int *media_clock_binding_release_failed,
    int *transport_session_active)
{
    int abort_ready = 0;
    int has_started_mpeg = 0;

    if (input_runtime == NULL ||
        input_runtime_ready == NULL ||
        media_clock_binding == NULL ||
        media_clock_binding_active == NULL ||
        media_clock_binding_release_failed == NULL ||
        transport_session_active == NULL)
        return 0;

    if (*input_runtime_ready) {
        if (pstvnc_input_runtime_shutdown(input_runtime) != 0)
            return 0;
        *input_runtime_ready = 0;
    }

    has_started_mpeg =
        mpeg_product_ready &&
        mpeg_product != NULL &&
        pstvnc_app_mpeg_product_has_started_run(mpeg_product);

    if (has_started_mpeg && *transport_session_active) {
        if (pstvnc_transport_session_begin_abort() !=
                PSTVNC_TRANSPORT_OK)
            return 0;

        while (!abort_ready) {
            if (pstvnc_app_mpeg_product_service_session_abort(
                    mpeg_product,
                    &abort_ready) != PSTVNC_APP_MPEG_PRODUCT_OK)
                return 0;

            if (!abort_ready &&
                pstvnc_ps2_system_delay_us(
                    PSTVNC_APP_IDLE_POLL_DELAY_US) < 0)
                return 0;
        }
    }

    if (*media_clock_binding_active) {
        if (pstvnc_ps2_media_clock_binding_release(
                media_clock_binding) < 0)
            *media_clock_binding_release_failed = 1;

        *media_clock_binding_active = 0;
    }

    if (*transport_session_active) {
        pstvnc_transport_result_t close_result =
            has_started_mpeg
                ? pstvnc_transport_session_close()
                : pstvnc_transport_session_abort();

        if (close_result != PSTVNC_TRANSPORT_OK)
            return 0;

        *transport_session_active = 0;
    }

    return !*media_clock_binding_release_failed;
}

int pstvnc_app_run_with_session_profiles(
    const pstvnc_transport_session_config_t *transport_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_transport_config,
    const pstvnc_config_media_clock_profile_t *media_clock_profile)
{
    static const char net_ready[] = "PSTVNC_STAGE NET_READY";
    static const char gs_ready[] = "PSTVNC_STAGE GS_READY";
    static const char desktop_ready[] = "PSTVNC_STAGE DESKTOP_READY";
    static const char input_ready[] = "PSTVNC_STAGE INPUT_READY";
    static const char fatal[] = "PSTVNC_STAGE FATAL";
    static pstvnc_input_runtime_t input_runtime;

    pstvnc_app_product_bindings_snapshot_t desired_product_bindings;
    int graphics_ready = 0;
    int diagnostics_ready = 0;

    if (transport_config == NULL ||
        mpeg_transport_config == NULL ||
        media_clock_profile == NULL)
        return -1;

    if (pstvnc_ps2_system_prepare_iop() < 0)
        goto fail_resident;

    if (pstvnc_ps2_network_init() < 0)
        goto fail_resident;

    if (pstvnc_ps2_network_wait_link() < 0)
        goto fail_resident;

    /*
     * R32 acquires desired binding authority once per resident process, after
     * the private link exists and before the first physical PSTV attempt.
     * Fetch/parse failures are deliberately nonfatal zero-binding fallbacks.
     *
     * The snapshot is not installed into Input in R32. It remains immutable
     * resident Application authority across all provider-replacement attempts.
     */
    if (!pstvnc_app_product_bindings_acquire(&desired_product_bindings))
        goto fail_resident;

    if (pstvnc_diagnostics_init() == 0)
        diagnostics_ready = 1;

    /*
     * IOP/network/link, the R32 desired-binding snapshot, diagnostics, and an
     * already-created graphics context are resident application ownership.
     * Every pass below is one ordinary RFB
     * attempt with fresh network, Transport/Q4, media-clock, RFB, framebuffer
     * and input authority. A typed provider failure is the only condition that
     * re-enters this admission path.
     */
    for (;;) {
        pstvnc_framebuffer_t framebuffer;
        pstvnc_local_controller_t local_controller;
        pstvnc_local_ui_t local_ui;
        pstvnc_osk_t osk;
        pstvnc_rfb_session_t session;
        pstvnc_rfb_flow_policy_t rfb_flow_policy;
        pstvnc_transport_access_t transport_access;
        pstvnc_app_mpeg_product_t mpeg_product;
        app_published_pointer_state_t published_pointer;
        pstvnc_ps2_media_clock_binding_t media_clock_binding =
            PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER;
        pstvnc_media_clock_sync_t media_clock_sync;
        pstvnc_media_clock_t media_clock;
        uint32_t media_clock_ticks_per_second = 0u;
        int media_clock_armed = 1;
        int socket_fd = -1;
        int transport_session_active = 0;
        int media_clock_binding_active = 0;
        int media_clock_binding_release_failed = 0;
        int input_runtime_ready = 0;
        int mpeg_product_ready = 0;
        int mouse_interpretation_suspended = 0;
        int mpeg_session_failure = 0;

        /*
         * Application owns the freshly connected descriptor only until
         * Transport successfully adopts it. Every replacement attempt obtains a
         * new descriptor and therefore a new Q4 Wire Session; no old descriptor,
         * access ticket, channel state, media-clock synchronization authority or
         * Pi attachment can be rebound here.
         */
        socket_fd = pstvnc_ps2_network_connect_pstv();
        if (socket_fd < 0)
            goto attempt_fatal;

        if (pstvnc_transport_session_open_with_mpeg(
                &socket_fd,
                transport_config,
                mpeg_transport_config) != PSTVNC_TRANSPORT_OK)
            goto attempt_fatal;

        transport_session_active = 1;

        memset(&transport_access, 0, sizeof(transport_access));
        if (pstvnc_transport_access_acquire(
                &transport_access) != PSTVNC_TRANSPORT_OK)
            goto attempt_fatal;

        /*
         * R26 binding storage is one-session authority. This declaration has a
         * fresh C object lifetime on every loop entry; after release this exact
         * object is discarded rather than reset/reinitialized for a successor
         * Wire Session.
         */
        if (pstvnc_ps2_media_clock_binding_init(
                &media_clock_binding) < 0)
            goto attempt_fatal;

        media_clock_binding_active = 1;

        if (pstvnc_ps2_media_clock_binding_sync(
                &media_clock_binding,
                &media_clock_sync) < 0)
            goto attempt_fatal;

        if (pstvnc_ps2_media_clock_binding_tick_rate(
                &media_clock_binding,
                &media_clock_ticks_per_second) < 0)
            goto attempt_fatal;

        if (pstvnc_media_clock_init(
                &media_clock,
                media_clock_profile,
                media_clock_ticks_per_second,
                &media_clock_sync) != PSTVNC_MEDIA_CLOCK_OK)
            goto attempt_fatal;

        if (pstvnc_media_clock_is_armed(
                &media_clock,
                &media_clock_armed) != PSTVNC_MEDIA_CLOCK_OK ||
            media_clock_armed)
            goto attempt_fatal;

        send_diagnostic_literal(
            diagnostics_ready,
            net_ready,
            sizeof(net_ready) - 1u);

        /*
         * Reinitialize framebuffer authority for every attempt. A failed
         * attempt's valid/dirty state is never inherited by the replacement
         * RFB parser.
         */
        if (!pstvnc_framebuffer_init(
                &framebuffer,
                remote_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT))
            goto attempt_fatal;

        if (!pstvnc_framebuffer_set_geometry(
                &framebuffer,
                PSTVNC_DISPLAY_WIDTH,
                PSTVNC_DISPLAY_HEIGHT))
            goto attempt_fatal;

        /*
         * Graphics is resident process state rather than RFB-attempt state.
         * Keep a proven graphics owner alive across an ordinary provider
         * replacement while every remote-authority owner below is rebuilt.
         */
        if (!graphics_ready) {
            if (pstvnc_ps2_graphics_init() < 0)
                goto attempt_fatal;

            graphics_ready = 1;

            send_diagnostic_literal(
                diagnostics_ready,
                gs_ready,
                sizeof(gs_ready) - 1u);
        }

        pstvnc_local_controller_init(&local_controller);
        pstvnc_local_ui_init(&local_ui);
        pstvnc_osk_reset_for_open(&osk);
        pstvnc_rfb_session_init(&session);

        if (!pstvnc_rfb_session_start(
                &session,
                PSTVNC_DISPLAY_WIDTH,
                PSTVNC_DISPLAY_HEIGHT))
            goto attempt_failed;

        if (!pstvnc_rfb_session_receive_initial_frame(
                &session,
                &framebuffer))
            goto attempt_failed;

        /*
         * The startup full frame is the existing RFB-session authority. P2
         * begins only after that proof and is attempt-local: provider/Wire
         * replacement must never inherit outstanding/freeze/FULL debt.
         */
        pstvnc_rfb_flow_policy_init(&rfb_flow_policy);

        if (!pstvnc_app_mpeg_product_init(
                &mpeg_product,
                &transport_access,
                &media_clock,
                PSTVNC_DISPLAY_WIDTH,
                PSTVNC_DISPLAY_HEIGHT,
                mpeg_calibration_frozen_pixels,
                mpeg_calibration_work_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT))
            goto attempt_fatal;

        mpeg_product_ready = 1;

        if (!present_current_application_frame(
                &framebuffer,
                1,
                &local_ui,
                &osk))
            goto attempt_fatal;

        send_diagnostic_literal(
            diagnostics_ready,
            desktop_ready,
            sizeof(desktop_ready) - 1u);

        if (pstvnc_input_runtime_init(
                &input_runtime,
                PSTVNC_DISPLAY_WIDTH,
                PSTVNC_DISPLAY_HEIGHT,
                PSTVNC_APP_CONTROLLER_PORT,
                PSTVNC_APP_CONTROLLER_SLOT) < 0)
            goto attempt_fatal;

        input_runtime_ready = 1;

        if (pstvnc_input_runtime_set_product_action_bindings(
                &input_runtime,
                desired_product_bindings.desired.bindings,
                desired_product_bindings.desired.binding_count) < 0)
            goto attempt_fatal;

        if (!sync_product_action_desktop_eligibility(
                &input_runtime,
                &mpeg_product,
                &local_ui))
            goto attempt_fatal;

        /*
         * Published input authority is reconstructed, not inherited. Every
         * attempt starts from a fresh neutral-center publication after fresh
         * input-runtime initialization.
         */
        published_pointer.cursor_x = PSTVNC_DISPLAY_WIDTH / 2u;
        published_pointer.cursor_y = PSTVNC_DISPLAY_HEIGHT / 2u;
        published_pointer.click_buttons = 0;

        if (!pstvnc_rfb_session_send_pointer_event(
                &session,
                0,
                (uint16_t)published_pointer.cursor_x,
                (uint16_t)published_pointer.cursor_y))
            goto attempt_failed;

        if (pstvnc_input_runtime_start(&input_runtime) < 0)
            goto attempt_fatal;

        send_diagnostic_literal(
            diagnostics_ready,
            input_ready,
            sizeof(input_ready) - 1u);

        if (!service_rfb_flow_request(&session, &rfb_flow_policy))
            goto attempt_failed;

        for (;;) {
            pstvnc_rfb_session_receive_result_t receive_result;
            uint64_t current_tick = 0u;

            if (!sync_product_action_desktop_eligibility(
                    &input_runtime,
                    &mpeg_product,
                    &local_ui))
                goto attempt_fatal;

            mpeg_session_failure = 0;
            if (!service_semantic_input_events(
                    &input_runtime,
                    &mpeg_product,
                    &rfb_flow_policy,
                    &session,
                    &published_pointer,
                    &local_controller,
                    &local_ui,
                    &osk,
                    &mouse_interpretation_suspended,
                    &mpeg_session_failure))
                goto attempt_failed;

            if (!sync_product_action_desktop_eligibility(
                    &input_runtime,
                    &mpeg_product,
                    &local_ui))
                goto attempt_fatal;

            if (pstvnc_local_ui_needs_present(&local_ui)) {
                if (!present_current_application_frame(
                        &framebuffer,
                        0,
                        &local_ui,
                        &osk))
                    goto attempt_fatal;
            }

            if (!resume_desktop_mouse_if_ready(
                    &input_runtime,
                    &local_ui,
                    &mouse_interpretation_suspended))
                goto attempt_fatal;

            if (pstvnc_app_mpeg_product_has_started_run(&mpeg_product)) {
                if (pstvnc_ps2_media_clock_binding_current_tick(
                        &media_clock_binding,
                        &current_tick) < 0)
                    goto attempt_fatal;

                if (pstvnc_app_mpeg_product_service_live(
                        &mpeg_product,
                        current_tick) != PSTVNC_APP_MPEG_PRODUCT_OK) {
                    mpeg_session_failure = 1;
                    goto attempt_failed;
                }
            }

            receive_result = pstvnc_rfb_session_try_receive_update(
                &session,
                &framebuffer);

            if (receive_result == PSTVNC_RFB_SESSION_RECEIVE_FAILED)
                goto attempt_failed;

            if (receive_result == PSTVNC_RFB_SESSION_RECEIVE_IDLE) {
                if (pstvnc_ps2_system_delay_us(
                        PSTVNC_APP_IDLE_POLL_DELAY_US) < 0)
                    goto attempt_fatal;
                continue;
            }

            if (receive_result != PSTVNC_RFB_SESSION_RECEIVE_UPDATE)
                goto attempt_fatal;

            /*
             * The parser has completed exactly one live update response.
             * Retire that one P2 obligation before considering presentation or
             * any successor request. IDLE never reaches this accounting edge.
             */
            if (!pstvnc_rfb_flow_policy_record_update_complete(
                    &rfb_flow_policy))
                goto attempt_fatal;

            if (!framebuffer.valid)
                goto attempt_fatal;

            /*
             * Remote framebuffer truth already advanced in the RFB owner.
             * Publication is a separate Application composition decision.
             * Ordinary behavior remains thawed while P2 is authoritative.
             * The dormant session media clock is intentionally unrelated to
             * ordinary RFB publication and remains unarmed.
             */
            if (framebuffer.dirty &&
                pstvnc_rfb_flow_policy_allows_remote_publication(
                    &rfb_flow_policy)) {
                if (!present_current_application_frame(
                        &framebuffer,
                        1,
                        &local_ui,
                        &osk))
                    goto attempt_fatal;
            }

            if (!service_rfb_flow_request(&session, &rfb_flow_policy))
                goto attempt_failed;
        }

attempt_failed:
        /*
         * This switch is the Application-owned R16B recovery-policy boundary.
         * The three R16A provider-local causes remain typed facts even though
         * the initial product policy deliberately treats them alike.
         */
        if (!mpeg_session_failure) {
            switch (session.error) {
                case PSTVNC_RFB_SESSION_ERROR_PROVIDER_CONNECT:
                case PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ:
                case PSTVNC_RFB_SESSION_ERROR_PROVIDER_WRITE:
                    break;

                default:
                    goto attempt_fatal;
            }
        }

        /*
         * Replacement is admitted only after complete reverse-order retirement:
         * input worker dormancy, exact R26 binding release, then Transport
         * abort/receiver/session retirement. Failure of any proof prevents the
         * next physical connect.
         */
        if (!retire_attempt_owners(
                &input_runtime,
                &input_runtime_ready,
                &mpeg_product,
                mpeg_product_ready,
                &media_clock_binding,
                &media_clock_binding_active,
                &media_clock_binding_release_failed,
                &transport_session_active))
            goto attempt_fatal;

        /*
         * There is no success-by-delay and no in-place component restart. The
         * next iteration creates fresh automatic binding/clock objects and must
         * independently reach its normal healthy boundary.
         */
        continue;

attempt_fatal:
        /*
         * Fatal convergence preserves the same reverse ownership order. A
         * pre-binding failure does not fabricate release authority.
         */
        if (transport_session_active) {
            (void)retire_attempt_owners(
                &input_runtime,
                &input_runtime_ready,
                &mpeg_product,
                mpeg_product_ready,
                &media_clock_binding,
                &media_clock_binding_active,
                &media_clock_binding_release_failed,
                &transport_session_active);
        } else {
            if (input_runtime_ready)
                (void)pstvnc_input_runtime_shutdown(&input_runtime);

            if (media_clock_binding_active) {
                (void)pstvnc_ps2_media_clock_binding_release(
                    &media_clock_binding);
                media_clock_binding_active = 0;
            }

            if (socket_fd >= 0)
                pstvnc_ps2_network_close(socket_fd);
        }

        goto fail_resident;
    }

fail_resident:
    send_diagnostic_literal(
        diagnostics_ready,
        fatal,
        sizeof(fatal) - 1u);

    if (graphics_ready)
        pstvnc_ps2_graphics_shutdown();

    if (diagnostics_ready)
        pstvnc_diagnostics_shutdown();

    return -1;
}

int pstvnc_app_run(void)
{
    pstvnc_transport_session_config_t transport_config;
    const pstvnc_config_mpeg_runtime_profile_t *mpeg_profile;
    pstvnc_config_media_clock_profile_t media_clock_profile;

    /*
     * Required selected authority is resolved before any IOP/network/platform
     * startup. A missing RFB projection or MPEG profile therefore owns nothing.
     * The selected R26 media-clock profile is an immutable by-value authority.
     */
    if (!pstvnc_config_rfb_runtime_profile_selected(&transport_config))
        return -1;

    mpeg_profile = pstvnc_config_mpeg_runtime_profile_selected();
    if (mpeg_profile == NULL)
        return -1;

    media_clock_profile = pstvnc_config_media_clock_profile_selected();

    return pstvnc_app_run_with_session_profiles(
        &transport_config,
        &mpeg_profile->transport,
        &media_clock_profile);
}
