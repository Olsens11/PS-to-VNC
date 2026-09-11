#include "h1_mpeg_calibration_render.h"

#include <string.h>

int pstvnc_h1_mpeg_calibration_prepare_render_plan(
    const pstvnc_mpeg_calibration_t *calibration,
    pstvnc_h1_mpeg_calibration_render_plan_t *plan)
{
    if (calibration == NULL || plan == NULL ||
        calibration->screen == PSTVNC_MPEG_CAL_INACTIVE)
        return 0;

    memset(plan, 0, sizeof(*plan));
    plan->region = calibration->current;
    plan->suppression_rect =
        pstvnc_mpeg_calibration_rfb_suppression_rect(
            calibration,
            &calibration->current);

    switch (calibration->screen) {
        case PSTVNC_MPEG_CAL_EDIT:
            plan->kind = PSTVNC_H1_MPEG_CAL_RENDER_EDIT;
            plan->title = "MPEG Presentation Calibration";
            plan->body_line_1 = "D-pad resize | R1 move | R2 inner | L2 outer";
            plan->body_line_2 = "START reset | TRIANGLE controls | X review | O cancel";
            plan->footer = "Hold START+SELECT to enter calibration";
            return 1;

        case PSTVNC_MPEG_CAL_CONTROLS:
            plan->kind = PSTVNC_H1_MPEG_CAL_RENDER_CONTROLS;
            plan->title = "Calibration Controls";
            plan->body_line_1 = "R2: Right +all, Left -all, Down +vertical, Up -vertical";
            plan->body_line_2 = "L2: Left +all, Right -all, Up +vertical, Down -vertical";
            plan->footer = "TRIANGLE/O back | X review";
            return 1;

        case PSTVNC_MPEG_CAL_REVIEW:
            plan->kind = PSTVNC_H1_MPEG_CAL_RENDER_REVIEW;
            plan->title = "Review MPEG Region";
            plan->body_line_1 = "Outer matte edge defines the RFB suppression perimeter";
            plan->body_line_2 = "Release X, then press X again to accept";
            plan->footer = "O back";
            return 1;

        default:
            return 0;
    }
}
