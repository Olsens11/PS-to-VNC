/*
 * File synopsis:
 * Disposable CP2P retire-ACK hold diagnostic.
 *
 * This derivative reuses the exact qualified CP2P session coordinator and wraps
 * only its service entry point. After the first MPEG generation reaches
 * MPEG_OWNED and remains there for ten seconds, it performs only the Pi-side
 * exact-generation retirement transaction. Once the matching RETIRE ACK is
 * observed on the PS2, it deliberately stops there.
 *
 * Purpose:
 *     split "getting off the bike" at the wire fence. If RFB + PCM remain alive
 *     after the exact Pi generation has retired and the PS2 has observed the ACK,
 *     then the global freeze belongs to the local post-ACK teardown path rather
 *     than producer retirement / RETIRE ACK transport itself.
 *
 * Deliberately NOT performed after ACK:
 *     - no MPEG worker stop request
 *     - no worker join/delete
 *     - no IPU/DMA teardown
 *     - no MPEG_Destroy()
 *     - no graphics clear
 *     - no MPEG queue finalize/discard
 *     - no owner transition to RFB_ONLY
 *     - no recalibration
 *
 * The frozen/last MPEG image may therefore remain visible. That is expected.
 * The decisive observation is whether PCM, RFB outside/behind that presentation,
 * controller/service progress, and PS2 telemetry continue after RETIRE_ACK_HOLD.
 *
 * No queue sizing, worker priority, decoder policy, pacing, RFB suppression,
 * transport ownership, or PCM behavior is changed.
 */

/*
 * Compile the existing coordinator into this translation unit under a private
 * service symbol. All ordinary session behavior remains authoritative.
 */
#define pstvnc_h1_cp2p_session_coordinator_service \
    pstvnc_h1_cp2p_session_coordinator_service_baseline
#include "h1_cp2p_session_coordinator.c"
#undef pstvnc_h1_cp2p_session_coordinator_service

#include <stdio.h>
#include <string.h>
#include <timer.h>

#define H1_CP2P_STOP_ONLY_DELAY_US UINT64_C(10000000)

typedef struct h1_cp2p_stop_only_state {
    uint32_t session_id;
    uint32_t generation;
    uint64_t mpeg_owned_since_us;
    unsigned armed : 1;
    unsigned retire_started : 1;
    unsigned ack_hold_reached : 1;
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

    /*
     * Once the exact ACK has been observed, deliberately hold the session in
     * the pre-local-teardown state forever. Do not consume any local lifecycle
     * primitive. Baseline service still runs so ordinary RFB/input/PCM service
     * can prove whether the PS2 remains healthy.
     */
    if (h1_cp2p_stop_only.ack_hold_reached)
        return 1;

    owner_state = pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator->mpeg_handoff.owner);

    /*
     * Begin the timer only after the first frame has promoted ownership to
     * MPEG_OWNED. Calibration and WAIT_FIRST_FRAME are unchanged.
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
            "H1_CP2P_ACK_HOLD=ARM generation=%u delay_us=%u\n",
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
            "H1_CP2P_ACK_HOLD=RETIRE_BEGIN generation=%u\n",
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

    /*
     * This is the experimental endpoint. Leave pi_retire_pending/generation,
     * worker state, queue epoch, graphics, and owner state untouched. The ACK
     * is proven; nothing local is torn down.
     */
    h1_cp2p_stop_only.ack_hold_reached = 1;

    printf(
        "H1_CP2P_ACK_HOLD=RETIRE_ACK_HOLD generation=%u\n",
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
