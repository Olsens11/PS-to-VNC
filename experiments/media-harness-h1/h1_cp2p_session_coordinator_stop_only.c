/*
 * File synopsis:
 * Disposable CP2P stop-only lifecycle diagnostic.
 *
 * This derivative reuses the exact qualified CP2P session coordinator and wraps
 * only its service entry point. After the first MPEG generation reaches
 * MPEG_OWNED and remains there for two seconds, it performs one autonomous
 * retirement transaction and returns presentation ownership to RFB_ONLY.
 *
 * Purpose:
 *     prove whether an otherwise-healthy RFB + PCM + MPEG session can tear MPEG
 *     down completely and return to the original RFB + PCM state without
 *     entering recalibration. No second calibration is started automatically.
 *
 * The stop sequence deliberately reuses the existing production operations:
 *     retire exact Pi generation -> wait for ACK -> clear/join local MPEG worker
 *     and pixels -> finalize old MPEG queue -> owner stop -> one full RFB refresh.
 *
 * The two-second delay is diagnostic only. It avoids the separately-proven
 * spontaneous active MPEG read stall so teardown is exercised while the MPEG
 * worker is still healthy. No queue sizing, worker priority, decoder policy,
 * pacing, RFB suppression, transport ownership, or PCM behavior is changed.
 */

/*
 * Compile the existing coordinator into this translation unit under a private
 * service symbol. All of its static lifecycle helpers remain the authoritative
 * implementation used by the stop-only wrapper below.
 */
#define pstvnc_h1_cp2p_session_coordinator_service \
    pstvnc_h1_cp2p_session_coordinator_service_baseline
#include "h1_cp2p_session_coordinator.c"
#undef pstvnc_h1_cp2p_session_coordinator_service

#include <stdio.h>
#include <string.h>
#include <timer.h>

#define H1_CP2P_STOP_ONLY_DELAY_US UINT64_C(2000000)

typedef struct h1_cp2p_stop_only_state {
    uint32_t session_id;
    uint32_t generation;
    uint64_t mpeg_owned_since_us;
    unsigned armed : 1;
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
    pstvnc_h1_mpeg_presentation_owner_state_t owner_state;
    uint32_t generation;
    uint64_t now_us;
    int retire_poll;

    if (coordinator == NULL || !coordinator->initialized ||
        coordinator->transport == NULL)
        return 0;

    if (h1_cp2p_stop_only.session_id != coordinator->session_id)
        h1_cp2p_stop_only_reset_for_session(coordinator->session_id);

    if (h1_cp2p_stop_only.completed)
        return 1;

    owner_state = pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator->mpeg_handoff.owner);

    /*
     * We intentionally begin the clock only after the first frame has promoted
     * ownership to MPEG_OWNED. Calibration and WAIT_FIRST_FRAME are unchanged.
     */
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
            "H1_CP2P_STOP_ONLY=ARM generation=%u delay_us=%u\n",
            (unsigned int)generation,
            (unsigned int)H1_CP2P_STOP_ONLY_DELAY_US);
        return 1;
    }

    if (generation != h1_cp2p_stop_only.generation)
        return 0;

    if (!h1_cp2p_stop_only.retire_started) {
        if (now_us < h1_cp2p_stop_only.mpeg_owned_since_us) {
            h1_cp2p_stop_only.mpeg_owned_since_us = now_us;
            return 1;
        }

        if (now_us - h1_cp2p_stop_only.mpeg_owned_since_us <
            H1_CP2P_STOP_ONLY_DELAY_US)
            return 1;

        if (coordinator->pi_retire_pending ||
            !pstvnc_h1_transport_mpeg_retire_begin(
                coordinator->transport, generation))
            return 0;

        coordinator->pi_retire_generation = generation;
        coordinator->pi_retire_pending = 1;
        h1_cp2p_stop_only.retire_started = 1;

        printf(
            "H1_CP2P_STOP_ONLY=RETIRE_BEGIN generation=%u\n",
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
        "H1_CP2P_STOP_ONLY=RETIRE_ACK generation=%u\n",
        (unsigned int)generation);

    /*
     * Reuse the exact existing post-ACK teardown helper. It stops/joins the MPEG
     * worker, clears MPEG pixels, and finalizes/discards the old queue epoch.
     * It deliberately does not mutate presentation-owner state.
     */
    if (!h1_cp2p_session_clear_mpeg_after_pi_retire(
            coordinator, generation))
        return 0;

    /*
     * This is the entire point of the experiment: return to square one and stop.
     * owner_stop() changes the owner to RFB_ONLY and arms its ordinary one-shot
     * full refresh. No recalibration seed/watch/foreground transition is used.
     */
    if (!pstvnc_h1_mpeg_start_handoff_stop(
            &coordinator->mpeg_handoff, generation))
        return 0;

    coordinator->pi_retire_pending = 0;
    coordinator->pi_retire_generation = 0u;
    coordinator->current_start_contract_valid = 0;
    h1_cp2p_stop_only.completed = 1;

    printf(
        "H1_CP2P_STOP_ONLY=RFB_ONLY generation=%u\n",
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
