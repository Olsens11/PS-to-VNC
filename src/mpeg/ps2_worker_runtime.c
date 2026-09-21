/*
 * File synopsis:
 * Implements A003 R5's concrete PlayStation 2 execution adapter for the clean
 * MPEG decoder/worker contracts. It maps exact requested allocations to
 * memalign/free, owns separate decoder and worker semaphores, provides one
 * retained count-1 frame event, and maps the portable R4 thread contract to EE
 * CreateThread/StartThread/ReferThreadStatus/DeleteThread mechanics.
 *
 * The portable R4 entry returns normally, so this adapter executes it through a
 * PS2 trampoline that calls ExitThread(). Join proves THS_DORMANT with explicit
 * caller-supplied poll bounds before deletion is permitted. No force termination
 * or hidden H1 stack/priority/timing defaults are used.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-PS2-WORKER-EXECUTION-BINDING-R5.
 */

#include "ps2_worker_runtime.h"

#include <delaythread.h>
#include <kernel.h>
#include <limits.h>
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int pstvnc_mpeg_ps2_worker_runtime_values_valid(
    const pstvnc_mpeg_ps2_worker_runtime_values_t *values)
{
    return values != NULL &&
        values->join_poll_delay_us != 0u &&
        values->join_poll_max_count != 0u;
}

static int pstvnc_mpeg_ps2_worker_runtime_alignment_valid(size_t alignment)
{
    return alignment != 0u &&
        (alignment & (alignment - 1u)) == 0u;
}

static void *pstvnc_mpeg_ps2_worker_runtime_allocate(
    void *context,
    size_t byte_count,
    size_t alignment)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;
    void *memory;

    if (runtime == NULL ||
        !runtime->initialized ||
        byte_count == 0u ||
        !pstvnc_mpeg_ps2_worker_runtime_alignment_valid(alignment))
        return NULL;

    memory = memalign(alignment, byte_count);
    if (memory == NULL ||
        ((uintptr_t)memory % (uintptr_t)alignment) != 0u) {
        free(memory);
        return NULL;
    }

    runtime->live_allocations++;
    return memory;
}

static void pstvnc_mpeg_ps2_worker_runtime_free(
    void *context,
    void *memory)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

    if (runtime == NULL || memory == NULL || runtime->live_allocations == 0u)
        return;

    free(memory);
    runtime->live_allocations--;
}

static int pstvnc_mpeg_ps2_worker_runtime_lock_semaphore(int semaphore_id)
{
    return semaphore_id >= 0 && WaitSema(semaphore_id) >= 0 ? 0 : -1;
}

static int pstvnc_mpeg_ps2_worker_runtime_unlock_semaphore(int semaphore_id)
{
    return semaphore_id >= 0 && SignalSema(semaphore_id) >= 0 ? 0 : -1;
}

static int pstvnc_mpeg_ps2_worker_runtime_decoder_lock(void *context)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

    if (runtime == NULL)
        return -1;

    return pstvnc_mpeg_ps2_worker_runtime_lock_semaphore(
        runtime->decoder_lock_sema_id);
}

static int pstvnc_mpeg_ps2_worker_runtime_decoder_unlock(void *context)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

    if (runtime == NULL)
        return -1;

    return pstvnc_mpeg_ps2_worker_runtime_unlock_semaphore(
        runtime->decoder_lock_sema_id);
}

static int pstvnc_mpeg_ps2_worker_runtime_worker_lock(void *context)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

    if (runtime == NULL)
        return -1;

    return pstvnc_mpeg_ps2_worker_runtime_lock_semaphore(
        runtime->worker_lock_sema_id);
}

static int pstvnc_mpeg_ps2_worker_runtime_worker_unlock(void *context)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

    if (runtime == NULL)
        return -1;

    return pstvnc_mpeg_ps2_worker_runtime_unlock_semaphore(
        runtime->worker_lock_sema_id);
}

static void pstvnc_mpeg_ps2_worker_runtime_event_signal(void *context)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

    if (runtime == NULL || runtime->frame_event_sema_id < 0)
        return;

    /*
     * max_count=1 deliberately coalesces repeated signals. If the one retained
     * wake is already pending, a further SignalSema failure does not remove it.
     * R4 always rechecks its protected predicate after the blocking wait.
     */
    (void)SignalSema(runtime->frame_event_sema_id);
}

