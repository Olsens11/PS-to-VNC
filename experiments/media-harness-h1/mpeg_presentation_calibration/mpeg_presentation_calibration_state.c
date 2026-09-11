#include <string.h>
#include "mpeg_presentation_calibration.h"
void pstvnc_mpeg_calibration_edit_geometry(
    pstvnc_mpeg_calibration_t*,const pstvnc_mpeg_cal_input_t*);
void pstvnc_mpeg_calibration_init(pstvnc_mpeg_calibration_t*c,int w,int h){
    int rw,rh; memset(c,0,sizeof(*c)); c->canvas_width=w;c->canvas_height=h;
    rw=(w/2)&~15; rh=(h/2)&~15; if(rw<16)rw=16;if(rh<16)rh=16;
    c->defaults.x=(w-rw)/2;c->defaults.y=(h-rh)/2;
    c->defaults.width=rw;c->defaults.height=rh;
    c->current=c->defaults;c->committed=c->defaults;
}
int pstvnc_mpeg_calibration_owns_foreground(const pstvnc_mpeg_calibration_t*c){
    return c&&(c->screen!=PSTVNC_MPEG_CAL_INACTIVE||c->release_quarantine_pending);
}
pstvnc_mpeg_cal_effects_t pstvnc_mpeg_calibration_update(
    pstvnc_mpeg_calibration_t*c,const pstvnc_mpeg_cal_input_t*i){
    pstvnc_mpeg_cal_effects_t e={0,0,0,0,0}; uint32_t chord=PSTVNC_MPEG_CAL_START|PSTVNC_MPEG_CAL_SELECT;
    int chord_down;
    if(!c||!i)return e;
    chord_down=(i->down&chord)==chord;
    if(!chord_down)c->entry_latched=0;
    if(c->release_quarantine_pending){
        if((i->down&PSTVNC_MPEG_CAL_ALL)==0){c->release_quarantine_pending=0;e.release_quarantine_complete=1;}
        return e;
    }
    if(c->screen==PSTVNC_MPEG_CAL_INACTIVE){
        if(chord_down&&!c->entry_latched){c->entry_latched=1;c->current=c->has_committed?c->committed:c->defaults;c->screen=PSTVNC_MPEG_CAL_EDIT;e.enter_foreground=1;}
        return e;
    }
    if(c->screen==PSTVNC_MPEG_CAL_CONTROLS){
        if(i->pressed&PSTVNC_MPEG_CAL_CROSS){c->screen=PSTVNC_MPEG_CAL_REVIEW;c->review_cross_armed=0;}
        else if(i->pressed&(PSTVNC_MPEG_CAL_TRIANGLE|PSTVNC_MPEG_CAL_CIRCLE))c->screen=PSTVNC_MPEG_CAL_EDIT;
        return e;
    }
    if(c->screen==PSTVNC_MPEG_CAL_REVIEW){
        if(!(i->down&PSTVNC_MPEG_CAL_CROSS))c->review_cross_armed=1;
        if(i->pressed&PSTVNC_MPEG_CAL_CIRCLE){c->screen=PSTVNC_MPEG_CAL_EDIT;c->review_cross_armed=0;return e;}
        if(c->review_cross_armed&&(i->pressed&PSTVNC_MPEG_CAL_CROSS)){
            c->committed=c->current;c->has_committed=1;c->screen=PSTVNC_MPEG_CAL_INACTIVE;
            c->release_quarantine_pending=1;e.accepted=1;e.leave_foreground=1;
        } return e;
    }
    if(i->pressed&PSTVNC_MPEG_CAL_CROSS){c->screen=PSTVNC_MPEG_CAL_REVIEW;c->review_cross_armed=0;return e;}
    if(i->pressed&PSTVNC_MPEG_CAL_TRIANGLE){c->screen=PSTVNC_MPEG_CAL_CONTROLS;return e;}
    if(i->pressed&PSTVNC_MPEG_CAL_START){c->current=c->defaults;return e;}
    if(i->pressed&PSTVNC_MPEG_CAL_CIRCLE){
        c->current=c->has_committed?c->committed:c->defaults;c->screen=PSTVNC_MPEG_CAL_INACTIVE;
        c->release_quarantine_pending=1;e.cancelled=1;e.leave_foreground=1;return e;
    }
    pstvnc_mpeg_calibration_edit_geometry(c,i); return e;
}
