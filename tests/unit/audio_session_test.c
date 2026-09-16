/*
 * File synopsis:
 * Exercises the A002 audio-session worker lifecycle with deterministic memory,
 * thread, synchronization, Transport-activity, clock, and playback fakes. The
 * fixture proves explicit resource authority, non-consuming reservoir waits,
 * future common-clock publication/deadline gating, cancellation, exact playback
 * result propagation, and join-before-reclaim fencing without a real PS2 thread.
 */

#include "audio/session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int test_failures;

#define TEST_CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            test_failures += 1; \
        } \
    } while (0)

typedef struct fake_memory_record {
    void *raw;
    void *aligned;
    size_t size;
    size_t alignment;
    int released;
} fake_memory_record_t;

typedef struct fake_memory {
    fake_memory_record_t records[4];
    int allocate_calls;
    int release_calls;
    int fail_allocate_call;
} fake_memory_t;

typedef struct fake_thread {
    pstvnc_audio_session_thread_entry_t entry;
    void *argument;
    void *stack;
    size_t stack_bytes;
    int priority;
    int thread_id;
    int create_calls;
    int start_calls;
    int join_calls;
    int destroy_calls;
    int fail_create;
    int fail_start;
    int fail_join;
    int fail_destroy;
    int run_on_start;
    int run_on_join;
    int entry_ran;
} fake_thread_t;

typedef struct fake_sync {
    int locked;
    int lock_calls;
    int unlock_calls;
    int fail_lock;
    int fail_unlock;
} fake_sync_t;

typedef struct fake_transport {
    uint32_t activity_sequence;
    size_t available_count;
    int producer_done;
    int snapshot_calls;
    int status_calls;
    pstvnc_transport_result_t snapshot_result;
    pstvnc_transport_result_t status_result;
} fake_transport_t;

typedef struct fake_time {
    uint64_t now_tick;
    int read_calls;
    int delay_calls;
    int reservoir_delay_calls;
    int clock_delay_calls;
    int fail_read;
    int fail_delay_call;
    uint32_t reservoir_poll_us;
    uint32_t clock_poll_us;
    int transport_change_on_reservoir_call;
    size_t transport_new_available;
    int transport_new_done;
    pstvnc_audio_session_t *stop_session;
    int stop_on_reservoir_call;
    int stop_on_clock_call;
    pstvnc_media_clock_t *publish_clock;
    int publish_on_clock_call;
    uint64_t publish_epoch;
} fake_time_t;

typedef struct fake_playback {
    int calls;
    pstvnc_audio_playback_result_t result;
    pstvnc_config_pcm_profile_t observed_profile;
    uint8_t *observed_buffer;
    size_t observed_capacity;
    const pstvnc_audio_service_ops_t *observed_service;
    uint64_t observed_now_tick;
    pstvnc_audio_playback_report_t report;
} fake_playback_t;

static fake_transport_t g_transport;
static fake_time_t g_time;
static fake_playback_t g_playback;
static int g_clock_forced_result;
static int g_clock_arm_calls;

static void reset_globals(void)
{
    memset(&g_transport, 0, sizeof(g_transport));
    memset(&g_time, 0, sizeof(g_time));
    memset(&g_playback, 0, sizeof(g_playback));
    g_transport.snapshot_result = PSTVNC_TRANSPORT_OK;
    g_transport.status_result = PSTVNC_TRANSPORT_OK;
    g_playback.result = PSTVNC_AUDIO_PLAYBACK_COMPLETE;
    g_playback.report.submitted_bytes = 123u;
    g_playback.report.submitted_chunks = 3u;
    g_playback.report.cleanup_attempted = 1;
    g_clock_forced_result = 9999;
    g_clock_arm_calls = 0;
}

static void *fake_allocate(void *context, size_t byte_count, size_t alignment)
{
    fake_memory_t *memory = (fake_memory_t *)context;
    fake_memory_record_t *record;
    uintptr_t address;
    void *raw;
    size_t extra;

    memory->allocate_calls += 1;
    if (memory->fail_allocate_call == memory->allocate_calls)
        return NULL;

    TEST_CHECK(memory->allocate_calls <= 4);
    record = &memory->records[memory->allocate_calls - 1];
    extra = alignment > 1u ? alignment - 1u : 0u;
    raw = malloc(byte_count + extra);
    TEST_CHECK(raw != NULL);
    if (raw == NULL)
        return NULL;

    address = (uintptr_t)raw;
    if (alignment > 1u)
        address = (address + alignment - 1u) &
            ~((uintptr_t)alignment - 1u);

    record->raw = raw;
    record->aligned = (void *)address;
    record->size = byte_count;
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

    TEST_CHECK(0 && "release must target one live allocation");
}

