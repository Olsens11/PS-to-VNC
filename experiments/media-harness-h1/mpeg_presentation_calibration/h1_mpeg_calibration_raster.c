/*
 * File synopsis:
 * Renders the native MPEG-calibration selector and modal text into a complete
 * caller-owned CT16 surface while preserving the last actually-presented RFB
 * desktop as the immutable visual underlay.
 */
#include "h1_mpeg_calibration_raster.h"

#include <stdio.h>
#include <string.h>

#define CAL_SELECTOR_SEGMENT 11
#define CAL_SELECTOR_THICKNESS 2
#define CAL_PANEL_X 36
#define CAL_PANEL_Y 34
#define CAL_PANEL_MARGIN 18

typedef struct pstvnc_h1_mpeg_cal_glyph {
    char character;
    uint8_t rows[7];
} pstvnc_h1_mpeg_cal_glyph_t;

static const pstvnc_h1_mpeg_cal_glyph_t calibration_font[] = {
    { ' ', {0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    { 'A', {0x0e,0x11,0x11,0x1f,0x11,0x11,0x11} },
    { 'B', {0x1e,0x11,0x11,0x1e,0x11,0x11,0x1e} },
    { 'C', {0x0f,0x10,0x10,0x10,0x10,0x10,0x0f} },
    { 'D', {0x1e,0x11,0x11,0x11,0x11,0x11,0x1e} },
    { 'E', {0x1f,0x10,0x10,0x1e,0x10,0x10,0x1f} },
    { 'F', {0x1f,0x10,0x10,0x1e,0x10,0x10,0x10} },
    { 'G', {0x0f,0x10,0x10,0x17,0x11,0x11,0x0f} },
    { 'H', {0x11,0x11,0x11,0x1f,0x11,0x11,0x11} },
    { 'I', {0x1f,0x04,0x04,0x04,0x04,0x04,0x1f} },
    { 'J', {0x07,0x02,0x02,0x02,0x12,0x12,0x0c} },
    { 'K', {0x11,0x12,0x14,0x18,0x14,0x12,0x11} },
    { 'L', {0x10,0x10,0x10,0x10,0x10,0x10,0x1f} },
    { 'M', {0x11,0x1b,0x15,0x15,0x11,0x11,0x11} },
    { 'N', {0x11,0x19,0x15,0x13,0x11,0x11,0x11} },
    { 'O', {0x0e,0x11,0x11,0x11,0x11,0x11,0x0e} },
    { 'P', {0x1e,0x11,0x11,0x1e,0x10,0x10,0x10} },
    { 'Q', {0x0e,0x11,0x11,0x11,0x15,0x12,0x0d} },
    { 'R', {0x1e,0x11,0x11,0x1e,0x14,0x12,0x11} },
    { 'S', {0x0f,0x10,0x10,0x0e,0x01,0x01,0x1e} },
    { 'T', {0x1f,0x04,0x04,0x04,0x04,0x04,0x04} },
    { 'U', {0x11,0x11,0x11,0x11,0x11,0x11,0x0e} },
    { 'V', {0x11,0x11,0x11,0x11,0x11,0x0a,0x04} },
    { 'W', {0x11,0x11,0x11,0x15,0x15,0x15,0x0a} },
    { 'X', {0x11,0x11,0x0a,0x04,0x0a,0x11,0x11} },
    { 'Y', {0x11,0x11,0x0a,0x04,0x04,0x04,0x04} },
    { 'Z', {0x1f,0x01,0x02,0x04,0x08,0x10,0x1f} },
    { '0', {0x0e,0x11,0x13,0x15,0x19,0x11,0x0e} },
    { '1', {0x04,0x0c,0x04,0x04,0x04,0x04,0x0e} },
    { '2', {0x0e,0x11,0x01,0x02,0x04,0x08,0x1f} },
    { '3', {0x1e,0x01,0x01,0x0e,0x01,0x01,0x1e} },
    { '4', {0x02,0x06,0x0a,0x12,0x1f,0x02,0x02} },
    { '5', {0x1f,0x10,0x10,0x1e,0x01,0x01,0x1e} },
    { '6', {0x0e,0x10,0x10,0x1e,0x11,0x11,0x0e} },
    { '7', {0x1f,0x01,0x02,0x04,0x08,0x08,0x08} },
    { '8', {0x0e,0x11,0x11,0x0e,0x11,0x11,0x0e} },
    { '9', {0x0e,0x11,0x11,0x0f,0x01,0x01,0x0e} },
    { '+', {0x00,0x04,0x04,0x1f,0x04,0x04,0x00} },
    { '-', {0x00,0x00,0x00,0x1f,0x00,0x00,0x00} },
    { '/', {0x01,0x02,0x02,0x04,0x08,0x08,0x10} },
    { ':', {0x00,0x06,0x06,0x00,0x06,0x06,0x00} },
    { '?', {0x0e,0x11,0x01,0x02,0x04,0x00,0x04} }
};

static char calibration_upper(char c){ if(c>='a'&&c<='z') return (char)(c-('a'-'A')); return c; }
static const uint8_t *calibration_glyph(char c){ size_t i; char u=calibration_upper(c); for(i=0;i<sizeof(calibration_font)/sizeof(calibration_font[0]);i++) if(calibration_font[i].character==u) return calibration_font[i].rows; return calibration_font[sizeof(calibration_font)/sizeof(calibration_font[0])-1u].rows; }
static int calibration_surface_valid(int w,int h,size_t n){ return w>0&&h>0&&n>=(size_t)w*(size_t)h; }

static void calibration_fill_rect(uint16_t*p,int w,int h,int x,int y,int rw,int rh,uint16_t c){ int x0=x<0?0:x,y0=y<0?0:y,x1=x+rw,y1=y+rh,r,col; if(x1>w)x1=w;if(y1>h)y1=h;if(x0>=x1||y0>=y1)return; for(r=y0;r<y1;r++)for(col=x0;col<x1;col++)p[(size_t)r*(size_t)w+(size_t)col]=c; }
static uint16_t calibration_darken_pixel(uint16_t p){ unsigned r=p&31u,g=(p>>5)&31u,b=(p>>10)&31u; return (uint16_t)(0x8000u|(r>>1)|((g>>1)<<5)|((b>>1)<<10)); }
static void calibration_darken_rect(uint16_t*p,int w,int h,int x,int y,int rw,int rh){ int x0=x<0?0:x,y0=y<0?0:y,x1=x+rw,y1=y+rh,r,c; if(x1>w)x1=w;if(y1>h)y1=h;if(x0>=x1||y0>=y1)return; for(r=y0;r<y1;r++)for(c=x0;c<x1;c++){size_t i=(size_t)r*(size_t)w+(size_t)c;p[i]=calibration_darken_pixel(p[i]);} }
static void calibration_copy_rect(uint16_t*d,const uint16_t*s,int w,int h,int x,int y,int rw,int rh){ int x0=x<0?0:x,y0=y<0?0:y,x1=x+rw,y1=y+rh,r; if(x1>w)x1=w;if(y1>h)y1=h;if(x0>=x1||y0>=y1)return; for(r=y0;r<y1;r++){size_t i=(size_t)r*(size_t)w+(size_t)x0;memcpy(d+i,s+i,(size_t)(x1-x0)*sizeof(*d));} }

static void calibration_draw_h(uint16_t*p,int w,int h,int x,int y,int len){int o=0,s=0;while(o<len){int span=CAL_SELECTOR_SEGMENT;uint16_t c=(s&1)?PSTVNC_H1_MPEG_CAL_COLOR_TEAL:PSTVNC_H1_MPEG_CAL_COLOR_ORANGE;if(span>len-o)span=len-o;calibration_fill_rect(p,w,h,x+o,y,span,CAL_SELECTOR_THICKNESS,c);o+=span;s++;}}
static void calibration_draw_v(uint16_t*p,int w,int h,int x,int y,int len){int o=0,s=0;while(o<len){int span=CAL_SELECTOR_SEGMENT;uint16_t c=(s&1)?PSTVNC_H1_MPEG_CAL_COLOR_TEAL:PSTVNC_H1_MPEG_CAL_COLOR_ORANGE;if(span>len-o)span=len-o;calibration_fill_rect(p,w,h,x,y+o,CAL_SELECTOR_THICKNESS,span,c);o+=span;s++;}}
static void calibration_draw_selector(uint16_t*p,int w,int h,const pstvnc_mpeg_cal_region_t*r){if(r->width<=0||r->height<=0)return;calibration_draw_h(p,w,h,r->x,r->y,r->width);calibration_draw_h(p,w,h,r->x,r->y+r->height-CAL_SELECTOR_THICKNESS,r->width);calibration_draw_v(p,w,h,r->x,r->y,r->height);calibration_draw_v(p,w,h,r->x+r->width-CAL_SELECTOR_THICKNESS,r->y,r->height);}

static int calibration_text_width(const char*t,int scale){size_t n;if(!t||scale<=0)return 0;n=strlen(t);return n? (int)(n*(size_t)(6*scale)-(size_t)scale):0;}
static void calibration_draw_glyph(uint16_t*p,int w,int h,int x,int y,char ch,int scale,uint16_t c){const uint8_t*rows=calibration_glyph(ch);int r,col;for(r=0;r<7;r++)for(col=0;col<5;col++)if(rows[r]&(uint8_t)(1u<<(4-col)))calibration_fill_rect(p,w,h,x+col*scale,y+r*scale,scale,scale,c);}
static void calibration_draw_text_raw(uint16_t*p,int w,int h,int x,int y,const char*t,int scale,uint16_t c){size_t i;for(i=0;t&&t[i];i++)calibration_draw_glyph(p,w,h,x+(int)i*6*scale,y,t[i],scale,c);}
static void calibration_draw_text(uint16_t*p,int w,int h,int x,int y,const char*t,int scale){calibration_draw_text_raw(p,w,h,x-1,y,t,scale,PSTVNC_H1_MPEG_CAL_COLOR_BLACK);calibration_draw_text_raw(p,w,h,x+1,y,t,scale,PSTVNC_H1_MPEG_CAL_COLOR_BLACK);calibration_draw_text_raw(p,w,h,x,y-1,t,scale,PSTVNC_H1_MPEG_CAL_COLOR_BLACK);calibration_draw_text_raw(p,w,h,x,y+1,t,scale,PSTVNC_H1_MPEG_CAL_COLOR_BLACK);calibration_draw_text_raw(p,w,h,x,y,t,scale,PSTVNC_H1_MPEG_CAL_COLOR_WHITE);}
static void calibration_draw_right(uint16_t*p,int w,int h,int right,int y,const char*t,int scale){int x=right-calibration_text_width(t,scale);if(x<0)x=0;calibration_draw_text(p,w,h,x,y,t,scale);}

static void calibration_draw_edit_hint(uint16_t*p,int w,int h,const pstvnc_mpeg_cal_region_t*r){static const char full[]="TRIANGLE: CONTROLS    X: CONFIRM",first[]="TRIANGLE: CONTROLS",second[]="X: CONFIRM";int right=r->x+r->width,scale=2,line,y;if(calibration_text_width(full,scale)>right)scale=1;line=7*scale;if(r->y+r->height+6+line<=h)y=r->y+r->height+6;else if(r->y-6-line>=0)y=r->y-6-line;else y=1;if(calibration_text_width(full,scale)<=right){calibration_draw_right(p,w,h,right,y,full,scale);return;}if(r->y+r->height+6+2*line+2<=h)y=r->y+r->height+6;else if(r->y-6-2*line-2>=0)y=r->y-6-2*line-2;else y=1;calibration_draw_right(p,w,h,right,y,first,scale);calibration_draw_right(p,w,h,right,y+line+2,second,scale);}
static void calibration_draw_inner(uint16_t*p,int w,int h,const pstvnc_mpeg_cal_region_t*r){int ix=r->inner_matte_x,iy=r->inner_matte_y,mw=r->width-2*ix;if(ix>0){calibration_fill_rect(p,w,h,r->x,r->y,ix,r->height,PSTVNC_H1_MPEG_CAL_COLOR_BLACK);calibration_fill_rect(p,w,h,r->x+r->width-ix,r->y,ix,r->height,PSTVNC_H1_MPEG_CAL_COLOR_BLACK);}if(iy>0&&mw>0){calibration_fill_rect(p,w,h,r->x+ix,r->y,mw,iy,PSTVNC_H1_MPEG_CAL_COLOR_BLACK);calibration_fill_rect(p,w,h,r->x+ix,r->y+r->height-iy,mw,iy,PSTVNC_H1_MPEG_CAL_COLOR_BLACK);}}
static void calibration_draw_preview(uint16_t*p,const uint16_t*f,int w,int h,const pstvnc_h1_mpeg_calibration_render_plan_t*plan){calibration_darken_rect(p,w,h,plan->suppression_rect.x,plan->suppression_rect.y,plan->suppression_rect.width,plan->suppression_rect.height);calibration_copy_rect(p,f,w,h,plan->region.x,plan->region.y,plan->region.width,plan->region.height);calibration_draw_inner(p,w,h,&plan->region);calibration_draw_selector(p,w,h,&plan->region);}

static void calibration_draw_panel(uint16_t*p,int w,int h){int pw=w-2*CAL_PANEL_X,ph=h-2*CAL_PANEL_Y;calibration_darken_rect(p,w,h,CAL_PANEL_X,CAL_PANEL_Y,pw,ph);calibration_fill_rect(p,w,h,CAL_PANEL_X,CAL_PANEL_Y,pw,2,PSTVNC_H1_MPEG_CAL_COLOR_WHITE);calibration_fill_rect(p,w,h,CAL_PANEL_X,CAL_PANEL_Y+ph-2,pw,2,PSTVNC_H1_MPEG_CAL_COLOR_WHITE);calibration_fill_rect(p,w,h,CAL_PANEL_X,CAL_PANEL_Y,2,ph,PSTVNC_H1_MPEG_CAL_COLOR_WHITE);calibration_fill_rect(p,w,h,CAL_PANEL_X+pw-2,CAL_PANEL_Y,2,ph,PSTVNC_H1_MPEG_CAL_COLOR_WHITE);}
static void calibration_draw_controls(uint16_t*p,int w,int h){static const char*lines[]={"DPAD: RESIZE","R1 + DPAD: MOVE","R2 + DPAD: INNER MATTE","L2 + DPAD: OUTER MATTE","INNER L/R: LEFT -  RIGHT +","INNER U/D: UP -  DOWN +","OUTER L/R: LEFT +  RIGHT -","OUTER U/D: UP +  DOWN -"};size_t i;int x=CAL_PANEL_X+CAL_PANEL_MARGIN,y=CAL_PANEL_Y+22;calibration_draw_panel(p,w,h);calibration_draw_text(p,w,h,x,y,"CALIBRATION CONTROLS",2);y+=34;for(i=0;i<sizeof(lines)/sizeof(lines[0]);i++){calibration_draw_text(p,w,h,x,y,lines[i],2);y+=31;if(i==3u)y+=8;}calibration_draw_text(p,w,h,x,h-CAL_PANEL_Y-24,"TRIANGLE/O: BACK    X: REVIEW",1);}
static void calibration_draw_review(uint16_t*p,int w,int h,const pstvnc_h1_mpeg_calibration_render_plan_t*plan){char region[64],inner[48],outer[48],mask[64];int x=CAL_PANEL_X+CAL_PANEL_MARGIN,y=CAL_PANEL_Y+28;(void)snprintf(region,sizeof(region),"REGION X:%d Y:%d W:%d H:%d",plan->region.x,plan->region.y,plan->region.width,plan->region.height);(void)snprintf(inner,sizeof(inner),"INNER X:%d Y:%d",plan->region.inner_matte_x,plan->region.inner_matte_y);(void)snprintf(outer,sizeof(outer),"OUTER X:%d Y:%d",plan->region.outer_matte_x,plan->region.outer_matte_y);(void)snprintf(mask,sizeof(mask),"RFB MASK X:%d Y:%d W:%d H:%d",plan->suppression_rect.x,plan->suppression_rect.y,plan->suppression_rect.width,plan->suppression_rect.height);calibration_draw_panel(p,w,h);calibration_draw_text(p,w,h,x,y,"REVIEW MPEG REGION",2);y+=48;calibration_draw_text(p,w,h,x,y,region,2);y+=40;calibration_draw_text(p,w,h,x,y,inner,2);y+=34;calibration_draw_text(p,w,h,x,y,outer,2);y+=34;calibration_draw_text(p,w,h,x,y,mask,2);calibration_draw_text(p,w,h,x,h-CAL_PANEL_Y-28,"O: BACK    X: ACCEPT",2);}

int pstvnc_h1_mpeg_calibration_rasterize(const pstvnc_h1_mpeg_calibration_render_plan_t*plan,const uint16_t*frozen,size_t frozen_count,int w,int h,uint16_t*surface,size_t capacity){size_t required;if(!plan||!frozen||!surface||!calibration_surface_valid(w,h,frozen_count)||!calibration_surface_valid(w,h,capacity))return 0;required=(size_t)w*(size_t)h;memcpy(surface,frozen,required*sizeof(*surface));switch(plan->kind){case PSTVNC_H1_MPEG_CAL_RENDER_EDIT:calibration_draw_preview(surface,frozen,w,h,plan);calibration_draw_edit_hint(surface,w,h,&plan->region);return 1;case PSTVNC_H1_MPEG_CAL_RENDER_CONTROLS:calibration_draw_controls(surface,w,h);return 1;case PSTVNC_H1_MPEG_CAL_RENDER_REVIEW:calibration_draw_review(surface,w,h,plan);return 1;default:return 0;}}
