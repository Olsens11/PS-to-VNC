/*
 * File synopsis:
 * Defines A003's MPEG-owned asynchronous decoder worker and one-slot borrowed-
 * picture rendezvous. The worker is the sole initialize/step/release owner for
 * one synchronous decoder interval. A later consumer may nonblockingly claim
 * exactly one borrowed MPEG-domain picture and must release that exact claim
 * before the decoder can advance or reclaim its picture storage.
 *
 * Thread, stack, synchronization, and retained-signal event mechanics are
 * injected by the caller. This module owns no Display scheduler/compositor,
 * Presentation, GS, RFB, Application, Transport queue internals, or generation
 * allocation.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-MPEG-WORKER-FRAME-RENDEZVOUS-R4.
 */

#ifndef PSTVNC_MPEG_WORKER_H
#define PSTVNC_MPEG_WORKER_H

#include "decoder.h"

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_MPEG_WORKER_STACK_ALIGNMENT 16u

typedef struct pstvnc_mpeg_worker_values {
    size_t worker_stack_bytes;
    int worker_priority;
} pstvnc_mpeg_worker_values_t;

typedef struct pstvnc_mpeg_worker_memory_ops {
    void *(*allocate)(void *context, size_t byte_count, size_t alignment);
    void (*release)(void *context, void *memory);
    void *context;
} pstvnc_mpeg_worker_memory_ops_t;

typedef void (*pstvnc_mpeg_worker_thread_entry_t)(void *argument);

typedef struct pstvnc_mpeg_worker_thread_ops {
    int (*create)(
        void *context,
        pstvnc_mpeg_worker_thread_entry_t entry,
        void *argument,
        void *stack,
        size_t stack_bytes,
        int priority,
        int *thread_id);
    int (*start)(void *context, int thread_id);
    int (*join)(void *context, int thread_id);
    int (*destroy)(void *context, int thread_id);
    void *context;
} pstvnc_mpeg_worker_thread_ops_t;

typedef struct pstvnc_mpeg_worker_sync_ops {
    int (*lock)(void *context);
    int (*unlock)(void *context);
    void *context;
} pstvnc_mpeg_worker_sync_ops_t;

/*
 * signal() must retain one pending wake until a later wait() consumes it.
 * wait() may return immediately for a signal that happened before wait entry.
 * This retained-signal contract is what makes release/stop wakeups lost-wake
 * safe without a correctness-critical polling timer.
 */
typedef struct pstvnc_mpeg_worker_event_ops {
    void (*signal)(void *context);
    void (*wait)(void *context);
    void *context;
} pstvnc_mpeg_worker_event_ops_t;

typedef enum pstvnc_mpeg_worker_slot_state {
    PSTVNC_MPEG_WORKER_SLOT_EMPTY = 0,
    PSTVNC_MPEG_WORKER_SLOT_AVAILABLE,
    PSTVNC_MPEG_WORKER_SLOT_CLAIMED
} pstvnc_mpeg_worker_slot_state_t;

typedef enum pstvnc_mpeg_worker_result {
    PSTVNC_MPEG_WORKER_OK = 0,
    PSTVNC_MPEG_WORKER_INVALID = -1,
    PSTVNC_MPEG_WORKER_STACK_ALLOCATION_FAILED = -2,
    PSTVNC_MPEG_WORKER_THREAD_CREATE_FAILED = -3,
    PSTVNC_MPEG_WORKER_THREAD_START_FAILED = -4,
    PSTVNC_MPEG_WORKER_WRONG_GENERATION = -5,
    PSTVNC_MPEG_WORKER_SYNC_FAILED = -6,
    PSTVNC_MPEG_WORKER_FRAME_UNAVAILABLE = -7,
    PSTVNC_MPEG_WORKER_CLAIM_MISMATCH = -8,
    PSTVNC_MPEG_WORKER_FRAME_OUTSTANDING = -9,
    PSTVNC_MPEG_WORKER_THREAD_JOIN_FAILED = -10,
    PSTVNC_MPEG_WORKER_NOT_FINISHED = -11,
    PSTVNC_MPEG_WORKER_NOT_JOINED = -12,
    PSTVNC_MPEG_WORKER_THREAD_DESTROY_FAILED = -13,
    PSTVNC_MPEG_WORKER_DECODER_STOP_FAILED = -14,
    PSTVNC_MPEG_WORKER_INTERNAL_FAILED = -15
} pstvnc_mpeg_worker_result_t;

