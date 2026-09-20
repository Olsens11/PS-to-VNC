/*
 * File synopsis:
 * Implements the platform-neutral MPEG CALIBRATION geometry and foreground
 * state owner used by A004 reconstruction.
 *
 * Geometry editing preserves the recovered H1 meanings: macroblock-aligned
 * base dimensions with pixel-precision placement, presentation-local inner
 * matte extents, and an outer/suppression footprint. Accept/cancel and held-X
 * review safety are local state only. This file performs no RFB, Transport,
 * decoder, Pi, GS, media-clock, or DESKTOP CALIBRATION work.
 *
 * Context:
 *   docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md;
 *   docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md.
 */

#include "mpeg_calibration.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

static int32_t clamp_i64(
    int64_t value,
    int32_t minimum,
    int32_t maximum)
{
    if (value < minimum)
        return minimum;

    if (value > maximum)
        return maximum;

    return (int32_t)value;
}

static int region_is_valid(
    const pstvnc_mpeg_calibration_t *calibration,
    const pstvnc_mpeg_calibration_region_t *region)
{
    if (calibration == NULL ||
        region == NULL ||
        calibration->canvas_width < 16 ||
        calibration->canvas_height < 16 ||
        region->width < 16 ||
        region->height < 16 ||
        (region->width & 15) != 0 ||
        (region->height & 15) != 0 ||
        region->width > calibration->canvas_width ||
        region->height > calibration->canvas_height ||
        region->x < 0 ||
        region->y < 0 ||
        region->x > calibration->canvas_width - region->width ||
        region->y > calibration->canvas_height - region->height ||
        region->inner_matte_x < 0 ||
        region->inner_matte_y < 0 ||
        region->inner_matte_x > region->width / 2 ||
        region->inner_matte_y > region->height / 2 ||
        region->outer_matte_x < 0 ||
        region->outer_matte_y < 0 ||
        region->outer_matte_x > calibration->canvas_width ||
        region->outer_matte_y > calibration->canvas_height)
        return 0;

    return 1;
}

static void clamp_current_region(
    pstvnc_mpeg_calibration_t *calibration)
{
    pstvnc_mpeg_calibration_region_t *region;
    int32_t maximum_width;
    int32_t maximum_height;

    region = &calibration->current;
    maximum_width = calibration->canvas_width & ~15;
    maximum_height = calibration->canvas_height & ~15;

    region->width = clamp_i64(region->width, 16, maximum_width);
    region->height = clamp_i64(region->height, 16, maximum_height);

    region->width &= ~15;
    region->height &= ~15;

    region->x = clamp_i64(
        region->x,
        0,
        calibration->canvas_width - region->width);

    region->y = clamp_i64(
        region->y,
        0,
        calibration->canvas_height - region->height);

    region->inner_matte_x = clamp_i64(
        region->inner_matte_x,
        0,
        region->width / 2);

    region->inner_matte_y = clamp_i64(
        region->inner_matte_y,
        0,
        region->height / 2);

    region->outer_matte_x = clamp_i64(
        region->outer_matte_x,
        0,
        calibration->canvas_width);

    region->outer_matte_y = clamp_i64(
        region->outer_matte_y,
        0,
        calibration->canvas_height);
}

