/*
 * File synopsis:
 * Deterministic host fixture for A003's synchronous MPEG decoder owner. Injected
 * memory/synchronization/platform seams and a scripted public Transport MPEG
 * seam prove explicit geometry/feed bounds, event-driven starvation wakeups,
 * truthful finite exhaustion/failure, payload-versus-padding accounting,
 * decoder-call reclaim fencing, and the critical no-synthetic-EOF stop rule.
 */

#include "mpeg/decoder.h"
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

typedef struct fake_memory_record {
    void *raw;
    void *aligned;
    size_t byte_count;
    size_t alignment;
    int released;
} fake_memory_record_t;

typedef struct fake_memory {
    fake_memory_record_t records[4];
    int allocate_calls;
    int release_calls;
    int fail_allocate_call;
} fake_memory_t;

typedef struct fake_sync {
    int locked;
    int fail_lock;
    int fail_unlock;
} fake_sync_t;

typedef enum fake_picture_mode {
    FAKE_PICTURE_STREAM = 0,
    FAKE_PICTURE_STOP_THEN_FEED = 1,
    FAKE_PICTURE_UNEXPECTED_END = 2,
    FAKE_PICTURE_BAD_SEQUENCE = 3,
    FAKE_PICTURE_RELEASE_THEN_STOP_FEED = 4
} fake_picture_mode_t;

typedef struct fake_platform {
    pstvnc_mpeg_feed_callback_t feed_callback;
    void *feed_context;
    pstvnc_mpeg_sequence_callback_t sequence_callback;
    void *sequence_context;
    pstvnc_mpeg_decoder_t *decoder;
    fake_picture_mode_t picture_mode;
    int prepare_calls;
    int initialize_calls;
    int picture_calls;
    int submit_calls;
    int destroy_calls;
    int release_state_calls;
    int fail_prepare;
    int fail_initialize;
    int fail_submit;
    int fail_destroy;
    int fail_release_state;
    size_t last_payload_bytes;
    size_t last_transfer_bytes;
    uint8_t last_transfer[128];
    pstvnc_mpeg_decoder_result_t active_release_result;
} fake_platform_t;

typedef struct fake_transport {
    pstvnc_transport_result_t acquire_result;
    pstvnc_transport_result_t forced_result;
    uint8_t payload[64];
    size_t payload_length;
    int would_block_first;
    int read_calls;
    int wait_calls;
    int data_delivered;
    uint32_t activity_sequence;
} fake_transport_t;

typedef struct fixture {
    pstvnc_mpeg_decoder_t decoder;
    pstvnc_mpeg_decoder_config_t config;
    pstvnc_mpeg_decoder_memory_ops_t memory_ops;
    pstvnc_mpeg_decoder_sync_ops_t sync_ops;
    pstvnc_mpeg_decoder_platform_ops_t platform_ops;
    fake_memory_t memory;
    fake_sync_t sync;
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

    memory->allocate_calls += 1;
    if (memory->fail_allocate_call == memory->allocate_calls)
        return NULL;

    CHECK(memory->allocate_calls <= 4);
    record = &memory->records[memory->allocate_calls - 1];
    extra = alignment > 1u ? alignment - 1u : 0u;
    raw = malloc(byte_count + extra);
    if (raw == NULL)
        return NULL;

    address = (uintptr_t)raw;
    if (alignment > 1u)
        address = (address + alignment - 1u) & ~((uintptr_t)alignment - 1u);

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

    CHECK(0 && "release must target a live allocation");
}

static int fake_lock(void *context)
{
    fake_sync_t *sync = (fake_sync_t *)context;

    if (sync->fail_lock || sync->locked)
        return -1;
    sync->locked = 1;
    return 0;
}

static int fake_unlock(void *context)
{
    fake_sync_t *sync = (fake_sync_t *)context;

    if (!sync->locked)
        return -1;
    sync->locked = 0;
    return sync->fail_unlock ? -1 : 0;
}

