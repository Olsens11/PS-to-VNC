/*
 * File synopsis:
 * Implements CP2M's pure Test11F L1+D-pad arrow-key chord semantics.
 *
 * This file deliberately contains no PS2SDK PAD_* values and no side effects.
 * Native controller acquisition has already been translated into project-owned
 * controller facts by input_runtime before this router sees them.
 */

#include "h1_rfb_keyboard_chord.h"

#include <stddef.h>
#include <string.h>

static int h1_keyboard_chord_state_is_valid(
    const pstvnc_controller_state_t *state)
{
    uint16_t not_down;

    if (state == NULL)
        return 0;

    not_down = (uint16_t)~state->buttons_down;

    if ((state->buttons_pressed & not_down) != 0)
        return 0;

    if ((state->buttons_released & state->buttons_down) != 0)
        return 0;

    if ((state->buttons_pressed & state->buttons_released) != 0)
        return 0;

    return 1;
}

static int h1_keyboard_chord_append(
    pstvnc_h1_rfb_keyboard_chord_result_t *result,
    uint32_t keysym)
{
    if (result == NULL ||
        result->keysym_count >= PSTVNC_H1_RFB_KEYBOARD_CHORD_MAX_TAPS)
        return 0;

    result->keysyms[result->keysym_count] = keysym;
    result->keysym_count++;
    return 1;
}

static int h1_keyboard_chord_append_direction(
    const pstvnc_controller_state_t *state,
    uint16_t edge_mask,
    uint16_t direction,
    uint32_t keysym,
    pstvnc_h1_rfb_keyboard_chord_result_t *result)
{
    if ((state->buttons_down & direction) == 0)
        return 1;

    /*
     * Historical Test11F fired when either member of the L1+direction pair
     * changed into the held combination. Thus pressing L1 while a direction is
     * already held, or pressing a direction while L1 is held, each yields one
     * tap. An unchanged held pair yields none.
     */
    if ((edge_mask &
         (uint16_t)(PSTVNC_CONTROLLER_BUTTON_L1 | direction)) == 0)
        return 1;

    return h1_keyboard_chord_append(result, keysym);
}

int pstvnc_h1_rfb_keyboard_chord_route(
    int chord_active,
    const pstvnc_controller_state_t *state,
    pstvnc_h1_rfb_keyboard_chord_result_t *result)
{
    uint16_t edge_mask;
    int l1_down;

    if (result == NULL || !h1_keyboard_chord_state_is_valid(state))
        return 0;

    memset(result, 0, sizeof(*result));

    l1_down =
        (state->buttons_down & PSTVNC_CONTROLLER_BUTTON_L1) != 0;

    result->enter_chord = !chord_active && l1_down;
    result->exit_chord = chord_active && !l1_down;

    if (!l1_down)
        return 1;

    edge_mask = state->buttons_pressed;

    /*
     * A connection/reacquisition baseline intentionally has no manufactured
     * pressed edges. Its complete buttons_down state is nevertheless the first
     * authoritative state of the epoch, matching the historical reset where
     * last_pressed became zero before the next valid sample.
     */
    if (state->connection_epoch_started)
        edge_mask |= state->buttons_down;

    if (!h1_keyboard_chord_append_direction(
            state,
            edge_mask,
            PSTVNC_CONTROLLER_BUTTON_UP,
            PSTVNC_H1_KEYSYM_UP,
            result))
        return 0;

    if (!h1_keyboard_chord_append_direction(
            state,
            edge_mask,
            PSTVNC_CONTROLLER_BUTTON_DOWN,
            PSTVNC_H1_KEYSYM_DOWN,
            result))
        return 0;

    if (!h1_keyboard_chord_append_direction(
            state,
            edge_mask,
            PSTVNC_CONTROLLER_BUTTON_LEFT,
            PSTVNC_H1_KEYSYM_LEFT,
            result))
        return 0;

    if (!h1_keyboard_chord_append_direction(
            state,
            edge_mask,
            PSTVNC_CONTROLLER_BUTTON_RIGHT,
            PSTVNC_H1_KEYSYM_RIGHT,
            result))
        return 0;

    return 1;
}