typedef enum pstvnc_mpeg_worker_outcome_kind {
    PSTVNC_MPEG_WORKER_OUTCOME_NONE = 0,
    PSTVNC_MPEG_WORKER_OUTCOME_COMPLETED,
    PSTVNC_MPEG_WORKER_OUTCOME_STOPPED,
    PSTVNC_MPEG_WORKER_OUTCOME_FAILED
} pstvnc_mpeg_worker_outcome_kind_t;

typedef struct pstvnc_mpeg_worker_frame {
    pstvnc_mpeg_decoded_picture_t picture;
    uint32_t run_generation;
    uint32_t claim_token;
} pstvnc_mpeg_worker_frame_t;

typedef struct pstvnc_mpeg_worker_status {
    pstvnc_mpeg_worker_slot_state_t slot_state;
    int stop_requested;
    int decoder_live;
    int worker_finished;
    int thread_joined;
} pstvnc_mpeg_worker_status_t;

typedef struct pstvnc_mpeg_worker_outcome {
    pstvnc_mpeg_worker_outcome_kind_t kind;
    pstvnc_mpeg_worker_result_t worker_result;
    pstvnc_mpeg_decoder_result_t decoder_result;
    pstvnc_mpeg_decoder_result_t decoder_release_result;
    pstvnc_mpeg_decoder_report_t decoder_report;
    uint32_t run_generation;
} pstvnc_mpeg_worker_outcome_t;

typedef struct pstvnc_mpeg_worker {
    pstvnc_mpeg_decoder_t decoder;
    pstvnc_mpeg_decoder_config_t decoder_config;
    pstvnc_mpeg_decoder_memory_ops_t decoder_memory_ops;
    pstvnc_mpeg_decoder_sync_ops_t decoder_sync_ops;
    pstvnc_mpeg_decoder_platform_ops_t decoder_platform_ops;

    pstvnc_mpeg_worker_values_t values;
    pstvnc_mpeg_worker_memory_ops_t memory_ops;
    pstvnc_mpeg_worker_thread_ops_t thread_ops;
    pstvnc_mpeg_worker_sync_ops_t sync_ops;
    pstvnc_mpeg_worker_event_ops_t event_ops;

    void *worker_stack;
    int thread_id;
    uint32_t run_generation;

    int initialized;
    int thread_created;
    int thread_started;
    int thread_joined;
    int thread_destroyed;
    int stop_requested;
    int decoder_live;
    int worker_finished;

    pstvnc_mpeg_worker_slot_state_t slot_state;
    pstvnc_mpeg_decoded_picture_t slot_picture;
    pstvnc_mpeg_worker_outcome_t outcome;
} pstvnc_mpeg_worker_t;

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_start(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    const pstvnc_mpeg_worker_values_t *values,
    const pstvnc_mpeg_decoder_config_t *decoder_config,
    const pstvnc_mpeg_decoder_memory_ops_t *decoder_memory_ops,
    const pstvnc_mpeg_decoder_sync_ops_t *decoder_sync_ops,
    const pstvnc_mpeg_decoder_platform_ops_t *decoder_platform_ops,
    const pstvnc_mpeg_worker_memory_ops_t *memory_ops,
    const pstvnc_mpeg_worker_thread_ops_t *thread_ops,
    const pstvnc_mpeg_worker_sync_ops_t *sync_ops,
    const pstvnc_mpeg_worker_event_ops_t *event_ops);

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_request_stop(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation);

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_claim(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    pstvnc_mpeg_worker_frame_t *frame);

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_release_frame(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    uint32_t claim_token);

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_status(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    pstvnc_mpeg_worker_status_t *status);

/*
 * join() is the worker-write visibility fence. It refuses a non-empty slot and
 * also refuses a worker that has not yet reached its synchronized finished
 * state, so no borrowed picture or live decoder storage can be reclaimed.
 */
pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_join(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation);

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_outcome(
    const pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    pstvnc_mpeg_worker_outcome_t *outcome);

/* Destroy injected thread state and reclaim stack only after exact join. */
pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_release(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation);

#endif /* PSTVNC_MPEG_WORKER_H */
