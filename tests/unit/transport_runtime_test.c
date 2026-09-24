/*
 * File synopsis:
 * Direct host behavior fixture for src/transport/runtime.c + quiesce.c using the
 * real logical-RFB channel implementation. Deterministic host kernel and
 * physical-stream primitives provide a blocking sole-receiver seam without
 * sleeps or timeouts. The fixture proves dispatch/fail-closed behavior,
 * producer activity, parser-credit accounting, fragmentation, finite quiesce,
 * fatal-stop completion ordering, retryable failure, and repeatable ownership.
 *
 * Context: LEDGE_FOREMAN_STATE revision 0003, packet C2-C7.
 */

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transport_host_stubs/kernel.h"
#include "transport/protocol.h"
#include "transport/rfb_channel.h"
#include "transport/runtime.h"

#define MAX_FAKE_SEMAS 32
#define MAX_FAKE_THREADS 8
#define MAX_RX_FRAMES 32
#define MAX_SEND_RECORDS 64
#define MAX_EVENT_RECORDS 64

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

typedef enum lifecycle_event {
    EVENT_SHUTDOWN_IO = 1,
    EVENT_RECEIVER_DONE_SIGNAL,
    EVENT_REFER_THREAD,
    EVENT_TERMINATE_THREAD,
    EVENT_DELETE_THREAD,
    EVENT_PHYSICAL_RELEASE
} lifecycle_event_t;

static lifecycle_event_t lifecycle_events[MAX_EVENT_RECORDS];
static size_t lifecycle_event_count;
static pthread_mutex_t event_mutex = PTHREAD_MUTEX_INITIALIZER;

static void record_event(lifecycle_event_t event)
{
    pthread_mutex_lock(&event_mutex);
    CHECK(lifecycle_event_count < MAX_EVENT_RECORDS);
    if (lifecycle_event_count < MAX_EVENT_RECORDS)
        lifecycle_events[lifecycle_event_count++] = event;
    pthread_mutex_unlock(&event_mutex);
}

static int event_index(lifecycle_event_t event)
{
    size_t index;
    int result = -1;

    pthread_mutex_lock(&event_mutex);
    for (index = 0u; index < lifecycle_event_count; index++) {
        if (lifecycle_events[index] == event) {
            result = (int)index;
            break;
        }
    }
    pthread_mutex_unlock(&event_mutex);
    return result;
}

typedef struct fake_semaphore {
    int used;
    int count;
    int maximum;
    int waiters;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} fake_semaphore_t;

static fake_semaphore_t fake_semaphores[MAX_FAKE_SEMAS];
static int create_sema_calls;
static int create_sema_fail_on_call;
static int receiver_done_semaphore_id = -1;
static int outbound_slot_semaphore_id = -1;
static int outbound_ready_semaphore_id = -1;
static int outbound_done_semaphore_id = -1;
static int outbound_submitter_drain_semaphore_id = -1;
static int rfb_outbound_credit_wait_semaphore_id = -1;
static int audio_activity_wait_semaphore_id = -1;
static int mpeg_activity_wait_semaphore_id = -1;
static int rfb_outbound_credit_wait_calls;
static int audio_activity_wait_calls;
static int mpeg_activity_wait_calls;

/*
 * R20C host-only barriers make the receiver's final completion edge
 * deterministic. They never alter product source or use sleeps/timeouts.
 */
static int receiver_done_signal_blocked;
static int receiver_done_signal_entered;
static int receiver_done_wait_calls;
static int outbound_submitter_drain_wait_calls;
static int outbound_ready_signal_blocked;
static int outbound_ready_signal_entered;
static int outbound_slot_signal_blocked;
static int outbound_slot_signal_entered;
static int wait_readable_blocked;
static int wait_readable_entered;
static pthread_mutex_t completion_fence_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t completion_fence_condition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t rfb_credit_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t rfb_credit_wait_condition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t media_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t media_wait_condition = PTHREAD_COND_INITIALIZER;

typedef struct fake_thread {
    int used;
    int started;
    int status;
    ee_thread_t definition;
    void *argument;
    pthread_t handle;
} fake_thread_t;

static fake_thread_t fake_threads[MAX_FAKE_THREADS];
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static int create_thread_calls;
static int create_thread_fail;
static int start_thread_fail;
static int delete_thread_calls;
static int terminate_thread_calls;
static int fail_first_refer;
static int failed_refer_consumed;
static int force_first_refer_running;
static int forced_refer_consumed;
static __thread int current_fake_thread_id = -1;

unsigned char _gp;

static void destroy_unused_host_state(void)
{
    int index;

    for (index = 1; index < MAX_FAKE_SEMAS; index++) {
        if (fake_semaphores[index].used) {
            pthread_mutex_destroy(&fake_semaphores[index].mutex);
            pthread_cond_destroy(&fake_semaphores[index].condition);
        }
    }

    memset(fake_semaphores, 0, sizeof(fake_semaphores));
    memset(fake_threads, 0, sizeof(fake_threads));
}

int CreateSema(ee_sema_t *semaphore)
{
    int id;

    create_sema_calls++;
    if (semaphore == NULL ||
        (create_sema_fail_on_call > 0 &&
         create_sema_calls == create_sema_fail_on_call))
        return -1;

    for (id = 1; id < MAX_FAKE_SEMAS; id++) {
        if (!fake_semaphores[id].used) {
            fake_semaphores[id].used = 1;
            fake_semaphores[id].count = semaphore->init_count;
            fake_semaphores[id].maximum = semaphore->max_count;
            pthread_mutex_init(&fake_semaphores[id].mutex, NULL);
            pthread_cond_init(&fake_semaphores[id].condition, NULL);
            return id;
        }
    }

    return -1;
}

int DeleteSema(int semaphore_id)
{
    fake_semaphore_t *semaphore;

    if (semaphore_id <= 0 || semaphore_id >= MAX_FAKE_SEMAS ||
        !fake_semaphores[semaphore_id].used)
        return -1;

    semaphore = &fake_semaphores[semaphore_id];
    pthread_mutex_destroy(&semaphore->mutex);
    pthread_cond_destroy(&semaphore->condition);
    memset(semaphore, 0, sizeof(*semaphore));
    return 0;
}

int WaitSema(int semaphore_id)
{
    fake_semaphore_t *semaphore;

    if (semaphore_id <= 0 || semaphore_id >= MAX_FAKE_SEMAS ||
        !fake_semaphores[semaphore_id].used)
        return -1;

    if (semaphore_id == receiver_done_semaphore_id) {
        pthread_mutex_lock(&completion_fence_mutex);
        receiver_done_wait_calls++;
        pthread_cond_broadcast(&completion_fence_condition);
        pthread_mutex_unlock(&completion_fence_mutex);
    }

    if (semaphore_id == rfb_outbound_credit_wait_semaphore_id) {
        pthread_mutex_lock(&rfb_credit_wait_mutex);
        rfb_outbound_credit_wait_calls++;
        pthread_cond_broadcast(&rfb_credit_wait_condition);
        pthread_mutex_unlock(&rfb_credit_wait_mutex);
    }

    if (semaphore_id == audio_activity_wait_semaphore_id ||
        semaphore_id == mpeg_activity_wait_semaphore_id) {
        pthread_mutex_lock(&media_wait_mutex);
        if (semaphore_id == audio_activity_wait_semaphore_id)
            audio_activity_wait_calls++;
        else
            mpeg_activity_wait_calls++;
        pthread_cond_broadcast(&media_wait_condition);
        pthread_mutex_unlock(&media_wait_mutex);
    }

    if (semaphore_id == outbound_submitter_drain_semaphore_id) {
        pthread_mutex_lock(&completion_fence_mutex);
        outbound_submitter_drain_wait_calls++;
        pthread_cond_broadcast(&completion_fence_condition);
        pthread_mutex_unlock(&completion_fence_mutex);
    }

    semaphore = &fake_semaphores[semaphore_id];
    pthread_mutex_lock(&semaphore->mutex);
    if (semaphore->count == 0) {
        semaphore->waiters++;
        pthread_cond_broadcast(&semaphore->condition);
        while (semaphore->count == 0)
            pthread_cond_wait(&semaphore->condition, &semaphore->mutex);
        semaphore->waiters--;
        pthread_cond_broadcast(&semaphore->condition);
    }
    semaphore->count--;
    pthread_mutex_unlock(&semaphore->mutex);
    return 0;
}

