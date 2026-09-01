/*
 * File synopsis:
 * Declares the fixed Issue #7 display-conversion contract while hiding PS2
 * graphics mechanisms.
 */

#ifndef PSTVNC_DISPLAY_H
#define PSTVNC_DISPLAY_H

#include <stddef.h>
#include <stdint.h>

#include "framebuffer.h"

#define PSTVNC_DISPLAY_PHYSICAL_WIDTH 720u
#define PSTVNC_DISPLAY_PHYSICAL_HEIGHT 480u
#define PSTVNC_DISPLAY_WIDTH 704u
#define PSTVNC_DISPLAY_HEIGHT 462u
#define PSTVNC_DISPLAY_OFFSET_X (-4)
#define PSTVNC_DISPLAY_OFFSET_Y 3
#define PSTVNC_DISPLAY_PIXEL_COUNT \
    ((size_t)PSTVNC_DISPLAY_WIDTH * (size_t)PSTVNC_DISPLAY_HEIGHT)

/*
 * Convert one complete authoritative RFB desktop into the conventional GS16
 * representation used by the fixed Standard 480p presentation path.
 *
 * The RFB wire format is B5:G5:R5 with bit 15 unused. GS CT16 uses the same
 * color bits plus A1 in bit 15, so presentation sets that bit locally while
 * leaving the authoritative remote framebuffer unchanged.
 */
int pstvnc_display_prepare_gs16(
    const pstvnc_framebuffer_t *framebuffer,
    uint16_t *gs_pixels,
    size_t gs_capacity_pixels);

#endif
