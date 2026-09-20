/*
 * File synopsis:
 * Implements A002's session-scoped audio worker/resource lifecycle around the
 * accepted synchronous PCM playback core. Startup reservoir readiness is
 * observed through Transport AUDIO status/activity without dequeuing PCM;
 * presentation waits through the existing common-clock audio deadline; and
 * stack/buffer reclamation is fenced behind proven worker completion/join.
 *
 * This owner never arms the common clock, never receives/closes the physical
 * Transport socket, and never duplicates synchronous wait/play/accounting
 * semantics. Application-level Transport abort/close ordering remains outside.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#include "session.h"

#include "transport/bridge.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

static int pstvnc_audio_session_values_valid(
    const pstvnc_audio_session_values_t *values)
{
    return values != NULL &&
           values->worker_stack_bytes != 0u &&
           values->worker_stack_bytes <= (size_t)INT_MAX &&
           values->worker_priority > 0 &&
           values->playback_buffer_capacity != 0u &&
           values->playback_buffer_capacity <= (size_t)INT_MAX &&
           values->startup_reservoir_bytes != 0u &&
           values->reservoir_poll_us != 0u &&
           values->clock_poll_us != 0u;
}

static int pstvnc_audio_session_memory_ops_valid(
    const pstvnc_audio_session_memory_ops_t *memory_ops)
{
    return memory_ops != NULL &&
           memory_ops->allocate != NULL &&
           memory_ops->release != NULL;
}

static int pstvnc_audio_session_thread_ops_valid(
    const pstvnc_audio_session_thread_ops_t *thread_ops)
{
    return thread_ops != NULL &&
           thread_ops->create != NULL &&
           thread_ops->start != NULL &&
           thread_ops->join != NULL &&
           thread_ops->destroy != NULL;
}

static int pstvnc_audio_session_sync_valid(
    const pstvnc_audio_session_sync_t *sync)
{
    return sync != NULL &&
           sync->lock != NULL &&
           sync->unlock != NULL;
}

static int pstvnc_audio_session_time_ops_valid(
    const pstvnc_media_clock_time_ops_t *time_ops)
{
    return time_ops != NULL &&
           time_ops->read_ticks != NULL &&
           time_ops->delay_us != NULL;
}

static void pstvnc_audio_session_free_owned_memory(
    pstvnc_audio_session_t *session)
{
    if (session->worker_stack != NULL) {
        session->memory_ops.release(
            session->memory_ops.context,
            session->worker_stack);
        session->worker_stack = NULL;
    }

    if (session->playback_buffer != NULL) {
        session->memory_ops.release(
            session->memory_ops.context,
            session->playback_buffer);
        session->playback_buffer = NULL;
    }
}

static int pstvnc_audio_session_observe_stop(
    pstvnc_audio_session_t *session,
    int *requested)
{
    int observed;

    if (session == NULL || requested == NULL)
        return -1;

    if (session->sync.lock(session->sync.context) != 0)
        return -1;

    observed = session->stop_requested;

    if (session->sync.unlock(session->sync.context) != 0)
        return -1;

    *requested = observed != 0;
    return 0;
}

static int pstvnc_audio_session_clock_stop_requested(void *context)
{
    pstvnc_audio_session_t *session =
        (pstvnc_audio_session_t *)context;
    int requested = 1;

    if (pstvnc_audio_session_observe_stop(session, &requested) != 0) {
        session->worker_sync_failed = 1;
        return 1;
    }

    return requested;
}

static void pstvnc_audio_session_set_transport_outcome(
    pstvnc_audio_session_t *session,
    pstvnc_transport_result_t result)
{
    session->outcome.kind = PSTVNC_AUDIO_SESSION_OUTCOME_TRANSPORT;
    session->outcome.transport_result = result;
}

static int pstvnc_audio_session_wait_reservoir(
    pstvnc_audio_session_t *session)
{
    uint32_t activity_sequence;
    pstvnc_transport_result_t transport_result;

    transport_result = pstvnc_transport_audio_activity_snapshot(
        &session->transport_access,
        &activity_sequence);
    if (transport_result != PSTVNC_TRANSPORT_OK) {
        pstvnc_audio_session_set_transport_outcome(
            session, transport_result);
        return 0;
    }

    for (;;) {
        size_t available_count = 0u;
        int producer_done = 0;
        int stop_requested = 0;

        if (pstvnc_audio_session_observe_stop(
                session, &stop_requested) != 0) {
            session->outcome.kind =
                PSTVNC_AUDIO_SESSION_OUTCOME_SYNC_FAILED;
            return 0;
        }

        if (stop_requested) {
            session->outcome.kind =
                PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED;
            return 0;
        }

        transport_result = pstvnc_transport_audio_status(
            &session->transport_access,
            &available_count,
            &producer_done);
        if (transport_result != PSTVNC_TRANSPORT_OK) {
            pstvnc_audio_session_set_transport_outcome(
                session, transport_result);
            return 0;
        }

        if (available_count >= session->values.startup_reservoir_bytes ||
            (producer_done && available_count != 0u)) {
            session->outcome.reservoir_bytes = available_count;
            session->outcome.producer_done = producer_done != 0;
            return 1;
        }

        if (producer_done) {
            session->outcome.kind =
                PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY;
            session->outcome.reservoir_bytes = 0u;
            session->outcome.producer_done = 1;
            return 0;
        }

        /*
         * The public blocking Transport activity wait is intentionally not used
         * here: a session-local stop cannot signal Transport's private waiter
         * without making audio a second Transport lifecycle owner. Instead,
         * poll only the persistent activity generation at an explicit bounded
         * cadence. A generation change cannot be lost, status is not re-polled
         * without activity, and session stop has a finite cancellation bound.
         */
        for (;;) {
            uint32_t observed_sequence = 0u;

            if (pstvnc_audio_session_observe_stop(
                    session, &stop_requested) != 0) {
                session->outcome.kind =
                    PSTVNC_AUDIO_SESSION_OUTCOME_SYNC_FAILED;
                return 0;
            }

            if (stop_requested) {
                session->outcome.kind =
                    PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED;
                return 0;
            }

            if (session->time_ops.delay_us(
                    session->time_ops.context,
                    session->values.reservoir_poll_us) != 0) {
                session->outcome.kind =
                    PSTVNC_AUDIO_SESSION_OUTCOME_RESERVOIR_DELAY_FAILED;
                return 0;
            }

            transport_result = pstvnc_transport_audio_activity_snapshot(
                &session->transport_access,
                &observed_sequence);
            if (transport_result != PSTVNC_TRANSPORT_OK) {
                pstvnc_audio_session_set_transport_outcome(
                    session, transport_result);
                return 0;
            }

            if (observed_sequence != activity_sequence) {
                activity_sequence = observed_sequence;
                break;
            }
        }
    }
}

