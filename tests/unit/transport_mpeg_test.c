/*
 * File synopsis:
 * Deterministic host fixture for A003 Transport MPEG behavior. A fake physical
 * stream plus pthread-backed PS2 kernel shims prove that channel-4 DATA is
 * dispatched by the same sole receiver as RFB/AUDIO, has independent bounded
 * queue/credit/activity state, wakes event waiters without timer polling, and
 * keeps real producer exhaustion distinct from Transport stop/failure.
 */

#include "transport/runtime.h"

#include <kernel.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

#define MAX_SEMAPHORES 32
#define MAX_THREADS 8
#define MAX_FRAMES 32
#define MAX_SENDS 64

typedef struct fake_semaphore {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int used;
    int count;
    int maximum;
} fake_semaphore_t;

typedef struct fake_thread {
    pthread_t thread;
    void (*entry)(void *argument);
    void *argument;
    int used;
    int started;
    int dormant;
    int joined;
} fake_thread_t;

typedef struct fake_frame {
    uint8_t kind;
    uint8_t channel;
    uint8_t flags;
    size_t payload_length;
    uint8_t payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];
} fake_frame_t;

typedef struct fake_send {
    uint8_t kind;
    uint8_t channel;
    uint8_t flags;
    size_t payload_length;
    uint8_t payload[16];
} fake_send_t;

typedef struct waiter_context {
    pstvnc_transport_runtime_t *runtime;
    uint32_t sequence;
    int result;
} waiter_context_t;

unsigned char _gp;

static fake_semaphore_t g_semaphores[MAX_SEMAPHORES];
static fake_thread_t g_threads[MAX_THREADS];
static pthread_mutex_t g_frame_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_frame_condition = PTHREAD_COND_INITIALIZER;
static fake_frame_t g_frames[MAX_FRAMES];
static int g_frame_read;
static int g_frame_write;
static int g_shutdown;
static int g_receive_active;
static int g_receive_active_max;
static int g_receive_thread_seen;
static pthread_t g_receive_thread;
static int g_receive_thread_mismatch;
static fake_send_t g_sends[MAX_SENDS];
static int g_send_count;

static void reset_fake_world(void)
{
    int index;

    for (index = 0; index < MAX_SEMAPHORES; ++index) {
        if (g_semaphores[index].used) {
            (void)pthread_mutex_destroy(&g_semaphores[index].mutex);
            (void)pthread_cond_destroy(&g_semaphores[index].condition);
        }
    }

    memset(g_semaphores, 0, sizeof(g_semaphores));
    memset(g_threads, 0, sizeof(g_threads));
    memset(g_frames, 0, sizeof(g_frames));
    memset(g_sends, 0, sizeof(g_sends));
    g_frame_read = 0;
    g_frame_write = 0;
    g_shutdown = 0;
    g_receive_active = 0;
    g_receive_active_max = 0;
    g_receive_thread_seen = 0;
    g_receive_thread_mismatch = 0;
    g_send_count = 0;
}

int CreateSema(ee_sema_t *semaphore)
{
    int index;

    for (index = 1; index < MAX_SEMAPHORES; ++index) {
        fake_semaphore_t *slot = &g_semaphores[index];
        if (!slot->used) {
            memset(slot, 0, sizeof(*slot));
            CHECK(pthread_mutex_init(&slot->mutex, NULL) == 0);
            CHECK(pthread_cond_init(&slot->condition, NULL) == 0);
            slot->used = 1;
            slot->count = semaphore->init_count;
            slot->maximum = semaphore->max_count;
            return index;
        }
    }
    return -1;
}

int DeleteSema(int semaphore_id)
{
    fake_semaphore_t *slot;

    if (semaphore_id <= 0 || semaphore_id >= MAX_SEMAPHORES ||
        !g_semaphores[semaphore_id].used)
        return -1;
    slot = &g_semaphores[semaphore_id];
    CHECK(pthread_mutex_destroy(&slot->mutex) == 0);
    CHECK(pthread_cond_destroy(&slot->condition) == 0);
    memset(slot, 0, sizeof(*slot));
    return 0;
}

