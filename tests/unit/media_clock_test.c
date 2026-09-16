/*
 * File synopsis:
 * Exercises the clean A002 common-media-clock owner through deterministic host
 * synchronization/time fakes. The fixture proves one-shot publication, shared
 * signed/saturating deadlines, and stop/error/zero-poll wait behavior without
 * selecting PCM/AUDSRV or MPEG/video runtime callsites.
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "media/clock.h"

#define TEST_TICKS_PER_SECOND 1000000u

typedef struct test_sync {
    int locked;
    int lock_calls;
    int unlock_calls;
    int fail_lock;
    int fail_unlock;
    pstvnc_media_clock_t *witness_clock;
    uint64_t expected_epoch;
    int witness_on_release;
    int witness_checked;
    int witness_failed;
} test_sync_t;

typedef struct test_time {
    uint64_t now_tick;
    int read_calls;
    int delay_calls;
    int fail_read;
    int fail_delay;
    pstvnc_media_clock_t *arm_clock;
    uint64_t arm_now_tick;
    int arm_on_delay_call;
    int arm_result;
} test_time_t;

typedef struct test_stop {
    int requested;
    int calls;
} test_stop_t;

static int test_failures;

#define TEST_CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            test_failures += 1; \
        } \
    } while (0)

static int test_lock(void *context)
{
    test_sync_t *sync = (test_sync_t *)context;

    sync->lock_calls += 1;
    if (sync->fail_lock || sync->locked)
        return -1;

    sync->locked = 1;
    return 0;
}

static int test_unlock(void *context)
{
    test_sync_t *sync = (test_sync_t *)context;

    sync->unlock_calls += 1;
    if (!sync->locked)
        return -1;

    sync->locked = 0;

    if (sync->witness_on_release) {
        sync->witness_checked += 1;
        if (
            sync->witness_clock == 0 ||
            !sync->witness_clock->armed ||
            sync->witness_clock->epoch_tick != sync->expected_epoch)
            sync->witness_failed = 1;
        sync->witness_on_release = 0;
    }

    if (sync->fail_unlock)
        return -1;

    return 0;
}

static int test_read_ticks(void *context, uint64_t *ticks)
{
    test_time_t *time = (test_time_t *)context;

    time->read_calls += 1;
    if (time->fail_read)
        return -1;

    *ticks = time->now_tick;
    return 0;
}

static int test_delay_us(void *context, uint32_t delay_us)
{
    test_time_t *time = (test_time_t *)context;

    time->delay_calls += 1;
    if (time->fail_delay)
        return -1;

    if (
        time->arm_clock != 0 &&
        time->arm_on_delay_call == time->delay_calls) {
        time->arm_result = pstvnc_media_clock_arm(
            time->arm_clock,
            time->arm_now_tick);
    }

    time->now_tick += (uint64_t)delay_us;
    return 0;
}

static int test_stop_requested(void *context)
{
    test_stop_t *stop = (test_stop_t *)context;

    stop->calls += 1;
    return stop->requested;
}

static pstvnc_config_media_clock_profile_t test_profile(
    uint32_t lead_us,
    int32_t audio_offset_us,
    int32_t video_offset_us)
{
    pstvnc_config_media_clock_profile_t profile;

    profile.epoch_lead_us = lead_us;
    profile.audio_presentation_offset_us = audio_offset_us;
    profile.video_presentation_offset_us = video_offset_us;
    return profile;
}

static pstvnc_media_clock_sync_t test_sync_ops(test_sync_t *sync)
{
    pstvnc_media_clock_sync_t ops;

    ops.lock = test_lock;
    ops.unlock = test_unlock;
    ops.context = sync;
    return ops;
}

static pstvnc_media_clock_time_ops_t test_time_ops(test_time_t *time)
{
    pstvnc_media_clock_time_ops_t ops;

    ops.read_ticks = test_read_ticks;
    ops.delay_us = test_delay_us;
    ops.context = time;
    return ops;
}

static pstvnc_media_clock_stop_observer_t test_stop_ops(test_stop_t *stop)
{
    pstvnc_media_clock_stop_observer_t observer;

    observer.is_stop_requested = test_stop_requested;
    observer.context = stop;
    return observer;
}

static void test_initialized_clock_is_unarmed(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_config_media_clock_profile_t profile = test_profile(0u, 0, 0);
    test_sync_t sync;
    pstvnc_media_clock_sync_t sync_ops;
    uint64_t epoch = 77u;
    uint64_t deadline = 88u;
    int armed = 1;

    memset(&sync, 0, sizeof(sync));
    sync_ops = test_sync_ops(&sync);

    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_is_armed(&clock, &armed) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(armed == 0);
    TEST_CHECK(pstvnc_media_clock_epoch(&clock, &epoch) == PSTVNC_MEDIA_CLOCK_UNARMED);
    TEST_CHECK(epoch == 77u);
    TEST_CHECK(
        pstvnc_media_clock_audio_deadline(&clock, 0u, &deadline) ==
        PSTVNC_MEDIA_CLOCK_UNARMED);
    TEST_CHECK(deadline == 88u);
}

static void test_arm_is_one_shot_and_publication_ordered(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_config_media_clock_profile_t profile = test_profile(25u, 0, 0);
    test_sync_t sync;
    pstvnc_media_clock_sync_t sync_ops;
    uint64_t epoch = 0u;

    memset(&sync, 0, sizeof(sync));
    sync_ops = test_sync_ops(&sync);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);

    sync.witness_clock = &clock;
    sync.expected_epoch = 125u;
    sync.witness_on_release = 1;

    TEST_CHECK(pstvnc_media_clock_arm(&clock, 100u) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(sync.witness_checked == 1);
    TEST_CHECK(sync.witness_failed == 0);
    TEST_CHECK(pstvnc_media_clock_epoch(&clock, &epoch) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(epoch == 125u);

    TEST_CHECK(pstvnc_media_clock_arm(&clock, 5000u) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_epoch(&clock, &epoch) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(epoch == 125u);
}

static void test_shared_epoch_produces_independent_deadlines(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_config_media_clock_profile_t profile = test_profile(100u, -250, 500);
    test_sync_t sync;
    pstvnc_media_clock_sync_t sync_ops;
    uint64_t audio_deadline = 0u;
    uint64_t video_deadline = 0u;

    memset(&sync, 0, sizeof(sync));
    sync_ops = test_sync_ops(&sync);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_arm(&clock, 10000u) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(
        pstvnc_media_clock_audio_deadline(&clock, 50u, &audio_deadline) ==
        PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(
        pstvnc_media_clock_video_deadline(&clock, 50u, &video_deadline) ==
        PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(audio_deadline == 9900u);
    TEST_CHECK(video_deadline == 10650u);
}

static void test_signed_boundaries_and_saturation(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_config_media_clock_profile_t profile;
    test_sync_t sync;
    pstvnc_media_clock_sync_t sync_ops;
    uint64_t deadline = 0u;
    uint64_t epoch = 0u;

    memset(&sync, 0, sizeof(sync));
    sync_ops = test_sync_ops(&sync);
    profile = test_profile(0u, INT32_MIN, INT32_MAX);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_arm(&clock, 3000000000ull) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(
        pstvnc_media_clock_audio_deadline(&clock, 0u, &deadline) ==
        PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(deadline == 852516352ull);
    TEST_CHECK(
        pstvnc_media_clock_video_deadline(&clock, 0u, &deadline) ==
        PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(deadline == 5147483647ull);

    profile = test_profile(0u, -100, 100);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_arm(&clock, 50u) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(
        pstvnc_media_clock_audio_deadline(&clock, 0u, &deadline) ==
        PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(deadline == 0u);

    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_arm(&clock, UINT64_MAX - 50u) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(
        pstvnc_media_clock_video_deadline(&clock, 0u, &deadline) ==
        PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(deadline == UINT64_MAX);

    profile = test_profile(0u, 0, 0);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_arm(&clock, UINT64_MAX - 5u) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(
        pstvnc_media_clock_audio_deadline(&clock, 10u, &deadline) ==
        PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(deadline == UINT64_MAX);

    profile = test_profile(100u, 0, 0);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_arm(&clock, UINT64_MAX - 50u) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_epoch(&clock, &epoch) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(epoch == UINT64_MAX);
}

static void test_wait_defers_until_arm_and_deadline(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_config_media_clock_profile_t profile = test_profile(0u, 100, 0);
    test_sync_t sync;
    pstvnc_media_clock_sync_t sync_ops;
    test_time_t time;
    pstvnc_media_clock_time_ops_t time_ops;

    memset(&sync, 0, sizeof(sync));
    memset(&time, 0, sizeof(time));
    sync_ops = test_sync_ops(&sync);
    time_ops = test_time_ops(&time);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);

    time.now_tick = 900u;
    time.arm_clock = &clock;
    time.arm_now_tick = 1000u;
    time.arm_on_delay_call = 1;
    time.arm_result = PSTVNC_MEDIA_CLOCK_INVALID;

    TEST_CHECK(
        pstvnc_media_clock_wait_audio(&clock, 0u, 50u, &time_ops, 0) ==
        PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(time.arm_result == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(time.delay_calls >= 1);
    TEST_CHECK(time.read_calls >= 1);
    TEST_CHECK(time.now_tick >= 1100u);
}

static void test_wait_stop_and_failures_are_explicit(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_config_media_clock_profile_t profile = test_profile(0u, 100, 0);
    test_sync_t sync;
    pstvnc_media_clock_sync_t sync_ops;
    test_time_t time;
    pstvnc_media_clock_time_ops_t time_ops;
    test_stop_t stop;
    pstvnc_media_clock_stop_observer_t stop_ops;

    memset(&sync, 0, sizeof(sync));
    memset(&time, 0, sizeof(time));
    memset(&stop, 0, sizeof(stop));
    sync_ops = test_sync_ops(&sync);
    time_ops = test_time_ops(&time);
    stop_ops = test_stop_ops(&stop);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);
    TEST_CHECK(pstvnc_media_clock_arm(&clock, 1000u) == PSTVNC_MEDIA_CLOCK_OK);

    stop.requested = 1;
    TEST_CHECK(
        pstvnc_media_clock_wait_audio(&clock, 0u, 10u, &time_ops, &stop_ops) ==
        PSTVNC_MEDIA_CLOCK_STOPPED);
    TEST_CHECK(time.read_calls == 0);
    TEST_CHECK(time.delay_calls == 0);

    stop.requested = 0;
    time.fail_read = 1;
    TEST_CHECK(
        pstvnc_media_clock_wait_audio(&clock, 0u, 10u, &time_ops, &stop_ops) ==
        PSTVNC_MEDIA_CLOCK_TIMER_FAILED);

    time.fail_read = 0;
    time.fail_delay = 1;
    time.now_tick = 0u;
    TEST_CHECK(
        pstvnc_media_clock_wait_audio(&clock, 0u, 10u, &time_ops, &stop_ops) ==
        PSTVNC_MEDIA_CLOCK_DELAY_FAILED);
}

static void test_zero_poll_is_rejected_without_spin(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_config_media_clock_profile_t profile = test_profile(0u, 0, 0);
    test_sync_t sync;
    pstvnc_media_clock_sync_t sync_ops;
    test_time_t time;
    pstvnc_media_clock_time_ops_t time_ops;

    memset(&sync, 0, sizeof(sync));
    memset(&time, 0, sizeof(time));
    sync_ops = test_sync_ops(&sync);
    time_ops = test_time_ops(&time);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);

    TEST_CHECK(
        pstvnc_media_clock_wait_audio(&clock, 0u, 0u, &time_ops, 0) ==
        PSTVNC_MEDIA_CLOCK_INVALID);
    TEST_CHECK(time.read_calls == 0);
    TEST_CHECK(time.delay_calls == 0);
}

static void test_sync_failures_surface(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_config_media_clock_profile_t profile = test_profile(0u, 0, 0);
    test_sync_t sync;
    pstvnc_media_clock_sync_t sync_ops;
    int armed = 99;

    memset(&sync, 0, sizeof(sync));
    sync_ops = test_sync_ops(&sync);
    TEST_CHECK(
        pstvnc_media_clock_init(
            &clock,
            &profile,
            TEST_TICKS_PER_SECOND,
            &sync_ops) == PSTVNC_MEDIA_CLOCK_OK);

    sync.fail_lock = 1;
    TEST_CHECK(
        pstvnc_media_clock_is_armed(&clock, &armed) ==
        PSTVNC_MEDIA_CLOCK_SYNC_FAILED);
    TEST_CHECK(armed == 99);
}

int main(void)
{
    test_initialized_clock_is_unarmed();
    test_arm_is_one_shot_and_publication_ordered();
    test_shared_epoch_produces_independent_deadlines();
    test_signed_boundaries_and_saturation();
    test_wait_defers_until_arm_and_deadline();
    test_wait_stop_and_failures_are_explicit();
    test_zero_poll_is_rejected_without_spin();
    test_sync_failures_surface();

    if (test_failures != 0) {
        fprintf(stderr, "media_clock_test: %d failure(s)\n", test_failures);
        return 1;
    }

    puts("media_clock_test: PASS");
    return 0;
}
