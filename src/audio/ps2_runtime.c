/*
 * File synopsis:
 * Implements R37's concrete PlayStation 2 execution adapter for the accepted
 * AUDIO session owner. It provides exact aligned allocation tracking, one
 * session-state semaphore, one retained completion semaphore, and one EE thread
 * slot whose join/destroy path requires actual THS_DORMANT proof.
 *
 * Resident LIBSD/AUDSRV preparation is explicit and idempotent. The embedded
 * AUDSRV image is loaded once after LIBSD and never unloaded per session.
 * Worker completion is retained after its one-shot semaphore is consumed, so a
 * later status failure can be retried without losing the completion fence.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A002-PS2-AUDIO-EXECUTION-BINDING-R37.
 */

#include "ps2_runtime.h"

#include <delaythread.h>
#include <kernel.h>
#include <loadfile.h>

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern unsigned char AUDSRV_irx[];
extern unsigned int size_AUDSRV_irx;

static int pstvnc_audio_ps2_alignment_valid(size_t alignment)
{
    return alignment != 0u &&
        (alignment & (alignment - 1u)) == 0u;
}

int pstvnc_audio_ps2_resident_prepare(
    pstvnc_audio_ps2_resident_t *resident)
{
    int result;

    if (resident == NULL)
        return -1;

    if (resident->audsrv_loaded && !resident->libsd_loaded)
        return -1;

    if (!resident->libsd_loaded) {
        result = SifLoadModule("rom0:LIBSD", 0, NULL);
        if (result < 0)
            return -1;

        resident->libsd_loaded = 1;
    }

    if (!resident->audsrv_loaded) {
        result = SifExecModuleBuffer(
            AUDSRV_irx,
            size_AUDSRV_irx,
            0,
            NULL,
            NULL);
        if (result < 0)
            return -1;

        resident->audsrv_loaded = 1;
    }

    return 0;
}

static pstvnc_audio_ps2_allocation_t *
pstvnc_audio_ps2_runtime_free_allocation_slot(
    pstvnc_audio_ps2_runtime_t *runtime)
{
    size_t index;

    for (index = 0u;
         index < PSTVNC_AUDIO_PS2_RUNTIME_MAX_ALLOCATIONS;
         index++) {
        if (runtime->allocations[index].aligned_memory == NULL)
            return &runtime->allocations[index];
    }

    return NULL;
}

static void *pstvnc_audio_ps2_runtime_allocate(
    void *context,
    size_t byte_count,
    size_t alignment)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)context;
    pstvnc_audio_ps2_allocation_t *slot;
    uintptr_t raw_address;
    uintptr_t aligned_address;
    void *raw_memory;
    size_t extra;

    if (runtime == NULL ||
        !runtime->initialized ||
        byte_count == 0u ||
        !pstvnc_audio_ps2_alignment_valid(alignment))
        return NULL;

    extra = alignment - 1u;
    if (byte_count > (size_t)-1 - extra)
        return NULL;

    slot = pstvnc_audio_ps2_runtime_free_allocation_slot(runtime);
    if (slot == NULL)
        return NULL;

    raw_memory = malloc(byte_count + extra);
    if (raw_memory == NULL)
        return NULL;

    raw_address = (uintptr_t)raw_memory;
    aligned_address =
        (raw_address + (uintptr_t)extra) &
        ~(uintptr_t)extra;

    if ((aligned_address % (uintptr_t)alignment) != 0u) {
        free(raw_memory);
        return NULL;
    }

    slot->raw_memory = raw_memory;
    slot->aligned_memory = (void *)aligned_address;
    runtime->live_allocations++;

    return slot->aligned_memory;
}

static void pstvnc_audio_ps2_runtime_free(
    void *context,
    void *memory)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)context;
    size_t index;

    if (runtime == NULL || memory == NULL)
        return;

    for (index = 0u;
         index < PSTVNC_AUDIO_PS2_RUNTIME_MAX_ALLOCATIONS;
         index++) {
        pstvnc_audio_ps2_allocation_t *slot =
            &runtime->allocations[index];

        if (slot->aligned_memory != memory)
            continue;

        if (slot->raw_memory == NULL || runtime->live_allocations == 0u)
            return;

        free(slot->raw_memory);
        slot->raw_memory = NULL;
        slot->aligned_memory = NULL;
        runtime->live_allocations--;
        return;
    }
}

