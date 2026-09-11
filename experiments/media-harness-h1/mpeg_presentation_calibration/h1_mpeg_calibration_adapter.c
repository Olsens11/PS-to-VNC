/*
 * File synopsis:
 * Implements the H1 experiment-local translation from normalized physical
 * controller facts into portable MPEG calibration semantics and exposes the
 * coordinator-level ownership facts needed to suppress ordinary input and RFB
 * visual work while calibration owns the foreground.
 */

#include "h1_mpeg_calibration_adapter.h"

#include <stddef.h>
#include <string.h>

static uint32_t h1_mpeg_calibration_map_buttons(uint16_t buttons)
{
    uint32_t mapped = 0;

    if (buttons & PSTVNC_CONTROLLER_BUTTON_UP)
        mapped |= PSTVNC_MPEG_CAL_UP;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_RIGHT)
        mapped |= PSTVNC_MPEG_CAL_RIGHT;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_DOWN)
        mapped |= PSTVNC_MPEG_CAL_DOWN;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_LEFT)
        mapped |= PSTVNC_MPEG_CAL_LEFT;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_L2)
        mapped |= PSTVNC_MPEG_CAL_L2;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_R2)
        mapped |= PSTVNC_MPEG_CAL_R2;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_R1)
        mapped |= PSTVNC_MPEG_CAL_R1;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_START)
        mapped |= PSTVNC_MPEG_CAL_START;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_SELECT)
        mapped |= PSTVNC_MPEG_CAL_SELECT;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_TRIANGLE)
        mapped |= PSTVNC_MPEG_CAL_TRIANGLE;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_CIRCLE)
        mapped |= PSTVNC_MPEG_CAL_CIRCLE;
    if (buttons & PSTVNC_CONTROLLER_BUTTON_CROSS)
        mapped |= PSTVNC_MPEG_CAL_CROSS;

    return mapped;
}

void pstvnc_h1_mpeg_calibration_adapter_init(
    pstvnc_h1_mpeg_calibration_adapter_t *adapter,
    int canvas_width,
    int canvas_height)
{
    if (adapter == NULL)
        return;

    memset(adapter, 0, sizeof(*adapter));
    pstvnc_mpeg_calibration_init(
        &adapter->calibration,
        canvas_width,
        canvas_height);
}

int pstvnc_h1_mpeg_calibration_adapter_service_controller(
    pstvnc_h1_mpeg_calibration_adapter_t *adapter,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_h1_mpeg_calibration_adapter_result_t *result)
{
    pstvnc_mpeg_cal_input_t calibration_input;
    pstvnc_mpeg_cal_effects_t effects;
    int owned_before;
    int owned_after;

    if (adapter == NULL || controller_state == NULL || result == NULL)
        return 0;

    memset(result, 0, sizeof(*result));

    owned_before = pstvnc_mpeg_calibration_owns_foreground(
        &adapter->calibration);

    calibration_input.down = h1_mpeg_calibration_map_buttons(
        controller_state->buttons_down);
    calibration_input.pressed = h1_mpeg_calibration_map_buttons(
        controller_state->buttons_pressed);
    calibration_input.released = h1_mpeg_calibration_map_buttons(
        controller_state->buttons_released);

    effects = pstvnc_mpeg_calibration_update(
        &adapter->calibration,
        &calibration_input);

    owned_after = pstvnc_mpeg_calibration_owns_foreground(
        &adapter->calibration);

    result->calibration_effects = effects;

    /*
     * Consume the entry sample itself, all active-calibration samples, and all
     * release-quarantine samples. This prevents SELECT from also opening OSK,
     * START from becoming an unrelated action, and calibration navigation from
     * leaking into ordinary controller/mouse semantics.
     */
    result->consume_controller_state =
        effects.enter_foreground || owned_before || owned_after;

    /*
     * RFB visual quiescence starts on the entry effect and remains asserted for
     * the entire foreground ownership interval, including release quarantine.
     * The coordinator/RFB worker is responsible for honoring this only at its
     * existing complete-server-message scheduling boundary.
     */
    result->freeze_rfb_visuals =
        effects.enter_foreground || owned_before || owned_after;

    adapter->foreground_claimed = owned_after ? 1u : 0u;
    return 1;
}
