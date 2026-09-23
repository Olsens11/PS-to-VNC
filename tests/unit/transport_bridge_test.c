/*
 * File synopsis:
 * Host-tests Transport's cross-component bridge ownership/result mapping with
 * deterministic private-runtime stubs. The fixture preserves established
 * RFB/AUDIO/session lifecycle proofs and covers A003 opt-in logical MPEG
 * opening, bounded consumer/status/activity, producer-completion publication,
 * R20 owner-correct generation-control stamping/completion projection, and
 * stale-access fencing without exposing physical descriptor/session authority
 * or decoder policy.
 *
 * Context: LEDGE_FOREMAN_STATE revisions 0009 and 0012.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "transport/bridge.h"
#include "transport/runtime.h"

static int failures;
static int initialize_result = 1;
static int initialize_audio_result = 1;
static int initialize_mpeg_result = 1;
static int initialize_audio_mpeg_result = 1;
static int start_result = 1;
static int request_stop_result = 1;
static int wait_done_result = 1;
static int release_result = 1;
static int establish_result = 1;
static uint32_t establish_session_id = 0x10203040u;
static pstvnc_wire_not_accepted_reason_t establish_rejection_reason =
    (pstvnc_wire_not_accepted_reason_t)0;
static int establish_calls;
static int physical_shutdown_calls;
static int physical_release_calls;
static int read_result = 1;
static int poll_result = 1;
static int write_result = 1;
static pstvnc_transport_result_t rfb_provider_failure_result =
    PSTVNC_TRANSPORT_WOULD_BLOCK;
static pstvnc_rfb_provider_failure_reason_t rfb_provider_failure_reason =
    PSTVNC_RFB_PROVIDER_FAILURE_NONE;
static int rfb_provider_failure_calls;
static int rfb_read_calls;
static int rfb_write_calls;
static int audio_status_calls;
static int mpeg_status_calls;
static pstvnc_transport_result_t audio_read_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t audio_status_result = PSTVNC_TRANSPORT_OK;
static int audio_snapshot_result = 1;
static int audio_wait_result = 1;
static pstvnc_transport_result_t mpeg_read_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t mpeg_status_result = PSTVNC_TRANSPORT_OK;
static int mpeg_snapshot_result = 1;
static int mpeg_wait_result = 1;
static int mpeg_done_result = 1;
static pstvnc_transport_result_t mpeg_run_open_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t mpeg_run_abort_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t mpeg_run_finalize_result = PSTVNC_TRANSPORT_OK;
static int mpeg_run_open_calls;
static int mpeg_run_abort_calls;
static int mpeg_run_finalize_calls;
static pstvnc_transport_result_t mpeg_start_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t mpeg_retire_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t mpeg_take_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
static int mpeg_start_calls;
static int mpeg_retire_calls;
static int mpeg_take_calls;
static pstvnc_mpeg_start_payload_t observed_start;
static pstvnc_mpeg_retire_payload_t observed_retire;
static pstvnc_mpeg_retire_payload_t available_completion;
static int quiesce_requested_result = 1;
static int quiesce_boundary_result = 1;
static int quiesce_commit_result = 1;
static int residual_snapshot_result = 1;
static int residual_discard_result = 1;
static int quiesce_complete_result = 1;
static int initialize_calls;
static int initialize_audio_calls;
static int initialize_mpeg_calls;
static int initialize_audio_mpeg_calls;
static int start_calls;
static int request_stop_calls;
static int wait_done_calls;
static int release_calls;
static int adopted_socket_fd;
static pstvnc_transport_audio_channel_config_t observed_audio_config;
static pstvnc_transport_mpeg_channel_config_t observed_mpeg_config;
static pstvnc_transport_runtime_t *observed_runtime;
static pstvnc_transport_access_t current_access;

typedef enum lifecycle_event {
    EVENT_STOP = 1,
    EVENT_WAIT = 2,
    EVENT_RELEASE = 3
} lifecycle_event_t;

static lifecycle_event_t lifecycle_events[8];
static size_t lifecycle_event_count;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static void record_lifecycle(lifecycle_event_t event)
{
    CHECK(lifecycle_event_count <
        sizeof(lifecycle_events) / sizeof(lifecycle_events[0]));
    if (lifecycle_event_count <
        sizeof(lifecycle_events) / sizeof(lifecycle_events[0]))
        lifecycle_events[lifecycle_event_count++] = event;
}

static void reset_fixture(void)
{
    initialize_result = 1;
    initialize_audio_result = 1;
    initialize_mpeg_result = 1;
    initialize_audio_mpeg_result = 1;
    start_result = 1;
    request_stop_result = 1;
    wait_done_result = 1;
    release_result = 1;
    establish_result = 1;
    establish_session_id = 0x10203040u;
    establish_rejection_reason = (pstvnc_wire_not_accepted_reason_t)0;
    establish_calls = 0;
    physical_shutdown_calls = 0;
    physical_release_calls = 0;
    read_result = 1;
    poll_result = 1;
    write_result = 1;
    rfb_provider_failure_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    rfb_provider_failure_reason = PSTVNC_RFB_PROVIDER_FAILURE_NONE;
    rfb_provider_failure_calls = 0;
    rfb_read_calls = 0;
    rfb_write_calls = 0;
    audio_status_calls = 0;
    mpeg_status_calls = 0;
    audio_read_result = PSTVNC_TRANSPORT_OK;
    audio_status_result = PSTVNC_TRANSPORT_OK;
    audio_snapshot_result = 1;
    audio_wait_result = 1;
    mpeg_read_result = PSTVNC_TRANSPORT_OK;
    mpeg_status_result = PSTVNC_TRANSPORT_OK;
    mpeg_snapshot_result = 1;
    mpeg_wait_result = 1;
    mpeg_done_result = 1;
    mpeg_run_open_result = PSTVNC_TRANSPORT_OK;
    mpeg_run_abort_result = PSTVNC_TRANSPORT_OK;
    mpeg_run_finalize_result = PSTVNC_TRANSPORT_OK;
    mpeg_run_open_calls = 0;
    mpeg_run_abort_calls = 0;
    mpeg_run_finalize_calls = 0;
    mpeg_start_result = PSTVNC_TRANSPORT_OK;
    mpeg_retire_result = PSTVNC_TRANSPORT_OK;
    mpeg_take_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    mpeg_start_calls = 0;
    mpeg_retire_calls = 0;
    mpeg_take_calls = 0;
    memset(&observed_start, 0, sizeof(observed_start));
    memset(&observed_retire, 0, sizeof(observed_retire));
    memset(&available_completion, 0, sizeof(available_completion));
    quiesce_requested_result = 1;
    quiesce_boundary_result = 1;
    quiesce_commit_result = 1;
    residual_snapshot_result = 1;
    residual_discard_result = 1;
    quiesce_complete_result = 1;
    initialize_calls = 0;
    initialize_audio_calls = 0;
    initialize_mpeg_calls = 0;
    initialize_audio_mpeg_calls = 0;
    start_calls = 0;
    request_stop_calls = 0;
    wait_done_calls = 0;
    release_calls = 0;
    adopted_socket_fd = -1;
    memset(&observed_audio_config, 0, sizeof(observed_audio_config));
    memset(&observed_mpeg_config, 0, sizeof(observed_mpeg_config));
    observed_runtime = NULL;
    memset(&current_access, 0, sizeof(current_access));
    memset(lifecycle_events, 0, sizeof(lifecycle_events));
    lifecycle_event_count = 0u;
}

static pstvnc_transport_session_config_t make_config(void)
{
    pstvnc_transport_session_config_t config;

    memset(&config, 0, sizeof(config));
    config.rfb_queue_capacity = 4096u;
    config.rfb_initial_credit_bytes = 4096u;
    config.rfb_credit_batch_bytes = 1024u;
    config.rfb_credit_flush_on_empty = 1;
    config.rfb_credit_return_enabled = 1;
    config.receiver_thread_stack_size = 4096u;
    config.receiver_thread_priority = 64;
    config.max_data_payload = 1024u;
    return config;
}

static pstvnc_transport_audio_channel_config_t make_audio_config(void)
{
    pstvnc_transport_audio_channel_config_t config;

    memset(&config, 0, sizeof(config));
    config.queue_capacity = 8192u;
    config.initial_credit_bytes = 4096u;
    config.credit_batch_bytes = 1024u;
    config.credit_flush_on_empty = 1;
    config.credit_return_enabled = 1;
    return config;
}

static pstvnc_transport_mpeg_channel_config_t make_mpeg_config(void)
{
    pstvnc_transport_mpeg_channel_config_t config;

    memset(&config, 0, sizeof(config));
    config.queue_capacity = 12288u;
    config.initial_credit_bytes = 6144u;
    config.credit_batch_bytes = 2048u;
    config.credit_flush_on_empty = 1;
    config.credit_return_enabled = 1;
    return config;
}

static void initialize_stub_runtime(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    int audio_enabled,
    int mpeg_enabled)
{
    memset(runtime, 0, sizeof(*runtime));
    runtime->initialized = 1;
    runtime->audio_enabled = audio_enabled;
    runtime->mpeg_enabled = mpeg_enabled;
    runtime->physical_stream.socket_fd = socket_fd;
    runtime->physical_stream.send_semaphore_id = 1;
    runtime->receiver_thread_id = -1;
    observed_runtime = runtime;
}

int pstvnc_transport_runtime_initialize(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config)
{
    initialize_calls++;
    adopted_socket_fd = socket_fd;
    if (!initialize_result || runtime == NULL || config == NULL)
        return 0;
    initialize_stub_runtime(runtime, socket_fd, 0, 0);
    return 1;
}

int pstvnc_transport_runtime_initialize_with_audio(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config)
{
    initialize_audio_calls++;
    adopted_socket_fd = socket_fd;
    if (!initialize_audio_result || runtime == NULL || config == NULL ||
        audio_config == NULL)
        return 0;
    observed_audio_config = *audio_config;
    initialize_stub_runtime(runtime, socket_fd, 1, 0);
    return 1;
}

int pstvnc_transport_runtime_initialize_with_mpeg(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    initialize_mpeg_calls++;
    adopted_socket_fd = socket_fd;
    if (!initialize_mpeg_result || runtime == NULL || config == NULL ||
        mpeg_config == NULL)
        return 0;
    observed_mpeg_config = *mpeg_config;
    initialize_stub_runtime(runtime, socket_fd, 0, 1);
    return 1;
}

int pstvnc_transport_runtime_initialize_with_audio_mpeg(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    initialize_audio_mpeg_calls++;
    adopted_socket_fd = socket_fd;
    if (!initialize_audio_mpeg_result || runtime == NULL || config == NULL ||
        audio_config == NULL || mpeg_config == NULL)
        return 0;
    observed_audio_config = *audio_config;
    observed_mpeg_config = *mpeg_config;
    initialize_stub_runtime(runtime, socket_fd, 1, 1);
    return 1;
}

static int initialize_stub_established_runtime(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    int socket_fd;

    if (runtime == NULL || physical_stream == NULL || config == NULL ||
        physical_stream->socket_fd < 0 ||
        physical_stream->next_send_sequence != 2u ||
        physical_stream->expected_receive_sequence != 2u)
        return 0;

    socket_fd = physical_stream->socket_fd;
    adopted_socket_fd = socket_fd;
    initialize_stub_runtime(
        runtime,
        socket_fd,
        audio_config != NULL,
        mpeg_config != NULL);
    runtime->physical_stream.next_send_sequence = 2u;
    runtime->physical_stream.expected_receive_sequence = 2u;

    physical_stream->socket_fd = -1;
    physical_stream->send_semaphore_id = -1;
    physical_stream->next_send_sequence = 1u;
    physical_stream->expected_receive_sequence = 1u;
    return 1;
}

int pstvnc_transport_runtime_initialize_established(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config)
{
    initialize_calls++;
    if (!initialize_result)
        return 0;
    return initialize_stub_established_runtime(
        runtime, physical_stream, config, NULL, NULL);
}

int pstvnc_transport_runtime_initialize_established_with_audio(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config)
{
    initialize_audio_calls++;
    if (!initialize_audio_result || audio_config == NULL)
        return 0;
    observed_audio_config = *audio_config;
    return initialize_stub_established_runtime(
        runtime, physical_stream, config, audio_config, NULL);
}

int pstvnc_transport_runtime_initialize_established_with_mpeg(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    initialize_mpeg_calls++;
    if (!initialize_mpeg_result || mpeg_config == NULL)
        return 0;
    observed_mpeg_config = *mpeg_config;
    return initialize_stub_established_runtime(
        runtime, physical_stream, config, NULL, mpeg_config);
}

int pstvnc_transport_runtime_initialize_established_with_audio_mpeg(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    initialize_audio_mpeg_calls++;
    if (!initialize_audio_mpeg_result ||
        audio_config == NULL || mpeg_config == NULL)
        return 0;
    observed_audio_config = *audio_config;
    observed_mpeg_config = *mpeg_config;
    return initialize_stub_established_runtime(
        runtime, physical_stream, config, audio_config, mpeg_config);
}

int pstvnc_transport_physical_stream_establish_client(
    pstvnc_transport_physical_stream_t *stream,
    int *socket_fd,
    uint32_t *session_id,
    pstvnc_wire_not_accepted_reason_t *rejection_reason)
{
    establish_calls++;

    if (stream == NULL || socket_fd == NULL || *socket_fd < 0 ||
        session_id == NULL || rejection_reason == NULL)
        return 0;

    if (establish_result == 0)
        return 0;

    stream->socket_fd = *socket_fd;
    *socket_fd = -1;
    stream->send_semaphore_id = 3;
    stream->next_send_sequence = 2u;
    stream->expected_receive_sequence = 2u;

    if (establish_result < 0) {
        *session_id = 0u;
        *rejection_reason = establish_rejection_reason;
        stream->socket_fd = -1;
        stream->send_semaphore_id = -1;
        stream->next_send_sequence = 1u;
        stream->expected_receive_sequence = 1u;
        return -1;
    }

    *session_id = establish_session_id;
    *rejection_reason = (pstvnc_wire_not_accepted_reason_t)0;
    return 1;
}

int pstvnc_transport_physical_stream_shutdown_io(
    pstvnc_transport_physical_stream_t *stream)
{
    physical_shutdown_calls++;
    return stream != NULL && stream->socket_fd >= 0;
}

void pstvnc_transport_physical_stream_release(
    pstvnc_transport_physical_stream_t *stream)
{
    physical_release_calls++;
    if (stream == NULL)
        return;
    stream->socket_fd = -1;
    stream->send_semaphore_id = -1;
    stream->next_send_sequence = 1u;
    stream->expected_receive_sequence = 1u;
}

int pstvnc_transport_runtime_start_receiver(
    pstvnc_transport_runtime_t *runtime)
{
    start_calls++;
    if (!start_result)
        return 0;
    runtime->receiver_thread_started = 1;
    observed_runtime = runtime;
    return 1;
}

int pstvnc_transport_runtime_request_stop(
    pstvnc_transport_runtime_t *runtime)
{
    request_stop_calls++;
    record_lifecycle(EVENT_STOP);
    if (!request_stop_result)
        return 0;
    runtime->stop_requested = 1;
    return 1;
}

int pstvnc_transport_runtime_wait_receiver_done(
    pstvnc_transport_runtime_t *runtime)
{
    wait_done_calls++;
    record_lifecycle(EVENT_WAIT);
    if (!wait_done_result)
        return 0;
    runtime->receiver_done = 1;
    return 1;
}

int pstvnc_transport_runtime_release(
    pstvnc_transport_runtime_t *runtime)
{
    release_calls++;
    record_lifecycle(EVENT_RELEASE);
    memset(runtime, 0, sizeof(*runtime));
    runtime->physical_stream.socket_fd = -1;
    runtime->physical_stream.send_semaphore_id = -1;
    runtime->receiver_thread_id = -1;
    return release_result;
}

int pstvnc_transport_runtime_rfb_read_exact(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t count)
{
    (void)runtime;
    (void)buffer;
    (void)count;
    rfb_read_calls += 1;
    return read_result;
}

int pstvnc_transport_runtime_rfb_poll_receive(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return poll_result;
}

int pstvnc_transport_runtime_rfb_write_exact(
    pstvnc_transport_runtime_t *runtime,
    const void *buffer,
    size_t count)
{
    (void)runtime;
    (void)buffer;
    (void)count;
    rfb_write_calls += 1;
    return write_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_rfb_provider_failure(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_rfb_provider_failure_reason_t *reason)
{
    (void)runtime;
    rfb_provider_failure_calls += 1;
    if (reason == NULL)
        return PSTVNC_TRANSPORT_INVALID;
    *reason = rfb_provider_failure_reason;
    return rfb_provider_failure_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_audio_read_available(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    (void)runtime;
    (void)buffer;
    (void)maximum_count;
    if (read_count != NULL)
        *read_count = audio_read_result == PSTVNC_TRANSPORT_OK ? 3u : 0u;
    return audio_read_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_audio_status(
    pstvnc_transport_runtime_t *runtime,
    size_t *available_count,
    int *producer_done)
{
    (void)runtime;
    audio_status_calls += 1;
    if (available_count != NULL)
        *available_count = 5u;
    if (producer_done != NULL)
        *producer_done = 1;
    return audio_status_result;
}

int pstvnc_transport_runtime_audio_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    (void)runtime;
    if (activity_sequence != NULL)
        *activity_sequence = 7u;
    return audio_snapshot_result;
}

int pstvnc_transport_runtime_audio_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    (void)runtime;
    if (activity_sequence != NULL)
        *activity_sequence += 1u;
    return audio_wait_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_read_available(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    (void)runtime;
    (void)buffer;
    (void)maximum_count;
    if (read_count != NULL)
        *read_count = mpeg_read_result == PSTVNC_TRANSPORT_OK ? 4u : 0u;
    return mpeg_read_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_status(
    pstvnc_transport_runtime_t *runtime,
    size_t *available_count,
    int *producer_done)
{
    (void)runtime;
    mpeg_status_calls += 1;
    if (available_count != NULL)
        *available_count = 6u;
    if (producer_done != NULL)
        *producer_done = 0;
    return mpeg_status_result;
}

int pstvnc_transport_runtime_mpeg_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    (void)runtime;
    if (activity_sequence != NULL)
        *activity_sequence = 11u;
    return mpeg_snapshot_result;
}

int pstvnc_transport_runtime_mpeg_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    (void)runtime;
    if (activity_sequence != NULL)
        *activity_sequence += 2u;
    return mpeg_wait_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_run_open(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    mpeg_run_open_calls++;
    return mpeg_run_open_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_run_abort_pre_start(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    mpeg_run_abort_calls++;
    return mpeg_run_abort_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_run_finalize(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    mpeg_run_finalize_calls++;
    return mpeg_run_finalize_result;
}

int pstvnc_transport_runtime_mpeg_mark_producer_done(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return mpeg_done_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_send_start(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_mpeg_start_payload_t *start)
{
    (void)runtime;
    mpeg_start_calls++;
    if (start != NULL)
        observed_start = *start;
    return mpeg_start_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_send_retire(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_mpeg_retire_payload_t *retire)
{
    (void)runtime;
    mpeg_retire_calls++;
    if (retire != NULL)
        observed_retire = *retire;
    return mpeg_retire_result;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_take_retire_completion(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_mpeg_retire_payload_t *completion)
{
    (void)runtime;
    mpeg_take_calls++;
    if (mpeg_take_result == PSTVNC_TRANSPORT_OK && completion != NULL)
        *completion = available_completion;
    return mpeg_take_result;
}

int pstvnc_transport_runtime_rfb_quiesce_requested(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return quiesce_requested_result;
}

int pstvnc_transport_runtime_rfb_send_quiesce_boundary(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return quiesce_boundary_result;
}

int pstvnc_transport_runtime_rfb_wait_quiesce_commit(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return quiesce_commit_result;
}

int pstvnc_transport_runtime_rfb_snapshot_residual(
    pstvnc_transport_runtime_t *runtime,
    size_t *residual_count)
{
    (void)runtime;
    if (residual_count != NULL)
        *residual_count = 7u;
    return residual_snapshot_result;
}

int pstvnc_transport_runtime_rfb_discard_quiesce_residual(
    pstvnc_transport_runtime_t *runtime,
    size_t expected_count,
    size_t *discarded_count)
{
    (void)runtime;
    if (discarded_count != NULL)
        *discarded_count = expected_count;
    return residual_discard_result;
}

int pstvnc_transport_runtime_rfb_send_quiesce_complete(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return quiesce_complete_result;
}

static void complete_and_close(void)
{
    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
}

static void test_rfb_only_open_and_close_regression(void)
{
    pstvnc_transport_session_config_t config = make_config();
    int socket_fd = 42;

    reset_fixture();
    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(socket_fd == -1);
    CHECK(adopted_socket_fd == 42);
    CHECK(initialize_calls == 1);
    CHECK(initialize_audio_calls == 0);
    CHECK(initialize_mpeg_calls == 0);
    CHECK(start_calls == 1);
    CHECK(observed_runtime != NULL && observed_runtime->audio_enabled == 0 &&
        observed_runtime->mpeg_enabled == 0);
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_WOULD_BLOCK);
    complete_and_close();
}

static void test_audio_open_requires_explicit_config(void)
{
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_audio_channel_config_t audio = make_audio_config();
    int socket_fd = 43;

    reset_fixture();
    CHECK(pstvnc_transport_session_open_with_audio(
        &socket_fd, &config, NULL) == PSTVNC_TRANSPORT_INVALID);
    CHECK(socket_fd == 43);
    CHECK(initialize_audio_calls == 0);

    CHECK(pstvnc_transport_session_open_with_audio(
        &socket_fd, &config, &audio) == PSTVNC_TRANSPORT_OK);
    CHECK(socket_fd == -1);
    CHECK(initialize_calls == 0);
    CHECK(initialize_audio_calls == 1);
    CHECK(observed_runtime != NULL && observed_runtime->audio_enabled == 1 &&
        observed_runtime->mpeg_enabled == 0);
    CHECK(memcmp(&observed_audio_config, &audio, sizeof(audio)) == 0);
    complete_and_close();
}

static void test_mpeg_open_and_combined_authority(void)
{
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_audio_channel_config_t audio = make_audio_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    int socket_fd = 44;

    reset_fixture();
    CHECK(pstvnc_transport_session_open_with_mpeg(
        &socket_fd, &config, NULL) == PSTVNC_TRANSPORT_INVALID);
    CHECK(socket_fd == 44);
    CHECK(initialize_mpeg_calls == 0);

    CHECK(pstvnc_transport_session_open_with_mpeg(
        &socket_fd, &config, &mpeg) == PSTVNC_TRANSPORT_OK);
    CHECK(socket_fd == -1);
    CHECK(initialize_mpeg_calls == 1);
    CHECK(observed_runtime != NULL && observed_runtime->mpeg_enabled == 1 &&
        observed_runtime->audio_enabled == 0);
    CHECK(memcmp(&observed_mpeg_config, &mpeg, sizeof(mpeg)) == 0);
    complete_and_close();

    reset_fixture();
    socket_fd = 45;
    CHECK(pstvnc_transport_session_open_with_audio_mpeg(
        &socket_fd, &config, &audio, &mpeg) == PSTVNC_TRANSPORT_OK);
    CHECK(socket_fd == -1);
    CHECK(initialize_audio_mpeg_calls == 1);
    CHECK(observed_runtime != NULL && observed_runtime->audio_enabled == 1 &&
        observed_runtime->mpeg_enabled == 1);
    CHECK(memcmp(&observed_audio_config, &audio, sizeof(audio)) == 0);
    CHECK(memcmp(&observed_mpeg_config, &mpeg, sizeof(mpeg)) == 0);
    complete_and_close();
}

static void test_failed_open_ownership_regression(void)
{
    pstvnc_transport_session_config_t config = make_config();
    int socket_fd = 17;

    reset_fixture();
    initialize_result = 0;
    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_FAILED);
    CHECK(socket_fd == -1);
    CHECK(start_calls == 0);
    CHECK(release_calls == 0);
    CHECK(physical_release_calls == 1);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);

    reset_fixture();
    start_result = 0;
    socket_fd = 23;
    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_FAILED);
    CHECK(socket_fd == -1);
    CHECK(release_calls == 1);
}

static void test_fatal_abort_order_regression(void)
{
    pstvnc_transport_session_config_t config = make_config();
    int socket_fd = 29;

    reset_fixture();
    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_session_abort() == PSTVNC_TRANSPORT_OK);
    CHECK(lifecycle_event_count == 3u);
    CHECK(lifecycle_events[0] == EVENT_STOP);
    CHECK(lifecycle_events[1] == EVENT_WAIT);
    CHECK(lifecycle_events[2] == EVENT_RELEASE);
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_INVALID);
}

static void test_rfb_result_mapping_regression(void)
{
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_rfb_provider_failure_reason_t reason =
        PSTVNC_RFB_PROVIDER_FAILURE_NONE;
    unsigned char byte = 0;
    int socket_fd = 31;

    reset_fixture();
    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_ACTIVE);
    CHECK(pstvnc_transport_access_acquire(&current_access) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_rfb_read_exact(&current_access, &byte, 1u) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_rfb_write_exact(&current_access, &byte, 1u) == PSTVNC_TRANSPORT_OK);

    CHECK(pstvnc_transport_rfb_provider_failure(
        &current_access, &reason) == PSTVNC_TRANSPORT_WOULD_BLOCK);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_NONE);
    CHECK(rfb_provider_failure_calls == 1);

    rfb_provider_failure_reason = PSTVNC_RFB_PROVIDER_FAILURE_READ;
    rfb_provider_failure_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_transport_rfb_provider_failure(
        &current_access, &reason) == PSTVNC_TRANSPORT_OK);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_READ);
    CHECK(rfb_provider_failure_calls == 2);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_ACTIVE);

    poll_result = 0;
    CHECK(pstvnc_transport_rfb_poll_receive(&current_access) == PSTVNC_TRANSPORT_WOULD_BLOCK);
    poll_result = 1;
    CHECK(pstvnc_transport_rfb_poll_receive(&current_access) == PSTVNC_TRANSPORT_OK);

    read_result = 0;
    observed_runtime->failed = 1;
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);
    CHECK(pstvnc_transport_rfb_provider_failure(
        &current_access, &reason) == PSTVNC_TRANSPORT_OK);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_READ);
    CHECK(pstvnc_transport_rfb_read_exact(&current_access, &byte, 1u) ==
        PSTVNC_TRANSPORT_FAILED);

    /*
     * The fixture clears the synthetic failure only so it can separately prove
     * receiver completion. Either independently known terminal fact makes Wire
     * unavailable before final release.
     */
    observed_runtime->failed = 0;
    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);
    CHECK(pstvnc_transport_rfb_read_exact(&current_access, &byte, 1u) ==
        PSTVNC_TRANSPORT_CLOSED);
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);
}

