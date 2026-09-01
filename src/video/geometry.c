/*
 * File synopsis:
 * Implements pure safe-area and presented-rectangle validation/resolution
 * without touching GS state or selecting live policy.
 *
 * Context: docs/CLEAN_ARCHITECTURE.md, "Display model and presentation" and
 * "Stable cross-domain values".
 */

#include "video/geometry.h"

#include <limits.h>
#include <stddef.h>

static int pstvnc_video_geometry_rect_fits(
    const pstvnc_video_rect_t *rect,
    unsigned int bounds_width,
    unsigned int bounds_height
)
{
    if (rect == NULL)
        return 0;

    if (rect->width == 0u || rect->height == 0u)
        return 0;

    if (rect->x > bounds_width ||
        rect->y > bounds_height)
        return 0;

    if (rect->width > bounds_width - rect->x ||
        rect->height > bounds_height - rect->y)
        return 0;

    return 1;
}

int pstvnc_video_geometry_safe_area_valid(
    const pstvnc_video_mode_t *mode,
    int width,
    int height,
    int offset_x,
    int offset_y
)
{
    int output_width;
    int output_height;
    int remaining_x;
    int remaining_y;
    int centered_x;
    int centered_y;
    int max_positive_x;
    int max_positive_y;

    if (mode == NULL)
        return 0;

    /*
     * Current PS-to-VNC modes are far below INT_MAX.  Keep the permanent
     * arithmetic explicit rather than relying on an implementation-defined
     * unsigned-to-signed conversion for a future malformed descriptor.
     */
    if (mode->raster_width > (unsigned int)INT_MAX ||
        mode->raster_height > (unsigned int)INT_MAX)
        return 0;

    output_width = (int)mode->raster_width;
    output_height = (int)mode->raster_height;

    if (width <= 0 || height <= 0)
        return 0;

    if (width > output_width ||
        height > output_height)
        return 0;

    remaining_x = output_width - width;
    remaining_y = output_height - height;

    centered_x = remaining_x / 2;
    centered_y = remaining_y / 2;

    /*
     * With an odd remainder, integer centering leaves the extra pixel on
     * the positive side.  This is the historical PS2VNC acceptance rule.
     */
    max_positive_x = remaining_x - centered_x;
    max_positive_y = remaining_y - centered_y;

    if (offset_x < -centered_x ||
        offset_x > max_positive_x)
        return 0;

    if (offset_y < -centered_y ||
        offset_y > max_positive_y)
        return 0;

    return 1;
}

int pstvnc_video_geometry_safe_area_rect(
    const pstvnc_video_mode_t *mode,
    int width,
    int height,
    int offset_x,
    int offset_y,
    pstvnc_video_rect_t *out_rect
)
{
    int x;
    int y;
    pstvnc_video_rect_t rect;

    if (mode == NULL || out_rect == NULL)
        return -1;

    if (!pstvnc_video_geometry_safe_area_valid(
            mode,
            width,
            height,
            offset_x,
            offset_y))
        return -1;

    x =
        ((int)mode->raster_width - width) / 2 +
        offset_x;

    y =
        ((int)mode->raster_height - height) / 2 +
        offset_y;

    if (x < 0 || y < 0)
        return -1;

    rect.x = (unsigned int)x;
    rect.y = (unsigned int)y;
    rect.width = (unsigned int)width;
    rect.height = (unsigned int)height;

    if (!pstvnc_video_geometry_rect_fits(
            &rect,
            mode->raster_width,
            mode->raster_height))
        return -1;

    *out_rect = rect;
    return 0;
}

int pstvnc_video_geometry_presented_rect(
    const pstvnc_video_mode_t *mode,
    const pstvnc_video_rect_t *safe_area,
    unsigned int desktop_capacity_width,
    unsigned int desktop_capacity_height,
    pstvnc_video_rect_t *out_rect
)
{
    pstvnc_video_rect_t result;

    if (mode == NULL ||
        safe_area == NULL ||
        out_rect == NULL)
        return -1;

    if (desktop_capacity_width == 0u ||
        desktop_capacity_height == 0u)
        return -1;

    if (!pstvnc_video_geometry_rect_fits(
            safe_area,
            mode->raster_width,
            mode->raster_height))
        return -1;

    result.width =
        (safe_area->width < desktop_capacity_width) ?
        safe_area->width :
        desktop_capacity_width;

    result.height =
        (safe_area->height < desktop_capacity_height) ?
        safe_area->height :
        desktop_capacity_height;

    result.x =
        safe_area->x +
        ((safe_area->width - result.width) / 2u);

    result.y =
        safe_area->y +
        ((safe_area->height - result.height) / 2u);

    if (!pstvnc_video_geometry_rect_fits(
            &result,
            mode->raster_width,
            mode->raster_height))
        return -1;

    *out_rect = result;
    return 0;
}
