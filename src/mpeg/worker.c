/*
 * File synopsis:
 * Implements A003's MPEG-owned asynchronous decoder worker and one-slot
 * borrowed-picture rendezvous. Decoder initialization, step calls and release
 * occur only on the worker thread. Consumer claim/release operates solely on
 * synchronized MPEG-domain metadata and cannot advance the decoder.
 *
 * Local stop never becomes decoder callback EOF. A stop request is forwarded to
 * the synchronous decoder's existing boundary-aware stop flag; if a picture was
 * already published, AVAILABLE may be discarded but CLAIMED remains pinned until
 * exact release. The worker waits on a retained-signal event and rechecks the
 * protected slot predicate after every wake.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-MPEG-WORKER-FRAME-RENDEZVOUS-R4.
 */

#include "worker.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

static int pstvnc_mpeg_worker_values_valid(
    const pstvnc_mpeg_worker_values_t *values)
{
    return values != NULL &&
        values->worker_stack_bytes != 0u &&
        values->worker_stack_bytes <= (size_t)INT_MAX &&
        values->worker_priority > 0;
}

static int pstvnc_mpeg_worker_memory_ops_valid(
    const pstvnc_mpeg_worker_memory_ops_t *memory_ops)
{
    return memory_ops != NULL &&
        memory_ops->allocate != NULL &&
        memory_ops->release != NULL;
}

static int pstvnc_mpeg_worker_thread_ops_valid(
    const pstvnc_mpeg_worker_thread_ops_t *thread_ops)
{
    return thread_ops != NULL &&
        thread_ops->create != NULL &&
        thread_ops->start != NULL &&
        thread_ops->join != NULL &&
        thread_ops->destroy != NULL;
}

static int pstvnc_mpeg_worker_sync_ops_valid(
    const pstvnc_mpeg_worker_sync_ops_t *sync_ops)
{
    return sync_ops != NULL &&
        sync_ops->lock != NULL &&
        sync_ops->unlock != NULL;
}

static int pstvnc_mpeg_worker_event_ops_valid(
    const pstvnc_mpeg_worker_event_ops_t *event_ops)
{
    return event_ops != NULL &&
        event_ops->signal != NULL &&
        event_ops->wait != NULL;
}

static int pstvnc_mpeg_worker_generation_valid(
    const pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation)
{
    return worker != NULL &&
        worker->initialized &&
        run_generation != 0u &&
        worker->run_generation == run_generation;
}

static void pstvnc_mpeg_worker_clear_slot(
    pstvnc_mpeg_worker_t *worker)
{
    memset(&worker->slot_picture, 0, sizeof(worker->slot_picture));
    worker->slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;
}

static void pstvnc_mpeg_worker_set_failed_outcome(
    pstvnc_mpeg_worker_t *worker,
    pstvnc_mpeg_worker_result_t worker_result,
    pstvnc_mpeg_decoder_result_t decoder_result,
    const pstvnc_mpeg_decoder_report_t *report)
{
    worker->outcome.kind = PSTVNC_MPEG_WORKER_OUTCOME_FAILED;
    worker->outcome.worker_result = worker_result;
    worker->outcome.decoder_result = decoder_result;
    if (report != NULL)
        worker->outcome.decoder_report = *report;
}

static int pstvnc_mpeg_worker_observe_stop(
    pstvnc_mpeg_worker_t *worker,
    int *stop_requested)
{
    int observed;

    if (worker == NULL || stop_requested == NULL)
        return 0;

    if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
        return 0;

    observed = worker->stop_requested;

    if (worker->sync_ops.unlock(worker->sync_ops.context) != 0)
        return 0;

    *stop_requested = observed != 0;
    return 1;
}

static int pstvnc_mpeg_worker_set_decoder_live(
    pstvnc_mpeg_worker_t *worker,
    int live)
{
    if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
        return 0;

    worker->decoder_live = live != 0;

    if (worker->sync_ops.unlock(worker->sync_ops.context) != 0)
        return 0;

    return 1;
}

