/*
 * File synopsis:
 * Deterministic host contracts for A003's MPEG worker/one-slot borrowed-frame
 * rendezvous. Injected thread/event mechanics let the fixture exercise exact
 * interleavings without introducing a second production threading model.
 */

#include "mpeg/worker.h"
#include "transport/bridge.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

#define TEST_GENERATION 700u

typedef struct fake_memory_record {
    void *raw;
    void *aligned;
    size_t byte_count;
    size_t alignment;
    int released;
} fake_memory_record_t;

typedef struct fake_memory {
    fake_memory_record_t records[8];
    int allocate_calls;
    int release_calls;
} fake_memory_t;

typedef struct fake_sync {
    int locked;
} fake_sync_t;

typedef struct fake_thread {
    pstvnc_mpeg_worker_thread_entry_t entry;
    void *argument;
    int thread_id;
    int create_calls;
    int start_calls;
    int join_calls;
    int destroy_calls;
    int ran;
} fake_thread_t;

struct fixture;

typedef enum fake_event_mode {
    FAKE_EVENT_RELEASE_EACH = 0,
    FAKE_EVENT_STOP_AVAILABLE,
    FAKE_EVENT_STOP_CLAIMED
} fake_event_mode_t;

typedef struct fake_event {
    struct fixture *fixture;
    fake_event_mode_t mode;
    int pending;
    int signal_calls;
    int wait_calls;
    int hook_calls;
    int retained_consumes;
} fake_event_t;

typedef struct fake_transport {
    uint32_t activity_sequence;
    int read_calls;
    int wait_calls;
} fake_transport_t;

typedef struct fake_platform {
    pstvnc_mpeg_feed_callback_t feed_callback;
    void *feed_context;
    pstvnc_mpeg_sequence_callback_t sequence_callback;
    void *sequence_context;
    pstvnc_mpeg_worker_t *worker;
    uint32_t generation;
    void *picture_buffer;
    size_t picture_capacity;
    int prepare_calls;
    int initialize_calls;
    int picture_calls;
    int submit_calls;
    int destroy_calls;
    int release_state_calls;
    int ready_count;
    int fail_call;
    int stop_on_call;
} fake_platform_t;

typedef struct fixture {
    pstvnc_mpeg_worker_t worker;
    pstvnc_mpeg_worker_values_t values;
    pstvnc_mpeg_decoder_config_t decoder_config;
    pstvnc_mpeg_decoder_memory_ops_t decoder_memory_ops;
    pstvnc_mpeg_decoder_sync_ops_t decoder_sync_ops;
    pstvnc_mpeg_decoder_platform_ops_t platform_ops;
    pstvnc_mpeg_worker_memory_ops_t worker_memory_ops;
    pstvnc_mpeg_worker_thread_ops_t thread_ops;
    pstvnc_mpeg_worker_sync_ops_t worker_sync_ops;
    pstvnc_mpeg_worker_event_ops_t event_ops;
    fake_memory_t memory;
    fake_sync_t decoder_sync;
    fake_sync_t worker_sync;
    fake_thread_t thread;
    fake_event_t event;
    fake_platform_t platform;
} fixture_t;

static fake_transport_t g_transport;

static void *fake_allocate(void *context, size_t byte_count, size_t alignment)
{
    fake_memory_t *memory = (fake_memory_t *)context;
    fake_memory_record_t *record;
    uintptr_t address;
    size_t extra;
    void *raw;

    CHECK(memory != NULL);
    CHECK(memory->allocate_calls < 8);
    if (memory == NULL || memory->allocate_calls >= 8)
        return NULL;

    record = &memory->records[memory->allocate_calls++];
    extra = alignment > 1u ? alignment - 1u : 0u;
    raw = malloc(byte_count + extra);
    if (raw == NULL)
        return NULL;

    address = (uintptr_t)raw;
    if (alignment > 1u)
        address = (address + alignment - 1u) &
            ~((uintptr_t)alignment - 1u);

    record->raw = raw;
    record->aligned = (void *)address;
    record->byte_count = byte_count;
    record->alignment = alignment;
    return record->aligned;
}

