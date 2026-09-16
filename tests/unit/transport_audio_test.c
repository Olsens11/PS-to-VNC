/*
 * File synopsis:
 * Direct deterministic host fixture for A002 Transport logical AUDIO behavior.
 * A scripted physical stream drives interleaved RFB/AUDIO DATA through the real
 * sole-receiver runtime while independent credit records and activity waits make
 * queue, finite-producer, failure, stop, and regression behavior observable.
 *
 * No PCM/AUDSRV or media-presentation behavior exists in this fixture.
 *
 * Context: LEDGE_FOREMAN_STATE revision 0009, packet A1-A8.
 */

#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transport_host_stubs/kernel.h"
#include "transport/protocol.h"
#include "transport/runtime.h"

#define MAX_SEMAS 32
#define MAX_THREADS 8
#define MAX_FRAMES 32
#define MAX_SENDS 64

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

typedef struct fake_sema {
    int used;
    int count;
    int maximum;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} fake_sema_t;

static fake_sema_t semaphores[MAX_SEMAS];

typedef struct fake_thread {
    int used;
    int started;
    int status;
    ee_thread_t definition;
    void *argument;
    pthread_t handle;
} fake_thread_t;

static fake_thread_t threads[MAX_THREADS];
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned char _gp;

int CreateSema(ee_sema_t *semaphore)
{
    int id;

    if (semaphore == NULL)
        return -1;

    for (id = 1; id < MAX_SEMAS; id++) {
        if (!semaphores[id].used) {
            semaphores[id].used = 1;
            semaphores[id].count = semaphore->init_count;
            semaphores[id].maximum = semaphore->max_count;
            pthread_mutex_init(&semaphores[id].mutex, NULL);
            pthread_cond_init(&semaphores[id].condition, NULL);
            return id;
        }
    }

    return -1;
}

int DeleteSema(int semaphore_id)
{
    fake_sema_t *semaphore;

    if (semaphore_id <= 0 || semaphore_id >= MAX_SEMAS ||
        !semaphores[semaphore_id].used)
        return -1;

    semaphore = &semaphores[semaphore_id];
    pthread_mutex_destroy(&semaphore->mutex);
    pthread_cond_destroy(&semaphore->condition);
    memset(semaphore, 0, sizeof(*semaphore));
    return 0;
}

int WaitSema(int semaphore_id)
{
    fake_sema_t *semaphore;

    if (semaphore_id <= 0 || semaphore_id >= MAX_SEMAS ||
        !semaphores[semaphore_id].used)
        return -1;

    semaphore = &semaphores[semaphore_id];
    pthread_mutex_lock(&semaphore->mutex);
    while (semaphore->count == 0)
        pthread_cond_wait(&semaphore->condition, &semaphore->mutex);
    semaphore->count--;
    pthread_mutex_unlock(&semaphore->mutex);
    return 0;
}

int SignalSema(int semaphore_id)
{
    fake_sema_t *semaphore;

    if (semaphore_id <= 0 || semaphore_id >= MAX_SEMAS ||
        !semaphores[semaphore_id].used)
        return -1;

    semaphore = &semaphores[semaphore_id];
    pthread_mutex_lock(&semaphore->mutex);
    if (semaphore->count >= semaphore->maximum) {
        pthread_mutex_unlock(&semaphore->mutex);
        return -1;
    }
    semaphore->count++;
    pthread_cond_signal(&semaphore->condition);
    pthread_mutex_unlock(&semaphore->mutex);
    return 0;
}

static void *fake_thread_entry(void *opaque)
{
    int thread_id = *(int *)opaque;
    void (*function)(void *);
    void *argument;

    free(opaque);
    pthread_mutex_lock(&thread_mutex);
    function = (void (*)(void *))threads[thread_id].definition.func;
    argument = threads[thread_id].argument;
    pthread_mutex_unlock(&thread_mutex);

    function(argument);

    pthread_mutex_lock(&thread_mutex);
    threads[thread_id].status = THS_DORMANT;
    pthread_mutex_unlock(&thread_mutex);
    return NULL;
}