static int pstvnc_mpeg_worker_publish_picture(
    pstvnc_mpeg_worker_t *worker,
    const pstvnc_mpeg_decoded_picture_t *picture,
    int *stop_requested)
{
    if (worker == NULL || picture == NULL || stop_requested == NULL)
        return 0;

    if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
        return 0;

    if (worker->stop_requested) {
        *stop_requested = 1;
    } else if (worker->slot_state != PSTVNC_MPEG_WORKER_SLOT_EMPTY) {
        (void)worker->sync_ops.unlock(worker->sync_ops.context);
        return 0;
    } else {
        worker->slot_picture = *picture;
        worker->slot_state = PSTVNC_MPEG_WORKER_SLOT_AVAILABLE;
        *stop_requested = 0;
    }

    if (worker->sync_ops.unlock(worker->sync_ops.context) != 0)
        return 0;

    return 1;
}

static int pstvnc_mpeg_worker_wait_slot_empty(
    pstvnc_mpeg_worker_t *worker,
    int *stop_requested)
{
    for (;;) {
        int slot_empty;
        int observed_stop;

        if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
            return 0;

        observed_stop = worker->stop_requested != 0;

        if (observed_stop &&
            worker->slot_state == PSTVNC_MPEG_WORKER_SLOT_AVAILABLE)
            pstvnc_mpeg_worker_clear_slot(worker);

        slot_empty =
            worker->slot_state == PSTVNC_MPEG_WORKER_SLOT_EMPTY;

        if (worker->sync_ops.unlock(worker->sync_ops.context) != 0)
            return 0;

        if (slot_empty) {
            *stop_requested = observed_stop;
            return 1;
        }

        worker->event_ops.wait(worker->event_ops.context);
    }
}

