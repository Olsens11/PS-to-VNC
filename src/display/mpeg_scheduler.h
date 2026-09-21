/*
 * File synopsis:
 * Defines the Display-owned, platform-neutral MPEG presentation scheduler used
 * only after P4 has physically presented a run's first frame and armed the
 * existing session media clock.
 *
 * The scheduler copies one caller-owned run generation, validates one narrow
 * rational source-rate/drop profile, derives one immutable per-run offset on
 * the existing session video-deadline axis, and classifies later picture
 * ordinals as WAIT, PRESENT_NOW, or DROP_PRESENTATION_ONLY.
 *
 * It never arms/re-arms/clears the media clock, never presents or sleeps, and
 * owns no decoder, Transport, RFB, calibration, GS/dmaKit, EOF, or Application
 * lifecycle state.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-ABSOLUTE-VIDEO-SCHEDULER-DROP-R6;
 *   docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md;
 *   docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#ifndef PSTVNC_MPEG_SCHEDULER_H
#define PSTVNC_MPEG_SCHEDULER_H

#include <stdint.h>

#include "media/clock.h"

#define PSTVNC_MPEG_SCHEDULER_MILLIFRAMES_PER_FRAME 1000u

typedef enum pstvnc_mpeg_scheduler_result {
    PSTVNC_MPEG_SCHEDULER_OK = 0,
    PSTVNC_MPEG_SCHEDULER_INVALID = -1,
    PSTVNC_MPEG_SCHEDULER_CLOCK_UNARMED = -2,
    PSTVNC_MPEG_SCHEDULER_CLOCK_FAILED = -3
} pstvnc_mpeg_scheduler_result_t;

typedef enum pstvnc_mpeg_scheduler_decision {
    PSTVNC_MPEG_SCHEDULER_DECISION_INVALID = -1,
    PSTVNC_MPEG_SCHEDULER_WAIT_UNTIL_DEADLINE = 0,
    PSTVNC_MPEG_SCHEDULER_PRESENT_NOW,
    PSTVNC_MPEG_SCHEDULER_DROP_PRESENTATION_ONLY
} pstvnc_mpeg_scheduler_decision_t;

typedef struct pstvnc_mpeg_scheduler_profile {
    uint32_t fps_numerator;
    uint32_t fps_denominator;
    int drop_enabled;
    uint32_t drop_threshold_milliframes;
} pstvnc_mpeg_scheduler_profile_t;

typedef struct pstvnc_mpeg_scheduler {
    const pstvnc_media_clock_t *clock;
    pstvnc_mpeg_scheduler_profile_t profile;
    uint32_t run_generation;
    uint64_t frame_period_ticks;
    uint64_t run_base_additional_ticks;
    uint64_t drop_threshold_ticks;
    int initialized;
} pstvnc_mpeg_scheduler_t;

typedef struct pstvnc_mpeg_scheduler_decision_result {
    pstvnc_mpeg_scheduler_decision_t decision;
    uint64_t deadline_tick;
    uint64_t lateness_ticks;
} pstvnc_mpeg_scheduler_decision_result_t;

/*
 * Initialize timing policy only after the exact P4 first synchronized frame.
 *
 * clock must already be armed. run_generation is copied verbatim and never
 * minted here. The scheduler observes the common video deadline for
 * additional_ticks=0 once and retains:
 *
 *   max(first_sync_tick - video_deadline_zero, 0)
 *
 * as this run's immutable additional-tick base on the existing session axis.
 * The common clock itself is never modified.
 */
int pstvnc_mpeg_scheduler_init_after_first_sync(
    pstvnc_mpeg_scheduler_t *scheduler,
    const pstvnc_media_clock_t *clock,
    const pstvnc_mpeg_scheduler_profile_t *profile,
    uint32_t run_generation,
    uint64_t first_sync_tick);

/*
 * Classify one subsequent run-local picture ordinal.
 *
 * picture_ordinal starts at 2; ordinal 1 is exclusively P4 first-frame
 * authority and is rejected here. current_tick is injected data. This function
 * never waits, presents, drops decoder state, or mutates scheduler/clock state.
 */
int pstvnc_mpeg_scheduler_decide(
    const pstvnc_mpeg_scheduler_t *scheduler,
    uint32_t run_generation,
    uint64_t picture_ordinal,
    uint64_t current_tick,
    pstvnc_mpeg_scheduler_decision_result_t *result);

#endif /* PSTVNC_MPEG_SCHEDULER_H */