int SignalSema(int semaphore_id)
{
    fake_semaphore_t *semaphore;

    if (semaphore_id <= 0 || semaphore_id >= MAX_FAKE_SEMAS ||
        !fake_semaphores[semaphore_id].used)
        return -1;

    if (semaphore_id == outbound_ready_semaphore_id) {
        pthread_mutex_lock(&completion_fence_mutex);
        if (outbound_ready_signal_blocked) {
            outbound_ready_signal_entered = 1;
            pthread_cond_broadcast(&completion_fence_condition);
            while (outbound_ready_signal_blocked)
                pthread_cond_wait(
                    &completion_fence_condition,
                    &completion_fence_mutex);
        }
        pthread_mutex_unlock(&completion_fence_mutex);
    }

    if (semaphore_id == outbound_slot_semaphore_id) {
        pthread_mutex_lock(&completion_fence_mutex);
        if (outbound_slot_signal_blocked) {
            outbound_slot_signal_entered = 1;
            pthread_cond_broadcast(&completion_fence_condition);
            while (outbound_slot_signal_blocked)
                pthread_cond_wait(
                    &completion_fence_condition,
                    &completion_fence_mutex);
        }
        pthread_mutex_unlock(&completion_fence_mutex);
    }

    if (semaphore_id == receiver_done_semaphore_id) {
        pthread_mutex_lock(&completion_fence_mutex);
        receiver_done_signal_entered = 1;
        pthread_cond_broadcast(&completion_fence_condition);
        while (receiver_done_signal_blocked)
            pthread_cond_wait(
                &completion_fence_condition,
                &completion_fence_mutex);
        pthread_mutex_unlock(&completion_fence_mutex);
        record_event(EVENT_RECEIVER_DONE_SIGNAL);
    }

    semaphore = &fake_semaphores[semaphore_id];
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
    current_fake_thread_id = thread_id;
    function = (void (*)(void *))fake_threads[thread_id].definition.func;
    argument = fake_threads[thread_id].argument;
    pthread_mutex_unlock(&thread_mutex);

    function(argument);

    pthread_mutex_lock(&thread_mutex);
    fake_threads[thread_id].status = THS_DORMANT;
    pthread_mutex_unlock(&thread_mutex);
    current_fake_thread_id = -1;
    return NULL;
}

