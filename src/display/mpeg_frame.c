/*
 * File synopsis:
 * Implements validation and deterministic detiling for one neutral decoded
 * MPEG RGB16 16x16-macroblock surface.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-SHARED-COMPOSITOR-FIRST-SYNC-R4.
 */

#include "mpeg_frame.h"

#include <stddef.h>
#include <string.h>

int pstvnc_mpeg_rgb16_macroblock_surface_valid(
    const pstvnc_mpeg_rgb16_macroblock_surface_t *surface)
{
    size_t required_pixels;

    if (surface == NULL ||
        surface->pixels == NULL ||
        surface->width == 0u ||
        surface->height == 0u ||
        surface->width > PSTVNC_MPEG_RGB16_MAX_WIDTH ||
        surface->height > PSTVNC_MPEG_RGB16_MAX_HEIGHT ||
        (surface->width & (PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE - 1u)) != 0u ||
        (surface->height & (PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE - 1u)) != 0u)
        return 0;

    required_pixels =
        (size_t)surface->width *
        (size_t)surface->height;

    return surface->pixel_capacity >= required_pixels;
}

int pstvnc_mpeg_rgb16_detile(
    const pstvnc_mpeg_rgb16_macroblock_surface_t *surface,
    uint16_t *linear_pixels,
    size_t linear_capacity_pixels)
{
    unsigned int macroblock_columns;
    unsigned int macroblock_rows;
    unsigned int macroblock_y;
    unsigned int macroblock_x;
    size_t required_pixels;

    if (!pstvnc_mpeg_rgb16_macroblock_surface_valid(surface) ||
        linear_pixels == NULL ||
        linear_pixels == surface->pixels)
        return 0;

    required_pixels =
        (size_t)surface->width *
        (size_t)surface->height;

    if (linear_capacity_pixels < required_pixels)
        return 0;

    macroblock_columns =
        surface->width /
        PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE;

    macroblock_rows =
        surface->height /
        PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE;

    for (macroblock_y = 0u;
         macroblock_y < macroblock_rows;
         ++macroblock_y) {

        for (macroblock_x = 0u;
             macroblock_x < macroblock_columns;
             ++macroblock_x) {

            const uint16_t *block =
                surface->pixels +
                (((size_t)macroblock_y * macroblock_columns +
                  macroblock_x) *
                 PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE *
                 PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE);

            unsigned int row;

            for (row = 0u;
                 row < PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE;
                 ++row) {

                uint16_t *destination =
                    linear_pixels +
                    (((size_t)macroblock_y *
                      PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE +
                      row) *
                     surface->width) +
                    ((size_t)macroblock_x *
                     PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE);

                memcpy(
                    destination,
                    block +
                        (size_t)row *
                        PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE,
                    PSTVNC_MPEG_RGB16_MACROBLOCK_SIZE *
                        sizeof(uint16_t));
            }
        }
    }

    return 1;
}
