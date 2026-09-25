/*
 * File synopsis:
 * Binds the platform-neutral A002 media-clock synchronization/time observer
 * seams to one session-owned PS2 EE semaphore, GetTimerSystemTime()/kBUSCLK,
 * and DelayThread().
 *
 * The media owner remains the sole epoch/armed/deadline implementation. This
 * adapter contains no media-clock arm call and no duplicate timing state. Its
 * only mutable authority is one semaphore handle with a strict
 * NEW -> ACTIVE -> RETIRED lifetime.
 *
 * The timer/yield mechanism matches frozen H1 forensic authority at commit
 * 3426f28b93de9519ca93e5f0e0aaf8b67cfca845, whose h1_media_clock.c observes
 * GetTimerSystemTime() in kBUSCLK ticks and yields via DelayThread().
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md;
 * docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26.
 */

#include "ps2_media_clock.h"

#include <delaythread.h>
#include <kernel.h>
#include <timer.h>

#include <limits.h>
#include <stddef.h>

static int pstvnc_ps2_media_clock_binding_usable(
    const pstvnc_ps2_media_clock_binding_t *binding)
{
    return
        binding != NULL &&
        binding->state == PSTVNC_PS2_MEDIA_CLOCK_BINDING_ACTIVE &&
        binding->semaphore_id >= 0;
}

static int pstvnc_ps2_media_clock_lock(void *context)
{
    pstvnc_ps2_media_clock_binding_t *binding =
        (pstvnc_ps2_media_clock_binding_t *)context;

    if (!pstvnc_ps2_media_clock_binding_usable(binding))
        return -1;

    return WaitSema(binding->semaphore_id) < 0 ? -1 : 0;
}

static int pstvnc_ps2_media_clock_unlock(void *context)
{
    pstvnc_ps2_media_clock_binding_t *binding =
        (pstvnc_ps2_media_clock_binding_t *)context;

    if (!pstvnc_ps2_media_clock_binding_usable(binding))
        return -1;

    return SignalSema(binding->semaphore_id) < 0 ? -1 : 0;
}

static int pstvnc_ps2_media_clock_read_ticks(
    void *context,
    uint64_t *ticks)
{
    pstvnc_ps2_media_clock_binding_t *binding =
        (pstvnc_ps2_media_clock_binding_t *)context;

    if (!pstvnc_ps2_media_clock_binding_usable(binding) || ticks == NULL)
        return -1;

    *ticks = (uint64_t)GetTimerSystemTime();
    return 0;
}

static int pstvnc_ps2_media_clock_delay_us(
    void *context,
    uint32_t delay_us)
{
    pstvnc_ps2_media_clock_binding_t *binding =
        (pstvnc_ps2_media_clock_binding_t *)context;

    if (
        !pstvnc_ps2_media_clock_binding_usable(binding) ||
        delay_us > (uint32_t)INT_MAX)
        return -1;

    return DelayThread((int)delay_us) < 0 ? -1 : 0;
}

int pstvnc_ps2_media_clock_binding_init(
    pstvnc_ps2_media_clock_binding_t *binding)
{
    ee_sema_t semaphore;
    int semaphore_id;

    if (
        binding == NULL ||
        binding->state != PSTVNC_PS2_MEDIA_CLOCK_BINDING_NEW)
        return -1;

    binding->semaphore_id = -1;
    binding->state = PSTVNC_PS2_MEDIA_CLOCK_BINDING_RETIRED;

    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;

    semaphore_id = CreateSema(&semaphore);
    if (semaphore_id < 0)
        return -1;

    binding->semaphore_id = semaphore_id;
    binding->state = PSTVNC_PS2_MEDIA_CLOCK_BINDING_ACTIVE;
    return 0;
}

int pstvnc_ps2_media_clock_binding_release(
    pstvnc_ps2_media_clock_binding_t *binding)
{
    int semaphore_id;

    if (!pstvnc_ps2_media_clock_binding_usable(binding))
        return -1;

    semaphore_id = binding->semaphore_id;

    /*
     * Revoke local authority before asking the kernel to delete the primitive.
     * Even if kernel deletion fails, stale observer copies can never regain
     * usable access or become authority for a later session.
     */
    binding->semaphore_id = -1;
    binding->state = PSTVNC_PS2_MEDIA_CLOCK_BINDING_RETIRED;

    return DeleteSema(semaphore_id) < 0 ? -1 : 0;
}

int pstvnc_ps2_media_clock_binding_sync(
    pstvnc_ps2_media_clock_binding_t *binding,
    pstvnc_media_clock_sync_t *sync)
{
    if (sync == NULL)
        return -1;

    sync->lock = NULL;
    sync->unlock = NULL;
    sync->context = NULL;

    if (!pstvnc_ps2_media_clock_binding_usable(binding))
        return -1;

    sync->lock = pstvnc_ps2_media_clock_lock;
    sync->unlock = pstvnc_ps2_media_clock_unlock;
    sync->context = binding;
    return 0;
}

int pstvnc_ps2_media_clock_binding_time_ops(
    pstvnc_ps2_media_clock_binding_t *binding,
    pstvnc_media_clock_time_ops_t *time_ops)
{
    if (time_ops == NULL)
        return -1;

    time_ops->read_ticks = NULL;
    time_ops->delay_us = NULL;
    time_ops->context = NULL;

    if (!pstvnc_ps2_media_clock_binding_usable(binding))
        return -1;

    time_ops->read_ticks = pstvnc_ps2_media_clock_read_ticks;
    time_ops->delay_us = pstvnc_ps2_media_clock_delay_us;
    time_ops->context = binding;
    return 0;
}

int pstvnc_ps2_media_clock_binding_tick_rate(
    const pstvnc_ps2_media_clock_binding_t *binding,
    uint32_t *ticks_per_second)
{
    if (
        !pstvnc_ps2_media_clock_binding_usable(binding) ||
        ticks_per_second == NULL)
        return -1;

    *ticks_per_second = (uint32_t)kBUSCLK;
    return 0;
}

int pstvnc_ps2_media_clock_binding_current_tick(
    const pstvnc_ps2_media_clock_binding_t *binding,
    uint64_t *tick)
{
    if (!pstvnc_ps2_media_clock_binding_usable(binding) || tick == NULL)
        return -1;

    *tick = (uint64_t)GetTimerSystemTime();
    return 0;
}
