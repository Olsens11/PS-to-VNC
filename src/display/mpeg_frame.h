/*
 * File synopsis:
 * Defines the neutral decoded MPEG RGB16 macroblock surface consumed by the
 * A004 presentation/compositor seam, plus the bounded host-testable detile
 * adapter from 16x16 macroblock order into a linear RGB16 surface.
 *
 * This value carries codec-surface facts only. It contains no MPEG decoder
 * private state, run generation, presentation geometry, GS resource, clock,
 * Transport, or Application ownership.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-SHARED-COMPOSITOR-FIRST-SYNC-R4.
 */

#ifndef PSTVNC_MPEG_FRAME_H
#define PSTVNC_MPEG_FRAME_H

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE 16u
#define PSTVNC_MPEG_RGB16_MAX_WIDTH 704u
#define PSTVNC_MPEG_RGB16_MAX_HEIGHT 480u
#define PSTVNC_MPEG_RGB16_MAX_PIXELS \
    ((size_t)PSTVNC_MPEG_RGB16_MAX_WIDTH * \
     (size_t)PSTVNC_MPEG_RGB16_MAX_HEIGHT)

typedef struct pstvnc_mpeg_rgb16_macroblock_surface {
    const uint16_t *pixels;
    size_t pixel_capacity;
    unsigned int width;
    unsigned int height;
} pstvnc_mpeg_rgb16_macroblock_surface_t;

/*
 * Validate one decoder-independent RGB16 macroblock surface.
 *
 * Width/height must be positive 16-pixel multiples within the qualified H1
 * maximum surface and pixel_capacity must cover the complete coded surface.
 */
int pstvnc_mpeg_rgb16_macroblock_surface_valid(
    const pstvnc_mpeg_rgb16_macroblock_surface_t *surface);

/*
 * Detile one valid 16x16-macroblock-order surface into row-major RGB16.
 * The destination is caller-owned bounded storage and may not alias source.
 */
int pstvnc_mpeg_rgb16_detile(
    const pstvnc_mpeg_rgb16_macroblock_surface_t *surface,
    uint16_t *linear_pixels,
    size_t linear_capacity_pixels);

#endif /* PSTVNC_MPEG_FRAME_H */
