/*
 * File synopsis:
 * Deterministic host contract tests for A004 P6 absolute post-first-frame MPEG
 * scheduling, one-session-axis run anchoring, and presentation-only stale drop.
 */

#include "display/mpeg_scheduler.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int test_lock(void *context)
{
    (void)context;
    return 0;
}

static int test_unlock(void *context)
{
    (void)context;
    return 0;
}

static void init_clock(
    pstvnc_media_clock_t *clock,
    uint32_t ticks_per_second,
    uint64_t arm_tick,
    int arm_now)
{
    pstvnc_config_media_clock_profile_t profile;
    pstvnc_media_clock_sync_t sync;

    memset(&profile, 0, sizeof(profile));
    memset(&sync, 0, sizeof(sync));
    sync.lock = test_lock;
    sync.unlock = test_unlock;

    assert(pstvnc_media_clock_init(
        clock,
        &profile,
        ticks_per_second,
        &sync) == PSTVNC_MEDIA_CLOCK_OK);

    if (arm_now) {
        assert(pstvnc_media_clock_arm(
            clock,
            arm_tick) == PSTVNC_MEDIA_CLOCK_OK);
    }
}

static pstvnc_mpeg_scheduler_profile_t reference_profile(
    int drop_enabled)
{
    pstvnc_mpeg_scheduler_profile_t profile;

    memset(&profile, 0, sizeof(profile));
    profile.fps_numerator = 30000u;
    profile.fps_denominator = 1001u;
    profile.drop_enabled = drop_enabled;
    profile.drop_threshold_milliframes =
        drop_enabled ? 1000u : 0u;
    return profile;
}

static void test_requires_armed_clock_and_rejects_first_frame(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_scheduler_t scheduler;
    pstvnc_mpeg_scheduler_profile_t profile =
        reference_profile(1);
    pstvnc_mpeg_scheduler_decision_result_t decision;

    init_clock(&clock, 90000u, 10000u, 0);

    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        7u,
        10000u) == PSTVNC_MPEG_SCHEDULER_CLOCK_UNARMED);

    assert(pstvnc_media_clock_arm(
        &clock,
        10000u) == PSTVNC_MEDIA_CLOCK_OK);

    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        7u,
        10000u) == PSTVNC_MPEG_SCHEDULER_OK);

    assert(scheduler.run_generation == 7u);
    assert(scheduler.frame_period_ticks == 3003u);
    assert(scheduler.run_base_additional_ticks == 0u);
    assert(scheduler.drop_threshold_ticks == 3003u);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        7u,
        1u,
        10000u,
        &decision) == PSTVNC_MPEG_SCHEDULER_INVALID);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        8u,
        2u,
        10000u,
        &decision) == PSTVNC_MPEG_SCHEDULER_INVALID);
}

static void test_wait_present_and_full_frame_stale_drop(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_scheduler_t scheduler;
    pstvnc_mpeg_scheduler_t before;
    pstvnc_mpeg_scheduler_profile_t profile =
        reference_profile(1);
    pstvnc_mpeg_scheduler_decision_result_t decision;

    init_clock(&clock, 90000u, 10000u, 1);

    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        10u,
        10000u) == PSTVNC_MPEG_SCHEDULER_OK);

    before = scheduler;

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        10u,
        2u,
        13002u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.decision ==
        PSTVNC_MPEG_SCHEDULER_WAIT_UNTIL_DEADLINE);
    assert(decision.deadline_tick == 13003u);
    assert(decision.lateness_ticks == 0u);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        10u,
        2u,
        13003u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.decision ==
        PSTVNC_MPEG_SCHEDULER_PRESENT_NOW);
    assert(decision.deadline_tick == 13003u);
    assert(decision.lateness_ticks == 0u);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        10u,
        2u,
        13004u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.decision ==
        PSTVNC_MPEG_SCHEDULER_PRESENT_NOW);
    assert(decision.lateness_ticks == 1u);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        10u,
        2u,
        16006u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.decision ==
        PSTVNC_MPEG_SCHEDULER_DROP_PRESENTATION_ONLY);
    assert(decision.lateness_ticks == 3003u);

    /* Decisions are pure inspection; they do not advance runtime state. */
    assert(memcmp(&before, &scheduler, sizeof(scheduler)) == 0);
}

static void test_late_frame_never_rebases_later_deadline(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_scheduler_t scheduler;
    pstvnc_mpeg_scheduler_profile_t profile =
        reference_profile(1);
    pstvnc_mpeg_scheduler_decision_result_t decision;

    init_clock(&clock, 90000u, 10000u, 1);

    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        11u,
        10000u) == PSTVNC_MPEG_SCHEDULER_OK);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        11u,
        2u,
        14503u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.decision ==
        PSTVNC_MPEG_SCHEDULER_PRESENT_NOW);
    assert(decision.deadline_tick == 13003u);
    assert(decision.lateness_ticks == 1500u);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        11u,
        3u,
        15000u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.decision ==
        PSTVNC_MPEG_SCHEDULER_WAIT_UNTIL_DEADLINE);
    assert(decision.deadline_tick == 16006u);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        11u,
        4u,
        16006u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.deadline_tick == 19009u);
}

