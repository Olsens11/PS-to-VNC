/*
 * File synopsis:
 * Defines the experiment-local CPU rasterizer for native MPEG presentation
 * calibration. It consumes only the platform-neutral render plan plus a frozen
 * CT16 desktop snapshot and writes a complete caller-owned CT16 surface.
 *
 * This module owns no GS resources, controller semantics, RFB framing, MPEG
 * decode, or permanent compositor state.
 */
#ifndef PSTVNC_H1_MPEG_CALIBRATION_RASTER_H
#define PSTVNC_H1_MPEG_CALIBRATION_RASTER_H

#include "h1_mpeg_calibration_render.h"

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_H1_MPEG_CAL_COLOR_BLACK  ((uint16_t)0x8000u)
#define PSTVNC_H1_MPEG_CAL_COLOR_WHITE  ((uint16_t)0xffffu)
#define PSTVNC_H1_MPEG_CAL_COLOR_ORANGE ((uint16_t)0x8159u) /* #CC5500 */
#define PSTVNC_H1_MPEG_CAL_COLOR_TEAL   ((uint16_t)0xb5e0u) /* #01796F */

int pstvnc_h1_mpeg_calibration_rasterize(
    const pstvnc_h1_mpeg_calibration_render_plan_t *plan,
    const uint16_t *frozen_desktop,
    size_t frozen_pixel_count,
    int canvas_width,
    int canvas_height,
    uint16_t *surface_pixels,
    size_t surface_capacity_pixels);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_RASTER_H */
