/*
 * File synopsis:
 * Defines the R21-R23C Application-owned, trigger-agnostic MPEG run
 * coordinator. One session-scoped owner allocates exact run generations,
 * composes accepted start/live-frame service, and owns the ordered retirement/
 * drain transaction across existing Presentation, P7, P2, MPEG worker/runtime
 * and Transport seams.
 *
 * R23C preserves the R23 execution-retirement fences but releases P2
 * suppression immediately after successful exact RETIRE serialization so
 * ordinary RFB restoration can overlap visible P3 RETIRING drain. Successful
 * execution retirement stops at RESTORE_PENDING with P3 retained and P2
 * thawed; final P3 seal/reveal remains later authority.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-APPLICATION-MPEG-RUN-START-R21,
 * A003-APPLICATION-MPEG-LIVE-SERVICE-R22,
 * A003-APPLICATION-MPEG-RETIREMENT-DRAIN-R23 and
 * A003-APPLICATION-MPEG-Q7-RESTORE-OVERLAP-R23C.
 */

#ifndef PSTVNC_APP_MPEG_RUN_H
#define PSTVNC_APP_MPEG_RUN_H

#include <stdint.h>

#include "app_mpeg_frame.h"
#include "config/mpeg_runtime_profile.h"
#include "display/mpeg_presentation.h"
#include "media/clock.h"
#include "mpeg/ps2_decoder_backend.h"
#include "mpeg/ps2_worker_runtime.h"
#include "mpeg/worker.h"
#include "rfb/flow_policy.h"
#include "transport/bridge.h"

typedef enum pstvnc_app_mpeg_run_state {
    PSTVNC_APP_MPEG_RUN_IDLE = 0,
    PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME,
    PSTVNC_APP_MPEG_RUN_MPEG_OWNED,
    PSTVNC_APP_MPEG_RUN_RETIRING,
    PSTVNC_APP_MPEG_RUN_RESTORE_PENDING,
    PSTVNC_APP_MPEG_RUN_FAULTED
} pstvnc_app_mpeg_run_state_t;

typedef enum pstvnc_app_mpeg_run_result {
    PSTVNC_APP_MPEG_RUN_OK = 0,

    PSTVNC_APP_MPEG_RUN_INVALID = -1,
    PSTVNC_APP_MPEG_RUN_NOT_IDLE = -2,
    PSTVNC_APP_MPEG_RUN_GENERATION_EXHAUSTED = -3,
    PSTVNC_APP_MPEG_RUN_PROFILE_UNAVAILABLE = -4,
    PSTVNC_APP_MPEG_RUN_GEOMETRY_INVALID = -5,
    PSTVNC_APP_MPEG_RUN_RFB_NOT_PROTECTED = -6,
    PSTVNC_APP_MPEG_RUN_PRESENTATION_NOT_IDLE = -7,
    PSTVNC_APP_MPEG_RUN_TRANSPORT_OPEN_FAILED = -8,
    PSTVNC_APP_MPEG_RUN_RUNTIME_INIT_FAILED = -9,
    PSTVNC_APP_MPEG_RUN_RUNTIME_OPS_FAILED = -10,
    PSTVNC_APP_MPEG_RUN_BACKEND_OPS_FAILED = -11,
    PSTVNC_APP_MPEG_RUN_WORKER_START_FAILED = -12,
    PSTVNC_APP_MPEG_RUN_PRESENTATION_ARM_FAILED = -13,
    PSTVNC_APP_MPEG_RUN_PRESENTATION_SNAPSHOT_FAILED = -14,
    PSTVNC_APP_MPEG_RUN_FRAME_CONSUMER_INIT_FAILED = -15,
    PSTVNC_APP_MPEG_RUN_START_FAILED = -16,
    PSTVNC_APP_MPEG_RUN_CLEANUP_FAILED = -17,
    PSTVNC_APP_MPEG_RUN_ALREADY_FAULTED = -18,

    PSTVNC_APP_MPEG_RUN_NOT_LIVE = -19,
    PSTVNC_APP_MPEG_RUN_LIVE_STATE_INVALID = -20,
    PSTVNC_APP_MPEG_RUN_FRAME_SERVICE_FAILED = -21,
    PSTVNC_APP_MPEG_RUN_UNEXPECTED_WORKER_FINISH = -22,
    PSTVNC_APP_MPEG_RUN_FRAME_SERVICE_CONTRADICTION = -23,

    PSTVNC_APP_MPEG_RUN_NOT_MPEG_OWNED = -24,
    PSTVNC_APP_MPEG_RUN_RETIRE_STATE_INVALID = -25,
    PSTVNC_APP_MPEG_RUN_PRESENTATION_RETIRE_FAILED = -26,
    PSTVNC_APP_MPEG_RUN_RETIRE_SEND_FAILED = -27,
    PSTVNC_APP_MPEG_RUN_NOT_RETIRING = -28,
    PSTVNC_APP_MPEG_RUN_RETIRE_COMPLETION_FAILED = -29,
    PSTVNC_APP_MPEG_RUN_RETIRE_COMPLETION_MISMATCH = -30,
    PSTVNC_APP_MPEG_RUN_PRODUCER_DONE_FAILED = -31,
    PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_EARLY_FINISH = -32,
    PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_STATUS_FAILED = -33,
    PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_JOIN_FAILED = -34,
    PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_OUTCOME_FAILED = -35,
    PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_RELEASE_FAILED = -36,
    PSTVNC_APP_MPEG_RUN_RETIRE_RUNTIME_RELEASE_FAILED = -37,
    PSTVNC_APP_MPEG_RUN_RETIRE_FINALIZE_FAILED = -38,
    PSTVNC_APP_MPEG_RUN_RFB_RESTORE_RELEASE_FAILED = -39
} pstvnc_app_mpeg_run_result_t;