static int pstvnc_audio_ps2_runtime_lock(void *context)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)context;

    if (runtime == NULL ||
        !runtime->initialized ||
        runtime->session_lock_sema_id < 0)
        return -1;

    return WaitSema(runtime->session_lock_sema_id) >= 0 ? 0 : -1;
}

static int pstvnc_audio_ps2_runtime_unlock(void *context)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)context;

    if (runtime == NULL ||
        !runtime->initialized ||
        runtime->session_lock_sema_id < 0)
        return -1;

    return SignalSema(runtime->session_lock_sema_id) >= 0 ? 0 : -1;
}

static void pstvnc_audio_ps2_runtime_thread_trampoline(void *argument)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)argument;
    pstvnc_audio_session_thread_entry_t entry;
    void *entry_argument;

    if (runtime == NULL)
        ExitThread();

    entry = runtime->thread_entry;
    entry_argument = runtime->thread_argument;

    if (entry != NULL)
        entry(entry_argument);

    if (runtime->completion_sema_id >= 0)
        (void)SignalSema(runtime->completion_sema_id);

    ExitThread();
}

static int pstvnc_audio_ps2_runtime_thread_create(
    void *context,
    pstvnc_audio_session_thread_entry_t entry,
    void *argument,
    void *stack,
    size_t stack_bytes,
    int priority,
    int *thread_id)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)context;
    ee_thread_t thread;
    int created_id;

    if (runtime == NULL ||
        !runtime->initialized ||
        entry == NULL ||
        stack == NULL ||
        stack_bytes == 0u ||
        stack_bytes > (size_t)INT_MAX ||
        priority <= 0 ||
        thread_id == NULL ||
        runtime->thread_slot_active)
        return -1;

    memset(&thread, 0, sizeof(thread));
    thread.func = (void *)pstvnc_audio_ps2_runtime_thread_trampoline;
    thread.stack = stack;
    thread.stack_size = (int)stack_bytes;
    thread.gp_reg = &_gp;
    thread.initial_priority = priority;
    thread.attr = 0;
    thread.option = 0;

    created_id = CreateThread(&thread);
    if (created_id < 0)
        return -1;

    runtime->thread_id = created_id;
    runtime->thread_entry = entry;
    runtime->thread_argument = argument;
    runtime->thread_slot_active = 1;
    runtime->thread_started = 0;
    runtime->completion_observed = 0;
    runtime->thread_dormant_proven = 0;

    *thread_id = created_id;
    return 0;
}

static int pstvnc_audio_ps2_runtime_thread_start(
    void *context,
    int thread_id)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)context;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->thread_slot_active ||
        runtime->thread_started ||
        runtime->thread_id != thread_id)
        return -1;

    if (StartThread(thread_id, runtime) < 0)
        return -1;

    runtime->thread_started = 1;
    return 0;
}

static int pstvnc_audio_ps2_runtime_thread_join(
    void *context,
    int thread_id)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)context;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->thread_slot_active ||
        !runtime->thread_started ||
        runtime->thread_id != thread_id)
        return -1;

    if (!runtime->completion_observed) {
        if (runtime->completion_sema_id < 0 ||
            WaitSema(runtime->completion_sema_id) < 0)
            return -1;

        runtime->completion_observed = 1;
    }

    if (runtime->thread_dormant_proven)
        return 0;

    for (;;) {
        ee_thread_status_t status;

        memset(&status, 0, sizeof(status));
        if (ReferThreadStatus(thread_id, &status) < 0)
            return -1;

        if (status.status == THS_DORMANT) {
            runtime->thread_dormant_proven = 1;
            return 0;
        }

        if (DelayThread(1u) < 0)
            return -1;
    }
}

static int pstvnc_audio_ps2_runtime_thread_destroy(
    void *context,
    int thread_id)
{
    pstvnc_audio_ps2_runtime_t *runtime =
        (pstvnc_audio_ps2_runtime_t *)context;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->thread_slot_active ||
        runtime->thread_id != thread_id)
        return -1;

    if (runtime->thread_started && !runtime->completion_observed)
        return -1;

    if (!runtime->thread_dormant_proven) {
        ee_thread_status_t status;

        memset(&status, 0, sizeof(status));
        if (ReferThreadStatus(thread_id, &status) < 0 ||
            status.status != THS_DORMANT)
            return -1;

        runtime->thread_dormant_proven = 1;
    }

    if (DeleteThread(thread_id) < 0)
        return -1;

    runtime->thread_id = -1;
    runtime->thread_entry = NULL;
    runtime->thread_argument = NULL;
    runtime->thread_slot_active = 0;
    runtime->thread_started = 0;
    runtime->completion_observed = 0;
    runtime->thread_dormant_proven = 0;

    return 0;
}

