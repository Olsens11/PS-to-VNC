/*
 * File synopsis:
 * Adds the experiment-local native MPEG-calibration presentation layer around
 * the existing H1 interaction coordinator without changing clean src/ owners.
 *
 * The wrapped coordinator remains authoritative for RFB/input/OSK behavior.
 * This layer only observes calibration visual state after coordinator service,
 * rasterizes onto the last actually-presented RFB CT16 desktop, and submits that
 * complete surface through the existing PS2 graphics owner.
 */
#include "h1_interaction_coordinator.h"
#include "mpeg_presentation_calibration/h1_mpeg_calibration_raster.h"
#include "platform/ps2_graphics.h"

#include <string.h>

int pstvnc_h1_interaction_coordinator_present_inner(
    void *context,
    const pstvnc_framebuffer_t *framebuffer);
int pstvnc_h1_interaction_coordinator_service_inner(
    void *context,
    pstvnc_rfb_session_t *session);

typedef struct pstvnc_h1_calibration_presenter_state {
    pstvnc_h1_interaction_coordinator_t *coordinator;
    pstvnc_mpeg_calibration_t last_calibration;
    int last_valid;
    int last_visible;
    uint16_t surface_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
        __attribute__((aligned(128)));
} pstvnc_h1_calibration_presenter_state_t;

static pstvnc_h1_calibration_presenter_state_t calibration_presenter;

static const pstvnc_mpeg_calibration_t *h1_calibration_state(
    const pstvnc_h1_interaction_coordinator_t *coordinator)
{
    if (coordinator == NULL)
        return NULL;
    return &coordinator->mpeg_calibration.runtime.foreground.adapter.calibration;
}

static int h1_calibration_region_equal(
    const pstvnc_mpeg_cal_region_t *left,
    const pstvnc_mpeg_cal_region_t *right)
{
    if (left == NULL || right == NULL)
        return 0;
    return left->x == right->x && left->y == right->y &&
        left->width == right->width && left->height == right->height &&
        left->inner_matte_x == right->inner_matte_x &&
        left->inner_matte_y == right->inner_matte_y &&
        left->outer_matte_x == right->outer_matte_x &&
        left->outer_matte_y == right->outer_matte_y;
}

static int h1_calibration_visual_equal(
    const pstvnc_mpeg_calibration_t *left,
    const pstvnc_mpeg_calibration_t *right)
{
    if (left == NULL || right == NULL)
        return 0;
    return left->screen == right->screen &&
        h1_calibration_region_equal(&left->current, &right->current);
}

static int h1_calibration_visible(
    const pstvnc_mpeg_calibration_t *calibration)
{
    return calibration != NULL &&
        calibration->screen != PSTVNC_MPEG_CAL_INACTIVE;
}

static void h1_calibration_presenter_attach(
    pstvnc_h1_interaction_coordinator_t *coordinator)
{
    const pstvnc_mpeg_calibration_t *calibration;

    if (calibration_presenter.coordinator == coordinator &&
        calibration_presenter.last_valid)
        return;

    memset(&calibration_presenter, 0, sizeof(calibration_presenter));
    calibration_presenter.coordinator = coordinator;
    calibration = h1_calibration_state(coordinator);
    if (calibration != NULL) {
        calibration_presenter.last_calibration = *calibration;
        calibration_presenter.last_visible = h1_calibration_visible(calibration);
        calibration_presenter.last_valid = 1;
    }
}

static int h1_calibration_increment_presentation(
    pstvnc_h1_interaction_coordinator_t *coordinator)
{
    if (coordinator == NULL ||
        coordinator->stats.local_presentations == UINT32_MAX)
        return 0;
    coordinator->stats.local_presentations++;
    return 1;
}

static int h1_calibration_present_if_changed(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    int force_visible)
{
    const pstvnc_mpeg_calibration_t *calibration;
    pstvnc_h1_mpeg_calibration_render_plan_t plan;
    int visible;
    int changed;

    if (coordinator == NULL)
        return 0;

    h1_calibration_presenter_attach(coordinator);
    calibration = h1_calibration_state(coordinator);
    if (calibration == NULL)
        return 0;

    visible = h1_calibration_visible(calibration);
    changed = !calibration_presenter.last_valid ||
        !h1_calibration_visual_equal(
            &calibration_presenter.last_calibration, calibration) ||
        calibration_presenter.last_visible != visible;

    if (!changed && !(force_visible && visible))
        return 1;

    if (visible) {
        if (!pstvnc_h1_mpeg_calibration_runtime_prepare_render_plan(
                &coordinator->mpeg_calibration.runtime, &plan))
            return 0;
        if (!pstvnc_h1_mpeg_calibration_rasterize(
                &plan,
                coordinator->gs_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT,
                PSTVNC_DISPLAY_WIDTH,
                PSTVNC_DISPLAY_HEIGHT,
                calibration_presenter.surface_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT))
            return 0;
        if (pstvnc_ps2_graphics_present(
                calibration_presenter.surface_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT,
                NULL) < 0)
            return 0;
        if (!h1_calibration_increment_presentation(coordinator))
            return 0;
    } else if (calibration_presenter.last_visible) {
        /*
         * Cancel/accept removes only the calibration layer. The RFB gate still
         * owns thaw timing through release quarantine, so redraw the exact
         * frozen desktop rather than consuming hidden decoder state.
         */
        if (pstvnc_ps2_graphics_present(
                coordinator->gs_pixels,
                PSTVNC_DISPLAY_PIXEL_COUNT,
                NULL) < 0)
            return 0;
        if (!h1_calibration_increment_presentation(coordinator))
            return 0;
    }

    calibration_presenter.last_calibration = *calibration;
    calibration_presenter.last_visible = visible;
    calibration_presenter.last_valid = 1;
    return 1;
}

int pstvnc_h1_interaction_coordinator_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer)
{
    pstvnc_h1_interaction_coordinator_t *coordinator =
        (pstvnc_h1_interaction_coordinator_t *)context;

    if (!pstvnc_h1_interaction_coordinator_present_inner(context, framebuffer))
        return 0;

    /*
     * Flow policy should suppress remote presentation while calibration owns
     * foreground. If a caller nevertheless reaches this seam, immediately
     * recompose calibration over the new base rather than expose a stale layer.
     */
    return h1_calibration_present_if_changed(coordinator, 1);
}

int pstvnc_h1_interaction_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_interaction_coordinator_t *coordinator =
        (pstvnc_h1_interaction_coordinator_t *)context;

    h1_calibration_presenter_attach(coordinator);
    if (!pstvnc_h1_interaction_coordinator_service_inner(context, session))
        return 0;
    return h1_calibration_present_if_changed(coordinator, 0);
}
