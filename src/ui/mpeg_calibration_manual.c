/*
 * File synopsis:
 * Implements the UI-owned A004 P8 manual MPEG CALIBRATION region source.
 *
 * Neutral controller facts are translated into the accepted P1 semantic action
 * vocabulary; P1 alone owns edit/review/accept/cancel geometry state. Visible
 * plans are resolved through P1's geometry API, and raster preparation writes
 * only a caller-owned CT16 surface over an immutable frozen-desktop underlay.
 *
 * This file deliberately contains no START+SELECT entry detector, hold timer,
 * libpad polling, DESKTOP CALIBRATION dependency, RFB/input-runtime operation,
 * Application/MPEG activation, Transport, Pi persistence, or Platform drawing.
 */

#include "mpeg_calibration_manual.h"

#include <stdint.h>
#include <string.h>

#define MANUAL_SELECTOR_SEGMENT 11
#define MANUAL_SELECTOR_THICKNESS 2
#define MANUAL_KIND_MARKER_WIDTH 18
#define MANUAL_KIND_MARKER_HEIGHT 5

static uint32_t map_controller_buttons(uint16_t buttons)
{
    uint32_t mapped = 0u;

    if ((buttons & PSTVNC_CONTROLLER_BUTTON_UP) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_UP;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_RIGHT) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_RIGHT;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_DOWN) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_DOWN;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_LEFT) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_LEFT;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_R1) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_MOVE;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_R2) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_INNER_MATTE;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_L2) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_OUTER_MATTE;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_START) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_RESET;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_CROSS) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT;
    if ((buttons & PSTVNC_CONTROLLER_BUTTON_CIRCLE) != 0u)
        mapped |= PSTVNC_MPEG_CALIBRATION_CANCEL;

    return mapped;
}

static int controller_state_valid(
    const pstvnc_controller_state_t *controller_state)
{
    if (controller_state == NULL)
        return 0;

    if ((controller_state->buttons_pressed &
            (uint16_t)~controller_state->buttons_down) != 0u)
        return 0;

    if ((controller_state->buttons_released &
            controller_state->buttons_down) != 0u)
        return 0;

    return 1;
}

static int rect_valid(
    const pstvnc_mpeg_calibration_rect_t *rect,
    int32_t canvas_width,
    int32_t canvas_height)
{
    int64_t right;
    int64_t bottom;

    if (rect == NULL ||
        canvas_width <= 0 ||
        canvas_height <= 0 ||
        rect->x < 0 ||
        rect->y < 0 ||
        rect->width <= 0 ||
        rect->height <= 0)
        return 0;

    right = (int64_t)rect->x + (int64_t)rect->width;
    bottom = (int64_t)rect->y + (int64_t)rect->height;

    return right <= canvas_width && bottom <= canvas_height;
}

static int rect_contains(
    const pstvnc_mpeg_calibration_rect_t *outer,
    const pstvnc_mpeg_calibration_rect_t *inner)
{
    int64_t outer_right;
    int64_t outer_bottom;
    int64_t inner_right;
    int64_t inner_bottom;

    if (outer == NULL || inner == NULL)
        return 0;

    outer_right = (int64_t)outer->x + outer->width;
    outer_bottom = (int64_t)outer->y + outer->height;
    inner_right = (int64_t)inner->x + inner->width;
    inner_bottom = (int64_t)inner->y + inner->height;

    return outer->x <= inner->x &&
        outer->y <= inner->y &&
        outer_right >= inner_right &&
        outer_bottom >= inner_bottom;
}

static int plan_valid(
    const pstvnc_mpeg_calibration_manual_plan_t *plan)
{
    if (plan == NULL ||
        (plan->kind != PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_EDIT &&
         plan->kind != PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_REVIEW) ||
        !rect_valid(
            &plan->geometry.base,
            plan->canvas_width,
            plan->canvas_height) ||
        !rect_valid(
            &plan->geometry.inner_content,
            plan->canvas_width,
            plan->canvas_height) ||
        !rect_valid(
            &plan->geometry.suppression,
            plan->canvas_width,
            plan->canvas_height) ||
        !rect_contains(
            &plan->geometry.base,
            &plan->geometry.inner_content) ||
        !rect_contains(
            &plan->geometry.suppression,
            &plan->geometry.base))
        return 0;

    return plan->candidate_region.x == plan->geometry.base.x &&
        plan->candidate_region.y == plan->geometry.base.y &&
        plan->candidate_region.width == plan->geometry.base.width &&
        plan->candidate_region.height == plan->geometry.base.height;
}

