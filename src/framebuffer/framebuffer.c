/*
 * File synopsis:
 * Implements the authoritative CPU-side desktop image: geometry, validity,
 * rectangle writes, and conservative dirty bounds.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Owned authoritative
 * framebuffer"; docs/CLEAN_ARCHITECTURE.md, "Desktop framebuffer".
 */

#include <string.h>

#include "framebuffer.h"

static int rect_fits(
    const pstvnc_framebuffer_t *framebuffer,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height)
{
    uint32_t right;
    uint32_t bottom;

    if (framebuffer == NULL || width == 0 || height == 0)
        return 0;

    /*
     * Widen before addition. Wire coordinates are 16-bit, so performing these
     * sums in that type could wrap a malformed rectangle back into bounds.
     */
    right = (uint32_t)x + (uint32_t)width;
    bottom = (uint32_t)y + (uint32_t)height;

    return right <= framebuffer->width &&
           bottom <= framebuffer->height;
}

static void include_dirty(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height)
{
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
    uint32_t current_right;
    uint32_t current_bottom;

    /*
     * Dirty state summarizes the most recently decoded server update. The
     * bounding box may include untouched pixels between rectangles; it is a
     * conservative presentation hint, never evidence that those pixels were
     * received in this update.
     */
    if (!framebuffer->dirty) {
        framebuffer->dirty_rect.x = x;
        framebuffer->dirty_rect.y = y;
        framebuffer->dirty_rect.width = width;
        framebuffer->dirty_rect.height = height;
        framebuffer->dirty = 1;
        return;
    }

    left = x < framebuffer->dirty_rect.x ?
        x : framebuffer->dirty_rect.x;
    top = y < framebuffer->dirty_rect.y ?
        y : framebuffer->dirty_rect.y;
    right = (uint32_t)x + width;
    bottom = (uint32_t)y + height;
    current_right = (uint32_t)framebuffer->dirty_rect.x +
        framebuffer->dirty_rect.width;
    current_bottom = (uint32_t)framebuffer->dirty_rect.y +
        framebuffer->dirty_rect.height;

    if (current_right > right)
        right = current_right;
    if (current_bottom > bottom)
        bottom = current_bottom;

    framebuffer->dirty_rect.x = (uint16_t)left;
    framebuffer->dirty_rect.y = (uint16_t)top;
    framebuffer->dirty_rect.width = (uint16_t)(right - left);
    framebuffer->dirty_rect.height = (uint16_t)(bottom - top);
}

int pstvnc_framebuffer_init(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t *pixels,
    size_t pixel_capacity)
{
    if (framebuffer == NULL || pixels == NULL || pixel_capacity == 0)
        return 0;

    memset(framebuffer, 0, sizeof(*framebuffer));
    framebuffer->pixels = pixels;
    framebuffer->pixel_capacity = pixel_capacity;
    return 1;
}

int pstvnc_framebuffer_set_geometry(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t width,
    uint16_t height)
{
    size_t required;

    if (framebuffer == NULL || framebuffer->pixels == NULL ||
        width == 0 || height == 0)
        return 0;

    required = (size_t)width * (size_t)height;
    if (required > framebuffer->pixel_capacity)
        return 0;

    framebuffer->width = width;
    framebuffer->height = height;

    /*
     * Geometry changes revoke pixel authority. Capacity alone proves that the
     * storage is safe to address; only the RFB startup coverage proof may later
     * establish that every logical pixel contains current desktop data.
     */
    pstvnc_framebuffer_invalidate(framebuffer);
    return 1;
}

size_t pstvnc_framebuffer_pixel_count(
    const pstvnc_framebuffer_t *framebuffer)
{
    if (framebuffer == NULL)
        return 0;

    return (size_t)framebuffer->width * framebuffer->height;
}

void pstvnc_framebuffer_invalidate(
    pstvnc_framebuffer_t *framebuffer)
{
    if (framebuffer == NULL)
        return;

    framebuffer->valid = 0;
    pstvnc_framebuffer_clear_dirty(framebuffer);
}

int pstvnc_framebuffer_mark_valid(
    pstvnc_framebuffer_t *framebuffer)
{
    if (framebuffer == NULL || framebuffer->pixels == NULL ||
        framebuffer->width == 0 || framebuffer->height == 0)
        return 0;

    /*
     * This function records a proof made by the owning session; it does not
     * attempt to infer completeness from dirty state or byte counts.
     */
    framebuffer->valid = 1;
    return 1;
}

int pstvnc_framebuffer_write_rect(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    const uint16_t *source,
    size_t source_stride)
{
    uint16_t row;

    if (framebuffer == NULL || framebuffer->pixels == NULL ||
        source == NULL || source_stride < width ||
        !rect_fits(framebuffer, x, y, width, height))
        return 0;

    for (row = 0; row < height; row++) {
        uint16_t *destination = &framebuffer->pixels[
            ((size_t)y + row) * framebuffer->width + x
        ];
        const uint16_t *source_row = &source[
            (size_t)row * source_stride
        ];

        memcpy(
            destination,
            source_row,
            (size_t)width * sizeof(*source)
        );
    }

    /*
     * Pixel mutation and whole-frame authority are intentionally separate.
     * Startup decoding writes rows while valid is still false; the session
     * promotes authority only after the complete update passes all checks.
     */
    include_dirty(framebuffer, x, y, width, height);
    return 1;
}

void pstvnc_framebuffer_clear_dirty(
    pstvnc_framebuffer_t *framebuffer)
{
    if (framebuffer == NULL)
        return;

    framebuffer->dirty = 0;
    memset(
        &framebuffer->dirty_rect,
        0,
        sizeof(framebuffer->dirty_rect)
    );
}

int pstvnc_framebuffer_get_dirty(
    const pstvnc_framebuffer_t *framebuffer,
    pstvnc_framebuffer_rect_t *rect)
{
    if (framebuffer == NULL || rect == NULL || !framebuffer->dirty)
        return 0;

    *rect = framebuffer->dirty_rect;
    return 1;
}