static void test_later_run_uses_immutable_session_axis_anchor(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_scheduler_t first_later_run;
    pstvnc_mpeg_scheduler_t second_later_run;
    pstvnc_mpeg_scheduler_profile_t profile =
        reference_profile(0);
    pstvnc_mpeg_scheduler_decision_result_t decision;
    uint64_t epoch_before;
    uint64_t epoch_after;

    init_clock(&clock, 90000u, 10000u, 1);
    assert(pstvnc_media_clock_epoch(
        &clock,
        &epoch_before) == PSTVNC_MEDIA_CLOCK_OK);

    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &first_later_run,
        &clock,
        &profile,
        20u,
        100000u) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(first_later_run.run_base_additional_ticks == 90000u);

    assert(pstvnc_mpeg_scheduler_decide(
        &first_later_run,
        20u,
        2u,
        100000u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.deadline_tick == 103003u);

    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &second_later_run,
        &clock,
        &profile,
        21u,
        150000u) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(second_later_run.run_base_additional_ticks == 140000u);

    assert(pstvnc_media_clock_epoch(
        &clock,
        &epoch_after) == PSTVNC_MEDIA_CLOCK_OK);
    assert(epoch_after == epoch_before);
}

static void test_drop_disabled_never_drops_valid_frame(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_scheduler_t scheduler;
    pstvnc_mpeg_scheduler_profile_t profile =
        reference_profile(0);
    pstvnc_mpeg_scheduler_decision_result_t decision;

    init_clock(&clock, 90000u, 10000u, 1);

    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        30u,
        10000u) == PSTVNC_MPEG_SCHEDULER_OK);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        30u,
        2u,
        UINT64_MAX,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.decision ==
        PSTVNC_MPEG_SCHEDULER_PRESENT_NOW);
}

static void test_invalid_profiles_and_saturating_arithmetic(void)
{
    pstvnc_media_clock_t clock;
    pstvnc_media_clock_t slow_clock;
    pstvnc_media_clock_t huge_clock;
    pstvnc_mpeg_scheduler_t scheduler;
    pstvnc_mpeg_scheduler_profile_t profile;
    pstvnc_mpeg_scheduler_decision_result_t decision;

    init_clock(&clock, 90000u, 10000u, 1);

    profile = reference_profile(1);
    profile.fps_numerator = 0u;
    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        40u,
        10000u) == PSTVNC_MPEG_SCHEDULER_INVALID);

    profile = reference_profile(1);
    profile.fps_denominator = 0u;
    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        40u,
        10000u) == PSTVNC_MPEG_SCHEDULER_INVALID);

    profile = reference_profile(1);
    profile.drop_threshold_milliframes = 0u;
    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        40u,
        10000u) == PSTVNC_MPEG_SCHEDULER_INVALID);

    profile = reference_profile(0);
    profile.drop_threshold_milliframes = 1000u;
    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &clock,
        &profile,
        40u,
        10000u) == PSTVNC_MPEG_SCHEDULER_INVALID);

    init_clock(&slow_clock, 1u, 0u, 1);
    memset(&profile, 0, sizeof(profile));
    profile.fps_numerator = 2u;
    profile.fps_denominator = 1u;
    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &slow_clock,
        &profile,
        41u,
        0u) == PSTVNC_MPEG_SCHEDULER_INVALID);

    init_clock(&huge_clock, UINT32_MAX, 0u, 1);
    memset(&profile, 0, sizeof(profile));
    profile.fps_numerator = 1u;
    profile.fps_denominator = UINT32_MAX;
    profile.drop_enabled = 1;
    profile.drop_threshold_milliframes = UINT32_MAX;

    assert(pstvnc_mpeg_scheduler_init_after_first_sync(
        &scheduler,
        &huge_clock,
        &profile,
        42u,
        0u) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(scheduler.frame_period_ticks != 0u);
    assert(scheduler.drop_threshold_ticks == UINT64_MAX);

    assert(pstvnc_mpeg_scheduler_decide(
        &scheduler,
        42u,
        UINT64_MAX,
        UINT64_MAX - 1u,
        &decision) == PSTVNC_MPEG_SCHEDULER_OK);
    assert(decision.deadline_tick == UINT64_MAX);
    assert(decision.decision ==
        PSTVNC_MPEG_SCHEDULER_WAIT_UNTIL_DEADLINE);
}

int main(void)
{
    test_requires_armed_clock_and_rejects_first_frame();
    test_wait_present_and_full_frame_stale_drop();
    test_late_frame_never_rebases_later_deadline();
    test_later_run_uses_immutable_session_axis_anchor();
    test_drop_disabled_never_drops_valid_frame();
    test_invalid_profiles_and_saturating_arithmetic();

    puts("MPEG_SCHEDULER_TEST=PASS");
    return 0;
}