static int fake_thread_create(
    void *context,
    pstvnc_audio_session_thread_entry_t entry,
    void *argument,
    void *stack,
    size_t stack_bytes,
    int priority,
    int *thread_id)
{
    fake_thread_t *thread = (fake_thread_t *)context;

    thread->create_calls += 1;
    thread->entry = entry;
    thread->argument = argument;
    thread->stack = stack;
    thread->stack_bytes = stack_bytes;
    thread->priority = priority;

    if (thread->fail_create)
        return -1;

    thread->thread_id = 41;
    *thread_id = thread->thread_id;
    return 0;
}

static void fake_thread_run_entry(fake_thread_t *thread)
{
    if (!thread->entry_ran) {
        thread->entry_ran = 1;
        thread->entry(thread->argument);
    }
}

static int fake_thread_start(void *context, int thread_id)
{
    fake_thread_t *thread = (fake_thread_t *)context;

    thread->start_calls += 1;
    TEST_CHECK(thread_id == thread->thread_id);
    if (thread->fail_start)
        return -1;
    if (thread->run_on_start)
        fake_thread_run_entry(thread);
    return 0;
}

static int fake_thread_join(void *context, int thread_id)
{
    fake_thread_t *thread = (fake_thread_t *)context;

    thread->join_calls += 1;
    TEST_CHECK(thread_id == thread->thread_id);
    if (thread->fail_join)
        return -1;
    if (thread->run_on_join)
        fake_thread_run_entry(thread);
    return 0;
}

static int fake_thread_destroy(void *context, int thread_id)
{
    fake_thread_t *thread = (fake_thread_t *)context;

    thread->destroy_calls += 1;
    TEST_CHECK(thread_id == thread->thread_id);
    return thread->fail_destroy ? -1 : 0;
}

static int fake_sync_lock(void *context)
{
    fake_sync_t *sync = (fake_sync_t *)context;

    sync->lock_calls += 1;
    if (sync->fail_lock || sync->locked)
        return -1;
    sync->locked = 1;
    return 0;
}

static int fake_sync_unlock(void *context)
{
    fake_sync_t *sync = (fake_sync_t *)context;

    sync->unlock_calls += 1;
    if (!sync->locked)
        return -1;
    sync->locked = 0;
    return sync->fail_unlock ? -1 : 0;
}