static void test_audio_result_mapping(void)
{
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_audio_channel_config_t audio = make_audio_config();
    unsigned char bytes[8];
    size_t count = 0u;
    size_t available = 0u;
    int producer_done = 0;
    uint32_t sequence = 0u;
    int socket_fd = 33;

    reset_fixture();
    CHECK(pstvnc_transport_session_open_with_audio(
        &socket_fd, &config, &audio) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&current_access) ==
        PSTVNC_TRANSPORT_OK);

    CHECK(pstvnc_transport_audio_read_available(&current_access,
        bytes, sizeof(bytes), &count) == PSTVNC_TRANSPORT_OK);
    CHECK(count == 3u);
    CHECK(pstvnc_transport_audio_status(&current_access,
        &available, &producer_done) == PSTVNC_TRANSPORT_OK);
    CHECK(available == 5u && producer_done == 1);
    CHECK(pstvnc_transport_audio_activity_snapshot(&current_access, &sequence) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(sequence == 7u);
    CHECK(pstvnc_transport_audio_wait_activity(&current_access, &sequence) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(sequence == 8u);

    audio_read_result = PSTVNC_TRANSPORT_EXHAUSTED;
    CHECK(pstvnc_transport_audio_read_available(&current_access,
        bytes, sizeof(bytes), &count) == PSTVNC_TRANSPORT_EXHAUSTED);
    audio_read_result = PSTVNC_TRANSPORT_STOPPED;
    CHECK(pstvnc_transport_audio_read_available(&current_access,
        bytes, sizeof(bytes), &count) == PSTVNC_TRANSPORT_STOPPED);
    audio_read_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_transport_audio_read_available(&current_access,
        bytes, sizeof(bytes), &count) == PSTVNC_TRANSPORT_FAILED);

    complete_and_close();
}

