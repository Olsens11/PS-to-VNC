/*
 * File synopsis:
 * Defines H1's session-scoped shared media clock.
 *
 * The qualified P11 video path remains the timing authority: the first video
 * presentation boundary arms the epoch. Audio may prefill before that point,
 * but it cannot present until the same epoch is visible. Signed per-stream
 * offsets are then applied relative to that common origin.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_MEDIA_CLOCK_H
#define PSTVNC_MEDIA_HARNESS_H1_MEDIA_CLOCK_H

#include <stdint.h>
#include <tamtypes.h>

typedef struct pstvnc_h1_media_clock {
    volatile int armed;
    volatile u64 epoch_tick;
    uint32_t lead_us;
} pstvnc_h1_media_clock_t;

void pstvnc_h1_media_clock_init(
    pstvnc_h1_media_clock_t *clock,
    uint32_t lead_us);

void pstvnc_h1_media_clock_arm_now(
    pstvnc_h1_media_clock_t *clock);

int pstvnc_h1_media_clock_is_armed(
    const pstvnc_h1_media_clock_t *clock);

u64 pstvnc_h1_media_clock_deadline(
    const pstvnc_h1_media_clock_t *clock,
    int32_t offset_us,
    u64 additional_ticks);

/*
 * Returns 1 when the requested presentation deadline is reached, 0 when the
 * optional stop flag becomes true, and -1 on DelayThread failure.
 */
int pstvnc_h1_media_clock_wait_offset(
    pstvnc_h1_media_clock_t *clock,
    int32_t offset_us,
    uint32_t poll_us,
    const volatile int *stop_requested);

#endif
