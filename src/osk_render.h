/*
 * File synopsis:
 * Defines the platform-neutral visual renderer for the reconstructed on-screen
 * keyboard.
 *
 * The historical customized OSK is the visual specification. This interface
 * does not preserve the historical implementation architecture.
 *
 * The renderer:
 *
 *   - consumes only pstvnc_osk_t state;
 *   - writes into caller-owned local presentation pixels;
 *   - reproduces the adopted 600x178 keyboard appearance;
 *   - has no controller, RFB, GS, platform, foreground, or presentation
 *     ownership.
 *
 * Pixel contract:
 * Each output pixel is an opaque 16-bit project presentation pixel with
 * five-bit red, green, and blue channels. This is a project-owned semantic
 * contract; translation to any platform SDK representation belongs below the
 * platform graphics boundary.
 *
 * Context:
 *   GitHub Issue #39;
 *   K5 historical OSK authority extraction;
 *   K7A historical renderer census.
 */

#ifndef PSTVNC_OSK_RENDER_H
#define PSTVNC_OSK_RENDER_H

#include <stddef.h>
#include <stdint.h>

#include "osk.h"

#define PSTVNC_OSK_SURFACE_PIXEL_COUNT \
    ((size_t)PSTVNC_OSK_WIDTH * (size_t)PSTVNC_OSK_HEIGHT)

/*
 * Render the complete current OSK appearance.
 *
 * surface_pixels must provide at least PSTVNC_OSK_SURFACE_PIXEL_COUNT pixels.
 *
 * Returns 1 on success and 0 for invalid state/storage.
 */
int pstvnc_osk_render_surface(
    const pstvnc_osk_t *osk,
    uint16_t *surface_pixels,
    size_t surface_capacity_pixels);

#endif /* PSTVNC_OSK_RENDER_H */
