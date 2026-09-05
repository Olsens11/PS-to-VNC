/*
 * File synopsis:
 * Owns PS-to-VNC's pure remote-mouse state and pointer/wheel response policy.
 *
 * This interface consumes only mouse-domain input selected by higher-level
 * controller/input routing. It does not poll libpad, resolve general hotkeys,
 * serialize RFB messages, own UI state, or manipulate GS/display state.
 *
 * Context: docs/CLEAN_ARCHITECTURE.md, "Input/controller"; GitHub Issue #38;
 * docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md, B07.
 */

#ifndef PSTVNC_MOUSE_H
#define PSTVNC_MOUSE_H

#include <stdint.h>

/*
 * Mouse-domain directional vocabulary.
 *
 * These are not PAD_* values. The controller/input owner decides whether the
 * physical D-pad currently belongs to the cursor, the mouse wheel, or another
 * product responsibility.
 */
#define PSTVNC_MOUSE_DIRECTION_UP     0x01u
#define PSTVNC_MOUSE_DIRECTION_DOWN   0x02u
#define PSTVNC_MOUSE_DIRECTION_LEFT   0x04u
#define PSTVNC_MOUSE_DIRECTION_RIGHT  0x08u
#define PSTVNC_MOUSE_DIRECTION_MASK   0x0fu

/*
 * Ordinary mouse click buttons.
 *
 * LCLICK/RCLICK are intentionally explicit so "left" and "right" cannot be
 * confused with D-pad directions while reading mouse code.
 */
#define PSTVNC_MOUSE_BUTTON_LCLICK  0x01u
#define PSTVNC_MOUSE_BUTTON_RCLICK  0x02u
#define PSTVNC_MOUSE_BUTTON_MASK    0x03u

typedef enum pstvnc_mouse_dpad_mode {
    PSTVNC_MOUSE_DPAD_NONE = 0,
    PSTVNC_MOUSE_DPAD_POINTER,
    PSTVNC_MOUSE_DPAD_WHEEL
} pstvnc_mouse_dpad_mode_t;

typedef enum pstvnc_mouse_wheel_direction {
    PSTVNC_MOUSE_WHEEL_NONE = 0,
    PSTVNC_MOUSE_WHEEL_UP,
    PSTVNC_MOUSE_WHEEL_DOWN,
    PSTVNC_MOUSE_WHEEL_LEFT,
    PSTVNC_MOUSE_WHEEL_RIGHT
} pstvnc_mouse_wheel_direction_t;

/*
 * One already-routed mouse-domain input sample.
 *
 * The higher-level controller/input owner maps physical controller facts into
 * ordinary mouse facts:
 *
 *   Cross              -> LCLICK
 *   Circle             -> RCLICK
 *   L3 pressed edge    -> wheel click
 *   Triangle + D-pad   -> D-pad wheel mode
 *
 * The left stick normally moves the cursor. Once wheel mode has been toggled
 * on by a wheel click, the same stick controls the mouse wheel until another
 * wheel click toggles the mode off.
 *
 * This is an intentional Issue #38 ergonomic change from frozen B4A behavior.
 * B4A required L3 to remain held while the stick scrolled; the clean product
 * treats L3 like clicking a mouse wheel to enter or leave analog wheel mode.
 *
 * stick_available is false whenever usable analog data is absent or another
 * product responsibility owns the stick.
 *
 * The qualified response constants assume one mouse update per controller
 * poll at approximately 60 Hz. The eventual input owner must preserve that
 * cadence unless the response model is deliberately converted to elapsed-time
 * input.
 */
typedef struct pstvnc_mouse_input {
    pstvnc_mouse_dpad_mode_t dpad_mode;
    uint16_t dpad_directions;

    int stick_available;
    unsigned char stick_x;
    unsigned char stick_y;

    int wheel_click_pressed;

    unsigned char click_buttons;
} pstvnc_mouse_input_t;

/*
 * Observable semantic result of one mouse sample.
 *
 * pointer_changed requests one ordinary pointer-state publication.
 *
 * wheel_direction is exactly one wheel notch when non-NONE. NONE means no
 * wheel event for this sample, so a second wheel-event boolean is unnecessary.
 */
typedef struct pstvnc_mouse_update {
    int pointer_changed;

    unsigned int cursor_x;
    unsigned int cursor_y;
    unsigned char click_buttons;

    pstvnc_mouse_wheel_direction_t wheel_direction;
} pstvnc_mouse_update_t;

/*
 * Persistent mouse-interpreter state.
 *
 * Cursor position and click state are the durable pointer state produced by
 * local input interpretation. They may temporarily be newer than the pointer
 * state application/main has successfully published to the remote RFB peer.
 *
 * Wheel mode is a persistent interaction mode selected by the user through
 * the L3 wheel-click toggle. Fractional movement and repeat fields are
 * transient controller-derived response history.
 *
 * pstvnc_mouse_reset_transient_history() clears only transient response
 * history. pstvnc_mouse_reset_derived() additionally leaves persistent wheel
 * mode at a true hard physical/ownership boundary.
 *
 * pstvnc_mouse_rebase_published_state() is the explicit boundary used when
 * application/main must replace that local interpreted state with its own
 * authoritative last-successfully-published remote pointer state.
 */
typedef struct pstvnc_mouse {
    unsigned int width;
    unsigned int height;

    int cursor_x;
    int cursor_y;
    unsigned char click_buttons;

    int wheel_mode_enabled;

    uint16_t dpad_hold_direction;
    unsigned int dpad_hold_ticks;
    int dpad_x_q8;
    int dpad_y_q8;

    int analog_x_q8;
    int analog_y_q8;

    pstvnc_mouse_wheel_direction_t last_wheel_direction;
    unsigned int wheel_repeat_countdown;
} pstvnc_mouse_t;

int pstvnc_mouse_init(
    pstvnc_mouse_t *mouse,
    unsigned int width,
    unsigned int height);

/*
 * Forget transient controller-derived movement, fractional, direction, and
 * repeat history while preserving persistent wheel mode and durable
 * cursor/button state.
 *
 * Use this when interpretation pauses without losing physical controller
 * continuity, such as temporary local-foreground ownership.
 */
void pstvnc_mouse_reset_transient_history(
    pstvnc_mouse_t *mouse);

/*
 * Establish a true hard mouse-state boundary.
 *
 * This performs the transient-history reset above and additionally leaves
 * persistent analog-wheel mode. Durable cursor and click-button state remain
 * unchanged.
 *
 * This function does not claim that preserved pointer state has already been
 * published remotely. Use pstvnc_mouse_rebase_published_state() when
 * application/main must reconcile it with last-successfully-published state.
 */
void pstvnc_mouse_reset_derived(
    pstvnc_mouse_t *mouse);

/*
 * Rebase the interpreter's durable pointer state to application-owned,
 * successfully published remote state at a hard ownership boundary.
 *
 * The input worker can advance its local mouse interpretation before queued
 * events are actually serialized by application/main. If those unpublished
 * events are discarded, their cursor/button state must not survive and emerge
 * after the boundary.
 *
 * This operation therefore replaces cursor/button state with the caller's
 * authoritative published values and clears all controller-derived motion,
 * repeat, and wheel-mode history.
 *
 * Returns 1 on success and 0 for invalid dimensions, coordinates, button bits,
 * or arguments.
 */
int pstvnc_mouse_rebase_published_state(
    pstvnc_mouse_t *mouse,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char published_click_buttons);

int pstvnc_mouse_update(
    pstvnc_mouse_t *mouse,
    const pstvnc_mouse_input_t *input,
    pstvnc_mouse_update_t *update);

#endif
