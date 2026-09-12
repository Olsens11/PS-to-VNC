/*
 * File synopsis:
 * Implements the experiment-local accepted-region -> first-frame -> MPEG-owned
 * presentation lifecycle without taking decoder, RFB, calibration, or GS
 * ownership.
 */
#include "h1_mpeg_presentation_owner.h"

#include <stddef.h>
#include <string.h>

static int h1_mpeg_region_valid(const pstvnc_mpeg_cal_region_t *region)
{
    return region != NULL &&
        region->x >= 0 &&
        region->y >= 0 &&
        region->width > 0 &&
        region->height > 0 &&
        region->inner_matte_x >= 0 &&
        region->inner_matte_y >= 0 &&
        region->outer_matte_x >= 0 &&
        region->outer_matte_y >= 0 &&
        region->inner_matte_x <= region->width / 2 &&
        region->inner_matte_y <= region->height / 2;
}

static int h1_suppression_contains_region(
    const pstvnc_mpeg_cal_rect_t *suppression,
    const pstvnc_mpeg_cal_region_t *region)
{
    int64_t suppression_right;
    int64_t suppression_bottom;
    int64_t region_right;
    int64_t region_bottom;

    if (suppression == NULL || region == NULL ||
        suppression->x < 0 || suppression->y < 0 ||
        suppression->width <= 0 || suppression->height <= 0)
        return 0;

    suppression_right =
        (int64_t)suppression->x + (int64_t)suppression->width;
    suppression_bottom =
        (int64_t)suppression->y + (int64_t)suppression->height;
    region_right = (int64_t)region->x + (int64_t)region->width;
    region_bottom = (int64_t)region->y + (int64_t)region->height;

    return
        suppression->x <= region->x &&
        suppression->y <= region->y &&
        suppression_right >= region_right &&
        suppression_bottom >= region_bottom;
}

static uint32_t h1_next_generation(uint32_t current)
{
    current++;
    if (current == 0u)
        current = 1u;
    return current;
}

static void h1_return_to_rfb(
    pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    owner->state = PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY;
    owner->has_region = 0;
    owner->full_refresh_pending = 1;
    memset(&owner->region, 0, sizeof(owner->region));
    memset(&owner->suppression_rect, 0, sizeof(owner->suppression_rect));
}

void pstvnc_h1_mpeg_presentation_owner_init(
    pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    if (owner == NULL)
        return;

    memset(owner, 0, sizeof(*owner));
    owner->state = PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY;
}

int pstvnc_h1_mpeg_presentation_owner_arm(
    pstvnc_h1_mpeg_presentation_owner_t *owner,
    const pstvnc_mpeg_cal_region_t *region,
    const pstvnc_mpeg_cal_rect_t *suppression_rect,
    uint32_t *generation)
{
    if (owner == NULL || generation == NULL ||
        owner->state != PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY ||
        !h1_mpeg_region_valid(region) ||
        !h1_suppression_contains_region(suppression_rect, region))
        return 0;

    owner->generation = h1_next_generation(owner->generation);
    owner->region = *region;
    owner->suppression_rect = *suppression_rect;
    owner->has_region = 1;
    owner->full_refresh_pending = 0;
    owner->state = PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
    *generation = owner->generation;
    return 1;
}

int pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
    pstvnc_h1_mpeg_presentation_owner_t *owner,
    uint32_t generation)
{
    if (owner == NULL ||
        owner->state != PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||
        !owner->has_region ||
        generation == 0u || generation != owner->generation)
        return 0;

    owner->state = PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED;

    /*
     * RFB requests were deliberately paused while calibration/startup held the
     * frozen desktop. The first MPEG frame permits RFB to resume elsewhere, but
     * it must resume from one full/nonincremental refresh before incremental
     * updates continue.
     */
    owner->full_refresh_pending = 1;
    return 1;
}

int pstvnc_h1_mpeg_presentation_owner_abort_start(
    pstvnc_h1_mpeg_presentation_owner_t *owner,
    uint32_t generation)
{
    if (owner == NULL ||
        owner->state != PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||
        generation == 0u || generation != owner->generation)
        return 0;

    h1_return_to_rfb(owner);
    return 1;
}

int pstvnc_h1_mpeg_presentation_owner_stop(
    pstvnc_h1_mpeg_presentation_owner_t *owner,
    uint32_t generation)
{
    if (owner == NULL ||
        owner->state != PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED ||
        generation == 0u || generation != owner->generation)
        return 0;

    h1_return_to_rfb(owner);
    return 1;
}

pstvnc_h1_mpeg_presentation_owner_state_t
pstvnc_h1_mpeg_presentation_owner_state(
    const pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    if (owner == NULL)
        return PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY;

    return owner->state;
}

int pstvnc_h1_mpeg_presentation_owner_allows_rfb_request(
    const pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    return owner != NULL &&
        owner->state != PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
}

pstvnc_h1_mpeg_remote_present_mode_t
pstvnc_h1_mpeg_presentation_owner_remote_present_mode(
    const pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    if (owner == NULL ||
        owner->state == PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY)
        return PSTVNC_H1_MPEG_REMOTE_PRESENT_DIRECT_RFB;

    if (owner->state == PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME)
        return PSTVNC_H1_MPEG_REMOTE_PRESENT_FROZEN_DESKTOP;

    return PSTVNC_H1_MPEG_REMOTE_PRESENT_COMPOSITED;
}

int pstvnc_h1_mpeg_presentation_owner_has_mpeg(
    const pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    return owner != NULL &&
        owner->state == PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED;
}

int pstvnc_h1_mpeg_presentation_owner_take_full_refresh(
    pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    int pending;

    if (owner == NULL)
        return 0;

    pending = owner->full_refresh_pending ? 1 : 0;
    owner->full_refresh_pending = 0;
    return pending;
}

const pstvnc_mpeg_cal_region_t *
pstvnc_h1_mpeg_presentation_owner_region(
    const pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    if (owner == NULL || !owner->has_region)
        return NULL;

    return &owner->region;
}

const pstvnc_mpeg_cal_rect_t *
pstvnc_h1_mpeg_presentation_owner_suppression_rect(
    const pstvnc_h1_mpeg_presentation_owner_t *owner)
{
    if (owner == NULL || !owner->has_region)
        return NULL;

    return &owner->suppression_rect;
}
