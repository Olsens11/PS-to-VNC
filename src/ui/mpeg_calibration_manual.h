/*
 * File synopsis:
 * Defines one UI-owned manual MPEG CALIBRATION region source for A004 P8.
 *
 * This module composes the accepted platform-neutral MPEG CALIBRATION core with
 * already-normalized controller facts and a caller-owned CT16 visualization.
 * Explicit begin remains caller policy. The module returns region data only:
 * it does not poll libpad, own DESKTOP CALIBRATION, suspend input runtime, touch
 * RFB/Transport/MPEG execution, call Platform graphics, persist configuration,
 * or activate an MPEG run.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-MPEG-CALIBRATION-MANUAL-REGION-SOURCE-P8;
 *   docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md.
 */

#ifndef PSTVNC_MPEG_CALIBRATION_MANUAL_H
#define PSTVNC_MPEG_CALIBRATION_MANUAL_H

#include <stddef.h>
#include <stdint.h>

#include "input/controller.h"
#include "mpeg_calibration.h"

#define PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_BLACK  ((uint16_t)0x8000u)
#define PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_ORANGE ((uint16_t)0x8159u)
#define PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_TEAL   ((uint16_t)0xb5e0u)

typedef enum pstvnc_mpeg_calibration_manual_view_kind {
    PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_EDIT = 1,
    PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_REVIEW
} pstvnc_mpeg_calibration_manual_view_kind_t;

typedef struct pstvnc_mpeg_calibration_manual_result {
    unsigned consume_controller_state : 1;
    unsigned accepted : 1;
    unsigned cancelled : 1;
    unsigned release_quarantine_complete : 1;
    pstvnc_mpeg_calibration_region_t accepted_region;
} pstvnc_mpeg_calibration_manual_result_t;

typedef struct pstvnc_mpeg_calibration_manual_plan {
    pstvnc_mpeg_calibration_manual_view_kind_t kind;
    pstvnc_mpeg_calibration_region_t candidate_region;
    pstvnc_mpeg_calibration_geometry_t geometry;
    int32_t canvas_width;
    int32_t canvas_height;
    unsigned review_accept_armed : 1;
    const char *title;
    const char *help_primary;
    const char *help_secondary;
} pstvnc_mpeg_calibration_manual_plan_t;

typedef struct pstvnc_mpeg_calibration_manual_source {
    pstvnc_mpeg_calibration_t calibration;
} pstvnc_mpeg_calibration_manual_source_t;

/* Initialize one manual region source for a caller-owned local canvas. */
int pstvnc_mpeg_calibration_manual_init(
    pstvnc_mpeg_calibration_manual_source_t *source,
    int32_t canvas_width,
    int32_t canvas_height);

/*
 * Explicitly begin one manual calibration foreground transaction.
 *
 * Entry gesture/chord/hold policy and consumption of the triggering sample are
 * deliberately caller-owned. This function does not inspect controller state.
 */
int pstvnc_mpeg_calibration_manual_begin(
    pstvnc_mpeg_calibration_manual_source_t *source);

/*
 * Service one already-normalized physical-controller sample.
 *
 * While the accepted P1 core owns EDIT, REVIEW, or release quarantine, result
 * consume_controller_state is true so the sample is not also routed as ordinary
 * desktop/OSK input. The accepted edge is copied exactly once from P1.
 */
int pstvnc_mpeg_calibration_manual_service_controller(
    pstvnc_mpeg_calibration_manual_source_t *source,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_mpeg_calibration_manual_result_t *result);

/*
 * Describe the current visible EDIT or REVIEW state using only the exact P1
 * candidate and pstvnc_mpeg_calibration_resolve_geometry() result.
 */
int pstvnc_mpeg_calibration_manual_prepare_plan(
    const pstvnc_mpeg_calibration_manual_source_t *source,
    pstvnc_mpeg_calibration_manual_plan_t *plan);

/*
 * Render the supplied plan over one immutable frozen CT16 desktop into a
 * distinct caller-owned CT16 output surface.
 *
 * The raster uses only plan.geometry for base/inner/suppression meanings. Input
 * and output buffers must not overlap. Capacity and rectangle validation fail
 * closed before any write.
 */
int pstvnc_mpeg_calibration_manual_rasterize(
    const pstvnc_mpeg_calibration_manual_plan_t *plan,
    const uint16_t *frozen_desktop,
    size_t frozen_pixel_count,
    uint16_t *surface_pixels,
    size_t surface_capacity_pixels);

#endif /* PSTVNC_MPEG_CALIBRATION_MANUAL_H */
