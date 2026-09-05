/*
 * File synopsis:
 * Implements pure balanced keyboard sequence construction without owning RFB,
 * controller acquisition, OSK state, local UI policy, or application routing.
 *
 * Context:
 *   docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md, B08;
 *   GitHub Issue #39.
 */

#include <stddef.h>

#include "keyboard.h"

int pstvnc_keyboard_build_tap_sequence(
    uint32_t keysym,
    unsigned int modifiers,
    pstvnc_keyboard_sequence_t *sequence)
{
    unsigned int event_index = 0;

    if (sequence == NULL)
        return 0;

    /*
     * Zero publication authority first. If validation fails below, callers can
     * never observe a partially valid sequence.
     */
    sequence->event_count = 0;

    if ((modifiers & ~PSTVNC_KEYBOARD_MODIFIER_MASK) != 0)
        return 0;

    /*
     * Press real modifiers in deterministic nesting order.
     */
    if ((modifiers & PSTVNC_KEYBOARD_MODIFIER_SHIFT) != 0) {
        sequence->events[event_index].down = 1;
        sequence->events[event_index].keysym =
            PSTVNC_KEYBOARD_KEYSYM_SHIFT_L;
        event_index++;
    }

    if ((modifiers & PSTVNC_KEYBOARD_MODIFIER_CTRL) != 0) {
        sequence->events[event_index].down = 1;
        sequence->events[event_index].keysym =
            PSTVNC_KEYBOARD_KEYSYM_CONTROL_L;
        event_index++;
    }

    if ((modifiers & PSTVNC_KEYBOARD_MODIFIER_ALT) != 0) {
        sequence->events[event_index].down = 1;
        sequence->events[event_index].keysym =
            PSTVNC_KEYBOARD_KEYSYM_ALT_L;
        event_index++;
    }

    /*
     * A logical tap always contains an explicit target down and target up.
     */
    sequence->events[event_index].down = 1;
    sequence->events[event_index].keysym = keysym;
    event_index++;

    sequence->events[event_index].down = 0;
    sequence->events[event_index].keysym = keysym;
    event_index++;

    /*
     * Release modifiers in reverse nesting order.
     */
    if ((modifiers & PSTVNC_KEYBOARD_MODIFIER_ALT) != 0) {
        sequence->events[event_index].down = 0;
        sequence->events[event_index].keysym =
            PSTVNC_KEYBOARD_KEYSYM_ALT_L;
        event_index++;
    }

    if ((modifiers & PSTVNC_KEYBOARD_MODIFIER_CTRL) != 0) {
        sequence->events[event_index].down = 0;
        sequence->events[event_index].keysym =
            PSTVNC_KEYBOARD_KEYSYM_CONTROL_L;
        event_index++;
    }

    if ((modifiers & PSTVNC_KEYBOARD_MODIFIER_SHIFT) != 0) {
        sequence->events[event_index].down = 0;
        sequence->events[event_index].keysym =
            PSTVNC_KEYBOARD_KEYSYM_SHIFT_L;
        event_index++;
    }

    if (event_index > PSTVNC_KEYBOARD_SEQUENCE_MAX_EVENTS) {
        /*
         * This cannot occur with the current three-modifier contract, but keep
         * the bounded-storage invariant explicit next to publication.
         */
        sequence->event_count = 0;
        return 0;
    }

    sequence->event_count = event_index;
    return 1;
}