static void fake_release(void *context, void *pointer)
{
    fake_memory_t *memory = (fake_memory_t *)context;
    int index;

    memory->release_calls += 1;

    for (index = 0; index < memory->allocate_calls; ++index) {
        fake_memory_record_t *record = &memory->records[index];

        if (record->aligned == pointer && !record->released) {
            record->released = 1;
            free(record->raw);
            return;
        }
    }

    CHECK(0 && "release must target a live fake allocation");
}

static int fake_lock(void *context)
{
    fake_sync_t *sync = (fake_sync_t *)context;

    if (sync == NULL || sync->locked)
        return -1;

    sync->locked = 1;
    return 0;
}

static int fake_unlock(void *context)
{
    fake_sync_t *sync = (fake_sync_t *)context;

    if (sync == NULL || !sync->locked)
        return -1;

    sync->locked = 0;
    return 0;
}

static int fake_thread_create(
    void *context,
    pstvnc_mpeg_worker_thread_entry_t entry,
    void *argument,
    void *stack,
    size_t stack_bytes,
    int priority,
    int *thread_id)
{
    fake_thread_t *thread = (fake_thread_t *)context;

    CHECK(thread != NULL);
    CHECK(entry != NULL);
    CHECK(argument != NULL);
    CHECK(stack != NULL);
    CHECK(stack_bytes == 4096u);
    CHECK(priority == 43);
    CHECK(thread_id != NULL);

    if (thread == NULL || entry == NULL || argument == NULL ||
        stack == NULL || thread_id == NULL)
        return -1;

    thread->create_calls += 1;
    thread->entry = entry;
    thread->argument = argument;
    thread->thread_id = 91;
    *thread_id = thread->thread_id;
    return 0;
}

static int fake_thread_start(void *context, int thread_id)
{
    fake_thread_t *thread = (fake_thread_t *)context;

    CHECK(thread != NULL);
    CHECK(thread_id == thread->thread_id);
    if (thread == NULL || thread_id != thread->thread_id)
        return -1;

    thread->start_calls += 1;
    return 0;
}

static int fake_thread_join(void *context, int thread_id)
{
    fake_thread_t *thread = (fake_thread_t *)context;

    CHECK(thread != NULL);
    CHECK(thread_id == thread->thread_id);
    CHECK(thread->ran);
    if (thread == NULL || thread_id != thread->thread_id || !thread->ran)
        return -1;

    thread->join_calls += 1;
    return 0;
}

static int fake_thread_destroy(void *context, int thread_id)
{
    fake_thread_t *thread = (fake_thread_t *)context;

    CHECK(thread != NULL);
    CHECK(thread_id == thread->thread_id);
    if (thread == NULL || thread_id != thread->thread_id)
        return -1;

    thread->destroy_calls += 1;
    return 0;
}

static void fake_event_signal(void *context)
{
    fake_event_t *event = (fake_event_t *)context;

    CHECK(event != NULL);
    if (event == NULL)
        return;

    event->signal_calls += 1;
    event->pending = 1;
}

static void fake_event_hook_release_each(fake_event_t *event)
{
    fixture_t *fixture = event->fixture;
    pstvnc_mpeg_worker_status_t status;
    pstvnc_mpeg_worker_frame_t frame;
    int expected_ordinal = event->hook_calls + 1;
    int picture_calls_before = fixture->platform.picture_calls;

    CHECK(picture_calls_before == expected_ordinal);
    CHECK(pstvnc_mpeg_worker_status(
        &fixture->worker,
        TEST_GENERATION,
        &status) == PSTVNC_MPEG_WORKER_OK);
    CHECK(status.slot_state == PSTVNC_MPEG_WORKER_SLOT_AVAILABLE);

    CHECK(pstvnc_mpeg_worker_claim(
        &fixture->worker,
        TEST_GENERATION + 1u,
        &frame) == PSTVNC_MPEG_WORKER_WRONG_GENERATION);

    CHECK(pstvnc_mpeg_worker_claim(
        &fixture->worker,
        TEST_GENERATION,
        &frame) == PSTVNC_MPEG_WORKER_OK);
    CHECK(frame.run_generation == TEST_GENERATION);
    CHECK(frame.claim_token == (uint32_t)expected_ordinal);
    CHECK(frame.picture.picture_ordinal == (uint32_t)expected_ordinal);
    CHECK(frame.picture.pixels == fixture->platform.picture_buffer);
    CHECK(frame.picture.byte_count == 1024u);
    CHECK(frame.picture.capacity_bytes == 1024u);
    CHECK(frame.picture.width == 32u);
    CHECK(frame.picture.height == 16u);
    CHECK(frame.picture.bytes_per_pixel == 2u);
    CHECK(fixture->platform.picture_calls == picture_calls_before);

    CHECK(pstvnc_mpeg_worker_status(
        &fixture->worker,
        TEST_GENERATION,
        &status) == PSTVNC_MPEG_WORKER_OK);
    CHECK(status.slot_state == PSTVNC_MPEG_WORKER_SLOT_CLAIMED);

    if (event->hook_calls == 0) {
        CHECK(pstvnc_mpeg_worker_release_frame(
            &fixture->worker,
            TEST_GENERATION,
            frame.claim_token + 100u) ==
            PSTVNC_MPEG_WORKER_CLAIM_MISMATCH);
        CHECK(fixture->platform.picture_calls == picture_calls_before);
    }

    CHECK(pstvnc_mpeg_worker_release_frame(
        &fixture->worker,
        TEST_GENERATION,
        frame.claim_token) == PSTVNC_MPEG_WORKER_OK);
    CHECK(fixture->platform.picture_calls == picture_calls_before);
    event->hook_calls += 1;
}

