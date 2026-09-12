/*
 * File synopsis:
 * Implements the CP2P-only committed-region -> MPEG-start ownership handoff.
 *
 * Capture/base presentation geometry stays unchanged by inner/outer mattes.
 * Inner matte is preserved for the future shared compositor; outer matte is
 * reduced to the clipped RFB-suppression rectangle. The contained presentation
 * owner supplies first-frame generation protection and lifecycle state only.
 */
#include "h1_mpeg_start_handoff.h"

#include <stddef.h>
#include <string.h>

static int h1_mpeg_start_region_valid(
    const pstvnc_h1_mpeg_start_handoff_t *handoff,
    const pstvnc_mpeg_cal_region_t *region)
{
    int64_t right;
    int64_t bottom;

    if (handoff == NULL || region == NULL ||
        handoff->canvas_width <= 0 || handoff->canvas_height <= 0 ||
        region->x < 0 || region->y < 0 ||
        region->width < 16 || region->height < 16 ||
        (region->width & 15) != 0 || (region->height & 15) != 0 ||
        region->inner_matte_x < 0 || region->inner_matte_y < 0 ||
        region->inner_matte_x > region->width / 2 ||
        region->inner_matte_y > region->height / 2 ||
        region->outer_matte_x < 0 || region->outer_matte_y < 0)
        return 0;

    right = (int64_t)region->x + (int64_t)region->width;
    bottom = (int64_t)region->y + (int64_t)region->height;

    return right <= (int64_t)handoff->canvas_width &&
        bottom <= (int64_t)handoff->canvas_height;
}

void pstvnc_h1_mpeg_start_handoff_init(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    int canvas_width,
    int canvas_height)
{
    if (handoff == NULL)
        return;

    memset(handoff, 0, sizeof(*handoff));
    handoff->canvas_width = canvas_width;
    handoff->canvas_height = canvas_height;
    pstvnc_h1_mpeg_presentation_owner_init(&handoff->owner);
}

int pstvnc_h1_mpeg_start_handoff_prepare_start(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    const pstvnc_mpeg_cal_region_t *committed_region,
    pstvnc_h1_mpeg_start_contract_t *contract)
{
    pstvnc_mpeg_calibration_t geometry_context;
    pstvnc_mpeg_cal_rect_t suppression;
    uint32_t generation;

    if (handoff == NULL || contract == NULL ||
        !h1_mpeg_start_region_valid(handoff, committed_region))
        return 0;

    memset(&geometry_context, 0, sizeof(geometry_context));
    geometry_context.canvas_width = handoff->canvas_width;
    geometry_context.canvas_height = handoff->canvas_height;

    suppression = pstvnc_mpeg_calibration_rfb_suppression_rect(
        &geometry_context,
        committed_region);

    if (!pstvnc_h1_mpeg_presentation_owner_arm(
            &handoff->owner,
            committed_region,
            &suppression,
            &generation))
        return 0;

    memset(contract, 0, sizeof(*contract));
    contract->draw_x = committed_region->x;
    contract->draw_y = committed_region->y;
    contract->draw_width = committed_region->width;
    contract->draw_height = committed_region->height;
    contract->inner_matte_x = committed_region->inner_matte_x;
    contract->inner_matte_y = committed_region->inner_matte_y;
    contract->suppression_rect = suppression;
    contract->generation = generation;
    return 1;
}

int pstvnc_h1_mpeg_start_handoff_first_frame_presented(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    uint32_t generation)
{
    return handoff != NULL &&
        pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
            &handoff->owner,
            generation);
}

int pstvnc_h1_mpeg_start_handoff_abort_start(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    uint32_t generation)
{
    return handoff != NULL &&
        pstvnc_h1_mpeg_presentation_owner_abort_start(
            &handoff->owner,
            generation);
}

int pstvnc_h1_mpeg_start_handoff_stop(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    uint32_t generation)
{
    return handoff != NULL &&
        pstvnc_h1_mpeg_presentation_owner_stop(
            &handoff->owner,
            generation);
}

int pstvnc_h1_mpeg_start_handoff_allows_rfb_request(
    const pstvnc_h1_mpeg_start_handoff_t *handoff)
{
    return handoff != NULL &&
        pstvnc_h1_mpeg_presentation_owner_allows_rfb_request(
            &handoff->owner);
}

pstvnc_h1_mpeg_remote_present_mode_t
pstvnc_h1_mpeg_start_handoff_remote_present_mode(
    const pstvnc_h1_mpeg_start_handoff_t *handoff)
{
    if (handoff == NULL)
        return PSTVNC_H1_MPEG_REMOTE_PRESENT_DIRECT_RFB;

    return pstvnc_h1_mpeg_presentation_owner_remote_present_mode(
        &handoff->owner);
}

int pstvnc_h1_mpeg_start_handoff_take_full_refresh(
    pstvnc_h1_mpeg_start_handoff_t *handoff)
{
    return handoff != NULL &&
        pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&handoff->owner);
}

const pstvnc_mpeg_cal_rect_t *
pstvnc_h1_mpeg_start_handoff_suppression_rect(
    const pstvnc_h1_mpeg_start_handoff_t *handoff)
{
    if (handoff == NULL)
        return NULL;

    return pstvnc_h1_mpeg_presentation_owner_suppression_rect(
        &handoff->owner);
}