static int pixel_count(
    int32_t width,
    int32_t height,
    size_t *count)
{
    size_t w;
    size_t h;

    if (count == NULL || width <= 0 || height <= 0)
        return 0;

    w = (size_t)width;
    h = (size_t)height;

    if (w > SIZE_MAX / h)
        return 0;

    *count = w * h;
    return 1;
}

static int buffers_overlap(
    const uint16_t *left,
    const uint16_t *right,
    size_t count)
{
    uintptr_t left_begin;
    uintptr_t right_begin;
    uintptr_t left_end;
    uintptr_t right_end;
    size_t bytes;

    if (left == NULL || right == NULL ||
        count > SIZE_MAX / sizeof(*left))
        return 1;

    bytes = count * sizeof(*left);
    left_begin = (uintptr_t)left;
    right_begin = (uintptr_t)right;

    if (left_begin > UINTPTR_MAX - bytes ||
        right_begin > UINTPTR_MAX - bytes)
        return 1;

    left_end = left_begin + bytes;
    right_end = right_begin + bytes;

    return left_begin < right_end && right_begin < left_end;
}

static void fill_rect(
    uint16_t *pixels,
    int32_t canvas_width,
    const pstvnc_mpeg_calibration_rect_t *rect,
    uint16_t color)
{
    int32_t y;
    int32_t x;

    for (y = rect->y; y < rect->y + rect->height; y++) {
        for (x = rect->x; x < rect->x + rect->width; x++)
            pixels[(size_t)y * (size_t)canvas_width + (size_t)x] = color;
    }
}

static uint16_t darken_pixel(uint16_t pixel)
{
    uint16_t red = (uint16_t)(pixel & 31u);
    uint16_t green = (uint16_t)((pixel >> 5) & 31u);
    uint16_t blue = (uint16_t)((pixel >> 10) & 31u);

    return (uint16_t)(
        0x8000u |
        (red >> 1) |
        ((green >> 1) << 5) |
        ((blue >> 1) << 10));
}

static void darken_rect(
    uint16_t *pixels,
    int32_t canvas_width,
    const pstvnc_mpeg_calibration_rect_t *rect)
{
    int32_t y;
    int32_t x;

    for (y = rect->y; y < rect->y + rect->height; y++) {
        for (x = rect->x; x < rect->x + rect->width; x++) {
            size_t index =
                (size_t)y * (size_t)canvas_width + (size_t)x;
            pixels[index] = darken_pixel(pixels[index]);
        }
    }
}

static void copy_rect(
    uint16_t *destination,
    const uint16_t *source,
    int32_t canvas_width,
    const pstvnc_mpeg_calibration_rect_t *rect)
{
    int32_t y;

    for (y = rect->y; y < rect->y + rect->height; y++) {
        size_t index =
            (size_t)y * (size_t)canvas_width + (size_t)rect->x;

        memcpy(
            destination + index,
            source + index,
            (size_t)rect->width * sizeof(*destination));
    }
}

static void draw_inner_matte(
    uint16_t *pixels,
    int32_t canvas_width,
    const pstvnc_mpeg_calibration_geometry_t *geometry)
{
    pstvnc_mpeg_calibration_rect_t strip;
    int32_t base_right =
        geometry->base.x + geometry->base.width;
    int32_t base_bottom =
        geometry->base.y + geometry->base.height;
    int32_t inner_right =
        geometry->inner_content.x + geometry->inner_content.width;
    int32_t inner_bottom =
        geometry->inner_content.y + geometry->inner_content.height;

    strip.x = geometry->base.x;
    strip.y = geometry->base.y;
    strip.width = geometry->inner_content.x - geometry->base.x;
    strip.height = geometry->base.height;
    if (strip.width > 0)
        fill_rect(
            pixels,
            canvas_width,
            &strip,
            PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_BLACK);

    strip.x = inner_right;
    strip.width = base_right - inner_right;
    if (strip.width > 0)
        fill_rect(
            pixels,
            canvas_width,
            &strip,
            PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_BLACK);

    strip.x = geometry->inner_content.x;
    strip.y = geometry->base.y;
    strip.width = geometry->inner_content.width;
    strip.height = geometry->inner_content.y - geometry->base.y;
    if (strip.height > 0)
        fill_rect(
            pixels,
            canvas_width,
            &strip,
            PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_BLACK);

    strip.y = inner_bottom;
    strip.height = base_bottom - inner_bottom;
    if (strip.height > 0)
        fill_rect(
            pixels,
            canvas_width,
            &strip,
            PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_BLACK);
}

