/*
 * File synopsis:
 * Exercises the concrete A002 PS2SDK AUDSRV adapter against a host stub. The
 * fixture proves exact format/volume mapping, bounded byte-count conversion,
 * full-chunk submission accounting, stop behavior, and absence of any reachable
 * per-session audsrv_quit operation.
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "audio/audsrv_service.h"
#include <audsrv.h>

static int test_failures;
static int init_calls;
static int set_format_calls;
static int set_volume_calls;
static int wait_calls;
static int play_calls;
static int stop_calls;
static int quit_calls;
static int init_result;
static int format_result;
static int volume_result;
static int wait_result;
static int play_result;
static int stop_result;
static audsrv_fmt_t observed_format;
static int observed_volume;
static int observed_wait_bytes;
static int observed_play_bytes;
static const char *observed_play_buffer;

#define TEST_CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            test_failures += 1; \
        } \
    } while (0)

static void test_reset(void)
{
    init_calls = 0;
    set_format_calls = 0;
    set_volume_calls = 0;
    wait_calls = 0;
    play_calls = 0;
    stop_calls = 0;
    quit_calls = 0;
    init_result = 0;
    format_result = 0;
    volume_result = 0;
    wait_result = 0;
    play_result = 0;
    stop_result = 0;
    memset(&observed_format, 0, sizeof(observed_format));
    observed_volume = -1;
    observed_wait_bytes = -1;
    observed_play_bytes = -1;
    observed_play_buffer = NULL;
}

int audsrv_init(void)
{
    init_calls += 1;
    return init_result;
}

int audsrv_quit(void)
{
    quit_calls += 1;
    return 0;
}

int audsrv_set_format(audsrv_fmt_t *format)
{
    set_format_calls += 1;
    observed_format = *format;
    return format_result;
}

int audsrv_set_volume(int volume)
{
    set_volume_calls += 1;
    observed_volume = volume;
    return volume_result;
}

int audsrv_wait_audio(int bytes)
{
    wait_calls += 1;
    observed_wait_bytes = bytes;
    return wait_result;
}

int audsrv_play_audio(const char *chunk, int bytes)
{
    play_calls += 1;
    observed_play_buffer = chunk;
    observed_play_bytes = bytes;
    return play_result;
}

int audsrv_stop_audio(void)
{
    stop_calls += 1;
    return stop_result;
}

static void test_exact_sdk_mapping_and_no_quit(void)
{
    static const uint8_t bytes[] = {1u, 2u, 3u, 4u};
    pstvnc_audio_service_ops_t service;

    test_reset();
    service = pstvnc_audio_audsrv_service_ops();

    TEST_CHECK(service.context == NULL);
    TEST_CHECK(service.initialize(service.context) == 0);
    TEST_CHECK(service.set_format(service.context, 48000u, 2u, 16u) == 0);
    TEST_CHECK(service.set_volume(service.context, 61u) == 0);
    TEST_CHECK(service.wait_audio(service.context, sizeof(bytes)) == 0);
    play_result = (int)sizeof(bytes);
    TEST_CHECK(service.play_audio(service.context, bytes, sizeof(bytes)) == 0);
    TEST_CHECK(service.stop_audio(service.context) == 0);

    TEST_CHECK(init_calls == 1);
    TEST_CHECK(set_format_calls == 1);
    TEST_CHECK(observed_format.freq == 48000);
    TEST_CHECK(observed_format.channels == 2);
    TEST_CHECK(observed_format.bits == 16);
    TEST_CHECK(set_volume_calls == 1 && observed_volume == 61);
    TEST_CHECK(wait_calls == 1 && observed_wait_bytes == (int)sizeof(bytes));
    TEST_CHECK(play_calls == 1 && observed_play_bytes == (int)sizeof(bytes));
    TEST_CHECK(observed_play_buffer == (const char *)bytes);
    TEST_CHECK(stop_calls == 1);
    TEST_CHECK(quit_calls == 0);
}

static void test_unrepresentable_values_are_rejected_before_sdk(void)
{
    static const uint8_t byte = 9u;
    pstvnc_audio_service_ops_t service;

    test_reset();
    service = pstvnc_audio_audsrv_service_ops();

    TEST_CHECK(service.set_format(service.context, 0u, 2u, 16u) != 0);
    TEST_CHECK(service.set_format(service.context, 44100u, 3u, 16u) != 0);
    TEST_CHECK(service.set_format(service.context, 44100u, 2u, 24u) != 0);
    TEST_CHECK(service.set_volume(service.context, 101u) != 0);
    TEST_CHECK(service.wait_audio(service.context, 0u) != 0);
    TEST_CHECK(service.play_audio(service.context, &byte, 0u) != 0);
#if SIZE_MAX > INT_MAX
    TEST_CHECK(service.wait_audio(service.context, (size_t)INT_MAX + 1u) != 0);
    TEST_CHECK(service.play_audio(
        service.context, &byte, (size_t)INT_MAX + 1u) != 0);
#endif
    TEST_CHECK(set_format_calls == 0);
    TEST_CHECK(set_volume_calls == 0);
    TEST_CHECK(wait_calls == 0);
    TEST_CHECK(play_calls == 0);
    TEST_CHECK(quit_calls == 0);
}

static void test_partial_or_error_play_is_not_success(void)
{
    static const uint8_t bytes[] = {4u, 5u, 6u};
    pstvnc_audio_service_ops_t service;

    test_reset();
    service = pstvnc_audio_audsrv_service_ops();

    play_result = 2;
    TEST_CHECK(service.play_audio(service.context, bytes, sizeof(bytes)) != 0);
    TEST_CHECK(play_calls == 1);

    play_result = -1;
    TEST_CHECK(service.play_audio(service.context, bytes, sizeof(bytes)) != 0);
    TEST_CHECK(play_calls == 2);
    TEST_CHECK(quit_calls == 0);
}

static void test_sdk_failures_propagate(void)
{
    static const uint8_t byte = 1u;
    pstvnc_audio_service_ops_t service;

    test_reset();
    service = pstvnc_audio_audsrv_service_ops();

    init_result = 1;
    TEST_CHECK(service.initialize(service.context) != 0);
    format_result = 1;
    TEST_CHECK(service.set_format(service.context, 44100u, 2u, 16u) != 0);
    volume_result = 1;
    TEST_CHECK(service.set_volume(service.context, 50u) != 0);
    wait_result = 1;
    TEST_CHECK(service.wait_audio(service.context, 1u) != 0);
    play_result = -1;
    TEST_CHECK(service.play_audio(service.context, &byte, 1u) != 0);
    stop_result = 1;
    TEST_CHECK(service.stop_audio(service.context) != 0);
    TEST_CHECK(quit_calls == 0);
}

int main(void)
{
    test_exact_sdk_mapping_and_no_quit();
    test_unrepresentable_values_are_rejected_before_sdk();
    test_partial_or_error_play_is_not_success();
    test_sdk_failures_propagate();

    if (test_failures != 0) {
        fprintf(stderr, "audio_audsrv_service_test: %d failure(s)\n", test_failures);
        return 1;
    }

    puts("audio_audsrv_service_test: PASS");
    return 0;
}
