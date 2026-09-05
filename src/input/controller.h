/*
 * File synopsis:
 * Defines PS-to-VNC's platform-neutral physical-controller fact vocabulary.
 *
 * The direct physical-controller owner translates native PS2SDK button
 * observations into this value before controller state crosses the
 * input-runtime -> application boundary. Higher layers therefore reason about
 * physical buttons without importing PS2SDK controller vocabulary or acquiring
 * controller-device ownership.
 *
 * This interface describes facts only. It does not assign product actions,
 * resolve chords/bindings, own foreground state, interpret mouse behavior,
 * serialize remote-input traffic, or manipulate display state.
 *
 * Context:
 *   docs/CLEAN_ARCHITECTURE.md, Input/controller;
 *   GitHub Issues #38 and #39.
 */

#ifndef PSTVNC_CONTROLLER_H
#define PSTVNC_CONTROLLER_H

#include <stdint.h>

/*
 * Stable project-owned physical button bits.
 *
 * Numeric values are private to this interface. The physical controller owner
 * performs an explicit native-to-project translation so no consumer depends on
 * platform SDK vocabulary or numeric layout.
 */
#define PSTVNC_CONTROLLER_BUTTON_SELECT    0x0001u
#define PSTVNC_CONTROLLER_BUTTON_L3        0x0002u
#define PSTVNC_CONTROLLER_BUTTON_R3        0x0004u
#define PSTVNC_CONTROLLER_BUTTON_START     0x0008u

#define PSTVNC_CONTROLLER_BUTTON_UP        0x0010u
#define PSTVNC_CONTROLLER_BUTTON_RIGHT     0x0020u
#define PSTVNC_CONTROLLER_BUTTON_DOWN      0x0040u
#define PSTVNC_CONTROLLER_BUTTON_LEFT      0x0080u

#define PSTVNC_CONTROLLER_BUTTON_L2        0x0100u
#define PSTVNC_CONTROLLER_BUTTON_R2        0x0200u
#define PSTVNC_CONTROLLER_BUTTON_L1        0x0400u
#define PSTVNC_CONTROLLER_BUTTON_R1        0x0800u

#define PSTVNC_CONTROLLER_BUTTON_TRIANGLE  0x1000u
#define PSTVNC_CONTROLLER_BUTTON_CIRCLE    0x2000u
#define PSTVNC_CONTROLLER_BUTTON_CROSS     0x4000u
#define PSTVNC_CONTROLLER_BUTTON_SQUARE    0x8000u

#define PSTVNC_CONTROLLER_BUTTON_MASK      0xffffu

/*
 * One trustworthy physical-controller observation.
 *
 * buttons_down is the complete current active-high project button state.
 * buttons_pressed and buttons_released are edges relative to the immediately
 * preceding trustworthy sample in the same physical connection epoch.
 *
 * connection_epoch_started marks the first trustworthy sample after startup,
 * reconnect, or another hard physical-history invalidation. That sample
 * deliberately carries no manufactured press edges, but its complete
 * buttons_down state is authoritative.
 */
typedef struct pstvnc_controller_state {
    uint16_t buttons_down;
    uint16_t buttons_pressed;
    uint16_t buttons_released;

    int connection_epoch_started;
} pstvnc_controller_state_t;

#endif /* PSTVNC_CONTROLLER_H */