pstvnc_transport_result_t pstvnc_transport_audio_activity_snapshot(
    uint32_t *activity_sequence)
{
    g_transport.snapshot_calls += 1;
    if (g_transport.snapshot_result != PSTVNC_TRANSPORT_OK)
        return g_transport.snapshot_result;
    *activity_sequence = g_transport.activity_sequence;
    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_audio_status(
    size_t *available_count,
    int *producer_done)
{
    g_transport.status_calls += 1;
    if (g_transport.status_result != PSTVNC_TRANSPORT_OK)
        return g_transport.status_result;
    *available_count = g_transport.available_count;
    *producer_done = g_transport.producer_done;
    return PSTVNC_TRANSPORT_OK;
}

static int fake_read_ticks(void *context, uint64_t *ticks)
{
    fake_time_t *time = (fake_time_t *)context;

    time->read_calls += 1;
    if (time->fail_read)
        return -1;
    *ticks = time->now_tick;
    return 0;
}

static int fake_delay_us(void *context, uint32_t delay_us)
{
    fake_time_t *time = (fake_time_t *)context;
    int relevant_call = 0;

    time->delay_calls += 1;

    if (delay_us == time->reservoir_poll_us) {
        time->reservoir_delay_calls += 1;
        relevant_call = time->reservoir_delay_calls;
        if (time->stop_session != NULL &&
            time->stop_on_reservoir_call == relevant_call) {
            TEST_CHECK(
                pstvnc_audio_session_request_stop(time->stop_session) ==
                PSTVNC_AUDIO_SESSION_OK);
        }
        if (time->transport_change_on_reservoir_call == relevant_call) {
            g_transport.available_count = time->transport_new_available;
            g_transport.producer_done = time->transport_new_done;
            g_transport.activity_sequence += 1u;
        }
    }

    if (delay_us == time->clock_poll_us) {
        time->clock_delay_calls += 1;
        relevant_call = time->clock_delay_calls;
        if (time->stop_session != NULL &&
            time->stop_on_clock_call == relevant_call) {
            TEST_CHECK(
                pstvnc_audio_session_request_stop(time->stop_session) ==
                PSTVNC_AUDIO_SESSION_OK);
        }
        if (time->publish_clock != NULL &&
            time->publish_on_clock_call == relevant_call) {
            time->publish_clock->epoch_tick = time->publish_epoch;
            time->publish_clock->armed = 1;
        }
    }

    if (time->fail_delay_call == time->delay_calls)
        return -1;

    time->now_tick += (uint64_t)delay_us;
    return 0;
}

/*
 * Deterministic host seam mirroring the already-tested media-clock wait
 * contract so this fixture focuses on audio-session composition. Production
 * session.c calls the real pstvnc_media_clock_wait_audio().
 */
int pstvnc_media_clock_wait_audio(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint32_t poll_us,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_media_clock_stop_observer_t *stop_observer)
{
    uint64_t deadline;

    if (g_clock_forced_result != 9999)
        return g_clock_forced_result;

    while (!clock->armed) {
        if (stop_observer != NULL &&
            stop_observer->is_stop_requested(stop_observer->context))
            return PSTVNC_MEDIA_CLOCK_STOPPED;
        if (time_ops->delay_us(time_ops->context, poll_us) != 0)
            return PSTVNC_MEDIA_CLOCK_DELAY_FAILED;
    }

    if (clock->profile.audio_presentation_offset_us < 0) {
        uint64_t magnitude =
            (uint64_t)(-(int64_t)clock->profile.audio_presentation_offset_us);
        deadline = magnitude > clock->epoch_tick ?
            0u : clock->epoch_tick - magnitude;
    } else {
        uint64_t offset =
            (uint64_t)clock->profile.audio_presentation_offset_us;
        deadline = UINT64_MAX - clock->epoch_tick < offset ?
            UINT64_MAX : clock->epoch_tick + offset;
    }

    if (UINT64_MAX - deadline < additional_ticks)
        deadline = UINT64_MAX;
    else
        deadline += additional_ticks;

    for (;;) {
        uint64_t now_tick = 0u;

        if (stop_observer != NULL &&
            stop_observer->is_stop_requested(stop_observer->context))
            return PSTVNC_MEDIA_CLOCK_STOPPED;
        if (time_ops->read_ticks(time_ops->context, &now_tick) != 0)
            return PSTVNC_MEDIA_CLOCK_TIMER_FAILED;
        if (now_tick >= deadline)
            return PSTVNC_MEDIA_CLOCK_OK;
        if (time_ops->delay_us(time_ops->context, poll_us) != 0)
            return PSTVNC_MEDIA_CLOCK_DELAY_FAILED;
    }
}

int pstvnc_media_clock_arm(
    pstvnc_media_clock_t *clock,
    uint64_t observed_now_tick)
{
    (void)clock;
    (void)observed_now_tick;
    g_clock_arm_calls += 1;
    return PSTVNC_MEDIA_CLOCK_OK;
}

pstvnc_audio_playback_result_t pstvnc_audio_playback_run(
    const pstvnc_config_pcm_profile_t *profile,
    uint8_t *buffer,
    size_t buffer_capacity,
    const pstvnc_audio_service_ops_t *service,
    pstvnc_audio_playback_report_t *report)
{
    g_playback.calls += 1;
    g_playback.observed_profile = *profile;
    g_playback.observed_buffer = buffer;
    g_playback.observed_capacity = buffer_capacity;
    g_playback.observed_service = service;
    g_playback.observed_now_tick = g_time.now_tick;
    *report = g_playback.report;
    return g_playback.result;
}

static int service_ok0(void *context) { (void)context; return 0; }
static int service_ok_format(void *context, uint32_t a, uint32_t b, uint32_t c)
{ (void)context; (void)a; (void)b; (void)c; return 0; }
static int service_ok_volume(void *context, uint32_t value)
{ (void)context; (void)value; return 0; }
static int service_ok_wait(void *context, size_t value)
{ (void)context; (void)value; return 0; }
static int service_ok_play(void *context, const uint8_t *bytes, size_t value)
{ (void)context; (void)bytes; (void)value; return 0; }

typedef struct fixture {
    pstvnc_audio_session_t session;
    pstvnc_audio_session_values_t values;
    pstvnc_config_pcm_profile_t pcm;
    pstvnc_audio_service_ops_t service;
    pstvnc_media_clock_t clock;
    pstvnc_audio_session_memory_ops_t memory_ops;
    pstvnc_audio_session_thread_ops_t thread_ops;
    pstvnc_audio_session_sync_t sync_ops;
    pstvnc_media_clock_time_ops_t time_ops;
    fake_memory_t memory;
    fake_thread_t thread;
    fake_sync_t sync;
} fixture_t;

static void fixture_init(fixture_t *fixture)
{
    memset(fixture, 0, sizeof(*fixture));
    reset_globals();

    fixture->values.worker_stack_bytes = 4096u;
    fixture->values.worker_priority = 37;
    fixture->values.playback_buffer_capacity = 256u;
    fixture->values.startup_reservoir_bytes = 64u;
    fixture->values.reservoir_poll_us = 7u;
    fixture->values.clock_poll_us = 10u;

    fixture->pcm.rate_hz = 44100u;
    fixture->pcm.channels = 2u;
    fixture->pcm.bits_per_sample = 16u;
    fixture->pcm.volume_percent = 73u;

    fixture->service.initialize = service_ok0;
    fixture->service.set_format = service_ok_format;
    fixture->service.set_volume = service_ok_volume;
    fixture->service.wait_audio = service_ok_wait;
    fixture->service.play_audio = service_ok_play;
    fixture->service.stop_audio = service_ok0;
    fixture->service.context = fixture;

    fixture->clock.profile.audio_presentation_offset_us = 25;
    fixture->clock.ticks_per_second = 1000000u;
    fixture->clock.armed = 1;

    fixture->memory_ops.allocate = fake_allocate;
    fixture->memory_ops.release = fake_release;
    fixture->memory_ops.context = &fixture->memory;
    fixture->thread_ops.create = fake_thread_create;
    fixture->thread_ops.start = fake_thread_start;
    fixture->thread_ops.join = fake_thread_join;
    fixture->thread_ops.destroy = fake_thread_destroy;
    fixture->thread_ops.context = &fixture->thread;
    fixture->sync_ops.lock = fake_sync_lock;
    fixture->sync_ops.unlock = fake_sync_unlock;
    fixture->sync_ops.context = &fixture->sync;
    fixture->time_ops.read_ticks = fake_read_ticks;
    fixture->time_ops.delay_us = fake_delay_us;
    fixture->time_ops.context = &g_time;

    g_time.reservoir_poll_us = fixture->values.reservoir_poll_us;
    g_time.clock_poll_us = fixture->values.clock_poll_us;
    g_time.now_tick = 100u;
    g_transport.activity_sequence = 1u;
    g_transport.available_count = fixture->values.startup_reservoir_bytes;
    fixture->thread.run_on_start = 1;
}

static pstvnc_audio_session_result_t fixture_start(fixture_t *fixture)
{
    g_time.stop_session = &fixture->session;
    return pstvnc_audio_session_start(
        &fixture->session,
        &fixture->values,
        &fixture->pcm,
        &fixture->service,
        &fixture->clock,
        &fixture->time_ops,
        &fixture->memory_ops,
        &fixture->thread_ops,
        &fixture->sync_ops);
}

static pstvnc_audio_session_outcome_t fixture_join_outcome(fixture_t *fixture)
{
    pstvnc_audio_session_outcome_t outcome;
    memset(&outcome, 0, sizeof(outcome));
    TEST_CHECK(pstvnc_audio_session_join(&fixture->session) == PSTVNC_AUDIO_SESSION_OK);
    TEST_CHECK(pstvnc_audio_session_outcome(&fixture->session, &outcome) == PSTVNC_AUDIO_SESSION_OK);
    return outcome;
}

static void fixture_release(fixture_t *fixture)
{
    TEST_CHECK(pstvnc_audio_session_release(&fixture->session) == PSTVNC_AUDIO_SESSION_OK);
}

static void test_explicit_authority_and_reclaim_fence(void)
{
    fixture_t fixture;
    pstvnc_audio_session_outcome_t outcome;

    fixture_init(&fixture);
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    TEST_CHECK(fixture.memory.records[0].size == 256u);
    TEST_CHECK(fixture.memory.records[1].size == 4096u);
    TEST_CHECK(fixture.memory.records[1].alignment == PSTVNC_AUDIO_SESSION_STACK_ALIGNMENT);
    TEST_CHECK(fixture.thread.stack_bytes == 4096u);
    TEST_CHECK(fixture.thread.priority == 37);
    TEST_CHECK(pstvnc_audio_session_release(&fixture.session) == PSTVNC_AUDIO_SESSION_WORKER_LIVE);
    TEST_CHECK(fixture.memory.release_calls == 0);

    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK);
    TEST_CHECK(outcome.playback_result == PSTVNC_AUDIO_PLAYBACK_COMPLETE);
    TEST_CHECK(g_playback.observed_capacity == 256u);
    TEST_CHECK(g_playback.observed_profile.rate_hz == 44100u);
    TEST_CHECK(g_playback.observed_profile.volume_percent == 73u);
    fixture_release(&fixture);
    TEST_CHECK(fixture.memory.release_calls == 2);
    TEST_CHECK(pstvnc_audio_session_release(&fixture.session) == PSTVNC_AUDIO_SESSION_OK);
    TEST_CHECK(fixture.memory.release_calls == 2);
}

