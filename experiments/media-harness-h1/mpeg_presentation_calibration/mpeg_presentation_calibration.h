#ifndef PSTVNC_MPEG_PRESENTATION_CALIBRATION_H
#define PSTVNC_MPEG_PRESENTATION_CALIBRATION_H
#include <stdint.h>
typedef struct { int x,y,width,height; } pstvnc_mpeg_cal_rect_t;
typedef struct {
    int x,y,width,height;
    int inner_matte_x, inner_matte_y;
    int outer_matte_x, outer_matte_y;
} pstvnc_mpeg_cal_region_t;
typedef enum {
    PSTVNC_MPEG_CAL_INACTIVE=0,
    PSTVNC_MPEG_CAL_EDIT,
    PSTVNC_MPEG_CAL_CONTROLS,
    PSTVNC_MPEG_CAL_REVIEW
} pstvnc_mpeg_cal_screen_t;
enum {
    PSTVNC_MPEG_CAL_UP=1u<<0, PSTVNC_MPEG_CAL_RIGHT=1u<<1,
    PSTVNC_MPEG_CAL_DOWN=1u<<2, PSTVNC_MPEG_CAL_LEFT=1u<<3,
    PSTVNC_MPEG_CAL_L2=1u<<4, PSTVNC_MPEG_CAL_R2=1u<<5,
    PSTVNC_MPEG_CAL_R1=1u<<6, PSTVNC_MPEG_CAL_START=1u<<7,
    PSTVNC_MPEG_CAL_SELECT=1u<<8, PSTVNC_MPEG_CAL_TRIANGLE=1u<<9,
    PSTVNC_MPEG_CAL_CIRCLE=1u<<10, PSTVNC_MPEG_CAL_CROSS=1u<<11
};
#define PSTVNC_MPEG_CAL_ALL ((1u<<12)-1u)
typedef struct { uint32_t down,pressed,released; } pstvnc_mpeg_cal_input_t;
typedef struct {
    unsigned enter_foreground:1, leave_foreground:1, accepted:1, cancelled:1,
             release_quarantine_complete:1;
} pstvnc_mpeg_cal_effects_t;
typedef struct {
    int canvas_width, canvas_height;
    pstvnc_mpeg_cal_region_t defaults,current,committed;
    pstvnc_mpeg_cal_screen_t screen;
    unsigned has_committed:1, entry_latched:1, review_cross_armed:1,
             release_quarantine_pending:1;
} pstvnc_mpeg_calibration_t;
void pstvnc_mpeg_calibration_init(pstvnc_mpeg_calibration_t*,int,int);
pstvnc_mpeg_cal_effects_t pstvnc_mpeg_calibration_update(
    pstvnc_mpeg_calibration_t*, const pstvnc_mpeg_cal_input_t*);
int pstvnc_mpeg_calibration_owns_foreground(
    const pstvnc_mpeg_calibration_t*);
pstvnc_mpeg_cal_rect_t pstvnc_mpeg_calibration_rfb_suppression_rect(
    const pstvnc_mpeg_calibration_t*, const pstvnc_mpeg_cal_region_t*);
#endif