pstvnc_transport_result_t pstvnc_transport_access_acquire(
    pstvnc_transport_access_t *transport_access)
{
    if (transport_access == NULL)
        return PSTVNC_TRANSPORT_INVALID;
    if (g_transport.acquire_result != PSTVNC_TRANSPORT_OK)
        return g_transport.acquire_result;

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
    if (read_count == NULL || buffer == NULL || maximum_count == 0u)
        return PSTVNC_TRANSPORT_INVALID;
    *read_count = 0u;

    if (g_transport.forced_result != PSTVNC_TRANSPORT_OK)
        return g_transport.forced_result;

    if (g_transport.would_block_first && g_transport.read_calls == 1)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    if (!g_transport.data_delivered && g_transport.payload_length != 0u) {
        CHECK(g_transport.payload_length <= maximum_count);
        if (g_transport.payload_length > maximum_count)
            return PSTVNC_TRANSPORT_FAILED;
        memcpy(buffer, g_transport.payload, g_transport.payload_length);
        *read_count = g_transport.payload_length;
        g_transport.data_delivered = 1;
        return PSTVNC_TRANSPORT_OK;
    }

    return PSTVNC_TRANSPORT_EXHAUSTED;
}

static int fake_prepare_known_state(void *context)
{
    fake_platform_t *platform = (fake_platform_t *)context;
    platform->prepare_calls += 1;
    return platform->fail_prepare ? -1 : 0;
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
    return platform->fail_initialize ? -1 : 0;
}

static int fake_submit_feed(
    void *context,
    const uint8_t *bytes,
    size_t payload_bytes,
    size_t transfer_bytes)
{
    fake_platform_t *platform = (fake_platform_t *)context;

    platform->submit_calls += 1;
    platform->last_payload_bytes = payload_bytes;
    platform->last_transfer_bytes = transfer_bytes;
    CHECK(transfer_bytes <= sizeof(platform->last_transfer));
    if (transfer_bytes <= sizeof(platform->last_transfer))
        memcpy(platform->last_transfer, bytes, transfer_bytes);
    return platform->fail_submit ? -1 : 0;
}

static int fake_destroy(void *context)
{
    fake_platform_t *platform = (fake_platform_t *)context;
    platform->destroy_calls += 1;
    return platform->fail_destroy ? -1 : 0;
}

static int fake_release_known_state(void *context)
{
    fake_platform_t *platform = (fake_platform_t *)context;
    platform->release_state_calls += 1;
    return platform->fail_release_state ? -1 : 0;
}

static void fake_accept_sequence(fake_platform_t *platform)
{
    size_t capacity = 0u;
    void *picture = platform->sequence_callback(
        platform->sequence_context,
        64u,
        32u,
        &capacity);

    CHECK(picture != NULL);
    CHECK(capacity == 4096u);
}

static int fake_picture(void *context)
{
    fake_platform_t *platform = (fake_platform_t *)context;
    int feed_result;

    platform->picture_calls += 1;

    if (platform->picture_mode == FAKE_PICTURE_UNEXPECTED_END)
        return 0;

    if (platform->picture_mode == FAKE_PICTURE_BAD_SEQUENCE) {
        size_t capacity = 0u;
        CHECK(platform->sequence_callback(
            platform->sequence_context,
            65u,
            32u,
            &capacity) == NULL);
        return -1;
    }

    if (platform->picture_calls == 1)
        fake_accept_sequence(platform);

    if (platform->picture_mode == FAKE_PICTURE_STOP_THEN_FEED &&
        platform->picture_calls == 1) {
        CHECK(pstvnc_mpeg_decoder_request_stop(platform->decoder) ==
            PSTVNC_MPEG_DECODER_COMPLETE);
    }

    if (platform->picture_mode == FAKE_PICTURE_RELEASE_THEN_STOP_FEED &&
        platform->picture_calls == 1) {
        platform->active_release_result =
            pstvnc_mpeg_decoder_release(platform->decoder);
        CHECK(pstvnc_mpeg_decoder_request_stop(platform->decoder) ==
            PSTVNC_MPEG_DECODER_COMPLETE);
    }

    feed_result = platform->feed_callback(platform->feed_context);
    if (feed_result < 0)
        return -1;
    if (feed_result == 0)
        return 0;
    return 1;
}