int CreateThread(ee_thread_t *thread)
{
    int id;

    if (thread == NULL)
        return -1;

    pthread_mutex_lock(&thread_mutex);
    for (id = 1; id < MAX_THREADS; id++) {
        if (!threads[id].used) {
            threads[id].used = 1;
            threads[id].status = THS_DORMANT;
            threads[id].definition = *thread;
            pthread_mutex_unlock(&thread_mutex);
            return id;
        }
    }
    pthread_mutex_unlock(&thread_mutex);
    return -1;
}

int StartThread(int thread_id, void *argument)
{
    int *thread_id_copy;

    if (thread_id <= 0 || thread_id >= MAX_THREADS ||
        !threads[thread_id].used)
        return -1;

    thread_id_copy = (int *)malloc(sizeof(*thread_id_copy));
    if (thread_id_copy == NULL)
        return -1;
    *thread_id_copy = thread_id;

    threads[thread_id].argument = argument;
    threads[thread_id].started = 1;
    threads[thread_id].status = THS_RUNNING;

    if (pthread_create(
            &threads[thread_id].handle,
            NULL,
            fake_thread_entry,
            thread_id_copy) != 0) {
        free(thread_id_copy);
        return -1;
    }

    return 0;
}

int ReferThreadStatus(int thread_id, ee_thread_status_t *status)
{
    if (thread_id <= 0 || thread_id >= MAX_THREADS ||
        !threads[thread_id].used || status == NULL)
        return -1;

    pthread_mutex_lock(&thread_mutex);
    status->status = threads[thread_id].status;
    pthread_mutex_unlock(&thread_mutex);
    return 0;
}

int TerminateThread(int thread_id)
{
    if (thread_id <= 0 || thread_id >= MAX_THREADS ||
        !threads[thread_id].used)
        return -1;

    pthread_mutex_lock(&thread_mutex);
    threads[thread_id].status = THS_DORMANT;
    pthread_mutex_unlock(&thread_mutex);
    return 0;
}

int DeleteThread(int thread_id)
{
    pthread_t handle;
    int started;

    if (thread_id <= 0 || thread_id >= MAX_THREADS ||
        !threads[thread_id].used)
        return -1;

    handle = threads[thread_id].handle;
    started = threads[thread_id].started;
    if (started && !pthread_equal(pthread_self(), handle))
        pthread_join(handle, NULL);

    memset(&threads[thread_id], 0, sizeof(threads[thread_id]));
    return 0;
}

void ExitThread(void)
{
}

typedef struct fake_frame {
    pstvnc_transport_header_t header;
    uint8_t payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];
} fake_frame_t;

static fake_frame_t frames[MAX_FRAMES];
static size_t frame_count;
static size_t frame_index;
static int receive_calls;
static int receive_active;
static int max_receive_active;
static int receive_thread_mismatch;
static int receive_shutdown;
static int receive_eof;
static pthread_t receiver_identity;
static int receiver_identity_valid;
static pthread_mutex_t receive_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t receive_condition = PTHREAD_COND_INITIALIZER;
static pthread_cond_t receive_progress = PTHREAD_COND_INITIALIZER;

typedef struct fake_send_record {
    uint8_t kind;
    uint8_t channel;
    uint8_t flags;
    size_t payload_length;
    uint8_t payload[32];
} fake_send_record_t;

static fake_send_record_t send_records[MAX_SENDS];
static size_t send_record_count;
static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
static int shutdown_calls;
static int release_calls;
static int adopt_calls;

int pstvnc_transport_physical_stream_adopt(
    pstvnc_transport_physical_stream_t *stream,
    int socket_fd)
{
    adopt_calls++;
    if (stream == NULL || socket_fd < 0)
        return 0;

    stream->socket_fd = socket_fd;
    stream->send_semaphore_id = 77;
    stream->next_send_sequence = 1u;
    stream->expected_receive_sequence = 1u;
    return 1;
}

int pstvnc_transport_physical_stream_send_frame(
    pstvnc_transport_physical_stream_t *stream,
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const void *payload,
    size_t payload_length)
{
    fake_send_record_t *record;

    CHECK(stream != NULL);
    pthread_mutex_lock(&send_mutex);
    CHECK(send_record_count < MAX_SENDS);
    if (send_record_count >= MAX_SENDS) {
        pthread_mutex_unlock(&send_mutex);
        return 0;
    }

    record = &send_records[send_record_count++];
    memset(record, 0, sizeof(*record));
    record->kind = kind;
    record->channel = channel;
    record->flags = flags;
    record->payload_length = payload_length;
    CHECK(payload_length <= sizeof(record->payload));
    if (payload_length != 0u && payload != NULL &&
        payload_length <= sizeof(record->payload))
        memcpy(record->payload, payload, payload_length);
    pthread_mutex_unlock(&send_mutex);
    return 1;
}

