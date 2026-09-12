/*
 * File synopsis:
 * Implements the CP2P recalibration boundary as a fresh transaction rather than
 * an in-place mutation of an MPEG generation.
 */
#include "h1_mpeg_recalibration.h"

#include <stddef.h>
#include <string.h>

void pstvnc_h1_mpeg_recalibration_init(
    pstvnc_h1_mpeg_recalibration_t *recalibration)
{
    if (recalibration == NULL)
        return;

    memset(recalibration, 0, sizeof(*recalibration));
}

int pstvnc_h1_mpeg_recalibration_begin(
    pstvnc_h1_mpeg_recalibration_t *recalibration,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    pstvnc_mpeg_calibration_t *calibration,
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *rfb_flow,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context,
    pstvnc_h1_mpeg_recalibration_begin_result_t *result)
{
    pstvnc_h1_mpeg_presentation_owner_state_t owner_state;
    const pstvnc_mpeg_cal_region_t *owner_region;
    pstvnc_mpeg_cal_region_t calibration_seed;
    uint32_t generation;

    if (recalibration == NULL ||
        handoff == NULL ||
        calibration == NULL ||
        rfb_flow == NULL ||
        result == NULL ||
        recalibration->awaiting_restored_full_rfb ||
        rfb_flow->presentation_owner != &handoff->owner ||
        rfb_flow->calibration_runtime == NULL ||
        &rfb_flow->calibration_runtime->foreground.adapter.calibration !=
            calibration)
        return 0;

    owner_state = pstvnc_h1_mpeg_presentation_owner_state(&handoff->owner);

    if (owner_state == PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY) {
        *result = PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW;
        return 1;
    }

    if (owner_state != PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME &&
        owner_state != PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED)
        return 0;

    owner_region = pstvnc_h1_mpeg_presentation_owner_region(&handoff->owner);
    generation = handoff->owner.generation;

    if (owner_region == NULL || generation == 0u || clear_mpeg == NULL)
        return 0;

    /*
     * Copy before retirement clears the owner's region. This is the only state
     * carried forward: immutable calibration settings, not generation history.
     */
    calibration_seed = *owner_region;

    /*
     * Withdraw MPEG pixels/producer state while the old owner still identifies
     * the exact generation being removed. The callback deliberately does not
     * mutate presentation-owner state.
     */
    if (!clear_mpeg(clear_mpeg_context, generation))
        return 0;

    if (owner_state == PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME) {
        if (!pstvnc_h1_mpeg_start_handoff_abort_start(handoff, generation))
            return 0;
    } else if (!pstvnc_h1_mpeg_start_handoff_stop(handoff, generation)) {
        return 0;
    }

    calibration->committed = calibration_seed;
    calibration->has_committed = 1;

    /*
     * Arm after retirement so an already-outstanding pre-retirement FULL cannot
     * satisfy the transition. The flow watch only accepts a FULL request sent
     * while this new watch is armed.
     */
    pstvnc_h1_mpeg_cp2p_rfb_flow_arm_restoration_watch(rfb_flow);
    recalibration->awaiting_restored_full_rfb = 1;
    *result = PSTVNC_H1_MPEG_RECALIBRATION_WAIT_FOR_FULL_RFB;
    return 1;
}

int pstvnc_h1_mpeg_recalibration_take_entry_ready(
    pstvnc_h1_mpeg_recalibration_t *recalibration,
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *rfb_flow)
{
    if (recalibration == NULL || rfb_flow == NULL ||
        !recalibration->awaiting_restored_full_rfb)
        return 0;

    if (!pstvnc_h1_mpeg_cp2p_rfb_flow_take_restoration_complete(rfb_flow))
        return 0;

    recalibration->awaiting_restored_full_rfb = 0;
    return 1;
}
