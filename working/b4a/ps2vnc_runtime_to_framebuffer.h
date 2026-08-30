#ifndef PS2VNC_RUNTIME_TO_FRAMEBUFFER_H
#define PS2VNC_RUNTIME_TO_FRAMEBUFFER_H

/*
 * M3K migration interface:
 * runtime-support exports consumed by framebuffer engine.
 *
 * Generated from the first real-TU compiler boundary.
 */

#define CONTROLLER_STACK_SIZE 8192

#define TEST31_HDR_SENTINEL 0x5A

#define RFB_ENCODING_RAW                    0x00000000u

#define RFB_ENCODING_HEXTILE                0x00000005u

#define RFB_ENCODING_EXTENDED_DESKTOP_SIZE  0xFFFFFECCu

#define HEXTILE_RAW                  0x01u

#define HEXTILE_BACKGROUND_SPECIFIED 0x02u

#define HEXTILE_FOREGROUND_SPECIFIED 0x04u

#define HEXTILE_ANY_SUBRECTS         0x08u

#define HEXTILE_SUBRECTS_COLORED     0x10u

extern unsigned char controller_stack[CONTROLLER_STACK_SIZE]
    __attribute__((aligned(16)));

extern int controller_thread_id ;

extern volatile unsigned int rfb_out_head ;

extern volatile unsigned int rfb_out_tail ;

extern volatile unsigned int rfb_out_count ;

extern volatile unsigned int live_hdr_start_pos ;

extern volatile unsigned int live_hdr_start_end ;

extern volatile unsigned int live_hdr_end_pos ;

extern volatile unsigned int live_hdr_end_end ;

extern volatile unsigned int live_hdr_recv_call ;

extern volatile unsigned int live_update_start_pos ;

extern volatile unsigned int live_update_start_end ;

extern volatile unsigned int live_update_end_pos ;

extern volatile unsigned int live_update_end_end ;

extern volatile unsigned int live_update_recv_call ;

extern unsigned char test29_row_scratch[VNC_WIDTH * 2];

extern unsigned int rfb_dirty_y0 ;

extern unsigned int rfb_dirty_y1 ;

extern int rfb_dirty_valid ;

extern unsigned int rfb_dirty_row_x0[VNC_HEIGHT];

extern unsigned int rfb_dirty_row_x1[VNC_HEIGHT];

extern unsigned char rfb_dirty_row_valid[VNC_HEIGHT];

extern int controller_sock ;

extern u64 profile_hextile_ticks ;

extern int profile_hextile_active ;

extern u64 profile_dirty_work_ticks ;

extern u64 profile_dirty_work_start_tick ;

extern unsigned int profile_dirty_updates ;

extern unsigned int profile_raw_rects ;

extern unsigned int profile_hextile_rects ;

extern volatile int rfb_message_boundary_local_yield ;

extern int rfb_extended_desktop_seen ;

extern unsigned int rfb_extended_desktop_reason ;

extern unsigned int rfb_extended_desktop_result ;

extern unsigned int rfb_extended_desktop_width ;

extern unsigned int rfb_extended_desktop_height ;

extern unsigned int rfb_extended_desktop_screen_count ;

extern unsigned int rfb_first_screen_id ;

extern unsigned int rfb_first_screen_x ;

extern unsigned int rfb_first_screen_y ;

extern unsigned int rfb_first_screen_width ;

extern unsigned int rfb_first_screen_height ;

extern unsigned int rfb_first_screen_flags ;

extern int rfb_resize_expected ;

extern unsigned int rfb_resize_expected_width ;

extern unsigned int rfb_resize_expected_height ;

extern int ps2vnc_hires_requires_vertical_mapping (void);

extern unsigned int read_be16 (const unsigned char *p);

extern unsigned int read_be32 (const unsigned char *p);

extern int recv_exact (int sock, void *data, int len);

extern void rfb_dirty_mark_rect (unsigned int x, unsigned int y, unsigned int w, unsigned int h);

extern void rfb_dirty_reset (void);

#endif /* PS2VNC_RUNTIME_TO_FRAMEBUFFER_H */
