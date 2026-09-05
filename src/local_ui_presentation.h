/*
 * File synopsis:
 * Defines the platform-neutral presentation description for PS2-local UI.
 *
 * This module bridges local foreground state to a visual overlay description.
 * It does not own controller input, RFB transport, graphics hardware, VRAM,
 * framebuffer authority, or application actions.
 *
 * For Reconstruction Stage 2 the only local overlay is the adopted historical
 * on-screen keyboard. Its normal fixed-mode placement is preserved:
 *
 *   - native 600x178 surface;
 *   - centered horizontally within the 704x462 logical desktop;
 *   - six logical pixels above the bottom edge.
 *
 * The remote framebuffer remains authoritative and is never modified to draw
 * local UI.
 *
 * Context:
 *   GitHub Issue #39;
 *   K4 local foreground/generation owner;
 *   K7B clean historical OSK renderer;
 *   historical B4A local-overlay placement evidence.
 */

#ifndef PSTVNC_LOCAL_UI_PRESENTATION_H
#define PSTVNC_LOCAL_UI_PRESENTATION_H

#include <stddef.h>
#include <stdint.h>

#include "local_ui.h"
#include "osk.h"
#include "osk_render.h"

typedef struct pstvnc_local_ui_presentation {
    int overlay_visible;

    const uint16_t *overlay_pixels;
    size_t overlay_pixel_count;

    unsigned int overlay_width;
    unsigned int overlay_height;

    unsigned int overlay_x;
    unsigned int overlay_y;

    uint32_t generation;
} pstvnc_local_ui_presentation_t;

/*
 * Prepare the visual description for one exact local-UI generation.
 *
 * DESKTOP foreground returns a valid presentation with no local overlay.
 *
 * OSK foreground renders the current keyboard into caller-owned surface
 * storage and describes its fixed-mode bottom-centered placement.
 *
 * No state is acknowledged as presented here. Application/main may call
 * pstvnc_local_ui_mark_presented() only after the platform presentation
 * boundary reports success.
 */
int pstvnc_local_ui_prepare_presentation(
    const pstvnc_local_ui_t *ui,
    const pstvnc_osk_t *osk,
    uint16_t *overlay_surface_pixels,
    size_t overlay_surface_capacity_pixels,
    pstvnc_local_ui_presentation_t *presentation);

#endif /* PSTVNC_LOCAL_UI_PRESENTATION_H */