int CreateThread(ee_thread_t *thread)
{
    int id;

    create_thread_calls++;
    if (create_thread_fail || thread == NULL)
        return -1;

    pthread_mutex_lock(&thread_mutex);
    for (id = 1; id < MAX_FAKE_THREADS; id++) {
        if (!fake_threads[id].used) {
            fake_threads[id].used = 1;
            fake_threads[id].status = THS_DORMANT;
            fake_threads[id].definition = *thread;
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

    if (start_thread_fail || thread_id <= 0 ||
        thread_id >= MAX_FAKE_THREADS || !fake_threads[thread_id].used)
        return -1;

    thread_id_copy = (int *)malloc(sizeof(*thread_id_copy));
    if (thread_id_copy == NULL)
        return -1;
    *thread_id_copy = thread_id;

    pthread_mutex_lock(&thread_mutex);
    fake_threads[thread_id].argument = argument;
    fake_threads[thread_id].status = THS_RUNNING;
    fake_threads[thread_id].started = 1;
    pthread_mutex_unlock(&thread_mutex);

    if (pthread_create(
            &fake_threads[thread_id].handle,
            NULL,
            fake_thread_entry,
            thread_id_copy) != 0) {
        free(thread_id_copy);
        pthread_mutex_lock(&thread_mutex);
        fake_threads[thread_id].started = 0;
        fake_threads[thread_id].status = THS_DORMANT;
        pthread_mutex_unlock(&thread_mutex);
        return -1;
    }

    return 0;
}

int ReferThreadStatus(int thread_id, ee_thread_status_t *status)
{
    if (thread_id <= 0 || thread_id >= MAX_FAKE_THREADS ||
        !fake_threads[thread_id].used || status == NULL)
        return -1;

    record_event(EVENT_REFER_THREAD);
    if (fail_first_refer && !failed_refer_consumed) {
        failed_refer_consumed = 1;
        return -1;
    }

    pthread_mutex_lock(&thread_mutex);
    if (force_first_refer_running && !forced_refer_consumed) {
        status->status = THS_RUNNING;
        forced_refer_consumed = 1;
    } else {
        status->status = fake_threads[thread_id].status;
    }
    pthread_mutex_unlock(&thread_mutex);
    return 0;
}

int TerminateThread(int thread_id)
{
    if (thread_id <= 0 || thread_id >= MAX_FAKE_THREADS ||
        !fake_threads[thread_id].used)
        return -1;

    terminate_thread_calls++;
    record_event(EVENT_TERMINATE_THREAD);
    pthread_mutex_lock(&thread_mutex);
    fake_threads[thread_id].status = THS_DORMANT;
    pthread_mutex_unlock(&thread_mutex);
    return 0;
}

int DeleteThread(int thread_id)
{
    pthread_t handle;
    int started;

    if (thread_id <= 0 || thread_id >= MAX_FAKE_THREADS ||
        !fake_threads[thread_id].used)
        return -1;

    delete_thread_calls++;
    record_event(EVENT_DELETE_THREAD);
    pthread_mutex_lock(&thread_mutex);
    handle = fake_threads[thread_id].handle;
    started = fake_threads[thread_id].started;
    pthread_mutex_unlock(&thread_mutex);

    if (started && !pthread_equal(pthread_self(), handle))
        pthread_join(handle, NULL);

    pthread_mutex_lock(&thread_mutex);
    memset(&fake_threads[thread_id], 0, sizeof(fake_threads[thread_id]));
    pthread_mutex_unlock(&thread_mutex);
    return 0;
}

void ExitThread(void)
{
    /* The host wrapper marks the thread dormant after the product entry returns. */
    CHECK(current_fake_thread_id > 0);
}

typedef struct fake_rx_frame {
    pstvnc_transport_header_t header;
    uint8_t payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];
} fake_rx_frame_t;

static fake_rx_frame_t rx_frames[MAX_RX_FRAMES];
static size_t rx_frame_count;
static size_t rx_frame_index;
static int receive_calls;
static int receive_active;
static int max_receive_active;
static pthread_t first_receive_thread;
static int first_receive_thread_valid;
static int receive_thread_mismatch;
static int receive_shutdown;
static pthread_mutex_t rx_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t rx_condition = PTHREAD_COND_INITIALIZER;
static pthread_cond_t rx_progress_condition = PTHREAD_COND_INITIALIZER;

typedef struct fake_send_record {
    uint8_t kind;
    uint8_t channel;
    uint8_t flags;
    size_t payload_length;
    uint8_t payload[32];
} fake_send_record_t;

static fake_send_record_t send_records[MAX_SEND_RECORDS];
static size_t send_record_count;
static int send_calls;
static int send_fail_on_call;
static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;

static int physical_adopt_calls;
static int physical_adopt_fail;
static int physical_release_calls;
static int physical_shutdown_calls;
static int physical_shutdown_result = 1;

static void reset_fake_physical(void)
{
    pthread_mutex_lock(&rx_mutex);
    memset(rx_frames, 0, sizeof(rx_frames));
    rx_frame_count = 0u;
    rx_frame_index = 0u;
    receive_calls = 0;
    receive_active = 0;
    max_receive_active = 0;
    first_receive_thread_valid = 0;
    receive_thread_mismatch = 0;
    receive_shutdown = 0;
    pthread_mutex_unlock(&rx_mutex);

    pthread_mutex_lock(&send_mutex);
    memset(send_records, 0, sizeof(send_records));
    send_record_count = 0u;
    send_calls = 0;
    send_fail_on_call = 0;
    pthread_mutex_unlock(&send_mutex);

    physical_adopt_calls = 0;
    physical_adopt_fail = 0;
    physical_release_calls = 0;
    physical_shutdown_calls = 0;
    physical_shutdown_result = 1;
}

static void reset_fixture(void)
{
    destroy_unused_host_state();
    create_sema_calls = 0;
    create_sema_fail_on_call = 0;
    receiver_done_semaphore_id = -1;
    outbound_slot_semaphore_id = -1;
    outbound_ready_semaphore_id = -1;
    outbound_done_semaphore_id = -1;
    outbound_submitter_drain_semaphore_id = -1;
    rfb_outbound_credit_wait_semaphore_id = -1;
    audio_activity_wait_semaphore_id = -1;
    mpeg_activity_wait_semaphore_id = -1;

    pthread_mutex_lock(&completion_fence_mutex);
    receiver_done_signal_blocked = 0;
    receiver_done_signal_entered = 0;
    receiver_done_wait_calls = 0;
    outbound_submitter_drain_wait_calls = 0;
    outbound_ready_signal_blocked = 0;
    outbound_ready_signal_entered = 0;
    outbound_slot_signal_blocked = 0;
    outbound_slot_signal_entered = 0;
    wait_readable_blocked = 0;
    wait_readable_entered = 0;
    pthread_mutex_unlock(&completion_fence_mutex);

    pthread_mutex_lock(&rfb_credit_wait_mutex);
    rfb_outbound_credit_wait_calls = 0;
    pthread_mutex_unlock(&rfb_credit_wait_mutex);

    pthread_mutex_lock(&media_wait_mutex);
    audio_activity_wait_calls = 0;
    mpeg_activity_wait_calls = 0;
    pthread_mutex_unlock(&media_wait_mutex);
    create_thread_calls = 0;
    create_thread_fail = 0;
    start_thread_fail = 0;
    delete_thread_calls = 0;
    terminate_thread_calls = 0;
    fail_first_refer = 0;
    failed_refer_consumed = 0;
    force_first_refer_running = 0;
    forced_refer_consumed = 0;

    pthread_mutex_lock(&event_mutex);
    memset(lifecycle_events, 0, sizeof(lifecycle_events));
    lifecycle_event_count = 0u;
    pthread_mutex_unlock(&event_mutex);

    reset_fake_physical();
}

int pstvnc_transport_physical_stream_adopt(
    pstvnc_transport_physical_stream_t *stream,
    int socket_fd)
{
    physical_adopt_calls++;
    if (physical_adopt_fail || stream == NULL || socket_fd < 0)
        return 0;

    stream->socket_fd = socket_fd;
    stream->send_semaphore_id = 77;
    stream->next_send_sequence = 1u;
    stream->expected_receive_sequence = 1u;
    return 1;
}

int pstvnc_transport_physical_stream_transfer_established(
    pstvnc_transport_physical_stream_t *destination,
    pstvnc_transport_physical_stream_t *source)
{
    if (destination == NULL || source == NULL ||
        source->socket_fd < 0 || source->send_semaphore_id < 0 ||
        source->next_send_sequence != 2u ||
        source->expected_receive_sequence != 2u)
        return 0;

    *destination = *source;
    source->socket_fd = -1;
    source->send_semaphore_id = -1;
    source->next_send_sequence = 1u;
    source->expected_receive_sequence = 1u;
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
    send_calls++;
    if (send_fail_on_call > 0 && send_calls == send_fail_on_call) {
        pthread_mutex_unlock(&send_mutex);
        return 0;
    }

    CHECK(send_record_count < MAX_SEND_RECORDS);
    if (send_record_count >= MAX_SEND_RECORDS) {
        pthread_mutex_unlock(&send_mutex);
        return 0;
    }

    record = &send_records[send_record_count++];
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
    fake_rx_frame_t frame;

    CHECK(stream != NULL);
    pthread_mutex_lock(&rx_mutex);
    receive_calls++;
    receive_active++;
    if (receive_active > max_receive_active)
        max_receive_active = receive_active;
    if (!first_receive_thread_valid) {
        first_receive_thread = pthread_self();
        first_receive_thread_valid = 1;
    } else if (!pthread_equal(first_receive_thread, pthread_self())) {
        receive_thread_mismatch = 1;
    }
    pthread_cond_broadcast(&rx_progress_condition);

    while (rx_frame_index >= rx_frame_count && !receive_shutdown)
        pthread_cond_wait(&rx_condition, &rx_mutex);

    if (receive_shutdown) {
        receive_active--;
        pthread_mutex_unlock(&rx_mutex);
        return 0;
    }

    frame = rx_frames[rx_frame_index++];
    receive_active--;
    pthread_mutex_unlock(&rx_mutex);

    CHECK(frame.header.payload_length <= payload_capacity);
    if (frame.header.payload_length > payload_capacity)
        return 0;

    *header = frame.header;
    if (frame.header.payload_length != 0u)
        memcpy(payload, frame.payload, frame.header.payload_length);
    return 1;
}


int pstvnc_transport_physical_stream_wait_readable(
    pstvnc_transport_physical_stream_t *stream,
    uint32_t timeout_us)
{
    int readable;

    (void)timeout_us;
    CHECK(stream != NULL);

    /*
     * Host-only R20C rendezvous: when armed, hold the sole I/O owner inside
     * this readiness pass before it samples inbound state. Tests can then make
     * outbound work pending and queue a terminal inbound frame without allowing
     * the owner to loop back and service outbound first.
     */
    pthread_mutex_lock(&completion_fence_mutex);
    if (wait_readable_blocked) {
        wait_readable_entered = 1;
        pthread_cond_broadcast(&completion_fence_condition);
        while (wait_readable_blocked)
            pthread_cond_wait(
                &completion_fence_condition,
                &completion_fence_mutex);
    }
    pthread_mutex_unlock(&completion_fence_mutex);

    pthread_mutex_lock(&rx_mutex);
    readable =
        rx_frame_index < rx_frame_count ||
        receive_shutdown;
    pthread_mutex_unlock(&rx_mutex);

    return readable ? 1 : 0;
}

int pstvnc_transport_physical_stream_shutdown_io(
    pstvnc_transport_physical_stream_t *stream)
{
    CHECK(stream != NULL);
    physical_shutdown_calls++;
    record_event(EVENT_SHUTDOWN_IO);
    if (!physical_shutdown_result)
        return 0;

    pthread_mutex_lock(&rx_mutex);
    receive_shutdown = 1;
    pthread_cond_broadcast(&rx_condition);
    pthread_mutex_unlock(&rx_mutex);
    return 1;
}

void pstvnc_transport_physical_stream_release(
    pstvnc_transport_physical_stream_t *stream)
{
    physical_release_calls++;
    record_event(EVENT_PHYSICAL_RELEASE);
    if (stream != NULL) {
        stream->socket_fd = -1;
        stream->send_semaphore_id = -1;
        stream->next_send_sequence = 1u;
        stream->expected_receive_sequence = 1u;
    }
}

static pstvnc_transport_session_config_t make_config(void)
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

static pstvnc_transport_audio_channel_config_t make_audio_config(void)
{
    pstvnc_transport_audio_channel_config_t config;

    memset(&config, 0, sizeof(config));
    config.queue_capacity = 16u;
    config.initial_credit_bytes = 8u;
    config.credit_batch_bytes = 4u;
    config.credit_flush_on_empty = 1;
    config.credit_return_enabled = 1;
    return config;
}

static pstvnc_transport_mpeg_channel_config_t make_mpeg_config(void)
{
    pstvnc_transport_mpeg_channel_config_t config;

    memset(&config, 0, sizeof(config));
    config.queue_capacity = 16u;
    config.initial_credit_bytes = 8u;
    config.credit_batch_bytes = 4u;
    config.credit_flush_on_empty = 1;
    config.credit_return_enabled = 1;
    return config;
}

static void push_rx_frame(
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const uint8_t *payload,
    size_t payload_length)
{
    fake_rx_frame_t *frame;

    pthread_mutex_lock(&rx_mutex);
    CHECK(rx_frame_count < MAX_RX_FRAMES);
    if (rx_frame_count >= MAX_RX_FRAMES) {
        pthread_mutex_unlock(&rx_mutex);
        return;
    }

    frame = &rx_frames[rx_frame_count++];
    memset(frame, 0, sizeof(*frame));
    frame->header.version = PSTVNC_TRANSPORT_VERSION;
    frame->header.kind = kind;
    frame->header.channel = channel;
    frame->header.flags = flags;
    frame->header.sequence = (uint32_t)rx_frame_count;
    frame->header.payload_length = (uint32_t)payload_length;
    if (payload_length != 0u)
        memcpy(frame->payload, payload, payload_length);

    pthread_cond_broadcast(&rx_condition);
    pthread_mutex_unlock(&rx_mutex);
}

static void wait_for_receive_calls(int target)
{
    pthread_mutex_lock(&rx_mutex);
    while (receive_calls < target)
        pthread_cond_wait(&rx_progress_condition, &rx_mutex);
    pthread_mutex_unlock(&rx_mutex);
}

static void clear_send_records(void)
{
    pthread_mutex_lock(&send_mutex);
    memset(send_records, 0, sizeof(send_records));
    send_record_count = 0u;
    send_calls = 0;
    send_fail_on_call = 0;
    pthread_mutex_unlock(&send_mutex);
}

static void wait_for_rfb_credit_wait_calls(int target)
{
    pthread_mutex_lock(&rfb_credit_wait_mutex);
    while (rfb_outbound_credit_wait_calls < target)
        pthread_cond_wait(
            &rfb_credit_wait_condition,
            &rfb_credit_wait_mutex);
    pthread_mutex_unlock(&rfb_credit_wait_mutex);
}

static void wait_for_media_wait_calls(int audio_target, int mpeg_target)
{
    pthread_mutex_lock(&media_wait_mutex);
    while (audio_activity_wait_calls < audio_target ||
           mpeg_activity_wait_calls < mpeg_target)
        pthread_cond_wait(&media_wait_condition, &media_wait_mutex);
    pthread_mutex_unlock(&media_wait_mutex);
}

static void wait_for_receiver_done_signal_barrier(void)
{
    pthread_mutex_lock(&completion_fence_mutex);
    while (!receiver_done_signal_entered)
        pthread_cond_wait(
            &completion_fence_condition,
            &completion_fence_mutex);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void wait_for_receiver_done_wait_calls(int target)
{
    pthread_mutex_lock(&completion_fence_mutex);
    while (receiver_done_wait_calls < target)
        pthread_cond_wait(
            &completion_fence_condition,
            &completion_fence_mutex);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void wait_for_outbound_ready_signal_barrier(void)
{
    pthread_mutex_lock(&completion_fence_mutex);
    while (!outbound_ready_signal_entered)
        pthread_cond_wait(
            &completion_fence_condition,
            &completion_fence_mutex);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void wait_for_outbound_slot_signal_barrier(void)
{
    pthread_mutex_lock(&completion_fence_mutex);
    while (!outbound_slot_signal_entered)
        pthread_cond_wait(
            &completion_fence_condition,
            &completion_fence_mutex);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void wait_for_outbound_submitter_drain_wait_calls(int target)
{
    pthread_mutex_lock(&completion_fence_mutex);
    while (outbound_submitter_drain_wait_calls < target)
        pthread_cond_wait(
            &completion_fence_condition,
            &completion_fence_mutex);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void wait_for_semaphore_waiters(int semaphore_id, int target)
{
    fake_semaphore_t *semaphore = &fake_semaphores[semaphore_id];

    CHECK(semaphore_id > 0 && semaphore_id < MAX_FAKE_SEMAS);
    pthread_mutex_lock(&semaphore->mutex);
    while (semaphore->waiters < target)
        pthread_cond_wait(&semaphore->condition, &semaphore->mutex);
    pthread_mutex_unlock(&semaphore->mutex);
}

static int semaphore_count(int semaphore_id)
{
    fake_semaphore_t *semaphore = &fake_semaphores[semaphore_id];
    int count;

    CHECK(semaphore_id > 0 && semaphore_id < MAX_FAKE_SEMAS);
    pthread_mutex_lock(&semaphore->mutex);
    count = semaphore->count;
    pthread_mutex_unlock(&semaphore->mutex);
    return count;
}

static int semaphore_waiters(int semaphore_id)
{
    fake_semaphore_t *semaphore = &fake_semaphores[semaphore_id];
    int waiters;

    CHECK(semaphore_id > 0 && semaphore_id < MAX_FAKE_SEMAS);
    pthread_mutex_lock(&semaphore->mutex);
    waiters = semaphore->waiters;
    pthread_mutex_unlock(&semaphore->mutex);
    return waiters;
}

static void wait_for_wait_readable_barrier(void)
{
    pthread_mutex_lock(&completion_fence_mutex);
    while (!wait_readable_entered)
        pthread_cond_wait(
            &completion_fence_condition,
            &completion_fence_mutex);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void set_wait_readable_blocked(int blocked)
{
    pthread_mutex_lock(&completion_fence_mutex);
    wait_readable_blocked = blocked;
    if (!blocked)
        pthread_cond_broadcast(&completion_fence_condition);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void set_receiver_done_signal_blocked(int blocked)
{
    pthread_mutex_lock(&completion_fence_mutex);
    receiver_done_signal_blocked = blocked;
    if (!blocked)
        pthread_cond_broadcast(&completion_fence_condition);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void set_outbound_ready_signal_blocked(int blocked)
{
    pthread_mutex_lock(&completion_fence_mutex);
    outbound_ready_signal_blocked = blocked;
    if (!blocked)
        pthread_cond_broadcast(&completion_fence_condition);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static void set_outbound_slot_signal_blocked(int blocked)
{
    pthread_mutex_lock(&completion_fence_mutex);
    outbound_slot_signal_blocked = blocked;
    if (!blocked)
        pthread_cond_broadcast(&completion_fence_condition);
    pthread_mutex_unlock(&completion_fence_mutex);
}

static uint32_t credit_record_amount(size_t index)
{
    CHECK(index < send_record_count);
    CHECK(send_records[index].payload_length ==
        PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE);
    return pstvnc_transport_read_be32(send_records[index].payload);
}

static void capture_runtime_test_semaphores(
    const pstvnc_transport_runtime_t *runtime)
{
    receiver_done_semaphore_id = runtime->receiver_done_semaphore_id;
    outbound_slot_semaphore_id = runtime->outbound_slot_semaphore_id;
    outbound_ready_semaphore_id = runtime->outbound_ready_semaphore_id;
    outbound_done_semaphore_id = runtime->outbound_done_semaphore_id;
    outbound_submitter_drain_semaphore_id =
        runtime->outbound_submitter_drain_semaphore_id;
    rfb_outbound_credit_wait_semaphore_id =
        runtime->rfb_outbound_credit_semaphore_id;
    audio_activity_wait_semaphore_id = runtime->audio_activity_semaphore_id;
    mpeg_activity_wait_semaphore_id = runtime->mpeg_activity_semaphore_id;
}

static void initialize_runtime(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_session_config_t *config)
{
    memset(runtime, 0xa5, sizeof(*runtime));
    CHECK(pstvnc_transport_runtime_initialize(runtime, 91, config) == 1);
    capture_runtime_test_semaphores(runtime);
    CHECK(runtime->initialized == 1);
}

static void initialize_media_runtime(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio,
    const pstvnc_transport_mpeg_channel_config_t *mpeg)
{
    memset(runtime, 0xa5, sizeof(*runtime));
    CHECK(pstvnc_transport_runtime_initialize_with_audio_mpeg(
        runtime, 91, config, audio, mpeg) == 1);
    capture_runtime_test_semaphores(runtime);
    CHECK(runtime->initialized == 1);
    CHECK(runtime->audio_enabled == 1);
    CHECK(runtime->mpeg_enabled == 1);
}

static void start_runtime(pstvnc_transport_runtime_t *runtime)
{
    CHECK(pstvnc_transport_runtime_start_receiver(runtime) == 1);
    CHECK(runtime->receiver_thread_started == 1);

    /*
     * The single physical-I/O owner performs a readiness poll before receive.
     * With no queued inbound frame, startup must not call receive_frame().
     */
    CHECK(receive_calls == 0);
}

static void stop_and_release_runtime(pstvnc_transport_runtime_t *runtime)
{
    if (runtime->receiver_thread_started && !runtime->receiver_done) {
        CHECK(pstvnc_transport_runtime_request_stop(runtime) == 1);
        CHECK(pstvnc_transport_runtime_wait_receiver_done(runtime) == 1);
    }
    CHECK(pstvnc_transport_runtime_release(runtime) == 1);
}


static void test_established_lineage_initialization_preserves_sequence(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_physical_stream_t established_stream;
    pstvnc_transport_session_config_t config = make_config();

    reset_fixture();
    memset(&established_stream, 0, sizeof(established_stream));
    established_stream.socket_fd = 93;
    established_stream.send_semaphore_id = 77;
    established_stream.next_send_sequence = 2u;
    established_stream.expected_receive_sequence = 2u;

    CHECK(pstvnc_transport_runtime_initialize_established(
        &runtime, &established_stream, &config) == 1);
    CHECK(physical_adopt_calls == 0);
    CHECK(established_stream.socket_fd == -1);
    CHECK(established_stream.send_semaphore_id == -1);
    CHECK(established_stream.next_send_sequence == 1u);
    CHECK(established_stream.expected_receive_sequence == 1u);
    CHECK(runtime.physical_stream.socket_fd == 93);
    CHECK(runtime.physical_stream.send_semaphore_id == 77);
    CHECK(runtime.physical_stream.next_send_sequence == 2u);
    CHECK(runtime.physical_stream.expected_receive_sequence == 2u);

    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
    CHECK(physical_release_calls == 1);
}

static void test_initialize_and_start_failure_ownership(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();

    reset_fixture();
    config.max_data_payload = config.rfb_queue_capacity + 1u;
    CHECK(pstvnc_transport_runtime_initialize(&runtime, 91, &config) == 0);
    CHECK(physical_adopt_calls == 0);

    reset_fixture();
    config = make_config();
    create_sema_fail_on_call = 2;
    CHECK(pstvnc_transport_runtime_initialize(&runtime, 91, &config) == 0);
    CHECK(physical_adopt_calls == 0);
    CHECK(runtime.initialized == 0);
    CHECK(runtime.rfb_queue_storage == NULL);
    CHECK(runtime.receiver_stack_allocation == NULL);

    reset_fixture();
    physical_adopt_fail = 1;
    CHECK(pstvnc_transport_runtime_initialize(&runtime, 91, &config) == 0);
    CHECK(physical_adopt_calls == 1);
    CHECK(physical_release_calls == 0);
    CHECK(runtime.initialized == 0);

    /*
     * The single-physical-I/O-owner runtime starts its owner before startup
     * credits are submitted. A startup CREDIT send failure therefore retires
     * an already-started owner rather than failing before thread creation.
     */
    reset_fixture();
    initialize_runtime(&runtime, &config);
    send_fail_on_call = 1;
    CHECK(pstvnc_transport_runtime_start_receiver(&runtime) == 0);
    CHECK(runtime.failed == 1);
    CHECK(runtime.receiver_thread_started == 1);
    CHECK(runtime.receiver_done == 1);
    CHECK(create_thread_calls == 1);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
    CHECK(runtime.receiver_thread_started == 0);
    CHECK(delete_thread_calls == 1);

    reset_fixture();
    initialize_runtime(&runtime, &config);
    create_thread_fail = 1;
    CHECK(pstvnc_transport_runtime_start_receiver(&runtime) == 0);
    CHECK(runtime.failed == 1);
    CHECK(runtime.receiver_thread_started == 0);
    CHECK(create_thread_calls == 1);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_thread_fail = 1;
    CHECK(pstvnc_transport_runtime_start_receiver(&runtime) == 0);
    CHECK(runtime.failed == 1);
    CHECK(runtime.receiver_thread_started == 0);
    CHECK(delete_thread_calls == 1);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_sole_receiver_dispatch_and_activity(void)
{
    static const uint8_t payload[] = { 1, 2, 3 };
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    uint32_t before;
    uint32_t observed;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    CHECK(pstvnc_transport_runtime_rfb_activity_snapshot(
        &runtime, &before) == 1);
    observed = before;

    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        payload,
        sizeof(payload));
    wait_for_receive_calls(1);

    CHECK(max_receive_active == 1);
    CHECK(receive_thread_mismatch == 0);
    CHECK(pstvnc_transport_rfb_channel_available(&runtime.rfb_channel) ==
        sizeof(payload));
    CHECK(runtime.activity_sequence != before);
    CHECK(pstvnc_transport_runtime_rfb_wait_activity(
        &runtime, &observed) == 1);
    CHECK(observed == runtime.activity_sequence);
    CHECK(pstvnc_transport_runtime_rfb_poll_receive(&runtime) == 1);

    stop_and_release_runtime(&runtime);
}

static void test_invalid_frame_converges_fail_closed(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);

    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_CREDIT,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        NULL,
        0u);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);
    CHECK(runtime.receiver_done == 1);
    CHECK(receive_calls == 1);
    CHECK(pstvnc_transport_rfb_channel_available(&runtime.rfb_channel) == 0u);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);

    /* Exact CREDIT framing with a zero amount is also invalid and terminal. */
    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    {
        uint8_t zero_credit[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE] = {
            0u, 0u, 0u, 0u
        };
        push_rx_frame(
            PSTVNC_TRANSPORT_FRAME_CREDIT,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            0u,
            zero_credit,
            sizeof(zero_credit));
    }
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);
    CHECK(runtime.receiver_done == 1);
    CHECK(runtime.rfb_outbound_credit_bytes == 0u);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO,
        0u,
        NULL,
        0u);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);
    CHECK(receive_calls == 1);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_parser_credit_batch_flush_and_residual_distinction(void)
{
    static const uint8_t first[] = { 1, 2, 3, 4 };
    static const uint8_t second[] = { 5, 6 };
    static const uint8_t residual[] = { 7, 8, 9 };
    uint8_t output[2];
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    size_t residual_count;
    size_t discarded;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        first,
        sizeof(first));
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        second,
        sizeof(second));
    wait_for_receive_calls(2);

    CHECK(pstvnc_transport_runtime_rfb_read_exact(
        &runtime, output, sizeof(output)) == 1);
    CHECK(runtime.rfb_credit_pending == 2u);
    CHECK(send_record_count == 0u);

    CHECK(pstvnc_transport_runtime_rfb_read_exact(
        &runtime, output, sizeof(output)) == 1);
    CHECK(runtime.rfb_credit_pending == 0u);
    CHECK(send_record_count == 1u);
    CHECK(send_records[0].kind == PSTVNC_TRANSPORT_FRAME_CREDIT);
    CHECK(credit_record_amount(0u) == 4u);

    CHECK(pstvnc_transport_runtime_rfb_read_exact(
        &runtime, output, sizeof(output)) == 1);
    CHECK(runtime.rfb_credit_pending == 0u);
    CHECK(send_record_count == 2u);
    CHECK(credit_record_amount(1u) == 2u);

    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        residual,
        sizeof(residual));
    wait_for_receive_calls(3);

    CHECK(pstvnc_transport_runtime_rfb_read_exact(&runtime, output, 1u) == 1);
    CHECK(runtime.rfb_credit_pending == 1u);
    CHECK(send_record_count == 2u);

    runtime.rfb_quiesce_commit_received = 1u;
    CHECK(pstvnc_transport_runtime_rfb_snapshot_residual(
        &runtime, &residual_count) == 1);
    CHECK(residual_count == 2u);
    CHECK(pstvnc_transport_runtime_rfb_discard_quiesce_residual(
        &runtime, residual_count, &discarded) == 1);
    CHECK(discarded == 2u);
    CHECK(runtime.rfb_credit_pending == 1u);
    CHECK(send_record_count == 2u);

    stop_and_release_runtime(&runtime);
}


typedef struct rfb_writer_test_context {
    pstvnc_transport_runtime_t *runtime;
    const uint8_t *payload;
    size_t payload_length;
    int result;
} rfb_writer_test_context_t;

typedef struct runtime_call_test_context {
    pstvnc_transport_runtime_t *runtime;
    int result;
} runtime_call_test_context_t;

typedef struct media_waiter_test_context {
    pstvnc_transport_runtime_t *runtime;
    uint32_t activity_sequence;
    int use_mpeg;
    int result;
} media_waiter_test_context_t;

static void *rfb_writer_test_thread(void *opaque)
{
    rfb_writer_test_context_t *context =
        (rfb_writer_test_context_t *)opaque;

    context->result = pstvnc_transport_runtime_rfb_write_exact(
        context->runtime,
        context->payload,
        context->payload_length);
    return NULL;
}

static void *outbound_submit_test_thread(void *opaque)
{
    static const uint8_t payload[] = { 0x5au };
    runtime_call_test_context_t *context =
        (runtime_call_test_context_t *)opaque;

    context->result = pstvnc_transport_runtime_submit_frame(
        context->runtime,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        payload,
        sizeof(payload));
    return NULL;
}

static void *receiver_done_wait_test_thread(void *opaque)
{
    runtime_call_test_context_t *context =
        (runtime_call_test_context_t *)opaque;

    context->result =
        pstvnc_transport_runtime_wait_receiver_done(context->runtime);
    return NULL;
}

static void *runtime_release_test_thread(void *opaque)
{
    runtime_call_test_context_t *context =
        (runtime_call_test_context_t *)opaque;

    context->result = pstvnc_transport_runtime_release(context->runtime);
    return NULL;
}

static void *media_waiter_test_thread(void *opaque)
{
    media_waiter_test_context_t *context =
        (media_waiter_test_context_t *)opaque;

    if (context->use_mpeg) {
        context->result = pstvnc_transport_runtime_mpeg_wait_activity(
            context->runtime,
            &context->activity_sequence);
    } else {
        context->result = pstvnc_transport_runtime_audio_wait_activity(
            context->runtime,
            &context->activity_sequence);
    }

    return NULL;
}

static void push_rfb_credit(uint32_t amount)
{
    uint8_t payload[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE];

    pstvnc_transport_write_be32(payload, amount);
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_CREDIT,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        payload,
        sizeof(payload));
}

