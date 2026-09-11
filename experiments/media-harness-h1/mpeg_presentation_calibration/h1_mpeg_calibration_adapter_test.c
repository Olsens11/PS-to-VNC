#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "h1_mpeg_calibration_adapter.h"

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

int main(void)
{
    pstvnc_h1_mpeg_calibration_adapter_t adapter;
    pstvnc_h1_mpeg_calibration_adapter_result_t result;
    pstvnc_controller_state_t state;

    pstvnc_h1_mpeg_calibration_adapter_init(&adapter, 704, 462);

    /* Ordinary desktop SELECT alone remains available to the normal router. */
    state = controller(
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);
    assert(pstvnc_h1_mpeg_calibration_adapter_service_controller(
        &adapter, &state, &result));
    assert(!result.calibration_effects.enter_foreground);
    assert(!result.consume_controller_state);
    assert(!result.freeze_rfb_visuals);

    /* Held START+SELECT enters even when only SELECT has a fresh edge. */
    state = controller(
        PSTVNC_CONTROLLER_BUTTON_START | PSTVNC_CONTROLLER_BUTTON_SELECT,
        PSTVNC_CONTROLLER_BUTTON_SELECT,
        0);
    assert(pstvnc_h1_mpeg_calibration_adapter_service_controller(
        &adapter, &state, &result));
    assert(result.calibration_effects.enter_foreground);
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);

    /* Calibration navigation is consumed and keeps RFB visuals frozen. */
    state = controller(
        PSTVNC_CONTROLLER_BUTTON_R1 | PSTVNC_CONTROLLER_BUTTON_RIGHT,
        0,
        PSTVNC_CONTROLLER_BUTTON_START | PSTVNC_CONTROLLER_BUTTON_SELECT);
    assert(pstvnc_h1_mpeg_calibration_adapter_service_controller(
        &adapter, &state, &result));
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);

    /* Circle cancels but the cancel sample remains quarantined. */
    state = controller(
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        0);
    assert(pstvnc_h1_mpeg_calibration_adapter_service_controller(
        &adapter, &state, &result));
    assert(result.calibration_effects.cancelled);
    assert(result.calibration_effects.leave_foreground);
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);

    /* Held Circle still belongs to calibration release quarantine. */
    state = controller(PSTVNC_CONTROLLER_BUTTON_CIRCLE, 0, 0);
    assert(pstvnc_h1_mpeg_calibration_adapter_service_controller(
        &adapter, &state, &result));
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);

    /* Full release completes quarantine; that release sample is consumed. */
    state = controller(0, 0, PSTVNC_CONTROLLER_BUTTON_CIRCLE);
    assert(pstvnc_h1_mpeg_calibration_adapter_service_controller(
        &adapter, &state, &result));
    assert(result.calibration_effects.release_quarantine_complete);
    assert(result.consume_controller_state);
    assert(result.freeze_rfb_visuals);

    /* The following neutral sample is ordinary desktop ownership again. */
    state = controller(0, 0, 0);
    assert(pstvnc_h1_mpeg_calibration_adapter_service_controller(
        &adapter, &state, &result));
    assert(!result.consume_controller_state);
    assert(!result.freeze_rfb_visuals);

    puts("MPEG_CALIBRATION_ADAPTER_HOST_TEST=PASS");
    return 0;
}
