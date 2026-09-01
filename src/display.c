/*
 * File synopsis:
 * Converts a valid authoritative RFB framebuffer into GS-compatible CT16
 * presentation pixels without modifying remote authority.
 */

#include "display.h"

int pstvnc_display_prepare_gs16(
    const pstvnc_framebuffer_t *framebuffer,
    uint16_t *gs_pixels,
    size_t gs_capacity_pixels)
{
    size_t i;

    if (framebuffer == NULL ||
        gs_pixels == NULL ||
        !framebuffer->valid ||
        framebuffer->pixels == NULL ||
        framebuffer->width != PSTVNC_DISPLAY_WIDTH ||
        framebuffer->height != PSTVNC_DISPLAY_HEIGHT ||
        gs_capacity_pixels < PSTVNC_DISPLAY_PIXEL_COUNT)
        return 0;

    /*
     * The negotiated RFB format already uses the GS CT16 B5:G5:R5 color-bit
     * layout. Presentation therefore preserves the lower 15 bits and sets A1,
     * which gsKit expects for an opaque CT16 texel. Conversion targets a
     * separate buffer so display requirements never alter remote authority.
     */
    for (i = 0; i < PSTVNC_DISPLAY_PIXEL_COUNT; i++)
        gs_pixels[i] = (uint16_t)(framebuffer->pixels[i] | 0x8000u);

    return 1;
}
