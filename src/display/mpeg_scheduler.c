/*
 * File synopsis:
 * Implements the clean post-first-frame MPEG absolute scheduling policy on the
 * existing session media-clock axis.
 *
 * Historical H1 used absolute source-rate deadlines and optional stale visible
 * draw suppression. This clean owner preserves only those timing semantics:
 * immutable run placement, a non-rebasing deadline grid, and presentation-only
 * stale classification. It performs no timing wait or runtime integration.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-ABSOLUTE-VIDEO-SCHEDULER-DROP-R6.
 */

#include "mpeg_scheduler.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

static uint64_t pstvnc_mpeg_scheduler_add_saturating(
    uint64_t left,
    uint64_t right)
{
    if (UINT64_MAX - left < right)
        return UINT64_MAX;

    return left + right;
}

static uint64_t pstvnc_mpeg_scheduler_multiply_saturating(
    uint64_t left,
    uint64_t right)
{
    if (left != 0u && right > UINT64_MAX / left)
        return UINT64_MAX;

    return left * right;
}

/*
 * Return floor(value * multiplier / divisor) without allowing intermediate
 * multiplication to wrap. Saturation is monotonic: an impossible far-future
 * value remains far-future rather than wrapping into an early deadline.
 */
static uint64_t pstvnc_mpeg_scheduler_multiply_divide_saturating(
    uint64_t value,
    uint64_t multiplier,
    uint64_t divisor)
{
    uint64_t whole;
    uint64_t remainder;
    uint64_t whole_scaled;
    uint64_t remainder_scaled;

    if (divisor == 0u)
        return UINT64_MAX;

    whole = value / divisor;
    remainder = value % divisor;
    whole_scaled = pstvnc_mpeg_scheduler_multiply_saturating(
        whole,
        multiplier);
    remainder_scaled = pstvnc_mpeg_scheduler_multiply_saturating(
        remainder,
        multiplier);

    return pstvnc_mpeg_scheduler_add_saturating(
        whole_scaled,
        remainder_scaled / divisor);
}

static int pstvnc_mpeg_scheduler_clock_result(int clock_result)
{
    if (clock_result == PSTVNC_MEDIA_CLOCK_UNARMED)
        return PSTVNC_MPEG_SCHEDULER_CLOCK_UNARMED;

    if (clock_result == PSTVNC_MEDIA_CLOCK_OK)
        return PSTVNC_MPEG_SCHEDULER_OK;

    return PSTVNC_MPEG_SCHEDULER_CLOCK_FAILED;
}

