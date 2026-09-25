/*
 * R21-R24 focused host proof for the trigger-agnostic Application MPEG run
 * coordinator. Lower owners are represented only through their accepted public
 * seams so this fixture proves start/live service, ordered retirement/Q7
 * restoration overlap, explicit restored-RFB presentation proof and exact
 * synchronized final reveal without duplicating lower-owner mechanisms.
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
    EV_CONSUMER_SERVICE,
    EV_CONSUMER_STATUS,
    EV_PRESENTATION_BEGIN_RETIRE,
    EV_RETIRE,
    EV_RETIRE_TAKE,
    EV_PRESENTATION_SEAL,
    EV_COMPOSITOR_REVEAL,
    EV_PRODUCER_DONE,
    EV_WORKER_STATUS,
    EV_FINALIZE,
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
static pstvnc_rfb_flow_policy_t *observed_flow_policy;
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
static pstvnc_app_mpeg_frame_result_t consumer_service_result;
static pstvnc_app_mpeg_frame_service_result_t consumer_service_detail;
static int consumer_service_promotes_presentation;
static int consumer_service_faults_consumer;
static pstvnc_transport_result_t start_result;
static pstvnc_transport_result_t retire_result;
static pstvnc_transport_result_t retire_take_result;
static uint32_t retire_completion_generation;
static pstvnc_transport_result_t producer_done_result;
static pstvnc_transport_result_t finalize_result;
static int presentation_begin_retirement_result;
static int presentation_seal_result;
static int presentation_abort_result;
static pstvnc_mpeg_compositor_result_t compositor_reveal_result;
static pstvnc_mpeg_compositor_effects_t compositor_reveal_effects;
static int compositor_reveal_commits_presentation;
static pstvnc_mpeg_worker_result_t worker_status_result;
static pstvnc_mpeg_worker_status_t worker_status_value;
static pstvnc_mpeg_worker_result_t worker_stop_result;
static pstvnc_mpeg_worker_result_t worker_join_result;
static pstvnc_mpeg_worker_result_t worker_outcome_result;
static pstvnc_mpeg_worker_outcome_kind_t worker_outcome_kind;
static pstvnc_mpeg_decoder_result_t worker_outcome_decoder_result;
static pstvnc_mpeg_decoder_result_t worker_outcome_release_result;
static uint32_t worker_outcome_generation;
static pstvnc_mpeg_worker_result_t worker_release_result;
static int runtime_release_result;
static pstvnc_transport_result_t abort_result;

static uint32_t observed_worker_generation;
static pstvnc_transport_mpeg_start_request_t observed_start;
static pstvnc_mpeg_presentation_geometry_t observed_presentation_geometry;
static uint32_t observed_presentation_generation;
static uint32_t observed_reveal_generation;

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

static unsigned int event_occurrences(test_event_t event)
{
    size_t i;
    unsigned int count = 0u;

    for (i = 0u; i < event_count; i++) {
        if (events[i] == event)
            count++;
    }

    return count;
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
    selected_profile.scheduler.fps_numerator = 30000u;
    selected_profile.scheduler.fps_denominator = 1001u;
    selected_profile.scheduler.drop_enabled = 1;
    selected_profile.scheduler.drop_threshold_milliframes = 1500u;

    observed_flow_policy = NULL;
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
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_IDLE;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_IDLE;
    consumer_service_promotes_presentation = 0;
    consumer_service_faults_consumer = 0;
    start_result = PSTVNC_TRANSPORT_OK;
    retire_result = PSTVNC_TRANSPORT_OK;
    retire_take_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    retire_completion_generation = 1u;
    producer_done_result = PSTVNC_TRANSPORT_OK;
    finalize_result = PSTVNC_TRANSPORT_OK;
    presentation_begin_retirement_result = 1;
    presentation_seal_result = 1;
    presentation_abort_result = 1;
    compositor_reveal_result = PSTVNC_MPEG_COMPOSITOR_OK;
    memset(&compositor_reveal_effects, 0,
        sizeof(compositor_reveal_effects));
    compositor_reveal_effects.synchronized = 1u;
    compositor_reveal_effects.retirement_revealed = 1u;
    compositor_reveal_commits_presentation = 1;
    worker_status_result = PSTVNC_MPEG_WORKER_OK;
    memset(&worker_status_value, 0, sizeof(worker_status_value));
    worker_status_value.slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;
    worker_status_value.decoder_live = 1;
    worker_stop_result = PSTVNC_MPEG_WORKER_OK;
    worker_join_result = PSTVNC_MPEG_WORKER_OK;
    worker_outcome_result = PSTVNC_MPEG_WORKER_OK;
    worker_outcome_kind = PSTVNC_MPEG_WORKER_OUTCOME_STOPPED;
    worker_outcome_decoder_result = PSTVNC_MPEG_DECODER_STOPPED;
    worker_outcome_release_result = PSTVNC_MPEG_DECODER_COMPLETE;
    worker_outcome_generation = 0u;
    worker_release_result = PSTVNC_MPEG_WORKER_OK;
    runtime_release_result = 0;
    abort_result = PSTVNC_TRANSPORT_OK;

    observed_worker_generation = 0u;
    memset(&observed_start, 0, sizeof(observed_start));
    memset(&observed_presentation_geometry, 0,
        sizeof(observed_presentation_geometry));
    observed_presentation_generation = 0u;
    observed_reveal_generation = 0u;
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

pstvnc_transport_result_t pstvnc_transport_mpeg_send_retire(
    const pstvnc_transport_access_t *access,
    const pstvnc_transport_mpeg_retire_request_t *request)
{
    CHECK(access != NULL);
    CHECK(request != NULL);
    CHECK(request == NULL || request->generation != 0u);
    CHECK(event_index(EV_PRESENTATION_BEGIN_RETIRE) >= 0);
    CHECK(observed_flow_policy != NULL);
    CHECK(observed_flow_policy == NULL || observed_flow_policy->frozen);
    CHECK(observed_flow_policy == NULL ||
        !pstvnc_rfb_flow_policy_allows_remote_publication(
            observed_flow_policy));
    record_event(EV_RETIRE);
    return retire_result;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_take_retire_completion(
    const pstvnc_transport_access_t *access,
    pstvnc_transport_mpeg_retire_completion_t *completion)
{
    CHECK(access != NULL);
    CHECK(completion != NULL);
    record_event(EV_RETIRE_TAKE);

    if (retire_take_result == PSTVNC_TRANSPORT_OK && completion != NULL)
        completion->generation = retire_completion_generation;

    return retire_take_result;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_mark_producer_done(
    const pstvnc_transport_access_t *access)
{
    CHECK(access != NULL);
    record_event(EV_PRODUCER_DONE);
    return producer_done_result;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_run_finalize(
    const pstvnc_transport_access_t *access)
{
    CHECK(access != NULL);
    record_event(EV_FINALIZE);
    return finalize_result;
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

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_status(
    pstvnc_mpeg_worker_t *worker,
    uint32_t generation,
    pstvnc_mpeg_worker_status_t *status)
{
    CHECK(worker != NULL);
    CHECK(status != NULL);
    CHECK(worker == NULL || worker->run_generation == generation);
    record_event(EV_WORKER_STATUS);

    if (worker_status_result == PSTVNC_MPEG_WORKER_OK && status != NULL)
        *status = worker_status_value;

    return worker_status_result;
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
        outcome->kind = worker_outcome_kind;
        outcome->worker_result = PSTVNC_MPEG_WORKER_OK;
        outcome->decoder_result = worker_outcome_decoder_result;
        outcome->decoder_release_result = worker_outcome_release_result;
        outcome->run_generation =
            worker_outcome_generation != 0u
                ? worker_outcome_generation
                : generation;
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
    if (!presentation_snapshot_result ||
        presentation == NULL ||
        !presentation->snapshot_valid)
        return 0;
    *geometry = presentation->geometry;
    *generation = presentation->run_generation;
    return 1;
}

int pstvnc_mpeg_presentation_begin_retirement(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t generation)
{
    record_event(EV_PRESENTATION_BEGIN_RETIRE);
    if (!presentation_begin_retirement_result)
        return 0;
    CHECK(presentation != NULL);
    CHECK(presentation == NULL ||
        presentation->run_generation == generation);
    if (presentation != NULL)
        presentation->state = PSTVNC_MPEG_PRESENTATION_RETIRING;
    return 1;
}

int pstvnc_mpeg_presentation_seal_retirement(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t generation)
{
    record_event(EV_PRESENTATION_SEAL);
    CHECK(presentation != NULL);
    CHECK(presentation == NULL ||
        presentation->run_generation == generation);

    if (!presentation_seal_result ||
        presentation == NULL ||
        presentation->state != PSTVNC_MPEG_PRESENTATION_RETIRING ||
        presentation->run_generation != generation)
        return 0;

    presentation->state = PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING;
    return 1;
}

int pstvnc_mpeg_presentation_owns_mpeg_visual(
    const pstvnc_mpeg_presentation_t *presentation)
{
    if (presentation == NULL)
        return 0;

    return presentation->state == PSTVNC_MPEG_PRESENTATION_MPEG_OWNED ||
        presentation->state == PSTVNC_MPEG_PRESENTATION_RETIRING ||
        presentation->state == PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING;
}

int pstvnc_mpeg_compositor_reveal_retired(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t generation,
    pstvnc_mpeg_compositor_effects_t *effects)
{
    CHECK(presentation != NULL);
    CHECK(effects != NULL);
    record_event(EV_COMPOSITOR_REVEAL);
    observed_reveal_generation = generation;

    if (effects != NULL)
        *effects = compositor_reveal_effects;

    if (presentation == NULL || effects == NULL)
        return PSTVNC_MPEG_COMPOSITOR_INVALID;

    CHECK(presentation->run_generation == generation);
    CHECK(presentation->state == PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING);

    if (compositor_reveal_result == PSTVNC_MPEG_COMPOSITOR_OK &&
        compositor_reveal_commits_presentation) {
        presentation->state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
        presentation->snapshot_valid = 0u;
        presentation->run_generation = 0u;
        memset(&presentation->geometry, 0, sizeof(presentation->geometry));
    }

    return compositor_reveal_result;
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

pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_service(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t generation,
    uint64_t current_tick,
    pstvnc_app_mpeg_frame_service_result_t *result)
{
    (void)current_tick;

    CHECK(consumer != NULL);
    CHECK(result != NULL);
    CHECK(consumer->run_generation == generation);
    record_event(EV_CONSUMER_SERVICE);

    *result = consumer_service_detail;
    result->result = consumer_service_result;

    consumer->claim_outstanding = !!result->claim_outstanding;
    if (consumer->claim_outstanding)
        consumer->held_frame.picture.picture_ordinal =
            result->picture_ordinal;
    else
        memset(&consumer->held_frame, 0, sizeof(consumer->held_frame));

    if (consumer_service_result == PSTVNC_APP_MPEG_FRAME_PRESENTED ||
        consumer_service_result == PSTVNC_APP_MPEG_FRAME_DROPPED)
        consumer->last_consumed_ordinal = result->picture_ordinal;

    if (consumer_service_result == PSTVNC_APP_MPEG_FRAME_PRESENTED)
        consumer->presented_count += 1u;
    if (consumer_service_result == PSTVNC_APP_MPEG_FRAME_DROPPED)
        consumer->dropped_count += 1u;

    if (consumer_service_promotes_presentation) {
        consumer->presentation->state =
            PSTVNC_MPEG_PRESENTATION_MPEG_OWNED;
        consumer->presentation->run_generation = generation;
        consumer->presentation->snapshot_valid = 1u;
        consumer->scheduler_initialized = 1;
    }

    if (consumer_service_faults_consumer)
        consumer->faulted = 1;

    return consumer_service_result;
}

pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_status(
    const pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t generation,
    pstvnc_app_mpeg_frame_status_t *status)
{
    CHECK(consumer != NULL);
    CHECK(status != NULL);
    record_event(EV_CONSUMER_STATUS);

    if (consumer == NULL || status == NULL)
        return PSTVNC_APP_MPEG_FRAME_INVALID;
    if (!consumer->initialized)
        return PSTVNC_APP_MPEG_FRAME_INVALID;
    if (consumer->run_generation != generation)
        return PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION;

    memset(status, 0, sizeof(*status));
    status->run_generation = consumer->run_generation;
    status->last_consumed_ordinal = consumer->last_consumed_ordinal;
    status->presented_count = consumer->presented_count;
    status->dropped_count = consumer->dropped_count;
    status->scheduler_initialized = consumer->scheduler_initialized;
    status->claim_outstanding = consumer->claim_outstanding;
    status->faulted = consumer->faulted;
    if (consumer->claim_outstanding)
        status->held_ordinal =
            consumer->held_frame.picture.picture_ordinal;
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
    pstvnc_rfb_flow_policy_init(policy);
    CHECK(pstvnc_rfb_flow_policy_set_frozen(policy, 1));
    observed_flow_policy = policy;
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
    CHECK(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));
    CHECK(pstvnc_app_mpeg_run_start(
        &run, &geometry, &policy, &access, &presentation, &clock) ==
        PSTVNC_APP_MPEG_RUN_RFB_NOT_PROTECTED);
    CHECK(event_index(EV_OPEN) < 0);

    CHECK(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
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
        PSTVNC_APP_MPEG_RUN_ALREADY_FAULTED);
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


static void start_live_test_run(
    pstvnc_app_mpeg_run_t *run,
    pstvnc_rfb_flow_policy_t *policy,
    pstvnc_transport_access_t *access,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock)
{
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();

    init_inputs(run, policy, access, presentation, clock);
    CHECK(pstvnc_app_mpeg_run_start(
        run,
        &geometry,
        policy,
        access,
        presentation,
        clock) == PSTVNC_APP_MPEG_RUN_OK);
    reset_attempt_observation();
}

static void configure_first_presented(void)
{
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_PRESENTED;
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_PRESENTED;
    consumer_service_detail.picture_ordinal = 1u;
    consumer_service_detail.compositor_effects.synchronized = 1u;
    consumer_service_detail.compositor_effects.first_frame_promoted = 1u;
    consumer_service_promotes_presentation = 1;
}

static void promote_live_test_run(
    pstvnc_app_mpeg_run_t *run)
{
    pstvnc_app_mpeg_frame_service_result_t service_result;

    configure_first_presented();
    CHECK(pstvnc_app_mpeg_run_service(
        run,
        1000u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run->state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED);
    reset_attempt_observation();
}

static void test_service_rejects_without_successful_start(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    pstvnc_app_mpeg_run_init(&run);

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        0u,
        &service_result) == PSTVNC_APP_MPEG_RUN_NOT_LIVE);
    CHECK(event_index(EV_CONSUMER_SERVICE) < 0);

    run.state = PSTVNC_APP_MPEG_RUN_FAULTED;
    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        0u,
        &service_result) == PSTVNC_APP_MPEG_RUN_ALREADY_FAULTED);
    CHECK(event_index(EV_CONSUMER_SERVICE) < 0);

    run.state = PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME;
    run.current_generation = 1u;
    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        0u,
        &service_result) == PSTVNC_APP_MPEG_RUN_LIVE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(event_index(EV_CONSUMER_SERVICE) < 0);
}

static void test_pre_first_idle_preserves_exact_wait(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;
    pstvnc_app_mpeg_run_status_t status;

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        500u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_IDLE);
    CHECK(!service_result.worker_finished);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME);
    CHECK(run.current_generation == 1u);
    CHECK(run.session_teardown_required == 0);
    CHECK(presentation.state ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
    CHECK(presentation.run_generation == 1u);
    CHECK(event_index(EV_CONSUMER_SERVICE) >= 0);
    CHECK(event_index(EV_CONSUMER_STATUS) >= 0);
    CHECK(event_index(EV_PRESENTATION_ABORT) < 0);
    CHECK(event_index(EV_WORKER_STOP) < 0);
    CHECK(event_index(EV_ABORT) < 0);

    CHECK(pstvnc_app_mpeg_run_status(&run, &status) ==
        PSTVNC_APP_MPEG_RUN_OK);
    CHECK(status.state ==
        PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME);
    CHECK(status.current_generation == 1u);
}

static void test_first_presented_promotes_only_after_exact_p7_p3_proof(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;
    pstvnc_app_mpeg_run_status_t status;

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    configure_first_presented();

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        1000u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_PRESENTED);
    CHECK(service_result.picture_ordinal == 1u);
    CHECK(service_result.compositor_effects.synchronized);
    CHECK(service_result.compositor_effects.first_frame_promoted);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED);
    CHECK(run.current_generation == 1u);
    CHECK(run.session_teardown_required == 0);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_MPEG_OWNED);
    CHECK(presentation.run_generation == 1u);

    CHECK(pstvnc_app_mpeg_run_status(&run, &status) ==
        PSTVNC_APP_MPEG_RUN_OK);
    CHECK(status.state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED);
    CHECK(status.current_generation == 1u);
}

static void test_first_presented_requires_exact_promotion(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    configure_first_presented();
    consumer_service_detail.compositor_effects.first_frame_promoted = 0u;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        1000u,
        &service_result) ==
        PSTVNC_APP_MPEG_RUN_FRAME_SERVICE_CONTRADICTION);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_PRESENTED);
    CHECK(!service_result.compositor_effects.first_frame_promoted);
    CHECK(run.frame_consumer.initialized);
    CHECK(event_index(EV_WORKER_STOP) < 0);
    CHECK(event_index(EV_PRESENTATION_ABORT) < 0);

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    configure_first_presented();
    consumer_service_promotes_presentation = 0;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        1000u,
        &service_result) ==
        PSTVNC_APP_MPEG_RUN_FRAME_SERVICE_CONTRADICTION);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(presentation.state ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
}

static void test_post_first_live_outcomes_preserve_authority(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    promote_live_test_run(&run);

    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_WAIT;
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_WAIT;
    consumer_service_detail.picture_ordinal = 2u;
    consumer_service_detail.deadline_tick = 2222u;
    consumer_service_detail.claim_outstanding = 1;
    consumer_service_promotes_presentation = 0;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        1100u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED);
    CHECK(run.current_generation == 1u);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_WAIT);
    CHECK(service_result.picture_ordinal == 2u);
    CHECK(service_result.deadline_tick == 2222u);
    CHECK(service_result.claim_outstanding);
    CHECK(run.frame_consumer.claim_outstanding);
    CHECK(run.frame_consumer.held_frame.picture.picture_ordinal == 2u);

    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_PRESENTED;
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_PRESENTED;
    consumer_service_detail.picture_ordinal = 2u;
    consumer_service_detail.compositor_effects.synchronized = 1u;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        2222u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_PRESENTED);
    CHECK(!service_result.compositor_effects.first_frame_promoted);

    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_DROPPED;
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_DROPPED;
    consumer_service_detail.picture_ordinal = 3u;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        3333u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_DROPPED);

    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_IDLE;
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_IDLE;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        4444u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED);
    CHECK(run.current_generation == 1u);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_MPEG_OWNED);
}

static void test_unexpected_worker_finish_faults_before_and_after_promotion(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    consumer_service_detail.worker_finished = 1;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        500u,
        &service_result) ==
        PSTVNC_APP_MPEG_RUN_UNEXPECTED_WORKER_FINISH);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(service_result.worker_finished);
    CHECK(run.frame_consumer.initialized);
    CHECK(event_index(EV_WORKER_STOP) < 0);

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    promote_live_test_run(&run);
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_IDLE;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_IDLE;
    consumer_service_detail.worker_finished = 1;
    consumer_service_promotes_presentation = 0;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        1500u,
        &service_result) ==
        PSTVNC_APP_MPEG_RUN_UNEXPECTED_WORKER_FINISH);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.current_generation == 1u);
    CHECK(run.session_teardown_required == 1);
}

static void test_negative_p7_results_fault_without_erasing_evidence(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;
    int value;

    for (value = PSTVNC_APP_MPEG_FRAME_INVALID;
         value >= PSTVNC_APP_MPEG_FRAME_FAULTED;
         value--) {
        reset_fixture();
        start_live_test_run(
            &run, &policy, &access, &presentation, &clock);

        memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
        consumer_service_result =
            (pstvnc_app_mpeg_frame_result_t)value;
        consumer_service_detail.result =
            (pstvnc_app_mpeg_frame_result_t)value;
        consumer_service_detail.picture_ordinal = 77u;
        consumer_service_detail.claim_outstanding = 1;
        consumer_service_detail.worker_stop_result =
            PSTVNC_MPEG_WORKER_SYNC_FAILED;

        CHECK(pstvnc_app_mpeg_run_service(
            &run,
            700u,
            &service_result) ==
            PSTVNC_APP_MPEG_RUN_FRAME_SERVICE_FAILED);
        CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
        CHECK(run.session_teardown_required == 1);
        CHECK(run.current_generation == 1u);
        CHECK(run.frame_consumer.initialized);
        CHECK(run.frame_consumer.run_generation == 1u);
        CHECK(run.frame_consumer.claim_outstanding);
        CHECK(service_result.result ==
            (pstvnc_app_mpeg_frame_result_t)value);
        CHECK(service_result.picture_ordinal == 77u);
        CHECK(service_result.claim_outstanding);
        CHECK(service_result.worker_stop_result ==
            PSTVNC_MPEG_WORKER_SYNC_FAILED);
        CHECK(event_index(EV_PRESENTATION_ABORT) < 0);
        CHECK(event_index(EV_WORKER_STOP) < 0);
        CHECK(event_index(EV_ABORT) < 0);
    }
}

static void test_generation_and_state_contradictions_fail_closed(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    run.frame_consumer.run_generation = 2u;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        800u,
        &service_result) ==
        PSTVNC_APP_MPEG_RUN_LIVE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(event_index(EV_CONSUMER_SERVICE) < 0);

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    presentation.state = PSTVNC_MPEG_PRESENTATION_MPEG_OWNED;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        800u,
        &service_result) ==
        PSTVNC_APP_MPEG_RUN_LIVE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.current_generation == 1u);
    CHECK(event_index(EV_CONSUMER_SERVICE) < 0);

    reset_fixture();
    start_live_test_run(
        &run, &policy, &access, &presentation, &clock);
    consumer_service_faults_consumer = 1;

    CHECK(pstvnc_app_mpeg_run_service(
        &run,
        800u,
        &service_result) ==
        PSTVNC_APP_MPEG_RUN_LIVE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.frame_consumer.faulted);
    CHECK(run.frame_consumer.initialized);
}


static void configure_retirement_fixture(void)
{
    consumer_service_promotes_presentation = 0;
    consumer_service_faults_consumer = 0;
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_IDLE;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_IDLE;

    worker_status_result = PSTVNC_MPEG_WORKER_OK;
    memset(&worker_status_value, 0, sizeof(worker_status_value));
    worker_status_value.slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;
    worker_status_value.decoder_live = 1;

    retire_result = PSTVNC_TRANSPORT_OK;
    retire_take_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    retire_completion_generation = 1u;
    producer_done_result = PSTVNC_TRANSPORT_OK;
    finalize_result = PSTVNC_TRANSPORT_OK;

    worker_join_result = PSTVNC_MPEG_WORKER_OK;
    worker_outcome_result = PSTVNC_MPEG_WORKER_OK;
    worker_outcome_kind = PSTVNC_MPEG_WORKER_OUTCOME_COMPLETED;
    worker_outcome_decoder_result = PSTVNC_MPEG_DECODER_COMPLETE;
    worker_outcome_release_result = PSTVNC_MPEG_DECODER_COMPLETE;
    worker_outcome_generation = 0u;
    worker_release_result = PSTVNC_MPEG_WORKER_OK;
    runtime_release_result = 0;
    presentation_begin_retirement_result = 1;
}

static void start_owned_test_run(
    pstvnc_app_mpeg_run_t *run,
    pstvnc_rfb_flow_policy_t *policy,
    pstvnc_transport_access_t *access,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock)
{
    start_live_test_run(run, policy, access, presentation, clock);
    promote_live_test_run(run);
    configure_retirement_fixture();
    reset_attempt_observation();
}

static void check_q7_restoration_started(
    const pstvnc_rfb_flow_policy_t *policy)
{
    pstvnc_rfb_flow_request_t next_request;

    CHECK(policy != NULL);
    if (policy == NULL)
        return;

    CHECK(!policy->frozen);
    CHECK(pstvnc_rfb_flow_policy_allows_remote_publication(policy));

    next_request = pstvnc_rfb_flow_policy_next_request(policy);
    if (pstvnc_rfb_flow_policy_has_outstanding_request(policy))
        CHECK(next_request == PSTVNC_RFB_FLOW_REQUEST_HOLD);
    else
        CHECK(next_request == PSTVNC_RFB_FLOW_REQUEST_FULL);
}

static void begin_retirement_test_run(
    pstvnc_app_mpeg_run_t *run)
{
    CHECK(run != NULL);
    CHECK(run == NULL || run->rfb_flow_policy != NULL);
    CHECK(run == NULL || run->rfb_flow_policy == NULL ||
        run->rfb_flow_policy->frozen);

    CHECK(pstvnc_app_mpeg_run_begin_retirement(run) ==
        PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run->state == PSTVNC_APP_MPEG_RUN_RETIRING);
    CHECK(run->retire_invoked);
    CHECK(event_index(EV_PRESENTATION_BEGIN_RETIRE) >= 0);
    CHECK(event_index(EV_PRESENTATION_BEGIN_RETIRE) <
        event_index(EV_RETIRE));
    check_q7_restoration_started(run->rfb_flow_policy);
    reset_attempt_observation();
}


static void complete_retirement_test_run(
    pstvnc_app_mpeg_run_t *run)
{
    pstvnc_app_mpeg_frame_service_result_t service_result;

    CHECK(run != NULL);
    CHECK(run == NULL ||
        run->state == PSTVNC_APP_MPEG_RUN_RETIRING);

    retire_take_result = PSTVNC_TRANSPORT_OK;
    retire_completion_generation =
        run != NULL ? run->current_generation : 0u;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        run,
        1000u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run->retire_completion_taken);
    CHECK(run->producer_done_published);

    reset_attempt_observation();
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_IDLE;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_IDLE;
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    worker_status_value.slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        run,
        1200u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run->state == PSTVNC_APP_MPEG_RUN_RESTORE_PENDING);
    CHECK(!run->transport_run_open);
    CHECK(!run->worker_runtime_owned);
    CHECK(!run->worker_started);
    CHECK(!run->frame_consumer_initialized);
    CHECK(run->worker_joined);
    reset_attempt_observation();
}

static void prepare_restore_pending_test_run(
    pstvnc_app_mpeg_run_t *run,
    pstvnc_rfb_flow_policy_t *policy,
    pstvnc_transport_access_t *access,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock)
{
    start_owned_test_run(run, policy, access, presentation, clock);
    begin_retirement_test_run(run);
    complete_retirement_test_run(run);
}

static void complete_post_thaw_full_refresh(
    pstvnc_rfb_flow_policy_t *policy)
{
    CHECK(policy != NULL);
    CHECK(policy == NULL ||
        pstvnc_rfb_flow_policy_next_request(policy) ==
            PSTVNC_RFB_FLOW_REQUEST_FULL);
    CHECK(pstvnc_rfb_flow_policy_record_request_sent(
        policy,
        PSTVNC_RFB_FLOW_REQUEST_FULL));
    CHECK(pstvnc_rfb_flow_policy_has_outstanding_request(policy));
    CHECK(pstvnc_rfb_flow_policy_record_update_complete(policy));
    CHECK(!pstvnc_rfb_flow_policy_has_outstanding_request(policy));
    CHECK(pstvnc_rfb_flow_policy_next_request(policy) ==
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);
}

static void test_q7_thaw_creates_real_refresh_debt_and_allows_overlap(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);

    CHECK(policy.frozen);
    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);

    begin_retirement_test_run(&run);

    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);
    CHECK(!pstvnc_rfb_flow_policy_has_outstanding_request(&policy));

    CHECK(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_FULL));
    CHECK(pstvnc_rfb_flow_policy_has_outstanding_request(&policy));
    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);

    consumer_service_result = PSTVNC_APP_MPEG_FRAME_WAIT;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_WAIT;
    consumer_service_detail.picture_ordinal = 2u;
    consumer_service_detail.deadline_tick = 2222u;
    consumer_service_detail.claim_outstanding = 1;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run,
        1100u,
        &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_WAIT);
    CHECK(service_result.claim_outstanding);
    CHECK(service_result.deadline_tick == 2222u);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_RETIRING);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_RETIRING);
    CHECK(pstvnc_rfb_flow_policy_has_outstanding_request(&policy));
    CHECK(pstvnc_rfb_flow_policy_allows_remote_publication(&policy));
}

static void test_q7_thaw_respects_real_prior_outstanding_request(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();

    reset_fixture();
    init_inputs(&run, &policy, &access, &presentation, &clock);

    CHECK(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));
    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);
    CHECK(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_FULL));
    CHECK(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    CHECK(pstvnc_rfb_flow_policy_has_outstanding_request(&policy));

    CHECK(pstvnc_app_mpeg_run_start(
        &run,
        &geometry,
        &policy,
        &access,
        &presentation,
        &clock) == PSTVNC_APP_MPEG_RUN_OK);
    reset_attempt_observation();
    promote_live_test_run(&run);
    configure_retirement_fixture();
    reset_attempt_observation();

    begin_retirement_test_run(&run);
    CHECK(pstvnc_rfb_flow_policy_has_outstanding_request(&policy));
    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);
    CHECK(pstvnc_rfb_flow_policy_allows_remote_publication(&policy));
}

static void test_retirement_service_requires_q7_restoration_started(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);

    CHECK(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    CHECK(!pstvnc_rfb_flow_policy_allows_remote_publication(&policy));

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run,
        1000u,
        &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(event_index(EV_CONSUMER_SERVICE) < 0);
    CHECK(event_index(EV_RETIRE_TAKE) < 0);
}

static void test_retirement_admission_and_irreversible_retire(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;

    reset_fixture();
    pstvnc_app_mpeg_run_init(&run);
    CHECK(pstvnc_app_mpeg_run_begin_retirement(&run) ==
        PSTVNC_APP_MPEG_RUN_NOT_MPEG_OWNED);
    CHECK(event_index(EV_RETIRE) < 0);

    run.state = PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME;
    CHECK(pstvnc_app_mpeg_run_begin_retirement(&run) ==
        PSTVNC_APP_MPEG_RUN_NOT_MPEG_OWNED);
    CHECK(event_index(EV_RETIRE) < 0);

    run.state = PSTVNC_APP_MPEG_RUN_FAULTED;
    CHECK(pstvnc_app_mpeg_run_begin_retirement(&run) ==
        PSTVNC_APP_MPEG_RUN_ALREADY_FAULTED);
    CHECK(event_index(EV_RETIRE) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    run.worker_runtime_owned = 0;
    CHECK(pstvnc_app_mpeg_run_begin_retirement(&run) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(event_index(EV_RETIRE) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    retire_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_app_mpeg_run_begin_retirement(&run) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_SEND_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required == 1);
    CHECK(run.retire_invoked);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_RETIRING);
    CHECK(event_index(EV_PRESENTATION_BEGIN_RETIRE) <
        event_index(EV_RETIRE));
    CHECK(policy.frozen);
    CHECK(!pstvnc_rfb_flow_policy_allows_remote_publication(&policy));
    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);
    CHECK(event_index(EV_PRESENTATION_ABORT) < 0);
    CHECK(event_index(EV_WORKER_STOP) < 0);
    CHECK(event_index(EV_ABORT) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    CHECK(pstvnc_app_mpeg_run_begin_retirement(&run) ==
        PSTVNC_APP_MPEG_RUN_NOT_MPEG_OWNED);
    CHECK(event_index(EV_RETIRE) < 0);
}

static void test_retiring_drain_preserves_all_benign_p7_outcomes(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_IDLE);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_RETIRING);
    CHECK(!run.retire_completion_taken);
    CHECK(event_index(EV_RETIRE_TAKE) >= 0);
    CHECK(event_index(EV_PRODUCER_DONE) < 0);

    reset_attempt_observation();
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_WAIT;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_WAIT;
    consumer_service_detail.picture_ordinal = 2u;
    consumer_service_detail.deadline_tick = 2222u;
    consumer_service_detail.claim_outstanding = 1;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1100u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_WAIT);
    CHECK(service_result.claim_outstanding);
    CHECK(service_result.deadline_tick == 2222u);
    CHECK(run.frame_consumer.claim_outstanding);
    CHECK(event_index(EV_PRODUCER_DONE) < 0);

    reset_attempt_observation();
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_PRESENTED;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_PRESENTED;
    consumer_service_detail.picture_ordinal = 2u;
    consumer_service_detail.compositor_effects.synchronized = 1u;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 2222u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_PRESENTED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_RETIRING);

    reset_attempt_observation();
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_DROPPED;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_DROPPED;
    consumer_service_detail.picture_ordinal = 3u;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 3333u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(service_result.result == PSTVNC_APP_MPEG_FRAME_DROPPED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_RETIRING);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_RETIRING);
    check_q7_restoration_started(&policy);
}

static void test_retire_completion_fence_and_producer_done_order(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);

    retire_take_result = PSTVNC_TRANSPORT_OK;
    retire_completion_generation = 2u;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_COMPLETION_MISMATCH);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(!run.producer_done_published);
    CHECK(event_index(EV_PRODUCER_DONE) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_COMPLETION_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(event_index(EV_PRODUCER_DONE) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    retire_completion_generation = 1u;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.retire_completion_taken);
    CHECK(run.producer_done_published);
    CHECK(event_index(EV_RETIRE_TAKE) >= 0);
    CHECK(event_index(EV_RETIRE_TAKE) < event_index(EV_PRODUCER_DONE));
    CHECK(event_index(EV_WORKER_JOIN) < 0);

    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1100u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(event_index(EV_RETIRE_TAKE) < 0);
    CHECK(event_index(EV_PRODUCER_DONE) < 0);
}

static void test_worker_finish_fence_and_clean_reclaim_order(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;
    pstvnc_app_mpeg_run_status_t status;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    consumer_service_detail.worker_finished = 1;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_EARLY_FINISH);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(event_index(EV_WORKER_JOIN) < 0);
    CHECK(event_index(EV_WORKER_STOP) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);

    reset_attempt_observation();
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_IDLE;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_IDLE;
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    worker_status_value.slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1200u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_RESTORE_PENDING);
    CHECK(run.current_generation == 1u);
    CHECK(run.retire_invoked);
    CHECK(run.retire_completion_taken);
    CHECK(run.producer_done_published);
    CHECK(run.worker_joined);
    CHECK(!run.frame_consumer_initialized);
    CHECK(!run.worker_started);
    CHECK(!run.worker_runtime_owned);
    CHECK(!run.transport_run_open);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_RETIRING);
    CHECK(presentation.run_generation == 1u);
    CHECK(presentation.snapshot_valid);
    check_q7_restoration_started(&policy);

    CHECK(event_index(EV_WORKER_STATUS) >= 0);
    CHECK(event_index(EV_WORKER_STATUS) < event_index(EV_WORKER_JOIN));
    CHECK(event_index(EV_WORKER_JOIN) < event_index(EV_WORKER_OUTCOME));
    CHECK(event_index(EV_WORKER_OUTCOME) < event_index(EV_WORKER_RELEASE));
    CHECK(event_index(EV_WORKER_RELEASE) < event_index(EV_RUNTIME_RELEASE));
    CHECK(event_index(EV_RUNTIME_RELEASE) < event_index(EV_FINALIZE));
    CHECK(event_index(EV_WORKER_STOP) < 0);
    CHECK(event_index(EV_PRESENTATION_ABORT) < 0);
    CHECK(event_index(EV_ABORT) < 0);

    CHECK(pstvnc_app_mpeg_run_status(&run, &status) ==
        PSTVNC_APP_MPEG_RUN_OK);
    CHECK(status.state == PSTVNC_APP_MPEG_RUN_RESTORE_PENDING);
    CHECK(status.current_generation == 1u);
    CHECK(status.retire_invoked);
    CHECK(status.retire_completion_taken);
    CHECK(status.producer_done_published);
    CHECK(status.worker_joined);
}

static void test_retirement_reclaim_failures_preserve_truth(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    reset_attempt_observation();
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    worker_outcome_kind = PSTVNC_MPEG_WORKER_OUTCOME_STOPPED;
    worker_outcome_decoder_result = PSTVNC_MPEG_DECODER_STOPPED;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1200u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_OUTCOME_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.worker_joined);
    CHECK(run.worker_started);
    CHECK(run.worker_runtime_owned);
    CHECK(run.transport_run_open);
    CHECK(event_index(EV_WORKER_RELEASE) < 0);
    CHECK(event_index(EV_FINALIZE) < 0);
    check_q7_restoration_started(&policy);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    reset_attempt_observation();
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    runtime_release_result = -1;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1200u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_RUNTIME_RELEASE_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(!run.frame_consumer_initialized);
    CHECK(!run.worker_started);
    CHECK(run.worker_runtime_owned);
    CHECK(run.transport_run_open);
    CHECK(event_index(EV_FINALIZE) < 0);
    check_q7_restoration_started(&policy);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);
    reset_attempt_observation();
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    finalize_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1200u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_FINALIZE_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(!run.frame_consumer_initialized);
    CHECK(!run.worker_started);
    CHECK(!run.worker_runtime_owned);
    CHECK(run.transport_run_open);
    CHECK(event_index(EV_FINALIZE) >= 0);
    check_q7_restoration_started(&policy);
}

static void test_post_fence_borrow_and_join_failure_fail_closed(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);

    reset_attempt_observation();
    consumer_service_result = PSTVNC_APP_MPEG_FRAME_WAIT;
    memset(&consumer_service_detail, 0, sizeof(consumer_service_detail));
    consumer_service_detail.result = PSTVNC_APP_MPEG_FRAME_WAIT;
    consumer_service_detail.picture_ordinal = 2u;
    consumer_service_detail.deadline_tick = 2222u;
    consumer_service_detail.claim_outstanding = 1;
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    worker_status_value.slot_state = PSTVNC_MPEG_WORKER_SLOT_CLAIMED;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 2222u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.frame_consumer_initialized);
    CHECK(run.frame_consumer.claim_outstanding);
    CHECK(run.worker_started);
    CHECK(run.worker_runtime_owned);
    CHECK(run.transport_run_open);
    CHECK(event_index(EV_WORKER_JOIN) < 0);
    CHECK(event_index(EV_WORKER_RELEASE) < 0);
    CHECK(event_index(EV_RUNTIME_RELEASE) < 0);
    CHECK(event_index(EV_FINALIZE) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);

    reset_attempt_observation();
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    worker_status_value.slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;
    worker_join_result = PSTVNC_MPEG_WORKER_THREAD_JOIN_FAILED;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1200u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_JOIN_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(!run.worker_joined);
    CHECK(run.frame_consumer_initialized);
    CHECK(run.worker_started);
    CHECK(run.worker_runtime_owned);
    CHECK(run.transport_run_open);
    CHECK(event_index(EV_WORKER_JOIN) >= 0);
    CHECK(event_index(EV_WORKER_OUTCOME) < 0);
    CHECK(event_index(EV_WORKER_RELEASE) < 0);
    CHECK(event_index(EV_RUNTIME_RELEASE) < 0);
    CHECK(event_index(EV_FINALIZE) < 0);
    CHECK(event_index(EV_WORKER_STOP) < 0);
}

static void test_worker_release_and_producer_done_failures_preserve_truth(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    producer_done_result = PSTVNC_TRANSPORT_FAILED;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_PRODUCER_DONE_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.retire_completion_taken);
    CHECK(!run.producer_done_published);
    CHECK(run.frame_consumer_initialized);
    CHECK(run.worker_started);
    CHECK(run.worker_runtime_owned);
    CHECK(run.transport_run_open);
    CHECK(event_index(EV_RETIRE_TAKE) < event_index(EV_PRODUCER_DONE));
    CHECK(event_index(EV_WORKER_JOIN) < 0);
    CHECK(event_index(EV_FINALIZE) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);

    reset_attempt_observation();
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    worker_status_value.slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;
    worker_release_result = PSTVNC_MPEG_WORKER_THREAD_DESTROY_FAILED;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1200u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_RELEASE_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.worker_joined);
    CHECK(!run.frame_consumer_initialized);
    CHECK(run.worker_started);
    CHECK(run.worker_runtime_owned);
    CHECK(run.transport_run_open);
    CHECK(event_index(EV_WORKER_RELEASE) >= 0);
    CHECK(event_index(EV_RUNTIME_RELEASE) < 0);
    CHECK(event_index(EV_FINALIZE) < 0);
    CHECK(event_index(EV_WORKER_STOP) < 0);
}

static void test_failed_worker_outcome_is_not_clean_retirement(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_app_mpeg_frame_service_result_t service_result;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    begin_retirement_test_run(&run);
    retire_take_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1000u, &service_result) == PSTVNC_APP_MPEG_RUN_OK);

    reset_attempt_observation();
    consumer_service_detail.worker_finished = 1;
    worker_status_value.worker_finished = 1;
    worker_status_value.decoder_live = 0;
    worker_status_value.slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;
    worker_outcome_kind = PSTVNC_MPEG_WORKER_OUTCOME_FAILED;
    worker_outcome_decoder_result = PSTVNC_MPEG_DECODER_PICTURE_FAILED;

    CHECK(pstvnc_app_mpeg_run_retirement_service(
        &run, 1200u, &service_result) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_WORKER_OUTCOME_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.worker_joined);
    CHECK(run.worker_started);
    CHECK(run.worker_runtime_owned);
    CHECK(run.transport_run_open);
    CHECK(event_index(EV_WORKER_OUTCOME) >= 0);
    CHECK(event_index(EV_WORKER_RELEASE) < 0);
    CHECK(event_index(EV_FINALIZE) < 0);
}

static void test_retirement_requires_frozen_p2_and_exact_p3(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    CHECK(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));
    CHECK(pstvnc_app_mpeg_run_begin_retirement(&run) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(event_index(EV_RETIRE) < 0);

    reset_fixture();
    start_owned_test_run(
        &run, &policy, &access, &presentation, &clock);
    presentation.run_generation = 2u;
    CHECK(pstvnc_app_mpeg_run_begin_retirement(&run) ==
        PSTVNC_APP_MPEG_RUN_RETIRE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(event_index(EV_RETIRE) < 0);
}


static void test_r24_admission_and_protocol_freshness_gates(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_effects_t effects;

    reset_fixture();
    pstvnc_app_mpeg_run_init(&run);
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        1u) == PSTVNC_APP_MPEG_RUN_NOT_RESTORE_PENDING);
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) == PSTVNC_APP_MPEG_RUN_NOT_RESTORE_PENDING);
    CHECK(event_index(EV_PRESENTATION_SEAL) < 0);
    CHECK(event_index(EV_COMPOSITOR_REVEAL) < 0);

    run.state = PSTVNC_APP_MPEG_RUN_FAULTED;
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        1u) == PSTVNC_APP_MPEG_RUN_ALREADY_FAULTED);

    reset_fixture();
    pstvnc_app_mpeg_run_init(&run);
    run.state = PSTVNC_APP_MPEG_RUN_RESTORE_PENDING;
    run.current_generation = 1u;
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        1u) == PSTVNC_APP_MPEG_RUN_RESTORE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(event_index(EV_PRESENTATION_SEAL) < 0);

    reset_fixture();
    prepare_restore_pending_test_run(
        &run, &policy, &access, &presentation, &clock);

    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        run.current_generation) ==
        PSTVNC_APP_MPEG_RUN_RFB_RESTORE_NOT_FRESH);
    CHECK(!run.rfb_restoration_presented);
    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);

    CHECK(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_FULL));
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        run.current_generation) ==
        PSTVNC_APP_MPEG_RUN_RFB_RESTORE_NOT_FRESH);
    CHECK(!run.rfb_restoration_presented);
    CHECK(pstvnc_rfb_flow_policy_has_outstanding_request(&policy));

    CHECK(pstvnc_rfb_flow_policy_record_update_complete(&policy));
    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);

    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) ==
        PSTVNC_APP_MPEG_RUN_RFB_RESTORE_NOT_PRESENTED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_RESTORE_PENDING);
    CHECK(event_index(EV_PRESENTATION_SEAL) < 0);
    CHECK(event_index(EV_COMPOSITOR_REVEAL) < 0);

    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        run.current_generation + 1u) ==
        PSTVNC_APP_MPEG_RUN_RESTORE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(event_index(EV_PRESENTATION_SEAL) < 0);
}

static void test_r24_exact_marker_seal_and_successful_reveal(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_app_mpeg_run_status_t status;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_media_clock_t clock_before;
    pstvnc_mpeg_presentation_geometry_t snapshot;
    pstvnc_mpeg_compositor_effects_t effects;
    uint32_t snapshot_generation = 0u;
    uint32_t generation;

    reset_fixture();
    prepare_restore_pending_test_run(
        &run, &policy, &access, &presentation, &clock);
    generation = run.current_generation;

    clock.ticks_per_second = 12345u;
    clock.epoch_tick = 67890u;
    clock.armed = 1;
    clock_before = clock;

    complete_post_thaw_full_refresh(&policy);

    CHECK(!run.rfb_restoration_presented);
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        generation) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.rfb_restoration_presented);
    CHECK(pstvnc_app_mpeg_run_status(&run, &status) ==
        PSTVNC_APP_MPEG_RUN_OK);
    CHECK(status.rfb_restoration_presented);

    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) == PSTVNC_APP_MPEG_RUN_OK);

    CHECK(event_occurrences(EV_PRESENTATION_SEAL) == 1u);
    CHECK(event_occurrences(EV_COMPOSITOR_REVEAL) == 1u);
    CHECK(event_index(EV_PRESENTATION_SEAL) <
        event_index(EV_COMPOSITOR_REVEAL));
    CHECK(observed_reveal_generation == generation);
    CHECK(effects.synchronized);
    CHECK(effects.retirement_revealed);

    CHECK(run.state == PSTVNC_APP_MPEG_RUN_IDLE);
    CHECK(run.current_generation == 0u);
    CHECK(run.last_allocated_generation == generation);
    CHECK(!run.session_teardown_required);
    CHECK(!run.rfb_restoration_presented);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_RFB_ONLY);
    CHECK(!pstvnc_mpeg_presentation_owns_mpeg_visual(&presentation));
    CHECK(!pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &snapshot_generation));
    CHECK(!policy.frozen);
    CHECK(pstvnc_rfb_flow_policy_allows_remote_publication(&policy));
    CHECK(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);
    CHECK(memcmp(&clock, &clock_before, sizeof(clock)) == 0);
}

static void test_r24_retryable_reveal_does_not_reseal_or_teardown(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_app_mpeg_run_status_t status;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_presentation_geometry_t snapshot;
    pstvnc_mpeg_compositor_effects_t effects;
    uint32_t snapshot_generation = 0u;
    uint32_t generation;

    reset_fixture();
    prepare_restore_pending_test_run(
        &run, &policy, &access, &presentation, &clock);
    generation = run.current_generation;
    complete_post_thaw_full_refresh(&policy);
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        generation) == PSTVNC_APP_MPEG_RUN_OK);

    compositor_reveal_result = PSTVNC_MPEG_COMPOSITOR_PLATFORM_FAILED;
    memset(&compositor_reveal_effects, 0,
        sizeof(compositor_reveal_effects));

    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) ==
        PSTVNC_APP_MPEG_RUN_REVEAL_PLATFORM_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_REVEAL_PENDING);
    CHECK(run.current_generation == generation);
    CHECK(run.rfb_restoration_presented);
    CHECK(!run.session_teardown_required);
    CHECK(event_occurrences(EV_PRESENTATION_SEAL) == 1u);
    CHECK(event_occurrences(EV_COMPOSITOR_REVEAL) == 1u);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING);
    CHECK(pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &snapshot_generation));
    CHECK(snapshot_generation == generation);

    compositor_reveal_result = PSTVNC_MPEG_COMPOSITOR_SYNC_INVALID;
    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) ==
        PSTVNC_APP_MPEG_RUN_REVEAL_SYNC_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_REVEAL_PENDING);
    CHECK(!run.session_teardown_required);
    CHECK(event_occurrences(EV_PRESENTATION_SEAL) == 0u);
    CHECK(event_occurrences(EV_COMPOSITOR_REVEAL) == 1u);

    compositor_reveal_result = PSTVNC_MPEG_COMPOSITOR_OK;
    memset(&compositor_reveal_effects, 0,
        sizeof(compositor_reveal_effects));
    compositor_reveal_effects.synchronized = 1u;
    compositor_reveal_effects.retirement_revealed = 1u;
    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_IDLE);
    CHECK(event_occurrences(EV_PRESENTATION_SEAL) == 0u);
    CHECK(event_occurrences(EV_COMPOSITOR_REVEAL) == 1u);

    CHECK(pstvnc_app_mpeg_run_status(&run, &status) ==
        PSTVNC_APP_MPEG_RUN_OK);
    CHECK(status.state == PSTVNC_APP_MPEG_RUN_IDLE);
    CHECK(status.current_generation == 0u);
}

static void test_r24_freshness_gap_and_reveal_contradictions_fail_closed(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_effects_t effects;
    uint32_t generation;

    reset_fixture();
    prepare_restore_pending_test_run(
        &run, &policy, &access, &presentation, &clock);
    generation = run.current_generation;
    complete_post_thaw_full_refresh(&policy);
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        generation) == PSTVNC_APP_MPEG_RUN_OK);

    CHECK(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL));
    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) ==
        PSTVNC_APP_MPEG_RUN_RFB_RESTORE_NOT_FRESH);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_RESTORE_PENDING);
    CHECK(run.rfb_restoration_presented);
    CHECK(event_index(EV_PRESENTATION_SEAL) < 0);
    CHECK(event_index(EV_COMPOSITOR_REVEAL) < 0);

    CHECK(pstvnc_rfb_flow_policy_record_update_complete(&policy));
    presentation.run_generation = generation + 1u;
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) ==
        PSTVNC_APP_MPEG_RUN_RESTORE_STATE_INVALID);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required);
    CHECK(event_index(EV_PRESENTATION_SEAL) < 0);

    reset_fixture();
    prepare_restore_pending_test_run(
        &run, &policy, &access, &presentation, &clock);
    generation = run.current_generation;
    complete_post_thaw_full_refresh(&policy);
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        generation) == PSTVNC_APP_MPEG_RUN_OK);
    compositor_reveal_result = PSTVNC_MPEG_COMPOSITOR_INVALID;
    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) == PSTVNC_APP_MPEG_RUN_REVEAL_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required);
    CHECK(run.current_generation == generation);
    CHECK(run.rfb_restoration_presented);
    CHECK(event_occurrences(EV_PRESENTATION_SEAL) == 1u);
    CHECK(event_occurrences(EV_COMPOSITOR_REVEAL) == 1u);

    reset_fixture();
    prepare_restore_pending_test_run(
        &run, &policy, &access, &presentation, &clock);
    generation = run.current_generation;
    complete_post_thaw_full_refresh(&policy);
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        generation) == PSTVNC_APP_MPEG_RUN_OK);
    compositor_reveal_result =
        PSTVNC_MPEG_COMPOSITOR_RETIREMENT_COMMIT_FAILED;
    memset(&compositor_reveal_effects, 0,
        sizeof(compositor_reveal_effects));
    compositor_reveal_effects.synchronized = 1u;
    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) == PSTVNC_APP_MPEG_RUN_REVEAL_FAILED);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING);

    reset_fixture();
    prepare_restore_pending_test_run(
        &run, &policy, &access, &presentation, &clock);
    generation = run.current_generation;
    complete_post_thaw_full_refresh(&policy);
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        generation) == PSTVNC_APP_MPEG_RUN_OK);
    compositor_reveal_result = PSTVNC_MPEG_COMPOSITOR_OK;
    compositor_reveal_commits_presentation = 1;
    memset(&compositor_reveal_effects, 0,
        sizeof(compositor_reveal_effects));
    compositor_reveal_effects.synchronized = 1u;
    reset_attempt_observation();
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) ==
        PSTVNC_APP_MPEG_RUN_REVEAL_CONTRADICTION);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_FAULTED);
    CHECK(run.session_teardown_required);
    CHECK(run.current_generation == generation);
    CHECK(run.rfb_restoration_presented);
    CHECK(presentation.state == PSTVNC_MPEG_PRESENTATION_RFB_ONLY);
}

static void test_r24_success_allows_next_monotonic_generation_after_refreeze(void)
{
    pstvnc_app_mpeg_run_t run;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_transport_access_t access;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_effects_t effects;
    uint32_t first_generation;

    reset_fixture();
    prepare_restore_pending_test_run(
        &run, &policy, &access, &presentation, &clock);
    first_generation = run.current_generation;
    complete_post_thaw_full_refresh(&policy);
    CHECK(pstvnc_app_mpeg_run_record_restored_rfb_presented(
        &run,
        first_generation) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(pstvnc_app_mpeg_run_reveal_restored(
        &run,
        &effects) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.state == PSTVNC_APP_MPEG_RUN_IDLE);
    CHECK(run.current_generation == 0u);
    CHECK(run.last_allocated_generation == first_generation);

    CHECK(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    CHECK(policy.frozen);
    CHECK(pstvnc_app_mpeg_run_start(
        &run,
        &geometry,
        &policy,
        &access,
        &presentation,
        &clock) == PSTVNC_APP_MPEG_RUN_OK);
    CHECK(run.current_generation == first_generation + 1u);
    CHECK(run.last_allocated_generation == first_generation + 1u);
    CHECK(observed_worker_generation == first_generation + 1u);
    CHECK(observed_start.generation == first_generation + 1u);
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

    test_service_rejects_without_successful_start();
    test_pre_first_idle_preserves_exact_wait();
    test_first_presented_promotes_only_after_exact_p7_p3_proof();
    test_first_presented_requires_exact_promotion();
    test_post_first_live_outcomes_preserve_authority();
    test_unexpected_worker_finish_faults_before_and_after_promotion();
    test_negative_p7_results_fault_without_erasing_evidence();
    test_generation_and_state_contradictions_fail_closed();

    test_q7_thaw_creates_real_refresh_debt_and_allows_overlap();
    test_q7_thaw_respects_real_prior_outstanding_request();
    test_retirement_service_requires_q7_restoration_started();
    test_retirement_admission_and_irreversible_retire();
    test_retiring_drain_preserves_all_benign_p7_outcomes();
    test_retire_completion_fence_and_producer_done_order();
    test_worker_finish_fence_and_clean_reclaim_order();
    test_retirement_reclaim_failures_preserve_truth();
    test_post_fence_borrow_and_join_failure_fail_closed();
    test_worker_release_and_producer_done_failures_preserve_truth();
    test_failed_worker_outcome_is_not_clean_retirement();
    test_retirement_requires_frozen_p2_and_exact_p3();

    test_r24_admission_and_protocol_freshness_gates();
    test_r24_exact_marker_seal_and_successful_reveal();
    test_r24_retryable_reveal_does_not_reseal_or_teardown();
    test_r24_freshness_gap_and_reveal_contradictions_fail_closed();
    test_r24_success_allows_next_monotonic_generation_after_refreeze();

    if (failures != 0) {
        fprintf(stderr, "app_mpeg_run_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("app_mpeg_run_test: PASS");
    return 0;
}
