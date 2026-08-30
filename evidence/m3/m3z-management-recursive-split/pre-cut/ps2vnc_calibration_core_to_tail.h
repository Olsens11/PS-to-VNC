#ifndef PS2VNC_CALIBRATION_CORE_TO_TAIL_H
#define PS2VNC_CALIBRATION_CORE_TO_TAIL_H

/*
 * M3X calibration tail interface.
 *
 * SCREEN_FIT constants are exact definitions
 * from management_config. Function declarations
 * are derived from qualified pre-cut GCC aux-info.
 */

#define SCREEN_FIT_FINE_STEP            1
#define SCREEN_FIT_FAST_STEP            4
#define SCREEN_FIT_BORDER               4
#define SCREEN_FIT_CORNER_SIZE          56
#define SCREEN_FIT_REPEAT_DELAY_POLLS  18
#define SCREEN_FIT_REPEAT_POLLS         4
#define SCREEN_FIT_START_WIDTH   (OUTPUT_WIDTH)
#define SCREEN_FIT_START_HEIGHT  (OUTPUT_HEIGHT)
#define SCREEN_FIT_ACTION_MOVE   0x10000u

extern int ps2vnc_display_save_to_pi (int width, int height, int offset_x, int offset_y);
extern void screen_fit_clamp_interactive (int *width, int *height, int *offset_x, int *offset_y);
extern int screen_fit_geometry_valid (int width, int height, int offset_x, int offset_y);
extern void screen_fit_prim_sprite (GSGLOBAL *g, int x0, int y0, int x1, int y1, int z, u64 color);

#endif