static void test_outbound_fragmentation_and_failure_propagation(void)
{
    static const uint8_t payload[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    push_rfb_credit((uint32_t)sizeof(payload));
    wait_for_receive_calls(1);
    CHECK(pstvnc_transport_runtime_rfb_write_exact(
        &runtime, payload, sizeof(payload)) == 1);
    CHECK(send_record_count == 3u);
    CHECK(send_records[0].kind == PSTVNC_TRANSPORT_FRAME_DATA);
    CHECK(send_records[0].channel == PSTVNC_TRANSPORT_CHANNEL_RFB);
    CHECK(send_records[0].payload_length == 4u);
    CHECK(send_records[1].payload_length == 4u);
    CHECK(send_records[2].payload_length == 2u);
    CHECK(memcmp(send_records[0].payload, payload, 4u) == 0);
    CHECK(memcmp(send_records[1].payload, payload + 4u, 4u) == 0);
    CHECK(memcmp(send_records[2].payload, payload + 8u, 2u) == 0);
    CHECK(runtime.rfb_outbound_credit_bytes == 0u);
    stop_and_release_runtime(&runtime);

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    push_rfb_credit((uint32_t)sizeof(payload));
    wait_for_receive_calls(1);
    send_fail_on_call = send_calls + 2;
    CHECK(pstvnc_transport_runtime_rfb_write_exact(
        &runtime, payload, sizeof(payload)) == 0);
    CHECK(runtime.failed == 1);
    CHECK(send_record_count == 1u);
    stop_and_release_runtime(&runtime);
}

static void test_outbound_rfb_waits_for_partial_pi_credit(void)
{
    static const uint8_t payload[] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 };
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    rfb_writer_test_context_t writer;
    pthread_t writer_thread;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    memset(&writer, 0, sizeof(writer));
    writer.runtime = &runtime;
    writer.payload = payload;
    writer.payload_length = sizeof(payload);
    writer.result = -1;

    CHECK(pthread_create(
        &writer_thread, NULL, rfb_writer_test_thread, &writer) == 0);

    /* No Pi CREDIT means absolutely no outbound channel-1 DATA. */
    wait_for_rfb_credit_wait_calls(1);
    CHECK(send_record_count == 0u);

    push_rfb_credit(3u);
    wait_for_receive_calls(1);
    wait_for_rfb_credit_wait_calls(2);
    CHECK(send_record_count == 1u);
    CHECK(send_records[0].payload_length == 3u);
    CHECK(memcmp(send_records[0].payload, payload, 3u) == 0);

    push_rfb_credit(2u);
    wait_for_receive_calls(2);
    wait_for_rfb_credit_wait_calls(3);
    CHECK(send_record_count == 2u);
    CHECK(send_records[1].payload_length == 2u);
    CHECK(memcmp(send_records[1].payload, payload + 3u, 2u) == 0);

    push_rfb_credit(1u);
    wait_for_receive_calls(3);
    CHECK(pthread_join(writer_thread, NULL) == 0);
    CHECK(writer.result == 1);
    CHECK(send_record_count == 3u);
    CHECK(send_records[2].payload_length == 1u);
    CHECK(send_records[2].payload[0] == payload[5]);
    CHECK(runtime.rfb_outbound_credit_bytes == 0u);

    stop_and_release_runtime(&runtime);
}


