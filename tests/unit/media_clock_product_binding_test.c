/*
 * File synopsis:
 * Proves R26's selected media-clock profile and PS2 Platform binding with
 * deterministic kernel/timer/delay stubs. The fixture exercises no live PS2
 * resources and deliberately does not arm the platform-neutral media clock.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config/media_clock_profile.h"
#include "platform/ps2_media_clock.h"

#include "kernel.h"
#include "timer.h"

static int test_failures;

#define TEST_CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            test_failures += 1; \
        } \
    } while (0)

typedef struct test_kernel_state {
    int create_result;
    int create_calls;
    int delete_calls;
    int wait_calls;
    int signal_calls;
    int fail_delete;
    int fail_wait;
    int fail_signal;
    int last_delete_id;
    int last_wait_id;
    int last_signal_id;
    ee_sema_t last_create;
    uint64_t timer_tick;
    int delay_calls;
    int last_delay_us;
    int fail_delay;
} test_kernel_state_t;

static test_kernel_state_t kernel_state;

static void reset_kernel_state(int create_result)
{
    memset(&kernel_state, 0, sizeof(kernel_state));
    kernel_state.create_result = create_result;
    kernel_state.last_delete_id = -1;
    kernel_state.last_wait_id = -1;
    kernel_state.last_signal_id = -1;
}

int CreateSema(ee_sema_t *semaphore)
{
    kernel_state.create_calls += 1;
    if (semaphore != NULL)
        kernel_state.last_create = *semaphore;
    return kernel_state.create_result;
}

int DeleteSema(int semaphore_id)
{
    kernel_state.delete_calls += 1;
    kernel_state.last_delete_id = semaphore_id;
    return kernel_state.fail_delete ? -1 : 0;
}

int WaitSema(int semaphore_id)
{
    kernel_state.wait_calls += 1;
    kernel_state.last_wait_id = semaphore_id;
    return kernel_state.fail_wait ? -1 : 0;
}

int SignalSema(int semaphore_id)
{
    kernel_state.signal_calls += 1;
    kernel_state.last_signal_id = semaphore_id;
    return kernel_state.fail_signal ? -1 : 0;
}

uint64_t GetTimerSystemTime(void)
{
    return kernel_state.timer_tick;
}

int DelayThread(int microseconds)
{
    kernel_state.delay_calls += 1;
    kernel_state.last_delay_us = microseconds;
    return kernel_state.fail_delay ? -1 : 0;
}

static void test_selected_profile_is_exact_copy_authority(void)
{
    pstvnc_config_media_clock_profile_t first =
        pstvnc_config_media_clock_profile_selected();
    pstvnc_config_media_clock_profile_t second;

    TEST_CHECK(first.epoch_lead_us == 0u);
    TEST_CHECK(first.audio_presentation_offset_us == 0);
    TEST_CHECK(first.video_presentation_offset_us == 0);

    first.epoch_lead_us = 999u;
    first.audio_presentation_offset_us = -123;
    first.video_presentation_offset_us = 456;

    second = pstvnc_config_media_clock_profile_selected();
    TEST_CHECK(second.epoch_lead_us == 0u);
    TEST_CHECK(second.audio_presentation_offset_us == 0);
    TEST_CHECK(second.video_presentation_offset_us == 0);
}

static void test_successful_binding_exposes_exact_observers(void)
{
    pstvnc_ps2_media_clock_binding_t binding =
        PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER;
    pstvnc_media_clock_sync_t sync;
    pstvnc_media_clock_time_ops_t time_ops;
    uint32_t tick_rate = 0u;
    uint64_t tick = 0u;

    reset_kernel_state(41);
    kernel_state.timer_tick = UINT64_C(0x1122334455667788);

    TEST_CHECK(pstvnc_ps2_media_clock_binding_init(&binding) == 0);
    TEST_CHECK(kernel_state.create_calls == 1);
    TEST_CHECK(kernel_state.last_create.init_count == 1);
    TEST_CHECK(kernel_state.last_create.max_count == 1);
    TEST_CHECK(kernel_state.last_create.option == 0);
    TEST_CHECK(binding.semaphore_id == 41);
    TEST_CHECK(binding.state == PSTVNC_PS2_MEDIA_CLOCK_BINDING_ACTIVE);

    TEST_CHECK(pstvnc_ps2_media_clock_binding_sync(&binding, &sync) == 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_time_ops(&binding, &time_ops) == 0);
    TEST_CHECK(sync.context == &binding);
    TEST_CHECK(time_ops.context == &binding);

    TEST_CHECK(sync.lock(sync.context) == 0);
    TEST_CHECK(kernel_state.wait_calls == 1);
    TEST_CHECK(kernel_state.last_wait_id == 41);
    TEST_CHECK(sync.unlock(sync.context) == 0);
    TEST_CHECK(kernel_state.signal_calls == 1);
    TEST_CHECK(kernel_state.last_signal_id == 41);

    TEST_CHECK(
        pstvnc_ps2_media_clock_binding_tick_rate(&binding, &tick_rate) == 0);
    TEST_CHECK(tick_rate == (uint32_t)kBUSCLK);

    TEST_CHECK(
        pstvnc_ps2_media_clock_binding_current_tick(&binding, &tick) == 0);
    TEST_CHECK(tick == UINT64_C(0x1122334455667788));

    tick = 0u;
    TEST_CHECK(time_ops.read_ticks(time_ops.context, &tick) == 0);
    TEST_CHECK(tick == UINT64_C(0x1122334455667788));

    TEST_CHECK(time_ops.delay_us(time_ops.context, 1234u) == 0);
    TEST_CHECK(kernel_state.delay_calls == 1);
    TEST_CHECK(kernel_state.last_delay_us == 1234);

    TEST_CHECK(pstvnc_ps2_media_clock_binding_release(&binding) == 0);
    TEST_CHECK(kernel_state.delete_calls == 1);
    TEST_CHECK(kernel_state.last_delete_id == 41);
    TEST_CHECK(binding.semaphore_id == -1);
    TEST_CHECK(binding.state == PSTVNC_PS2_MEDIA_CLOCK_BINDING_RETIRED);

    TEST_CHECK(sync.lock(sync.context) < 0);
    TEST_CHECK(sync.unlock(sync.context) < 0);
    TEST_CHECK(time_ops.read_ticks(time_ops.context, &tick) < 0);
    TEST_CHECK(time_ops.delay_us(time_ops.context, 1u) < 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_release(&binding) < 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_init(&binding) < 0);
    TEST_CHECK(kernel_state.create_calls == 1);
}

static void test_init_and_observer_failures_are_not_promoted(void)
{
    pstvnc_ps2_media_clock_binding_t failed =
        PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER;
    pstvnc_ps2_media_clock_binding_t live =
        PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER;
    pstvnc_media_clock_sync_t sync;
    pstvnc_media_clock_time_ops_t time_ops;

    reset_kernel_state(-1);
    memset(&sync, 0x5a, sizeof(sync));
    memset(&time_ops, 0x5a, sizeof(time_ops));

    TEST_CHECK(pstvnc_ps2_media_clock_binding_init(&failed) < 0);
    TEST_CHECK(failed.semaphore_id == -1);
    TEST_CHECK(failed.state == PSTVNC_PS2_MEDIA_CLOCK_BINDING_RETIRED);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_sync(&failed, &sync) < 0);
    TEST_CHECK(sync.lock == NULL && sync.unlock == NULL && sync.context == NULL);
    TEST_CHECK(
        pstvnc_ps2_media_clock_binding_time_ops(&failed, &time_ops) < 0);
    TEST_CHECK(
        time_ops.read_ticks == NULL &&
        time_ops.delay_us == NULL &&
        time_ops.context == NULL);

    reset_kernel_state(51);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_init(&live) == 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_sync(&live, &sync) == 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_time_ops(&live, &time_ops) == 0);

    kernel_state.fail_wait = 1;
    TEST_CHECK(sync.lock(sync.context) < 0);
    kernel_state.fail_wait = 0;

    kernel_state.fail_signal = 1;
    TEST_CHECK(sync.unlock(sync.context) < 0);
    kernel_state.fail_signal = 0;

    kernel_state.fail_delay = 1;
    TEST_CHECK(time_ops.delay_us(time_ops.context, 777u) < 0);
    TEST_CHECK(kernel_state.last_delay_us == 777);
    kernel_state.fail_delay = 0;

    TEST_CHECK(pstvnc_ps2_media_clock_binding_release(&live) == 0);
}

static void test_repeated_sessions_use_distinct_terminal_authority(void)
{
    pstvnc_ps2_media_clock_binding_t session_a =
        PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER;
    pstvnc_ps2_media_clock_binding_t session_b =
        PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER;
    pstvnc_media_clock_sync_t sync_a;
    pstvnc_media_clock_sync_t sync_b;

    reset_kernel_state(61);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_init(&session_a) == 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_sync(&session_a, &sync_a) == 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_release(&session_a) == 0);

    kernel_state.create_result = 62;
    TEST_CHECK(pstvnc_ps2_media_clock_binding_init(&session_b) == 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_sync(&session_b, &sync_b) == 0);

    TEST_CHECK(&session_a != &session_b);
    TEST_CHECK(sync_a.context == &session_a);
    TEST_CHECK(sync_b.context == &session_b);
    TEST_CHECK(session_a.state == PSTVNC_PS2_MEDIA_CLOCK_BINDING_RETIRED);
    TEST_CHECK(session_b.state == PSTVNC_PS2_MEDIA_CLOCK_BINDING_ACTIVE);
    TEST_CHECK(session_a.semaphore_id == -1);
    TEST_CHECK(session_b.semaphore_id == 62);

    TEST_CHECK(sync_a.lock(sync_a.context) < 0);
    TEST_CHECK(sync_b.lock(sync_b.context) == 0);
    TEST_CHECK(kernel_state.last_wait_id == 62);
    TEST_CHECK(sync_b.unlock(sync_b.context) == 0);

    TEST_CHECK(pstvnc_ps2_media_clock_binding_release(&session_b) == 0);
}

static void test_release_failure_still_revokes_local_authority(void)
{
    pstvnc_ps2_media_clock_binding_t binding =
        PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER;
    pstvnc_media_clock_sync_t sync;

    reset_kernel_state(71);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_init(&binding) == 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_sync(&binding, &sync) == 0);

    kernel_state.fail_delete = 1;
    TEST_CHECK(pstvnc_ps2_media_clock_binding_release(&binding) < 0);
    TEST_CHECK(binding.state == PSTVNC_PS2_MEDIA_CLOCK_BINDING_RETIRED);
    TEST_CHECK(binding.semaphore_id == -1);
    TEST_CHECK(sync.lock(sync.context) < 0);
    TEST_CHECK(pstvnc_ps2_media_clock_binding_release(&binding) < 0);
}

int main(void)
{
    test_selected_profile_is_exact_copy_authority();
    test_successful_binding_exposes_exact_observers();
    test_init_and_observer_failures_are_not_promoted();
    test_repeated_sessions_use_distinct_terminal_authority();
    test_release_failure_still_revokes_local_authority();

    if (test_failures != 0) {
        fprintf(stderr, "MEDIA_CLOCK_PRODUCT_BINDING_TEST=FAIL count=%d\n", test_failures);
        return 1;
    }

    printf("MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS\n");
    return 0;
}
