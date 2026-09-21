/*
 * File synopsis:
 * Defines A003 R5's MPEG-owned PlayStation 2 execution binding for the already
 * accepted synchronous decoder and one-slot MPEG worker. One explicit binding
 * owns aligned allocation callbacks, independent decoder/worker locks, one
 * retained frame-slot wake event, and one EE worker-thread slot.
 *
 * The binding supplies mechanism only. Caller-owned worker stack size, priority,
 * and bounded join polling values remain explicit; this interface does not start
 * an MPEG run, allocate generation identity, perform decode/presentation work,
 * or own Transport, RFB, producer, scheduler, or Application lifecycle.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-PS2-WORKER-EXECUTION-BINDING-R5.
 */

#ifndef PSTVNC_MPEG_PS2_WORKER_RUNTIME_H
#define PSTVNC_MPEG_PS2_WORKER_RUNTIME_H

#include "worker.h"

#include <stddef.h>

typedef struct pstvnc_mpeg_ps2_worker_runtime_values {
    unsigned int join_poll_delay_us;
    unsigned int join_poll_max_count;
} pstvnc_mpeg_ps2_worker_runtime_values_t;

typedef struct pstvnc_mpeg_ps2_worker_runtime {
    pstvnc_mpeg_ps2_worker_runtime_values_t values;

    int decoder_lock_sema_id;
    int worker_lock_sema_id;
    int frame_event_sema_id;

    int thread_id;
    pstvnc_mpeg_worker_thread_entry_t thread_entry;
    void *thread_argument;

    size_t live_allocations;

    int resources_owned;
    int initialized;
    int thread_slot_active;
    int thread_started;
    int thread_dormant_proven;
} pstvnc_mpeg_ps2_worker_runtime_t;

/*
 * Acquire the three PS2 semaphore resources owned by one worker interval.
 *
 * join_poll_delay_us and join_poll_max_count are caller policy and must both be
 * nonzero. No stack size, worker priority, or timing default is introduced here.
 */
int pstvnc_mpeg_ps2_worker_runtime_init(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime,
    const pstvnc_mpeg_ps2_worker_runtime_values_t *values);

/*
 * Bind one initialized runtime into the already-accepted R2/R4 operation tables.
 *
 * decoder_sync_ops and worker_sync_ops intentionally refer to different owned
 * semaphore resources. frame_event_ops refers to a third count-0/max-1
 * semaphore that retains one signal until a later blocking wait consumes it.
 */
int pstvnc_mpeg_ps2_worker_runtime_operations(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime,
    pstvnc_mpeg_decoder_memory_ops_t *decoder_memory_ops,
    pstvnc_mpeg_decoder_sync_ops_t *decoder_sync_ops,
    pstvnc_mpeg_worker_memory_ops_t *worker_memory_ops,
    pstvnc_mpeg_worker_thread_ops_t *worker_thread_ops,
    pstvnc_mpeg_worker_sync_ops_t *worker_sync_ops,
    pstvnc_mpeg_worker_event_ops_t *frame_event_ops);

/*
 * Release kernel resources only after R4 has joined and released its worker.
 *
 * A live thread slot or any allocation still owned through this binding causes
 * failure and leaves resources intact. Partial-init cleanup failures can also be
 * retried through this operation because resources_owned remains explicit.
 */
int pstvnc_mpeg_ps2_worker_runtime_release(
    pstvnc_mpeg_ps2_worker_runtime_t *runtime);

#endif /* PSTVNC_MPEG_PS2_WORKER_RUNTIME_H */
