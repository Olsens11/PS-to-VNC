/*
 * File synopsis:
 * Defines the Application/main-thread A004 MPEG frame consumer that bridges one
 * exact A003 worker claim into the accepted Display compositor/scheduler seams.
 *
 * The consumer owns only cross-domain coordination: exact run/ordinal fencing,
 * borrowed RGB16 metadata mapping, first-frame handoff, post-first-frame
 * scheduling, claim lifetime, and failure containment. MPEG still owns decode
 * and worker lifetime; Display still owns Presentation, scheduling and physical
 * composition; the media clock remains session-owned.
 *
 * This module does not create/start/join/destroy the MPEG worker, arm
 * Presentation, start/retire the Pi producer, own Transport/RFB state, execute
 * P5 retirement transitions, wait/sleep for deadlines, or own GS resources.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A004-MAIN-THREAD-FRAME-CONSUMER-P7.
 */

#ifndef PSTVNC_APP_MPEG_FRAME_H
#define PSTVNC_APP_MPEG_FRAME_H

#include <stdint.h>

#include "display/mpeg_compositor.h"
#include "display/mpeg_scheduler.h"
#include "media/clock.h"
#include "mpeg/worker.h"

typedef enum pstvnc_app_mpeg_frame_result {
    PSTVNC_APP_MPEG_FRAME_OK = 0,
    PSTVNC_APP_MPEG_FRAME_IDLE = 1,
    PSTVNC_APP_MPEG_FRAME_WAIT = 2,
    PSTVNC_APP_MPEG_FRAME_PRESENTED = 3,
    PSTVNC_APP_MPEG_FRAME_DROPPED = 4,

    PSTVNC_APP_MPEG_FRAME_INVALID = -1,
    PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION = -2,
    PSTVNC_APP_MPEG_FRAME_PRESENTATION_STATE = -3,
    PSTVNC_APP_MPEG_FRAME_WORKER_FAILED = -4,
    PSTVNC_APP_MPEG_FRAME_FRAME_INVALID = -5,
    PSTVNC_APP_MPEG_FRAME_ORDINAL_INVALID = -6,
    PSTVNC_APP_MPEG_FRAME_SCHEDULER_FAILED = -7,
    PSTVNC_APP_MPEG_FRAME_COMPOSITOR_FAILED = -8,
    PSTVNC_APP_MPEG_FRAME_COMPOSITOR_EFFECTS_INVALID = -9,
    PSTVNC_APP_MPEG_FRAME_RELEASE_FAILED = -10,
    PSTVNC_APP_MPEG_FRAME_FAULTED = -11
} pstvnc_app_mpeg_frame_result_t;

typedef struct pstvnc_app_mpeg_frame_service_result {
    pstvnc_app_mpeg_frame_result_t result;
    uint32_t picture_ordinal;
    uint64_t deadline_tick;

    int claim_outstanding;
    int worker_finished;
    int stop_attempted;
    int release_attempted;

    pstvnc_mpeg_worker_result_t worker_claim_result;
    pstvnc_mpeg_worker_result_t worker_status_result;
    pstvnc_mpeg_worker_result_t worker_stop_result;
    pstvnc_mpeg_worker_result_t worker_release_result;

    pstvnc_mpeg_scheduler_result_t scheduler_result;
    pstvnc_mpeg_scheduler_decision_result_t scheduler_decision;

    pstvnc_mpeg_compositor_result_t compositor_result;
    pstvnc_mpeg_compositor_effects_t compositor_effects;
} pstvnc_app_mpeg_frame_service_result_t;

typedef struct pstvnc_app_mpeg_frame_status {
    uint32_t run_generation;
    uint32_t last_consumed_ordinal;
    uint32_t held_ordinal;
    uint32_t presented_count;
    uint32_t dropped_count;

    int scheduler_initialized;
    int claim_outstanding;
    int faulted;
} pstvnc_app_mpeg_frame_status_t;

typedef struct pstvnc_app_mpeg_frame_consumer {
    pstvnc_mpeg_worker_t *worker;
    pstvnc_mpeg_presentation_t *presentation;
    pstvnc_media_clock_t *clock;

    pstvnc_mpeg_scheduler_profile_t scheduler_profile;
    pstvnc_mpeg_scheduler_t scheduler;
    pstvnc_mpeg_worker_frame_t held_frame;

    uint32_t run_generation;
    uint32_t last_consumed_ordinal;
    uint32_t presented_count;
    uint32_t dropped_count;

    int initialized;
    int scheduler_initialized;
    int claim_outstanding;
    int faulted;
} pstvnc_app_mpeg_frame_consumer_t;

/*
 * Initialize the main-thread consumer for one already-started exact run.
 *
 * Presentation must already be WAIT_FIRST_FRAME for the same nonzero
 * generation, and the MPEG worker must accept that exact generation. The
 * scheduler profile is copied as immutable run policy but is intentionally
 * initialized only after the real first synchronized frame.
 */
pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_init(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation,
    pstvnc_mpeg_worker_t *worker,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock,
    const pstvnc_mpeg_scheduler_profile_t *scheduler_profile);

/*
 * Service at most one currently borrowed frame without blocking the main loop.
 *
 * current_tick is caller-observed data used only by P6 for ordinals >=2.
 * WAIT retains the exact worker claim and returns its absolute deadline.
 */
pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_service(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation,
    uint64_t current_tick,
    pstvnc_app_mpeg_frame_service_result_t *result);

/*
 * Abandon one exact outstanding P7 claim for abnormal enclosing-session
 * teardown. The claimed worker slot is released without compositor/scheduler
 * presentation and the consumer is fault-contained so it cannot resume frame
 * service afterward. No-claim is a successful terminalization.
 */
pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_abandon_claim(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation);

/* Read Application-owned claim/accounting state; no worker outcome is inferred. */
pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_status(
    const pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation,
    pstvnc_app_mpeg_frame_status_t *status);

#endif /* PSTVNC_APP_MPEG_FRAME_H */