typedef struct pstvnc_app_mpeg_run_status {
    pstvnc_app_mpeg_run_state_t state;
    pstvnc_app_mpeg_run_result_t last_result;
    uint32_t last_allocated_generation;
    uint32_t current_generation;
    int session_teardown_required;
    int retire_invoked;
    int retire_completion_taken;
    int producer_done_published;
    int worker_joined;
} pstvnc_app_mpeg_run_status_t;

typedef struct pstvnc_app_mpeg_run {
    pstvnc_app_mpeg_run_state_t state;
    pstvnc_app_mpeg_run_result_t last_result;

    uint32_t last_allocated_generation;
    uint32_t current_generation;
    int session_teardown_required;

    pstvnc_transport_access_t transport_access;
    pstvnc_config_mpeg_runtime_profile_t profile;

    pstvnc_mpeg_ps2_worker_runtime_t worker_runtime;
    pstvnc_ps2_mpeg_decoder_backend_t decoder_backend;
    pstvnc_mpeg_worker_t worker;
    pstvnc_app_mpeg_frame_consumer_t frame_consumer;

    pstvnc_mpeg_presentation_t *presentation;
    pstvnc_rfb_flow_policy_t *rfb_flow_policy;
    pstvnc_media_clock_t *media_clock;

    int transport_run_open;
    int worker_runtime_owned;
    int worker_started;
    int presentation_armed;
    int frame_consumer_initialized;
    int start_invoked;

    int retire_invoked;
    int retire_completion_taken;
    int producer_done_published;
    int worker_joined;
} pstvnc_app_mpeg_run_t;

/*
 * Initialize one coordinator for one Wire Session lifetime.
 *
 * Reinitializing a coordinator within the same Wire Session would discard its
 * generation-allocation history and is therefore outside this contract.
 */
void pstvnc_app_mpeg_run_init(
    pstvnc_app_mpeg_run_t *run);

/*
 * Execute one start transaction using already-resolved geometry and already-
 * established RFB protection. The selected R7 profile is read internally.
 */
pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_start(
    pstvnc_app_mpeg_run_t *run,
    const pstvnc_mpeg_presentation_geometry_t *geometry,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *media_clock);

/*
 * Service one nonblocking P7 frame-consumer step for the exact current run.
 *
 * Detailed P7 result/evidence is returned unchanged for caller policy. R22 may
 * advance WAIT_FIRST_FRAME to MPEG_OWNED only after exact synchronized P7/P3
 * confirmation. Any live contradiction faults the run and requires outer
 * teardown; this operation performs no retirement or cleanup.
 */
pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_service(
    pstvnc_app_mpeg_run_t *run,
    uint64_t current_tick,
    pstvnc_app_mpeg_frame_service_result_t *service_result);

/*
 * Begin exact-generation retirement from one healthy MPEG_OWNED run.
 *
 * P3 enters RETIRING before the one RETIRE invocation. Crossing that invocation
 * is irreversible even if Transport reports failure; no rollback or cleanup is
 * manufactured here. After RETIRE returns OK, R23C releases P2 through its
 * accepted thaw seam and verifies genuine FULL-refresh debt/HOLD accounting.
 */
pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_begin_retirement(
    pstvnc_app_mpeg_run_t *run);

/*
 * Service one nonblocking retirement/drain step.
 *
 * P7 continues draining in P3 RETIRING while ordinary RFB restoration is
 * permitted through already-thawed P2. Exact RETIRE completion fences producer
 * done; only natural worker completion with no outstanding P7 borrow permits
 * join/outcome/reclaim and Transport finalization. Success stops at
 * RESTORE_PENDING with P3 retained and P2 still thawed.
 */
pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_retirement_service(
    pstvnc_app_mpeg_run_t *run,
    uint64_t current_tick,
    pstvnc_app_mpeg_frame_service_result_t *service_result);

/* Read Application-owned run/generation state without advancing lifecycle. */
pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_status(
    const pstvnc_app_mpeg_run_t *run,
    pstvnc_app_mpeg_run_status_t *status);

#endif /* PSTVNC_APP_MPEG_RUN_H */
