/*
 * File synopsis:
 * Disposable CP2P post-RETIRE-ACK worker-stop-request diagnostic.
 *
 * This derivative reuses the exact qualified CP2P session coordinator and wraps
 * only its service entry point. After the first MPEG generation reaches
 * MPEG_OWNED and remains healthy briefly, it retires the exact Pi generation,
 * waits for the matching RETIRE ACK, then performs exactly one PS2-local action:
 * it sets the MPEG worker's volatile stop_requested flag.
 *
 * Purpose:
 *     distinguish worker cancellation/video-runtime exit from every later local
 *     teardown operation. If RFB + PCM survive and worker->finished becomes true,
 *     then cancellation and decoder/runtime teardown are sound and the fault lies
 *     later in join/delete, graphics clear, queue finalize, or owner stop. If the
 *     system globally freezes immediately after stop_requested, the fault lies
 *     inside the worker cancellation/video-runtime shutdown path.
 *
 * Deliberately NOT performed after stop_requested:
 *     - no worker join polling
 *     - no DeleteThread()
 *     - no graphics clear
 *     - no MPEG queue finalize/discard
 *     - no owner transition to RFB_ONLY
 *     - no recalibration
 *
 * The delay is intentionally reduced to two seconds after MPEG_OWNED so this
 * test reaches cancellation before the separately-proven spontaneous MPEG read
 * stall is likely to occur. No queue sizing, worker priority, decoder policy,
 * pacing, RFB suppression, transport ownership, or PCM behavior is changed.
 */

#define pstvnc_h1_cp2p_session_coordinator_service \
    pstvnc_h1_cp2p_session_coordinator_service_baseline
#include "h1_cp2p_session_coordinator.c"
#undef pstvnc_h1_cp2p_session_coordinator_service

#include "h1_cp2p_mpeg_worker.h"

#include <stdio.h>
#include <string.h>
#include <timer.h>

#define H1_CP2P_STOP_REQUEST_DELAY_US UINT64_C(2000000)
#define H1_CP2P_STOP_REQUEST_MARKER UINT32_C(0xE4000001)
#define H1_CP2P_WORKER_FINISHED_MARKER UINT32_C(0xE4000002)

typedef struct h1_cp2p_stop_request_state {
    uint32_t session_id;
    uint32_t generation;
    uint64_t mpeg_owned_since_us;
    unsigned armed : 1;
    unsigned retire_started : 1;
    unsigned stop_request_sent : 1;
    unsigned worker_finished_observed : 1;
} h1_cp2p_stop_request_state_t;

static h1_cp2p_stop_request_state_t h1_cp2p_stop_request;

static uint64_t h1_cp2p_stop_request_monotonic_us(void)
{
    unsigned int seconds = 0u;
    unsigned int microseconds = 0u;

    TimerBusClock2USec(
        GetTimerSystemTime(),
        &seconds,
        &microseconds);

    return
        (uint64_t)seconds * UINT64_C(1000000) +
        (uint64_t)microseconds;
}

static void h1_cp2p_stop_request_reset_for_session(uint32_t session_id)
{
    memset(&h1_cp2p_stop_request, 0, sizeof(h1_cp2p_stop_request));
    h1_cp2p_stop_request.session_id = session_id;
}

static pstvnc_h1_cp2p_mpeg_worker_t *h1_cp2p_stop_request_worker(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator)
{
    if (coordinator == NULL)
        return NULL;
    return (pstvnc_h1_cp2p_mpeg_worker_t *)coordinator->clear_mpeg_context;
}

