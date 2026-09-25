/*
 * File synopsis:
 * Focused host composition proof for A004 P10 protected P9 -> R21 start
 * handoff. Accepted Application-owner seams are represented by deterministic
 * stubs; the real P2 flow policy proves continuous freeze and rollback debt.
 */

#include "app_mpeg_activation.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum start_mode {
    START_MODE_EXACT_SUCCESS = 0,
    START_MODE_SUCCESS_BAD_RUN_STATE,
    START_MODE_SUCCESS_BAD_GEOMETRY,
    START_MODE_CLEAN_FAILURE,
    START_MODE_IRREVERSIBLE_FAILURE,
    START_MODE_FAILURE_DIRTY_P3
} start_mode_t;

static start_mode_t start_mode;
static int copy_geometry_allowed;
static int fail_commit;
static int fail_abort;
static unsigned int copy_calls;
static unsigned int start_calls;
static unsigned int status_calls;
static unsigned int commit_calls;
static unsigned int abort_calls;
static unsigned int fault_calls;

static pstvnc_mpeg_presentation_geometry_t accepted_geometry;
static pstvnc_mpeg_presentation_geometry_t observed_start_geometry;
static pstvnc_rfb_flow_policy_t *expected_flow;
static pstvnc_mpeg_presentation_t *expected_presentation;
static const pstvnc_transport_access_t *expected_transport;
static pstvnc_media_clock_t *expected_clock;

static void seed_geometry(
    pstvnc_mpeg_presentation_geometry_t *geometry,
    int32_t offset)
{
    memset(geometry, 0, sizeof(*geometry));

    geometry->base.x = 16 + offset;
    geometry->base.y = 32;
    geometry->base.width = 320;
    geometry->base.height = 240;

    geometry->inner_content.x = 20 + offset;
    geometry->inner_content.y = 36;
    geometry->inner_content.width = 312;
    geometry->inner_content.height = 232;

    geometry->suppression.x = 12 + offset;
    geometry->suppression.y = 28;
    geometry->suppression.width = 328;
    geometry->suppression.height = 248;
}

static int geometry_equal(
    const pstvnc_mpeg_presentation_geometry_t *left,
    const pstvnc_mpeg_presentation_geometry_t *right)
{
    return memcmp(left, right, sizeof(*left)) == 0;
}

static void reset_fixture(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_app_mpeg_run_t *run,
    pstvnc_rfb_flow_policy_t *flow,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_transport_access_t *transport,
    pstvnc_media_clock_t *clock)
{
    memset(calibration, 0, sizeof(*calibration));
    memset(run, 0, sizeof(*run));
    memset(presentation, 0, sizeof(*presentation));
    memset(transport, 0, sizeof(*transport));
    memset(clock, 0, sizeof(*clock));

    pstvnc_rfb_flow_policy_init(flow);
    assert(pstvnc_rfb_flow_policy_set_frozen(flow, 1));

    seed_geometry(&accepted_geometry, 0);
    memset(&observed_start_geometry, 0, sizeof(observed_start_geometry));

    calibration->state =
        PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED;
    calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    calibration->accepted_geometry = accepted_geometry;
    calibration->accepted_geometry_valid = 1u;
    calibration->rfb_flow_policy = flow;
    calibration->presentation = presentation;

    run->state = PSTVNC_APP_MPEG_RUN_IDLE;
    run->last_result = PSTVNC_APP_MPEG_RUN_OK;

    presentation->state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;

    start_mode = START_MODE_EXACT_SUCCESS;
    copy_geometry_allowed = 1;
    fail_commit = 0;
    fail_abort = 0;
    copy_calls = 0u;
    start_calls = 0u;
    status_calls = 0u;
    commit_calls = 0u;
    abort_calls = 0u;
    fault_calls = 0u;

    expected_flow = flow;
    expected_presentation = presentation;
    expected_transport = transport;
    expected_clock = clock;
}

/* ----- P9 public seams ----- */

int pstvnc_app_mpeg_calibration_copy_accepted_geometry(
    const pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_mpeg_presentation_geometry_t *geometry)
{
    copy_calls++;

    if (!copy_geometry_allowed ||
        calibration == NULL ||
        geometry == NULL ||
        calibration->state !=
            PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED ||
        !calibration->accepted_geometry_valid)
        return 0;

    *geometry = calibration->accepted_geometry;
    return 1;
}

pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_commit_protected_handoff(
    pstvnc_app_mpeg_calibration_t *calibration,
    uint32_t run_generation)
{
    commit_calls++;

    assert(calibration != NULL);
    assert(calibration->rfb_flow_policy == expected_flow);
    assert(expected_flow->frozen);
    assert(calibration->presentation == expected_presentation);
    assert(expected_presentation->state ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
    assert(expected_presentation->snapshot_valid);
    assert(expected_presentation->run_generation == run_generation);
    assert(geometry_equal(
        &calibration->accepted_geometry,
        &expected_presentation->geometry));

    if (fail_commit) {
        calibration->state = PSTVNC_APP_MPEG_CALIBRATION_FAULTED;
        calibration->last_result =
            PSTVNC_APP_MPEG_CALIBRATION_HANDOFF_STATE_INVALID;
        return calibration->last_result;
    }

    calibration->accepted_geometry_valid = 0u;
    memset(
        &calibration->accepted_geometry,
        0,
        sizeof(calibration->accepted_geometry));
    calibration->state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    calibration->rfb_flow_policy = NULL;
    calibration->presentation = NULL;
    return PSTVNC_APP_MPEG_CALIBRATION_OK;
}

pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_abort_accepted(
    pstvnc_app_mpeg_calibration_t *calibration)
{
    abort_calls++;

    if (fail_abort) {
        calibration->state = PSTVNC_APP_MPEG_CALIBRATION_FAULTED;
        return PSTVNC_APP_MPEG_CALIBRATION_HANDOFF_STATE_INVALID;
    }

    assert(calibration != NULL);
    assert(calibration->state ==
        PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED);
    assert(calibration->rfb_flow_policy == expected_flow);
    assert(expected_flow->frozen);
    assert(pstvnc_rfb_flow_policy_set_frozen(expected_flow, 0));
    calibration->state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    calibration->accepted_geometry_valid = 0u;
    return PSTVNC_APP_MPEG_CALIBRATION_OK;
}

pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_fault_protected_handoff(
    pstvnc_app_mpeg_calibration_t *calibration)
{
    fault_calls++;

    if (calibration == NULL)
        return PSTVNC_APP_MPEG_CALIBRATION_INVALID;

    if (calibration->state ==
            PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED) {
        calibration->state = PSTVNC_APP_MPEG_CALIBRATION_FAULTED;
        calibration->last_result =
            PSTVNC_APP_MPEG_CALIBRATION_HANDOFF_FAULTED;
        return calibration->last_result;
    }

    return PSTVNC_APP_MPEG_CALIBRATION_ALREADY_FAULTED;
}

/* ----- R21/P3 public seams ----- */

pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_status(
    const pstvnc_app_mpeg_run_t *run,
    pstvnc_app_mpeg_run_status_t *status)
{
    status_calls++;

    if (run == NULL || status == NULL)
        return PSTVNC_APP_MPEG_RUN_INVALID;

    memset(status, 0, sizeof(*status));
    status->state = run->state;
    status->last_result = run->last_result;
    status->last_allocated_generation =
        run->last_allocated_generation;
    status->current_generation = run->current_generation;
    status->session_teardown_required =
        run->session_teardown_required;
    return PSTVNC_APP_MPEG_RUN_OK;
}

pstvnc_mpeg_presentation_state_t
pstvnc_mpeg_presentation_state(
    const pstvnc_mpeg_presentation_t *presentation)
{
    if (presentation == NULL)
        return PSTVNC_MPEG_PRESENTATION_STATE_INVALID;

    return presentation->state;
}

int pstvnc_mpeg_presentation_snapshot(
    const pstvnc_mpeg_presentation_t *presentation,
    pstvnc_mpeg_presentation_geometry_t *geometry,
    uint32_t *run_generation)
{
    if (presentation == NULL ||
        geometry == NULL ||
        run_generation == NULL ||
        !presentation->snapshot_valid)
        return 0;

    *geometry = presentation->geometry;
    *run_generation = presentation->run_generation;
    return 1;
}

pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_start(
    pstvnc_app_mpeg_run_t *run,
    const pstvnc_mpeg_presentation_geometry_t *geometry,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *media_clock)
{
    uint32_t generation;

    start_calls++;

    assert(run != NULL);
    assert(geometry != NULL);
    assert(rfb_flow_policy == expected_flow);
    assert(transport_access == expected_transport);
    assert(presentation == expected_presentation);
    assert(media_clock == expected_clock);
    assert(rfb_flow_policy->frozen);
    observed_start_geometry = *geometry;

    if (start_mode == START_MODE_CLEAN_FAILURE) {
        run->last_result = PSTVNC_APP_MPEG_RUN_TRANSPORT_OPEN_FAILED;
        return run->last_result;
    }

    if (start_mode == START_MODE_FAILURE_DIRTY_P3) {
        presentation->state =
            PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
        presentation->snapshot_valid = 1u;
        presentation->run_generation =
            run->last_allocated_generation + 1u;
        presentation->geometry = *geometry;
        run->last_result = PSTVNC_APP_MPEG_RUN_TRANSPORT_OPEN_FAILED;
        return run->last_result;
    }

    run->last_allocated_generation++;
    generation = run->last_allocated_generation;
    run->current_generation = generation;
    run->presentation = presentation;
    run->rfb_flow_policy = rfb_flow_policy;
    run->media_clock = media_clock;

    presentation->state =
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
    presentation->snapshot_valid = 1u;
    presentation->run_generation = generation;
    presentation->geometry = *geometry;

    if (start_mode == START_MODE_IRREVERSIBLE_FAILURE) {
        run->state = PSTVNC_APP_MPEG_RUN_FAULTED;
        run->last_result = PSTVNC_APP_MPEG_RUN_START_FAILED;
        run->session_teardown_required = 1;
        return run->last_result;
    }

    run->state =
        start_mode == START_MODE_SUCCESS_BAD_RUN_STATE
            ? PSTVNC_APP_MPEG_RUN_MPEG_OWNED
            : PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME;
    run->last_result = PSTVNC_APP_MPEG_RUN_OK;

    if (start_mode == START_MODE_SUCCESS_BAD_GEOMETRY)
        presentation->geometry.base.x++;

    return PSTVNC_APP_MPEG_RUN_OK;
}

/* ----- P10 proofs ----- */

static void test_admission_rejects_before_start(void)
{
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_transport_access_t transport;
    pstvnc_media_clock_t clock;

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);

    calibration.state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED);
    assert(start_calls == 0u);

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    flow.frozen = 0u;
    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED);
    assert(start_calls == 0u);

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    presentation.state = PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
    presentation.snapshot_valid = 1u;
    presentation.run_generation = 7u;
    presentation.geometry = accepted_geometry;
    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED);
    assert(start_calls == 0u);

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    run.state = PSTVNC_APP_MPEG_RUN_MPEG_OWNED;
    run.current_generation = 7u;
    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED);
    assert(copy_calls == 0u);
    assert(start_calls == 0u);

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    run.state = PSTVNC_APP_MPEG_RUN_FAULTED;
    run.session_teardown_required = 1;
    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED);
    assert(copy_calls == 0u);
    assert(start_calls == 0u);

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    copy_geometry_allowed = 0;
    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED);
    assert(copy_calls == 1u);
    assert(start_calls == 0u);
    assert(flow.frozen);
}