int pstvnc_transport_physical_stream_receive_frame(
    pstvnc_transport_physical_stream_t *stream,
    pstvnc_transport_header_t *header,
    void *payload,
    size_t payload_capacity)
{
    fake_frame_t frame;

    CHECK(stream != NULL);
    pthread_mutex_lock(&receive_mutex);
    receive_calls++;
    receive_active++;
    if (receive_active > max_receive_active)
        max_receive_active = receive_active;

    if (!receiver_identity_valid) {
        receiver_identity = pthread_self();
        receiver_identity_valid = 1;
    } else if (!pthread_equal(receiver_identity, pthread_self())) {
        receive_thread_mismatch = 1;
    }

    pthread_cond_broadcast(&receive_progress);
    while (frame_index >= frame_count && !receive_shutdown && !receive_eof)
        pthread_cond_wait(&receive_condition, &receive_mutex);

    if (receive_shutdown || receive_eof) {
        receive_active--;
        pthread_mutex_unlock(&receive_mutex);
        return 0;
    }

    frame = frames[frame_index++];
    receive_active--;
    pthread_mutex_unlock(&receive_mutex);

    if (frame.header.payload_length > payload_capacity)
        return 0;

    *header = frame.header;
    if (frame.header.payload_length != 0u)
        memcpy(payload, frame.payload, frame.header.payload_length);
    return 1;
}

int pstvnc_transport_physical_stream_shutdown_io(
    pstvnc_transport_physical_stream_t *stream)
{
    CHECK(stream != NULL);
    shutdown_calls++;
    pthread_mutex_lock(&receive_mutex);
    receive_shutdown = 1;
    pthread_cond_broadcast(&receive_condition);
    pthread_mutex_unlock(&receive_mutex);
    return 1;
}

void pstvnc_transport_physical_stream_release(
    pstvnc_transport_physical_stream_t *stream)
{
    release_calls++;
    if (stream != NULL) {
        stream->socket_fd = -1;
        stream->send_semaphore_id = -1;
    }
}

static void reset_fixture(void)
{
    int id;

    for (id = 1; id < MAX_SEMAS; id++) {
        if (semaphores[id].used)
            (void)DeleteSema(id);
    }

    memset(threads, 0, sizeof(threads));

    pthread_mutex_lock(&receive_mutex);
    memset(frames, 0, sizeof(frames));
    frame_count = 0u;
    frame_index = 0u;
    receive_calls = 0;
    receive_active = 0;
    max_receive_active = 0;
    receive_thread_mismatch = 0;
    receive_shutdown = 0;
    receive_eof = 0;
    receiver_identity_valid = 0;
    pthread_mutex_unlock(&receive_mutex);

    pthread_mutex_lock(&send_mutex);
    memset(send_records, 0, sizeof(send_records));
    send_record_count = 0u;
    pthread_mutex_unlock(&send_mutex);

    shutdown_calls = 0;
    release_calls = 0;
    adopt_calls = 0;
}

static pstvnc_transport_session_config_t make_rfb_config(void)
{
    pstvnc_transport_session_config_t config;

    memset(&config, 0, sizeof(config));
    config.rfb_queue_capacity = 16u;
    config.rfb_initial_credit_bytes = 8u;
    config.rfb_credit_batch_bytes = 4u;
    config.rfb_credit_flush_on_empty = 1;
    config.rfb_credit_return_enabled = 1;
    config.receiver_thread_stack_size = 512u;
    config.receiver_thread_priority = 64;
    config.max_data_payload = 4u;
    return config;
}

static pstvnc_transport_audio_channel_config_t make_audio_config(
    uint32_t capacity)
{
    pstvnc_transport_audio_channel_config_t config;

    memset(&config, 0, sizeof(config));
    config.queue_capacity = capacity;
    config.initial_credit_bytes = capacity;
    config.credit_batch_bytes = 4u;
    config.credit_flush_on_empty = 1;
    config.credit_return_enabled = 1;
    return config;
}

