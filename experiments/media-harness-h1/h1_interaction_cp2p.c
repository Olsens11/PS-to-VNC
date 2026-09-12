/*
 * File synopsis:
 * Implements CP2P's narrow interaction composition around the existing H1
 * coordinator. RFB-only behavior remains delegated; CP2P owns only presentation
 * generation state, the composed RFB policy, and recalibration entry wiring.
 */
#include "h1_interaction_cp2p.h"

#include <stddef.h>
#include <string.h>

int pstvnc_h1_interaction_cp2p_init(
    pstvnc_h1_interaction_cp2p_t *cp2p,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context)
{
    pstvnc_mpeg_calibration_t *calibration;

    if (cp2p == NULL)
        return 0;

    memset(cp2p, 0, sizeof(*cp2p));
    pstvnc_h1_interaction_coordinator_init(&cp2p->interaction);
    pstvnc_h1_mpeg_start_handoff_init(
        &cp2p->mpeg_handoff,
        PSTVNC_DISPLAY_WIDTH,
        PSTVNC_DISPLAY_HEIGHT);
    pstvnc_h1_mpeg_recalibration_init(&cp2p->recalibration);
    pstvnc_h1_mpeg_recalibration_entry_bridge_init(&cp2p->entry_bridge);

    pstvnc_h1_mpeg_cp2p_rfb_flow_init(
        &cp2p->rfb_flow,
        &cp2p->interaction.mpeg_calibration.runtime,
        &cp2p->mpeg_handoff.owner);

    if (!pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(
            &cp2p->rfb_flow,
            &cp2p->rfb_policy))
        return 0;

    calibration =
        &cp2p->interaction.mpeg_calibration.runtime.foreground.adapter.calibration;

    if (!pstvnc_h1_mpeg_recalibration_entry_bridge_attach(
            &cp2p->entry_bridge,
            &cp2p->interaction.mpeg_calibration_entry_hold,
            &cp2p->mpeg_handoff,
            calibration,
            &cp2p->rfb_flow,
            &cp2p->recalibration,
            clear_mpeg,
            clear_mpeg_context))
        return 0;

    cp2p->initialized = 1;
    return 1;
}

int pstvnc_h1_interaction_cp2p_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer)
{
    pstvnc_h1_interaction_cp2p_t *cp2p =
        (pstvnc_h1_interaction_cp2p_t *)context;

    if (cp2p == NULL || !cp2p->initialized)
        return 0;

    /*
     * In the CP2P target the public PS2 graphics owner is the shared compositor,
     * so the unchanged calibration presenter updates cached RFB while MPEG stays
     * visually protected by its suppression/matte footprint.
     */
    return pstvnc_h1_interaction_coordinator_present(
        &cp2p->interaction,
        framebuffer);
}

int pstvnc_h1_interaction_cp2p_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_interaction_cp2p_t *cp2p =
        (pstvnc_h1_interaction_cp2p_t *)context;

    if (cp2p == NULL || !cp2p->initialized)
        return 0;

    /*
     * The generated CP2P coordinator redirects its entry-hold poll through the
     * attached bridge. RFB runtime calls this service only after any completed
     * update has had its presentation opportunity, so a restoration completion
     * can safely re-issue calibration activation here.
     */
    return pstvnc_h1_interaction_coordinator_service(
        &cp2p->interaction,
        session);
}

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_interaction_cp2p_rfb_policy(
    const pstvnc_h1_interaction_cp2p_t *cp2p)
{
    if (cp2p == NULL || !cp2p->initialized)
        return NULL;

    return &cp2p->rfb_policy;
}

pstvnc_h1_mpeg_start_handoff_t *
pstvnc_h1_interaction_cp2p_mpeg_handoff(
    pstvnc_h1_interaction_cp2p_t *cp2p)
{
    if (cp2p == NULL || !cp2p->initialized)
        return NULL;

    return &cp2p->mpeg_handoff;
}

int pstvnc_h1_interaction_cp2p_shutdown(
    pstvnc_h1_interaction_cp2p_t *cp2p)
{
    int result;

    if (cp2p == NULL || !cp2p->initialized)
        return -1;

    result = pstvnc_h1_interaction_coordinator_shutdown(&cp2p->interaction);
    if (!pstvnc_h1_mpeg_recalibration_entry_bridge_detach(
            &cp2p->entry_bridge))
        result = -1;

    cp2p->initialized = 0;
    return result;
}