static void test_allocation_create_start_join_failures(void)
{
    fixture_t fixture;

    fixture_init(&fixture);
    fixture.memory.fail_allocate_call = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_BUFFER_ALLOCATION_FAILED);

    fixture_init(&fixture);
    fixture.memory.fail_allocate_call = 2;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_STACK_ALLOCATION_FAILED);
    TEST_CHECK(fixture.memory.release_calls == 1);

    fixture_init(&fixture);
    fixture.thread.fail_create = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_THREAD_CREATE_FAILED);
    TEST_CHECK(fixture.memory.release_calls == 2);

    fixture_init(&fixture);
    fixture.thread.fail_start = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_THREAD_START_FAILED);
    TEST_CHECK(fixture.thread.destroy_calls == 1);
    TEST_CHECK(fixture.memory.release_calls == 2);

    fixture_init(&fixture);
    fixture.thread.run_on_start = 0;
    fixture.thread.run_on_join = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    fixture.thread.fail_join = 1;
    TEST_CHECK(pstvnc_audio_session_join(&fixture.session) == PSTVNC_AUDIO_SESSION_THREAD_JOIN_FAILED);
    TEST_CHECK(fixture.memory.release_calls == 0);
    fixture.thread.fail_join = 0;
    TEST_CHECK(pstvnc_audio_session_join(&fixture.session) == PSTVNC_AUDIO_SESSION_OK);
    fixture_release(&fixture);
}

