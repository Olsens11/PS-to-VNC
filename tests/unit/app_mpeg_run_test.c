/*
 * R21 focused host proof for the trigger-agnostic Application MPEG run-start
 * transaction. Lower owners are represented only through their accepted public
 * seams so this fixture can prove Application ordering, generation allocation,
 * geometry mapping, reverse-order unwind and the irreversible START boundary.
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_mpeg_run.h"

static int failures;

#define CHECK(condition) do {     if (!(condition)) {         fprintf(stderr, "CHECK failed at %s:%d: %s\n",             __FILE__, __LINE__, #condition);         failures++;     } } while (0)

typedef enum test_event {
    EV_OPEN = 1,
    EV_RUNTIME_INIT,
    EV_RUNTIME_OPS,
    EV_BACKEND_INIT,
    EV_BACKEND_OPS,
    EV_WORKER_START,
    EV_PRESENTATION_ARM,
    EV_PRESENTATION_SNAPSHOT,
    EV_CONSUMER_INIT,
    EV_START,
    EV_PRESENTATION_ABORT,
    EV_WORKER_STOP,
    EV_WORKER_JOIN,
    EV_WORKER_OUTCOME,
    EV_WORKER_RELEASE,
    EV_RUNTIME_RELEASE,
    EV_ABORT
} test_event_t;

static test_event_t events[128];
static size_t event_count;

static pstvnc_config_mpeg_runtime_profile_t selected_profile;
static int publication_allowed;
static pstvnc_transport_result_t open_result;
static int runtime_init_result;
static int runtime_init_retains_resources_on_failure;
static int runtime_ops_result;
static int backend_ops_result;
static pstvnc_mpeg_worker_result_t worker_start_result;
static int worker_start_retains_partial;
static int presentation_arm_result;
static int presentation_snapshot_result;
static pstvnc_app_mpeg_frame_result_t consumer_init_result;
static pstvnc_transport_result_t start_result;
static int presentation_abort_result;
static pstvnc_mpeg_worker_result_t worker_stop_result;
static pstvnc_mpeg_worker_result_t worker_join_result;
static pstvnc_mpeg_worker_result_t worker_outcome_result;
static pstvnc_mpeg_worker_result_t worker_release_result;
static int runtime_release_result;
static pstvnc_transport_result_t abort_result;

static uint32_t observed_worker_generation;
static pstvnc_transport_mpeg_start_request_t observed_start;
static pstvnc_mpeg_presentation_geometry_t observed_presentation_geometry;
static uint32_t observed_presentation_generation;

static void record_event(test_event_t event)
{
    CHECK(event_count < sizeof(events) / sizeof(events[0]));
    if (event_count < sizeof(events) / sizeof(events[0]))
        events[event_count++] = event;
}

static int event_index(test_event_t event)
{
    size_t i;

    for (i = 0u; i < event_count; i++) {
        if (events[i] == event)
            return (int)i;
    }

    return -1;
}

static pstvnc_mpeg_presentation_geometry_t valid_geometry(void)
{
    pstvnc_mpeg_presentation_geometry_t geometry;

    memset(&geometry, 0, sizeof(geometry));
    geometry.base.x = 32;
    geometry.base.y = 16;
    geometry.base.width = 640;
    geometry.base.height = 448;

    geometry.inner_content.x = 48;
    geometry.inner_content.y = 32;
    geometry.inner_content.width = 608;
    geometry.inner_content.height = 416;

    geometry.suppression.x = 16;
    geometry.suppression.y = 0;
    geometry.suppression.width = 672;
    geometry.suppression.height = 480;
    return geometry;
}

static void reset_fixture(void)
{
    memset(events, 0, sizeof(events));
    event_count = 0u;

    memset(&selected_profile, 0, sizeof(selected_profile));
    selected_profile.decoder.max_width = 640u;
    selected_profile.decoder.max_height = 448u;
    selected_profile.decoder.bytes_per_pixel = 2u;
    selected_profile.decoder.feed_payload_capacity = 4096u;
    selected_profile.decoder.transfer_alignment = 16u;
    selected_profile.decoder.buffer_alignment = 16u;
    selected_profile.worker.worker_stack_bytes = 16384u;
    selected_profile.worker.worker_priority = 60;
    selected_profile.ps2_worker_runtime.join_poll_delay_us = 1000u;
    selected_profile.ps2_worker_runtime.join_poll_max_count = 10u;
    selected_profile.scheduler.frame_duration_ticks = 3000u;
    selected_profile.scheduler.late_drop_ticks = 6000u;

    publication_allowed = 0;
    open_result = PSTVNC_TRANSPORT_OK;
    runtime_init_result = 0;
    runtime_init_retains_resources_on_failure = 0;
    runtime_ops_result = 0;
    backend_ops_result = 0;
    worker_start_result = PSTVNC_MPEG_WORKER_OK;
    worker_start_retains_partial = 0;
    presentation_arm_result = 1;
    presentation_snapshot_result = 1;
    consumer_init_result = PSTVNC_APP_MPEG_FRAME_OK;
    start_result = PSTVNC_TRANSPORT_OK;
    presentation_abort_result = 1;
    worker_stop_result = PSTVNC_MPEG_WORKER_OK;
    worker_join_result = PSTVNC_MPEG_WORKER_OK;
    worker_outcome_result = PSTVNC_MPEG_WORKER_OK;
    worker_release_result = PSTVNC_MPEG_WORKER_OK;
    runtime_release_result = 0;
    abort_result = PSTVNC_TRANSPORT_OK;

    observed_worker_generation = 0u;
    memset(&observed_start, 0, sizeof(observed_start));
    memset(&observed_presentation_geometry, 0,
        sizeof(observed_presentation_geometry));
    observed_presentation_generation = 0u;
}

static void reset_attempt_observation(void)
{
    memset(events, 0, sizeof(events));
    event_count = 0u;
    observed_worker_generation = 0u;
    memset(&observed_start, 0, sizeof(observed_start));
}

const pstvnc_config_mpeg_runtime_profile_t *
pstvnc_config_mpeg_runtime_profile_selected(void)
{
    return &selected_profile;
}

int pstvnc_rfb_flow_policy_allows_remote_publication(
    const pstvnc_rfb_flow_policy_t *policy)
{
    return policy != NULL ? publication_allowed : 0;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_run_open(
    const pstvnc_transport_access_t *access)
{
    CHECK(access != NULL);
    record_event(EV_OPEN);
    return open_result;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_run_abort_pre_start(
    const pstvnc_transport_access_t *access)
{
    CHECK(access != NULL);
    record_event(EV_ABORT);
    return abort_result;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_send_start(
    const pstvnc_transport_access_t *access,
    const pstvnc_transport_mpeg_start_request_t *request)
{
    CHECK(access != NULL);
    CHECK(request != NULL);
    record_event(EV_START);
    if (request != NULL)
        observed_start = *request;
    return start_result;
}

int pstvnc_mpeg_ps2_worker_runtime_init(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime,
    const pstvnc_mpeg_ps2_worker_runtime_values_t *values)
{
    CHECK(runtime != NULL);
    CHECK(values != NULL);
    record_event(EV_RUNTIME_INIT);
    memset(runtime, 0, sizeof(*runtime));

    if (runtime_init_result != 0) {
        runtime->resources_owned =
            runtime_init_retains_resources_on_failure;
        return runtime_init_result;
    }

    runtime->initialized = 1;
    runtime->resources_owned = 1;
    return 0;
}

int pstvnc_mpeg_ps2_worker_runtime_operations(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime,
    pstvnc_mpeg_decoder_memory_ops_t *decoder_memory_ops,
    pstvnc_mpeg_decoder_sync_ops_t *decoder_sync_ops,
    pstvnc_mpeg_worker_memory_ops_t *worker_memory_ops,
    pstvnc_mpeg_worker_thread_ops_t *worker_thread_ops,
    pstvnc_mpeg_worker_sync_ops_t *worker_sync_ops,
    pstvnc_mpeg_worker_event_ops_t *frame_event_ops)
{
    CHECK(runtime != NULL);
    record_event(EV_RUNTIME_OPS);
    memset(decoder_memory_ops, 0, sizeof(*decoder_memory_ops));
    memset(decoder_sync_ops, 0, sizeof(*decoder_sync_ops));
    memset(worker_memory_ops, 0, sizeof(*worker_memory_ops));
    memset(worker_thread_ops, 0, sizeof(*worker_thread_ops));
    memset(worker_sync_ops, 0, sizeof(*worker_sync_ops));
    memset(frame_event_ops, 0, sizeof(*frame_event_ops));
    return runtime_ops_result;
}

int pstvnc_mpeg_ps2_worker_runtime_release(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime)
{
    record_event(EV_RUNTIME_RELEASE);
    if (runtime_release_result == 0) {
        runtime->initialized = 0;
        runtime->resources_owned = 0;
    }
    return runtime_release_result;
}

void pstvnc_ps2_mpeg_decoder_backend_init(
    pstvnc_ps2_mpeg_decoder_backend_t *backend)
{
    record_event(EV_BACKEND_INIT);
    memset(backend, 0, sizeof(*backend));
}

int pstvnc_ps2_mpeg_decoder_backend_platform_ops(
    pstvnc_ps2_mpeg_decoder_backend_t *backend,
    pstvnc_mpeg_decoder_platform_ops_t *ops)
{
    CHECK(backend != NULL);
    CHECK(ops != NULL);
    record_event(EV_BACKEND_OPS);
    memset(ops, 0, sizeof(*ops));
    return backend_ops_result;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_start(
    pstvnc_mpeg_worker_t *worker,
    uint32_t generation,
    const pstvnc_mpeg_worker_values_t *values,
    const pstvnc_mpeg_decoder_config_t *decoder_config,
    const pstvnc_mpeg_decoder_memory_ops_t *decoder_memory_ops,
    const pstvnc_mpeg_decoder_sync_ops_t *decoder_sync_ops,
    const pstvnc_mpeg_decoder_platform_ops_t *decoder_platform_ops,
    const pstvnc_mpeg_worker_memory_ops_t *memory_ops,
    const pstvnc_mpeg_worker_thread_ops_t *thread_ops,
    const pstvnc_mpeg_worker_sync_ops_t *sync_ops,
    const pstvnc_mpeg_worker_event_ops_t *event_ops)
{
    (void)values;
    (void)decoder_config;
    (void)decoder_memory_ops;
    (void)decoder_sync_ops;
    (void)decoder_platform_ops;
    (void)memory_ops;
    (void)thread_ops;
    (void)sync_ops;
    (void)event_ops;

    record_event(EV_WORKER_START);
    observed_worker_generation = generation;
    memset(worker, 0, sizeof(*worker));

    if (worker_start_result == PSTVNC_MPEG_WORKER_OK) {
        worker->initialized = 1;
        worker->thread_started = 1;
        worker->run_generation = generation;
    } else if (worker_start_retains_partial) {
        worker->initialized = 1;
        worker->run_generation = generation;
    }

    return worker_start_result;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_request_stop(
    pstvnc_mpeg_worker_t *worker,
    uint32_t generation)
{
    CHECK(worker != NULL);
    CHECK(worker->run_generation == generation);
    record_event(EV_WORKER_STOP);
    return worker_stop_result;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_join(
    pstvnc_mpeg_worker_t *worker,
    uint32_t generation)
{
    CHECK(worker != NULL);
    CHECK(worker->run_generation == generation);
    record_event(EV_WORKER_JOIN);
    if (worker_join_result == PSTVNC_MPEG_WORKER_OK) {
        worker->worker_finished = 1;
        worker->thread_joined = 1;
    }
    return worker_join_result;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_outcome(
    const pstvnc_mpeg_worker_t *worker,
    uint32_t generation,
    pstvnc_mpeg_worker_outcome_t *outcome)
{
    CHECK(worker != NULL);
    CHECK(outcome != NULL);
    record_event(EV_WORKER_OUTCOME);
    if (worker_outcome_result == PSTVNC_MPEG_WORKER_OK) {
        memset(outcome, 0, sizeof(*outcome));
        outcome->kind = PSTVNC_MPEG_WORKER_OUTCOME_STOPPED;
        outcome->worker_result = PSTVNC_MPEG_WORKER_OK;
        outcome->run_generation = generation;
    }
    return worker_outcome_result;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_release(
    pstvnc_mpeg_worker_t *worker,
    uint32_t generation)
{
    CHECK(worker != NULL);
    CHECK(worker->run_generation == generation);
    record_event(EV_WORKER_RELEASE);
    if (worker_release_result == PSTVNC_MPEG_WORKER_OK)
        worker->initialized = 0;
    return worker_release_result;
}

pstvnc_mpeg_presentation_state_t pstvnc_mpeg_presentation_state(
    const pstvnc_mpeg_presentation_t *presentation)
{
    return presentation != NULL
        ? presentation->state
        : PSTVNC_MPEG_PRESENTATION_STATE_INVALID;
}

int pstvnc_mpeg_presentation_arm(
    pstvnc_mpeg_presentation_t *presentation,
    const pstvnc_mpeg_presentation_geometry_t *geometry,
    uint32_t generation)
{
    record_event(EV_PRESENTATION_ARM);
    if (!presentation_arm_result)
        return 0;

    presentation->geometry = *geometry;
    presentation->run_generation = generation;
    presentation->snapshot_valid = 1u;
    presentation->state = PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
    observed_presentation_geometry = *geometry;
    observed_presentation_generation = generation;
    return 1;
}

int pstvnc_mpeg_presentation_snapshot(
    const pstvnc_mpeg_presentation_t *presentation,
    pstvnc_mpeg_presentation_geometry_t *geometry,
    uint32_t *generation)
{
    record_event(EV_PRESENTATION_SNAPSHOT);
    if (!presentation_snapshot_result)
        return 0;
    *geometry = presentation->geometry;
    *generation = presentation->run_generation;
    return 1;
}

int pstvnc_mpeg_presentation_abort_pending(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t generation)
{
    record_event(EV_PRESENTATION_ABORT);
    if (!presentation_abort_result)
        return 0;
    CHECK(presentation->run_generation == generation);
    presentation->state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
    presentation->snapshot_valid = 0u;
    presentation->run_generation = 0u;
    memset(&presentation->geometry, 0, sizeof(presentation->geometry));
    return 1;
}

pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_init(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t generation,
    pstvnc_mpeg_worker_t *worker,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock,
    const pstvnc_mpeg_scheduler_profile_t *scheduler_profile)
{
    record_event(EV_CONSUMER_INIT);
    if (consumer_init_result != PSTVNC_APP_MPEG_FRAME_OK)
        return consumer_init_result;

    memset(consumer, 0, sizeof(*consumer));
    consumer->worker = worker;
    consumer->presentation = presentation;
    consumer->clock = clock;
    consumer->scheduler_profile = *scheduler_profile;
    consumer->run_generation = generation;
    consumer->initialized = 1;
    return PSTVNC_APP_MPEG_FRAME_OK;
}

static void init_inputs(
    pstvnc_app_mpeg_run_t *run,
    pstvnc_rfb_flow_policy_t *policy,
    pstvnc_transport_access_t *access,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock)
{
    pstvnc_app_mpeg_run_init(run);
    memset(policy, 0, sizeof(*policy));
    policy->frozen = 1u;
    memset(access, 0, sizeof(*access));
    access->opaque_ticket = 77u;
    memset(presentation, 0, sizeof(*presentation));
    presentation->state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
    memset(clock, 0, sizeof(*clock));
}

static void test_success_orders_all_owners_and_maps_exact_geometry(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();
    pstvnc_app_mpeg_run_status_t status;

    reset_fixture();
    init_inputs(&run, &policy, &access, &presentation, &clock);

    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_OK);

    CHECK(run.state == PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME);
    CHECK(run.current_generation == 1u);
    CHECK(run.last_allocated_generation == 1u);
    CHECK(run.session_teardown_required == 0);
    CHECK(run.transport_run_open == 1);
    CHECK(run.worker_runtime_owned == 1);
    CHECK(run.worker_started == 1);
    CHECK(run.presentation_armed == 1);
    CHECK(run.frame_consumer_initialized == 1);
    CHECK(run.start_invoked == 1);

    CHECK(event_index(EV_OPEN) < event_index(EV_RUNTIME_INIT));
    CHECK(event_index(EV_RUNTIME_INIT) < event_index(EV_RUNTIME_OPS));
    CHECK(event_index(EV_RUNTIME_OPS) < event_index(EV_BACKEND_OPS));
    CHECK(event_index(EV_BACKEND_OPS) < event_index(EV_WORKER_START));
    CHECK(event_index(EV_WORKER_START) < event_index(EV_PRESENTATION_ARM));
    CHECK(event_index(EV_PRESENTATION_ARM) <
        event_index(EV_PRESENTATION_SNAPSHOT));
    CHECK(event_index(EV_PRESENTATION_SNAPSHOT) <
        event_index(EV_CONSUMER_INIT));
    CHECK(event_index(EV_CONSUMER_INIT) < event_index(EV_START));
    CHECK((size_t)(event_index(EV_START) + 1) == event_count);

    CHECK(observed_worker_generation == 1u);
    CHECK(observed_presentation_generation == 1u);
    CHECK(observed_presentation_geometry.inner_content.x ==
        geometry.inner_content.x);
    CHECK(observed_presentation_geometry.inner_content.width ==
        geometry.inner_content.width);

    CHECK(observed_start.generation == 1u);
    CHECK(observed_start.base_x == (uint32_t)geometry.base.x);
    CHECK(observed_start.base_y == (uint32_t)geometry.base.y);
    CHECK(observed_start.base_width == (uint32_t)geometry.base.width);
    CHECK(observed_start.base_height == (uint32_t)geometry.base.height);
    CHECK(observed_start.suppression_x ==
        (uint32_t)geometry.suppression.x);
    CHECK(observed_start.suppression_y ==
        (uint32_t)geometry.suppression.y);
    CHECK(observed_start.suppression_width ==
        (uint32_t)geometry.suppression.width);
    CHECK(observed_start.suppression_height ==
        (uint32_t)geometry.suppression.height);

    CHECK(pstvnc_app_mpeg_run_status(&run, &status) ==
        PSTVNC_APP_MPEG_RUN_OK);
    CHECK(status.state == PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME);
    CHECK(status.current_generation == 1u);
    CHECK(status.last_allocated_generation == 1u);
}

static void test_generation_is_never_reused_across_prestart_failures(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();

    reset_fixture();
    init_inputs(&run, &policy, &access, &presentation, &clock);

    open_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_TRANSPORT_OPEN_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_IDLE);
    CHECK(run.current_generation == 0u);
    CHECK(run.last_allocated_generation == 1u);

    reset_attempt_observation();
    open_result = PSTVNC_TRANSPORT_OK;
    backend_ops_result = -1;
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_BACKEND_OPS_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_IDLE);
    CHECK(run.current_generation == 0u);
    CHECK(run.last_allocated_generation == 2u);
    CHECK(event_index(EV_RUNTIME_RELEASE) < event_index(EV_ABORT));

    reset_attempt_observation();
    backend_ops_result = 0;
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.current_generation == 3u);
    CHECK(observed_worker_generation == 3u);
    CHECK(observed_start.generation == 3u);
}

static void test_invalid_or_unprotected_input_fails_before_open(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();

    reset_fixture();
    init_inputs(&run, &policy, &access, &presentation, &clock);

    geometry.base.width = 639;
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_GEOMETRY_INVALID);
    CHECK(event_index(EV_OPEN) < 0);
    CHECK(run.last_allocated_generation == 0u);

    geometry = valid_geometry();
    geometry.inner_content.width = 607;
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_GEOMETRY_INVALID);
    CHECK(event_index(EV_OPEN) < 0);

    geometry = valid_geometry();
    publication_allowed = 1;
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_RFB_NOT_PROTECTED);
    CHECK(event_index(EV_OPEN) < 0);

    publication_allowed = 0;
    presentation.state = PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_PRESENTATION_NOT_IDLE);
    CHECK(event_index(EV_OPEN) < 0);
}

static void test_generation_exhaustion_faults_without_wrap(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();

    reset_fixture();
    init_inputs(&run, &policy, &access, &presentation, &clock);
    run.last_allocated_generation = UINT32_MAX;

    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_GENERATION_EXHAUSTED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.last_allocated_generation == UINT32_MAX);
    CHECK(run.current_generation == 0u);
    CHECK(event_index(EV_OPEN) < 0);
}

typedef enum failure_stage {
    FAIL_OPEN = 0,
    FAIL_RUNTIME_INIT,
    FAIL_RUNTIME_OPS,
    FAIL_BACKEND_OPS,
    FAIL_WORKER_START,
    FAIL_PRESENTATION_ARM,
    FAIL_CONSUMER_INIT,
    FAIL_PRESENTATION_SNAPSHOT
} failure_stage_t;

static pstvnc_app_mpeg_run_result_t expected_stage_result(
    failure_stage_t stage)
{
    switch (stage) {
        case FAIL_OPEN:
            return PSTVNC_APP_MPEG_RUN_TRANSPORT_OPEN_FAILED;
        case FAIL_RUNTIME_INIT:
            return PSTVNC_APP_MPEG_RUN_RUNTIME_INIT_FAILED;
        case FAIL_RUNTIME_OPS:
            return PSTVNC_APP_MPEG_RUN_RUNTIME_OPS_FAILED;
        case FAIL_BACKEND_OPS:
            return PSTVNC_APP_MPEG_RUN_BACKEND_OPS_FAILED;
        case FAIL_WORKER_START:
            return PSTVNC_APP_MPEG_RUN_WORKER_START_FAILED;
        case FAIL_PRESENTATION_ARM:
            return PSTVNC_APP_MPEG_RUN_PRESENTATION_ARM_FAILED;
        case FAIL_CONSUMER_INIT:
            return PSTVNC_APP_MPEG_RUN_FRAME_CONSUMER_INIT_FAILED;
        case FAIL_PRESENTATION_SNAPSHOT:
            return PSTVNC_APP_MPEG_RUN_PRESENTATION_SNAPSHOT_FAILED;
    }
    return PSTVNC_APP_MPEG_RUN_INVALID;
}

static void test_every_prestart_failure_unwinds_without_start(void)
{
    int stage_value;

    for (stage_value = FAIL_OPEN;
         stage_value <= FAIL_PRESENTATION_SNAPSHOT;
         stage_value++) {
        pstvnc_app_mpeg_run_t run;
        pstvnc_rfb_flow_policy_t policy;
        pstvnc_transport_access_t access;
        pstvnc_mpeg_presentation_t presentation;
        pstvnc_media_clock_t clock;
        pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();
        failure_stage_t stage = (failure_stage_t)stage_value;

        reset_fixture();
        init_inputs(&run, &policy, &access, &presentation, &clock);

        switch (stage) {
            case FAIL_OPEN:
                open_result = PSTVNC_TRANSPORT_FAILED;
                break;
            case FAIL_RUNTIME_INIT:
                runtime_init_result = -1;
                runtime_init_retains_resources_on_failure = 1;
                break;
            case FAIL_RUNTIME_OPS:
                runtime_ops_result = -1;
                break;
            case FAIL_BACKEND_OPS:
                backend_ops_result = -1;
                break;
            case FAIL_WORKER_START:
                worker_start_result =
                    PSTVNC_MPEG_WORKER_STACK_ALLOCATION_FAILED;
                break;
            case FAIL_PRESENTATION_ARM:
                presentation_arm_result = 0;
                break;
            case FAIL_CONSUMER_INIT:
                consumer_init_result = PSTVNC_APP_MPEG_FRAME_INVALID;
                break;
            case FAIL_PRESENTATION_SNAPSHOT:
                presentation_snapshot_result = 0;
                break;
        }

        CHECK(pstvnc_app_mpeg_run_start(
            &run, &geometry, &policy, &access, &presentation, &clock) ==
            expected_stage_result(stage));
        CHECK(event_index(EV_START) < 0);
        CHECK(run.state == PSTVNC_APP_MPEG_RUN_IDLE);
        CHECK(run.current_generation == 0u);
        CHECK(run.last_allocated_generation == 1u);

        if (event_index(EV_PRESENTATION_ABORT) >= 0) {
            CHECK(event_index(EV_PRESENTATION_ABORT) <
                event_index(EV_WORKER_STOP));
        }
        if (event_index(EV_WORKER_STOP) >= 0) {
            CHECK(event_index(EV_WORKER_STOP) < event_index(EV_WORKER_JOIN));
            CHECK(event_index(EV_WORKER_JOIN) <
                event_index(EV_WORKER_OUTCOME));
            CHECK(event_index(EV_WORKER_OUTCOME) <
                event_index(EV_WORKER_RELEASE));
            CHECK(event_index(EV_WORKER_RELEASE) <
                event_index(EV_RUNTIME_RELEASE));
        }
        if (event_index(EV_RUNTIME_RELEASE) >= 0)
            CHECK(event_index(EV_RUNTIME_RELEASE) < event_index(EV_ABORT));

        if (stage == FAIL_OPEN)
            CHECK(event_index(EV_ABORT) < 0);
        else
            CHECK(event_index(EV_ABORT) >= 0);
    }
}

static void test_cleanup_failure_faults_and_blocks_retry(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();

    reset_fixture();
    init_inputs(&run, &policy, &access, &presentation, &clock);
    consumer_init_result = PSTVNC_APP_MPEG_FRAME_INVALID;
    worker_stop_result = PSTVNC_MPEG_WORKER_SYNC_FAILED;

    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_CLEANUP_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(event_index(EV_PRESENTATION_ABORT) >= 0);
    CHECK(event_index(EV_WORKER_STOP) >= 0);
    CHECK(event_index(EV_WORKER_JOIN) < 0);
    CHECK(event_index(EV_RUNTIME_RELEASE) < 0);
    CHECK(event_index(EV_ABORT) < 0);
    CHECK(event_index(EV_START) < 0);

    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(event_count == 0u);
}

static void test_start_failure_is_irreversible_and_never_prestart_aborts(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();

    reset_fixture();
    init_inputs(&run, &policy, &access, &presentation, &clock);
    start_result = PSTVNC_TRANSPORT_FAILED;

    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_START_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(run.current_generation == 1u);
    CHECK(run.start_invoked == 1);
    CHECK(event_index(EV_START) >= 0);
    CHECK((size_t)(event_index(EV_START) + 1) == event_count);
    CHECK(event_index(EV_ABORT) < 0);
    CHECK(event_index(EV_PRESENTATION_ABORT) < 0);
    CHECK(event_index(EV_WORKER_STOP) < 0);
    CHECK(event_index(EV_RUNTIME_RELEASE) < 0);
}

int main(void)
{
    test_success_orders_all_owners_and_maps_exact_geometry();
    test_generation_is_never_reused_across_prestart_failures();
    test_invalid_or_unprotected_input_fails_before_open();
    test_generation_exhaustion_faults_without_wrap();
    test_every_prestart_failure_unwinds_without_start();
    test_cleanup_failure_faults_and_blocks_retry();
    test_start_failure_is_irreversible_and_never_prestart_aborts();

    if (failures != 0) {
        fprintf(stderr, "app_mpeg_run_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("app_mpeg_run_test: PASS");
    return 0;
}
