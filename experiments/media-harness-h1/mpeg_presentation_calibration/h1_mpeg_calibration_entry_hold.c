/*
 * File synopsis:
 * Implements the experiment-local 750 ms held START+SELECT calibration-entry
 * policy against caller-supplied monotonic microseconds.
 */
#include "h1_mpeg_calibration_entry_hold.h"

#include <stddef.h>

void pstvnc_h1_mpeg_calibration_entry_hold_init(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold)
{
    if (hold == NULL)
        return;

    hold->started_us = 0;
    hold->armed = 0;
}

int pstvnc_h1_mpeg_calibration_entry_hold_observe(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int entry_chord_down,
    int *consume_controller_state)
{
    if (hold == NULL || consume_controller_state == NULL)
        return 0;

    *consume_controller_state = 0;

    if (!entry_available) {
        hold->armed = 0;
        return 1;
    }

    if (entry_chord_down) {
        if (!hold->armed) {
            hold->started_us = now_us;
            hold->armed = 1;
        }

        *consume_controller_state = 1;
        return 1;
    }

    if (hold->armed) {
        hold->armed = 0;
        *consume_controller_state = 1;
    }

    return 1;
}

int pstvnc_h1_mpeg_calibration_entry_hold_poll(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int *activate_calibration)
{
    if (hold == NULL || activate_calibration == NULL)
        return 0;

    *activate_calibration = 0;

    if (!entry_available) {
        hold->armed = 0;
        return 1;
    }

    if (!hold->armed)
        return 1;

    /*
     * GetTimerSystemTime() is monotonic in the live PS2 composition. If a test
     * or future platform violates that assumption, re-arm rather than turning a
     * backwards jump into a huge unsigned elapsed interval.
     */
    if (now_us < hold->started_us) {
        hold->started_us = now_us;
        return 1;
    }

    if (now_us - hold->started_us <
        PSTVNC_H1_MPEG_CALIBRATION_ENTRY_HOLD_US)
        return 1;

    hold->armed = 0;
    *activate_calibration = 1;
    return 1;
}