static void edit_current_region(
    pstvnc_mpeg_calibration_t *calibration,
    const pstvnc_mpeg_calibration_input_t *input)
{
    pstvnc_mpeg_calibration_region_t *region;
    int horizontal;
    int vertical;

    region = &calibration->current;

    horizontal =
        ((input->down & PSTVNC_MPEG_CALIBRATION_RIGHT) != 0) -
        ((input->down & PSTVNC_MPEG_CALIBRATION_LEFT) != 0);

    vertical =
        ((input->down & PSTVNC_MPEG_CALIBRATION_DOWN) != 0) -
        ((input->down & PSTVNC_MPEG_CALIBRATION_UP) != 0);

    if ((input->down & PSTVNC_MPEG_CALIBRATION_MOVE) != 0) {
        region->x = clamp_i64(
            (int64_t)region->x + horizontal,
            INT32_MIN,
            INT32_MAX);

        region->y = clamp_i64(
            (int64_t)region->y + vertical,
            INT32_MIN,
            INT32_MAX);

        clamp_current_region(calibration);
        return;
    }

    if ((input->down & PSTVNC_MPEG_CALIBRATION_INNER_MATTE) != 0) {
        region->inner_matte_x = clamp_i64(
            (int64_t)region->inner_matte_x + horizontal,
            INT32_MIN,
            INT32_MAX);

        region->inner_matte_y = clamp_i64(
            (int64_t)region->inner_matte_y + horizontal + vertical,
            INT32_MIN,
            INT32_MAX);

        clamp_current_region(calibration);
        return;
    }

    if ((input->down & PSTVNC_MPEG_CALIBRATION_OUTER_MATTE) != 0) {
        region->outer_matte_x = clamp_i64(
            (int64_t)region->outer_matte_x - horizontal,
            INT32_MIN,
            INT32_MAX);

        region->outer_matte_y = clamp_i64(
            (int64_t)region->outer_matte_y - horizontal - vertical,
            INT32_MIN,
            INT32_MAX);

        clamp_current_region(calibration);
        return;
    }

    if (horizontal != 0) {
        int32_t new_width;
        int64_t center_twice;

        center_twice =
            (int64_t)region->x * 2 + region->width;

        new_width = clamp_i64(
            (int64_t)region->width + (int64_t)horizontal * 16,
            16,
            calibration->canvas_width & ~15);

        new_width &= ~15;
        region->width = new_width;
        region->x = clamp_i64(
            (center_twice - new_width) / 2,
            INT32_MIN,
            INT32_MAX);
    }

    if (vertical != 0) {
        int32_t new_height;
        int64_t center_twice;

        center_twice =
            (int64_t)region->y * 2 + region->height;

        new_height = clamp_i64(
            (int64_t)region->height - (int64_t)vertical * 16,
            16,
            calibration->canvas_height & ~15);

        new_height &= ~15;
        region->height = new_height;
        region->y = clamp_i64(
            (center_twice - new_height) / 2,
            INT32_MIN,
            INT32_MAX);
    }

    clamp_current_region(calibration);
}

int pstvnc_mpeg_calibration_init(
    pstvnc_mpeg_calibration_t *calibration,
    int32_t canvas_width,
    int32_t canvas_height)
{
    int32_t default_width;
    int32_t default_height;

    if (calibration == NULL ||
        canvas_width < 16 ||
        canvas_height < 16)
        return 0;

    default_width = (canvas_width / 2) & ~15;
    default_height = (canvas_height / 2) & ~15;

    if (default_width < 16)
        default_width = 16;

    if (default_height < 16)
        default_height = 16;

    memset(calibration, 0, sizeof(*calibration));

    calibration->canvas_width = canvas_width;
    calibration->canvas_height = canvas_height;

    calibration->defaults.x =
        (canvas_width - default_width) / 2;

    calibration->defaults.y =
        (canvas_height - default_height) / 2;

    calibration->defaults.width = default_width;
    calibration->defaults.height = default_height;

    calibration->current = calibration->defaults;
    calibration->committed = calibration->defaults;
    calibration->screen = PSTVNC_MPEG_CALIBRATION_INACTIVE;

    return 1;
}

int pstvnc_mpeg_calibration_begin(
    pstvnc_mpeg_calibration_t *calibration)
{
    if (calibration == NULL ||
        calibration->screen != PSTVNC_MPEG_CALIBRATION_INACTIVE ||
        calibration->release_quarantine_pending)
        return 0;

    calibration->current =
        calibration->has_committed ?
            calibration->committed :
            calibration->defaults;

    calibration->review_accept_armed = 0;
    calibration->screen = PSTVNC_MPEG_CALIBRATION_EDIT;
    return 1;
}