int WaitSema(int semaphore_id)
{
    fake_semaphore_t *slot;

    if (semaphore_id <= 0 || semaphore_id >= MAX_SEMAPHORES ||
        !g_semaphores[semaphore_id].used)
        return -1;
    slot = &g_semaphores[semaphore_id];
    if (pthread_mutex_lock(&slot->mutex) != 0)
        return -1;
    while (slot->count == 0) {
        if (pthread_cond_wait(&slot->condition, &slot->mutex) != 0) {
            (void)pthread_mutex_unlock(&slot->mutex);
            return -1;
        }
    }
    slot->count -= 1;
    return pthread_mutex_unlock(&slot->mutex) == 0 ? 0 : -1;
}

int SignalSema(int semaphore_id)
{
    fake_semaphore_t *slot;

    if (semaphore_id <= 0 || semaphore_id >= MAX_SEMAPHORES ||
        !g_semaphores[semaphore_id].used)
        return -1;
    slot = &g_semaphores[semaphore_id];
    if (pthread_mutex_lock(&slot->mutex) != 0)
        return -1;
    if (slot->count >= slot->maximum) {
        (void)pthread_mutex_unlock(&slot->mutex);
        return -1;
    }
    slot->count += 1;
    (void)pthread_cond_signal(&slot->condition);
    return pthread_mutex_unlock(&slot->mutex) == 0 ? 0 : -1;
}

static void *fake_thread_entry(void *argument)
{
    fake_thread_t *thread = (fake_thread_t *)argument;

    thread->entry(thread->argument);
    thread->dormant = 1;
    return NULL;
}

int CreateThread(ee_thread_t *thread)
{
    int index;

    for (index = 1; index < MAX_THREADS; ++index) {
        if (!g_threads[index].used) {
            g_threads[index].used = 1;
            g_threads[index].entry = (void (*)(void *))thread->func;
            return index;
        }
    }
    return -1;
}

int StartThread(int thread_id, void *argument)
{
    fake_thread_t *thread;

    if (thread_id <= 0 || thread_id >= MAX_THREADS ||
        !g_threads[thread_id].used)
        return -1;
    thread = &g_threads[thread_id];
    thread->argument = argument;
    thread->started = 1;
    thread->dormant = 0;
    return pthread_create(&thread->thread, NULL, fake_thread_entry, thread) == 0
        ? 0 : -1;
}

int DeleteThread(int thread_id)
{
    fake_thread_t *thread;

    if (thread_id <= 0 || thread_id >= MAX_THREADS ||
        !g_threads[thread_id].used)
        return -1;
    thread = &g_threads[thread_id];
    if (thread->started && !thread->joined) {
        if (pthread_join(thread->thread, NULL) != 0)
            return -1;
        thread->joined = 1;
    }
    memset(thread, 0, sizeof(*thread));
    return 0;
}

int ReferThreadStatus(int thread_id, ee_thread_status_t *status)
{
    if (thread_id <= 0 || thread_id >= MAX_THREADS ||
        !g_threads[thread_id].used || status == NULL)
        return -1;
    status->status = g_threads[thread_id].dormant ? THS_DORMANT : THS_RUNNING;
    return 0;
}

int TerminateThread(int thread_id)
{
    fake_thread_t *thread;

    if (thread_id <= 0 || thread_id >= MAX_THREADS ||
        !g_threads[thread_id].used)
        return -1;
    thread = &g_threads[thread_id];
    if (thread->started && !thread->dormant) {
        if (pthread_cancel(thread->thread) != 0)
            return -1;
        if (pthread_join(thread->thread, NULL) != 0)
            return -1;
        thread->joined = 1;
        thread->dormant = 1;
    }
    return 0;
}

void ExitThread(void)
{
}

