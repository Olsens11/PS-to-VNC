#ifndef PS2VNC_UI_TO_DISPLAY_H
#define PS2VNC_UI_TO_DISPLAY_H

/*
 * M3M migration interface:
 * UI-controller helpers consumed by display orchestration.
 */

extern void display_menu_normalize_selection (void);
extern int display_menu_tab_for_mode (const ps2vnc_video_mode_t *mode);
extern void display_restore_ack_render_bitmap (void);
extern void display_confirm_render_bitmap (void);
extern void system_menu_render_bitmap (void);
extern void display_menu_render_bitmap (void);

#endif /* PS2VNC_UI_TO_DISPLAY_H */