static void push_frame(
    uint8_t kind,
    uint8_t channel,
    const uint8_t *payload,
    size_t payload_length)
{
    fake_frame_t *frame;

    pthread_mutex_lock(&receive_mutex);
    CHECK(frame_count < MAX_FRAMES);
    frame = &frames[frame_count++];
    memset(frame, 0, sizeof(*frame));
    frame->header.version = PSTVNC_TRANSPORT_VERSION;
    frame->header.kind = kind;
    frame->header.channel = channel;
    frame->header.sequence = (uint32_t)frame_count;
    frame->header.payload_length = (uint32_t)payload_length;
    if (payload_length != 0u)
        memcpy(frame->payload, payload, payload_length);
    pthread_cond_broadcast(&receive_condition);
    pthread_mutex_unlock(&receive_mutex);
}

static void wait_for_receive_calls(int target)
{
    pthread_mutex_lock(&receive_mutex);
    while (receive_calls < target)
        pthread_cond_wait(&receive_progress, &receive_mutex);
    pthread_mutex_unlock(&receive_mutex);
}

static void clear_send_records(void)
{
    pthread_mutex_lock(&send_mutex);
    memset(send_records, 0, sizeof(send_records));
    send_record_count = 0u;
    pthread_mutex_unlock(&send_mutex);
}

static uint32_t send_credit_amount(size_t index)
{
    CHECK(index < send_record_count);
    return pstvnc_transport_read_be32(send_records[index].payload);
}

static void initialize_audio_runtime(
    pstvnc_transport_runtime_t *runtime,
    uint32_t capacity)
{
    pstvnc_transport_session_config_t rfb_config = make_rfb_config();
    pstvnc_transport_audio_channel_config_t audio_config =
        make_audio_config(capacity);

    memset(runtime, 0xa5, sizeof(*runtime));
    CHECK(pstvnc_transport_runtime_initialize_with_audio(
        runtime, 91, &rfb_config, &audio_config) == 1);
    CHECK(runtime->audio_enabled == 1);
}

static void start_runtime(pstvnc_transport_runtime_t *runtime)
{
    CHECK(pstvnc_transport_runtime_start_receiver(runtime) == 1);
    wait_for_receive_calls(1);
}

static void stop_and_release_runtime(pstvnc_transport_runtime_t *runtime)
{
    if (runtime->receiver_thread_started && !runtime->receiver_done) {
        CHECK(pstvnc_transport_runtime_request_stop(runtime) == 1);
        CHECK(pstvnc_transport_runtime_wait_receiver_done(runtime) == 1);
    }
    CHECK(pstvnc_transport_runtime_release(runtime) == 1);
}