static void pstvnc_audio_session_worker(void *argument)
{
    pstvnc_audio_session_t *session =
        (pstvnc_audio_session_t *)argument;
    pstvnc_media_clock_stop_observer_t stop_observer;
    int reservoir_ready;
    int clock_result;
    int stop_requested = 0;

    memset(&session->outcome, 0, sizeof(session->outcome));

    reservoir_ready = pstvnc_audio_session_wait_reservoir(session);
    if (!reservoir_ready)
        goto done;

    stop_observer.is_stop_requested =
        pstvnc_audio_session_clock_stop_requested;
    stop_observer.context = session;

    clock_result = pstvnc_media_clock_wait_audio(
        session->clock,
        0u,
        session->values.clock_poll_us,
        &session->time_ops,
        &stop_observer);

    if (session->worker_sync_failed) {
        session->outcome.kind =
            PSTVNC_AUDIO_SESSION_OUTCOME_SYNC_FAILED;
        goto done;
    }

    if (clock_result == PSTVNC_MEDIA_CLOCK_STOPPED) {
        session->outcome.kind =
            PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED;
        goto done;
    }

    if (clock_result != PSTVNC_MEDIA_CLOCK_OK) {
        session->outcome.kind =
            PSTVNC_AUDIO_SESSION_OUTCOME_CLOCK;
        session->outcome.clock_result =
            (pstvnc_media_clock_result_t)clock_result;
        goto done;
    }

    if (pstvnc_audio_session_observe_stop(
            session, &stop_requested) != 0) {
        session->outcome.kind =
            PSTVNC_AUDIO_SESSION_OUTCOME_SYNC_FAILED;
        goto done;
    }

    if (stop_requested) {
        session->outcome.kind =
            PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED;
        goto done;
    }

    session->outcome.kind =
        PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK;
    session->outcome.playback_result =
        pstvnc_audio_playback_run(
            &session->transport_access,
            &session->pcm_profile,
            session->playback_buffer,
            session->values.playback_buffer_capacity,
            &session->service,
            &session->outcome.playback_report);

done:
    session->worker_finished = 1;
}

