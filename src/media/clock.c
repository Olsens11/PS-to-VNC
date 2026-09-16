/*
 * File synopsis:
 * Implements the clean session common-media-clock state machine, exact signed
 * offset conversion, saturating deadline arithmetic, and stop/error-aware
 * waiting through injected synchronization and time observers. Platform timer
 * and lock bindings remain outside this owner, as do all media consumers.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#include "clock.h"

#include <limits.h>
#include <stdint.h>

#define PSTVNC_MEDIA_CLOCK_US_PER_SECOND 1000000ull

static uint64_t pstvnc_media_clock_add_saturating(
    uint64_t left,
    uint64_t right)
{
    if (UINT64_MAX - left < right)
        return UINT64_MAX;

    return left + right;
}

static uint64_t pstvnc_media_clock_unsigned_us_to_ticks(
    uint32_t microseconds,
    uint32_t ticks_per_second)
{
    return
        ((uint64_t)microseconds * (uint64_t)ticks_per_second) /
        PSTVNC_MEDIA_CLOCK_US_PER_SECOND;
}

static int64_t pstvnc_media_clock_signed_us_to_ticks(
    int32_t microseconds,
    uint32_t ticks_per_second)
{
    return
        ((int64_t)microseconds * (int64_t)ticks_per_second) /
        (int64_t)PSTVNC_MEDIA_CLOCK_US_PER_SECOND;
}

static uint64_t pstvnc_media_clock_apply_offset(
    uint64_t epoch_tick,
    int32_t offset_us,
    uint32_t ticks_per_second)
{
    int64_t signed_offset = pstvnc_media_clock_signed_us_to_ticks(
        offset_us,
        ticks_per_second);

    if (signed_offset < 0) {
        uint64_t magnitude = (uint64_t)(-signed_offset);

        return magnitude > epoch_tick ? 0u : epoch_tick - magnitude;
    }

    return pstvnc_media_clock_add_saturating(
        epoch_tick,
        (uint64_t)signed_offset);
}

static int pstvnc_media_clock_lock(const pstvnc_media_clock_t *clock)
{
    if (clock == 0 || clock->sync.lock == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    if (clock->sync.lock(clock->sync.context) != 0)
        return PSTVNC_MEDIA_CLOCK_SYNC_FAILED;

    return PSTVNC_MEDIA_CLOCK_OK;
}

static int pstvnc_media_clock_unlock(const pstvnc_media_clock_t *clock)
{
    if (clock == 0 || clock->sync.unlock == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    if (clock->sync.unlock(clock->sync.context) != 0)
        return PSTVNC_MEDIA_CLOCK_SYNC_FAILED;

    return PSTVNC_MEDIA_CLOCK_OK;
}

static int pstvnc_media_clock_deadline_with_offset(
    const pstvnc_media_clock_t *clock,
    int32_t offset_us,
    uint64_t additional_ticks,
    uint64_t *deadline_tick)
{
    uint64_t epoch_tick;
    uint64_t deadline;
    int result;

    if (clock == 0 || deadline_tick == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    result = pstvnc_media_clock_epoch(clock, &epoch_tick);
    if (result != PSTVNC_MEDIA_CLOCK_OK)
        return result;

    deadline = pstvnc_media_clock_apply_offset(
        epoch_tick,
        offset_us,
        clock->ticks_per_second);
    deadline = pstvnc_media_clock_add_saturating(deadline, additional_ticks);
    *deadline_tick = deadline;
    return PSTVNC_MEDIA_CLOCK_OK;
}

static int pstvnc_media_clock_stop_requested(
    const pstvnc_media_clock_stop_observer_t *stop_observer)
{
    if (stop_observer == 0)
        return 0;

    return stop_observer->is_stop_requested(stop_observer->context) != 0;
}

static int pstvnc_media_clock_wait_offset(
    const pstvnc_media_clock_t *clock,
    int32_t offset_us,
    uint64_t additional_ticks,
    uint32_t poll_us,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_media_clock_stop_observer_t *stop_observer)
{
    uint64_t deadline_tick;
    int result;

    if (
        clock == 0 ||
        time_ops == 0 ||
        time_ops->read_ticks == 0 ||
        time_ops->delay_us == 0 ||
        poll_us == 0u ||
        (stop_observer != 0 && stop_observer->is_stop_requested == 0))
        return PSTVNC_MEDIA_CLOCK_INVALID;

    for (;;) {
        if (pstvnc_media_clock_stop_requested(stop_observer))
            return PSTVNC_MEDIA_CLOCK_STOPPED;

        result = pstvnc_media_clock_deadline_with_offset(
            clock,
            offset_us,
            additional_ticks,
            &deadline_tick);
        if (result == PSTVNC_MEDIA_CLOCK_OK)
            break;
        if (result != PSTVNC_MEDIA_CLOCK_UNARMED)
            return result;

        if (time_ops->delay_us(time_ops->context, poll_us) != 0)
            return PSTVNC_MEDIA_CLOCK_DELAY_FAILED;
    }

    for (;;) {
        uint64_t now_tick;

        if (pstvnc_media_clock_stop_requested(stop_observer))
            return PSTVNC_MEDIA_CLOCK_STOPPED;

        if (time_ops->read_ticks(time_ops->context, &now_tick) != 0)
            return PSTVNC_MEDIA_CLOCK_TIMER_FAILED;

        if (now_tick >= deadline_tick)
            return PSTVNC_MEDIA_CLOCK_OK;

        if (time_ops->delay_us(time_ops->context, poll_us) != 0)
            return PSTVNC_MEDIA_CLOCK_DELAY_FAILED;
    }
}

int pstvnc_media_clock_init(
    pstvnc_media_clock_t *clock,
    const pstvnc_config_media_clock_profile_t *profile,
    uint32_t ticks_per_second,
    const pstvnc_media_clock_sync_t *sync)
{
    if (
        clock == 0 ||
        profile == 0 ||
        sync == 0 ||
        sync->lock == 0 ||
        sync->unlock == 0 ||
        ticks_per_second == 0u)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    clock->profile = *profile;
    clock->sync = *sync;
    clock->ticks_per_second = ticks_per_second;
    clock->epoch_tick = 0u;
    clock->armed = 0;
    return PSTVNC_MEDIA_CLOCK_OK;
}

int pstvnc_media_clock_is_armed(
    const pstvnc_media_clock_t *clock,
    int *armed)
{
    int observed_armed;
    int result;

    if (clock == 0 || armed == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    result = pstvnc_media_clock_lock(clock);
    if (result != PSTVNC_MEDIA_CLOCK_OK)
        return result;

    observed_armed = clock->armed;
    result = pstvnc_media_clock_unlock(clock);
    if (result != PSTVNC_MEDIA_CLOCK_OK)
        return result;

    *armed = observed_armed != 0;
    return PSTVNC_MEDIA_CLOCK_OK;
}

int pstvnc_media_clock_epoch(
    const pstvnc_media_clock_t *clock,
    uint64_t *epoch_tick)
{
    uint64_t observed_epoch;
    int observed_armed;
    int result;

    if (clock == 0 || epoch_tick == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    result = pstvnc_media_clock_lock(clock);
    if (result != PSTVNC_MEDIA_CLOCK_OK)
        return result;

    observed_armed = clock->armed;
    observed_epoch = clock->epoch_tick;
    result = pstvnc_media_clock_unlock(clock);
    if (result != PSTVNC_MEDIA_CLOCK_OK)
        return result;

    if (!observed_armed)
        return PSTVNC_MEDIA_CLOCK_UNARMED;

    *epoch_tick = observed_epoch;
    return PSTVNC_MEDIA_CLOCK_OK;
}

int pstvnc_media_clock_arm(
    pstvnc_media_clock_t *clock,
    uint64_t observed_now_tick)
{
    uint64_t lead_ticks;
    int result;

    if (clock == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    result = pstvnc_media_clock_lock(clock);
    if (result != PSTVNC_MEDIA_CLOCK_OK)
        return result;

    if (clock->armed) {
        result = pstvnc_media_clock_unlock(clock);
        return result;
    }

    lead_ticks = pstvnc_media_clock_unsigned_us_to_ticks(
        clock->profile.epoch_lead_us,
        clock->ticks_per_second);

    /* Publish the complete epoch before armed while the session lock is held. */
    clock->epoch_tick = pstvnc_media_clock_add_saturating(
        observed_now_tick,
        lead_ticks);
    clock->armed = 1;

    result = pstvnc_media_clock_unlock(clock);
    return result;
}