static void test_finite_quiesce_order_is_distinct_from_fatal_abort(void)
{
    static const uint8_t residual[] = { 0xaa, 0xbb };
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    size_t residual_count;
    size_t discarded;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    /* Pi REQUEST: zero-length RFB DATA accepted by the sole receiver. */
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        NULL,
        0u);
    wait_for_receive_calls(1);
    CHECK(pstvnc_transport_runtime_rfb_quiesce_requested(&runtime) == 1);
    CHECK(runtime.stop_requested == 0);
    CHECK(physical_shutdown_calls == 0);

    /* External RFB owner selects a complete-message safe boundary. */
    CHECK(pstvnc_transport_runtime_rfb_send_quiesce_boundary(&runtime) == 1);
    CHECK(runtime.rfb_quiesce_boundary_sent == 1u);
    CHECK(send_record_count == 1u);
    CHECK(send_records[0].kind == PSTVNC_TRANSPORT_FRAME_DATA);
    CHECK(send_records[0].payload_length == 0u);

    /* Bytes already in flight after BOUNDARY become exact terminal residual. */
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        residual,
        sizeof(residual));
    wait_for_receive_calls(2);

    /* Pi COMMIT: next zero-length RFB DATA is accepted only after BOUNDARY. */
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        NULL,
        0u);
    wait_for_receive_calls(3);
    CHECK(pstvnc_transport_runtime_rfb_wait_quiesce_commit(&runtime) == 1);
    CHECK(runtime.rfb_quiesce_commit_received == 1u);

    CHECK(pstvnc_transport_runtime_rfb_snapshot_residual(
        &runtime, &residual_count) == 1);
    CHECK(residual_count == sizeof(residual));
    CHECK(pstvnc_transport_runtime_rfb_send_quiesce_complete(&runtime) == 0);

    CHECK(pstvnc_transport_runtime_rfb_discard_quiesce_residual(
        &runtime, residual_count, &discarded) == 1);
    CHECK(discarded == sizeof(residual));
    CHECK(runtime.rfb_credit_pending == 0u);
    CHECK(send_record_count == 1u); /* discard earns no CREDIT */

    CHECK(pstvnc_transport_runtime_rfb_send_quiesce_complete(&runtime) == 1);
    CHECK(runtime.rfb_quiesce_complete_sent == 1u);
    CHECK(send_record_count == 2u);
    CHECK(send_records[1].kind == PSTVNC_TRANSPORT_FRAME_DATA);
    CHECK(send_records[1].payload_length == 0u);

    /* Finite quiesce itself never requested fatal socket interruption. */
    CHECK(runtime.stop_requested == 0);
    CHECK(physical_shutdown_calls == 0);
    CHECK(runtime.receiver_done == 0);

    /* Test-only cleanup invokes the separate fatal-stop mechanism afterward. */
    stop_and_release_runtime(&runtime);
    CHECK(physical_shutdown_calls == 1);
}

