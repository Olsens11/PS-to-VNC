#ifndef PS2VNC_FRAMEBUFFER_CORE_TO_TAIL_H
#define PS2VNC_FRAMEBUFFER_CORE_TO_TAIL_H

/*
 * M3U recursive framebuffer split interface.
 *
 * Declarations are derived from the qualified
 * pre-cut translation unit and GCC aux-info.
 */

extern int hires_bg_is_field_separated (void);
extern unsigned int hires_bg_storage_row (unsigned int raster_y);
extern unsigned int ps2vnc_hires_mapped_storage_row (unsigned int presentation_y);
extern int sync_copy_hires_dirty_rows (short unsigned int *dst, const short unsigned int *src);

#endif