static void test_explicit_audio_config_and_rfb_only_regression(void)
{
    static const uint8_t rfb_payload[] = { 1, 2 };
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t rfb_config = make_rfb_config();
    pstvnc_transport_audio_channel_config_t audio_config = make_audio_config(3u);
    uint8_t output[2];

    reset_fixture();
    CHECK(pstvnc_transport_runtime_initialize_with_audio(
        &runtime, 91, &rfb_config, NULL) == 0);
    CHECK(adopt_calls == 0);
    CHECK(pstvnc_transport_runtime_initialize_with_audio(
        &runtime, 91, &rfb_config, &audio_config) == 0);
    CHECK(adopt_calls == 0);

    reset_fixture();
    CHECK(pstvnc_transport_runtime_initialize(
        &runtime, 91, &rfb_config) == 1);
    CHECK(runtime.audio_enabled == 0);
    start_runtime(&runtime);
    CHECK(send_record_count == 1u);
    CHECK(send_records[0].channel == PSTVNC_TRANSPORT_CHANNEL_RFB);

    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        rfb_payload,
        sizeof(rfb_payload));
    wait_for_receive_calls(2);
    CHECK(pstvnc_transport_runtime_rfb_read_exact(
        &runtime, output, sizeof(output)) == 1);
    CHECK(memcmp(output, rfb_payload, sizeof(output)) == 0);

    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO,
        NULL,
        0u);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_interleaved_single_receiver_and_independent_credit(void)
{
    static const uint8_t rfb_payload[] = { 9, 8, 7 };
    static const uint8_t audio_payload[] = { 1, 2, 3 };
    pstvnc_transport_runtime_t runtime;
    uint8_t rfb_output[3];
    uint8_t audio_output[4];
    size_t read_count;

    reset_fixture();
    initialize_audio_runtime(&runtime, 8u);
    start_runtime(&runtime);

    CHECK(send_record_count == 2u);
    CHECK(send_records[0].channel == PSTVNC_TRANSPORT_CHANNEL_RFB);
    CHECK(send_credit_amount(0u) == 8u);
    CHECK(send_records[1].channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO);
    CHECK(send_credit_amount(1u) == 8u);
    clear_send_records();

    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        rfb_payload,
        sizeof(rfb_payload));
    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO,
        audio_payload,
        sizeof(audio_payload));
    wait_for_receive_calls(3);

    CHECK(max_receive_active == 1);
    CHECK(receive_thread_mismatch == 0);

    CHECK(pstvnc_transport_runtime_rfb_read_exact(
        &runtime, rfb_output, sizeof(rfb_output)) == 1);
    CHECK(memcmp(rfb_output, rfb_payload, sizeof(rfb_output)) == 0);
    CHECK(send_record_count == 1u);
    CHECK(send_records[0].channel == PSTVNC_TRANSPORT_CHANNEL_RFB);
    CHECK(send_credit_amount(0u) == 3u);

    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, audio_output, 2u, &read_count) == PSTVNC_TRANSPORT_OK);
    CHECK(read_count == 2u);
    CHECK(audio_output[0] == 1u && audio_output[1] == 2u);
    CHECK(send_record_count == 1u);

    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, audio_output, 4u, &read_count) == PSTVNC_TRANSPORT_OK);
    CHECK(read_count == 1u && audio_output[0] == 3u);
    CHECK(send_record_count == 2u);
    CHECK(send_records[1].channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO);
    CHECK(send_credit_amount(1u) == 3u);

    stop_and_release_runtime(&runtime);
}

static void test_marker_drain_exhaust_and_post_marker_reject(void)
{
    static const uint8_t audio_payload[] = { 4, 5, 6, 7 };
    static const uint8_t bad_payload[] = { 9 };
    pstvnc_transport_runtime_t runtime;
    uint8_t output[8];
    size_t read_count;
    size_t available;
    int producer_done;

    reset_fixture();
    initialize_audio_runtime(&runtime, 8u);
    start_runtime(&runtime);
    clear_send_records();

    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO,
        audio_payload,
        sizeof(audio_payload));
    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO,
        NULL,
        0u);
    wait_for_receive_calls(3);

    CHECK(pstvnc_transport_runtime_audio_status(
        &runtime, &available, &producer_done) == PSTVNC_TRANSPORT_OK);
    CHECK(available == 4u && producer_done == 1);

    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, output, 2u, &read_count) == PSTVNC_TRANSPORT_OK);
    CHECK(read_count == 2u);
    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, output + 2u, 6u, &read_count) == PSTVNC_TRANSPORT_OK);
    CHECK(read_count == 2u);
    CHECK(memcmp(output, audio_payload, 4u) == 0);
    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, output, 1u, &read_count) == PSTVNC_TRANSPORT_EXHAUSTED);
    CHECK(read_count == 0u);

    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO,
        bad_payload,
        sizeof(bad_payload));
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);
    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, output, 1u, &read_count) == PSTVNC_TRANSPORT_FAILED);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_overflow_fails_but_committed_bytes_drain(void)
{
    static const uint8_t first_payload[] = { 1, 2, 3 };
    static const uint8_t overflow_payload[] = { 4, 5 };
    pstvnc_transport_runtime_t runtime;
    uint8_t output[4];
    size_t read_count;

    reset_fixture();
    initialize_audio_runtime(&runtime, 4u);
    start_runtime(&runtime);

    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO,
        first_payload,
        sizeof(first_payload));
    push_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO,
        overflow_payload,
        sizeof(overflow_payload));

    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);
    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, output, sizeof(output), &read_count) == PSTVNC_TRANSPORT_OK);
    CHECK(read_count == 3u);
    CHECK(memcmp(output, first_payload, 3u) == 0);
    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, output, sizeof(output), &read_count) == PSTVNC_TRANSPORT_FAILED);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

typedef struct activity_waiter {
    pstvnc_transport_runtime_t *runtime;
    uint32_t sequence;
    int result;
} activity_waiter_t;