int pstvnc_mpeg_calibration_update(
    pstvnc_mpeg_calibration_t *calibration,
    const pstvnc_mpeg_calibration_input_t *input,
    pstvnc_mpeg_calibration_effects_t *effects)
{
    if (calibration == NULL ||
        input == NULL ||
        effects == NULL ||
        (input->down & ~PSTVNC_MPEG_CALIBRATION_ALL_ACTIONS) != 0 ||
        (input->pressed & ~PSTVNC_MPEG_CALIBRATION_ALL_ACTIONS) != 0 ||
        (input->pressed & ~input->down) != 0)
        return 0;

    memset(effects, 0, sizeof(*effects));

    if (calibration->release_quarantine_pending) {
        if ((input->down & PSTVNC_MPEG_CALIBRATION_ALL_ACTIONS) == 0) {
            calibration->release_quarantine_pending = 0;
            effects->release_quarantine_complete = 1;
        }

        return 1;
    }

    if (calibration->screen == PSTVNC_MPEG_CALIBRATION_INACTIVE)
        return 1;

    if (calibration->screen == PSTVNC_MPEG_CALIBRATION_REVIEW) {
        if ((input->down & PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT) == 0)
            calibration->review_accept_armed = 1;

        if ((input->pressed & PSTVNC_MPEG_CALIBRATION_CANCEL) != 0) {
            calibration->screen = PSTVNC_MPEG_CALIBRATION_EDIT;
            calibration->review_accept_armed = 0;
            return 1;
        }

        if (calibration->review_accept_armed &&
            (input->pressed & PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT) != 0) {
            if (!region_is_valid(calibration, &calibration->current))
                return 0;

            calibration->committed = calibration->current;
            calibration->has_committed = 1;
            calibration->screen = PSTVNC_MPEG_CALIBRATION_INACTIVE;
            calibration->review_accept_armed = 0;
            calibration->release_quarantine_pending = 1;

            effects->accepted = 1;
            effects->accepted_region = calibration->committed;
        }

        return 1;
    }

    if ((input->pressed & PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT) != 0) {
        calibration->screen = PSTVNC_MPEG_CALIBRATION_REVIEW;
        calibration->review_accept_armed = 0;
        return 1;
    }

    if ((input->pressed & PSTVNC_MPEG_CALIBRATION_CANCEL) != 0) {
        calibration->current =
            calibration->has_committed ?
                calibration->committed :
                calibration->defaults;

        calibration->screen = PSTVNC_MPEG_CALIBRATION_INACTIVE;
        calibration->review_accept_armed = 0;
        calibration->release_quarantine_pending = 1;
        effects->cancelled = 1;
        return 1;
    }

    if ((input->pressed & PSTVNC_MPEG_CALIBRATION_RESET) != 0) {
        calibration->current = calibration->defaults;
        return 1;
    }

    edit_current_region(calibration, input);

    return region_is_valid(calibration, &calibration->current);
}

int pstvnc_mpeg_calibration_owns_foreground(
    const pstvnc_mpeg_calibration_t *calibration)
{
    return
        calibration != NULL &&
        (calibration->screen != PSTVNC_MPEG_CALIBRATION_INACTIVE ||
         calibration->release_quarantine_pending);
}

int pstvnc_mpeg_calibration_resolve_geometry(
    const pstvnc_mpeg_calibration_t *calibration,
    const pstvnc_mpeg_calibration_region_t *region,
    pstvnc_mpeg_calibration_geometry_t *geometry)
{
    int64_t right;
    int64_t bottom;
    int32_t left;
    int32_t top;

    if (geometry == NULL ||
        !region_is_valid(calibration, region))
        return 0;

    geometry->base.x = region->x;
    geometry->base.y = region->y;
    geometry->base.width = region->width;
    geometry->base.height = region->height;

    geometry->inner_content.x =
        region->x + region->inner_matte_x;

    geometry->inner_content.y =
        region->y + region->inner_matte_y;

    geometry->inner_content.width =
        region->width - region->inner_matte_x * 2;

    geometry->inner_content.height =
        region->height - region->inner_matte_y * 2;

    left = clamp_i64(
        (int64_t)region->x - region->outer_matte_x,
        0,
        calibration->canvas_width);

    top = clamp_i64(
        (int64_t)region->y - region->outer_matte_y,
        0,
        calibration->canvas_height);

    right =
        (int64_t)region->x +
        region->width +
        region->outer_matte_x;

    bottom =
        (int64_t)region->y +
        region->height +
        region->outer_matte_y;

    geometry->suppression.x = left;
    geometry->suppression.y = top;

    geometry->suppression.width =
        clamp_i64(right, 0, calibration->canvas_width) - left;

    geometry->suppression.height =
        clamp_i64(bottom, 0, calibration->canvas_height) - top;

    return 1;
}