pstvnc_audio_session_result_t pstvnc_audio_session_start(
    pstvnc_audio_session_t *session,
    const pstvnc_audio_session_values_t *values,
    const pstvnc_config_pcm_profile_t *pcm_profile,
    const pstvnc_audio_service_ops_t *service,
    const pstvnc_media_clock_t *clock,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_audio_session_memory_ops_t *memory_ops,
    const pstvnc_audio_session_thread_ops_t *thread_ops,
    const pstvnc_audio_session_sync_t *sync)
{
    pstvnc_transport_result_t transport_result;
    int create_result;
    int start_result;

    if (session == NULL ||
        !pstvnc_audio_session_values_valid(values) ||
        pcm_profile == NULL ||
        service == NULL ||
        clock == NULL ||
        !pstvnc_audio_session_time_ops_valid(time_ops) ||
        !pstvnc_audio_session_memory_ops_valid(memory_ops) ||
        !pstvnc_audio_session_thread_ops_valid(thread_ops) ||
        !pstvnc_audio_session_sync_valid(sync))
        return PSTVNC_AUDIO_SESSION_INVALID;

    memset(session, 0, sizeof(*session));

    transport_result =
        pstvnc_transport_access_acquire(&session->transport_access);
    if (transport_result != PSTVNC_TRANSPORT_OK)
        return PSTVNC_AUDIO_SESSION_TRANSPORT_UNAVAILABLE;

    session->values = *values;
    session->pcm_profile = *pcm_profile;
    session->service = *service;
    session->clock = clock;
    session->time_ops = *time_ops;
    session->memory_ops = *memory_ops;
    session->thread_ops = *thread_ops;
    session->sync = *sync;
    session->thread_id = -1;
    session->initialized = 1;

    session->playback_buffer =
        (uint8_t *)session->memory_ops.allocate(
            session->memory_ops.context,
            session->values.playback_buffer_capacity,
            1u);
    if (session->playback_buffer == NULL) {
        session->initialized = 0;
        return PSTVNC_AUDIO_SESSION_BUFFER_ALLOCATION_FAILED;
    }

    session->worker_stack =
        session->memory_ops.allocate(
            session->memory_ops.context,
            session->values.worker_stack_bytes,
            PSTVNC_AUDIO_SESSION_STACK_ALIGNMENT);
    if (session->worker_stack == NULL ||
        ((uintptr_t)session->worker_stack %
         (uintptr_t)PSTVNC_AUDIO_SESSION_STACK_ALIGNMENT) != 0u) {
        pstvnc_audio_session_free_owned_memory(session);
        session->initialized = 0;
        return PSTVNC_AUDIO_SESSION_STACK_ALLOCATION_FAILED;
    }

    create_result = session->thread_ops.create(
        session->thread_ops.context,
        pstvnc_audio_session_worker,
        session,
        session->worker_stack,
        session->values.worker_stack_bytes,
        session->values.worker_priority,
        &session->thread_id);
    if (create_result != 0 || session->thread_id < 0) {
        pstvnc_audio_session_free_owned_memory(session);
        session->thread_id = -1;
        session->initialized = 0;
        return PSTVNC_AUDIO_SESSION_THREAD_CREATE_FAILED;
    }

    session->thread_created = 1;

    start_result = session->thread_ops.start(
        session->thread_ops.context,
        session->thread_id);
    if (start_result != 0) {
        if (session->thread_ops.destroy(
                session->thread_ops.context,
                session->thread_id) == 0) {
            session->thread_destroyed = 1;
            session->thread_created = 0;
            session->thread_id = -1;
            pstvnc_audio_session_free_owned_memory(session);
            session->initialized = 0;
        }

        return PSTVNC_AUDIO_SESSION_THREAD_START_FAILED;
    }

    session->thread_started = 1;
    return PSTVNC_AUDIO_SESSION_OK;
}

