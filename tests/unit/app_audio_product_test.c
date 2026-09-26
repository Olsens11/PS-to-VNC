/*
 * File synopsis:
 * Deterministic R41 host proof for the Application AUDIO lifecycle coordinator.
 * Accepted lower-owner seams are injected as stubs so exact ordering, retained
 * ownership, nonblocking completion, finite retirement and reservoir gating can
 * be observed without PS2 hardware or ordinary app.c composition.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A006-AUDIO-APPLICATION-LIFECYCLE-COORDINATOR-R41.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_audio_product.h"

static int failures;

#define CHECK(expr) do {     if (!(expr)) {         fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);         failures++;     } } while (0)

enum test_event {
    EVENT_RUNTIME_INIT = 1,
    EVENT_RUNTIME_OPS,
    EVENT_TRANSPORT_ACTIVATE,
    EVENT_AUDSRV_OPS,
    EVENT_SESSION_START,
    EVENT_TRANSPORT_STATUS,
    EVENT_RETAINED_PROOF,
    EVENT_SESSION_STOP,
    EVENT_SESSION_POLL,
    EVENT_SESSION_JOIN,
    EVENT_SESSION_OUTCOME,
    EVENT_SESSION_RELEASE,
    EVENT_RUNTIME_RELEASE
};

static int events[128];
static size_t event_count;

static int selected_result;
static int profile_valid_result;
static int runtime_init_result;
static int runtime_init_partial_owned;
static int runtime_ops_result;
static int runtime_release_result;
static pstvnc_transport_result_t activate_result;
static pstvnc_transport_result_t status_result;
static size_t status_available;
static int status_producer_done;
static pstvnc_transport_result_t retained_result;
static pstvnc_audio_session_result_t session_start_result;
static int session_start_leave_never_started_owner;
static pstvnc_audio_session_result_t session_poll_result;
static pstvnc_audio_session_completion_state_t poll_state;
static pstvnc_audio_session_result_t session_join_result;
static pstvnc_audio_session_result_t session_outcome_result;
static pstvnc_audio_session_outcome_t configured_outcome;
static pstvnc_audio_session_result_t session_release_result;
static pstvnc_audio_session_result_t session_stop_result;

static unsigned int selected_calls;
static unsigned int profile_valid_calls;
static unsigned int runtime_init_calls;
static unsigned int runtime_ops_calls;
static unsigned int runtime_release_calls;
static unsigned int activate_calls;
static unsigned int status_calls;
static unsigned int retained_calls;
static unsigned int audsrv_ops_calls;
static unsigned int session_start_calls;
static unsigned int session_poll_calls;
static unsigned int session_join_calls;
static unsigned int session_outcome_calls;
static unsigned int session_release_calls;
static unsigned int session_stop_calls;
static size_t observed_startup_reservoir;
static int observed_clock_identity;

static pstvnc_media_clock_t test_clock;

static void push_event(int event)
{
    if (event_count < sizeof(events) / sizeof(events[0]))
        events[event_count++] = event;
}

static int event_index(int event)
{
    size_t index;

    for (index = 0u; index < event_count; index++) {
        if (events[index] == event)
            return (int)index;
    }

    return -1;
}

static int dummy_read_ticks(void *context, uint64_t *ticks)
{
    (void)context;
    if (ticks != NULL)
        *ticks = 1u;
    return 0;
}

static int dummy_delay_us(void *context, uint32_t delay_us)
{
    (void)context;
    (void)delay_us;
    return 0;
}

static int dummy_service_initialize(void *context)
{
    (void)context;
    return 0;
}

static int dummy_service_format(
    void *context,
    uint32_t rate_hz,
    uint32_t channels,
    uint32_t bits_per_sample)
{
    (void)context;
    (void)rate_hz;
    (void)channels;
    (void)bits_per_sample;
    return 0;
}

static int dummy_service_volume(void *context, uint32_t volume_percent)
{
    (void)context;
    (void)volume_percent;
    return 0;
}

static int dummy_service_wait(void *context, size_t byte_count)
{
    (void)context;
    (void)byte_count;
    return 0;
}

static int dummy_service_play(
    void *context,
    const uint8_t *bytes,
    size_t byte_count)
{
    (void)context;
    (void)bytes;
    (void)byte_count;
    return 0;
}

static int dummy_service_stop(void *context)
{
    (void)context;
    return 0;
}

static void reset_fixture(void)
{
    memset(events, 0, sizeof(events));
    event_count = 0u;

    selected_result = 1;
    profile_valid_result = 1;
    runtime_init_result = 0;
    runtime_init_partial_owned = 0;
    runtime_ops_result = 0;
    runtime_release_result = 0;
    activate_result = PSTVNC_TRANSPORT_OK;
    status_result = PSTVNC_TRANSPORT_OK;
    status_available = 0u;
    status_producer_done = 0;
    retained_result = PSTVNC_TRANSPORT_OK;
    session_start_result = PSTVNC_AUDIO_SESSION_OK;
    session_start_leave_never_started_owner = 0;
    session_poll_result = PSTVNC_AUDIO_SESSION_OK;
    poll_state = PSTVNC_AUDIO_SESSION_COMPLETION_PENDING;
    session_join_result = PSTVNC_AUDIO_SESSION_OK;
    session_outcome_result = PSTVNC_AUDIO_SESSION_OK;
    memset(&configured_outcome, 0, sizeof(configured_outcome));
    configured_outcome.kind = PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY;
    session_release_result = PSTVNC_AUDIO_SESSION_OK;
    session_stop_result = PSTVNC_AUDIO_SESSION_OK;

    selected_calls = 0u;
    profile_valid_calls = 0u;
    runtime_init_calls = 0u;
    runtime_ops_calls = 0u;
    runtime_release_calls = 0u;
    activate_calls = 0u;
    status_calls = 0u;
    retained_calls = 0u;
    audsrv_ops_calls = 0u;
    session_start_calls = 0u;
    session_poll_calls = 0u;
    session_join_calls = 0u;
    session_outcome_calls = 0u;
    session_release_calls = 0u;
    session_stop_calls = 0u;
    observed_startup_reservoir = 0u;
    observed_clock_identity = 0;

    memset(&test_clock, 0, sizeof(test_clock));
}

int pstvnc_config_audio_runtime_profile_selected(
    pstvnc_config_audio_runtime_profile_t *profile)
{
    selected_calls++;

    if (!selected_result || profile == NULL)
        return 0;

    memset(profile, 0, sizeof(*profile));
    profile->transport.queue_capacity = 524288u;
    profile->transport.initial_credit_bytes = 524288u;
    profile->transport.credit_batch_bytes = 65536u;
    profile->transport.credit_flush_on_empty = 1;
    profile->transport.credit_return_enabled = 1;
    profile->pcm.rate_hz = 48000u;
    profile->pcm.channels = 2u;
    profile->pcm.bits_per_sample = 16u;
    profile->pcm.volume_percent = 80u;
    profile->session.worker_stack_bytes = 32768u;
    profile->session.worker_priority = 64;
    profile->session.playback_buffer_capacity = 32768u;
    profile->session.startup_reservoir_bytes = 458752u;
    profile->session.reservoir_poll_us = 1000u;
    profile->session.clock_poll_us = 1000u;
    return 1;
}

int pstvnc_config_audio_runtime_profile_valid(
    const pstvnc_config_audio_runtime_profile_t *profile)
{
    profile_valid_calls++;
    return profile != NULL && profile_valid_result;
}

int pstvnc_audio_ps2_runtime_init(pstvnc_audio_ps2_runtime_t *runtime)
{
    runtime_init_calls++;
    push_event(EVENT_RUNTIME_INIT);

    memset(runtime, 0, sizeof(*runtime));
    runtime->session_lock_sema_id = -1;
    runtime->completion_sema_id = -1;
    runtime->thread_id = -1;

    if (runtime_init_result == 0) {
        runtime->resources_owned = 1;
        runtime->initialized = 1;
    } else if (runtime_init_partial_owned) {
        runtime->resources_owned = 1;
    }

    return runtime_init_result;
}

int pstvnc_audio_ps2_runtime_operations(
    pstvnc_audio_ps2_runtime_t *runtime,
    pstvnc_audio_session_memory_ops_t *memory_ops,
    pstvnc_audio_session_thread_ops_t *thread_ops,
    pstvnc_audio_session_sync_t *sync)
{
    runtime_ops_calls++;
    push_event(EVENT_RUNTIME_OPS);
    (void)runtime;

    if (runtime_ops_result != 0)
        return runtime_ops_result;

    memset(memory_ops, 0, sizeof(*memory_ops));
    memset(thread_ops, 0, sizeof(*thread_ops));
    memset(sync, 0, sizeof(*sync));
    return 0;
}

int pstvnc_audio_ps2_runtime_release(pstvnc_audio_ps2_runtime_t *runtime)
{
    runtime_release_calls++;
    push_event(EVENT_RUNTIME_RELEASE);

    if (runtime_release_result != 0)
        return runtime_release_result;

    runtime->resources_owned = 0;
    runtime->initialized = 0;
    return 0;
}

pstvnc_audio_service_ops_t pstvnc_audio_audsrv_service_ops(void)
{
    pstvnc_audio_service_ops_t service;

    audsrv_ops_calls++;
    push_event(EVENT_AUDSRV_OPS);
    memset(&service, 0, sizeof(service));
    service.initialize = dummy_service_initialize;
    service.set_format = dummy_service_format;
    service.set_volume = dummy_service_volume;
    service.wait_audio = dummy_service_wait;
    service.play_audio = dummy_service_play;
    service.stop_audio = dummy_service_stop;
    return service;
}

pstvnc_transport_result_t pstvnc_transport_audio_activate(
    const pstvnc_transport_access_t *transport_access)
{
    activate_calls++;
    push_event(EVENT_TRANSPORT_ACTIVATE);
    CHECK(transport_access != NULL);
    CHECK(transport_access->opaque_ticket == 77u);
    return activate_result;
}

pstvnc_transport_result_t pstvnc_transport_audio_status(
    const pstvnc_transport_access_t *transport_access,
    size_t *available_count,
    int *producer_done)
{
    status_calls++;
    push_event(EVENT_TRANSPORT_STATUS);
    CHECK(transport_access != NULL);
    CHECK(transport_access->opaque_ticket == 77u);

    if (available_count != NULL)
        *available_count = status_available;
    if (producer_done != NULL)
        *producer_done = status_producer_done;
    return status_result;
}

pstvnc_transport_result_t pstvnc_transport_session_abort_storage_retained(
    const pstvnc_transport_access_t *transport_access)
{
    retained_calls++;
    push_event(EVENT_RETAINED_PROOF);
    CHECK(transport_access != NULL);
    CHECK(transport_access->opaque_ticket == 77u);
    return retained_result;
}

pstvnc_audio_session_result_t pstvnc_audio_session_start(
    pstvnc_audio_session_t *session,
    const pstvnc_audio_session_values_t *values,
    const pstvnc_config_pcm_profile_t *pcm_profile,
    const pstvnc_audio_service_ops_t *service,
    const pstvnc_media_clock_t *clock,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_audio_session_memory_ops_t *memory_ops,
    const pstvnc_audio_session_thread_ops_t *thread_ops,
    const pstvnc_audio_session_sync_t *sync)
{
    (void)pcm_profile;
    (void)service;
    (void)time_ops;
    (void)memory_ops;
    (void)thread_ops;
    (void)sync;

    session_start_calls++;
    push_event(EVENT_SESSION_START);
    observed_startup_reservoir =
        values != NULL ? values->startup_reservoir_bytes : 0u;
    observed_clock_identity = clock == &test_clock;

    memset(session, 0, sizeof(*session));
    session->thread_id = -1;

    if (session_start_result == PSTVNC_AUDIO_SESSION_OK) {
        session->initialized = 1;
        session->thread_created = 1;
        session->thread_started = 1;
        session->thread_id = 9;
    } else if (session_start_leave_never_started_owner) {
        session->initialized = 1;
        session->thread_created = 1;
        session->thread_started = 0;
        session->thread_id = 9;
    }

    return session_start_result;
}

pstvnc_audio_session_result_t pstvnc_audio_session_poll(
    pstvnc_audio_session_t *session,
    pstvnc_audio_session_completion_state_t *state)
{
    session_poll_calls++;
    push_event(EVENT_SESSION_POLL);
    CHECK(session != NULL);

    if (session_poll_result == PSTVNC_AUDIO_SESSION_OK && state != NULL)
        *state = poll_state;
    return session_poll_result;
}

pstvnc_audio_session_result_t pstvnc_audio_session_join(
    pstvnc_audio_session_t *session)
{
    session_join_calls++;
    push_event(EVENT_SESSION_JOIN);

    if (session_join_result == PSTVNC_AUDIO_SESSION_OK) {
        session->thread_joined = 1;
        session->worker_finished = 1;
    }
    return session_join_result;
}

pstvnc_audio_session_result_t pstvnc_audio_session_outcome(
    const pstvnc_audio_session_t *session,
    pstvnc_audio_session_outcome_t *outcome)
{
    session_outcome_calls++;
    push_event(EVENT_SESSION_OUTCOME);
    (void)session;

    if (session_outcome_result == PSTVNC_AUDIO_SESSION_OK &&
        outcome != NULL)
        *outcome = configured_outcome;
    return session_outcome_result;
}

pstvnc_audio_session_result_t pstvnc_audio_session_release(
    pstvnc_audio_session_t *session)
{
    session_release_calls++;
    push_event(EVENT_SESSION_RELEASE);

    if (session_release_result != PSTVNC_AUDIO_SESSION_OK)
        return session_release_result;

    memset(session, 0, sizeof(*session));
    session->thread_id = -1;
    return PSTVNC_AUDIO_SESSION_OK;
}

pstvnc_audio_session_result_t pstvnc_audio_session_request_stop(
    pstvnc_audio_session_t *session)
{
    session_stop_calls++;
    push_event(EVENT_SESSION_STOP);

    if (session_stop_result == PSTVNC_AUDIO_SESSION_OK)
        session->stop_requested = 1;
    return session_stop_result;
}

static int init_product(pstvnc_app_audio_product_t *product)
{
    pstvnc_transport_access_t access;
    pstvnc_media_clock_time_ops_t time_ops;

    memset(&access, 0, sizeof(access));
    access.opaque_ticket = 77u;
    memset(&time_ops, 0, sizeof(time_ops));
    time_ops.read_ticks = dummy_read_ticks;
    time_ops.delay_us = dummy_delay_us;

    return pstvnc_app_audio_product_init(
        product,
        &access,
        &test_clock,
        &time_ops);
}

static void start_active(pstvnc_app_audio_product_t *product)
{
    CHECK(init_product(product));
    CHECK(pstvnc_app_audio_product_start(product) ==
        PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(product->state == PSTVNC_APP_AUDIO_PRODUCT_ACTIVE);
}

static void test_init_is_inert_and_copies_selected_profile(void)
{
    pstvnc_app_audio_product_t product;

    reset_fixture();
    CHECK(init_product(&product));
    CHECK(selected_calls == 1u);
    CHECK(profile_valid_calls == 1u);
    CHECK(product.profile.session.startup_reservoir_bytes == 458752u);
    CHECK(product.transport_access.opaque_ticket == 77u);
    CHECK(product.media_clock == &test_clock);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_DORMANT);
    CHECK(event_count == 0u);
    CHECK(runtime_init_calls == 0u);
    CHECK(activate_calls == 0u);
    CHECK(audsrv_ops_calls == 0u);
    CHECK(session_start_calls == 0u);
}

static void test_start_order_and_one_shot_activation(void)
{
    pstvnc_app_audio_product_t product;

    reset_fixture();
    start_active(&product);

    CHECK(event_index(EVENT_RUNTIME_INIT) >= 0);
    CHECK(event_index(EVENT_RUNTIME_INIT) < event_index(EVENT_RUNTIME_OPS));
    CHECK(event_index(EVENT_RUNTIME_OPS) <
        event_index(EVENT_TRANSPORT_ACTIVATE));
    CHECK(event_index(EVENT_TRANSPORT_ACTIVATE) <
        event_index(EVENT_SESSION_START));
    CHECK(observed_startup_reservoir == 458752u);
    CHECK(observed_clock_identity);
    CHECK(pstvnc_app_audio_product_transport_activated(&product));
    CHECK(activate_calls == 1u);

    CHECK(pstvnc_app_audio_product_start(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_INVALID);
    CHECK(activate_calls == 1u);
}

static void test_pre_activation_failure_rolls_back_or_retains_local_debt(void)
{
    pstvnc_app_audio_product_t product;
    pstvnc_app_audio_product_status_t status;
    int abort_ready = 0;

    reset_fixture();
    runtime_init_result = -1;
    runtime_init_partial_owned = 1;
    CHECK(init_product(&product));
    CHECK(pstvnc_app_audio_product_start(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_INIT_FAILED);
    CHECK(runtime_release_calls == 1u);
    CHECK(activate_calls == 0u);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_DORMANT);

    reset_fixture();
    runtime_ops_result = -1;
    CHECK(init_product(&product));
    CHECK(pstvnc_app_audio_product_start(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_OPS_FAILED);
    CHECK(runtime_release_calls == 1u);
    CHECK(activate_calls == 0u);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_DORMANT);

    reset_fixture();
    runtime_init_result = -1;
    runtime_init_partial_owned = 1;
    runtime_release_result = -1;
    CHECK(init_product(&product));
    CHECK(pstvnc_app_audio_product_start(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_LOCAL_CLEANUP_FAILED);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_FAULTED);
    CHECK(!product.transport_activation_attempted);
    CHECK(retained_calls == 0u);

    runtime_release_result = 0;
    CHECK(pstvnc_app_audio_product_service_session_abort(
        &product, &abort_ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(abort_ready == 1);
    CHECK(retained_calls == 0u);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_ABORT_READY);
    CHECK(pstvnc_app_audio_product_status(&product, &status) ==
        PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(status.first_failure ==
        PSTVNC_APP_AUDIO_PRODUCT_RUNTIME_INIT_FAILED);
}

static void test_post_activation_failure_requires_retained_ticket(void)
{
    pstvnc_app_audio_product_t product;
    int abort_ready = 0;
    unsigned int releases_before;

    reset_fixture();
    activate_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(init_product(&product));
    CHECK(pstvnc_app_audio_product_start(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_TRANSPORT_ACTIVATION_FAILED);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_FAULTED);
    CHECK(product.transport_activation_attempted);
    CHECK(product.requires_session_abort);
    CHECK(session_start_calls == 0u);

    CHECK(pstvnc_app_audio_product_start(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_INVALID);
    CHECK(activate_calls == 1u);

    releases_before = runtime_release_calls;
    retained_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    CHECK(pstvnc_app_audio_product_service_session_abort(
        &product, &abort_ready) ==
        PSTVNC_APP_AUDIO_PRODUCT_RETAINED_STORAGE_REQUIRED);
    CHECK(abort_ready == 0);
    CHECK(runtime_release_calls == releases_before);

    retained_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_app_audio_product_service_session_abort(
        &product, &abort_ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(abort_ready == 1);
    CHECK(runtime_release_calls == releases_before + 1u);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_ABORT_READY);
}

static void test_partial_never_started_owner_is_retryable_after_proof(void)
{
    pstvnc_app_audio_product_t product;
    int abort_ready = 0;

    reset_fixture();
    session_start_result = PSTVNC_AUDIO_SESSION_THREAD_START_FAILED;
    session_start_leave_never_started_owner = 1;
    CHECK(init_product(&product));
    CHECK(pstvnc_app_audio_product_start(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_SESSION_START_FAILED);
    CHECK(product.session_owned);
    CHECK(!product.session.thread_started);
    CHECK(product.requires_session_abort);

    session_release_result = PSTVNC_AUDIO_SESSION_THREAD_DESTROY_FAILED;
    CHECK(pstvnc_app_audio_product_service_session_abort(
        &product, &abort_ready) ==
        PSTVNC_APP_AUDIO_PRODUCT_SESSION_RELEASE_FAILED);
    CHECK(abort_ready == 0);
    CHECK(retained_calls == 1u);
    CHECK(session_stop_calls == 0u);
    CHECK(session_poll_calls == 0u);
    CHECK(session_join_calls == 0u);
    CHECK(runtime_release_calls == 0u);

    session_release_result = PSTVNC_AUDIO_SESSION_OK;
    CHECK(pstvnc_app_audio_product_service_session_abort(
        &product, &abort_ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(abort_ready == 1);
    CHECK(retained_calls == 1u);
    CHECK(session_release_calls == 2u);
    CHECK(runtime_release_calls == 1u);
}

static void test_active_pending_never_joins(void)
{
    pstvnc_app_audio_product_t product;

    reset_fixture();
    start_active(&product);
    poll_state = PSTVNC_AUDIO_SESSION_COMPLETION_PENDING;

    CHECK(pstvnc_app_audio_product_service(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(session_poll_calls == 1u);
    CHECK(session_join_calls == 0u);
    CHECK(session_outcome_calls == 0u);
    CHECK(session_release_calls == 0u);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_ACTIVE);

    session_poll_result = PSTVNC_AUDIO_SESSION_THREAD_STATUS_FAILED;
    CHECK(pstvnc_app_audio_product_service(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_SESSION_POLL_FAILED);
    CHECK(session_join_calls == 0u);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_FAULTED);
    CHECK(product.requires_session_abort);
}

static void test_reservoir_gate_is_nonconsuming_and_exact(void)
{
    pstvnc_app_audio_product_t product;
    int ready = -1;

    reset_fixture();
    start_active(&product);

    status_available = 458751u;
    status_producer_done = 0;
    CHECK(pstvnc_app_audio_product_first_presentation_ready(
        &product, &ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(ready == 0);

    status_available = 458752u;
    CHECK(pstvnc_app_audio_product_first_presentation_ready(
        &product, &ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(ready == 1);

    status_available = 0u;
    status_producer_done = 1;
    CHECK(pstvnc_app_audio_product_first_presentation_ready(
        &product, &ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(ready == 1);
    CHECK(status_calls == 3u);

    status_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_app_audio_product_first_presentation_ready(
        &product, &ready) ==
        PSTVNC_APP_AUDIO_PRODUCT_TRANSPORT_STATUS_FAILED);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_FAULTED);
    CHECK(product.requires_session_abort);
}

static void test_clean_finite_empty_and_playback_complete_retire_locally(void)
{
    pstvnc_app_audio_product_t product;
    int ready = 0;

    reset_fixture();
    start_active(&product);
    poll_state = PSTVNC_AUDIO_SESSION_COMPLETION_DONE;
    configured_outcome.kind = PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY;

    CHECK(pstvnc_app_audio_product_service(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(session_join_calls == 1u);
    CHECK(session_outcome_calls == 1u);
    CHECK(session_release_calls == 1u);
    CHECK(runtime_release_calls == 1u);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_FINITE_COMPLETE);
    CHECK(product.outcome_valid);
    CHECK(!product.session_owned);
    CHECK(!product.runtime_owned);
    CHECK(pstvnc_app_audio_product_transport_activated(&product));
    CHECK(pstvnc_app_audio_product_start(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_INVALID);
    CHECK(activate_calls == 1u);

    status_producer_done = 1;
    CHECK(pstvnc_app_audio_product_first_presentation_ready(
        &product, &ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(ready == 1);

    reset_fixture();
    start_active(&product);
    poll_state = PSTVNC_AUDIO_SESSION_COMPLETION_DONE;
    configured_outcome.kind = PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK;
    configured_outcome.playback_result = PSTVNC_AUDIO_PLAYBACK_COMPLETE;

    CHECK(pstvnc_app_audio_product_service(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_FINITE_COMPLETE);
    CHECK(product.outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK);
    CHECK(product.outcome.playback_result == PSTVNC_AUDIO_PLAYBACK_COMPLETE);
}

static void test_failed_outcome_is_preserved_until_retained_abort_cleanup(void)
{
    pstvnc_app_audio_product_t product;
    int abort_ready = 0;

    reset_fixture();
    start_active(&product);
    poll_state = PSTVNC_AUDIO_SESSION_COMPLETION_DONE;
    configured_outcome.kind = PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK;
    configured_outcome.playback_result =
        PSTVNC_AUDIO_PLAYBACK_SERVICE_WAIT_FAILED;

    CHECK(pstvnc_app_audio_product_service(&product) ==
        PSTVNC_APP_AUDIO_PRODUCT_SESSION_OUTCOME_FAILURE);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_FAULTED);
    CHECK(product.outcome_valid);
    CHECK(product.session_owned);
    CHECK(product.runtime_owned);
    CHECK(session_release_calls == 0u);
    CHECK(runtime_release_calls == 0u);

    event_count = 0u;
    CHECK(pstvnc_app_audio_product_service_session_abort(
        &product, &abort_ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(abort_ready == 1);
    CHECK(event_index(EVENT_RETAINED_PROOF) >= 0);
    CHECK(event_index(EVENT_RETAINED_PROOF) <
        event_index(EVENT_SESSION_RELEASE));
    CHECK(event_index(EVENT_SESSION_RELEASE) <
        event_index(EVENT_RUNTIME_RELEASE));
    CHECK(product.outcome.playback_result ==
        PSTVNC_AUDIO_PLAYBACK_SERVICE_WAIT_FAILED);
}

static void test_active_abort_is_nonblocking_and_ordered_after_retained_proof(void)
{
    pstvnc_app_audio_product_t product;
    int abort_ready = -1;

    reset_fixture();
    start_active(&product);
    event_count = 0u;
    configured_outcome.kind = PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED;
    poll_state = PSTVNC_AUDIO_SESSION_COMPLETION_PENDING;

    CHECK(pstvnc_app_audio_product_service_session_abort(
        &product, &abort_ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(abort_ready == 0);
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_FAULTED);
    CHECK(retained_calls == 1u);
    CHECK(session_stop_calls == 1u);
    CHECK(session_poll_calls == 1u);
    CHECK(session_join_calls == 0u);
    CHECK(event_index(EVENT_RETAINED_PROOF) <
        event_index(EVENT_SESSION_STOP));
    CHECK(event_index(EVENT_SESSION_STOP) <
        event_index(EVENT_SESSION_POLL));

    poll_state = PSTVNC_AUDIO_SESSION_COMPLETION_DONE;
    CHECK(pstvnc_app_audio_product_service_session_abort(
        &product, &abort_ready) == PSTVNC_APP_AUDIO_PRODUCT_OK);
    CHECK(abort_ready == 1);
    CHECK(retained_calls == 1u);
    CHECK(session_stop_calls == 1u);
    CHECK(session_poll_calls == 2u);
    CHECK(session_join_calls == 1u);
    CHECK(session_outcome_calls == 1u);
    CHECK(session_release_calls == 1u);
    CHECK(runtime_release_calls == 1u);
    CHECK(event_index(EVENT_SESSION_JOIN) <
        event_index(EVENT_SESSION_OUTCOME));
    CHECK(event_index(EVENT_SESSION_OUTCOME) <
        event_index(EVENT_SESSION_RELEASE));
    CHECK(event_index(EVENT_SESSION_RELEASE) <
        event_index(EVENT_RUNTIME_RELEASE));
    CHECK(product.state == PSTVNC_APP_AUDIO_PRODUCT_ABORT_READY);
    CHECK(product.outcome_valid);
    CHECK(product.outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED);
}

int main(void)
{
    test_init_is_inert_and_copies_selected_profile();
    test_start_order_and_one_shot_activation();
    test_pre_activation_failure_rolls_back_or_retains_local_debt();
    test_post_activation_failure_requires_retained_ticket();
    test_partial_never_started_owner_is_retryable_after_proof();
    test_active_pending_never_joins();
    test_reservoir_gate_is_nonconsuming_and_exact();
    test_clean_finite_empty_and_playback_complete_retire_locally();
    test_failed_outcome_is_preserved_until_retained_abort_cleanup();
    test_active_abort_is_nonblocking_and_ordered_after_retained_proof();

    if (failures != 0) {
        fprintf(stderr, "app_audio_product_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("APP_AUDIO_PRODUCT_TEST=PASS");
    return 0;
}
