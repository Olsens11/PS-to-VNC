/*
 * File synopsis:
 * Defines the Display-owned MPEG visible-ownership state and one immutable
 * resolved presentation-geometry snapshot for the exact caller-owned MPEG run.
 *
 * This module owns only presentation-visible lifecycle facts:
 * RFB_ONLY -> WAIT_FIRST_FRAME -> MPEG_OWNED -> RETIRING -> REVEAL_PENDING
 * -> RFB_ONLY, the resolved base/inner/suppression geometry used by composition,
 * and the exact run generation fence.
 *
 * RETIRING/REVEAL_PENDING describe visual ownership only. They do not claim
 * producer closure, decoder safe-stop, Transport residual isolation, or RFB
 * underlay freshness; those cross-owner proofs remain Application obligations.
 *
 * This module does not edit calibration, mint MPEG generations, start/stop
 * Transport or decoding, draw through gsKit, arm/clear the media clock, own RFB
 * refresh debt, or perform Application orchestration.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3;
 *   docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md;
 *   docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md, Q6/Q7.
 */

#ifndef PSTVNC_MPEG_PRESENTATION_H
#define PSTVNC_MPEG_PRESENTATION_H

#include <stdint.h>

typedef struct pstvnc_mpeg_presentation_rect {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} pstvnc_mpeg_presentation_rect_t;

typedef struct pstvnc_mpeg_presentation_geometry {
    /* Exact MPEG capture/presentation rectangle. */
    pstvnc_mpeg_presentation_rect_t base;

    /* Base content remaining after symmetric presentation-local inner matte. */
    pstvnc_mpeg_presentation_rect_t inner_content;

    /* Outer visual-ownership footprint used by later RFB suppression/composite. */
    pstvnc_mpeg_presentation_rect_t suppression;
} pstvnc_mpeg_presentation_geometry_t;

typedef enum pstvnc_mpeg_presentation_state {
    PSTVNC_MPEG_PRESENTATION_STATE_INVALID = -1,
    PSTVNC_MPEG_PRESENTATION_RFB_ONLY = 0,
    PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME,
    PSTVNC_MPEG_PRESENTATION_MPEG_OWNED,
    PSTVNC_MPEG_PRESENTATION_RETIRING,
    PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING
} pstvnc_mpeg_presentation_state_t;

typedef enum pstvnc_mpeg_presentation_mode {
    PSTVNC_MPEG_PRESENTATION_MODE_INVALID = -1,
    PSTVNC_MPEG_PRESENTATION_DIRECT_RFB = 0,
    PSTVNC_MPEG_PRESENTATION_FROZEN_RFB_DESKTOP,
    PSTVNC_MPEG_PRESENTATION_COMPOSITED
} pstvnc_mpeg_presentation_mode_t;

typedef struct pstvnc_mpeg_presentation {
    pstvnc_mpeg_presentation_state_t state;
    pstvnc_mpeg_presentation_geometry_t geometry;
    uint32_t run_generation;
    unsigned snapshot_valid : 1;
} pstvnc_mpeg_presentation_t;

/* Initialize ordinary RFB-only presentation with no live MPEG run snapshot. */
void pstvnc_mpeg_presentation_init(
    pstvnc_mpeg_presentation_t *presentation);

/*
 * Arm one caller-owned exact MPEG run from an already-resolved geometry value.
 *
 * run_generation must be nonzero and is copied verbatim; Presentation never
 * allocates or increments run identity. Arm is legal only from RFB_ONLY.
 * Successful arm snapshots geometry/generation and enters WAIT_FIRST_FRAME.
 */
int pstvnc_mpeg_presentation_arm(
    pstvnc_mpeg_presentation_t *presentation,
    const pstvnc_mpeg_presentation_geometry_t *geometry,
    uint32_t run_generation);

/*
 * Promote the exact pending run only at the real first-physical-frame boundary.
 *
 * This state transition itself performs no draw, flip, upload, decode or
 * media-clock arm. A later physical presentation mechanism must invoke it only
 * after the exact run's first MPEG frame has actually crossed the synchronized
 * presentation boundary.
 */
int pstvnc_mpeg_presentation_first_frame_presented(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation);

/*
 * Abort only an exact WAIT_FIRST_FRAME start and return to RFB_ONLY.
 *
 * This owner does not create or consume RFB FULL-refresh debt. Later Application
 * orchestration coordinates P2 thaw/recovery separately.
 */
int pstvnc_mpeg_presentation_abort_pending(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation);

/*
 * Begin visible retirement for the exact currently MPEG-owned run.
 *
 * This transition records no external lifecycle proof. The caller is
 * responsible for closing new production/admission and coordinating RFB
 * restoration. Successful begin preserves the immutable run snapshot and
 * leaves MPEG/mattes visibly composited so already accepted frames may drain.
 */
int pstvnc_mpeg_presentation_begin_retirement(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation);

/*
 * Seal the exact retiring run after the caller has reached its external
 * safe-stop/restoration boundary. REVEAL_PENDING keeps the retained MPEG
 * composite visible but prevents further frame presentation.
 */
int pstvnc_mpeg_presentation_seal_retirement(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation);

/*
 * Commit logical RFB_ONLY only after the exact REVEAL_PENDING run has crossed
 * the synchronized physical no-MPEG reveal boundary.
 */
int pstvnc_mpeg_presentation_commit_reveal(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation);

pstvnc_mpeg_presentation_state_t
pstvnc_mpeg_presentation_state(
    const pstvnc_mpeg_presentation_t *presentation);

pstvnc_mpeg_presentation_mode_t
pstvnc_mpeg_presentation_mode(
    const pstvnc_mpeg_presentation_t *presentation);

/* WAIT_FIRST_FRAME alone requires generic global RFB ownership-gap protection. */
int pstvnc_mpeg_presentation_requires_global_rfb_protection(
    const pstvnc_mpeg_presentation_t *presentation);

/* True while MPEG/mattes remain the visible owner, including retirement. */
int pstvnc_mpeg_presentation_owns_mpeg_visual(
    const pstvnc_mpeg_presentation_t *presentation);

/*
 * Copy the immutable current run snapshot for read-only downstream use.
 * Returns 0 in RFB_ONLY or for invalid arguments.
 */
int pstvnc_mpeg_presentation_snapshot(
    const pstvnc_mpeg_presentation_t *presentation,
    pstvnc_mpeg_presentation_geometry_t *geometry,
    uint32_t *run_generation);

#endif /* PSTVNC_MPEG_PRESENTATION_H */
