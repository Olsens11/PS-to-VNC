/*
 * File synopsis:
 * Defines R21's Application-owned, trigger-agnostic MPEG run-start transaction.
 * One session-scoped coordinator allocates nonzero monotonically increasing run
 * generations and composes already-accepted RFB protection, Transport run
 * admission, PS2 MPEG execution owners, Presentation and frame-consumer seams.
 *
 * START is the final irreversible action. This module does not choose a product
 * trigger, freeze/thaw RFB, service MPEG frames, retire a started run, activate
 * the Pi MPEG product runtime, or modify ordinary Application lifecycle policy.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-APPLICATION-MPEG-RUN-START-R21.
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
    PSTVNC_APP_MPEG_RUN_FAULTED = -18
} pstvnc_app_mpeg_run_result_t;

typedef struct pstvnc_app_mpeg_run_status {
    pstvnc_app_mpeg_run_state_t state;
    pstvnc_app_mpeg_run_result_t last_result;
    uint32_t last_allocated_generation;
    uint32_t current_generation;
    int session_teardown_required;
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

/* Read Application-owned run/generation state without advancing lifecycle. */
pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_status(
    const pstvnc_app_mpeg_run_t *run,
    pstvnc_app_mpeg_run_status_t *status);

#endif /* PSTVNC_APP_MPEG_RUN_H */
