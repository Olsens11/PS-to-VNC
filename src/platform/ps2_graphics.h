/*
 * File synopsis:
 * Declares the single PS2 graphics/dmaKit owner for ordinary desktop/local-UI
 * presentation plus the A004 neutral MPEG macroblock compositor mechanism.
 *
 * Platform owns gsKit, VRAM, detile destination storage, layer submission,
 * synchronized flips, and the post-sync PS2 timer observation. It deliberately
 * does not own MPEG run/generation identity, Presentation lifecycle, media-clock
 * state, calibration, RFB truth, or Application orchestration.
 *
 * Context:
 *   docs/reconstruction/ISSUE7_MINIMAL_CORE.md;
 *   docs/CLEAN_ARCHITECTURE.md, "PS2 platform mechanisms";
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-SHARED-COMPOSITOR-FIRST-SYNC-R4.
 */

#ifndef PSTVNC_PS2_GRAPHICS_H
#define PSTVNC_PS2_GRAPHICS_H

#include <stddef.h>
#include <stdint.h>

#include "display/mpeg_frame.h"

typedef struct pstvnc_ps2_graphics_overlay {
    const uint16_t *pixels;
    size_t pixel_count;

    unsigned int width;
    unsigned int height;

    unsigned int x;
    unsigned int y;
} pstvnc_ps2_graphics_overlay_t;

typedef struct pstvnc_ps2_graphics_rect {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} pstvnc_ps2_graphics_rect_t;

typedef struct pstvnc_ps2_graphics_video {
    pstvnc_mpeg_rgb16_macroblock_surface_t surface;
    pstvnc_ps2_graphics_rect_t base;
    pstvnc_ps2_graphics_rect_t inner_content;
    pstvnc_ps2_graphics_rect_t suppression;
} pstvnc_ps2_graphics_video_t;

typedef struct pstvnc_ps2_graphics_sync_result {
    int synchronized;
    uint64_t observed_sync_tick;
    uint32_t ticks_per_second;
} pstvnc_ps2_graphics_sync_result_t;

/* Initialize DMAKit and the fixed hardware-proven Standard 480p GS instance. */
int pstvnc_ps2_graphics_init(void);

/*
 * Present the latest complete logical desktop and optional local overlay.
 *
 * If an MPEG surface has previously been installed through
 * pstvnc_ps2_graphics_present_video_macroblocks(), it remains composited above
 * the refreshed desktop and below the local overlay. This permits authoritative
 * RFB truth to advance without revealing pixels inside Presentation's retained
 * suppression footprint.
 */
int pstvnc_ps2_graphics_present(
    const uint16_t *desktop_pixels,
    size_t desktop_pixel_count,
    const pstvnc_ps2_graphics_overlay_t *local_overlay);

/*
 * Install one validated RGB16 macroblock frame into the bounded reusable video
 * resource and present the complete cached desktop/video/local-overlay stack.
 *
 * Success is returned only after gsKit_sync_flip() and a GetTimerSystemTime()
 * observation immediately after that synchronized boundary. The timer result is
 * in kBUSCLK ticks; callers must match that tick domain before media-clock arm.
 *
 * This operation carries no run generation and performs no lifecycle promotion.
 */
int pstvnc_ps2_graphics_present_video_macroblocks(
    const pstvnc_ps2_graphics_video_t *video,
    pstvnc_ps2_graphics_sync_result_t *sync_result);

void pstvnc_ps2_graphics_shutdown(void);

#endif /* PSTVNC_PS2_GRAPHICS_H */