static void *activity_waiter_main(void *opaque)
{
    activity_waiter_t *waiter = (activity_waiter_t *)opaque;

    waiter->result = pstvnc_transport_runtime_audio_wait_activity(
        waiter->runtime,
        &waiter->sequence);
    return NULL;
}

static void wait_until_audio_wait_is_armed(pstvnc_transport_runtime_t *runtime)
{
    int armed = 0;

    while (!armed) {
        CHECK(WaitSema(runtime->audio_queue_semaphore_id) == 0);
        armed = runtime->audio_activity_wait_armed;
        CHECK(SignalSema(runtime->audio_queue_semaphore_id) == 0);
        if (!armed)
            sched_yield();
    }
}

static void run_terminal_wakeup_case(int mode)
{
    pstvnc_transport_runtime_t runtime;
    activity_waiter_t waiter;
    pthread_t waiter_thread;
    uint8_t output;
    size_t read_count;

    reset_fixture();
    initialize_audio_runtime(&runtime, 8u);
    start_runtime(&runtime);

    waiter.runtime = &runtime;
    waiter.result = 0;
    CHECK(pstvnc_transport_runtime_audio_activity_snapshot(
        &runtime, &waiter.sequence) == 1);
    CHECK(pthread_create(
        &waiter_thread, NULL, activity_waiter_main, &waiter) == 0);
    wait_until_audio_wait_is_armed(&runtime);

    if (mode == 0) {
        pthread_mutex_lock(&receive_mutex);
        receive_eof = 1;
        pthread_cond_broadcast(&receive_condition);
        pthread_mutex_unlock(&receive_mutex);
    } else if (mode == 1) {
        push_frame(
            PSTVNC_TRANSPORT_FRAME_CREDIT,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            NULL,
            0u);
    } else {
        CHECK(pstvnc_transport_runtime_request_stop(&runtime) == 1);
    }

    CHECK(pthread_join(waiter_thread, NULL) == 0);
    CHECK(waiter.result == 1);
    CHECK(runtime.audio_activity_wait_armed == 0);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);

    if (mode == 2) {
        CHECK(pstvnc_transport_runtime_audio_read_available(
            &runtime, &output, 1u, &read_count) == PSTVNC_TRANSPORT_STOPPED);
    } else {
        CHECK(pstvnc_transport_runtime_audio_read_available(
            &runtime, &output, 1u, &read_count) == PSTVNC_TRANSPORT_FAILED);
    }

    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_terminal_wakeups(void)
{
    run_terminal_wakeup_case(0);
    run_terminal_wakeup_case(1);
    run_terminal_wakeup_case(2);
}

static void test_release_preserves_signaled_audio_waiter_resources(void)
{
    pstvnc_transport_runtime_t runtime;
    int activity_semaphore_id;

    reset_fixture();
    initialize_audio_runtime(&runtime, 8u);
    activity_semaphore_id = runtime.audio_activity_semaphore_id;

    CHECK(WaitSema(runtime.audio_queue_semaphore_id) == 0);
    runtime.receiver_done = 1;
    runtime.audio_activity_wait_armed = 2;
    CHECK(SignalSema(runtime.audio_queue_semaphore_id) == 0);

    CHECK(pstvnc_transport_runtime_release(&runtime) == 0);
    CHECK(runtime.initialized == 1);
    CHECK(release_calls == 0);
    CHECK(activity_semaphore_id > 0 && activity_semaphore_id < MAX_SEMAS);
    CHECK(semaphores[activity_semaphore_id].used == 1);

    CHECK(WaitSema(runtime.audio_queue_semaphore_id) == 0);
    runtime.audio_activity_wait_armed = 0;
    CHECK(SignalSema(runtime.audio_queue_semaphore_id) == 0);

    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
    CHECK(release_calls == 1);
}

int main(void)
{
    test_explicit_audio_config_and_rfb_only_regression();
    test_interleaved_single_receiver_and_independent_credit();
    test_marker_drain_exhaust_and_post_marker_reject();
    test_overflow_fails_but_committed_bytes_drain();
    test_terminal_wakeups();
    test_release_preserves_signaled_audio_waiter_resources();
    reset_fixture();

    if (failures != 0) {
        fprintf(stderr, "transport_audio_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("transport_audio_test: PASS");
    return 0;
}
