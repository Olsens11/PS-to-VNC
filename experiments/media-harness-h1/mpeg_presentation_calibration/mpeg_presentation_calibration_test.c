#include <assert.h>
#include <stdio.h>
#include "mpeg_presentation_calibration.h"
static pstvnc_mpeg_cal_input_t s(uint32_t d,uint32_t p){pstvnc_mpeg_cal_input_t x={d,p,0};return x;}
int main(void){
    pstvnc_mpeg_calibration_t c; pstvnc_mpeg_cal_input_t i; pstvnc_mpeg_cal_effects_t e; pstvnc_mpeg_cal_rect_t r;
    pstvnc_mpeg_calibration_init(&c,704,462);
    assert(c.current.x==176&&c.current.y==119&&c.current.width==352&&c.current.height==224);
    i=s(PSTVNC_MPEG_CAL_START|PSTVNC_MPEG_CAL_SELECT,0);e=pstvnc_mpeg_calibration_update(&c,&i);assert(e.enter_foreground);
    i=s(PSTVNC_MPEG_CAL_RIGHT,0);pstvnc_mpeg_calibration_update(&c,&i);assert(c.current.width==368&&c.current.x==168);
    i=s(PSTVNC_MPEG_CAL_R1|PSTVNC_MPEG_CAL_RIGHT|PSTVNC_MPEG_CAL_DOWN,0);pstvnc_mpeg_calibration_update(&c,&i);assert(c.current.x==169&&c.current.y==120);
    i=s(PSTVNC_MPEG_CAL_R2|PSTVNC_MPEG_CAL_RIGHT,0);pstvnc_mpeg_calibration_update(&c,&i);assert(c.current.inner_matte_x==1&&c.current.inner_matte_y==1);
    i=s(PSTVNC_MPEG_CAL_L2|PSTVNC_MPEG_CAL_LEFT,0);pstvnc_mpeg_calibration_update(&c,&i);assert(c.current.outer_matte_x==1&&c.current.outer_matte_y==1);
    r=pstvnc_mpeg_calibration_rfb_suppression_rect(&c,&c.current);assert(r.x==168&&r.y==119&&r.width==370&&r.height==226);
    i=s(PSTVNC_MPEG_CAL_CROSS,PSTVNC_MPEG_CAL_CROSS);pstvnc_mpeg_calibration_update(&c,&i);assert(c.screen==PSTVNC_MPEG_CAL_REVIEW);
    i=s(0,0);pstvnc_mpeg_calibration_update(&c,&i);assert(c.review_cross_armed);
    i=s(PSTVNC_MPEG_CAL_CROSS,PSTVNC_MPEG_CAL_CROSS);e=pstvnc_mpeg_calibration_update(&c,&i);assert(e.accepted&&e.leave_foreground&&pstvnc_mpeg_calibration_owns_foreground(&c));
    i=s(0,0);e=pstvnc_mpeg_calibration_update(&c,&i);assert(e.release_quarantine_complete&&!pstvnc_mpeg_calibration_owns_foreground(&c));
    puts("MPEG_CALIBRATION_HOST_TEST=PASS"); return 0;
}
