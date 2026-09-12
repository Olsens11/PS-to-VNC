/*
 * File synopsis:
 * Defines the experiment-local timing policy for the temporary held
 * START+SELECT MPEG-calibration entry gesture.
 *
 * The caller supplies time from the program-wide monotonic clock. This module
 * owns only the 750 ms hold policy and whether pre-entry controller samples must
 * be consumed; it does not read PS2 timers, poll libpad, or claim foreground.
 */
#ifndef PSTVNC_H1_MPEG_CALIBRATION_ENTRY_HOLD_H
#define PSTVNC_H1_MPEG_CALIBRATION_ENTRY_HOLD_H

#include <stdint.h>

#define PSTVNC_H1_MPEG_CALIBRATION_ENTRY_HOLD_US UINT64_C(750000)

typedef struct pstvnc_h1_mpeg_calibration_entry_hold {
    uint64_t started_us;
    unsigned armed : 1;
} pstvnc_h1_mpeg_calibration_entry_hold_t;

void pstvnc_h1_mpeg_calibration_entry_hold_init(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold);

/*
 * Observe one authoritative controller transition.
 *
 * `entry_available` is true only while ordinary desktop/RFB foreground is
 * available. `entry_chord_down` means START+SELECT are both physically down.
 *
 * A newly held chord arms the timer and is consumed so SELECT cannot also open
 * the OSK. Further transition samples while the chord remains down are consumed
 * without resetting the deadline. Releasing either chord button before the
 * deadline cancels the attempt and consumes that cancellation sample.
 *
 * When entry is unavailable (for example while OSK owns foreground), the hold
 * is disarmed and the sample passes through to its ordinary owner.
 */
int pstvnc_h1_mpeg_calibration_entry_hold_observe(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int entry_chord_down,
    int *consume_controller_state);

/*
 * Poll the nonblocking deadline from the caller's normal service cadence.
 * `activate_calibration` becomes true exactly once after 750 ms of uninterrupted
 * armed hold while entry remains available.
 */
int pstvnc_h1_mpeg_calibration_entry_hold_poll(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int *activate_calibration);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_ENTRY_HOLD_H */
