/*
 * File synopsis:
 * Bridges the existing 750 ms START+SELECT entry policy to CP2P's clean
 * MPEG-retirement/recalibration lifecycle without changing the qualified
 * interaction coordinator source.
 *
 * The CP2P coordinator translation unit mechanically redirects only the entry
 * hold observe/poll calls here. Ordinary RFB-only entry remains identical. When
 * MPEG exists, a matured entry chord retires that generation and defers the
 * synthetic calibration activation until a new post-retirement full RFB update
 * has completed. Controller observations are consumed during that short defer so
 * START/SELECT cannot leak into desktop/OSK semantics.
 */
#ifndef PSTVNC_H1_MPEG_RECALIBRATION_ENTRY_BRIDGE_H
#define PSTVNC_H1_MPEG_RECALIBRATION_ENTRY_BRIDGE_H

#include "h1_mpeg_calibration_entry_hold.h"
#include "h1_mpeg_recalibration.h"

typedef struct pstvnc_h1_mpeg_recalibration_entry_bridge {
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold;
    pstvnc_h1_mpeg_start_handoff_t *handoff;
    pstvnc_mpeg_calibration_t *calibration;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *rfb_flow;
    pstvnc_h1_mpeg_recalibration_t *recalibration;
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg;
    void *clear_mpeg_context;
    unsigned attached : 1;
} pstvnc_h1_mpeg_recalibration_entry_bridge_t;

void pstvnc_h1_mpeg_recalibration_entry_bridge_init(
    pstvnc_h1_mpeg_recalibration_entry_bridge_t *bridge);

/*
 * Attach the one live H1 CP2P interaction instance. H1 has one foreground RFB
 * coordinator, so a one-owner bridge keeps this seam bounded instead of adding
 * a registry or generation-history framework.
 */
int pstvnc_h1_mpeg_recalibration_entry_bridge_attach(
    pstvnc_h1_mpeg_recalibration_entry_bridge_t *bridge,
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    pstvnc_mpeg_calibration_t *calibration,
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *rfb_flow,
    pstvnc_h1_mpeg_recalibration_t *recalibration,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context);

int pstvnc_h1_mpeg_recalibration_entry_bridge_detach(
    pstvnc_h1_mpeg_recalibration_entry_bridge_t *bridge);

/* Drop-in CP2P replacements for the two entry-hold calls in the coordinator. */
int pstvnc_h1_mpeg_recalibration_entry_bridge_observe(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int entry_chord_down,
    int *consume_controller_state);

int pstvnc_h1_mpeg_recalibration_entry_bridge_poll(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int *activate_calibration);

#endif /* PSTVNC_H1_MPEG_RECALIBRATION_ENTRY_BRIDGE_H */
