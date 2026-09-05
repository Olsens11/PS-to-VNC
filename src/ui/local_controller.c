/*
 * File synopsis:
 * Implements the pure Stage 2 controller-to-local-action router.
 *
 * Physical controller acquisition remains below this file. Local UI/OSK state
 * mutation and every remote side effect remain above it.
 */

#include "local_controller.h"

#include <string.h>

#define PSTVNC_LOCAL_CONTROLLER_OWNED_BUTTONS \
    (PSTVNC_CONTROLLER_BUTTON_SELECT | \
     PSTVNC_CONTROLLER_BUTTON_START | \
     PSTVNC_CONTROLLER_BUTTON_UP | \
     PSTVNC_CONTROLLER_BUTTON_RIGHT | \
     PSTVNC_CONTROLLER_BUTTON_DOWN | \
     PSTVNC_CONTROLLER_BUTTON_LEFT | \
     PSTVNC_CONTROLLER_BUTTON_R1 | \
     PSTVNC_CONTROLLER_BUTTON_TRIANGLE | \
     PSTVNC_CONTROLLER_BUTTON_CIRCLE | \
     PSTVNC_CONTROLLER_BUTTON_CROSS | \
     PSTVNC_CONTROLLER_BUTTON_SQUARE)

static void local_controller_clear_result(
    pstvnc_local_controller_result_t *result)
{
    if (result != NULL)
        memset(result, 0, sizeof(*result));
}

static int local_controller_state_is_valid(
    const pstvnc_controller_state_t *state)
{
    uint16_t not_down;

    if (state == NULL)
        return 0;

    not_down =
        (uint16_t)~state->buttons_down;

    if ((state->buttons_pressed &
         not_down) != 0)
        return 0;

    if ((state->buttons_released &
         state->buttons_down) != 0)
        return 0;

    if ((state->buttons_pressed &
         state->buttons_released) != 0)
        return 0;

    return 1;
}

static int local_controller_append_action(
    pstvnc_local_controller_result_t *result,
    pstvnc_local_controller_action_t action)
{
    if (result == NULL ||
        result->action_count >=
            PSTVNC_LOCAL_CONTROLLER_MAX_ACTIONS)
        return 0;

    result->actions[result->action_count] =
        action;

    result->action_count++;

    return 1;
}

void pstvnc_local_controller_init(
    pstvnc_local_controller_t *controller)
{
    if (controller == NULL)
        return;

    memset(controller, 0, sizeof(*controller));
}

int pstvnc_local_controller_route(
    pstvnc_local_controller_t *controller,
    pstvnc_local_ui_foreground_t foreground,
    int input_quarantined,
    const pstvnc_controller_state_t *state,
    pstvnc_local_controller_result_t *result)
{
    uint16_t local_pressed;

    if (controller == NULL ||
        result == NULL)
        return 0;

    local_controller_clear_result(result);

    if (!local_controller_state_is_valid(state))
        return 0;

    if (foreground !=
            PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP &&
        foreground !=
            PSTVNC_LOCAL_UI_FOREGROUND_OSK)
        return 0;

    /*
     * Complete trustworthy current-down state proves release of every retained
     * local button no longer physically held. The same rule handles an ordinary
     * release sample and a reconnect baseline.
     */
    controller->owned_buttons_awaiting_release &=
        state->buttons_down;

    local_pressed =
        (uint16_t)(
            state->buttons_pressed &
            PSTVNC_LOCAL_CONTROLLER_OWNED_BUTTONS);

    /*
     * OSK foreground owns its fixed Issue #39 controls. During transition
     * quarantine, newly pressed local controls are retained too so they cannot
     * be held through the ownership boundary.
     */
    if (input_quarantined ||
        foreground ==
            PSTVNC_LOCAL_UI_FOREGROUND_OSK) {

        controller->owned_buttons_awaiting_release |=
            local_pressed;
    }

    if (input_quarantined)
        return 1;

    if (foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP) {

        if (state->buttons_pressed &
            PSTVNC_CONTROLLER_BUTTON_SELECT) {

            /*
             * Opening owns every relevant local button already physically down
             * in the transition sample. Select+Cross, Select+Circle, etc. can
             * never emerge as fresh OSK actions after entry.
             */
            controller->owned_buttons_awaiting_release |=
                (uint16_t)(
                    state->buttons_down &
                    PSTVNC_LOCAL_CONTROLLER_OWNED_BUTTONS);

            return local_controller_append_action(
                result,
                PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK);
        }

        return 1;
    }

    /*
     * Preserve historical OSK evaluation order:
     * D-pad, Triangle, Cross, Square, Start, R1, then Circle/Select close.
     */
    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_LEFT) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_LEFT))
        return 0;

    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_RIGHT) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_RIGHT))
        return 0;

    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_UP) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_UP))
        return 0;

    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_DOWN) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_DOWN))
        return 0;

    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_TRIANGLE) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_TOGGLE_SHIFT))
        return 0;

    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_CROSS) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_ACTIVATE_SELECTED))
        return 0;

    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_SQUARE) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_BACKSPACE))
        return 0;

    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_START) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_ENTER))
        return 0;

    if ((local_pressed &
         PSTVNC_CONTROLLER_BUTTON_R1) &&
        !local_controller_append_action(
            result,
            PSTVNC_LOCAL_CONTROLLER_ACTION_TAB))
        return 0;

    if (local_pressed &
        (PSTVNC_CONTROLLER_BUTTON_CIRCLE |
         PSTVNC_CONTROLLER_BUTTON_SELECT)) {

        if (!local_controller_append_action(
                result,
                PSTVNC_LOCAL_CONTROLLER_ACTION_CLOSE_OSK))
            return 0;
    }

    return 1;
}

int pstvnc_local_controller_release_is_proven(
    const pstvnc_local_controller_t *controller)
{
    return
        controller != NULL &&
        controller->owned_buttons_awaiting_release == 0;
}
