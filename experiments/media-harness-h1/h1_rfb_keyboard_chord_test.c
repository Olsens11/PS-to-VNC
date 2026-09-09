/*
 * File synopsis:
 * Host unit tests for CP2M's pure transient L1+D-pad keyboard chord router.
 */

#include "h1_rfb_keyboard_chord.h"

#include <stdio.h>
#include <string.h>

static int require(int condition, const char *label)
{
    if (!condition) {
        printf("CP2M_KEYBOARD_CHORD_TEST=FAIL detail=%s\n", label);
        return 0;
    }
    return 1;
}

int main(void)
{
    pstvnc_controller_state_t state;
    pstvnc_h1_rfb_keyboard_chord_result_t result;

    memset(&state, 0, sizeof(state));
    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_L1 |
        PSTVNC_CONTROLLER_BUTTON_UP;
    state.buttons_pressed =
        PSTVNC_CONTROLLER_BUTTON_L1 |
        PSTVNC_CONTROLLER_BUTTON_UP;

    if (!pstvnc_h1_rfb_keyboard_chord_route(0, &state, &result) ||
        !require(result.enter_chord, "enter") ||
        !require(!result.exit_chord, "no_exit") ||
        !require(result.keysym_count == 1u, "enter_one_tap") ||
        !require(result.keysyms[0] == PSTVNC_H1_KEYSYM_UP, "enter_up"))
        return 1;

    memset(&state, 0, sizeof(state));
    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_L1 |
        PSTVNC_CONTROLLER_BUTTON_UP;

    if (!pstvnc_h1_rfb_keyboard_chord_route(1, &state, &result) ||
        !require(result.keysym_count == 0u, "held_no_repeat"))
        return 1;

    memset(&state, 0, sizeof(state));
    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_L1 |
        PSTVNC_CONTROLLER_BUTTON_LEFT;
    state.buttons_pressed = PSTVNC_CONTROLLER_BUTTON_LEFT;

    if (!pstvnc_h1_rfb_keyboard_chord_route(1, &state, &result) ||
        !require(result.keysym_count == 1u, "direction_edge_one") ||
        !require(result.keysyms[0] == PSTVNC_H1_KEYSYM_LEFT, "direction_left"))
        return 1;

    memset(&state, 0, sizeof(state));
    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_L1 |
        PSTVNC_CONTROLLER_BUTTON_RIGHT;
    state.buttons_pressed = PSTVNC_CONTROLLER_BUTTON_L1;

    if (!pstvnc_h1_rfb_keyboard_chord_route(0, &state, &result) ||
        !require(result.enter_chord, "l1_enters_held_direction") ||
        !require(result.keysym_count == 1u, "l1_edge_one") ||
        !require(result.keysyms[0] == PSTVNC_H1_KEYSYM_RIGHT, "l1_edge_right"))
        return 1;

    memset(&state, 0, sizeof(state));
    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_L1 |
        PSTVNC_CONTROLLER_BUTTON_DOWN;
    state.connection_epoch_started = 1;

    if (!pstvnc_h1_rfb_keyboard_chord_route(0, &state, &result) ||
        !require(result.enter_chord, "epoch_enter") ||
        !require(result.keysym_count == 1u, "epoch_one") ||
        !require(result.keysyms[0] == PSTVNC_H1_KEYSYM_DOWN, "epoch_down"))
        return 1;

    memset(&state, 0, sizeof(state));
    state.buttons_released = PSTVNC_CONTROLLER_BUTTON_L1;

    if (!pstvnc_h1_rfb_keyboard_chord_route(1, &state, &result) ||
        !require(result.exit_chord, "release_exit") ||
        !require(result.keysym_count == 0u, "release_no_tap"))
        return 1;

    printf("CP2M_KEYBOARD_CHORD_TEST=PASS\n");
    return 0;
}