int pstvnc_mpeg_scheduler_init_after_first_sync(
    pstvnc_mpeg_scheduler_t *scheduler,
    const pstvnc_media_clock_t *clock,
    const pstvnc_mpeg_scheduler_profile_t *profile,
    uint32_t run_generation,
    uint64_t first_sync_tick)
{
    pstvnc_mpeg_scheduler_t candidate;
    uint32_t ticks_per_second;
    uint64_t period_numerator;
    uint64_t video_deadline_zero;
    int armed;
    int clock_result;

    if (scheduler == NULL ||
        clock == NULL ||
        profile == NULL ||
        run_generation == 0u ||
        profile->fps_numerator == 0u ||
        profile->fps_denominator == 0u ||
        (profile->drop_enabled != 0 && profile->drop_enabled != 1) ||
        (profile->drop_enabled == 0 &&
         profile->drop_threshold_milliframes != 0u) ||
        (profile->drop_enabled != 0 &&
         profile->drop_threshold_milliframes == 0u))
        return PSTVNC_MPEG_SCHEDULER_INVALID;

    clock_result = pstvnc_media_clock_is_armed(clock, &armed);
    if (clock_result != PSTVNC_MEDIA_CLOCK_OK)
        return pstvnc_mpeg_scheduler_clock_result(clock_result);

    if (!armed)
        return PSTVNC_MPEG_SCHEDULER_CLOCK_UNARMED;

    clock_result = pstvnc_media_clock_tick_rate(
        clock,
        &ticks_per_second);
    if (clock_result != PSTVNC_MEDIA_CLOCK_OK)
        return pstvnc_mpeg_scheduler_clock_result(clock_result);

    period_numerator = pstvnc_mpeg_scheduler_multiply_saturating(
        (uint64_t)ticks_per_second,
        (uint64_t)profile->fps_denominator);

    memset(&candidate, 0, sizeof(candidate));
    candidate.frame_period_ticks =
        period_numerator / (uint64_t)profile->fps_numerator;

    if (candidate.frame_period_ticks == 0u)
        return PSTVNC_MPEG_SCHEDULER_INVALID;

    clock_result = pstvnc_media_clock_video_deadline(
        clock,
        0u,
        &video_deadline_zero);
    if (clock_result != PSTVNC_MEDIA_CLOCK_OK)
        return pstvnc_mpeg_scheduler_clock_result(clock_result);

    candidate.clock = clock;
    candidate.profile = *profile;
    candidate.run_generation = run_generation;
    candidate.run_base_additional_ticks =
        first_sync_tick > video_deadline_zero
        ? first_sync_tick - video_deadline_zero
        : 0u;

    if (profile->drop_enabled) {
        candidate.drop_threshold_ticks =
            pstvnc_mpeg_scheduler_multiply_divide_saturating(
                candidate.frame_period_ticks,
                (uint64_t)profile->drop_threshold_milliframes,
                PSTVNC_MPEG_SCHEDULER_MILLIFRAMES_PER_FRAME);

        if (candidate.drop_threshold_ticks == 0u)
            return PSTVNC_MPEG_SCHEDULER_INVALID;
    }

    candidate.initialized = 1;
    *scheduler = candidate;
    return PSTVNC_MPEG_SCHEDULER_OK;
}

int pstvnc_mpeg_scheduler_decide(
    const pstvnc_mpeg_scheduler_t *scheduler,
    uint32_t run_generation,
    uint64_t picture_ordinal,
    uint64_t current_tick,
    pstvnc_mpeg_scheduler_decision_result_t *result)
{
    pstvnc_mpeg_scheduler_decision_result_t candidate;
    uint64_t ordinal_offset;
    uint64_t additional_ticks;
    int clock_result;

    if (scheduler == NULL ||
        result == NULL ||
        !scheduler->initialized ||
        scheduler->clock == NULL ||
        scheduler->run_generation == 0u ||
        run_generation == 0u ||
        run_generation != scheduler->run_generation ||
        picture_ordinal < 2u)
        return PSTVNC_MPEG_SCHEDULER_INVALID;

    ordinal_offset = pstvnc_mpeg_scheduler_multiply_saturating(
        picture_ordinal - 1u,
        scheduler->frame_period_ticks);
    additional_ticks = pstvnc_mpeg_scheduler_add_saturating(
        scheduler->run_base_additional_ticks,
        ordinal_offset);

    memset(&candidate, 0, sizeof(candidate));
    candidate.decision = PSTVNC_MPEG_SCHEDULER_DECISION_INVALID;

    clock_result = pstvnc_media_clock_video_deadline(
        scheduler->clock,
        additional_ticks,
        &candidate.deadline_tick);
    if (clock_result != PSTVNC_MEDIA_CLOCK_OK)
        return pstvnc_mpeg_scheduler_clock_result(clock_result);

    if (current_tick < candidate.deadline_tick) {
        candidate.decision =
            PSTVNC_MPEG_SCHEDULER_WAIT_UNTIL_DEADLINE;
        *result = candidate;
        return PSTVNC_MPEG_SCHEDULER_OK;
    }

    candidate.lateness_ticks =
        current_tick - candidate.deadline_tick;

    if (scheduler->profile.drop_enabled &&
        candidate.lateness_ticks >= scheduler->drop_threshold_ticks) {
        candidate.decision =
            PSTVNC_MPEG_SCHEDULER_DROP_PRESENTATION_ONLY;
    } else {
        candidate.decision =
            PSTVNC_MPEG_SCHEDULER_PRESENT_NOW;
    }

    *result = candidate;
    return PSTVNC_MPEG_SCHEDULER_OK;
}