static void pstvnc_mpeg_worker_thread(void *argument)
{
    pstvnc_mpeg_worker_t *worker = (pstvnc_mpeg_worker_t *)argument;
    pstvnc_mpeg_decoder_report_t report;
    pstvnc_mpeg_decoder_result_t decoder_result;
    pstvnc_mpeg_decoder_result_t release_result =
        PSTVNC_MPEG_DECODER_COMPLETE;
    int decoder_initialized = 0;
    int stop_requested = 0;

    memset(&report, 0, sizeof(report));
    memset(&worker->outcome, 0, sizeof(worker->outcome));
    worker->outcome.run_generation = worker->run_generation;
    worker->outcome.decoder_release_result = PSTVNC_MPEG_DECODER_COMPLETE;

    decoder_result = pstvnc_mpeg_decoder_initialize(
        &worker->decoder,
        &worker->decoder_config,
        &worker->decoder_memory_ops,
        &worker->decoder_sync_ops,
        &worker->decoder_platform_ops);

    if (decoder_result != PSTVNC_MPEG_DECODER_COMPLETE) {
        pstvnc_mpeg_worker_set_failed_outcome(
            worker,
            PSTVNC_MPEG_WORKER_OK,
            decoder_result,
            &report);
        goto done;
    }

    decoder_initialized = 1;

    if (!pstvnc_mpeg_worker_set_decoder_live(worker, 1)) {
        pstvnc_mpeg_worker_set_failed_outcome(
            worker,
            PSTVNC_MPEG_WORKER_SYNC_FAILED,
            PSTVNC_MPEG_DECODER_COMPLETE,
            &report);
        goto release_decoder;
    }

    if (!pstvnc_mpeg_worker_observe_stop(worker, &stop_requested)) {
        pstvnc_mpeg_worker_set_failed_outcome(
            worker,
            PSTVNC_MPEG_WORKER_SYNC_FAILED,
            PSTVNC_MPEG_DECODER_COMPLETE,
            &report);
        goto release_decoder;
    }

    if (stop_requested) {
        decoder_result = pstvnc_mpeg_decoder_request_stop(&worker->decoder);
        if (decoder_result != PSTVNC_MPEG_DECODER_COMPLETE) {
            pstvnc_mpeg_worker_set_failed_outcome(
                worker,
                PSTVNC_MPEG_WORKER_DECODER_STOP_FAILED,
                decoder_result,
                &report);
            goto release_decoder;
        }
    }

    for (;;) {
        pstvnc_mpeg_decoded_picture_t picture;

        decoder_result = pstvnc_mpeg_decoder_step(
            &worker->decoder,
            &picture,
            &report);

        if (decoder_result == PSTVNC_MPEG_DECODER_PICTURE_READY) {
            if (!pstvnc_mpeg_worker_publish_picture(
                    worker,
                    &picture,
                    &stop_requested)) {
                pstvnc_mpeg_worker_set_failed_outcome(
                    worker,
                    PSTVNC_MPEG_WORKER_SYNC_FAILED,
                    decoder_result,
                    &report);
                goto release_decoder;
            }

            if (stop_requested) {
                decoder_result =
                    pstvnc_mpeg_decoder_request_stop(&worker->decoder);
                if (decoder_result != PSTVNC_MPEG_DECODER_COMPLETE) {
                    pstvnc_mpeg_worker_set_failed_outcome(
                        worker,
                        PSTVNC_MPEG_WORKER_DECODER_STOP_FAILED,
                        decoder_result,
                        &report);
                    goto release_decoder;
                }
                continue;
            }

            if (!pstvnc_mpeg_worker_wait_slot_empty(
                    worker,
                    &stop_requested)) {
                pstvnc_mpeg_worker_set_failed_outcome(
                    worker,
                    PSTVNC_MPEG_WORKER_SYNC_FAILED,
                    decoder_result,
                    &report);
                goto release_decoder;
            }

            if (stop_requested) {
                decoder_result =
                    pstvnc_mpeg_decoder_request_stop(&worker->decoder);
                if (decoder_result != PSTVNC_MPEG_DECODER_COMPLETE) {
                    pstvnc_mpeg_worker_set_failed_outcome(
                        worker,
                        PSTVNC_MPEG_WORKER_DECODER_STOP_FAILED,
                        decoder_result,
                        &report);
                    goto release_decoder;
                }
            }

            continue;
        }

        if (decoder_result == PSTVNC_MPEG_DECODER_COMPLETE) {
            worker->outcome.kind = PSTVNC_MPEG_WORKER_OUTCOME_COMPLETED;
            worker->outcome.worker_result = PSTVNC_MPEG_WORKER_OK;
        } else if (decoder_result == PSTVNC_MPEG_DECODER_STOPPED) {
            worker->outcome.kind = PSTVNC_MPEG_WORKER_OUTCOME_STOPPED;
            worker->outcome.worker_result = PSTVNC_MPEG_WORKER_OK;
        } else {
            worker->outcome.kind = PSTVNC_MPEG_WORKER_OUTCOME_FAILED;
            worker->outcome.worker_result = PSTVNC_MPEG_WORKER_OK;
        }

        worker->outcome.decoder_result = decoder_result;
        worker->outcome.decoder_report = report;
        break;
    }

release_decoder:
    if (decoder_initialized) {
        if (!pstvnc_mpeg_worker_set_decoder_live(worker, 0)) {
            worker->outcome.kind = PSTVNC_MPEG_WORKER_OUTCOME_FAILED;
            worker->outcome.worker_result = PSTVNC_MPEG_WORKER_SYNC_FAILED;
        }

        release_result = pstvnc_mpeg_decoder_release(&worker->decoder);
        worker->outcome.decoder_release_result = release_result;

        if (release_result != PSTVNC_MPEG_DECODER_COMPLETE) {
            worker->outcome.kind = PSTVNC_MPEG_WORKER_OUTCOME_FAILED;
            if (worker->outcome.worker_result == PSTVNC_MPEG_WORKER_OK)
                worker->outcome.worker_result =
                    PSTVNC_MPEG_WORKER_INTERNAL_FAILED;
        }
    }

done:
    if (worker->sync_ops.lock(worker->sync_ops.context) == 0) {
        worker->worker_finished = 1;
        (void)worker->sync_ops.unlock(worker->sync_ops.context);
    }
}

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
    const pstvnc_mpeg_worker_event_ops_t *event_ops)
{
    int create_result;
    int start_result;

    if (worker == NULL ||
        run_generation == 0u ||
        !pstvnc_mpeg_worker_values_valid(values) ||
        decoder_config == NULL ||
        decoder_memory_ops == NULL ||
        decoder_sync_ops == NULL ||
        decoder_platform_ops == NULL ||
        !pstvnc_mpeg_worker_memory_ops_valid(memory_ops) ||
        !pstvnc_mpeg_worker_thread_ops_valid(thread_ops) ||
        !pstvnc_mpeg_worker_sync_ops_valid(sync_ops) ||
        !pstvnc_mpeg_worker_event_ops_valid(event_ops))
        return PSTVNC_MPEG_WORKER_INVALID;

    memset(worker, 0, sizeof(*worker));

    worker->run_generation = run_generation;
    worker->values = *values;
    worker->decoder_config = *decoder_config;
    worker->decoder_memory_ops = *decoder_memory_ops;
    worker->decoder_sync_ops = *decoder_sync_ops;
    worker->decoder_platform_ops = *decoder_platform_ops;
    worker->memory_ops = *memory_ops;
    worker->thread_ops = *thread_ops;
    worker->sync_ops = *sync_ops;
    worker->event_ops = *event_ops;
    worker->thread_id = -1;
    worker->slot_state = PSTVNC_MPEG_WORKER_SLOT_EMPTY;
    worker->initialized = 1;

    worker->worker_stack = worker->memory_ops.allocate(
        worker->memory_ops.context,
        worker->values.worker_stack_bytes,
        PSTVNC_MPEG_WORKER_STACK_ALIGNMENT);

    if (worker->worker_stack == NULL ||
        ((uintptr_t)worker->worker_stack %
         (uintptr_t)PSTVNC_MPEG_WORKER_STACK_ALIGNMENT) != 0u) {
        if (worker->worker_stack != NULL)
            worker->memory_ops.release(
                worker->memory_ops.context,
                worker->worker_stack);
        worker->worker_stack = NULL;
        worker->initialized = 0;
        return PSTVNC_MPEG_WORKER_STACK_ALLOCATION_FAILED;
    }

    create_result = worker->thread_ops.create(
        worker->thread_ops.context,
        pstvnc_mpeg_worker_thread,
        worker,
        worker->worker_stack,
        worker->values.worker_stack_bytes,
        worker->values.worker_priority,
        &worker->thread_id);

    if (create_result != 0 || worker->thread_id < 0) {
        worker->memory_ops.release(
            worker->memory_ops.context,
            worker->worker_stack);
        worker->worker_stack = NULL;
        worker->thread_id = -1;
        worker->initialized = 0;
        return PSTVNC_MPEG_WORKER_THREAD_CREATE_FAILED;
    }

    worker->thread_created = 1;

    start_result = worker->thread_ops.start(
        worker->thread_ops.context,
        worker->thread_id);

    if (start_result != 0) {
        if (worker->thread_ops.destroy(
                worker->thread_ops.context,
                worker->thread_id) == 0) {
            worker->thread_destroyed = 1;
            worker->thread_created = 0;
            worker->thread_id = -1;
            worker->memory_ops.release(
                worker->memory_ops.context,
                worker->worker_stack);
            worker->worker_stack = NULL;
            worker->initialized = 0;
        }
        return PSTVNC_MPEG_WORKER_THREAD_START_FAILED;
    }

    worker->thread_started = 1;
    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_request_stop(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation)
{
    int decoder_live;
    pstvnc_mpeg_decoder_result_t decoder_result;

    if (!pstvnc_mpeg_worker_generation_valid(worker, run_generation))
        return worker != NULL && worker->initialized
            ? PSTVNC_MPEG_WORKER_WRONG_GENERATION
            : PSTVNC_MPEG_WORKER_INVALID;

    if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;

    worker->stop_requested = 1;
    decoder_live = worker->decoder_live;

    if (worker->slot_state == PSTVNC_MPEG_WORKER_SLOT_AVAILABLE)
        pstvnc_mpeg_worker_clear_slot(worker);

    if (worker->sync_ops.unlock(worker->sync_ops.context) != 0)
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;

    if (decoder_live) {
        decoder_result = pstvnc_mpeg_decoder_request_stop(&worker->decoder);
        if (decoder_result != PSTVNC_MPEG_DECODER_COMPLETE)
            return PSTVNC_MPEG_WORKER_DECODER_STOP_FAILED;
    }

    worker->event_ops.signal(worker->event_ops.context);
    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_claim(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    pstvnc_mpeg_worker_frame_t *frame)
{
    if (frame == NULL)
        return PSTVNC_MPEG_WORKER_INVALID;

    memset(frame, 0, sizeof(*frame));

    if (!pstvnc_mpeg_worker_generation_valid(worker, run_generation))
        return worker != NULL && worker->initialized
            ? PSTVNC_MPEG_WORKER_WRONG_GENERATION
            : PSTVNC_MPEG_WORKER_INVALID;

    if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;

    if (worker->slot_state != PSTVNC_MPEG_WORKER_SLOT_AVAILABLE) {
        (void)worker->sync_ops.unlock(worker->sync_ops.context);
        return PSTVNC_MPEG_WORKER_FRAME_UNAVAILABLE;
    }

    worker->slot_state = PSTVNC_MPEG_WORKER_SLOT_CLAIMED;
    frame->picture = worker->slot_picture;
    frame->run_generation = worker->run_generation;
    frame->claim_token = worker->slot_picture.picture_ordinal;

    if (worker->sync_ops.unlock(worker->sync_ops.context) != 0) {
        memset(frame, 0, sizeof(*frame));
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;
    }

    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_release_frame(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    uint32_t claim_token)
{
    if (!pstvnc_mpeg_worker_generation_valid(worker, run_generation))
        return worker != NULL && worker->initialized
            ? PSTVNC_MPEG_WORKER_WRONG_GENERATION
            : PSTVNC_MPEG_WORKER_INVALID;

    if (claim_token == 0u)
        return PSTVNC_MPEG_WORKER_CLAIM_MISMATCH;

    if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;

    if (worker->slot_state != PSTVNC_MPEG_WORKER_SLOT_CLAIMED ||
        worker->slot_picture.picture_ordinal != claim_token) {
        (void)worker->sync_ops.unlock(worker->sync_ops.context);
        return PSTVNC_MPEG_WORKER_CLAIM_MISMATCH;
    }

    pstvnc_mpeg_worker_clear_slot(worker);

    if (worker->sync_ops.unlock(worker->sync_ops.context) != 0)
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;

    worker->event_ops.signal(worker->event_ops.context);
    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_status(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    pstvnc_mpeg_worker_status_t *status)
{
    if (status == NULL)
        return PSTVNC_MPEG_WORKER_INVALID;

    memset(status, 0, sizeof(*status));

    if (!pstvnc_mpeg_worker_generation_valid(worker, run_generation))
        return worker != NULL && worker->initialized
            ? PSTVNC_MPEG_WORKER_WRONG_GENERATION
            : PSTVNC_MPEG_WORKER_INVALID;

    if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;

    status->slot_state = worker->slot_state;
    status->stop_requested = worker->stop_requested;
    status->decoder_live = worker->decoder_live;
    status->worker_finished = worker->worker_finished;
    status->thread_joined = worker->thread_joined;

    if (worker->sync_ops.unlock(worker->sync_ops.context) != 0) {
        memset(status, 0, sizeof(*status));
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;
    }

    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_join(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation)
{
    pstvnc_mpeg_worker_slot_state_t slot_state;
    int worker_finished;

    if (!pstvnc_mpeg_worker_generation_valid(worker, run_generation))
        return worker != NULL && worker->initialized
            ? PSTVNC_MPEG_WORKER_WRONG_GENERATION
            : PSTVNC_MPEG_WORKER_INVALID;

    if (worker->thread_joined)
        return PSTVNC_MPEG_WORKER_OK;

    if (!worker->thread_started)
        return PSTVNC_MPEG_WORKER_INVALID;

    if (worker->sync_ops.lock(worker->sync_ops.context) != 0)
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;

    slot_state = worker->slot_state;
    worker_finished = worker->worker_finished;

    if (worker->sync_ops.unlock(worker->sync_ops.context) != 0)
        return PSTVNC_MPEG_WORKER_SYNC_FAILED;

    if (slot_state != PSTVNC_MPEG_WORKER_SLOT_EMPTY)
        return PSTVNC_MPEG_WORKER_FRAME_OUTSTANDING;

    if (!worker_finished)
        return PSTVNC_MPEG_WORKER_NOT_FINISHED;

    if (worker->thread_ops.join(
            worker->thread_ops.context,
            worker->thread_id) != 0)
        return PSTVNC_MPEG_WORKER_THREAD_JOIN_FAILED;

    worker->thread_joined = 1;
    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_outcome(
    const pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    pstvnc_mpeg_worker_outcome_t *outcome)
{
    if (worker == NULL || outcome == NULL)
        return PSTVNC_MPEG_WORKER_INVALID;

    if (!worker->initialized ||
        run_generation == 0u ||
        worker->run_generation != run_generation)
        return worker->initialized
            ? PSTVNC_MPEG_WORKER_WRONG_GENERATION
            : PSTVNC_MPEG_WORKER_INVALID;

    if (!worker->worker_finished || !worker->thread_joined)
        return PSTVNC_MPEG_WORKER_NOT_FINISHED;

    *outcome = worker->outcome;
    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_reclaim_unstarted(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation)
{
    if (!pstvnc_mpeg_worker_generation_valid(worker, run_generation))
        return worker != NULL && worker->initialized
            ? PSTVNC_MPEG_WORKER_WRONG_GENERATION
            : PSTVNC_MPEG_WORKER_INVALID;

    /*
     * This is deliberately narrower than ordinary release. It describes only
     * the owner prefix created by create-success/start-failure/destroy-failure:
     * one created thread that never ran, its stack, and no decoder/slot/outcome
     * authority. Any other shape belongs to the normal started-worker path or
     * is contradictory and must remain represented.
     */
    if (!worker->thread_created ||
        worker->thread_started ||
        worker->thread_joined ||
        worker->thread_destroyed ||
        worker->thread_id < 0 ||
        worker->worker_stack == NULL ||
        worker->stop_requested ||
        worker->decoder_live ||
        worker->worker_finished ||
        worker->slot_state != PSTVNC_MPEG_WORKER_SLOT_EMPTY ||
        worker->outcome.kind != PSTVNC_MPEG_WORKER_OUTCOME_NONE)
        return PSTVNC_MPEG_WORKER_INVALID;

    if (worker->thread_ops.destroy(
            worker->thread_ops.context,
            worker->thread_id) != 0)
        return PSTVNC_MPEG_WORKER_THREAD_DESTROY_FAILED;

    worker->thread_destroyed = 1;
    worker->thread_created = 0;
    worker->thread_id = -1;

    worker->memory_ops.release(
        worker->memory_ops.context,
        worker->worker_stack);
    worker->worker_stack = NULL;

    worker->initialized = 0;
    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_release(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation)
{
    if (!pstvnc_mpeg_worker_generation_valid(worker, run_generation))
        return worker != NULL && worker->initialized
            ? PSTVNC_MPEG_WORKER_WRONG_GENERATION
            : PSTVNC_MPEG_WORKER_INVALID;

    if (!worker->thread_joined)
        return PSTVNC_MPEG_WORKER_NOT_JOINED;

    if (worker->thread_created && !worker->thread_destroyed) {
        if (worker->thread_ops.destroy(
                worker->thread_ops.context,
                worker->thread_id) != 0)
            return PSTVNC_MPEG_WORKER_THREAD_DESTROY_FAILED;

        worker->thread_destroyed = 1;
        worker->thread_created = 0;
        worker->thread_started = 0;
        worker->thread_id = -1;
    }

    if (worker->worker_stack != NULL) {
        worker->memory_ops.release(
            worker->memory_ops.context,
            worker->worker_stack);
        worker->worker_stack = NULL;
    }

    worker->initialized = 0;
    return PSTVNC_MPEG_WORKER_OK;
}