static void pstvnc_mpeg_ps2_worker_runtime_event_wait(void *context)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

    if (runtime == NULL || runtime->frame_event_sema_id < 0)
        return;

    (void)WaitSema(runtime->frame_event_sema_id);
}

static void pstvnc_mpeg_ps2_worker_runtime_thread_trampoline(void *argument)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)argument;
    pstvnc_mpeg_worker_thread_entry_t entry;
    void *entry_argument;

    if (runtime == NULL)
        ExitThread();

    entry = runtime->thread_entry;
    entry_argument = runtime->thread_argument;

    if (entry != NULL)
        entry(entry_argument);

    ExitThread();
}

static int pstvnc_mpeg_ps2_worker_runtime_thread_create(
    void *context,
    pstvnc_mpeg_worker_thread_entry_t entry,
    void *argument,
    void *stack,
    size_t stack_bytes,
    int priority,
    int *thread_id)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;
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
    thread.func = (void *)pstvnc_mpeg_ps2_worker_runtime_thread_trampoline;
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
    runtime->thread_dormant_proven = 0;

    *thread_id = created_id;
    return 0;
}

static int pstvnc_mpeg_ps2_worker_runtime_thread_start(
    void *context,
    int thread_id)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

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

static int pstvnc_mpeg_ps2_worker_runtime_thread_join(
    void *context,
    int thread_id)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;
    unsigned int poll;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->thread_slot_active ||
        !runtime->thread_started ||
        runtime->thread_id != thread_id)
        return -1;

    if (runtime->thread_dormant_proven)
        return 0;

    for (poll = 0u; poll < runtime->values.join_poll_max_count; poll++) {
        ee_thread_status_t status;

        memset(&status, 0, sizeof(status));
        if (ReferThreadStatus(thread_id, &status) < 0)
            return -1;

        if (status.status == THS_DORMANT) {
            runtime->thread_dormant_proven = 1;
            return 0;
        }

        if (poll + 1u < runtime->values.join_poll_max_count &&
            DelayThread(runtime->values.join_poll_delay_us) < 0)
            return -1;
    }

    return -1;
}

static int pstvnc_mpeg_ps2_worker_runtime_thread_destroy(
    void *context,
    int thread_id)
{
    pstvnc_mpeg_ps2_worker_runtime_t *runtime =
        (pstvnc_mpeg_ps2_worker_runtime_t *)context;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->thread_slot_active ||
        runtime->thread_id != thread_id)
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
    runtime->thread_dormant_proven = 0;

    return 0;
}

static int pstvnc_mpeg_ps2_worker_runtime_delete_owned_semaphores(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime)
{
    int result = 0;

    if (runtime->frame_event_sema_id >= 0) {
        if (DeleteSema(runtime->frame_event_sema_id) < 0) {
            result = -1;
        } else {
            runtime->frame_event_sema_id = -1;
        }
    }

    if (runtime->worker_lock_sema_id >= 0) {
        if (DeleteSema(runtime->worker_lock_sema_id) < 0) {
            result = -1;
        } else {
            runtime->worker_lock_sema_id = -1;
        }
    }

    if (runtime->decoder_lock_sema_id >= 0) {
        if (DeleteSema(runtime->decoder_lock_sema_id) < 0) {
            result = -1;
        } else {
            runtime->decoder_lock_sema_id = -1;
        }
    }

    return result;
}

