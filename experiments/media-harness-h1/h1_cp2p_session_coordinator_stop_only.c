/*
 * File synopsis:
 * Disposable CP2P completed-picture-boundary stop diagnostic.
 *
 * This wrapper keeps the ordinary CP2P coordinator but changes retirement order
 * for one autonomous two-second test. Instead of retiring the Pi producer first
 * and then cancelling the decoder from inside its data callback, it requests a
 * local MPEG stop while the producer remains live. The video runtime observes
 * that request only between completed MPEG_Picture() calls, destroys decoder
 * ownership there, and lets the worker finish. Only after worker completion do
 * we retire the exact Pi generation, finalize its old queue, clear presentation,
 * and return to RFB_ONLY.
 *
 * This is a clean-room behavioral adaptation of the mature control pattern seen
 * in PS2 movie players: stop/abort is observed at a decoder-call boundary, not
 * converted into an asynchronous end-of-input from inside the data callback.
 */

/*
 * Compile the existing coordinator into this translation unit under a private
 * service symbol. Its static post-ACK queue-finalization helper remains the
 * authoritative implementation after the local worker is already quiescent.
 */
#define pstvnc_h1_cp2p_session_coordinator_service \
    pstvnc_h1_cp2p_session_coordinator_service_baseline
#include "h1_cp2p_session_coordinator.c"
#undef pstvnc_h1_cp2p_session_coordinator_service

#include "h1_cp2p_mpeg_worker.h"

#include <stdio.h>
#include <string.h>
#include <timer.h>

#define H1_CP2P_STOP_ONLY_DELAY_US UINT64_C(2000000)
#define H1_CP2P_BOUNDARY_STOP_REQUESTED UINT32_C(0xE5000001)
#define H1_CP2P_BOUNDARY_WORKER_FINISHED UINT32_C(0xE5000002)

typedef struct h1_cp2p_stop_only_state {
    uint32_t session_id;
    uint32_t generation;
    uint64_t mpeg_owned_since_us;
    unsigned armed : 1;
    unsigned local_stop_requested : 1;
    unsigned worker_finished_observed : 1;
    unsigned retire_started : 1;
    unsigned completed : 1;
} h1_cp2p_stop_only_state_t;

static h1_cp2p_stop_only_state_t h1_cp2p_stop_only;

static uint64_t h1_cp2p_stop_only_monotonic_us(void)
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

static void h1_cp2p_stop_only_reset_for_session(uint32_t session_id)
{
    memset(&h1_cp2p_stop_only, 0, sizeof(h1_cp2p_stop_only));
    h1_cp2p_stop_only.session_id = session_id;
}