static void test_reservoir_activity_short_final_and_empty(void)
{
    fixture_t fixture;
    pstvnc_audio_session_outcome_t outcome;

    fixture_init(&fixture);
    g_transport.available_count = 12u;
    g_time.transport_change_on_reservoir_call = 2;
    g_time.transport_new_available = 64u;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK);
    TEST_CHECK(outcome.reservoir_bytes == 64u);
    TEST_CHECK(g_time.reservoir_delay_calls == 2);
    TEST_CHECK(g_transport.status_calls == 2);
    TEST_CHECK(g_transport.snapshot_calls == 3);
    fixture_release(&fixture);

    fixture_init(&fixture);
    g_transport.available_count = 10u;
    g_time.transport_change_on_reservoir_call = 1;
    g_time.transport_new_available = 21u;
    g_time.transport_new_done = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK);
    TEST_CHECK(outcome.reservoir_bytes == 21u);
    TEST_CHECK(outcome.producer_done == 1);
    fixture_release(&fixture);

    fixture_init(&fixture);
    g_transport.available_count = 0u;
    g_transport.producer_done = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY);
    TEST_CHECK(g_playback.calls == 0);
    fixture_release(&fixture);
}

static void test_unarmed_publication_deadline_and_no_audio_arm(void)
{
    fixture_t fixture;
    pstvnc_audio_session_outcome_t outcome;

    fixture_init(&fixture);
    fixture.clock.armed = 0;
    g_time.now_tick = 0u;
    g_time.publish_clock = &fixture.clock;
    g_time.publish_on_clock_call = 1;
    g_time.publish_epoch = 30u;

    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK);
    TEST_CHECK(g_playback.observed_now_tick >= 55u);
    TEST_CHECK(fixture.clock.epoch_tick == 30u);
    TEST_CHECK(g_clock_arm_calls == 0);
    fixture_release(&fixture);
}