static int pstvnc_mpeg_ps2_worker_runtime_create_semaphore(
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

int pstvnc_mpeg_ps2_worker_runtime_init(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime,
    const pstvnc_mpeg_ps2_worker_runtime_values_t *values)
{
    if (runtime == NULL ||
        !pstvnc_mpeg_ps2_worker_runtime_values_valid(values))
        return -1;

    memset(runtime, 0, sizeof(*runtime));
    runtime->values = *values;
    runtime->decoder_lock_sema_id = -1;
    runtime->worker_lock_sema_id = -1;
    runtime->frame_event_sema_id = -1;
    runtime->thread_id = -1;

    runtime->decoder_lock_sema_id =
        pstvnc_mpeg_ps2_worker_runtime_create_semaphore(1, 1);
    if (runtime->decoder_lock_sema_id < 0)
        return -1;

    runtime->resources_owned = 1;

    runtime->worker_lock_sema_id =
        pstvnc_mpeg_ps2_worker_runtime_create_semaphore(1, 1);
    if (runtime->worker_lock_sema_id < 0)
        goto fail;

    runtime->frame_event_sema_id =
        pstvnc_mpeg_ps2_worker_runtime_create_semaphore(0, 1);
    if (runtime->frame_event_sema_id < 0)
        goto fail;

    runtime->initialized = 1;
    return 0;

fail:
    if (pstvnc_mpeg_ps2_worker_runtime_delete_owned_semaphores(runtime) == 0)
        runtime->resources_owned = 0;

    return -1;
}

int pstvnc_mpeg_ps2_worker_runtime_operations(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime,
    pstvnc_mpeg_decoder_memory_ops_t *decoder_memory_ops,
    pstvnc_mpeg_decoder_sync_ops_t *decoder_sync_ops,
    pstvnc_mpeg_worker_memory_ops_t *worker_memory_ops,
    pstvnc_mpeg_worker_thread_ops_t *worker_thread_ops,
    pstvnc_mpeg_worker_sync_ops_t *worker_sync_ops,
    pstvnc_mpeg_worker_event_ops_t *frame_event_ops)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        runtime->decoder_lock_sema_id < 0 ||
        runtime->worker_lock_sema_id < 0 ||
        runtime->frame_event_sema_id < 0 ||
        decoder_memory_ops == NULL ||
        decoder_sync_ops == NULL ||
        worker_memory_ops == NULL ||
        worker_thread_ops == NULL ||
        worker_sync_ops == NULL ||
        frame_event_ops == NULL)
        return -1;

    memset(decoder_memory_ops, 0, sizeof(*decoder_memory_ops));
    memset(decoder_sync_ops, 0, sizeof(*decoder_sync_ops));
    memset(worker_memory_ops, 0, sizeof(*worker_memory_ops));
    memset(worker_thread_ops, 0, sizeof(*worker_thread_ops));
    memset(worker_sync_ops, 0, sizeof(*worker_sync_ops));
    memset(frame_event_ops, 0, sizeof(*frame_event_ops));

    decoder_memory_ops->allocate = pstvnc_mpeg_ps2_worker_runtime_allocate;
    decoder_memory_ops->release = pstvnc_mpeg_ps2_worker_runtime_free;
    decoder_memory_ops->context = runtime;

    decoder_sync_ops->lock = pstvnc_mpeg_ps2_worker_runtime_decoder_lock;
    decoder_sync_ops->unlock = pstvnc_mpeg_ps2_worker_runtime_decoder_unlock;
    decoder_sync_ops->context = runtime;

    worker_memory_ops->allocate = pstvnc_mpeg_ps2_worker_runtime_allocate;
    worker_memory_ops->release = pstvnc_mpeg_ps2_worker_runtime_free;
    worker_memory_ops->context = runtime;

    worker_thread_ops->create = pstvnc_mpeg_ps2_worker_runtime_thread_create;
    worker_thread_ops->start = pstvnc_mpeg_ps2_worker_runtime_thread_start;
    worker_thread_ops->join = pstvnc_mpeg_ps2_worker_runtime_thread_join;
    worker_thread_ops->destroy = pstvnc_mpeg_ps2_worker_runtime_thread_destroy;
    worker_thread_ops->context = runtime;

    worker_sync_ops->lock = pstvnc_mpeg_ps2_worker_runtime_worker_lock;
    worker_sync_ops->unlock = pstvnc_mpeg_ps2_worker_runtime_worker_unlock;
    worker_sync_ops->context = runtime;

    frame_event_ops->signal = pstvnc_mpeg_ps2_worker_runtime_event_signal;
    frame_event_ops->wait = pstvnc_mpeg_ps2_worker_runtime_event_wait;
    frame_event_ops->context = runtime;

    return 0;
}

int pstvnc_mpeg_ps2_worker_runtime_release(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime)
{
    if (runtime == NULL || !runtime->resources_owned)
        return -1;

    if (runtime->thread_slot_active || runtime->live_allocations != 0u)
        return -1;

    if (pstvnc_mpeg_ps2_worker_runtime_delete_owned_semaphores(runtime) != 0)
        return -1;

    runtime->initialized = 0;
    runtime->resources_owned = 0;
    return 0;
}