static void test_exact_success_transfers_ownership_without_thaw(void)
{
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_transport_access_t transport;
    pstvnc_media_clock_t clock;
    uint32_t generation;

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_OK);

    generation = run.current_generation;
    assert(generation == 1u);
    assert(copy_calls == 1u);
    assert(start_calls == 1u);
    assert(commit_calls == 1u);
    assert(abort_calls == 0u);
    assert(fault_calls == 0u);
    assert(geometry_equal(
        &observed_start_geometry,
        &accepted_geometry));
    assert(flow.frozen);
    assert(!flow.full_refresh_pending);
    assert(pstvnc_rfb_flow_policy_next_request(&flow) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);
    assert(calibration.state == PSTVNC_APP_MPEG_CALIBRATION_IDLE);
    assert(!calibration.accepted_geometry_valid);
    assert(run.state ==
        PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME);
    assert(run.current_generation == generation);
    assert(presentation.state ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
    assert(presentation.run_generation == generation);
    assert(geometry_equal(
        &presentation.geometry,
        &accepted_geometry));

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED);
    assert(start_calls == 1u);
}

static void test_clean_prestart_failure_uses_abort_and_full_debt(void)
{
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_transport_access_t transport;
    pstvnc_media_clock_t clock;

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    flow.request_outstanding = 1u;
    start_mode = START_MODE_CLEAN_FAILURE;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_START_ROLLED_BACK);

    assert(start_calls == 1u);
    assert(abort_calls == 1u);
    assert(fault_calls == 0u);
    assert(calibration.state == PSTVNC_APP_MPEG_CALIBRATION_IDLE);
    assert(!flow.frozen);
    assert(flow.full_refresh_pending);
    assert(pstvnc_rfb_flow_policy_next_request(&flow) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);

    assert(pstvnc_rfb_flow_policy_record_update_complete(&flow));
    assert(pstvnc_rfb_flow_policy_next_request(&flow) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);
}

static void test_irreversible_or_dirty_failure_never_thaws(void)
{
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_transport_access_t transport;
    pstvnc_media_clock_t clock;

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    start_mode = START_MODE_IRREVERSIBLE_FAILURE;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_START_FAILURE_TEARDOWN_REQUIRED);
    assert(start_calls == 1u);
    assert(abort_calls == 0u);
    assert(fault_calls == 1u);
    assert(flow.frozen);
    assert(!flow.full_refresh_pending);
    assert(calibration.state == PSTVNC_APP_MPEG_CALIBRATION_FAULTED);
    assert(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    assert(run.current_generation != 0u);
    assert(presentation.state ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    start_mode = START_MODE_FAILURE_DIRTY_P3;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_START_FAILURE_TEARDOWN_REQUIRED);
    assert(abort_calls == 0u);
    assert(fault_calls == 1u);
    assert(flow.frozen);
    assert(calibration.state == PSTVNC_APP_MPEG_CALIBRATION_FAULTED);
}