int pstvnc_transport_physical_stream_adopt(
    pstvnc_transport_physical_stream_t *stream,
    int socket_fd)
{
    if (stream == NULL || socket_fd < 0)
        return 0;
    memset(stream, 0, sizeof(*stream));
    stream->socket_fd = socket_fd;
    stream->send_semaphore_id = 99;
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
    fake_send_t *send;

    (void)stream;
    CHECK(g_send_count < MAX_SENDS);
    if (g_send_count >= MAX_SENDS)
        return 0;
    send = &g_sends[g_send_count++];
    send->kind = kind;
    send->channel = channel;
    send->flags = flags;
    send->payload_length = payload_length;
    if (payload_length != 0u) {
        CHECK(payload_length <= sizeof(send->payload));
        if (payload_length > sizeof(send->payload))
            return 0;
        memcpy(send->payload, payload, payload_length);
    }
    return 1;
}

int pstvnc_transport_physical_stream_receive_frame(
    pstvnc_transport_physical_stream_t *stream,
    pstvnc_transport_header_t *header,
    void *payload,
    size_t payload_capacity)
{
    fake_frame_t frame;

    (void)stream;
    if (pthread_mutex_lock(&g_frame_mutex) != 0)
        return 0;

    g_receive_active += 1;
    if (g_receive_active > g_receive_active_max)
        g_receive_active_max = g_receive_active;
    if (!g_receive_thread_seen) {
        g_receive_thread = pthread_self();
        g_receive_thread_seen = 1;
    } else if (!pthread_equal(g_receive_thread, pthread_self())) {
        g_receive_thread_mismatch = 1;
    }

    while (g_frame_read == g_frame_write && !g_shutdown)
        (void)pthread_cond_wait(&g_frame_condition, &g_frame_mutex);

    if (g_shutdown && g_frame_read == g_frame_write) {
        g_receive_active -= 1;
        (void)pthread_mutex_unlock(&g_frame_mutex);
        return 0;
    }

    frame = g_frames[g_frame_read % MAX_FRAMES];
    g_frame_read += 1;
    (void)pthread_cond_broadcast(&g_frame_condition);
    g_receive_active -= 1;
    (void)pthread_mutex_unlock(&g_frame_mutex);

    if (frame.payload_length > payload_capacity)
        return 0;
    header->kind = frame.kind;
    header->channel = frame.channel;
    header->flags = frame.flags;
    header->sequence = (uint32_t)g_frame_read;
    header->payload_length = (uint32_t)frame.payload_length;
    if (frame.payload_length != 0u)
        memcpy(payload, frame.payload, frame.payload_length);
    return 1;
}

int pstvnc_transport_physical_stream_shutdown_io(
    pstvnc_transport_physical_stream_t *stream)
{
    (void)stream;
    CHECK(pthread_mutex_lock(&g_frame_mutex) == 0);
    g_shutdown = 1;
    (void)pthread_cond_broadcast(&g_frame_condition);
    CHECK(pthread_mutex_unlock(&g_frame_mutex) == 0);
    return 1;
}

void pstvnc_transport_physical_stream_release(
    pstvnc_transport_physical_stream_t *stream)
{
    if (stream != NULL) {
        stream->socket_fd = -1;
        stream->send_semaphore_id = -1;
    }
}

static void push_frame(
    uint8_t kind,
    uint8_t channel,
    const uint8_t *payload,
    size_t payload_length)
{
    fake_frame_t *frame;

    CHECK(pthread_mutex_lock(&g_frame_mutex) == 0);
    CHECK(g_frame_write - g_frame_read < MAX_FRAMES);
    frame = &g_frames[g_frame_write % MAX_FRAMES];
    memset(frame, 0, sizeof(*frame));
    frame->kind = kind;
    frame->channel = channel;
    frame->payload_length = payload_length;
    if (payload_length != 0u)
        memcpy(frame->payload, payload, payload_length);
    g_frame_write += 1;
    (void)pthread_cond_broadcast(&g_frame_condition);
    CHECK(pthread_mutex_unlock(&g_frame_mutex) == 0);
}

static void wait_for_frames(int count)
{
    CHECK(pthread_mutex_lock(&g_frame_mutex) == 0);
    while (g_frame_read < count)
        (void)pthread_cond_wait(&g_frame_condition, &g_frame_mutex);
    CHECK(pthread_mutex_unlock(&g_frame_mutex) == 0);
}