static void test_mpeg_result_mapping(void)
{
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    unsigned char bytes[8];
    size_t count = 0u;
    size_t available = 0u;
    int producer_done = 1;
    uint32_t sequence = 0u;
    int socket_fd = 34;

    reset_fixture();
    CHECK(pstvnc_transport_session_open_with_mpeg(
        &socket_fd, &config, &mpeg) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&current_access) ==
        PSTVNC_TRANSPORT_OK);

    CHECK(pstvnc_transport_mpeg_run_open(&current_access) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(mpeg_run_open_calls == 1);
    CHECK(pstvnc_transport_mpeg_run_abort_pre_start(&current_access) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(mpeg_run_abort_calls == 1);
    CHECK(pstvnc_transport_mpeg_run_open(&current_access) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(mpeg_run_open_calls == 2);
    CHECK(pstvnc_transport_mpeg_run_finalize(&current_access) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(mpeg_run_finalize_calls == 1);

    mpeg_run_open_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    CHECK(pstvnc_transport_mpeg_run_open(&current_access) ==
        PSTVNC_TRANSPORT_WOULD_BLOCK);
    CHECK(mpeg_run_open_calls == 3);
    mpeg_run_open_result = PSTVNC_TRANSPORT_OK;

    CHECK(pstvnc_transport_mpeg_read_available(&current_access,
        bytes, sizeof(bytes), &count) == PSTVNC_TRANSPORT_OK);
    CHECK(count == 4u);
    CHECK(pstvnc_transport_mpeg_status(&current_access,
        &available, &producer_done) == PSTVNC_TRANSPORT_OK);
    CHECK(available == 6u && producer_done == 0);
    CHECK(pstvnc_transport_mpeg_activity_snapshot(&current_access, &sequence) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(sequence == 11u);
    CHECK(pstvnc_transport_mpeg_wait_activity(&current_access, &sequence) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(sequence == 13u);
    CHECK(pstvnc_transport_mpeg_mark_producer_done(&current_access) == PSTVNC_TRANSPORT_OK);

    mpeg_read_result = PSTVNC_TRANSPORT_EXHAUSTED;
    CHECK(pstvnc_transport_mpeg_read_available(&current_access,
        bytes, sizeof(bytes), &count) == PSTVNC_TRANSPORT_EXHAUSTED);
    mpeg_read_result = PSTVNC_TRANSPORT_STOPPED;
    CHECK(pstvnc_transport_mpeg_read_available(&current_access,
        bytes, sizeof(bytes), &count) == PSTVNC_TRANSPORT_STOPPED);
    mpeg_done_result = 0;
    observed_runtime->failed = 1;
    CHECK(pstvnc_transport_mpeg_mark_producer_done(&current_access) ==
        PSTVNC_TRANSPORT_FAILED);

    observed_runtime->failed = 0;
    complete_and_close();
}



static void fill_bridge_start_request(
    pstvnc_transport_mpeg_start_request_t *request)
{
    memset(request, 0, sizeof(*request));
    request->generation = 7u;
    request->base_x = 16u;
    request->base_y = 32u;
    request->base_width = 640u;
    request->base_height = 448u;
    request->suppression_x = 8u;
    request->suppression_y = 16u;
    request->suppression_width = 656u;
    request->suppression_height = 480u;
}

static void check_observed_start(
    uint32_t expected_session_id,
    const pstvnc_transport_mpeg_start_request_t *request)
{
    CHECK(observed_start.version == PSTVNC_MPEG_GENERATION_CONTROL_VERSION);
    CHECK(observed_start.session_id == expected_session_id);
    CHECK(observed_start.generation == request->generation);
    CHECK(observed_start.base_x == request->base_x);
    CHECK(observed_start.base_y == request->base_y);
    CHECK(observed_start.base_width == request->base_width);
    CHECK(observed_start.base_height == request->base_height);
    CHECK(observed_start.suppression_x == request->suppression_x);
    CHECK(observed_start.suppression_y == request->suppression_y);
    CHECK(observed_start.suppression_width == request->suppression_width);
    CHECK(observed_start.suppression_height == request->suppression_height);
}

static void test_mpeg_control_private_identity_and_stale_access_fence(void)
{
    const uint32_t session_a = 0x10203040u;
    const uint32_t session_b = 0x55667788u;
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    pstvnc_transport_access_t access_a;
    pstvnc_transport_access_t access_b;
    pstvnc_transport_mpeg_start_request_t start;
    pstvnc_transport_mpeg_retire_request_t retire;
    pstvnc_transport_mpeg_retire_completion_t completion;
    int socket_fd = 80;
    int starts_before;
    int retires_before;
    int takes_before;

    reset_fixture();
    memset(&access_a, 0, sizeof(access_a));
    memset(&access_b, 0, sizeof(access_b));
    memset(&retire, 0, sizeof(retire));
    memset(&completion, 0, sizeof(completion));
    fill_bridge_start_request(&start);
    retire.generation = start.generation;

    /*
     * These are the complete public RETIRE/completion representations. If Q4
     * identity or wire version leaks upward, these size assertions change.
     */
    CHECK(sizeof(retire) == sizeof(uint32_t));
    CHECK(sizeof(completion) == sizeof(uint32_t));
    CHECK(sizeof(start) == 9u * sizeof(uint32_t));

    establish_session_id = session_a;
    CHECK(pstvnc_transport_session_open_with_mpeg(
        &socket_fd, &config, &mpeg) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&access_a) == PSTVNC_TRANSPORT_OK);

    CHECK(pstvnc_transport_mpeg_send_start(
        &access_a, &start) == PSTVNC_TRANSPORT_OK);
    CHECK(mpeg_start_calls == 1);
    check_observed_start(session_a, &start);

    CHECK(pstvnc_transport_mpeg_send_retire(
        &access_a, &retire) == PSTVNC_TRANSPORT_OK);
    CHECK(mpeg_retire_calls == 1);
    CHECK(observed_retire.version == PSTVNC_MPEG_GENERATION_CONTROL_VERSION);
    CHECK(observed_retire.session_id == session_a);
    CHECK(observed_retire.generation == retire.generation);

    CHECK(pstvnc_transport_mpeg_take_retire_completion(
        &access_a, &completion) == PSTVNC_TRANSPORT_WOULD_BLOCK);
    CHECK(mpeg_take_calls == 1);

    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);

    establish_session_id = session_b;
    socket_fd = 81;
    CHECK(pstvnc_transport_session_open_with_mpeg(
        &socket_fd, &config, &mpeg) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&access_b) == PSTVNC_TRANSPORT_OK);
    CHECK(access_a.opaque_ticket != access_b.opaque_ticket);

    starts_before = mpeg_start_calls;
    retires_before = mpeg_retire_calls;
    takes_before = mpeg_take_calls;

    CHECK(pstvnc_transport_mpeg_send_start(
        &access_a, &start) == PSTVNC_TRANSPORT_CLOSED);
    CHECK(pstvnc_transport_mpeg_send_retire(
        &access_a, &retire) == PSTVNC_TRANSPORT_CLOSED);
    CHECK(pstvnc_transport_mpeg_take_retire_completion(
        &access_a, &completion) == PSTVNC_TRANSPORT_CLOSED);
    CHECK(mpeg_start_calls == starts_before);
    CHECK(mpeg_retire_calls == retires_before);
    CHECK(mpeg_take_calls == takes_before);

    /*
     * Reusing caller-owned run meaning with the current B ticket can only stamp
     * B's current private identity; there is no field in the request capable of
     * carrying A's retired Q4 session ID.
     */
    CHECK(pstvnc_transport_mpeg_send_start(
        &access_b, &start) == PSTVNC_TRANSPORT_OK);
    CHECK(mpeg_start_calls == starts_before + 1);
    check_observed_start(session_b, &start);

    CHECK(pstvnc_transport_mpeg_send_retire(
        &access_b, &retire) == PSTVNC_TRANSPORT_OK);
    CHECK(mpeg_retire_calls == retires_before + 1);
    CHECK(observed_retire.version == PSTVNC_MPEG_GENERATION_CONTROL_VERSION);
    CHECK(observed_retire.session_id == session_b);
    CHECK(observed_retire.generation == retire.generation);

    available_completion.version = PSTVNC_MPEG_GENERATION_CONTROL_VERSION;
    available_completion.session_id = session_a;
    available_completion.generation = retire.generation;
    mpeg_take_result = PSTVNC_TRANSPORT_OK;
    completion.generation = 0u;
    CHECK(pstvnc_transport_mpeg_take_retire_completion(
        &access_b, &completion) == PSTVNC_TRANSPORT_FAILED);
    CHECK(completion.generation == 0u);

    available_completion.session_id = session_b;
    available_completion.generation = retire.generation + 1u;
    completion.generation = 0u;
    CHECK(pstvnc_transport_mpeg_take_retire_completion(
        &access_b, &completion) == PSTVNC_TRANSPORT_OK);
    CHECK(completion.generation == retire.generation + 1u);

    available_completion.version =
        PSTVNC_MPEG_GENERATION_CONTROL_VERSION + 1u;
    completion.generation = 0u;
    CHECK(pstvnc_transport_mpeg_take_retire_completion(
        &access_b, &completion) == PSTVNC_TRANSPORT_FAILED);
    CHECK(completion.generation == 0u);

    mpeg_start_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_transport_mpeg_send_start(
        &access_b, &start) == PSTVNC_TRANSPORT_FAILED);

    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
}

static void test_mpeg_control_rejects_invalid_run_meaning_before_runtime(void)
{
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    pstvnc_transport_access_t access;
    pstvnc_transport_mpeg_start_request_t start;
    pstvnc_transport_mpeg_retire_request_t retire;
    int socket_fd = 82;
    int start_calls_before;
    int retire_calls_before;

    reset_fixture();
    memset(&access, 0, sizeof(access));
    fill_bridge_start_request(&start);
    retire.generation = start.generation;

    CHECK(pstvnc_transport_session_open_with_mpeg(
        &socket_fd, &config, &mpeg) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&access) == PSTVNC_TRANSPORT_OK);

    start_calls_before = mpeg_start_calls;
    retire_calls_before = mpeg_retire_calls;

    start.generation = 0u;
    CHECK(pstvnc_transport_mpeg_send_start(
        &access, &start) == PSTVNC_TRANSPORT_INVALID);
    CHECK(mpeg_start_calls == start_calls_before);
    start.generation = 7u;

    start.base_width = 0u;
    CHECK(pstvnc_transport_mpeg_send_start(
        &access, &start) == PSTVNC_TRANSPORT_INVALID);
    CHECK(mpeg_start_calls == start_calls_before);
    start.base_width = 640u;

    start.base_height = 447u;
    CHECK(pstvnc_transport_mpeg_send_start(
        &access, &start) == PSTVNC_TRANSPORT_INVALID);
    CHECK(mpeg_start_calls == start_calls_before);
    start.base_height = 448u;

    start.suppression_x = start.base_x + 1u;
    CHECK(pstvnc_transport_mpeg_send_start(
        &access, &start) == PSTVNC_TRANSPORT_INVALID);
    CHECK(mpeg_start_calls == start_calls_before);
    fill_bridge_start_request(&start);

    start.suppression_width = UINT32_MAX;
    CHECK(pstvnc_transport_mpeg_send_start(
        &access, &start) == PSTVNC_TRANSPORT_INVALID);
    CHECK(mpeg_start_calls == start_calls_before);
    fill_bridge_start_request(&start);

    retire.generation = 0u;
    CHECK(pstvnc_transport_mpeg_send_retire(
        &access, &retire) == PSTVNC_TRANSPORT_INVALID);
    CHECK(mpeg_retire_calls == retire_calls_before);

    CHECK(pstvnc_transport_mpeg_send_start(
        &access, NULL) == PSTVNC_TRANSPORT_INVALID);
    CHECK(pstvnc_transport_mpeg_send_retire(
        &access, NULL) == PSTVNC_TRANSPORT_INVALID);
    CHECK(pstvnc_transport_mpeg_take_retire_completion(
        &access, NULL) == PSTVNC_TRANSPORT_INVALID);
    CHECK(mpeg_start_calls == start_calls_before);
    CHECK(mpeg_retire_calls == retire_calls_before);
    CHECK(mpeg_take_calls == 0);

    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
}

