/*
 * File synopsis:
 * Defines the platform-neutral Stage 2 controller-to-local-action router.
 *
 * This owner consumes project physical-controller facts plus current local
 * foreground/quarantine state and produces only local semantic actions.
 *
 * It does not poll a physical controller, manipulate the mouse, mutate OSK/UI
 * state, serialize remote protocol traffic, present frames, or execute actions.
 *
 * It remembers locally-owned buttons that remain physically held so an opening
 * or closing gesture cannot leak across a foreground transition.
 *
 * Context:
 *   GitHub Issue #39;
 *   historical B4A OSK control behavior;
 *   K8 controller/foreground census.
 */

#ifndef PSTVNC_LOCAL_CONTROLLER_H
#define PSTVNC_LOCAL_CONTROLLER_H

#include <stdint.h>

#include "controller.h"
#include "local_ui.h"

#define PSTVNC_LOCAL_CONTROLLER_MAX_ACTIONS 10u

typedef enum pstvnc_local_controller_action {
    PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK = 0,
    PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_LEFT,
    PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_RIGHT,
    PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_UP,
    PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_DOWN,
    PSTVNC_LOCAL_CONTROLLER_ACTION_TOGGLE_SHIFT,
    PSTVNC_LOCAL_CONTROLLER_ACTION_ACTIVATE_SELECTED,
    PSTVNC_LOCAL_CONTROLLER_ACTION_BACKSPACE,
    PSTVNC_LOCAL_CONTROLLER_ACTION_ENTER,
    PSTVNC_LOCAL_CONTROLLER_ACTION_TAB,
    PSTVNC_LOCAL_CONTROLLER_ACTION_CLOSE_OSK
} pstvnc_local_controller_action_t;

typedef struct pstvnc_local_controller_result {
    pstvnc_local_controller_action_t
        actions[PSTVNC_LOCAL_CONTROLLER_MAX_ACTIONS];

    unsigned int action_count;
} pstvnc_local_controller_result_t;

typedef struct pstvnc_local_controller {
    uint16_t owned_buttons_awaiting_release;
} pstvnc_local_controller_t;

void pstvnc_local_controller_init(
    pstvnc_local_controller_t *controller);

int pstvnc_local_controller_route(
    pstvnc_local_controller_t *controller,
    pstvnc_local_ui_foreground_t foreground,
    int input_quarantined,
    const pstvnc_controller_state_t *state,
    pstvnc_local_controller_result_t *result);

int pstvnc_local_controller_release_is_proven(
    const pstvnc_local_controller_t *controller);

#endif /* PSTVNC_LOCAL_CONTROLLER_H */
