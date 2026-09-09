/*
 * File synopsis:
 * Defines CP2M's experiment-local, pure transient L1+D-pad keyboard-chord
 * router. It consumes only project physical-controller facts and produces
 * already-resolved X11 arrow keysyms; it owns no libpad, mouse state, RFB
 * serialization, UI, GS state, or transport.
 *
 * The behavior is the historical Test11F contract: L1 is a physical chord
 * modifier, not a latched mode. A direction fires once when either the
 * direction or L1 newly enters the held chord. Ordinary key repeat remains the
 * remote desktop's responsibility after the explicit tap; CP2M does not invent
 * controller-side repeat.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_KEYBOARD_CHORD_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_KEYBOARD_CHORD_H

#include "controller.h"

#include <stdint.h>

#define PSTVNC_H1_KEYSYM_LEFT  0x0000ff51u
#define PSTVNC_H1_KEYSYM_UP    0x0000ff52u
#define PSTVNC_H1_KEYSYM_RIGHT 0x0000ff53u
#define PSTVNC_H1_KEYSYM_DOWN  0x0000ff54u

#define PSTVNC_H1_RFB_KEYBOARD_CHORD_MAX_TAPS 4u

typedef struct pstvnc_h1_rfb_keyboard_chord_result {
    int enter_chord;
    int exit_chord;

    uint32_t keysyms[PSTVNC_H1_RFB_KEYBOARD_CHORD_MAX_TAPS];
    unsigned int keysym_count;
} pstvnc_h1_rfb_keyboard_chord_result_t;

/*
 * Route one trustworthy physical-controller fact.
 *
 * chord_active is application/main's current ownership state for the transient
 * L1 keyboard chord. The result says whether this sample enters or leaves that
 * ownership state and emits zero or more arrow key taps in historical order:
 *
 *     Up, Down, Left, Right
 *
 * A connection-epoch baseline is treated as authoritative current state. This
 * mirrors the historical last_pressed=0 boundary without manufacturing generic
 * press edges outside this narrow chord router.
 */
int pstvnc_h1_rfb_keyboard_chord_route(
    int chord_active,
    const pstvnc_controller_state_t *state,
    pstvnc_h1_rfb_keyboard_chord_result_t *result);

#endif
