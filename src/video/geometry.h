/*
 * File synopsis:
 * Defines raster rectangles and reusable geometry operations; display
 * transactions remain separate.
 */

#ifndef PSTVNC_VIDEO_GEOMETRY_H
#define PSTVNC_VIDEO_GEOMETRY_H

#include "video/mode.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Absolute rectangle in video-raster coordinates.
 */
typedef struct pstvnc_video_rect {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
} pstvnc_video_rect_t;

/*
 * Validate a safe-area selection.
 *
 * width/height describe the selected rectangle.
 * offset_x/offset_y are signed displacements from centered placement.
 */
int pstvnc_video_geometry_safe_area_valid(
    const pstvnc_video_mode_t *mode,
    int width,
    int height,
    int offset_x,
    int offset_y
);

/*
 * Convert a valid center-relative safe-area selection to an absolute
 * rectangle in the video-mode raster.
 */
int pstvnc_video_geometry_safe_area_rect(
    const pstvnc_video_mode_t *mode,
    int width,
    int height,
    int offset_x,
    int offset_y,
    pstvnc_video_rect_t *out_rect
);

/*
 * Fit a logical desktop capacity inside an already resolved safe area.
 *
 * The selected desktop dimensions are:
 *
 *     min(safe_area, desktop_capacity)
 *
 * and are centered inside the safe area.
 */
int pstvnc_video_geometry_presented_rect(
    const pstvnc_video_mode_t *mode,
    const pstvnc_video_rect_t *safe_area,
    unsigned int desktop_capacity_width,
    unsigned int desktop_capacity_height,
    pstvnc_video_rect_t *out_rect
);

#ifdef __cplusplus
}
#endif

#endif