static void test_receiver_completion_event_is_real_no_touch_fence(void)
{
    static const uint8_t writer_payload[] = { 0xa5u };
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_transport_audio_channel_config_t audio = make_audio_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    rfb_writer_test_context_t rfb_writer;
    media_waiter_test_context_t audio_waiter;
    media_waiter_test_context_t mpeg_waiter;
    runtime_call_test_context_t outbound_submit;
    runtime_call_test_context_t completion_wait;
    pthread_t rfb_writer_thread;
    pthread_t audio_waiter_thread;
    pthread_t mpeg_waiter_thread;
    pthread_t outbound_submit_thread;
    pthread_t completion_wait_thread;

    reset_fixture();
    config.rfb_initial_credit_bytes = 0u;
    initialize_media_runtime(&runtime, &config, &audio, &mpeg);
    start_runtime(&runtime);

    memset(&audio_waiter, 0, sizeof(audio_waiter));
    audio_waiter.runtime = &runtime;
    CHECK(pstvnc_transport_runtime_audio_activity_snapshot(
        &runtime, &audio_waiter.activity_sequence) == 1);

    memset(&mpeg_waiter, 0, sizeof(mpeg_waiter));
    mpeg_waiter.runtime = &runtime;
    mpeg_waiter.use_mpeg = 1;
    CHECK(pstvnc_transport_runtime_mpeg_activity_snapshot(
        &runtime, &mpeg_waiter.activity_sequence) == 1);

    CHECK(pthread_create(
        &audio_waiter_thread, NULL, media_waiter_test_thread, &audio_waiter)
        == 0);
    CHECK(pthread_create(
        &mpeg_waiter_thread, NULL, media_waiter_test_thread, &mpeg_waiter)
        == 0);
    wait_for_media_wait_calls(1, 1);

    memset(&rfb_writer, 0, sizeof(rfb_writer));
    rfb_writer.runtime = &runtime;
    rfb_writer.payload = writer_payload;
    rfb_writer.payload_length = sizeof(writer_payload);
    rfb_writer.result = -1;
    CHECK(pthread_create(
        &rfb_writer_thread, NULL, rfb_writer_test_thread, &rfb_writer) == 0);
    wait_for_rfb_credit_wait_calls(1);

    /*
     * First hold the owner inside one readiness pass. That prevents the host
     * loop from seeing outbound_pending at its top and consuming/awaiting the
     * deliberately withheld ready token before the terminal frame is admitted.
     */
    set_wait_readable_blocked(1);
    wait_for_wait_readable_barrier();

    /*
     * Hold a direct Transport submitter after it has published outbound_pending
     * but before outbound-ready notification. The receiver must resolve that
     * admitted slot on terminal exit before publishing completion.
     */
    set_outbound_ready_signal_blocked(1);
    memset(&outbound_submit, 0, sizeof(outbound_submit));
    outbound_submit.runtime = &runtime;
    outbound_submit.result = -1;
    CHECK(pthread_create(
        &outbound_submit_thread,
        NULL,
        outbound_submit_test_thread,
        &outbound_submit) == 0);
    wait_for_outbound_ready_signal_barrier();
    CHECK(runtime.outbound_pending == 1);

    /*
     * Hold the I/O owner at the exact final completion publication. An invalid
     * frame closes admission and drives terminal dispatch without request_stop()
     * pre-waking the logical media owners.
     */
    set_receiver_done_signal_blocked(1);
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_CREDIT,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        NULL,
        0u);
    set_wait_readable_blocked(0);
    wait_for_receiver_done_signal_barrier();

    CHECK(runtime.receiver_done == 1);
    CHECK(runtime.failed == 1);
    CHECK(runtime.outbound_pending == 0);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) < 0);
    CHECK(terminate_thread_calls == 0);
    CHECK(delete_thread_calls == 0);
    CHECK(physical_release_calls == 0);

    /*
     * Every logical waiter must have been made terminal before the receiver is
     * allowed to publish reclaim completion.
     */
    CHECK(pthread_join(rfb_writer_thread, NULL) == 0);
    CHECK(rfb_writer.result == 0);
    CHECK(pthread_join(audio_waiter_thread, NULL) == 0);
    CHECK(audio_waiter.result == 1);
    CHECK(pthread_join(mpeg_waiter_thread, NULL) == 0);
    CHECK(mpeg_waiter.result == 1);
    CHECK(runtime.rfb_outbound_credit_wait_state == 0);
    CHECK(runtime.audio_activity_wait_armed == 0);
    CHECK(runtime.mpeg_activity_wait_armed == 0);

    /* The racing outbound submitter was failed before completion publication. */
    set_outbound_ready_signal_blocked(0);
    CHECK(pthread_join(outbound_submit_thread, NULL) == 0);
    CHECK(outbound_submit.result == 0);
    CHECK(runtime.outbound_pending == 0);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) < 0);

    /*
     * Even though early receiver_done is already visible, wait_receiver_done()
     * must block on the final ownership event.
     */
    memset(&completion_wait, 0, sizeof(completion_wait));
    completion_wait.runtime = &runtime;
    completion_wait.result = -1;
    CHECK(pthread_create(
        &completion_wait_thread,
        NULL,
        receiver_done_wait_test_thread,
        &completion_wait) == 0);
    wait_for_receiver_done_wait_calls(1);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) < 0);
    CHECK(runtime.rfb_queue_storage != NULL);
    CHECK(runtime.receiver_stack_allocation != NULL);
    CHECK(physical_release_calls == 0);

    set_receiver_done_signal_blocked(0);
    CHECK(pthread_join(completion_wait_thread, NULL) == 0);
    CHECK(completion_wait.result == 1);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) >= 0);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_outbound_submitter_drain_fences_active_and_queued_callers(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    runtime_call_test_context_t active_submit;
    runtime_call_test_context_t queued_submit;
    runtime_call_test_context_t release_call;
    pthread_t active_thread;
    pthread_t queued_thread;
    pthread_t release_thread;
    int ready_count_before_postterminal;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    /*
     * Hold the sole I/O owner inside readiness so one submitter can own the
     * outbound slot and a second can register then queue behind that slot before
     * terminality closes admission.
     */
    set_wait_readable_blocked(1);
    wait_for_wait_readable_barrier();

    memset(&active_submit, 0, sizeof(active_submit));
    active_submit.runtime = &runtime;
    active_submit.result = -1;
    CHECK(pthread_create(
        &active_thread, NULL, outbound_submit_test_thread, &active_submit) == 0);
    wait_for_semaphore_waiters(outbound_done_semaphore_id, 1);
    CHECK(runtime.outbound_pending == 1);
    CHECK(runtime.outbound_submitter_count == 1u);

    memset(&queued_submit, 0, sizeof(queued_submit));
    queued_submit.runtime = &runtime;
    queued_submit.result = -1;
    CHECK(pthread_create(
        &queued_thread, NULL, outbound_submit_test_thread, &queued_submit) == 0);
    wait_for_semaphore_waiters(outbound_slot_semaphore_id, 1);
    CHECK(runtime.outbound_submitter_count == 2u);

    /*
     * Hold the active caller after its terminal outbound-done wake at its final
     * slot release. This is precisely the R20C gap: done resolution is not yet
     * submitter completion.
     */
    set_outbound_slot_signal_blocked(1);
    set_receiver_done_signal_blocked(1);
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_CREDIT,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        NULL,
        0u);
    set_wait_readable_blocked(0);

    wait_for_outbound_slot_signal_barrier();
    wait_for_outbound_submitter_drain_wait_calls(1);

    CHECK(runtime.receiver_done == 1);
    CHECK(runtime.failed == 1);
    CHECK(runtime.outbound_pending == 0);
    CHECK(runtime.outbound_submitter_count == 2u);
    CHECK(runtime.outbound_submitter_drain_waiting == 1);
    CHECK(semaphore_waiters(outbound_slot_semaphore_id) == 1);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) < 0);

    /*
     * release() is allowed to observe early terminality but must remain blocked
     * on final receiver completion while either registered submitter can still
     * touch the outbound rendezvous.
     */
    memset(&release_call, 0, sizeof(release_call));
    release_call.runtime = &runtime;
    release_call.result = -1;
    force_first_refer_running = 1;
    CHECK(pthread_create(
        &release_thread, NULL, runtime_release_test_thread, &release_call) == 0);
    wait_for_receiver_done_wait_calls(1);
    CHECK(event_index(EVENT_REFER_THREAD) < 0);
    CHECK(delete_thread_calls == 0);
    CHECK(physical_release_calls == 0);
    CHECK(fake_semaphores[outbound_slot_semaphore_id].used);
    CHECK(fake_semaphores[outbound_ready_semaphore_id].used);
    CHECK(fake_semaphores[outbound_done_semaphore_id].used);
    CHECK(fake_semaphores[outbound_submitter_drain_semaphore_id].used);

    /*
     * Let the active caller make its final slot touch. The queued caller must
     * then acquire the slot, observe terminality, release it and unregister.
     * Only the second departure can reduce the drain set to zero.
     */
    set_outbound_slot_signal_blocked(0);
    wait_for_receiver_done_signal_barrier();

    CHECK(runtime.outbound_submitter_count == 0u);
    CHECK(runtime.outbound_submitter_drain_waiting == 0);
    CHECK(semaphore_waiters(outbound_slot_semaphore_id) == 0);
    CHECK(semaphore_count(outbound_slot_semaphore_id) == 1);

    CHECK(pthread_join(active_thread, NULL) == 0);
    CHECK(active_submit.result == 0);
    CHECK(pthread_join(queued_thread, NULL) == 0);
    CHECK(queued_submit.result == 0);

    /*
     * A caller beginning after early terminality is not registered and never
     * enters slot/ready/done ownership. Snapshot the ready token: the rejected
     * call must not consume or publish another outbound-ready event.
     */
    ready_count_before_postterminal =
        semaphore_count(outbound_ready_semaphore_id);
    CHECK(pstvnc_transport_runtime_submit_frame(
        &runtime,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        "x",
        1u) == 0);
    CHECK(runtime.outbound_submitter_count == 0u);
    CHECK(semaphore_waiters(outbound_slot_semaphore_id) == 0);
    CHECK(semaphore_count(outbound_slot_semaphore_id) == 1);
    CHECK(semaphore_count(outbound_ready_semaphore_id) ==
        ready_count_before_postterminal);

    set_receiver_done_signal_blocked(0);
    CHECK(pthread_join(release_thread, NULL) == 0);
    CHECK(release_call.result == 1);
    CHECK(terminate_thread_calls == 1);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) >= 0);
    CHECK(event_index(EVENT_TERMINATE_THREAD) >
        event_index(EVENT_RECEIVER_DONE_SIGNAL));
    CHECK(event_index(EVENT_DELETE_THREAD) >
        event_index(EVENT_RECEIVER_DONE_SIGNAL));
    CHECK(event_index(EVENT_PHYSICAL_RELEASE) >
        event_index(EVENT_DELETE_THREAD));
}