static uint32_t credit_amount(const fake_send_t *send)
{
    if (send->payload_length != 4u)
        return 0u;
    return ((uint32_t)send->payload[0] << 24) |
        ((uint32_t)send->payload[1] << 16) |
        ((uint32_t)send->payload[2] << 8) |
        (uint32_t)send->payload[3];
}

static int find_credit(uint8_t channel, uint32_t amount)
{
    int index;

    for (index = 0; index < g_send_count; ++index) {
        if (g_sends[index].kind == PSTVNC_TRANSPORT_FRAME_CREDIT &&
            g_sends[index].channel == channel &&
            credit_amount(&g_sends[index]) == amount)
            return 1;
    }
    return 0;
}

static pstvnc_transport_session_config_t make_base_config(void)
{
    pstvnc_transport_session_config_t config;

    memset(&config, 0, sizeof(config));
    config.rfb_queue_capacity = 64u;
    config.rfb_initial_credit_bytes = 8u;
    config.rfb_credit_batch_bytes = 4u;
    config.rfb_credit_flush_on_empty = 1;
    config.rfb_credit_return_enabled = 1;
    config.receiver_thread_stack_size = 4096u;
    config.receiver_thread_priority = 64;
    config.max_data_payload = 16u;
    return config;
}

static pstvnc_transport_audio_channel_config_t make_audio_config(void)
{
    pstvnc_transport_audio_channel_config_t config;

    memset(&config, 0, sizeof(config));
    config.queue_capacity = 64u;
    config.initial_credit_bytes = 12u;
    config.credit_batch_bytes = 4u;
    config.credit_flush_on_empty = 1;
    config.credit_return_enabled = 1;
    return config;
}

static pstvnc_transport_mpeg_channel_config_t make_mpeg_config(void)
{
    pstvnc_transport_mpeg_channel_config_t config;

    memset(&config, 0, sizeof(config));
    config.queue_capacity = 64u;
    config.initial_credit_bytes = 16u;
    config.credit_batch_bytes = 4u;
    config.credit_flush_on_empty = 1;
    config.credit_return_enabled = 1;
    return config;
}

static void *mpeg_waiter(void *argument)
{
    waiter_context_t *waiter = (waiter_context_t *)argument;
    waiter->result = pstvnc_transport_runtime_mpeg_wait_activity(
        waiter->runtime,
        &waiter->sequence);
    return NULL;
}

static void wait_until_mpeg_waiter_armed(pstvnc_transport_runtime_t *runtime)
{
    for (;;) {
        int armed;

        CHECK(WaitSema(runtime->mpeg_queue_semaphore_id) == 0);
        armed = runtime->mpeg_activity_wait_armed != 0;
        CHECK(SignalSema(runtime->mpeg_queue_semaphore_id) == 0);
        if (armed)
            return;
        (void)sched_yield();
    }
}

static void finish_runtime(pstvnc_transport_runtime_t *runtime)
{
    if (!runtime->receiver_done)
        CHECK(pstvnc_transport_runtime_request_stop(runtime) == 1);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(runtime) == 1);
    CHECK(pstvnc_transport_runtime_release(runtime) == 1);
}