static void fake_event_hook_stop_available(fake_event_t *event)
{
    fixture_t *fixture = event->fixture;
    pstvnc_mpeg_worker_status_t status;
    pstvnc_mpeg_worker_frame_t frame;

    CHECK(event->hook_calls == 0);
    CHECK(fixture->platform.picture_calls == 1);

    CHECK(pstvnc_mpeg_worker_request_stop(
        &fixture->worker,
        TEST_GENERATION) == PSTVNC_MPEG_WORKER_OK);

    CHECK(pstvnc_mpeg_worker_status(
        &fixture->worker,
        TEST_GENERATION,
        &status) == PSTVNC_MPEG_WORKER_OK);
    CHECK(status.stop_requested);
    CHECK(status.slot_state == PSTVNC_MPEG_WORKER_SLOT_EMPTY);

    CHECK(pstvnc_mpeg_worker_claim(
        &fixture->worker,
        TEST_GENERATION,
        &frame) == PSTVNC_MPEG_WORKER_FRAME_UNAVAILABLE);
    CHECK(fixture->platform.picture_calls == 1);
    event->hook_calls += 1;
}

static void fake_event_hook_stop_claimed(fake_event_t *event)
{
    fixture_t *fixture = event->fixture;
    pstvnc_mpeg_worker_status_t status;
    pstvnc_mpeg_worker_frame_t frame;
    const uint8_t *pixels;

    CHECK(event->hook_calls == 0);
    CHECK(fixture->platform.picture_calls == 1);

    CHECK(pstvnc_mpeg_worker_claim(
        &fixture->worker,
        TEST_GENERATION,
        &frame) == PSTVNC_MPEG_WORKER_OK);
    pixels = (const uint8_t *)frame.picture.pixels;
    CHECK(pixels != NULL);
    if (pixels != NULL)
        CHECK(pixels[0] == 1u);

    CHECK(pstvnc_mpeg_worker_request_stop(
        &fixture->worker,
        TEST_GENERATION) == PSTVNC_MPEG_WORKER_OK);

    CHECK(pstvnc_mpeg_worker_status(
        &fixture->worker,
        TEST_GENERATION,
        &status) == PSTVNC_MPEG_WORKER_OK);
    CHECK(status.stop_requested);
    CHECK(status.slot_state == PSTVNC_MPEG_WORKER_SLOT_CLAIMED);
    CHECK(fixture->platform.destroy_calls == 0);
    CHECK(fixture->platform.picture_calls == 1);

    CHECK(pstvnc_mpeg_worker_join(
        &fixture->worker,
        TEST_GENERATION) == PSTVNC_MPEG_WORKER_FRAME_OUTSTANDING);
    CHECK(fixture->thread.join_calls == 0);
    CHECK(fixture->platform.destroy_calls == 0);

    CHECK(pstvnc_mpeg_worker_release_frame(
        &fixture->worker,
        TEST_GENERATION + 1u,
        frame.claim_token) == PSTVNC_MPEG_WORKER_WRONG_GENERATION);
    CHECK(pstvnc_mpeg_worker_release_frame(
        &fixture->worker,
        TEST_GENERATION,
        frame.claim_token + 1u) == PSTVNC_MPEG_WORKER_CLAIM_MISMATCH);

    if (pixels != NULL)
        CHECK(pixels[0] == 1u);
    CHECK(fixture->platform.picture_calls == 1);
    CHECK(fixture->platform.destroy_calls == 0);

    CHECK(pstvnc_mpeg_worker_release_frame(
        &fixture->worker,
        TEST_GENERATION,
        frame.claim_token) == PSTVNC_MPEG_WORKER_OK);
    event->hook_calls += 1;
}