static void test_stale_access_cannot_cross_reconnect(void)
{
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_audio_channel_config_t audio = make_audio_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    pstvnc_transport_access_t access_a;
    pstvnc_transport_access_t access_b;
    unsigned char byte = 0u;
    size_t available = 0u;
    int producer_done = 0;
    int socket_fd = 71;
    int read_before;
    int write_before;
    int audio_before;
    int mpeg_before;

    reset_fixture();
    memset(&access_a, 0, sizeof(access_a));
    memset(&access_b, 0, sizeof(access_b));

    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&access_a) ==
        PSTVNC_TRANSPORT_OK);

    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);

    socket_fd = 72;
    CHECK(pstvnc_transport_session_open_with_audio_mpeg(
        &socket_fd, &config, &audio, &mpeg) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&access_b) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(access_a.opaque_ticket != 0u);
    CHECK(access_b.opaque_ticket != 0u);
    CHECK(access_a.opaque_ticket != access_b.opaque_ticket);

    read_before = rfb_read_calls;
    write_before = rfb_write_calls;
    audio_before = audio_status_calls;
    mpeg_before = mpeg_status_calls;

    CHECK(pstvnc_transport_rfb_read_exact(
        &access_a, &byte, 1u) == PSTVNC_TRANSPORT_CLOSED);
    CHECK(pstvnc_transport_rfb_write_exact(
        &access_a, &byte, 1u) == PSTVNC_TRANSPORT_CLOSED);
    CHECK(pstvnc_transport_audio_status(
        &access_a, &available, &producer_done) == PSTVNC_TRANSPORT_CLOSED);
    CHECK(pstvnc_transport_mpeg_status(
        &access_a, &available, &producer_done) == PSTVNC_TRANSPORT_CLOSED);

    CHECK(rfb_read_calls == read_before);
    CHECK(rfb_write_calls == write_before);
    CHECK(audio_status_calls == audio_before);
    CHECK(mpeg_status_calls == mpeg_before);

    CHECK(pstvnc_transport_rfb_read_exact(
        &access_b, &byte, 1u) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_rfb_write_exact(
        &access_b, &byte, 1u) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_audio_status(
        &access_b, &available, &producer_done) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_mpeg_status(
        &access_b, &available, &producer_done) == PSTVNC_TRANSPORT_OK);

    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
}