static void test_interleaved_channels_credit_and_event_wake(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t base = make_base_config();
    pstvnc_transport_audio_channel_config_t audio = make_audio_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    const uint8_t rfb[] = {'R', 'F'};
    const uint8_t aud[] = {'A', 'U', 'D'};
    const uint8_t mpg[] = {1u, 2u, 3u, 4u, 5u};
    const uint8_t wake_byte[] = {9u};
    uint8_t buffer[16];
    size_t count;
    uint32_t sequence;
    int producer_done;
    size_t available;
    pthread_t waiter_thread;
    waiter_context_t waiter;

    reset_fake_world();
    CHECK(pstvnc_transport_runtime_initialize_with_audio_mpeg(
        &runtime, 50, &base, &audio, &mpeg) == 1);
    CHECK(pstvnc_transport_runtime_start_receiver(&runtime) == 1);
    CHECK(find_credit(PSTVNC_TRANSPORT_CHANNEL_RFB, 8u));
    CHECK(find_credit(PSTVNC_TRANSPORT_CHANNEL_AUDIO, 12u));
    CHECK(find_credit(PSTVNC_TRANSPORT_CHANNEL_MPEG2, 16u));

    push_frame(PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB, rfb, sizeof(rfb));
    push_frame(PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_AUDIO, aud, sizeof(aud));
    push_frame(PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_MPEG2, mpg, sizeof(mpg));
    wait_for_frames(3);

    CHECK(runtime.failed == 0);
    CHECK(g_receive_active_max == 1);
    CHECK(g_receive_thread_mismatch == 0);

    memset(buffer, 0, sizeof(buffer));
    CHECK(pstvnc_transport_runtime_rfb_read_exact(&runtime, buffer, 2u) == 1);
    CHECK(memcmp(buffer, rfb, sizeof(rfb)) == 0);
    CHECK(find_credit(PSTVNC_TRANSPORT_CHANNEL_RFB, 2u));

    count = 0u;
    CHECK(pstvnc_transport_runtime_audio_read_available(
        &runtime, buffer, sizeof(buffer), &count) == PSTVNC_TRANSPORT_OK);
    CHECK(count == sizeof(aud));
    CHECK(memcmp(buffer, aud, sizeof(aud)) == 0);
    CHECK(find_credit(PSTVNC_TRANSPORT_CHANNEL_AUDIO, 3u));

    count = 0u;
    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, buffer, 2u, &count) == PSTVNC_TRANSPORT_OK);
    CHECK(count == 2u && buffer[0] == 1u && buffer[1] == 2u);
    CHECK(!find_credit(PSTVNC_TRANSPORT_CHANNEL_MPEG2, 2u));

    count = 0u;
    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, buffer, sizeof(buffer), &count) == PSTVNC_TRANSPORT_OK);
    CHECK(count == 3u && buffer[0] == 3u && buffer[2] == 5u);
    CHECK(find_credit(PSTVNC_TRANSPORT_CHANNEL_MPEG2, 5u));

    CHECK(pstvnc_transport_runtime_mpeg_status(
        &runtime, &available, &producer_done) == PSTVNC_TRANSPORT_OK);
    CHECK(available == 0u && producer_done == 0);

    CHECK(pstvnc_transport_runtime_mpeg_activity_snapshot(
        &runtime, &sequence) == 1);
    memset(&waiter, 0, sizeof(waiter));
    waiter.runtime = &runtime;
    waiter.sequence = sequence;
    CHECK(pthread_create(&waiter_thread, NULL, mpeg_waiter, &waiter) == 0);
    wait_until_mpeg_waiter_armed(&runtime);
    push_frame(PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_MPEG2, wake_byte, sizeof(wake_byte));
    wait_for_frames(4);
    CHECK(pthread_join(waiter_thread, NULL) == 0);
    CHECK(waiter.result == 1);
    CHECK(waiter.sequence != sequence);

    count = 0u;
    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, buffer, sizeof(buffer), &count) == PSTVNC_TRANSPORT_OK);
    CHECK(count == 1u && buffer[0] == 9u);

    CHECK(pstvnc_transport_runtime_mpeg_mark_producer_done(&runtime) == 1);
    CHECK(pstvnc_transport_runtime_mpeg_status(
        &runtime, &available, &producer_done) == PSTVNC_TRANSPORT_OK);
    CHECK(available == 0u && producer_done == 1);
    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, buffer, sizeof(buffer), &count) == PSTVNC_TRANSPORT_EXHAUSTED);

    push_frame(PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_MPEG2, wake_byte, sizeof(wake_byte));
    wait_for_frames(5);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);
    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, buffer, sizeof(buffer), &count) == PSTVNC_TRANSPORT_FAILED);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_zero_length_mpeg_is_not_eof(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t base = make_base_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    size_t available = 99u;
    int producer_done = 1;
    uint8_t byte = 0u;
    size_t count = 0u;

    reset_fake_world();
    CHECK(pstvnc_transport_runtime_initialize_with_mpeg(
        &runtime, 51, &base, &mpeg) == 1);
    CHECK(pstvnc_transport_runtime_start_receiver(&runtime) == 1);
    push_frame(PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_MPEG2, NULL, 0u);
    wait_for_frames(1);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);
    CHECK(pstvnc_transport_runtime_mpeg_status(
        &runtime, &available, &producer_done) == PSTVNC_TRANSPORT_OK);
    CHECK(available == 0u && producer_done == 0);
    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, &byte, 1u, &count) == PSTVNC_TRANSPORT_FAILED);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_queue_overflow_preserves_committed_bytes_then_failure(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t base = make_base_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    const uint8_t first[] = {1u, 2u, 3u, 4u, 5u, 6u};
    const uint8_t second[] = {7u, 8u, 9u, 10u};
    uint8_t buffer[16];
    size_t count = 0u;

    reset_fake_world();
    base.max_data_payload = 8u;
    mpeg.queue_capacity = 8u;
    mpeg.initial_credit_bytes = 8u;
    mpeg.credit_batch_bytes = 4u;
    CHECK(pstvnc_transport_runtime_initialize_with_mpeg(
        &runtime, 52, &base, &mpeg) == 1);
    CHECK(pstvnc_transport_runtime_start_receiver(&runtime) == 1);
    push_frame(PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_MPEG2, first, sizeof(first));
    push_frame(PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_MPEG2, second, sizeof(second));
    wait_for_frames(2);
    CHECK(pstvnc_transport_runtime_wait_receiver_done(&runtime) == 1);
    CHECK(runtime.failed == 1);

    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, buffer, sizeof(buffer), &count) == PSTVNC_TRANSPORT_OK);
    CHECK(count == sizeof(first));
    CHECK(memcmp(buffer, first, sizeof(first)) == 0);
    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, buffer, sizeof(buffer), &count) == PSTVNC_TRANSPORT_FAILED);
    CHECK(pstvnc_transport_runtime_release(&runtime) == 1);
}