static void fixture_init(fixture_t *fixture)
{
    memset(fixture, 0, sizeof(*fixture));
    memset(&g_transport, 0, sizeof(g_transport));

    fixture->config.max_width = 64u;
    fixture->config.max_height = 32u;
    fixture->config.bytes_per_pixel = 2u;
    fixture->config.feed_payload_capacity = 17u;
    fixture->config.transfer_alignment = 16u;
    fixture->config.buffer_alignment = 64u;

    fixture->memory_ops.allocate = fake_allocate;
    fixture->memory_ops.release = fake_release;
    fixture->memory_ops.context = &fixture->memory;

    fixture->sync_ops.lock = fake_lock;
    fixture->sync_ops.unlock = fake_unlock;
    fixture->sync_ops.context = &fixture->sync;

    fixture->platform.decoder = &fixture->decoder;
    fixture->platform.picture_mode = FAKE_PICTURE_STREAM;
    fixture->platform.active_release_result = PSTVNC_MPEG_DECODER_COMPLETE;

    fixture->platform_ops.prepare_known_state = fake_prepare_known_state;
    fixture->platform_ops.initialize = fake_initialize;
    fixture->platform_ops.picture = fake_picture;
    fixture->platform_ops.submit_feed = fake_submit_feed;
    fixture->platform_ops.destroy = fake_destroy;
    fixture->platform_ops.release_known_state = fake_release_known_state;
    fixture->platform_ops.context = &fixture->platform;

    g_transport.activity_sequence = 3u;
}

static pstvnc_mpeg_decoder_result_t fixture_initialize(fixture_t *fixture)
{
    return pstvnc_mpeg_decoder_initialize(
        &fixture->decoder,
        &fixture->config,
        &fixture->memory_ops,
        &fixture->sync_ops,
        &fixture->platform_ops);
}

static void set_payload(size_t count)
{
    size_t index;

    CHECK(count <= sizeof(g_transport.payload));
    g_transport.payload_length = count;
    for (index = 0u; index < count; ++index)
        g_transport.payload[index] = (uint8_t)(index + 1u);
}

static void test_bounds_padding_and_finite_exhaustion(void)
{
    fixture_t fixture;
    pstvnc_mpeg_decoder_report_t report;
    pstvnc_mpeg_decoder_result_t result;
    size_t index;

    fixture_init(&fixture);
    set_payload(17u);
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(fixture.memory.allocate_calls == 2);
    CHECK(fixture.memory.records[0].byte_count == 32u);
    CHECK(fixture.memory.records[0].alignment == 64u);
    CHECK(fixture.memory.records[1].byte_count == 4096u);
    CHECK(fixture.platform.prepare_calls == 1);
    CHECK(fixture.platform.initialize_calls == 1);

    result = pstvnc_mpeg_decoder_run(&fixture.decoder, &report);
    CHECK(result == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(fixture.platform.picture_calls == 2);
    CHECK(fixture.platform.submit_calls == 1);
    CHECK(fixture.platform.last_payload_bytes == 17u);
    CHECK(fixture.platform.last_transfer_bytes == 32u);
    for (index = 0u; index < 17u; ++index)
        CHECK(fixture.platform.last_transfer[index] == (uint8_t)(index + 1u));
    for (index = 17u; index < 32u; ++index)
        CHECK(fixture.platform.last_transfer[index] == 0u);
    CHECK(report.payload_bytes_consumed == 17u);
    CHECK(report.transfer_bytes_submitted == 32u);
    CHECK(report.feed_callbacks == 1u);
    CHECK(report.pictures_decoded == 1u);
    CHECK(report.transport_result == PSTVNC_TRANSPORT_EXHAUSTED);

    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(fixture.platform.destroy_calls == 1);
    CHECK(fixture.platform.release_state_calls == 1);
    CHECK(fixture.memory.release_calls == 2);
    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(fixture.memory.release_calls == 2);
}

static void test_event_driven_empty_queue_wait(void)
{
    fixture_t fixture;
    pstvnc_mpeg_decoder_report_t report;

    fixture_init(&fixture);
    set_payload(5u);
    g_transport.would_block_first = 1;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(pstvnc_mpeg_decoder_run(&fixture.decoder, &report) ==
        PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(g_transport.wait_calls == 1);
    CHECK(g_transport.read_calls == 3);
    CHECK(report.payload_bytes_consumed == 5u);
    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);
}

static void test_stop_inside_active_picture_does_not_synthesize_eof(void)
{
    fixture_t fixture;
    pstvnc_mpeg_decoder_report_t report;

    fixture_init(&fixture);
    set_payload(7u);
    fixture.platform.picture_mode = FAKE_PICTURE_STOP_THEN_FEED;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_COMPLETE);

    CHECK(pstvnc_mpeg_decoder_run(&fixture.decoder, &report) ==
        PSTVNC_MPEG_DECODER_STOPPED);
    CHECK(fixture.platform.picture_calls == 1);
    CHECK(fixture.platform.submit_calls == 1);
    CHECK(report.payload_bytes_consumed == 7u);
    CHECK(report.transfer_bytes_submitted == 16u);
    CHECK(report.pictures_decoded == 1u);
    CHECK(g_transport.data_delivered == 1);
    CHECK(g_transport.read_calls == 1);

    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);
}