static void fake_event_wait(void *context)
{
    fake_event_t *event = (fake_event_t *)context;

    CHECK(event != NULL);
    if (event == NULL)
        return;

    event->wait_calls += 1;

    if (event->mode == FAKE_EVENT_RELEASE_EACH)
        fake_event_hook_release_each(event);
    else if (event->mode == FAKE_EVENT_STOP_AVAILABLE)
        fake_event_hook_stop_available(event);
    else
        fake_event_hook_stop_claimed(event);

    /*
     * The hook changes the protected predicate and signals before this wait
     * consumes the event. A pending signal here proves signal-before-wait
     * retention rather than a polling retry.
     */
    CHECK(event->pending);
    if (event->pending) {
        event->pending = 0;
        event->retained_consumes += 1;
    }
}

pstvnc_transport_result_t pstvnc_transport_access_acquire(
    pstvnc_transport_access_t *transport_access)
{
    if (transport_access == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    transport_access->opaque_ticket = 1u;
    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_activity_snapshot(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    (void)transport_access;

    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    *activity_sequence = g_transport.activity_sequence;
    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_wait_activity(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    (void)transport_access;

    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    g_transport.wait_calls += 1;
    g_transport.activity_sequence += 1u;
    *activity_sequence = g_transport.activity_sequence;
    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_mpeg_read_available(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    (void)transport_access;

    g_transport.read_calls += 1;
    if (buffer == NULL || maximum_count == 0u || read_count == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    *read_count = 0u;
    return PSTVNC_TRANSPORT_EXHAUSTED;
}

static int fake_prepare_known_state(void *context)
{
    fake_platform_t *platform = (fake_platform_t *)context;
    platform->prepare_calls += 1;
    return 0;
}

static int fake_initialize(
    void *context,
    pstvnc_mpeg_feed_callback_t feed_callback,
    void *feed_context,
    pstvnc_mpeg_sequence_callback_t sequence_callback,
    void *sequence_context)
{
    fake_platform_t *platform = (fake_platform_t *)context;

    platform->initialize_calls += 1;
    platform->feed_callback = feed_callback;
    platform->feed_context = feed_context;
    platform->sequence_callback = sequence_callback;
    platform->sequence_context = sequence_context;
    return 0;
}

static int fake_submit_feed(
    void *context,
    const uint8_t *bytes,
    size_t payload_bytes,
    size_t transfer_bytes)
{
    fake_platform_t *platform = (fake_platform_t *)context;

    (void)bytes;
    (void)payload_bytes;
    (void)transfer_bytes;
    platform->submit_calls += 1;
    return 0;
}

static int fake_destroy(void *context)
{
    fake_platform_t *platform = (fake_platform_t *)context;
    platform->destroy_calls += 1;
    return 0;
}

static int fake_release_known_state(void *context)
{
    fake_platform_t *platform = (fake_platform_t *)context;
    platform->release_state_calls += 1;
    return 0;
}

static int fake_picture(void *context)
{
    fake_platform_t *platform = (fake_platform_t *)context;

    platform->picture_calls += 1;

    if (platform->picture_calls == 1) {
        size_t capacity = 0u;

        platform->picture_buffer = platform->sequence_callback(
            platform->sequence_context,
            32u,
            16u,
            &capacity);
        platform->picture_capacity = capacity;

        CHECK(platform->picture_buffer != NULL);
        CHECK(capacity == 1024u);
        if (platform->picture_buffer == NULL)
            return -1;
    }

    if (platform->stop_on_call == platform->picture_calls) {
        CHECK(pstvnc_mpeg_worker_request_stop(
            platform->worker,
            platform->generation) == PSTVNC_MPEG_WORKER_OK);
    }

    if (platform->fail_call == platform->picture_calls)
        return -1;

    if (platform->picture_calls <= platform->ready_count) {
        memset(
            platform->picture_buffer,
            platform->picture_calls,
            platform->picture_capacity);
        return 1;
    }

    return platform->feed_callback(platform->feed_context);
}

static void fixture_init(fixture_t *fixture)
{
    memset(fixture, 0, sizeof(*fixture));
    memset(&g_transport, 0, sizeof(g_transport));

    fixture->values.worker_stack_bytes = 4096u;
    fixture->values.worker_priority = 43;

    fixture->decoder_config.max_width = 32u;
    fixture->decoder_config.max_height = 16u;
    fixture->decoder_config.bytes_per_pixel = 2u;
    fixture->decoder_config.feed_payload_capacity = 64u;
    fixture->decoder_config.transfer_alignment = 16u;
    fixture->decoder_config.buffer_alignment = 16u;

    fixture->decoder_memory_ops.allocate = fake_allocate;
    fixture->decoder_memory_ops.release = fake_release;
    fixture->decoder_memory_ops.context = &fixture->memory;

    fixture->decoder_sync_ops.lock = fake_lock;
    fixture->decoder_sync_ops.unlock = fake_unlock;
    fixture->decoder_sync_ops.context = &fixture->decoder_sync;

    fixture->platform_ops.prepare_known_state = fake_prepare_known_state;
    fixture->platform_ops.initialize = fake_initialize;
    fixture->platform_ops.picture = fake_picture;
    fixture->platform_ops.submit_feed = fake_submit_feed;
    fixture->platform_ops.destroy = fake_destroy;
    fixture->platform_ops.release_known_state = fake_release_known_state;
    fixture->platform_ops.context = &fixture->platform;

    fixture->worker_memory_ops.allocate = fake_allocate;
    fixture->worker_memory_ops.release = fake_release;
    fixture->worker_memory_ops.context = &fixture->memory;

    fixture->thread_ops.create = fake_thread_create;
    fixture->thread_ops.start = fake_thread_start;
    fixture->thread_ops.join = fake_thread_join;
    fixture->thread_ops.destroy = fake_thread_destroy;
    fixture->thread_ops.context = &fixture->thread;

    fixture->worker_sync_ops.lock = fake_lock;
    fixture->worker_sync_ops.unlock = fake_unlock;
    fixture->worker_sync_ops.context = &fixture->worker_sync;

    fixture->event.fixture = fixture;
    fixture->event.mode = FAKE_EVENT_RELEASE_EACH;
    fixture->event_ops.signal = fake_event_signal;
    fixture->event_ops.wait = fake_event_wait;
    fixture->event_ops.context = &fixture->event;

    fixture->platform.worker = &fixture->worker;
    fixture->platform.generation = TEST_GENERATION;
}

static void fixture_start(fixture_t *fixture)
{
    CHECK(pstvnc_mpeg_worker_start(
        &fixture->worker,
        TEST_GENERATION,
        &fixture->values,
        &fixture->decoder_config,
        &fixture->decoder_memory_ops,
        &fixture->decoder_sync_ops,
        &fixture->platform_ops,
        &fixture->worker_memory_ops,
        &fixture->thread_ops,
        &fixture->worker_sync_ops,
        &fixture->event_ops) == PSTVNC_MPEG_WORKER_OK);

    CHECK(fixture->thread.create_calls == 1);
    CHECK(fixture->thread.start_calls == 1);
}

static void fixture_run_worker(fixture_t *fixture)
{
    CHECK(fixture->thread.entry != NULL);
    CHECK(fixture->thread.argument == &fixture->worker);

    if (fixture->thread.entry != NULL)
        fixture->thread.entry(fixture->thread.argument);

    fixture->thread.ran = 1;
}

static void fixture_join_release(
    fixture_t *fixture,
    pstvnc_mpeg_worker_outcome_t *outcome)
{
    CHECK(pstvnc_mpeg_worker_outcome(
        &fixture->worker,
        TEST_GENERATION,
        outcome) == PSTVNC_MPEG_WORKER_NOT_FINISHED);

    CHECK(pstvnc_mpeg_worker_join(
        &fixture->worker,
        TEST_GENERATION) == PSTVNC_MPEG_WORKER_OK);

    CHECK(pstvnc_mpeg_worker_outcome(
        &fixture->worker,
        TEST_GENERATION,
        outcome) == PSTVNC_MPEG_WORKER_OK);

    CHECK(pstvnc_mpeg_worker_release(
        &fixture->worker,
        TEST_GENERATION) == PSTVNC_MPEG_WORKER_OK);

    CHECK(fixture->thread.join_calls == 1);
    CHECK(fixture->thread.destroy_calls == 1);
}

static void test_one_slot_no_run_ahead_and_lost_wake(void)
{
    fixture_t fixture;
    pstvnc_mpeg_worker_status_t status;
    pstvnc_mpeg_worker_frame_t frame;
    pstvnc_mpeg_worker_outcome_t outcome;

    fixture_init(&fixture);
    fixture.platform.ready_count = 2;
    fixture.event.mode = FAKE_EVENT_RELEASE_EACH;
    fixture_start(&fixture);

    CHECK(pstvnc_mpeg_worker_status(
        &fixture.worker,
        TEST_GENERATION + 1u,
        &status) == PSTVNC_MPEG_WORKER_WRONG_GENERATION);
    CHECK(pstvnc_mpeg_worker_claim(
        &fixture.worker,
        TEST_GENERATION + 1u,
        &frame) == PSTVNC_MPEG_WORKER_WRONG_GENERATION);
    CHECK(pstvnc_mpeg_worker_request_stop(
        &fixture.worker,
        TEST_GENERATION + 1u) == PSTVNC_MPEG_WORKER_WRONG_GENERATION);

    fixture_run_worker(&fixture);

    CHECK(fixture.platform.picture_calls == 3);
    CHECK(fixture.event.wait_calls == 2);
    CHECK(fixture.event.hook_calls == 2);
    CHECK(fixture.event.signal_calls == 2);
    CHECK(fixture.event.retained_consumes == 2);
    CHECK(fixture.platform.destroy_calls == 1);
    CHECK(fixture.platform.release_state_calls == 1);

    CHECK(pstvnc_mpeg_worker_status(
        &fixture.worker,
        TEST_GENERATION,
        &status) == PSTVNC_MPEG_WORKER_OK);
    CHECK(status.slot_state == PSTVNC_MPEG_WORKER_SLOT_EMPTY);
    CHECK(status.worker_finished);
    CHECK(!status.decoder_live);

    fixture_join_release(&fixture, &outcome);

    CHECK(outcome.kind == PSTVNC_MPEG_WORKER_OUTCOME_COMPLETED);
    CHECK(outcome.worker_result == PSTVNC_MPEG_WORKER_OK);
    CHECK(outcome.decoder_result == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(outcome.decoder_release_result == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(outcome.decoder_report.pictures_decoded == 2u);
    CHECK(outcome.run_generation == TEST_GENERATION);
    CHECK(fixture.memory.release_calls == 3);
}

static void test_stop_discards_available_frame(void)
{
    fixture_t fixture;
    pstvnc_mpeg_worker_outcome_t outcome;

    fixture_init(&fixture);
    fixture.platform.ready_count = 4;
    fixture.event.mode = FAKE_EVENT_STOP_AVAILABLE;
    fixture_start(&fixture);
    fixture_run_worker(&fixture);

    CHECK(fixture.platform.picture_calls == 1);
    CHECK(fixture.event.wait_calls == 1);
    CHECK(fixture.event.retained_consumes == 1);

    fixture_join_release(&fixture, &outcome);

    CHECK(outcome.kind == PSTVNC_MPEG_WORKER_OUTCOME_STOPPED);
    CHECK(outcome.decoder_result == PSTVNC_MPEG_DECODER_STOPPED);
    CHECK(outcome.decoder_report.pictures_decoded == 1u);
}

static void test_stop_claimed_waits_for_exact_release(void)
{
    fixture_t fixture;
    pstvnc_mpeg_worker_outcome_t outcome;

    fixture_init(&fixture);
    fixture.platform.ready_count = 4;
    fixture.event.mode = FAKE_EVENT_STOP_CLAIMED;
    fixture_start(&fixture);
    fixture_run_worker(&fixture);

    CHECK(fixture.event.wait_calls == 1);
    CHECK(fixture.event.retained_consumes == 1);
    CHECK(fixture.platform.picture_calls == 1);
    CHECK(fixture.platform.destroy_calls == 1);

    fixture_join_release(&fixture, &outcome);

    CHECK(outcome.kind == PSTVNC_MPEG_WORKER_OUTCOME_STOPPED);
    CHECK(outcome.decoder_result == PSTVNC_MPEG_DECODER_STOPPED);
    CHECK(outcome.decoder_report.pictures_decoded == 1u);
}

static void test_stop_before_decoder_call(void)
{
    fixture_t fixture;
    pstvnc_mpeg_worker_outcome_t outcome;

    fixture_init(&fixture);
    fixture.platform.ready_count = 2;
    fixture_start(&fixture);

    CHECK(pstvnc_mpeg_worker_request_stop(
        &fixture.worker,
        TEST_GENERATION) == PSTVNC_MPEG_WORKER_OK);

    fixture_run_worker(&fixture);

    CHECK(fixture.platform.picture_calls == 0);
    CHECK(fixture.event.wait_calls == 0);

    fixture_join_release(&fixture, &outcome);

    CHECK(outcome.kind == PSTVNC_MPEG_WORKER_OUTCOME_STOPPED);
    CHECK(outcome.decoder_result == PSTVNC_MPEG_DECODER_STOPPED);
    CHECK(outcome.decoder_report.pictures_decoded == 0u);
}

static void test_stop_during_decoder_call_suppresses_publication(void)
{
    fixture_t fixture;
    pstvnc_mpeg_worker_frame_t frame;
    pstvnc_mpeg_worker_outcome_t outcome;

    fixture_init(&fixture);
    fixture.platform.ready_count = 2;
    fixture.platform.stop_on_call = 1;
    fixture_start(&fixture);
    fixture_run_worker(&fixture);

    CHECK(fixture.platform.picture_calls == 1);
    CHECK(fixture.event.wait_calls == 0);
    CHECK(pstvnc_mpeg_worker_claim(
        &fixture.worker,
        TEST_GENERATION,
        &frame) == PSTVNC_MPEG_WORKER_FRAME_UNAVAILABLE);

    fixture_join_release(&fixture, &outcome);

    CHECK(outcome.kind == PSTVNC_MPEG_WORKER_OUTCOME_STOPPED);
    CHECK(outcome.decoder_result == PSTVNC_MPEG_DECODER_STOPPED);
    CHECK(outcome.decoder_report.pictures_decoded == 1u);
    CHECK(g_transport.read_calls == 0);
}

static void test_later_failure_not_masked_by_earlier_frame(void)
{
    fixture_t fixture;
    pstvnc_mpeg_worker_outcome_t outcome;

    fixture_init(&fixture);
    fixture.platform.ready_count = 3;
    fixture.platform.fail_call = 2;
    fixture.event.mode = FAKE_EVENT_RELEASE_EACH;
    fixture_start(&fixture);
    fixture_run_worker(&fixture);

    CHECK(fixture.platform.picture_calls == 2);
    CHECK(fixture.event.wait_calls == 1);
    CHECK(fixture.event.hook_calls == 1);

    fixture_join_release(&fixture, &outcome);

    CHECK(outcome.kind == PSTVNC_MPEG_WORKER_OUTCOME_FAILED);
    CHECK(outcome.decoder_result == PSTVNC_MPEG_DECODER_PICTURE_FAILED);
    CHECK(outcome.decoder_report.pictures_decoded == 1u);
    CHECK(outcome.decoder_release_result == PSTVNC_MPEG_DECODER_COMPLETE);
}

int main(void)
{
    test_one_slot_no_run_ahead_and_lost_wake();
    test_stop_discards_available_frame();
    test_stop_claimed_waits_for_exact_release();
    test_stop_before_decoder_call();
    test_stop_during_decoder_call_suppresses_publication();
    test_later_failure_not_masked_by_earlier_frame();

    if (failures != 0) {
        fprintf(stderr, "mpeg_worker_test: FAIL (%d)\n", failures);
        return 1;
    }

    puts("MPEG_WORKER_TEST=PASS");
    return 0;
}
