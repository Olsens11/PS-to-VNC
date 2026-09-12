/*
 * File synopsis:
 * Defines the experiment-local single-owner RFB/MPEG/local-UI compositor used
 * by the CP2P integration candidate.
 *
 * The implementation wraps the unchanged through-Issue-39 PS2 graphics owner.
 * MPEG submits decoded RGB16 macroblocks plus the already-prepared calibration
 * start contract; callers never take direct GIF/GS ownership.
 */
#ifndef PSTVNC_H1_CUMULATIVE39_GRAPHICS_H
#define PSTVNC_H1_CUMULATIVE39_GRAPHICS_H

#include "mpeg_presentation_calibration/h1_mpeg_start_handoff.h"

#include <stdint.h>

int pstvnc_h1_graphics_present_video_macroblocks(
    const uint16_t *macroblock_pixels,
    unsigned int source_width,
    unsigned int source_height,
    const pstvnc_h1_mpeg_start_contract_t *contract);

int pstvnc_h1_graphics_clear_video(void);

#endif /* PSTVNC_H1_CUMULATIVE39_GRAPHICS_H */
