/*
 * File synopsis:
 * Implements the one-owner CP2P calibration-entry bridge. It delegates the
 * ordinary held-chord policy unchanged until that policy says "activate". At
 * that exact boundary it either lets RFB-only calibration enter immediately or
 * retires the current MPEG generation and waits for one new full RFB restore.
 */
#include "h1_mpeg_recalibration_entry_bridge.h"

#include <stddef.h>
#include <string.h>

static pstvnc_h1_mpeg_recalibration_entry_bridge_t *s_active_bridge;

static int bridge_matches_hold(
    const pstvnc_h1_mpeg_recalibration_entry_bridge_t *bridge,
    const pstvnc_h1_mpeg_calibration_entry_hold_t *hold)
{
    return bridge != NULL && bridge->attached && bridge->hold == hold;
}

void pstvnc_h1_mpeg_recalibration_entry_bridge_init(
    pstvnc_h1_mpeg_recalibration_entry_bridge_t *bridge)
{
    if (bridge == NULL)
        return;

    memset(bridge, 0, sizeof(*bridge));
}

int pstvnc_h1_mpeg_recalibration_entry_bridge_attach(
    pstvnc_h1_mpeg_recalibration_entry_bridge_t *bridge,
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    pstvnc_mpeg_calibration_t *calibration,
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *rfb_flow,
    pstvnc_h1_mpeg_recalibration_t *recalibration,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context)
{
    if (bridge == NULL || hold == NULL || handoff == NULL ||
        calibration == NULL || rfb_flow == NULL || recalibration == NULL ||
        (s_active_bridge != NULL && s_active_bridge != bridge))
        return 0;

    bridge->hold = hold;
    bridge->handoff = handoff;
    bridge->calibration = calibration;
    bridge->rfb_flow = rfb_flow;
    bridge->recalibration = recalibration;
    bridge->clear_mpeg = clear_mpeg;
    bridge->clear_mpeg_context = clear_mpeg_context;
    bridge->attached = 1;
    s_active_bridge = bridge;
    return 1;
}

int pstvnc_h1_mpeg_recalibration_entry_bridge_detach(
    pstvnc_h1_mpeg_recalibration_entry_bridge_t *bridge)
{
    if (bridge == NULL || !bridge->attached || s_active_bridge != bridge)
        return 0;

    s_active_bridge = NULL;
    memset(bridge, 0, sizeof(*bridge));
    return 1;
}

int pstvnc_h1_mpeg_recalibration_entry_bridge_observe(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int entry_chord_down,
    int *consume_controller_state)
{
    if (!bridge_matches_hold(s_active_bridge, hold)) {
        return pstvnc_h1_mpeg_calibration_entry_hold_observe(
            hold,
            now_us,
            entry_available,
            entry_chord_down,
            consume_controller_state);
    }

    if (consume_controller_state == NULL)
        return 0;

    /* The user's already-accepted entry intent owns controller semantics. */
    if (s_active_bridge->recalibration->awaiting_restored_full_rfb) {
        *consume_controller_state = 1;
        return 1;
    }

    return pstvnc_h1_mpeg_calibration_entry_hold_observe(
        hold,
        now_us,
        entry_available,
        entry_chord_down,
        consume_controller_state);
}

int pstvnc_h1_mpeg_recalibration_entry_bridge_poll(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int *activate_calibration)
{
    pstvnc_h1_mpeg_recalibration_begin_result_t begin_result;

    if (!bridge_matches_hold(s_active_bridge, hold)) {
        return pstvnc_h1_mpeg_calibration_entry_hold_poll(
            hold,
            now_us,
            entry_available,
            activate_calibration);
    }

    if (activate_calibration == NULL)
        return 0;

    /*
     * RFB runtime calls application service after completing and presenting an
     * update. Therefore this one-shot can only be consumed after the newly sent
     * restoration FULL has reached the visible desktop boundary.
     */
    if (s_active_bridge->recalibration->awaiting_restored_full_rfb) {
        *activate_calibration =
            pstvnc_h1_mpeg_recalibration_take_entry_ready(
                s_active_bridge->recalibration,
                s_active_bridge->rfb_flow)
            ? 1
            : 0;
        return 1;
    }

    if (!pstvnc_h1_mpeg_calibration_entry_hold_poll(
            hold,
            now_us,
            entry_available,
            activate_calibration))
        return 0;

    if (!*activate_calibration)
        return 1;

    if (!pstvnc_h1_mpeg_recalibration_begin(
            s_active_bridge->recalibration,
            s_active_bridge->handoff,
            s_active_bridge->calibration,
            s_active_bridge->rfb_flow,
            s_active_bridge->clear_mpeg,
            s_active_bridge->clear_mpeg_context,
            &begin_result))
        return 0;

    if (begin_result == PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW)
        return 1;

    if (begin_result != PSTVNC_H1_MPEG_RECALIBRATION_WAIT_FOR_FULL_RFB)
        return 0;

    /* Retired MPEG first; calibration activation will be re-issued after FULL. */
    *activate_calibration = 0;
    return 1;
}
