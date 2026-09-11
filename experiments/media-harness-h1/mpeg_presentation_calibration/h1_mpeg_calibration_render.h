/*
 * File synopsis:
 * Defines the experiment-local presentation description for native MPEG
 * calibration UI. This mirrors the clean UI/rendering ownership pattern:
 * calibration state is consumed here, while PS2/GS drawing remains below a
 * platform adapter.
 */
#ifndef PSTVNC_H1_MPEG_CALIBRATION_RENDER_H
#define PSTVNC_H1_MPEG_CALIBRATION_RENDER_H

#include "mpeg_presentation_calibration.h"

#include <stddef.h>
#include <stdint.h>

typedef enum pstvnc_h1_mpeg_calibration_render_kind {
    PSTVNC_H1_MPEG_CAL_RENDER_EDIT = 1,
    PSTVNC_H1_MPEG_CAL_RENDER_CONTROLS,
    PSTVNC_H1_MPEG_CAL_RENDER_REVIEW
} pstvnc_h1_mpeg_calibration_render_kind_t;

typedef struct pstvnc_h1_mpeg_calibration_render_plan {
    pstvnc_h1_mpeg_calibration_render_kind_t kind;
    pstvnc_mpeg_cal_region_t region;
    pstvnc_mpeg_cal_rect_t suppression_rect;
    const char *title;
    const char *body_line_1;
    const char *body_line_2;
    const char *footer;
} pstvnc_h1_mpeg_calibration_render_plan_t;

/*
 * Translate calibration state into a platform-neutral native-UI plan.
 * Returns 0 when calibration does not own a visible screen.
 */
int pstvnc_h1_mpeg_calibration_prepare_render_plan(
    const pstvnc_mpeg_calibration_t *calibration,
    pstvnc_h1_mpeg_calibration_render_plan_t *plan);

#endif