static int h1_cp2p_session_service_stop_only(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator)
{
    pstvnc_h1_cp2p_mpeg_worker_t *worker;
    pstvnc_h1_mpeg_presentation_owner_state_t owner_state;
    uint32_t generation;
    uint64_t now_us;
    int retire_poll;

    if (coordinator == NULL || !coordinator->initialized ||
        coordinator->transport == NULL ||
        coordinator->clear_mpeg_context == NULL)
        return 0;

    worker = (pstvnc_h1_cp2p_mpeg_worker_t *)coordinator->clear_mpeg_context;

    if (h1_cp2p_stop_only.session_id != coordinator->session_id)
        h1_cp2p_stop_only_reset_for_session(coordinator->session_id);

    if (h1_cp2p_stop_only.completed)
        return 1;

    owner_state = pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator->mpeg_handoff.owner);

    if (owner_state != PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED)
        return 1;

    generation = coordinator->mpeg_handoff.owner.generation;
    if (generation == 0u)
        return 0;

    now_us = h1_cp2p_stop_only_monotonic_us();

    if (!h1_cp2p_stop_only.armed) {
        h1_cp2p_stop_only.generation = generation;
        h1_cp2p_stop_only.mpeg_owned_since_us = now_us;
        h1_cp2p_stop_only.armed = 1;
        printf(
            "H1_CP2P_BOUNDARY_STOP=ARM generation=%u delay_us=%u\n",
            (unsigned int)generation,
            (unsigned int)H1_CP2P_STOP_ONLY_DELAY_US);
        return 1;
    }

    if (generation != h1_cp2p_stop_only.generation)
        return 0;

    if (!h1_cp2p_stop_only.local_stop_requested) {
        if (now_us < h1_cp2p_stop_only.mpeg_owned_since_us) {
            h1_cp2p_stop_only.mpeg_owned_since_us = now_us;
            return 1;
        }

        if (now_us - h1_cp2p_stop_only.mpeg_owned_since_us <
            H1_CP2P_STOP_ONLY_DELAY_US)
            return 1;

        if (!worker->initialized || !worker->armed ||
            !worker->thread_started || worker->generation != generation)
            return 0;

        /*
         * Producer deliberately remains live here. The video runtime ignores
         * this flag from its data callback and observes it only after the active
         * MPEG_Picture() has returned successfully.
         */
        pstvnc_h1_transport_set_diagnostic_word(
            coordinator->transport,
            H1_CP2P_BOUNDARY_STOP_REQUESTED);
        worker->stop_requested = 1;
        h1_cp2p_stop_only.local_stop_requested = 1;

        printf(
            "H1_CP2P_BOUNDARY_STOP=STOP_REQUESTED generation=%u producer=LIVE\n",
            (unsigned int)generation);
        return 1;
    }

    if (!h1_cp2p_stop_only.worker_finished_observed) {
        if (!worker->finished)
            return 1;

        if (worker->run_result < 0 || worker->failure_latched)
            return 0;

        pstvnc_h1_transport_set_diagnostic_word(
            coordinator->transport,
            H1_CP2P_BOUNDARY_WORKER_FINISHED);
        h1_cp2p_stop_only.worker_finished_observed = 1;

        printf(
            "H1_CP2P_BOUNDARY_STOP=WORKER_FINISHED generation=%u run_result=%d\n",
            (unsigned int)generation,
            worker->run_result);
    }

    /*
     * Only now may the Pi producer retire. No decoder callback can observe this
     * retirement because the video runtime and MPEG decoder have already ended.
     */
    if (!h1_cp2p_stop_only.retire_started) {
        if (coordinator->pi_retire_pending ||
            !pstvnc_h1_transport_mpeg_retire_begin(
                coordinator->transport, generation))
            return 0;

        coordinator->pi_retire_generation = generation;
        coordinator->pi_retire_pending = 1;
        h1_cp2p_stop_only.retire_started = 1;

        printf(
            "H1_CP2P_BOUNDARY_STOP=RETIRE_BEGIN generation=%u worker=FINISHED\n",
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

    printf(
        "H1_CP2P_BOUNDARY_STOP=RETIRE_ACK generation=%u\n",
        (unsigned int)generation);

    /*
     * Worker runtime is already finished. This existing helper therefore only
     * performs the ordinary join/DeleteThread, graphics clear, and exact old
     * queue finalization behind the Pi ACK fence.
     */
    if (!h1_cp2p_session_clear_mpeg_after_pi_retire(
            coordinator, generation))
        return 0;

    if (!pstvnc_h1_mpeg_start_handoff_stop(
            &coordinator->mpeg_handoff, generation))
        return 0;

    coordinator->pi_retire_pending = 0;
    coordinator->pi_retire_generation = 0u;
    coordinator->current_start_contract_valid = 0;
    h1_cp2p_stop_only.completed = 1;

    printf(
        "H1_CP2P_BOUNDARY_STOP=RFB_ONLY generation=%u\n",
        (unsigned int)generation);
    return 1;
}

int pstvnc_h1_cp2p_session_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;

    if (!h1_cp2p_session_service_stop_only(coordinator))
        return 0;

    return pstvnc_h1_cp2p_session_coordinator_service_baseline(
        context,
        session);
}
