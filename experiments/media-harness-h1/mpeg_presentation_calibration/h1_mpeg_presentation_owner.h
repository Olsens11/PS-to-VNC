/*
 * File synopsis:
 * Defines the experiment-local ownership contract that bridges an accepted
 * MPEG presentation calibration to first-frame activation and eventual return
 * to RFB-only presentation.
 *
 * This module owns lifecycle facts only. It does not decode MPEG, draw GS
 * primitives, parse/present RFB, edit calibration geometry, or synchronize
 * threads. The caller serializes access and performs the physical presentation
 * transition at the appropriate compositor boundary.
 */
#ifndef PSTVNC_H1_MPEG_PRESENTATION_OWNER_H
#define PSTVNC_H1_MPEG_PRESENTATION_OWNER_H

#include "mpeg_presentation_calibration.h"

#include <stdint.h>

typedef enum pstvnc_h1_mpeg_presentation_owner_state {
    PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY = 0,
    PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME,
    PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED
} pstvnc_h1_mpeg_presentation_owner_state_t;

typedef enum pstvnc_h1_mpeg_remote_present_mode {
    /* Ordinary complete RFB desktop may present directly. */
    PSTVNC_H1_MPEG_REMOTE_PRESENT_DIRECT_RFB = 0,

    /* Keep the last already-presented RFB desktop visible and unchanged. */
    PSTVNC_H1_MPEG_REMOTE_PRESENT_FROZEN_DESKTOP,

    /* RFB may advance, but visible presentation must pass through compositor. */
    PSTVNC_H1_MPEG_REMOTE_PRESENT_COMPOSITED
} pstvnc_h1_mpeg_remote_present_mode_t;

typedef struct pstvnc_h1_mpeg_presentation_owner {
    pstvnc_h1_mpeg_presentation_owner_state_t state;
    pstvnc_mpeg_cal_region_t region;
    pstvnc_mpeg_cal_rect_t suppression_rect;
    uint32_t generation;
    unsigned has_region : 1;
    unsigned full_refresh_pending : 1;
} pstvnc_h1_mpeg_presentation_owner_t;

void pstvnc_h1_mpeg_presentation_owner_init(
    pstvnc_h1_mpeg_presentation_owner_t *owner);

/*
 * Arm one accepted calibration for MPEG startup.
 *
 * The suppression rectangle must contain the complete base MPEG region. On
 * success the returned nonzero generation identifies this exact start attempt.
 * RFB requests/presentation remain frozen until that generation supplies the
 * first successfully presentable MPEG frame.
 */
int pstvnc_h1_mpeg_presentation_owner_arm(
    pstvnc_h1_mpeg_presentation_owner_t *owner,
    const pstvnc_mpeg_cal_region_t *region,
    const pstvnc_mpeg_cal_rect_t *suppression_rect,
    uint32_t *generation);

/*
 * Promote WAIT_FIRST_FRAME -> MPEG_OWNED for the exact current generation.
 * The caller must invoke this only at the physical first-frame presentation
 * boundary, so visible ownership and this state transition are atomic from the
 * user's perspective.
 */
int pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
    pstvnc_h1_mpeg_presentation_owner_t *owner,
    uint32_t generation);

/* Abort a start that has not yet produced its first visible frame. */
int pstvnc_h1_mpeg_presentation_owner_abort_start(
    pstvnc_h1_mpeg_presentation_owner_t *owner,
    uint32_t generation);

/* Retire an active MPEG owner and restore full RFB presentation. */
int pstvnc_h1_mpeg_presentation_owner_stop(
    pstvnc_h1_mpeg_presentation_owner_t *owner,
    uint32_t generation);

pstvnc_h1_mpeg_presentation_owner_state_t
pstvnc_h1_mpeg_presentation_owner_state(
    const pstvnc_h1_mpeg_presentation_owner_t *owner);

int pstvnc_h1_mpeg_presentation_owner_allows_rfb_request(
    const pstvnc_h1_mpeg_presentation_owner_t *owner);

pstvnc_h1_mpeg_remote_present_mode_t
pstvnc_h1_mpeg_presentation_owner_remote_present_mode(
    const pstvnc_h1_mpeg_presentation_owner_t *owner);

int pstvnc_h1_mpeg_presentation_owner_has_mpeg(
    const pstvnc_h1_mpeg_presentation_owner_t *owner);

/* One-shot request used when thawing/resuming RFB after a frozen interval. */
int pstvnc_h1_mpeg_presentation_owner_take_full_refresh(
    pstvnc_h1_mpeg_presentation_owner_t *owner);

const pstvnc_mpeg_cal_region_t *
pstvnc_h1_mpeg_presentation_owner_region(
    const pstvnc_h1_mpeg_presentation_owner_t *owner);

const pstvnc_mpeg_cal_rect_t *
pstvnc_h1_mpeg_presentation_owner_suppression_rect(
    const pstvnc_h1_mpeg_presentation_owner_t *owner);

#endif /* PSTVNC_H1_MPEG_PRESENTATION_OWNER_H */
