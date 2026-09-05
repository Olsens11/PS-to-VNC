/*
 * File synopsis:
 * Declares the narrow PS2 graphics seam without exposing gsKit or VRAM
 * ownership upstream.
 *
 * The platform boundary accepts project presentation pixels and an optional
 * generic local overlay. The product meaning and ownership of that overlay
 * remain entirely outside this platform module.
 *
 * Context:
 *   docs/reconstruction/ISSUE7_MINIMAL_CORE.md;
 *   docs/CLEAN_ARCHITECTURE.md, "PS2 platform mechanisms";
 *   GitHub Issue #39 local presentation integration.
 */

#ifndef PSTVNC_PS2_GRAPHICS_H
#define PSTVNC_PS2_GRAPHICS_H

#include <stddef.h>
#include <stdint.h>

typedef struct pstvnc_ps2_graphics_overlay {
    const uint16_t *pixels;
    size_t pixel_count;

    unsigned int width;
    unsigned int height;

    unsigned int x;
    unsigned int y;
} pstvnc_ps2_graphics_overlay_t;

/* Initialize DMAKit and the fixed hardware-proven Standard 480p GS instance. */
int pstvnc_ps2_graphics_init(void);

/*
 * Present one complete logical desktop and, optionally, one local overlay.
 *
 * local_overlay == NULL means desktop-only presentation.
 *
 * The application supplies complete coherent pixel surfaces. This platform
 * owner alone maps those surfaces into gsKit/VRAM mechanisms.
 */
int pstvnc_ps2_graphics_present(
    const uint16_t *desktop_pixels,
    size_t desktop_pixel_count,
    const pstvnc_ps2_graphics_overlay_t *local_overlay);

void pstvnc_ps2_graphics_shutdown(void);

#endif /* PSTVNC_PS2_GRAPHICS_H */
