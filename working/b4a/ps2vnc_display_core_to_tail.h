#ifndef PS2VNC_DISPLAY_CORE_TO_TAIL_H
#define PS2VNC_DISPLAY_CORE_TO_TAIL_H

/*
 * M3S recursive display split interface.
 *
 * Declarations are derived from the qualified
 * pre-cut translation unit and GCC aux-info.
 */

extern int ps2vnc_global_rollback_display_transaction (int old_sock, int transport_dirty, short unsigned int *rfb_framebuffer, short unsigned int *gs_framebuffer, GSGLOBAL **display, GSTEXTURE *tex, GSTEXTURE *osk_tex);
extern int ps2vnc_remote_control_poll_idle (void);
extern void ps2vnc_remote_display_fail (unsigned int error_code);
extern int ps2vnc_remote_display_lease_expired (void);
extern int ps2vnc_remote_result_post (const char *state, unsigned int error_code);
extern int ps2vnc_startup_finalize_display_transaction (void);
extern int ps2vnc_switch_display_mode (int sock, short unsigned int *rfb_framebuffer, short unsigned int *gs_framebuffer, GSGLOBAL **display, GSTEXTURE *tex, GSTEXTURE *osk_tex, const ps2vnc_video_mode_t *mode, unsigned int out_x, unsigned int out_y, unsigned int width, unsigned int height);

#endif
