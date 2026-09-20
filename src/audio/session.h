/*
 * File synopsis:
 * Defines A002's session-scoped audio worker/resource lifecycle around the
 * accepted synchronous PCM playback core. Explicit caller authority supplies
 * worker stack/priority, playback-buffer capacity, startup reservoir, and
 * reservoir/clock wait cadences; no production tuning defaults live here.
 *
 * The owner observes Transport AUDIO readiness without consuming PCM, waits on
 * the existing common media clock without arming it, and exposes separate
 * stop/join/release steps so worker-visible memory cannot be reclaimed live.
 * Application Transport abort/close orchestration, MPEG/video presentation,
 * receive-poison repair, and hardware qualification remain outside this file.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#ifndef PSTVNC_AUDIO_SESSION_H
#define PSTVNC_AUDIO_SESSION_H

#include "audio/playback.h"
#include "media/clock.h"
#include "transport/transport.h"

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_AUDIO_SESSION_STACK_ALIGNMENT 16u

typedef struct pstvnc_audio_session_values {
    size_t worker_stack_bytes;
    int worker_priority;
    size_t playback_buffer_capacity;
    size_t startup_reservoir_bytes;
    uint32_t reservoir_poll_us;
    uint32_t clock_poll_us;
} pstvnc_audio_session_values_t;

typedef struct pstvnc_audio_session_memory_ops {
    void *(*allocate)(
        void *context,
        size_t byte_count,
        size_t alignment);
    void (*release)(
        void *context,
        void *memory);
    void *context;
} pstvnc_audio_session_memory_ops_t;

typedef void (*pstvnc_audio_session_thread_entry_t)(void *argument);

typedef struct pstvnc_audio_session_thread_ops {
    int (*create)(
        void *context,
        pstvnc_audio_session_thread_entry_t entry,
        void *argument,
        void *stack,
        size_t stack_bytes,
        int priority,
        int *thread_id);
    int (*start)(
        void *context,
        int thread_id);
    int (*join)(
        void *context,
        int thread_id);
    int (*destroy)(
        void *context,
        int thread_id);
    void *context;
} pstvnc_audio_session_thread_ops_t;

typedef struct pstvnc_audio_session_sync {
    int (*lock)(void *context);
    int (*unlock)(void *context);
    void *context;
} pstvnc_audio_session_sync_t;

typedef enum pstvnc_audio_session_result {
    PSTVNC_AUDIO_SESSION_OK = 0,
    PSTVNC_AUDIO_SESSION_INVALID = -1,
    PSTVNC_AUDIO_SESSION_BUFFER_ALLOCATION_FAILED = -2,
    PSTVNC_AUDIO_SESSION_STACK_ALLOCATION_FAILED = -3,
    PSTVNC_AUDIO_SESSION_THREAD_CREATE_FAILED = -4,
    PSTVNC_AUDIO_SESSION_THREAD_START_FAILED = -5,
    PSTVNC_AUDIO_SESSION_THREAD_JOIN_FAILED = -6,
    PSTVNC_AUDIO_SESSION_THREAD_DESTROY_FAILED = -7,
    PSTVNC_AUDIO_SESSION_WORKER_LIVE = -8,
    PSTVNC_AUDIO_SESSION_SYNC_FAILED = -9,
    PSTVNC_AUDIO_SESSION_NOT_FINISHED = -10,
    PSTVNC_AUDIO_SESSION_TRANSPORT_UNAVAILABLE = -11
} pstvnc_audio_session_result_t;

typedef enum pstvnc_audio_session_outcome_kind {
    PSTVNC_AUDIO_SESSION_OUTCOME_NONE = 0,
    PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY = 1,
    PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED = 2,
    PSTVNC_AUDIO_SESSION_OUTCOME_TRANSPORT = 3,
    PSTVNC_AUDIO_SESSION_OUTCOME_CLOCK = 4,
    PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK = 5,
    PSTVNC_AUDIO_SESSION_OUTCOME_RESERVOIR_DELAY_FAILED = 6,
    PSTVNC_AUDIO_SESSION_OUTCOME_SYNC_FAILED = 7
} pstvnc_audio_session_outcome_kind_t;

typedef struct pstvnc_audio_session_outcome {
    pstvnc_audio_session_outcome_kind_t kind;
    pstvnc_transport_result_t transport_result;
    pstvnc_media_clock_result_t clock_result;
    pstvnc_audio_playback_result_t playback_result;
    pstvnc_audio_playback_report_t playback_report;
    size_t reservoir_bytes;
    int producer_done;
} pstvnc_audio_session_outcome_t;

/*
 * Thread join is the completion-visibility fence for worker-written outcome
 * state. The injected join() contract must return 0 only after the worker has
 * finished and its writes are visible to the owner thread.
 */
typedef struct pstvnc_audio_session {
    pstvnc_transport_access_t transport_access;
    pstvnc_audio_session_values_t values;
    pstvnc_config_pcm_profile_t pcm_profile;
    pstvnc_audio_service_ops_t service;
    const pstvnc_media_clock_t *clock;
    pstvnc_media_clock_time_ops_t time_ops;
    pstvnc_audio_session_memory_ops_t memory_ops;
    pstvnc_audio_session_thread_ops_t thread_ops;
    pstvnc_audio_session_sync_t sync;

    uint8_t *playback_buffer;
    void *worker_stack;
    int thread_id;

    int initialized;
    int thread_created;
    int thread_started;
    int thread_joined;
    int thread_destroyed;
    int stop_requested;
    int worker_finished;
    int worker_sync_failed;

    pstvnc_audio_session_outcome_t outcome;
} pstvnc_audio_session_t;

pstvnc_audio_session_result_t pstvnc_audio_session_start(
    pstvnc_audio_session_t *session,
    const pstvnc_audio_session_values_t *values,
    const pstvnc_config_pcm_profile_t *pcm_profile,
    const pstvnc_audio_service_ops_t *service,
    const pstvnc_media_clock_t *clock,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_audio_session_memory_ops_t *memory_ops,
    const pstvnc_audio_session_thread_ops_t *thread_ops,
    const pstvnc_audio_session_sync_t *sync);

/*
 * Cancels session-owned reservoir/common-clock waits. Once the accepted
 * synchronous playback core is active, the later application owner must pair
 * this with its existing Transport abort/stop ordering to wake any Transport
 * wait; the audio session never aborts or closes Transport itself.
 */
pstvnc_audio_session_result_t pstvnc_audio_session_request_stop(
    pstvnc_audio_session_t *session);

pstvnc_audio_session_result_t pstvnc_audio_session_join(
    pstvnc_audio_session_t *session);

pstvnc_audio_session_result_t pstvnc_audio_session_release(
    pstvnc_audio_session_t *session);

pstvnc_audio_session_result_t pstvnc_audio_session_outcome(
    const pstvnc_audio_session_t *session,
    pstvnc_audio_session_outcome_t *outcome);

#endif
