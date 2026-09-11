#include "mpeg_presentation_calibration.h"
static int clampi(int v,int lo,int hi){ return v<lo?lo:(v>hi?hi:v); }
static void clamp_region(pstvnc_mpeg_calibration_t*c){
    pstvnc_mpeg_cal_region_t*r=&c->current;
    r->x=clampi(r->x,0,c->canvas_width-r->width);
    r->y=clampi(r->y,0,c->canvas_height-r->height);
    r->inner_matte_x=clampi(r->inner_matte_x,0,r->width/2);
    r->inner_matte_y=clampi(r->inner_matte_y,0,r->height/2);
    r->outer_matte_x=clampi(r->outer_matte_x,0,c->canvas_width);
    r->outer_matte_y=clampi(r->outer_matte_y,0,c->canvas_height);
}
void pstvnc_mpeg_calibration_edit_geometry(
    pstvnc_mpeg_calibration_t*c,const pstvnc_mpeg_cal_input_t*i){
    int h=((i->down&PSTVNC_MPEG_CAL_RIGHT)!=0)-((i->down&PSTVNC_MPEG_CAL_LEFT)!=0);
    int updn=((i->down&PSTVNC_MPEG_CAL_UP)!=0)-((i->down&PSTVNC_MPEG_CAL_DOWN)!=0);
    pstvnc_mpeg_cal_region_t*r=&c->current;
    if(i->down&PSTVNC_MPEG_CAL_R1){ r->x+=h; r->y-=updn; clamp_region(c); return; }
    if(i->down&PSTVNC_MPEG_CAL_R2){
        if(h){r->inner_matte_x+=h;r->inner_matte_y+=h;}
        r->inner_matte_y-=updn; clamp_region(c); return;
    }
    if(i->down&PSTVNC_MPEG_CAL_L2){
        if(h){r->outer_matte_x-=h;r->outer_matte_y-=h;}
        r->outer_matte_y+=updn; clamp_region(c); return;
    }
    if(h){
        int nw=clampi((r->width+h*16)&~15,16,c->canvas_width&~15);
        int cx2=r->x*2+r->width; r->width=nw; r->x=(cx2-nw)/2;
    }
    if(updn){
        int nh=clampi((r->height+updn*16)&~15,16,c->canvas_height&~15);
        int cy2=r->y*2+r->height; r->height=nh; r->y=(cy2-nh)/2;
    }
    clamp_region(c);
}
pstvnc_mpeg_cal_rect_t pstvnc_mpeg_calibration_rfb_suppression_rect(
    const pstvnc_mpeg_calibration_t*c,const pstvnc_mpeg_cal_region_t*r){
    pstvnc_mpeg_cal_rect_t o={0,0,0,0}; int l,t,rr,b;
    if(!c||!r)return o;
    l=clampi(r->x-r->outer_matte_x,0,c->canvas_width);
    t=clampi(r->y-r->outer_matte_y,0,c->canvas_height);
    rr=clampi(r->x+r->width+r->outer_matte_x,0,c->canvas_width);
    b=clampi(r->y+r->height+r->outer_matte_y,0,c->canvas_height);
    o.x=l;o.y=t;o.width=rr-l;o.height=b-t;return o;
}
