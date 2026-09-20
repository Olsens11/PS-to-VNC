/*
 * File synopsis:
 * Implements the Display-owned MPEG visible-ownership state and immutable
 * resolved geometry/run snapshot without performing physical presentation.
 *
 * Geometry validation preserves the current A004 distinctions: the MPEG base
 * rectangle remains macroblock-sized, inner content is a symmetric inset owned
 * by presentation, and suppression contains the complete base footprint.
 * No UI/calibration, RFB, decoder, Transport, Platform or media-clock API is
 * imported here.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3;
 *   docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md;
 *   docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md, Q6/Q7.
 */

#include "mpeg_presentation.h"

#include <stddef.h>
#include <string.h>

static int pstvnc_mpeg_presentation_rect_end(
    const pstvnc_mpeg_presentation_rect_t *rect,
    int64_t *right,
    int64_t *bottom)
{
    if (rect == NULL ||
        right == NULL ||
        bottom == NULL ||
        rect->x < 0 ||
        rect->y < 0 ||
        rect->width < 0 ||
        rect->height < 0)
        return 0;

    *right = (int64_t)rect->x + (int64_t)rect->width;
    *bottom = (int64_t)rect->y + (int64_t)rect->height;
    return 1;
}

static int pstvnc_mpeg_presentation_geometry_valid(
    const pstvnc_mpeg_presentation_geometry_t *geometry)
{
    int64_t base_right;
    int64_t base_bottom;
    int64_t inner_right;
    int64_t inner_bottom;
    int64_t suppression_right;
    int64_t suppression_bottom;
    int64_t inner_left_inset;
    int64_t inner_right_inset;
    int64_t inner_top_inset;
    int64_t inner_bottom_inset;

    if (geometry == NULL ||
        geometry->base.width < 16 ||
        geometry->base.height < 16 ||
        (geometry->base.width & 15) != 0 ||
        (geometry->base.height & 15) != 0 ||
        geometry->suppression.width <= 0 ||
        geometry->suppression.height <= 0)
        return 0;

    if (!pstvnc_mpeg_presentation_rect_end(
            &geometry->base,
            &base_right,
            &base_bottom) ||
        !pstvnc_mpeg_presentation_rect_end(
            &geometry->inner_content,
            &inner_right,
            &inner_bottom) ||
        !pstvnc_mpeg_presentation_rect_end(
            &geometry->suppression,
            &suppression_right,
            &suppression_bottom))
        return 0;

    if (geometry->inner_content.x < geometry->base.x ||
        geometry->inner_content.y < geometry->base.y ||
        inner_right > base_right ||
        inner_bottom > base_bottom)
        return 0;

    inner_left_inset =
        (int64_t)geometry->inner_content.x -
        (int64_t)geometry->base.x;
    inner_right_inset =
        base_right - inner_right;
    inner_top_inset =
        (int64_t)geometry->inner_content.y -
        (int64_t)geometry->base.y;
    inner_bottom_inset =
        base_bottom - inner_bottom;

    if (inner_left_inset != inner_right_inset ||
        inner_top_inset != inner_bottom_inset)
        return 0;

    if (geometry->suppression.x > geometry->base.x ||
        geometry->suppression.y > geometry->base.y ||
        suppression_right < base_right ||
        suppression_bottom < base_bottom)
        return 0;

    return 1;
}

static void pstvnc_mpeg_presentation_return_to_rfb(
    pstvnc_mpeg_presentation_t *presentation)
{
    presentation->state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
    presentation->run_generation = 0u;
    presentation->snapshot_valid = 0u;
    memset(&presentation->geometry, 0, sizeof(presentation->geometry));
}

void pstvnc_mpeg_presentation_init(
    pstvnc_mpeg_presentation_t *presentation)
{
    if (presentation == NULL)
        return;

    memset(presentation, 0, sizeof(*presentation));
    presentation->state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
}

int pstvnc_mpeg_presentation_arm(
    pstvnc_mpeg_presentation_t *presentation,
    const pstvnc_mpeg_presentation_geometry_t *geometry,
    uint32_t run_generation)
{
    if (presentation == NULL ||
        presentation->state != PSTVNC_MPEG_PRESENTATION_RFB_ONLY ||
        presentation->snapshot_valid ||
        run_generation == 0u ||
        !pstvnc_mpeg_presentation_geometry_valid(geometry))
        return 0;

    presentation->geometry = *geometry;
    presentation->run_generation = run_generation;
    presentation->snapshot_valid = 1u;
    presentation->state = PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
    return 1;
}

