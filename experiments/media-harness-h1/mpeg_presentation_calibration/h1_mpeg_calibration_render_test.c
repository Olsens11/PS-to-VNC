#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "h1_mpeg_calibration_render.h"

int main(void)
{
    pstvnc_mpeg_calibration_t calibration;
    pstvnc_h1_mpeg_calibration_render_plan_t plan;

    pstvnc_mpeg_calibration_init(&calibration, 704, 462);

    assert(!pstvnc_h1_mpeg_calibration_prepare_render_plan(
        &calibration, &plan));

    calibration.screen = PSTVNC_MPEG_CAL_EDIT;
    calibration.current.outer_matte_x = 3;
    calibration.current.outer_matte_y = 5;
    assert(pstvnc_h1_mpeg_calibration_prepare_render_plan(
        &calibration, &plan));
    assert(plan.kind == PSTVNC_H1_MPEG_CAL_RENDER_EDIT);
    assert(plan.region.outer_matte_x == 3);
    assert(plan.region.outer_matte_y == 5);
    assert(plan.suppression_rect.width == calibration.current.width + 6);
    assert(plan.suppression_rect.height == calibration.current.height + 10);
    assert(strstr(plan.body_line_1, "R1 move") != NULL);

    calibration.screen = PSTVNC_MPEG_CAL_CONTROLS;
    assert(pstvnc_h1_mpeg_calibration_prepare_render_plan(
        &calibration, &plan));
    assert(plan.kind == PSTVNC_H1_MPEG_CAL_RENDER_CONTROLS);
    assert(strstr(plan.body_line_1, "R2:") != NULL);
    assert(strstr(plan.body_line_2, "L2:") != NULL);

    calibration.screen = PSTVNC_MPEG_CAL_REVIEW;
    assert(pstvnc_h1_mpeg_calibration_prepare_render_plan(
        &calibration, &plan));
    assert(plan.kind == PSTVNC_H1_MPEG_CAL_RENDER_REVIEW);
    assert(strstr(plan.body_line_1, "RFB suppression perimeter") != NULL);
    assert(strstr(plan.body_line_2, "press X again") != NULL);

    puts("MPEG_CALIBRATION_RENDER_HOST_TEST=PASS");
    return 0;
}
