#include <stdio.h>
#include <string.h>

#include "local_controller.h"

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, \
                "local_controller_test:%d: CHECK failed: %s\n", \
                __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

static void clear_state(
    pstvnc_controller_state_t *state)
{
    memset(state, 0, sizeof(*state));
}

int main(void)
{
    pstvnc_local_controller_t router;
    pstvnc_local_controller_result_t result;
    pstvnc_controller_state_t state;

    pstvnc_local_controller_init(&router);

    /*
     * Select owns the desktop->OSK transition. Simultaneously held Cross is
     * retained for quarantine instead of becoming a later OSK activation.
     */
    clear_state(&state);

    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_SELECT |
        PSTVNC_CONTROLLER_BUTTON_CROSS;

    state.buttons_pressed =
        state.buttons_down;

    CHECK(
        pstvnc_local_controller_route(
            &router,
            PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP,
            0,
            &state,
            &result));

    CHECK(result.action_count == 1);

    CHECK(
        result.actions[0] ==
        PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK);

    CHECK(
        !pstvnc_local_controller_release_is_proven(
            &router));

    /*
     * Releasing Select alone does not complete quarantine while Cross remains
     * physically held.
     */
    clear_state(&state);

    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_CROSS;

    state.buttons_released =
        PSTVNC_CONTROLLER_BUTTON_SELECT;

    CHECK(
        pstvnc_local_controller_route(
            &router,
            PSTVNC_LOCAL_UI_FOREGROUND_OSK,
            1,
            &state,
            &result));

    CHECK(result.action_count == 0);

    CHECK(
        !pstvnc_local_controller_release_is_proven(
            &router));

    clear_state(&state);

    state.buttons_released =
        PSTVNC_CONTROLLER_BUTTON_CROSS;

    CHECK(
        pstvnc_local_controller_route(
            &router,
            PSTVNC_LOCAL_UI_FOREGROUND_OSK,
            1,
            &state,
            &result));

    CHECK(
        pstvnc_local_controller_release_is_proven(
            &router));

    /*
     * Historical OSK action ordering is deterministic.
     */
    clear_state(&state);

    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_LEFT |
        PSTVNC_CONTROLLER_BUTTON_TRIANGLE |
        PSTVNC_CONTROLLER_BUTTON_SQUARE;

    state.buttons_pressed =
        state.buttons_down;

    CHECK(
        pstvnc_local_controller_route(
            &router,
            PSTVNC_LOCAL_UI_FOREGROUND_OSK,
            0,
            &state,
            &result));

    CHECK(result.action_count == 3);

    CHECK(
        result.actions[0] ==
        PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_LEFT);

    CHECK(
        result.actions[1] ==
        PSTVNC_LOCAL_CONTROLLER_ACTION_TOGGLE_SHIFT);

    CHECK(
        result.actions[2] ==
        PSTVNC_LOCAL_CONTROLLER_ACTION_BACKSPACE);

    /*
     * The OSK close gesture remains locally owned after foreground returns to
     * desktop.
     */
    clear_state(&state);

    state.buttons_down =
        PSTVNC_CONTROLLER_BUTTON_CIRCLE;

    state.buttons_pressed =
        PSTVNC_CONTROLLER_BUTTON_CIRCLE;

    CHECK(
        pstvnc_local_controller_route(
            &router,
            PSTVNC_LOCAL_UI_FOREGROUND_OSK,
            0,
            &state,
            &result));

    CHECK(result.action_count == 1);

    CHECK(
        result.actions[0] ==
        PSTVNC_LOCAL_CONTROLLER_ACTION_CLOSE_OSK);

    CHECK(
        !pstvnc_local_controller_release_is_proven(
            &router));

    /*
     * A fresh connection baseline with no artificial edge can still prove that
     * retained local controls are physically released.
     */
    clear_state(&state);
    state.connection_epoch_started = 1;

    CHECK(
        pstvnc_local_controller_route(
            &router,
            PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP,
            1,
            &state,
            &result));

    CHECK(result.action_count == 0);

    CHECK(
        pstvnc_local_controller_release_is_proven(
            &router));

    puts("LOCAL_CONTROLLER_TEST=PASS");
    return 0;
}