static void test_release_waits_for_receiver_completion_before_reclaim(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    runtime_call_test_context_t release_call;
    pthread_t release_thread;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);

    set_receiver_done_signal_blocked(1);
    CHECK(pstvnc_transport_runtime_request_stop(&runtime) == 1);
    wait_for_receiver_done_signal_barrier();
    CHECK(runtime.receiver_done == 1);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) < 0);

    /*
     * release() sees early terminality but must itself rendezvous on the final
     * completion event before it can inspect/force/delete the thread or reclaim
     * any Transport-owned storage.
     */
    force_first_refer_running = 1;
    memset(&release_call, 0, sizeof(release_call));
    release_call.runtime = &runtime;
    release_call.result = -1;
    CHECK(pthread_create(
        &release_thread, NULL, runtime_release_test_thread, &release_call)
        == 0);
    wait_for_receiver_done_wait_calls(1);

    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) < 0);
    CHECK(event_index(EVENT_REFER_THREAD) < 0);
    CHECK(terminate_thread_calls == 0);
    CHECK(delete_thread_calls == 0);
    CHECK(physical_release_calls == 0);
    CHECK(runtime.rfb_queue_storage != NULL);
    CHECK(runtime.receiver_stack_allocation != NULL);

    set_receiver_done_signal_blocked(0);
    CHECK(pthread_join(release_thread, NULL) == 0);
    CHECK(release_call.result == 1);
    CHECK(terminate_thread_calls == 1);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) >= 0);
    CHECK(event_index(EVENT_TERMINATE_THREAD) >
        event_index(EVENT_RECEIVER_DONE_SIGNAL));
    CHECK(event_index(EVENT_DELETE_THREAD) >
        event_index(EVENT_RECEIVER_DONE_SIGNAL));
    CHECK(event_index(EVENT_PHYSICAL_RELEASE) >
        event_index(EVENT_DELETE_THREAD));
}