static void test_q4_establishment_only_is_active_without_riders(void)
{
    pstvnc_transport_wire_establishment_result_t result;
    int socket_fd = 51;

    reset_fixture();

    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);

    result = pstvnc_transport_wire_establish(&socket_fd);
    CHECK(result.status == PSTVNC_TRANSPORT_WIRE_ESTABLISHED);
    CHECK(result.rejection_reason == (pstvnc_wire_not_accepted_reason_t)0);
    CHECK(socket_fd == -1);
    CHECK(establish_calls == 1);
    CHECK(initialize_calls == 0);
    CHECK(initialize_audio_calls == 0);
    CHECK(initialize_mpeg_calls == 0);
    CHECK(initialize_audio_mpeg_calls == 0);
    CHECK(start_calls == 0);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_ACTIVE);

    CHECK(pstvnc_transport_access_acquire(&current_access) ==
        PSTVNC_TRANSPORT_CLOSED);

    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
    CHECK(physical_release_calls == 1);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);
}

static void test_q4_typed_rejection_and_mechanism_failure(void)
{
    pstvnc_transport_wire_establishment_result_t result;
    int socket_fd;

    reset_fixture();
    establish_result = -1;
    establish_rejection_reason = PSTVNC_WIRE_NOT_ACCEPTED_PRODUCT_VERSION;
    socket_fd = 52;

    result = pstvnc_transport_wire_establish(&socket_fd);
    CHECK(result.status == PSTVNC_TRANSPORT_WIRE_NOT_ACCEPTED);
    CHECK(result.rejection_reason ==
        PSTVNC_WIRE_NOT_ACCEPTED_PRODUCT_VERSION);
    CHECK(socket_fd == -1);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);
    CHECK(start_calls == 0);

    reset_fixture();
    establish_result = 0;
    socket_fd = 53;
    result = pstvnc_transport_wire_establish(&socket_fd);
    CHECK(result.status == PSTVNC_TRANSPORT_WIRE_ESTABLISHMENT_FAILED);
    CHECK(result.rejection_reason == (pstvnc_wire_not_accepted_reason_t)0);
    CHECK(socket_fd == 53);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);
    CHECK(start_calls == 0);
}