static void test_transport_stop_wakes_mpeg_waiter_without_eof(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t base = make_base_config();
    pstvnc_transport_mpeg_channel_config_t mpeg = make_mpeg_config();
    waiter_context_t waiter;
    pthread_t waiter_thread;
    uint8_t byte = 0u;
    size_t count = 0u;

    reset_fake_world();
    CHECK(pstvnc_transport_runtime_initialize_with_mpeg(
        &runtime, 53, &base, &mpeg) == 1);
    CHECK(pstvnc_transport_runtime_start_receiver(&runtime) == 1);
    memset(&waiter, 0, sizeof(waiter));
    waiter.runtime = &runtime;
    CHECK(pstvnc_transport_runtime_mpeg_activity_snapshot(
        &runtime, &waiter.sequence) == 1);
    CHECK(pthread_create(&waiter_thread, NULL, mpeg_waiter, &waiter) == 0);
    wait_until_mpeg_waiter_armed(&runtime);

    CHECK(pstvnc_transport_runtime_request_stop(&runtime) == 1);
    CHECK(pthread_join(waiter_thread, NULL) == 0);
    CHECK(waiter.result == 1);
    CHECK(pstvnc_transport_runtime_mpeg_read_available(
        &runtime, &byte, 1u, &count) == PSTVNC_TRANSPORT_STOPPED);
    finish_runtime(&runtime);
}

int main(void)
{
    test_interleaved_channels_credit_and_event_wake();
    test_zero_length_mpeg_is_not_eof();
    test_queue_overflow_preserves_committed_bytes_then_failure();
    test_transport_stop_wakes_mpeg_waiter_without_eof();

    if (failures != 0) {
        fprintf(stderr, "%d Transport MPEG test(s) failed\n", failures);
        return 1;
    }

    puts("transport_mpeg_test: PASS");
    return 0;
}
