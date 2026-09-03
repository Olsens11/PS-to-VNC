/*
 * File synopsis:
 * Declares the narrow PS2 graphics seam without exposing gsKit or VRAM
 * ownership upstream.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Fixed Standard 480p
 * presentation"; docs/CLEAN_ARCHITECTURE.md, "PS2 platform mechanisms".
 */

#ifndef PSTVNC_PS2_GRAPHICS_H
#define PSTVNC_PS2_GRAPHICS_H

#include <stddef.h>
#include <stdint.h>

/* Initialize DMAKit and the fixed hardware-proven Standard 480p GS instance. */
int pstvnc_ps2_graphics_init(void);

/* Upload and present one complete 704x462 GS16 desktop. */
int pstvnc_ps2_graphics_present(
    const uint16_t *gs_pixels,
    size_t pixel_count);

void pstvnc_ps2_graphics_shutdown(void);

#endif