pstvnc_audio_session_result_t pstvnc_audio_session_request_stop(
    pstvnc_audio_session_t *session)
{
    if (session == NULL || !session->initialized)
        return PSTVNC_AUDIO_SESSION_INVALID;

    if (session->sync.lock(session->sync.context) != 0)
        return PSTVNC_AUDIO_SESSION_SYNC_FAILED;

    session->stop_requested = 1;

    if (session->sync.unlock(session->sync.context) != 0)
        return PSTVNC_AUDIO_SESSION_SYNC_FAILED;

    return PSTVNC_AUDIO_SESSION_OK;
}

pstvnc_audio_session_result_t pstvnc_audio_session_join(
    pstvnc_audio_session_t *session)
{
    if (session == NULL || !session->initialized)
        return PSTVNC_AUDIO_SESSION_INVALID;

    if (session->thread_joined)
        return PSTVNC_AUDIO_SESSION_OK;

    if (!session->thread_started)
        return PSTVNC_AUDIO_SESSION_INVALID;

    if (session->thread_ops.join(
            session->thread_ops.context,
            session->thread_id) != 0)
        return PSTVNC_AUDIO_SESSION_THREAD_JOIN_FAILED;

    if (!session->worker_finished)
        return PSTVNC_AUDIO_SESSION_NOT_FINISHED;

    session->thread_joined = 1;
    return PSTVNC_AUDIO_SESSION_OK;
}

pstvnc_audio_session_result_t pstvnc_audio_session_release(
    pstvnc_audio_session_t *session)
{
    if (session == NULL)
        return PSTVNC_AUDIO_SESSION_INVALID;

    if (!session->initialized)
        return PSTVNC_AUDIO_SESSION_OK;

    if (session->thread_started && !session->thread_joined)
        return PSTVNC_AUDIO_SESSION_WORKER_LIVE;

    if (session->thread_created && !session->thread_destroyed) {
        if (session->thread_ops.destroy(
                session->thread_ops.context,
                session->thread_id) != 0)
            return PSTVNC_AUDIO_SESSION_THREAD_DESTROY_FAILED;

        session->thread_destroyed = 1;
        session->thread_created = 0;
        session->thread_started = 0;
        session->thread_id = -1;
    }

    pstvnc_audio_session_free_owned_memory(session);
    session->initialized = 0;
    return PSTVNC_AUDIO_SESSION_OK;
}

pstvnc_audio_session_result_t pstvnc_audio_session_outcome(
    const pstvnc_audio_session_t *session,
    pstvnc_audio_session_outcome_t *outcome)
{
    if (session == NULL || outcome == NULL)
        return PSTVNC_AUDIO_SESSION_INVALID;

    if (!session->worker_finished || !session->thread_joined)
        return PSTVNC_AUDIO_SESSION_NOT_FINISHED;

    *outcome = session->outcome;
    return PSTVNC_AUDIO_SESSION_OK;
}