static void test_established_wire_then_rider_activation_preserves_lineage(void)
{
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_wire_establishment_result_t result;
    int socket_fd = 54;
    int no_descriptor = -1;

    reset_fixture();
    result = pstvnc_transport_wire_establish(&socket_fd);
    CHECK(result.status == PSTVNC_TRANSPORT_WIRE_ESTABLISHED);
    CHECK(start_calls == 0);

    CHECK(pstvnc_transport_session_open(
        &no_descriptor, &config) == PSTVNC_TRANSPORT_OK);
    CHECK(no_descriptor == -1);
    CHECK(establish_calls == 1);
    CHECK(initialize_calls == 1);
    CHECK(start_calls == 1);
    CHECK(observed_runtime != NULL);
    CHECK(observed_runtime->physical_stream.next_send_sequence == 2u);
    CHECK(observed_runtime->physical_stream.expected_receive_sequence == 2u);
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_ACTIVE);

    complete_and_close();
    CHECK(pstvnc_transport_wire_availability() ==
        PSTVNC_TRANSPORT_WIRE_INACTIVE);
}

static void test_repeated_wire_sessions_do_not_resume_rider_access(void)
{
    pstvnc_transport_wire_establishment_result_t result;
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_access_t access_a;
    pstvnc_transport_access_t access_b;
    int socket_fd = 55;
    int no_descriptor = -1;

    reset_fixture();
    memset(&access_a, 0, sizeof(access_a));
    memset(&access_b, 0, sizeof(access_b));

    result = pstvnc_transport_wire_establish(&socket_fd);
    CHECK(result.status == PSTVNC_TRANSPORT_WIRE_ESTABLISHED);
    CHECK(pstvnc_transport_session_open(
        &no_descriptor, &config) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&access_a) == PSTVNC_TRANSPORT_OK);
    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);

    establish_session_id = 0x55667788u;
    socket_fd = 56;
    no_descriptor = -1;
    result = pstvnc_transport_wire_establish(&socket_fd);
    CHECK(result.status == PSTVNC_TRANSPORT_WIRE_ESTABLISHED);
    CHECK(pstvnc_transport_session_open(
        &no_descriptor, &config) == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_access_acquire(&access_b) == PSTVNC_TRANSPORT_OK);
    CHECK(access_a.opaque_ticket != access_b.opaque_ticket);
    CHECK(pstvnc_transport_rfb_write_exact(
        &access_a, "x", 1u) == PSTVNC_TRANSPORT_CLOSED);
    CHECK(pstvnc_transport_rfb_write_exact(
        &access_b, "x", 1u) == PSTVNC_TRANSPORT_OK);

    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
}

int main(void)
{
    test_q4_establishment_only_is_active_without_riders();
    test_q4_typed_rejection_and_mechanism_failure();
    test_established_wire_then_rider_activation_preserves_lineage();
    test_repeated_wire_sessions_do_not_resume_rider_access();
    test_rfb_only_open_and_close_regression();
    test_audio_open_requires_explicit_config();
    test_mpeg_open_and_combined_authority();
    test_failed_open_ownership_regression();
    test_fatal_abort_order_regression();
    test_rfb_result_mapping_regression();
    test_audio_result_mapping();
    test_mpeg_result_mapping();
    test_mpeg_control_private_identity_and_stale_access_fence();
    test_mpeg_control_rejects_invalid_run_meaning_before_runtime();
    test_stale_access_cannot_cross_reconnect();

    if (failures != 0) {
        fprintf(stderr, "%d transport bridge test(s) failed\n", failures);
        return 1;
    }

    puts("transport bridge tests passed");
    return 0;
}