static void test_fatal_stop_completion_precedes_reclaim_and_fresh_session(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    uint8_t *queue_before;
    void *stack_before;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    queue_before = runtime.rfb_queue_storage;
    stack_before = runtime.receiver_stack_allocation;

    CHECK(queue_before != NULL);
    CHECK(stack_before != NULL);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 0);
    CHECK(runtime.rfb_queue_storage == queue_before);
    CHECK(runtime.receiver_stack_allocation == stack_before);
    CHECK(physical_release_calls == 0);

    /*
     * Stop intent is independently visible to the readiness-polling I/O owner.
     * A failed socket shutdown therefore makes request_stop() report failure,
     * but it does not strand owner completion: the bounded readiness loop can
     * observe stop_requested and retire normally without another I/O wake.
     */
    physical_shutdown_result = 0;
    CHECK(pstvnc_transport_runtime_request_stop(&runtime) == 0);
    CHECK(runtime.stop_requested == 1);
    CHECK(physical_shutdown_calls == 1);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.receiver_done == 1);
    CHECK(runtime.failed == 0);
    CHECK(physical_release_calls == 0);

    /*
     * Once completion is visible, request_stop() is idempotently successful
     * and must not reissue physical socket shutdown.
     */
    physical_shutdown_result = 1;
    CHECK(pstvnc_transport_runtime_request_stop(&runtime) == 1);
    CHECK(physical_shutdown_calls == 1);

    /* A pre-reclaim kernel-status failure leaves Transport ownership retryable. */
    fail_first_refer = 1;
    CHECK(pstvnc_transport_runtime_release(&runtime) == 0);
    CHECK(runtime.initialized == 1);
    CHECK(runtime.rfb_queue_storage == queue_before);
    CHECK(runtime.receiver_stack_allocation == stack_before);
    CHECK(physical_release_calls == 0);

    fail_first_refer = 0;
    force_first_refer_running = 1;
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
    CHECK(terminate_thread_calls == 1);
    CHECK(runtime.initialized == 0);
    CHECK(runtime.rfb_queue_storage == NULL);
    CHECK(runtime.receiver_stack_allocation == NULL);
    CHECK(physical_release_calls == 1);
    CHECK(event_index(EVENT_RECEIVER_DONE_SIGNAL) >= 0);
    CHECK(event_index(EVENT_TERMINATE_THREAD) >
        event_index(EVENT_RECEIVER_DONE_SIGNAL));
    CHECK(event_index(EVENT_DELETE_THREAD) >
        event_index(EVENT_RECEIVER_DONE_SIGNAL));
    CHECK(event_index(EVENT_PHYSICAL_RELEASE) >
        event_index(EVENT_DELETE_THREAD));

    /* Reusing the runtime object starts from clean session authority. */
    pthread_mutex_lock(&rx_mutex);
    receive_shutdown = 0;
    pthread_mutex_unlock(&rx_mutex);
    CHECK(pstvnc_transport_runtime_initialize(&runtime, 92, &config) == 1);
    CHECK(runtime.stop_requested == 0);
    CHECK(runtime.receiver_done == 0);
    CHECK(runtime.failed == 0);
    CHECK(runtime.receiver_thread_started == 0);
    CHECK(runtime.receiver_thread_id == -1);
    CHECK(runtime.receiver_done_semaphore_id > 0);
    CHECK(fake_semaphores[runtime.receiver_done_semaphore_id].count == 0);
    CHECK(runtime.outbound_submitter_drain_semaphore_id > 0);
    CHECK(fake_semaphores[
        runtime.outbound_submitter_drain_semaphore_id].count == 0);
    CHECK(runtime.outbound_submitter_count == 0u);
    CHECK(runtime.outbound_submitter_drain_waiting == 0);
    CHECK(runtime.rfb_quiesce_request_received == 0u);
    CHECK(runtime.rfb_quiesce_boundary_sent == 0u);
    CHECK(runtime.rfb_quiesce_commit_received == 0u);
    CHECK(runtime.rfb_quiesce_complete_sent == 0u);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
    CHECK(physical_adopt_calls == 2);
    CHECK(physical_release_calls == 2);
}

int main(void)
{
    test_established_lineage_initialization_preserves_sequence();
    test_initialize_and_start_failure_ownership();
    test_sole_receiver_dispatch_and_activity();
    test_invalid_frame_converges_fail_closed();
    test_parser_credit_batch_flush_and_residual_distinction();
    test_outbound_fragmentation_and_failure_propagation();
    test_outbound_rfb_waits_for_partial_pi_credit();
    test_finite_quiesce_order_is_distinct_from_fatal_abort();
    test_receiver_completion_event_is_real_no_touch_fence();
    test_outbound_submitter_drain_fences_active_and_queued_callers();
    test_release_waits_for_receiver_completion_before_reclaim();
    test_fatal_stop_completion_precedes_reclaim_and_fresh_session();

    reset_fixture();

    if (failures != 0) {
        fprintf(stderr,
            "transport_runtime_test: %d failure(s)\n",
            failures);
        return 1;
    }

    puts("transport_runtime_test: PASS");
    return 0;
}