static void test_release_is_fenced_while_picture_call_is_active(void)
{
    fixture_t fixture;
    pstvnc_mpeg_decoder_report_t report;

    fixture_init(&fixture);
    set_payload(3u);
    fixture.platform.picture_mode = FAKE_PICTURE_RELEASE_THEN_STOP_FEED;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_COMPLETE);

    CHECK(pstvnc_mpeg_decoder_run(&fixture.decoder, &report) ==
        PSTVNC_MPEG_DECODER_STOPPED);
    CHECK(fixture.platform.active_release_result ==
        PSTVNC_MPEG_DECODER_CALL_ACTIVE);
    CHECK(fixture.platform.destroy_calls == 0);
    CHECK(fixture.memory.release_calls == 0);
    CHECK(report.payload_bytes_consumed == 3u);

    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(fixture.platform.destroy_calls == 1);
    CHECK(fixture.memory.release_calls == 2);
}

static void test_invalid_sequence_and_unexpected_end_are_failures(void)
{
    fixture_t fixture;
    pstvnc_mpeg_decoder_report_t report;

    fixture_init(&fixture);
    fixture.platform.picture_mode = FAKE_PICTURE_BAD_SEQUENCE;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(pstvnc_mpeg_decoder_run(&fixture.decoder, &report) ==
        PSTVNC_MPEG_DECODER_SEQUENCE_INVALID);
    CHECK(fixture.platform.submit_calls == 0);
    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);

    fixture_init(&fixture);
    fixture.platform.picture_mode = FAKE_PICTURE_UNEXPECTED_END;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(pstvnc_mpeg_decoder_run(&fixture.decoder, &report) ==
        PSTVNC_MPEG_DECODER_UNEXPECTED_END);
    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);
}

static void test_transport_and_platform_failures_propagate(void)
{
    fixture_t fixture;
    pstvnc_mpeg_decoder_report_t report;

    fixture_init(&fixture);
    g_transport.forced_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(pstvnc_mpeg_decoder_run(&fixture.decoder, &report) ==
        PSTVNC_MPEG_DECODER_TRANSPORT_FAILED);
    CHECK(report.transport_result == PSTVNC_TRANSPORT_FAILED);
    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);

    fixture_init(&fixture);
    set_payload(5u);
    fixture.platform.fail_submit = 1;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_COMPLETE);
    CHECK(pstvnc_mpeg_decoder_run(&fixture.decoder, &report) ==
        PSTVNC_MPEG_DECODER_TRANSFER_FAILED);
    CHECK(pstvnc_mpeg_decoder_release(&fixture.decoder) ==
        PSTVNC_MPEG_DECODER_COMPLETE);
}

static void test_explicit_authority_and_setup_failures(void)
{
    fixture_t fixture;

    fixture_init(&fixture);
    fixture.config.feed_payload_capacity = 0u;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_INVALID);
    CHECK(fixture.memory.allocate_calls == 0);

    fixture_init(&fixture);
    fixture.config.transfer_alignment = 0u;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_INVALID);

    fixture_init(&fixture);
    fixture.memory.fail_allocate_call = 1;
    CHECK(fixture_initialize(&fixture) ==
        PSTVNC_MPEG_DECODER_ALLOCATION_FAILED);

    fixture_init(&fixture);
    fixture.platform.fail_prepare = 1;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_PREPARE_FAILED);
    CHECK(fixture.memory.release_calls == 2);

    fixture_init(&fixture);
    fixture.platform.fail_initialize = 1;
    CHECK(fixture_initialize(&fixture) == PSTVNC_MPEG_DECODER_INITIALIZE_FAILED);
    CHECK(fixture.platform.release_state_calls == 1);
    CHECK(fixture.memory.release_calls == 2);
}

int main(void)
{
    test_bounds_padding_and_finite_exhaustion();
    test_event_driven_empty_queue_wait();
    test_stop_inside_active_picture_does_not_synthesize_eof();
    test_release_is_fenced_while_picture_call_is_active();
    test_invalid_sequence_and_unexpected_end_are_failures();
    test_transport_and_platform_failures_propagate();
    test_explicit_authority_and_setup_failures();

    if (failures != 0) {
        fprintf(stderr, "%d MPEG decoder test(s) failed\n", failures);
        return 1;
    }

    puts("mpeg_decoder_test: PASS");
    return 0;
}
