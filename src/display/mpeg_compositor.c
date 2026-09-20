/*
 * File synopsis:
 * Implements exact-run A004 MPEG presentation coordination around the sole
 * Platform graphics owner, the P3 visible-ownership state, and one session media
 * clock.
 *
 * No physical completion is inferred from decode, detile, upload or queue
 * submission. The platform must report successful synchronized presentation and
 * a timer observation in the same tick domain as the session media clock before
 * clock arm or P3 first-frame promotion may occur.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-SHARED-COMPOSITOR-FIRST-SYNC-R4;
 *   docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md, Q6/Q7.
 */

#include "mpeg_compositor.h"

#include <stddef.h>
#include <string.h>

#include "display.h"
#include "platform/ps2_graphics.h"

static int rect_within_logical_display(
    const pstvnc_mpeg_presentation_rect_t *rect)
{
    if (rect == NULL ||
        rect->x < 0 ||
        rect->y < 0 ||
        rect->width <= 0 ||
        rect->height <= 0 ||
        rect->x >
            (int32_t)PSTVNC_DISPLAY_WIDTH -
            rect->width ||
        rect->y >
            (int32_t)PSTVNC_DISPLAY_HEIGHT -
            rect->height)
        return 0;

    return 1;
}

static void copy_rect(
    pstvnc_ps2_graphics_rect_t *destination,
    const pstvnc_mpeg_presentation_rect_t *source)
{
    destination->x = source->x;
    destination->y = source->y;
    destination->width = source->width;
    destination->height = source->height;
}

int pstvnc_mpeg_compositor_present(
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock,
    const pstvnc_mpeg_compositor_frame_t *frame,
    pstvnc_mpeg_compositor_effects_t *effects)
{
    pstvnc_mpeg_presentation_geometry_t geometry;
    pstvnc_mpeg_presentation_state_t state;
    pstvnc_ps2_graphics_video_t video;
    pstvnc_ps2_graphics_sync_result_t sync_result;
    uint32_t snapshot_generation;
    int clock_armed;
    int clock_result;

    if (effects == NULL)
        return PSTVNC_MPEG_COMPOSITOR_INVALID;

    memset(effects, 0, sizeof(*effects));

    if (presentation == NULL ||
        clock == NULL ||
        frame == NULL ||
        frame->run_generation == 0u ||
        clock->ticks_per_second == 0u ||
        clock->sync.lock == NULL ||
        clock->sync.unlock == NULL ||
        !pstvnc_mpeg_rgb16_macroblock_surface_valid(
            &frame->surface))
        return PSTVNC_MPEG_COMPOSITOR_INVALID;

    state = pstvnc_mpeg_presentation_state(presentation);

    if (state != PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME &&
        state != PSTVNC_MPEG_PRESENTATION_MPEG_OWNED &&
        state != PSTVNC_MPEG_PRESENTATION_RETIRING)
        return PSTVNC_MPEG_COMPOSITOR_INVALID;

    if (!pstvnc_mpeg_presentation_snapshot(
            presentation,
            &geometry,
            &snapshot_generation) ||
        snapshot_generation != frame->run_generation ||
        frame->surface.width !=
            (unsigned int)geometry.base.width ||
        frame->surface.height !=
            (unsigned int)geometry.base.height ||
        !rect_within_logical_display(&geometry.base) ||
        geometry.inner_content.x < 0 ||
        geometry.inner_content.y < 0 ||
        geometry.inner_content.width < 0 ||
        geometry.inner_content.height < 0 ||
        geometry.inner_content.x >
            (int32_t)PSTVNC_DISPLAY_WIDTH -
            geometry.inner_content.width ||
        geometry.inner_content.y >
            (int32_t)PSTVNC_DISPLAY_HEIGHT -
            geometry.inner_content.height ||
        !rect_within_logical_display(&geometry.suppression))
        return PSTVNC_MPEG_COMPOSITOR_INVALID;

    memset(&video, 0, sizeof(video));
    video.surface = frame->surface;
    copy_rect(&video.base, &geometry.base);
    copy_rect(&video.inner_content, &geometry.inner_content);
    copy_rect(&video.suppression, &geometry.suppression);

    memset(&sync_result, 0, sizeof(sync_result));

    if (pstvnc_ps2_graphics_present_video_macroblocks(
            &video,
            &sync_result) != 0)
        return PSTVNC_MPEG_COMPOSITOR_PLATFORM_FAILED;

    if (!sync_result.synchronized ||
        sync_result.ticks_per_second == 0u ||
        sync_result.ticks_per_second != clock->ticks_per_second)
        return PSTVNC_MPEG_COMPOSITOR_SYNC_INVALID;

    effects->synchronized = 1u;
    effects->observed_sync_tick =
        sync_result.observed_sync_tick;

    clock_result = pstvnc_media_clock_is_armed(
        clock,
        &clock_armed);

    if (clock_result != PSTVNC_MEDIA_CLOCK_OK)
        return PSTVNC_MPEG_COMPOSITOR_CLOCK_FAILED;

    if (state == PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME) {
        if (!clock_armed) {
            clock_result = pstvnc_media_clock_arm(
                clock,
                sync_result.observed_sync_tick);

            if (clock_result != PSTVNC_MEDIA_CLOCK_OK)
                return PSTVNC_MPEG_COMPOSITOR_CLOCK_FAILED;

            effects->clock_armed_now = 1u;
        }

        if (!pstvnc_mpeg_presentation_first_frame_presented(
                presentation,
                frame->run_generation))
            return PSTVNC_MPEG_COMPOSITOR_PROMOTION_FAILED;

        effects->first_frame_promoted = 1u;
        return PSTVNC_MPEG_COMPOSITOR_OK;
    }

    if (!clock_armed)
        return PSTVNC_MPEG_COMPOSITOR_CLOCK_FAILED;

    return PSTVNC_MPEG_COMPOSITOR_OK;
}