static void test_success_requires_independent_exact_proof(void)
{
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_transport_access_t transport;
    pstvnc_media_clock_t clock;

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    start_mode = START_MODE_SUCCESS_BAD_RUN_STATE;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_SUCCESS_PROOF_TEARDOWN_REQUIRED);
    assert(start_calls == 1u);
    assert(commit_calls == 0u);
    assert(abort_calls == 0u);
    assert(fault_calls == 1u);
    assert(flow.frozen);
    assert(calibration.state == PSTVNC_APP_MPEG_CALIBRATION_FAULTED);

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    start_mode = START_MODE_SUCCESS_BAD_GEOMETRY;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_SUCCESS_PROOF_TEARDOWN_REQUIRED);
    assert(commit_calls == 0u);
    assert(abort_calls == 0u);
    assert(fault_calls == 1u);
    assert(flow.frozen);
}

static void test_commit_or_rollback_failure_requires_teardown(void)
{
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_transport_access_t transport;
    pstvnc_media_clock_t clock;

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    fail_commit = 1;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_HANDOFF_COMMIT_TEARDOWN_REQUIRED);
    assert(start_calls == 1u);
    assert(commit_calls == 1u);
    assert(abort_calls == 0u);
    assert(flow.frozen);
    assert(calibration.state == PSTVNC_APP_MPEG_CALIBRATION_FAULTED);

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);
    start_mode = START_MODE_CLEAN_FAILURE;
    fail_abort = 1;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_ROLLBACK_TEARDOWN_REQUIRED);
    assert(start_calls == 1u);
    assert(abort_calls == 1u);
    assert(flow.frozen);
    assert(calibration.state == PSTVNC_APP_MPEG_CALIBRATION_FAULTED);
}

static void rearm_new_protected_acceptance(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_rfb_flow_policy_t *flow,
    pstvnc_mpeg_presentation_t *presentation,
    int32_t geometry_offset)
{
    pstvnc_rfb_flow_policy_init(flow);
    assert(pstvnc_rfb_flow_policy_set_frozen(flow, 1));

    seed_geometry(&accepted_geometry, geometry_offset);
    calibration->state =
        PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED;
    calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    calibration->accepted_geometry = accepted_geometry;
    calibration->accepted_geometry_valid = 1u;
    calibration->rfb_flow_policy = flow;
    calibration->presentation = presentation;

    presentation->state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
    presentation->snapshot_valid = 0u;
    presentation->run_generation = 0u;
    memset(&presentation->geometry, 0, sizeof(presentation->geometry));
}

static void test_generation_history_stays_run_owned_across_cycles(void)
{
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_transport_access_t transport;
    pstvnc_media_clock_t clock;
    uint32_t first_generation;

    reset_fixture(
        &calibration, &run, &flow, &presentation, &transport, &clock);

    run.last_allocated_generation = 40u;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_OK);
    first_generation = run.current_generation;
    assert(first_generation == 41u);

    /*
     * Model only the accepted R24 terminal facts. P10 owns none of these
     * transitions and carries no generation state between calls.
     */
    run.state = PSTVNC_APP_MPEG_RUN_IDLE;
    run.current_generation = 0u;
    run.session_teardown_required = 0;
    run.presentation = NULL;
    run.rfb_flow_policy = NULL;
    run.media_clock = NULL;

    rearm_new_protected_acceptance(
        &calibration,
        &flow,
        &presentation,
        2);

    expected_flow = &flow;
    expected_presentation = &presentation;
    start_calls = 0u;
    commit_calls = 0u;
    copy_calls = 0u;
    fault_calls = 0u;
    abort_calls = 0u;

    assert(pstvnc_app_mpeg_activation_start_protected(
        &calibration, &run, &transport, &clock) ==
        PSTVNC_APP_MPEG_ACTIVATION_OK);
    assert(run.current_generation == first_generation + 1u);
    assert(run.last_allocated_generation == first_generation + 1u);
    assert(start_calls == 1u);
    assert(commit_calls == 1u);
}

int main(void)
{
    test_admission_rejects_before_start();
    test_exact_success_transfers_ownership_without_thaw();
    test_clean_prestart_failure_uses_abort_and_full_debt();
    test_irreversible_or_dirty_failure_never_thaws();
    test_success_requires_independent_exact_proof();
    test_commit_or_rollback_failure_requires_teardown();
    test_generation_history_stays_run_owned_across_cycles();

    puts("APP_MPEG_ACTIVATION_TEST=PASS");
    return 0;
}
