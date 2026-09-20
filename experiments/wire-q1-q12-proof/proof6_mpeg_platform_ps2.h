/*
 * File synopsis:
 * Proof 6 PS2-only adapter between the clean synchronous MPEG decoder owner and
 * the already hardware-characterized PS2SDK libmpeg/IPU/GS mechanism.
 *
 * This is development apparatus, not final Presentation architecture. It
 * deliberately keeps libmpeg, DMA and GS details outside src/mpeg/decoder.c so
 * Proof 6 can exercise the clean decoder and current product Transport unchanged.
 */

#ifndef PSTVNC_WIRE_PROOF6_MPEG_PLATFORM_PS2_H
#define PSTVNC_WIRE_PROOF6_MPEG_PLATFORM_PS2_H

#include "mpeg/decoder.h"

#include <stdint.h>

typedef struct proof6_mpeg_platform {
    pstvnc_mpeg_decoder_t *decoder;
    void *state;
    volatile uint32_t pictures_presented;
    volatile int in_mpeg_picture;
    int graphics_ready;
} proof6_mpeg_platform_t;

int proof6_mpeg_platform_context_initialize(
    proof6_mpeg_platform_t *platform,
    pstvnc_mpeg_decoder_t *decoder);

void proof6_mpeg_platform_make_ops(
    proof6_mpeg_platform_t *platform,
    pstvnc_mpeg_decoder_platform_ops_t *ops);

void proof6_mpeg_platform_show_solid(
    proof6_mpeg_platform_t *platform,
    int red,
    int green,
    int blue);

void proof6_mpeg_platform_context_release(
    proof6_mpeg_platform_t *platform);

#endif
