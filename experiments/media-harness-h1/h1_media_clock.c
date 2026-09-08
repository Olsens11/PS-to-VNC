/*
 * File synopsis:
 * Implements H1's session-scoped shared media clock.
 */

#include "h1_media_clock.h"

#include <delaythread.h>
#include <ee_regs.h>
#include <kernel.h>
#include <timer.h>

#include <stdint.h>

#define H1_CLOCK_US_PER_SECOND 1000000ull

static u64 h1_clock_us_to_ticks_u64(uint32_t microseconds)
{
    return
        ((u64)microseconds * (u64)kBUSCLK) /
        H1_CLOCK_US_PER_SECOND;
}

static s64 h1_clock_us_to_ticks_s64(int32_t microseconds)
{
    s64 value = (s64)microseconds;

    return
        (value * (s64)kBUSCLK) /
        (s64)H1_CLOCK_US_PER_SECOND;
}

void pstvnc_h1_media_clock_init(
    pstvnc_h1_media_clock_t *clock,
    uint32_t lead_us)
{
    if (clock == 0)
        return;

    clock->armed = 0;
    clock->epoch_tick = 0;
    clock->lead_us = lead_us;
}

void pstvnc_h1_media_clock_arm_now(
    pstvnc_h1_media_clock_t *clock)
{
    u64 epoch;

    if (clock == 0 || clock->armed)
        return;

    epoch =
        GetTimerSystemTime() +
        h1_clock_us_to_ticks_u64(clock->lead_us);

    clock->epoch_tick = epoch;
    EE_SYNCL();
    clock->armed = 1;
    EE_SYNCL();
}

int pstvnc_h1_media_clock_is_armed(
    const pstvnc_h1_media_clock_t *clock)
{
    return clock != 0 && clock->armed != 0;
}

u64 pstvnc_h1_media_clock_deadline(
    const pstvnc_h1_media_clock_t *clock,
    int32_t offset_us,
    u64 additional_ticks)
{
    s64 signed_offset;
    u64 epoch;
    u64 deadline;

    if (clock == 0 || !clock->armed)
        return 0;

    EE_SYNCL();
    epoch = clock->epoch_tick;
    signed_offset = h1_clock_us_to_ticks_s64(offset_us);

    if (signed_offset < 0) {
        u64 magnitude = (u64)(-signed_offset);
        deadline = magnitude > epoch ? 0u : epoch - magnitude;
    } else {
        deadline = epoch + (u64)signed_offset;
    }

    if (~deadline < additional_ticks)
        return ~(u64)0;

    return deadline + additional_ticks;
}

int pstvnc_h1_media_clock_wait_offset(
    pstvnc_h1_media_clock_t *clock,
    int32_t offset_us,
    uint32_t poll_us,
    const volatile int *stop_requested)
{
    u64 deadline;

    if (clock == 0)
        return -1;

    if (poll_us == 0u)
        poll_us = 1u;

    while (!clock->armed) {
        if (stop_requested != 0 && *stop_requested)
            return 0;

        if (DelayThread(poll_us) < 0)
            return -1;
    }

    deadline = pstvnc_h1_media_clock_deadline(
        clock,
        offset_us,
        0u);

    for (;;) {
        u64 now;
        u64 remaining;
        u64 remaining_us;
        uint32_t delay_us;

        if (stop_requested != 0 && *stop_requested)
            return 0;

        now = GetTimerSystemTime();

        if (now >= deadline)
            return 1;

        remaining = deadline - now;
        remaining_us =
            (remaining * H1_CLOCK_US_PER_SECOND) /
            (u64)kBUSCLK;

        if (remaining_us == 0u)
            delay_us = 1u;
        else if (remaining_us > (u64)poll_us)
            delay_us = poll_us;
        else
            delay_us = (uint32_t)remaining_us;

        if (DelayThread(delay_us) < 0)
            return -1;
    }
}