static void draw_horizontal_selector(
    uint16_t *pixels,
    int32_t canvas_width,
    int32_t x,
    int32_t y,
    int32_t length)
{
    int32_t offset = 0;
    unsigned int segment = 0u;

    while (offset < length) {
        pstvnc_mpeg_calibration_rect_t strip;
        int32_t span = MANUAL_SELECTOR_SEGMENT;
        uint16_t color =
            (segment & 1u) != 0u
                ? PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_TEAL
                : PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_ORANGE;

        if (span > length - offset)
            span = length - offset;

        strip.x = x + offset;
        strip.y = y;
        strip.width = span;
        strip.height = MANUAL_SELECTOR_THICKNESS;
        fill_rect(pixels, canvas_width, &strip, color);

        offset += span;
        segment++;
    }
}

static void draw_vertical_selector(
    uint16_t *pixels,
    int32_t canvas_width,
    int32_t x,
    int32_t y,
    int32_t length)
{
    int32_t offset = 0;
    unsigned int segment = 0u;

    while (offset < length) {
        pstvnc_mpeg_calibration_rect_t strip;
        int32_t span = MANUAL_SELECTOR_SEGMENT;
        uint16_t color =
            (segment & 1u) != 0u
                ? PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_TEAL
                : PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_ORANGE;

        if (span > length - offset)
            span = length - offset;

        strip.x = x;
        strip.y = y + offset;
        strip.width = MANUAL_SELECTOR_THICKNESS;
        strip.height = span;
        fill_rect(pixels, canvas_width, &strip, color);

        offset += span;
        segment++;
    }
}

static void draw_selector(
    uint16_t *pixels,
    int32_t canvas_width,
    const pstvnc_mpeg_calibration_rect_t *base)
{
    draw_horizontal_selector(
        pixels,
        canvas_width,
        base->x,
        base->y,
        base->width);

    draw_horizontal_selector(
        pixels,
        canvas_width,
        base->x,
        base->y + base->height - MANUAL_SELECTOR_THICKNESS,
        base->width);

    draw_vertical_selector(
        pixels,
        canvas_width,
        base->x,
        base->y,
        base->height);

    draw_vertical_selector(
        pixels,
        canvas_width,
        base->x + base->width - MANUAL_SELECTOR_THICKNESS,
        base->y,
        base->height);
}

static void draw_kind_marker(
    uint16_t *pixels,
    int32_t canvas_width,
    const pstvnc_mpeg_calibration_manual_plan_t *plan)
{
    pstvnc_mpeg_calibration_rect_t marker;

    marker.x = 2;
    marker.y = 2;
    marker.width =
        plan->canvas_width < MANUAL_KIND_MARKER_WIDTH + 2
            ? plan->canvas_width - 2
            : MANUAL_KIND_MARKER_WIDTH;
    marker.height =
        plan->canvas_height < MANUAL_KIND_MARKER_HEIGHT + 2
            ? plan->canvas_height - 2
            : MANUAL_KIND_MARKER_HEIGHT;

    if (marker.width <= 0 || marker.height <= 0)
        return;

    fill_rect(
        pixels,
        canvas_width,
        &marker,
        plan->kind == PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_REVIEW
            ? PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_TEAL
            : PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_ORANGE);
}

int pstvnc_mpeg_calibration_manual_init(
    pstvnc_mpeg_calibration_manual_source_t *source,
    int32_t canvas_width,
    int32_t canvas_height)
{
    if (source == NULL)
        return 0;

    memset(source, 0, sizeof(*source));
    return pstvnc_mpeg_calibration_init(
        &source->calibration,
        canvas_width,
        canvas_height);
}

int pstvnc_mpeg_calibration_manual_begin(
    pstvnc_mpeg_calibration_manual_source_t *source)
{
    return source != NULL &&
        pstvnc_mpeg_calibration_begin(&source->calibration);
}