static int pstvnc_audio_ps2_runtime_create_semaphore(
    int initial_count,
    int maximum_count)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = initial_count;
    semaphore.max_count = maximum_count;
    semaphore.option = 0;

    return CreateSema(&semaphore);
}

static int pstvnc_audio_ps2_runtime_delete_owned_semaphores(
    pstvnc_audio_ps2_runtime_t *runtime)
{
    int result = 0;

    if (runtime->completion_sema_id >= 0) {
        if (DeleteSema(runtime->completion_sema_id) < 0) {
            result = -1;
        } else {
            runtime->completion_sema_id = -1;
        }
    }

    if (runtime->session_lock_sema_id >= 0) {
        if (DeleteSema(runtime->session_lock_sema_id) < 0) {
            result = -1;
        } else {
            runtime->session_lock_sema_id = -1;
        }
    }

    return result;
}

int pstvnc_audio_ps2_runtime_init(
    pstvnc_audio_ps2_runtime_t *runtime)
{
    if (runtime == NULL)
        return -1;

    memset(runtime, 0, sizeof(*runtime));
    runtime->session_lock_sema_id = -1;
    runtime->completion_sema_id = -1;
    runtime->thread_id = -1;

    runtime->session_lock_sema_id =
        pstvnc_audio_ps2_runtime_create_semaphore(1, 1);
    if (runtime->session_lock_sema_id < 0)
        return -1;

    runtime->resources_owned = 1;

    runtime->completion_sema_id =
        pstvnc_audio_ps2_runtime_create_semaphore(0, 1);
    if (runtime->completion_sema_id < 0)
        goto fail;

    runtime->initialized = 1;
    return 0;

fail:
    if (pstvnc_audio_ps2_runtime_delete_owned_semaphores(runtime) == 0)
        runtime->resources_owned = 0;

    return -1;
}

int pstvnc_audio_ps2_runtime_operations(
    pstvnc_audio_ps2_runtime_t *runtime,
    pstvnc_audio_session_memory_ops_t *memory_ops,
    pstvnc_audio_session_thread_ops_t *thread_ops,
    pstvnc_audio_session_sync_t *sync)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        runtime->session_lock_sema_id < 0 ||
        runtime->completion_sema_id < 0 ||
        memory_ops == NULL ||
        thread_ops == NULL ||
        sync == NULL)
        return -1;

    memset(memory_ops, 0, sizeof(*memory_ops));
    memset(thread_ops, 0, sizeof(*thread_ops));
    memset(sync, 0, sizeof(*sync));

    memory_ops->allocate = pstvnc_audio_ps2_runtime_allocate;
    memory_ops->release = pstvnc_audio_ps2_runtime_free;
    memory_ops->context = runtime;

    thread_ops->create = pstvnc_audio_ps2_runtime_thread_create;
    thread_ops->start = pstvnc_audio_ps2_runtime_thread_start;
    thread_ops->join = pstvnc_audio_ps2_runtime_thread_join;
    thread_ops->destroy = pstvnc_audio_ps2_runtime_thread_destroy;
    thread_ops->context = runtime;

    sync->lock = pstvnc_audio_ps2_runtime_lock;
    sync->unlock = pstvnc_audio_ps2_runtime_unlock;
    sync->context = runtime;

    return 0;
}

int pstvnc_audio_ps2_runtime_release(
    pstvnc_audio_ps2_runtime_t *runtime)
{
    if (runtime == NULL || !runtime->resources_owned)
        return -1;

    if (runtime->thread_slot_active || runtime->live_allocations != 0u)
        return -1;

    if (pstvnc_audio_ps2_runtime_delete_owned_semaphores(runtime) != 0)
        return -1;

    runtime->initialized = 0;
    runtime->resources_owned = 0;
    return 0;
}
