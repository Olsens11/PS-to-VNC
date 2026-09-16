/*
 * File synopsis:
 * Exercises the synchronous A002 PCM playback core with deterministic fake
 * Transport and resident-service operations. The fixture proves exact profile
 * setup, non-spinning Transport waits, wait-before-play ordering, truthful
 * accounting, finite exhaustion, terminal distinctions, and cleanup semantics.
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "audio/playback.h"
#include "transport/bridge.h"

#define TEST_MAX_STEPS 16
#define TEST_MAX_EVENTS 64
#define TEST_MAX_PLAYED 128

typedef struct test_transport_step {
    pstvnc_transport_result_t result;
    const uint8_t *bytes;
    size_t byte_count;
} test_transport_step_t;

typedef struct test_transport_state {
    test_transport_step_t steps[TEST_MAX_STEPS];
    size_t step_count;
    size_t next_step;
    int snapshot_calls;
    int wait_calls;
    uint32_t activity_sequence;
    pstvnc_transport_result_t snapshot_result;
    pstvnc_transport_result_t wait_result;
} test_transport_state_t;

typedef enum test_service_event {
    TEST_EVENT_INIT = 1,
    TEST_EVENT_FORMAT = 2,
    TEST_EVENT_VOLUME = 3,
    TEST_EVENT_WAIT = 4,
    TEST_EVENT_PLAY = 5,
    TEST_EVENT_STOP = 6
} test_service_event_t;

typedef struct test_service_state {
    test_service_event_t events[TEST_MAX_EVENTS];
    size_t event_count;
    uint32_t rate_hz;
    uint32_t channels;
    uint32_t bits_per_sample;
    uint32_t volume_percent;
    size_t wait_sizes[TEST_MAX_EVENTS];
    size_t wait_count;
    size_t play_sizes[TEST_MAX_EVENTS];
    size_t play_count;
    uint8_t played[TEST_MAX_PLAYED];
    size_t played_count;
    int fail_init;
    int fail_format;
    int fail_volume;
    int fail_wait_call;
    int fail_play_call;
    int fail_stop;
    int stop_calls;
} test_service_state_t;

static int test_failures;
static test_transport_state_t test_transport;

#define TEST_CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            test_failures += 1; \
        } \
    } while (0)

static void test_transport_reset(void)
{
    memset(&test_transport, 0, sizeof(test_transport));
    test_transport.snapshot_result = PSTVNC_TRANSPORT_OK;
    test_transport.wait_result = PSTVNC_TRANSPORT_OK;
    test_transport.activity_sequence = 10u;
}

static void test_transport_add(
    pstvnc_transport_result_t result,
    const uint8_t *bytes,
    size_t byte_count)
{
    test_transport_step_t *step;

    TEST_CHECK(test_transport.step_count < TEST_MAX_STEPS);
    if (test_transport.step_count >= TEST_MAX_STEPS)
        return;

    step = &test_transport.steps[test_transport.step_count++];
    step->result = result;
    step->bytes = bytes;
    step->byte_count = byte_count;
}

pstvnc_transport_result_t pstvnc_transport_audio_read_available(
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    test_transport_step_t *step;

    if (read_count == NULL || buffer == NULL || maximum_count == 0u)
        return PSTVNC_TRANSPORT_INVALID;
    *read_count = 0u;

    if (test_transport.next_step >= test_transport.step_count)
        return PSTVNC_TRANSPORT_FAILED;

    step = &test_transport.steps[test_transport.next_step++];
    if (step->result == PSTVNC_TRANSPORT_OK) {
        if (step->byte_count == 0u || step->byte_count > maximum_count)
            return PSTVNC_TRANSPORT_FAILED;
        memcpy(buffer, step->bytes, step->byte_count);
        *read_count = step->byte_count;
    }
    return step->result;
}

pstvnc_transport_result_t pstvnc_transport_audio_status(
    size_t *available_count,
    int *producer_done)
{
    (void)available_count;
    (void)producer_done;
    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_audio_activity_snapshot(
    uint32_t *activity_sequence)
{
    test_transport.snapshot_calls += 1;
    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;
    if (test_transport.snapshot_result == PSTVNC_TRANSPORT_OK)
        *activity_sequence = test_transport.activity_sequence;
    return test_transport.snapshot_result;
}

pstvnc_transport_result_t pstvnc_transport_audio_wait_activity(
    uint32_t *activity_sequence)
{
    test_transport.wait_calls += 1;
    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;
    if (test_transport.wait_result == PSTVNC_TRANSPORT_OK) {
        test_transport.activity_sequence += 1u;
        *activity_sequence = test_transport.activity_sequence;
    }
    return test_transport.wait_result;
}

static void test_record_event(
    test_service_state_t *service,
    test_service_event_t event)
{
    TEST_CHECK(service->event_count < TEST_MAX_EVENTS);
    if (service->event_count < TEST_MAX_EVENTS)
        service->events[service->event_count++] = event;
}

static int test_service_initialize(void *context)
{
    test_service_state_t *service = (test_service_state_t *)context;
    test_record_event(service, TEST_EVENT_INIT);
    return service->fail_init ? -1 : 0;
}

static int test_service_set_format(
    void *context,
    uint32_t rate_hz,
    uint32_t channels,
    uint32_t bits_per_sample)
{
    test_service_state_t *service = (test_service_state_t *)context;
    test_record_event(service, TEST_EVENT_FORMAT);
    service->rate_hz = rate_hz;
    service->channels = channels;
    service->bits_per_sample = bits_per_sample;
    return service->fail_format ? -1 : 0;
}

static int test_service_set_volume(void *context, uint32_t volume_percent)
{
    test_service_state_t *service = (test_service_state_t *)context;
    test_record_event(service, TEST_EVENT_VOLUME);
    service->volume_percent = volume_percent;
    return service->fail_volume ? -1 : 0;
}

static int test_service_wait_audio(void *context, size_t byte_count)
{
    test_service_state_t *service = (test_service_state_t *)context;
    test_record_event(service, TEST_EVENT_WAIT);
    TEST_CHECK(byte_count != 0u);
    if (service->wait_count < TEST_MAX_EVENTS)
        service->wait_sizes[service->wait_count] = byte_count;
    service->wait_count += 1u;
    return service->fail_wait_call == (int)service->wait_count ? -1 : 0;
}

static int test_service_play_audio(
    void *context,
    const uint8_t *bytes,
    size_t byte_count)
{
    test_service_state_t *service = (test_service_state_t *)context;
    test_record_event(service, TEST_EVENT_PLAY);
    TEST_CHECK(bytes != NULL);
    TEST_CHECK(byte_count != 0u);
    if (service->play_count < TEST_MAX_EVENTS)
        service->play_sizes[service->play_count] = byte_count;
    service->play_count += 1u;

    if (service->fail_play_call == (int)service->play_count)
        return -1;

    TEST_CHECK(service->played_count + byte_count <= TEST_MAX_PLAYED);
    if (service->played_count + byte_count <= TEST_MAX_PLAYED) {
        memcpy(service->played + service->played_count, bytes, byte_count);
        service->played_count += byte_count;
    }
    return 0;
}

static int test_service_stop_audio(void *context)
{
    test_service_state_t *service = (test_service_state_t *)context;
    test_record_event(service, TEST_EVENT_STOP);
    service->stop_calls += 1;
    return service->fail_stop ? -1 : 0;
}

static pstvnc_audio_service_ops_t test_service_ops(test_service_state_t *state)
{
    pstvnc_audio_service_ops_t ops;

    ops.initialize = test_service_initialize;
    ops.set_format = test_service_set_format;
    ops.set_volume = test_service_set_volume;
    ops.wait_audio = test_service_wait_audio;
    ops.play_audio = test_service_play_audio;
    ops.stop_audio = test_service_stop_audio;
    ops.context = state;
    return ops;
}

static pstvnc_config_pcm_profile_t test_profile(void)
{
    pstvnc_config_pcm_profile_t profile;

    profile.rate_hz = 44100u;
    profile.channels = 2u;
    profile.bits_per_sample = 16u;
    profile.volume_percent = 73u;
    return profile;
}

static void test_exact_setup_chunk_order_and_finite_completion(void)
{
    static const uint8_t first[] = {1u, 2u};
    static const uint8_t second[] = {3u, 4u, 5u, 6u};
    static const uint8_t expected[] = {1u, 2u, 3u, 4u, 5u, 6u};
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[4];
    pstvnc_audio_playback_result_t result;

    memset(&state, 0, sizeof(state));
    test_transport_reset();
    test_transport_add(PSTVNC_TRANSPORT_OK, first, sizeof(first));
    test_transport_add(PSTVNC_TRANSPORT_OK, second, sizeof(second));
    test_transport_add(PSTVNC_TRANSPORT_EXHAUSTED, NULL, 0u);
    service = test_service_ops(&state);

    result = pstvnc_audio_playback_run(
        &profile, buffer, sizeof(buffer), &service, &report);

    TEST_CHECK(result == PSTVNC_AUDIO_PLAYBACK_COMPLETE);
    TEST_CHECK(state.rate_hz == 44100u);
    TEST_CHECK(state.channels == 2u);
    TEST_CHECK(state.bits_per_sample == 16u);
    TEST_CHECK(state.volume_percent == 73u);
    TEST_CHECK(state.event_count == 8u);
    TEST_CHECK(state.events[0] == TEST_EVENT_INIT);
    TEST_CHECK(state.events[1] == TEST_EVENT_FORMAT);
    TEST_CHECK(state.events[2] == TEST_EVENT_VOLUME);
    TEST_CHECK(state.events[3] == TEST_EVENT_WAIT);
    TEST_CHECK(state.events[4] == TEST_EVENT_PLAY);
    TEST_CHECK(state.events[5] == TEST_EVENT_WAIT);
    TEST_CHECK(state.events[6] == TEST_EVENT_PLAY);
    TEST_CHECK(state.events[7] == TEST_EVENT_STOP);
    TEST_CHECK(state.wait_count == 2u && state.play_count == 2u);
    TEST_CHECK(state.wait_sizes[0] == sizeof(first));
    TEST_CHECK(state.play_sizes[0] == sizeof(first));
    TEST_CHECK(state.wait_sizes[1] == sizeof(second));
    TEST_CHECK(state.play_sizes[1] == sizeof(second));
    TEST_CHECK(state.played_count == sizeof(expected));
    TEST_CHECK(memcmp(state.played, expected, sizeof(expected)) == 0);
    TEST_CHECK(report.submitted_bytes == sizeof(expected));
    TEST_CHECK(report.submitted_chunks == 2u);
    TEST_CHECK(report.cleanup_attempted == 1);
    TEST_CHECK(report.cleanup_failed == 0);
    TEST_CHECK(state.stop_calls == 1);
}

static void test_temporary_unavailability_waits_for_activity(void)
{
    static const uint8_t byte = 0x5au;
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[2];

    memset(&state, 0, sizeof(state));
    test_transport_reset();
    test_transport_add(PSTVNC_TRANSPORT_WOULD_BLOCK, NULL, 0u);
    test_transport_add(PSTVNC_TRANSPORT_OK, &byte, 1u);
    test_transport_add(PSTVNC_TRANSPORT_EXHAUSTED, NULL, 0u);
    service = test_service_ops(&state);

    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_COMPLETE);
    TEST_CHECK(test_transport.wait_calls == 1);
    TEST_CHECK(test_transport.snapshot_calls == 3);
    TEST_CHECK(report.submitted_bytes == 1u);
    TEST_CHECK(state.play_count == 1u);
}

static pstvnc_audio_playback_result_t test_run_terminal(
    pstvnc_transport_result_t terminal)
{
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[4];
    pstvnc_audio_playback_result_t result;

    memset(&state, 0, sizeof(state));
    test_transport_reset();
    test_transport_add(terminal, NULL, 0u);
    service = test_service_ops(&state);
    result = pstvnc_audio_playback_run(
        &profile, buffer, sizeof(buffer), &service, &report);
    TEST_CHECK(state.stop_calls == 1);
    TEST_CHECK(state.play_count == 0u);
    TEST_CHECK(report.submitted_bytes == 0u);
    return result;
}

static void test_transport_terminal_results_remain_distinct(void)
{
    TEST_CHECK(test_run_terminal(PSTVNC_TRANSPORT_STOPPED) ==
        PSTVNC_AUDIO_PLAYBACK_STOPPED);
    TEST_CHECK(test_run_terminal(PSTVNC_TRANSPORT_CLOSED) ==
        PSTVNC_AUDIO_PLAYBACK_CLOSED);
    TEST_CHECK(test_run_terminal(PSTVNC_TRANSPORT_INVALID) ==
        PSTVNC_AUDIO_PLAYBACK_TRANSPORT_INVALID);
    TEST_CHECK(test_run_terminal(PSTVNC_TRANSPORT_FAILED) ==
        PSTVNC_AUDIO_PLAYBACK_TRANSPORT_FAILED);
}

static void test_wait_failure_prevents_play_and_accounting(void)
{
    static const uint8_t bytes[] = {9u, 8u, 7u};
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[4];

    memset(&state, 0, sizeof(state));
    state.fail_wait_call = 1;
    test_transport_reset();
    test_transport_add(PSTVNC_TRANSPORT_OK, bytes, sizeof(bytes));
    service = test_service_ops(&state);

    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_SERVICE_WAIT_FAILED);
    TEST_CHECK(state.wait_count == 1u);
    TEST_CHECK(state.play_count == 0u);
    TEST_CHECK(report.submitted_bytes == 0u);
    TEST_CHECK(report.submitted_chunks == 0u);
    TEST_CHECK(state.stop_calls == 1);
}

static void test_play_failure_does_not_advance_accounting(void)
{
    static const uint8_t bytes[] = {4u, 3u, 2u};
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[4];

    memset(&state, 0, sizeof(state));
    state.fail_play_call = 1;
    test_transport_reset();
    test_transport_add(PSTVNC_TRANSPORT_OK, bytes, sizeof(bytes));
    service = test_service_ops(&state);

    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED);
    TEST_CHECK(state.wait_count == 1u);
    TEST_CHECK(state.play_count == 1u);
    TEST_CHECK(report.submitted_bytes == 0u);
    TEST_CHECK(report.submitted_chunks == 0u);
    TEST_CHECK(state.stop_calls == 1);
}

static void test_setup_failures_and_cleanup_scope(void)
{
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[4];

    memset(&state, 0, sizeof(state));
    state.fail_init = 1;
    test_transport_reset();
    service = test_service_ops(&state);
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_SERVICE_INIT_FAILED);
    TEST_CHECK(state.stop_calls == 0);
    TEST_CHECK(report.cleanup_attempted == 0);

    memset(&state, 0, sizeof(state));
    state.fail_format = 1;
    test_transport_reset();
    service = test_service_ops(&state);
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_SERVICE_FORMAT_FAILED);
    TEST_CHECK(state.stop_calls == 1);

    memset(&state, 0, sizeof(state));
    state.fail_volume = 1;
    test_transport_reset();
    service = test_service_ops(&state);
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_SERVICE_VOLUME_FAILED);
    TEST_CHECK(state.stop_calls == 1);
}

static void test_cleanup_failure_preserves_primary_error(void)
{
    static const uint8_t byte = 1u;
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[4];

    memset(&state, 0, sizeof(state));
    state.fail_stop = 1;
    test_transport_reset();
    test_transport_add(PSTVNC_TRANSPORT_EXHAUSTED, NULL, 0u);
    service = test_service_ops(&state);
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_SERVICE_STOP_FAILED);
    TEST_CHECK(report.cleanup_failed == 1);

    memset(&state, 0, sizeof(state));
    state.fail_play_call = 1;
    state.fail_stop = 1;
    test_transport_reset();
    test_transport_add(PSTVNC_TRANSPORT_OK, &byte, 1u);
    service = test_service_ops(&state);
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED);
    TEST_CHECK(report.cleanup_failed == 1);
    TEST_CHECK(report.submitted_bytes == 0u);
}

static void test_invalid_authority_fails_before_service_use(void)
{
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[4];

    memset(&state, 0, sizeof(state));
    service = test_service_ops(&state);
    profile.channels = 3u;
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_INVALID);
    TEST_CHECK(state.event_count == 0u);

    profile = test_profile();
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, 0u, &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_INVALID);
    TEST_CHECK(state.event_count == 0u);

    service.play_audio = NULL;
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_INVALID);
    TEST_CHECK(state.event_count == 0u);
}

static void test_activity_wait_terminal_is_not_reclassified(void)
{
    pstvnc_config_pcm_profile_t profile = test_profile();
    test_service_state_t state;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    uint8_t buffer[4];

    memset(&state, 0, sizeof(state));
    test_transport_reset();
    test_transport_add(PSTVNC_TRANSPORT_WOULD_BLOCK, NULL, 0u);
    test_transport.wait_result = PSTVNC_TRANSPORT_STOPPED;
    service = test_service_ops(&state);
    TEST_CHECK(
        pstvnc_audio_playback_run(
            &profile, buffer, sizeof(buffer), &service, &report) ==
        PSTVNC_AUDIO_PLAYBACK_STOPPED);
    TEST_CHECK(test_transport.wait_calls == 1);
    TEST_CHECK(state.play_count == 0u);
    TEST_CHECK(state.stop_calls == 1);
}

int main(void)
{
    test_exact_setup_chunk_order_and_finite_completion();
    test_temporary_unavailability_waits_for_activity();
    test_transport_terminal_results_remain_distinct();
    test_wait_failure_prevents_play_and_accounting();
    test_play_failure_does_not_advance_accounting();
    test_setup_failures_and_cleanup_scope();
    test_cleanup_failure_preserves_primary_error();
    test_invalid_authority_fails_before_service_use();
    test_activity_wait_terminal_is_not_reclassified();

    if (test_failures != 0) {
        fprintf(stderr, "audio_playback_test: %d failure(s)\n", test_failures);
        return 1;
    }

    puts("audio_playback_test: PASS");
    return 0;
}