int pstvnc_media_clock_audio_deadline(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint64_t *deadline_tick)
{
    if (clock == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    return pstvnc_media_clock_deadline_with_offset(
        clock,
        clock->profile.audio_presentation_offset_us,
        additional_ticks,
        deadline_tick);
}

int pstvnc_media_clock_video_deadline(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint64_t *deadline_tick)
{
    if (clock == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    return pstvnc_media_clock_deadline_with_offset(
        clock,
        clock->profile.video_presentation_offset_us,
        additional_ticks,
        deadline_tick);
}

int pstvnc_media_clock_wait_audio(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint32_t poll_us,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_media_clock_stop_observer_t *stop_observer)
{
    if (clock == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    return pstvnc_media_clock_wait_offset(
        clock,
        clock->profile.audio_presentation_offset_us,
        additional_ticks,
        poll_us,
        time_ops,
        stop_observer);
}

int pstvnc_media_clock_wait_video(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint32_t poll_us,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_media_clock_stop_observer_t *stop_observer)
{
    if (clock == 0)
        return PSTVNC_MEDIA_CLOCK_INVALID;

    return pstvnc_media_clock_wait_offset(
        clock,
        clock->profile.video_presentation_offset_us,
        additional_ticks,
        poll_us,
        time_ops,
        stop_observer);
}
