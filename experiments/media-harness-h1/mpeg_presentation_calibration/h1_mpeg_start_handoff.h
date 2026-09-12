/*
 * File synopsis:
 * Defines the explicit CP2P-only handoff from an already-committed MPEG
 * calibration region to one generation-guarded MPEG presentation start.
 *
 * This module deliberately does not observe controller/calibration acceptance
 * itself. CP2O may therefore keep its existing accept -> release -> RFB-thaw
 * behavior unchanged. A later CP2P coordinator elects to call prepare_start()
 * only when MPEG is actually available for the accepted region.
 */
#ifndef PSTVNC_H1_MPEG_START_HANDOFF_H
#define PSTVNC_H1_MPEG_START_HANDOFF_H

#include "h1_mpeg_presentation_owner.h"

#include <stdint.h>

typedef struct pstvnc_h1_mpeg_start_contract {
    /* Exact base MPEG capture/presentation rectangle. Mattes do not alter it. */
    int draw_x;
    int draw_y;
    int draw_width;
    int draw_height;

    /* Symmetric black matte owned by MPEG presentation inside the base region. */
    int inner_matte_x;
    int inner_matte_y;

    /* Outer-expanded visual footprint from which RFB must be suppressed. */
    pstvnc_mpeg_cal_rect_t suppression_rect;

    /* Identifies this exact start attempt; stale decoder callbacks must reject. */
    uint32_t generation;
} pstvnc_h1_mpeg_start_contract_t;

typedef struct pstvnc_h1_mpeg_start_handoff {
    pstvnc_h1_mpeg_presentation_owner_t owner;
    int canvas_width;
    int canvas_height;
} pstvnc_h1_mpeg_start_handoff_t;

void pstvnc_h1_mpeg_start_handoff_init(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    int canvas_width,
    int canvas_height);

/*
 * Prepare one CP2P MPEG start from a committed calibration region.
 *
 * The base rectangle is copied unchanged into the returned decoder/presenter
 * contract. Inner matte remains separate presentation state and outer matte is
 * converted to the clipped RFB-suppression footprint. On success the handoff
 * enters WAIT_FIRST_FRAME and RFB must remain visually frozen.
 */
int pstvnc_h1_mpeg_start_handoff_prepare_start(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    const pstvnc_mpeg_cal_region_t *committed_region,
    pstvnc_h1_mpeg_start_contract_t *contract);

/* Physical presentation-boundary lifecycle events for the exact generation. */
int pstvnc_h1_mpeg_start_handoff_first_frame_presented(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    uint32_t generation);

int pstvnc_h1_mpeg_start_handoff_abort_start(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    uint32_t generation);

int pstvnc_h1_mpeg_start_handoff_stop(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    uint32_t generation);

/* Read-only ownership facts consumed later by the CP2P RFB/compositor wrapper. */
int pstvnc_h1_mpeg_start_handoff_allows_rfb_request(
    const pstvnc_h1_mpeg_start_handoff_t *handoff);

pstvnc_h1_mpeg_remote_present_mode_t
pstvnc_h1_mpeg_start_handoff_remote_present_mode(
    const pstvnc_h1_mpeg_start_handoff_t *handoff);

int pstvnc_h1_mpeg_start_handoff_take_full_refresh(
    pstvnc_h1_mpeg_start_handoff_t *handoff);

const pstvnc_mpeg_cal_rect_t *
pstvnc_h1_mpeg_start_handoff_suppression_rect(
    const pstvnc_h1_mpeg_start_handoff_t *handoff);

#endif /* PSTVNC_H1_MPEG_START_HANDOFF_H */
