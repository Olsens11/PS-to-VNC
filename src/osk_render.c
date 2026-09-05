/*
 * File synopsis:
 * Reconstructs the adopted historical OSK appearance into a caller-owned
 * platform-neutral local presentation surface.
 *
 * Historical visual facts retained here are product assets/invariants:
 *
 *   - the 5x7 glyph shapes;
 *   - panel/key/selection/active-state colors;
 *   - border treatment;
 *   - five-row spacing;
 *   - proportional per-row key widths;
 *   - centered label sizing;
 *   - persistent ABC/FUNC/modifier indication.
 *
 * Historical globals, platform texture objects, drawing ownership, controller
 * coupling, and wakeup mechanisms are deliberately not retained.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "osk_render.h"

typedef struct pstvnc_osk_font_glyph {
    unsigned char character;
    unsigned char rows[7];
} pstvnc_osk_font_glyph_t;

static const pstvnc_osk_font_glyph_t osk_font_glyphs[] = {
    { ' ', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { 'A', { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'B', { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E } },
    { 'C', { 0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F } },
    { 'D', { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E } },
    { 'E', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F } },
    { 'F', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 } },
    { 'G', { 0x0F, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0F } },
    { 'H', { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'I', { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F } },
    { 'J', { 0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C } },
    { 'K', { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 } },
    { 'L', { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F } },
    { 'M', { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 } },
    { 'N', { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 } },
    { 'O', { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'P', { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 } },
    { 'Q', { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D } },
    { 'R', { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 } },
    { 'S', { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E } },
    { 'T', { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 'U', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'V', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 } },
    { 'W', { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A } },
    { 'X', { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 } },
    { 'Y', { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 } },
    { 'Z', { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F } },
    { '0', { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E } },
    { '1', { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { '2', { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F } },
    { '3', { 0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E } },
    { '4', { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 } },
    { '5', { 0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E } },
    { '6', { 0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E } },
    { '7', { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 } },
    { '8', { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E } },
    { '9', { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E } },
    { '-', { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 } },
    { '=', { 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00 } },
    { ';', { 0x00, 0x04, 0x00, 0x04, 0x04, 0x08, 0x00 } },
    { ',', { 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08 } },
    { '.', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06 } },
    { '/', { 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10 } },
    { '_', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F } },
    { '+', { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00 } },
    { ':', { 0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00 } },
    { '<', { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 } },
    { '>', { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 } },
    { '?', { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 } },
    { '!', { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 } },
    { '@', { 0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0E } },
    { '#', { 0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A } },
    { '$', { 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04 } },
    { '%', { 0x19, 0x1A, 0x04, 0x08, 0x16, 0x06, 0x00 } },
    { '^', { 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00 } },
    { '&', { 0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D } },
    { '*', { 0x00, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x00 } },
    { '(', { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 } },
    { ')', { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 } },
    { 'a', { 0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F } },
    { 'b', { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1E } },
    { 'c', { 0x00, 0x00, 0x0F, 0x10, 0x10, 0x10, 0x0F } },
    { 'd', { 0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x0F } },
    { 'e', { 0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0F } },
    { 'f', { 0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08 } },
    { 'g', { 0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x0E } },
    { 'h', { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11 } },
    { 'i', { 0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E } },
    { 'j', { 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C } },
    { 'k', { 0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12 } },
    { 'l', { 0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { 'm', { 0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x15 } },
    { 'n', { 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11 } },
    { 'o', { 0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E } },
    { 'p', { 0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10 } },
    { 'q', { 0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x01 } },
    { 'r', { 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10 } },
    { 's', { 0x00, 0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E } },
    { 't', { 0x08, 0x08, 0x1C, 0x08, 0x08, 0x09, 0x06 } },
    { 'u', { 0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D } },
    { 'v', { 0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04 } },
    { 'w', { 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A } },
    { 'x', { 0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11 } },
    { 'y', { 0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E } },
    { 'z', { 0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F } },
    { 0x5B, { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E } },
    { 0x5D, { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E } },
    { 0x5C, { 0x10, 0x08, 0x08, 0x04, 0x02, 0x02, 0x01 } },
    { 0x27, { 0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 } },
    { 0x22, { 0x0A, 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 } },
    { 0x7B, { 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02 } },
    { 0x7D, { 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08 } },
    { 0x7C, { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 0x60, { 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 } },
    { 0x7E, { 0x00, 0x00, 0x09, 0x16, 0x00, 0x00, 0x00 } },
};

static uint16_t pack_opaque_surface_color(
    unsigned int red,
    unsigned int green,
    unsigned int blue)
{
    return (uint16_t)(
        0x8000u |
        ((blue & 0x1fu) << 10) |
        ((green & 0x1fu) << 5) |
        (red & 0x1fu));
}

static void fill_surface_rectangle(
    uint16_t *surface_pixels,
    int x,
    int y,
    int width,
    int height,
    uint16_t color)
{
    int row_index;
    int column_index;

    if (surface_pixels == NULL)
        return;

    if (x < 0) {
        width += x;
        x = 0;
    }

    if (y < 0) {
        height += y;
        y = 0;
    }

    if (x + width > (int)PSTVNC_OSK_WIDTH)
        width = (int)PSTVNC_OSK_WIDTH - x;

    if (y + height > (int)PSTVNC_OSK_HEIGHT)
        height = (int)PSTVNC_OSK_HEIGHT - y;

    if (width <= 0 || height <= 0)
        return;

    for (row_index = y;
         row_index < y + height;
         row_index++) {

        uint16_t *destination =
            &surface_pixels[
                (size_t)row_index *
                (size_t)PSTVNC_OSK_WIDTH +
                (size_t)x];

        for (column_index = 0;
             column_index < width;
             column_index++) {

            destination[column_index] = color;
        }
    }
}

static const unsigned char *find_font_glyph(
    unsigned char character)
{
    size_t glyph_index;

    for (glyph_index = 0;
         glyph_index <
             sizeof(osk_font_glyphs) /
             sizeof(osk_font_glyphs[0]);
         glyph_index++) {

        if (osk_font_glyphs[glyph_index].character ==
            character) {

            return osk_font_glyphs[glyph_index].rows;
        }
    }

    return NULL;
}

static void draw_surface_character(
    uint16_t *surface_pixels,
    int x,
    int y,
    unsigned char character,
    int scale,
    uint16_t color)
{
    const unsigned char *glyph;
    int glyph_row;
    int glyph_column;

    if (surface_pixels == NULL || scale <= 0)
        return;

    glyph = find_font_glyph(character);

    if (glyph == NULL)
        return;

    for (glyph_row = 0;
         glyph_row < 7;
         glyph_row++) {

        for (glyph_column = 0;
             glyph_column < 5;
             glyph_column++) {

            if ((glyph[glyph_row] &
                 (1u << (4 - glyph_column))) != 0u) {

                fill_surface_rectangle(
                    surface_pixels,
                    x + glyph_column * scale,
                    y + glyph_row * scale,
                    scale,
                    scale,
                    color);
            }
        }
    }
}

static void draw_centered_surface_text(
    uint16_t *surface_pixels,
    int x,
    int y,
    int width,
    int height,
    const char *text,
    int scale,
    uint16_t color)
{
    int character_count;
    int character_width;
    int text_width;
    int text_height;
    int text_x;
    int text_y;
    int character_index;

    if (surface_pixels == NULL ||
        text == NULL ||
        scale <= 0)
        return;

    character_count = (int)strlen(text);

    if (character_count <= 0)
        return;

    character_width = 6 * scale;
    text_width =
        character_count * character_width -
        scale;
    text_height = 7 * scale;

    text_x =
        x +
        (width - text_width) / 2;

    text_y =
        y +
        (height - text_height) / 2;

    for (character_index = 0;
         character_index < character_count;
         character_index++) {

        draw_surface_character(
            surface_pixels,
            text_x +
                character_index *
                character_width,
            text_y,
            (unsigned char)text[character_index],
            scale,
            color);
    }
}

static int utility_key_is_active(
    const pstvnc_osk_t *osk,
    unsigned int column)
{
    if (osk == NULL)
        return 0;

    switch ((pstvnc_osk_utility_key_t)column) {
    case PSTVNC_OSK_UTILITY_ABC:
        return
            osk->page ==
            PSTVNC_OSK_PAGE_ABC;

    case PSTVNC_OSK_UTILITY_FUNC:
        return
            osk->page ==
            PSTVNC_OSK_PAGE_FUNC;

    case PSTVNC_OSK_UTILITY_SHIFT:
        return osk->shift ? 1 : 0;

    case PSTVNC_OSK_UTILITY_CTRL:
        return osk->ctrl ? 1 : 0;

    case PSTVNC_OSK_UTILITY_ALT:
        return osk->alt ? 1 : 0;

    default:
        return 0;
    }
}

int pstvnc_osk_render_surface(
    const pstvnc_osk_t *osk,
    uint16_t *surface_pixels,
    size_t surface_capacity_pixels)
{
    const uint16_t panel_color =
        pack_opaque_surface_color(
            3u, 3u, 4u);

    const uint16_t ordinary_key_color =
        pack_opaque_surface_color(
            10u, 10u, 11u);

    const uint16_t selected_key_color =
        pack_opaque_surface_color(
            31u, 24u, 3u);

    const uint16_t active_key_color =
        pack_opaque_surface_color(
            6u, 20u, 8u);

    const uint16_t border_color =
        pack_opaque_surface_color(
            22u, 22u, 22u);

    const uint16_t ordinary_text_color =
        pack_opaque_surface_color(
            31u, 31u, 31u);

    const uint16_t selected_text_color =
        pack_opaque_surface_color(
            1u, 1u, 1u);

    unsigned int row;
    unsigned int selected_row_length;

    if (osk == NULL ||
        surface_pixels == NULL ||
        surface_capacity_pixels <
            PSTVNC_OSK_SURFACE_PIXEL_COUNT ||
        (unsigned int)osk->page >=
            (unsigned int)PSTVNC_OSK_PAGE_COUNT ||
        osk->row >= PSTVNC_OSK_ROWS) {

        return 0;
    }

    selected_row_length =
        pstvnc_osk_row_length(
            osk->page,
            osk->row);

    if (selected_row_length == 0u ||
        osk->col >= selected_row_length)
        return 0;

    fill_surface_rectangle(
        surface_pixels,
        0,
        0,
        (int)PSTVNC_OSK_WIDTH,
        (int)PSTVNC_OSK_HEIGHT,
        panel_color);

    for (row = 0;
         row < PSTVNC_OSK_ROWS;
         row++) {

        const unsigned int key_count =
            pstvnc_osk_row_length(
                osk->page,
                row);

        const int margin = 6;
        const int gap = 3;

        const int available_width =
            (int)PSTVNC_OSK_WIDTH -
            2 * margin -
            ((int)key_count - 1) * gap;

        const int key_width =
            available_width /
            (int)key_count;

        const int used_width =
            key_width *
                (int)key_count +
            gap *
                ((int)key_count - 1);

        const int row_x =
            ((int)PSTVNC_OSK_WIDTH -
             used_width) /
            2;

        const int row_y =
            4 +
            (int)row * 33;

        const int key_height =
            33 - 4;

        unsigned int column;

        if (key_count == 0u)
            return 0;

        for (column = 0;
             column < key_count;
             column++) {

            const int key_x =
                row_x +
                (int)column *
                    (key_width + gap);

            const int selected =
                row == osk->row &&
                column == osk->col;

            uint16_t background_color =
                selected
                ? selected_key_color
                : ordinary_key_color;

            const uint16_t text_color =
                selected
                ? selected_text_color
                : ordinary_text_color;

            const char *label = NULL;
            char character_label[2];
            int text_scale;

            if (row ==
                PSTVNC_OSK_UTILITY_ROW) {

                label =
                    pstvnc_osk_key_label(
                        osk,
                        row,
                        column);

                if (!selected &&
                    utility_key_is_active(
                        osk,
                        column)) {

                    background_color =
                        active_key_color;
                }

            } else if (
                osk->page ==
                PSTVNC_OSK_PAGE_ABC) {

                character_label[0] =
                    pstvnc_osk_display_char(
                        osk,
                        row,
                        column);

                character_label[1] = '\0';
                label = character_label;

            } else {
                label =
                    pstvnc_osk_key_label(
                        osk,
                        row,
                        column);
            }

            if (label == NULL ||
                label[0] == '\0')
                return 0;

            text_scale =
                strlen(label) >= 5u
                ? 1
                : 2;

            fill_surface_rectangle(
                surface_pixels,
                key_x - 1,
                row_y - 1,
                key_width + 2,
                key_height + 2,
                border_color);

            fill_surface_rectangle(
                surface_pixels,
                key_x,
                row_y,
                key_width,
                key_height,
                background_color);

            draw_centered_surface_text(
                surface_pixels,
                key_x,
                row_y,
                key_width,
                key_height,
                label,
                text_scale,
                text_color);
        }
    }

    return 1;
}