static int h1_cp2p_session_service_stop_request(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator)
{
    pstvnc_h1_mpeg_presentation_owner_state_t owner_state;
    pstvnc_h1_cp2p_mpeg_worker_t *worker;
    uint32_t generation;
    uint64_t now_us;
    int retire_poll;

    if (coordinator == NULL || !coordinator->initialized ||
        coordinator->transport == NULL)
        return 0;

    if (h1_cp2p_stop_request.session_id != coordinator->session_id)
        h1_cp2p_stop_request_reset_for_session(coordinator->session_id);

    worker = h1_cp2p_stop_request_worker(coordinator);

    /*
     * After issuing the one stop request, hold forever. Observe only whether the
     * worker itself reaches finished; do not join/delete/clear/finalize anything.
     */
    if (h1_cp2p_stop_request.stop_request_sent) {
        if (worker == NULL || !worker->initialized ||
            worker->generation != h1_cp2p_stop_request.generation)
            return 0;

        if (worker->finished &&
            !h1_cp2p_stop_request.worker_finished_observed) {
            h1_cp2p_stop_request.worker_finished_observed = 1;
            pstvnc_h1_transport_set_diagnostic_word(
                coordinator->transport,
                H1_CP2P_WORKER_FINISHED_MARKER);
            printf(
                "H1_CP2P_STOP_REQUEST=WORKER_FINISHED generation=%u run_result=%d\n",
                (unsigned int)worker->generation,
                (int)worker->run_result);
        }
        return 1;
    }

    owner_state = pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator->mpeg_handoff.owner);

    if (owner_state != PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED)
        return 1;

    generation = coordinator->mpeg_handoff.owner.generation;
    if (generation == 0u)
        return 0;

    now_us = h1_cp2p_stop_request_monotonic_us();

    if (!h1_cp2p_stop_request.armed) {
        h1_cp2p_stop_request.generation = generation;
        h1_cp2p_stop_request.mpeg_owned_since_us = now_us;
        h1_cp2p_stop_request.armed = 1;
        printf(
            "H1_CP2P_STOP_REQUEST=ARM generation=%u delay_us=%u\n",
            (unsigned int)generation,
            (unsigned int)H1_CP2P_STOP_REQUEST_DELAY_US);
        return 1;
    }

    if (generation != h1_cp2p_stop_request.generation)
        return 0;

    if (!h1_cp2p_stop_request.retire_started) {
        if (now_us < h1_cp2p_stop_request.mpeg_owned_since_us) {
            h1_cp2p_stop_request.mpeg_owned_since_us = now_us;
            return 1;
        }

        if (now_us - h1_cp2p_stop_request.mpeg_owned_since_us <
            H1_CP2P_STOP_REQUEST_DELAY_US)
            return 1;

        if (coordinator->pi_retire_pending ||
            !pstvnc_h1_transport_mpeg_retire_begin(
                coordinator->transport, generation))
            return 0;

        coordinator->pi_retire_generation = generation;
        coordinator->pi_retire_pending = 1;
        h1_cp2p_stop_request.retire_started = 1;

        printf(
            "H1_CP2P_STOP_REQUEST=RETIRE_BEGIN generation=%u\n",
            (unsigned int)generation);
        return 1;
    }

    if (!coordinator->pi_retire_pending ||
        coordinator->pi_retire_generation != generation)
        return 0;

    retire_poll = pstvnc_h1_transport_mpeg_retire_poll(
        coordinator->transport,
        generation);
    if (retire_poll < 0)
        return 0;
    if (retire_poll == 0)
        return 1;

    if (worker == NULL || !worker->initialized || !worker->armed ||
        !worker->thread_started || worker->generation != generation)
        return 0;

    /*
     * The sole post-ACK lifecycle action in this experiment.
     * Do not call worker_clear(): that would also join/delete and clear graphics.
     */
    pstvnc_h1_transport_set_diagnostic_word(
        coordinator->transport,
        H1_CP2P_STOP_REQUEST_MARKER);
    worker->stop_requested = 1;
    h1_cp2p_stop_request.stop_request_sent = 1;

    printf(
        "H1_CP2P_STOP_REQUEST=STOP_FLAG_SET generation=%u\n",
        (unsigned int)generation);
    return 1;
}

int pstvnc_h1_cp2p_session_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;

    if (!h1_cp2p_session_service_stop_request(coordinator))
        return 0;

    return pstvnc_h1_cp2p_session_coordinator_service_baseline(
        context,
        session);
}
