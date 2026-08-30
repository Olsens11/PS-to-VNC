#ifndef PS2VNC_UI_CORE_TO_TAIL_H
#define PS2VNC_UI_CORE_TO_TAIL_H

/*
 * M3Q recursive UI split interface.
 * Generated from the qualified parent and GCC aux-info.
 */

#define DPAD_ACCEL_START_TICKS 60
#define DPAD_SCROLL_REPEAT_POLLS 5
#define RFB_SCROLL_DOWN 0x10
#define RFB_SCROLL_LEFT 0x20
#define RFB_SCROLL_RIGHT 0x40
#define RFB_SCROLL_UP 0x08

extern int display_menu_move_horizontal_selection (int selection, int direction);
extern int display_menu_move_vertical_selection (int selection, int direction);
extern void display_menu_open_for_active (int parent_system);
extern int dpad_hold_velocity_q8 (unsigned int hold_ticks);
extern int osk_activate_selected (int sock);
extern void osk_move_horizontal (int direction);
extern void osk_move_vertical (int direction);
extern int osk_send_key_with_modifiers (int sock, unsigned int keysym);
extern unsigned char scroll_button_for_stick (unsigned char h, unsigned char v, int *magnitude);
extern unsigned int scroll_repeat_delay (int magnitude);
extern int send_scroll_pulse (int sock, unsigned char base_buttons, unsigned char scroll_button, int x, int y);
extern int stick_velocity_q8 (unsigned char value);
extern int wake_renderer_with_pointer_jiggle (int sock, int x, int y);

#endif