static void test_stop_and_clock_failures(void)
{
    fixture_t fixture;
    pstvnc_audio_session_outcome_t outcome;

    fixture_init(&fixture);
    g_transport.available_count = 1u;
    g_time.stop_on_reservoir_call = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED);
    TEST_CHECK(g_playback.calls == 0);
    fixture_release(&fixture);

    fixture_init(&fixture);
    fixture.clock.armed = 0;
    g_time.stop_on_clock_call = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED);
    TEST_CHECK(g_playback.calls == 0);
    fixture_release(&fixture);

    fixture_init(&fixture);
    g_clock_forced_result = PSTVNC_MEDIA_CLOCK_SYNC_FAILED;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_CLOCK);
    TEST_CHECK(outcome.clock_result == PSTVNC_MEDIA_CLOCK_SYNC_FAILED);
    fixture_release(&fixture);

    fixture_init(&fixture);
    fixture.clock.armed = 1;
    fixture.clock.epoch_tick = 200u;
    g_time.now_tick = 100u;
    g_time.fail_read = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.clock_result == PSTVNC_MEDIA_CLOCK_TIMER_FAILED);
    fixture_release(&fixture);

    fixture_init(&fixture);
    fixture.clock.armed = 0;
    g_time.fail_delay_call = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.clock_result == PSTVNC_MEDIA_CLOCK_DELAY_FAILED);
    fixture_release(&fixture);
}

static void test_transport_and_playback_results_preserved(void)
{
    fixture_t fixture;
    pstvnc_audio_session_outcome_t outcome;

    fixture_init(&fixture);
    g_transport.status_result = PSTVNC_TRANSPORT_CLOSED;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_TRANSPORT);
    TEST_CHECK(outcome.transport_result == PSTVNC_TRANSPORT_CLOSED);
    TEST_CHECK(g_playback.calls == 0);
    fixture_release(&fixture);

    fixture_init(&fixture);
    g_playback.result = PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED;
    g_playback.report.submitted_bytes = 77u;
    g_playback.report.submitted_chunks = 2u;
    g_playback.report.cleanup_failed = 1;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_OK);
    outcome = fixture_join_outcome(&fixture);
    TEST_CHECK(outcome.kind == PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK);
    TEST_CHECK(outcome.playback_result == PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED);
    TEST_CHECK(outcome.playback_report.submitted_bytes == 77u);
    TEST_CHECK(outcome.playback_report.cleanup_failed == 1);
    fixture_release(&fixture);
}

static void test_zero_authority_rejected(void)
{
    fixture_t fixture;

    fixture_init(&fixture);
    fixture.values.worker_stack_bytes = 0u;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_INVALID);
    fixture_init(&fixture);
    fixture.values.playback_buffer_capacity = 0u;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_INVALID);
    fixture_init(&fixture);
    fixture.values.startup_reservoir_bytes = 0u;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_INVALID);
    fixture_init(&fixture);
    fixture.values.reservoir_poll_us = 0u;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_INVALID);
    fixture_init(&fixture);
    fixture.values.clock_poll_us = 0u;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_INVALID);
    fixture_init(&fixture);
    fixture.values.worker_priority = 0;
    TEST_CHECK(fixture_start(&fixture) == PSTVNC_AUDIO_SESSION_INVALID);
}

int main(void)
{
    test_explicit_authority_and_reclaim_fence();
    test_allocation_create_start_join_failures();
    test_reservoir_activity_short_final_and_empty();
    test_unarmed_publication_deadline_and_no_audio_arm();
    test_stop_and_clock_failures();
    test_transport_and_playback_results_preserved();
    test_zero_authority_rejected();

    if (test_failures != 0) {
        fprintf(stderr, "audio_session_test: %d failure(s)\n", test_failures);
        return 1;
    }

    puts("audio_session_test: PASS");
    return 0;
}
