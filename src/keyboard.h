/*
 * File synopsis:
 * Defines pure keyboard-action sequencing independently of controller polling,
 * OSK/local-UI state, semantic-event transport, RFB serialization, and socket
 * ownership.
 *
 * A caller supplies an already-resolved native X11 keysym plus the real
 * modifier keys that must bracket it. This component produces a complete,
 * balanced sequence of key-down/key-up facts for later application routing.
 *
 * Printable Shift-layer policy deliberately remains outside this component.
 * An OSK may, for example, choose an already-shifted printable keysym without
 * requesting a real Shift modifier, while Shift+Tab can request true Shift.
 *
 * Context:
 *   docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md, B08;
 *   docs/CLEAN_ARCHITECTURE.md, keyboard/OSK and RFB ownership;
 *   GitHub Issue #39.
 */

#ifndef PSTVNC_KEYBOARD_H
#define PSTVNC_KEYBOARD_H

#include <stdint.h>

/*
 * Explicit modifier intent supplied by the keyboard/UI action owner.
 *
 * These bits are project semantic vocabulary rather than RFB wire bits.
 * They describe which real X11 modifier keys must surround the target key.
 */
#define PSTVNC_KEYBOARD_MODIFIER_SHIFT 0x01u
#define PSTVNC_KEYBOARD_MODIFIER_CTRL  0x02u
#define PSTVNC_KEYBOARD_MODIFIER_ALT   0x04u

#define PSTVNC_KEYBOARD_MODIFIER_MASK \
    (PSTVNC_KEYBOARD_MODIFIER_SHIFT | \
     PSTVNC_KEYBOARD_MODIFIER_CTRL | \
     PSTVNC_KEYBOARD_MODIFIER_ALT)

/*
 * Native X11 keysyms used when synthesizing real modifier state.
 *
 * The historical qualified B08 implementation used these same canonical X11
 * values. They are keyboard-domain constants, not controller aliases.
 */
#define PSTVNC_KEYBOARD_KEYSYM_BACKSPACE 0x0000ff08u
#define PSTVNC_KEYBOARD_KEYSYM_TAB       0x0000ff09u
#define PSTVNC_KEYBOARD_KEYSYM_ENTER     0x0000ff0du

#define PSTVNC_KEYBOARD_KEYSYM_SHIFT_L   0x0000ffe1u
#define PSTVNC_KEYBOARD_KEYSYM_CONTROL_L 0x0000ffe3u
#define PSTVNC_KEYBOARD_KEYSYM_ALT_L     0x0000ffe9u

/*
 * Three modifier downs + target down/up + three modifier ups.
 */
#define PSTVNC_KEYBOARD_SEQUENCE_MAX_EVENTS 8u

/*
 * One already-resolved logical keyboard tap intent.
 *
 * keysym identifies the target native X11 keysym.
 * modifiers identifies only real modifier keys that must bracket the target.
 *
 * This compact intent is suitable for application routing. Expansion into
 * explicit down/up facts remains the responsibility of the pure keyboard
 * sequence builder below.
 */
typedef struct pstvnc_keyboard_tap {
    uint32_t keysym;
    unsigned int modifiers;
} pstvnc_keyboard_tap_t;

/*
 * One native keyboard fact before RFB encoding.
 *
 * down is boolean semantic state:
 *   nonzero -> key down
 *   zero    -> key up
 *
 * keysym is a complete native 32-bit X11 keysym.
 */
typedef struct pstvnc_keyboard_key_event {
    int down;
    uint32_t keysym;
} pstvnc_keyboard_key_event_t;

/*
 * One complete balanced logical keyboard action.
 *
 * event_count is the authoritative number of initialized entries in events.
 * The bounded array avoids allocation and makes complete sequence construction
 * host-testable before any transport is involved.
 */
typedef struct pstvnc_keyboard_sequence {
    pstvnc_keyboard_key_event_t
        events[PSTVNC_KEYBOARD_SEQUENCE_MAX_EVENTS];

    unsigned int event_count;
} pstvnc_keyboard_sequence_t;

/*
 * Build one balanced logical key tap.
 *
 * Modifier presses are emitted in deterministic order:
 *
 *   Shift -> Ctrl -> Alt
 *
 * followed by:
 *
 *   target down -> target up
 *
 * and modifier releases in reverse order:
 *
 *   Alt -> Ctrl -> Shift
 *
 * The reverse release order makes the sequence a properly nested ownership
 * bracket around the target.
 *
 * Returns 1 on success.
 * Returns 0 for an invalid output pointer or unknown modifier bits.
 *
 * On any failure with a non-NULL sequence, event_count is zero so no partial
 * sequence can be mistaken for valid work.
 */
int pstvnc_keyboard_build_tap_sequence(
    uint32_t keysym,
    unsigned int modifiers,
    pstvnc_keyboard_sequence_t *sequence);

#endif /* PSTVNC_KEYBOARD_H */