int pstvnc_mpeg_presentation_first_frame_presented(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation)
{
    if (presentation == NULL ||
        presentation->state != PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||
        !presentation->snapshot_valid ||
        run_generation == 0u ||
        run_generation != presentation->run_generation)
        return 0;

    presentation->state = PSTVNC_MPEG_PRESENTATION_MPEG_OWNED;
    return 1;
}

int pstvnc_mpeg_presentation_abort_pending(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation)
{
    if (presentation == NULL ||
        presentation->state != PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||
        !presentation->snapshot_valid ||
        run_generation == 0u ||
        run_generation != presentation->run_generation)
        return 0;

    pstvnc_mpeg_presentation_return_to_rfb(presentation);
    return 1;
}

int pstvnc_mpeg_presentation_begin_retirement(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation)
{
    if (presentation == NULL ||
        presentation->state != PSTVNC_MPEG_PRESENTATION_MPEG_OWNED ||
        !presentation->snapshot_valid ||
        run_generation == 0u ||
        run_generation != presentation->run_generation)
        return 0;

    presentation->state = PSTVNC_MPEG_PRESENTATION_RETIRING;
    return 1;
}

int pstvnc_mpeg_presentation_seal_retirement(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation)
{
    if (presentation == NULL ||
        presentation->state != PSTVNC_MPEG_PRESENTATION_RETIRING ||
        !presentation->snapshot_valid ||
        run_generation == 0u ||
        run_generation != presentation->run_generation)
        return 0;

    presentation->state = PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING;
    return 1;
}

int pstvnc_mpeg_presentation_commit_reveal(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation)
{
    if (presentation == NULL ||
        presentation->state != PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING ||
        !presentation->snapshot_valid ||
        run_generation == 0u ||
        run_generation != presentation->run_generation)
        return 0;

    pstvnc_mpeg_presentation_return_to_rfb(presentation);
    return 1;
}

pstvnc_mpeg_presentation_state_t
pstvnc_mpeg_presentation_state(
    const pstvnc_mpeg_presentation_t *presentation)
{
    if (presentation == NULL)
        return PSTVNC_MPEG_PRESENTATION_STATE_INVALID;

    return presentation->state;
}

pstvnc_mpeg_presentation_mode_t
pstvnc_mpeg_presentation_mode(
    const pstvnc_mpeg_presentation_t *presentation)
{
    if (presentation == NULL)
        return PSTVNC_MPEG_PRESENTATION_MODE_INVALID;

    if (presentation->state == PSTVNC_MPEG_PRESENTATION_RFB_ONLY)
        return PSTVNC_MPEG_PRESENTATION_DIRECT_RFB;

    if (presentation->state == PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME)
        return PSTVNC_MPEG_PRESENTATION_FROZEN_RFB_DESKTOP;

    if (presentation->state == PSTVNC_MPEG_PRESENTATION_MPEG_OWNED ||
        presentation->state == PSTVNC_MPEG_PRESENTATION_RETIRING ||
        presentation->state == PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING)
        return PSTVNC_MPEG_PRESENTATION_COMPOSITED;

    return PSTVNC_MPEG_PRESENTATION_MODE_INVALID;
}

int pstvnc_mpeg_presentation_requires_global_rfb_protection(
    const pstvnc_mpeg_presentation_t *presentation)
{
    return presentation != NULL &&
        presentation->state == PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
}

int pstvnc_mpeg_presentation_owns_mpeg_visual(
    const pstvnc_mpeg_presentation_t *presentation)
{
    return presentation != NULL &&
        (presentation->state == PSTVNC_MPEG_PRESENTATION_MPEG_OWNED ||
         presentation->state == PSTVNC_MPEG_PRESENTATION_RETIRING ||
         presentation->state == PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING);
}

int pstvnc_mpeg_presentation_snapshot(
    const pstvnc_mpeg_presentation_t *presentation,
    pstvnc_mpeg_presentation_geometry_t *geometry,
    uint32_t *run_generation)
{
    if (presentation == NULL ||
        geometry == NULL ||
        run_generation == NULL ||
        !presentation->snapshot_valid ||
        presentation->state == PSTVNC_MPEG_PRESENTATION_RFB_ONLY ||
        presentation->run_generation == 0u)
        return 0;

    *geometry = presentation->geometry;
    *run_generation = presentation->run_generation;
    return 1;
}