int pstvnc_mpeg_calibration_manual_service_controller(
    pstvnc_mpeg_calibration_manual_source_t *source,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_mpeg_calibration_manual_result_t *result)
{
    pstvnc_mpeg_calibration_input_t input;
    pstvnc_mpeg_calibration_effects_t effects;
    int owned_before;
    int owned_after;

    if (source == NULL ||
        result == NULL ||
        !controller_state_valid(controller_state))
        return 0;

    memset(result, 0, sizeof(*result));

    owned_before =
        pstvnc_mpeg_calibration_owns_foreground(
            &source->calibration);

    if (!owned_before)
        return 1;

    input.down = map_controller_buttons(
        controller_state->buttons_down);
    input.pressed = map_controller_buttons(
        controller_state->buttons_pressed);

    if (!pstvnc_mpeg_calibration_update(
            &source->calibration,
            &input,
            &effects))
        return 0;

    owned_after =
        pstvnc_mpeg_calibration_owns_foreground(
            &source->calibration);

    result->consume_controller_state =
        owned_before || owned_after;
    result->accepted = effects.accepted ? 1u : 0u;
    result->cancelled = effects.cancelled ? 1u : 0u;
    result->release_quarantine_complete =
        effects.release_quarantine_complete ? 1u : 0u;

    if (effects.accepted)
        result->accepted_region = effects.accepted_region;

    return 1;
}

int pstvnc_mpeg_calibration_manual_prepare_plan(
    const pstvnc_mpeg_calibration_manual_source_t *source,
    pstvnc_mpeg_calibration_manual_plan_t *plan)
{
    if (source == NULL ||
        plan == NULL ||
        (source->calibration.screen != PSTVNC_MPEG_CALIBRATION_EDIT &&
         source->calibration.screen != PSTVNC_MPEG_CALIBRATION_REVIEW))
        return 0;

    memset(plan, 0, sizeof(*plan));

    plan->candidate_region = source->calibration.current;
    plan->canvas_width = source->calibration.canvas_width;
    plan->canvas_height = source->calibration.canvas_height;

    if (!pstvnc_mpeg_calibration_resolve_geometry(
            &source->calibration,
            &plan->candidate_region,
            &plan->geometry))
        return 0;

    if (source->calibration.screen == PSTVNC_MPEG_CALIBRATION_EDIT) {
        plan->kind = PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_EDIT;
        plan->title = "MPEG Calibration";
        plan->help_primary =
            "D-pad resize | R1 move | R2 inner | L2 outer";
        plan->help_secondary =
            "START reset | X review | O cancel";
    } else {
        plan->kind = PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_REVIEW;
        plan->review_accept_armed =
            source->calibration.review_accept_armed ? 1u : 0u;
        plan->title = "Review MPEG Region";
        plan->help_primary =
            plan->review_accept_armed
                ? "X accept | O back"
                : "Release X before final acceptance";
        plan->help_secondary =
            "Outer footprint is the RFB suppression region";
    }

    return plan_valid(plan);
}

int pstvnc_mpeg_calibration_manual_rasterize(
    const pstvnc_mpeg_calibration_manual_plan_t *plan,
    const uint16_t *frozen_desktop,
    size_t frozen_pixel_count,
    uint16_t *surface_pixels,
    size_t surface_capacity_pixels)
{
    size_t required;

    if (!plan_valid(plan) ||
        frozen_desktop == NULL ||
        surface_pixels == NULL ||
        !pixel_count(
            plan->canvas_width,
            plan->canvas_height,
            &required) ||
        frozen_pixel_count < required ||
        surface_capacity_pixels < required ||
        buffers_overlap(
            frozen_desktop,
            surface_pixels,
            required))
        return 0;

    memcpy(
        surface_pixels,
        frozen_desktop,
        required * sizeof(*surface_pixels));

    /*
     * H1's recovered visual meaning is preserved without inheriting its UI
     * state machine: the outer/suppression footprint is darkened, the base
     * desktop is restored, the P1 inner-content complement becomes a black
     * local matte, and the exact P1 base edge receives the selector.
     */
    darken_rect(
        surface_pixels,
        plan->canvas_width,
        &plan->geometry.suppression);

    copy_rect(
        surface_pixels,
        frozen_desktop,
        plan->canvas_width,
        &plan->geometry.base);

    draw_inner_matte(
        surface_pixels,
        plan->canvas_width,
        &plan->geometry);

    draw_selector(
        surface_pixels,
        plan->canvas_width,
        &plan->geometry.base);

    draw_kind_marker(
        surface_pixels,
        plan->canvas_width,
        plan);

    return 1;
}