int pstvnc_mpeg_compositor_reveal_retired(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation,
    pstvnc_mpeg_compositor_effects_t *effects)
{
    pstvnc_mpeg_presentation_geometry_t geometry;
    pstvnc_ps2_graphics_sync_result_t sync_result;
    uint32_t snapshot_generation;

    if (effects == NULL)
        return PSTVNC_MPEG_COMPOSITOR_INVALID;

    memset(effects, 0, sizeof(*effects));

    if (presentation == NULL ||
        run_generation == 0u ||
        pstvnc_mpeg_presentation_state(presentation) !=
            PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING ||
        !pstvnc_mpeg_presentation_snapshot(
            presentation,
            &geometry,
            &snapshot_generation) ||
        snapshot_generation != run_generation)
        return PSTVNC_MPEG_COMPOSITOR_INVALID;

    /*
     * The retained geometry is intentionally observed here even though Platform
     * already owns its physical layer copy. A successful snapshot proves this
     * is the exact pending run rather than a generic graphics-clear request.
     */
    (void)geometry;

    memset(&sync_result, 0, sizeof(sync_result));

    if (pstvnc_ps2_graphics_reveal_retained_video(
            &sync_result) != 0)
        return PSTVNC_MPEG_COMPOSITOR_PLATFORM_FAILED;

    if (!sync_result.synchronized)
        return PSTVNC_MPEG_COMPOSITOR_SYNC_INVALID;

    effects->synchronized = 1u;
    effects->observed_sync_tick =
        sync_result.observed_sync_tick;

    /*
     * Platform has no callback into Presentation; this coordinator serializes
     * the cross-owner handoff. The exact pending run validated before the
     * physical reveal is therefore the only legal logical commit target.
     */
    if (!pstvnc_mpeg_presentation_commit_reveal(
            presentation,
            run_generation))
        return PSTVNC_MPEG_COMPOSITOR_RETIREMENT_COMMIT_FAILED;

    effects->retirement_revealed = 1u;
    return PSTVNC_MPEG_COMPOSITOR_OK;
}
