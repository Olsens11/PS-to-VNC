/*
 * File synopsis:
 * Implements the A004 P9 Application-owned manual MPEG CALIBRATION foreground
 * safety transaction.
 *
 * The coordinator composes public owner seams in a fixed order without taking
 * their private mechanisms: P2 global protection, input-runtime suspension,
 * exact published-pointer neutralization/rebase, P8 manual calibration,
 * caller-owned frozen/work CT16 surfaces, ordinary Platform presentation, and
 * read-only P3 admission/endpoint checks.
 *
 * Any failure after P2 freeze fails closed: P2 remains frozen and this module
 * does not guess at mouse ownership, thaw, P3 state, or MPEG lifecycle repair.
 */

#include "app_mpeg_calibration.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "platform/ps2_graphics.h"

static int pixel_count_for_canvas(
    int32_t width,
    int32_t height,
    size_t *pixel_count)
{
    size_t w;
    size_t h;

    if (pixel_count == NULL ||
        width <= 0 ||
        height <= 0)
        return 0;

    w = (size_t)width;
    h = (size_t)height;

    if (w > SIZE_MAX / h)
        return 0;

    *pixel_count = w * h;
    return 1;
}

static int pixel_ranges_overlap(
    const uint16_t *left,
    size_t left_pixels,
    const uint16_t *right,
    size_t right_pixels)
{
    uintptr_t left_begin;
    uintptr_t left_end;
    uintptr_t right_begin;
    uintptr_t right_end;
    size_t left_bytes;
    size_t right_bytes;

    if (left == NULL || right == NULL)
        return 1;

    if (left_pixels > SIZE_MAX / sizeof(*left) ||
        right_pixels > SIZE_MAX / sizeof(*right))
        return 1;

    left_bytes = left_pixels * sizeof(*left);
    right_bytes = right_pixels * sizeof(*right);
    left_begin = (uintptr_t)left;
    right_begin = (uintptr_t)right;

    if (left_begin > UINTPTR_MAX - left_bytes ||
        right_begin > UINTPTR_MAX - right_bytes)
        return 1;

    left_end = left_begin + left_bytes;
    right_end = right_begin + right_bytes;

    return left_begin < right_end && right_begin < left_end;
}

static int presentation_is_exact_rfb_only(
    const pstvnc_mpeg_presentation_t *presentation)
{
    return presentation != NULL &&
        presentation->state == PSTVNC_MPEG_PRESENTATION_RFB_ONLY &&
        !presentation->snapshot_valid &&
        presentation->run_generation == 0u;
}

static int manual_source_is_inactive(
    const pstvnc_mpeg_calibration_manual_source_t *manual_source)
{
    return manual_source != NULL &&
        manual_source->calibration.screen ==
            PSTVNC_MPEG_CALIBRATION_INACTIVE &&
        !manual_source->calibration.release_quarantine_pending &&
        !pstvnc_mpeg_calibration_owns_foreground(
            &manual_source->calibration);
}

static void clear_live_owner_references(
    pstvnc_app_mpeg_calibration_t *calibration)
{
    calibration->rfb_flow_policy = NULL;
    calibration->input_runtime = NULL;
    calibration->rfb_session = NULL;
    calibration->local_ui = NULL;
    calibration->presentation = NULL;
    calibration->mouse_interpretation_suspended = 0u;
    calibration->accepted_pending_release = 0u;
    calibration->cancelled_pending_release = 0u;
}

static pstvnc_app_mpeg_calibration_result_t fail_closed(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_app_mpeg_calibration_result_t result)
{
    if (calibration != NULL) {
        calibration->state = PSTVNC_APP_MPEG_CALIBRATION_FAULTED;
        calibration->last_result = result;
    }

    return result;
}

static void copy_calibration_rect_to_presentation(
    pstvnc_mpeg_presentation_rect_t *destination,
    const pstvnc_mpeg_calibration_rect_t *source)
{
    destination->x = source->x;
    destination->y = source->y;
    destination->width = source->width;
    destination->height = source->height;
}

static int resolve_accepted_geometry(
    pstvnc_app_mpeg_calibration_t *calibration,
    const pstvnc_mpeg_calibration_region_t *accepted_region)
{
    pstvnc_mpeg_calibration_geometry_t resolved;

    if (calibration == NULL || accepted_region == NULL)
        return 0;

    if (!pstvnc_mpeg_calibration_resolve_geometry(
            &calibration->manual_source.calibration,
            accepted_region,
            &resolved))
        return 0;

    copy_calibration_rect_to_presentation(
        &calibration->accepted_geometry.base,
        &resolved.base);

    copy_calibration_rect_to_presentation(
        &calibration->accepted_geometry.inner_content,
        &resolved.inner_content);

    copy_calibration_rect_to_presentation(
        &calibration->accepted_geometry.suppression,
        &resolved.suppression);

    calibration->accepted_geometry_valid = 1u;
    return 1;
}

static pstvnc_app_mpeg_calibration_result_t
present_visible_calibration(
    pstvnc_app_mpeg_calibration_t *calibration)
{
    pstvnc_mpeg_calibration_manual_plan_t plan;

    if (calibration == NULL)
        return PSTVNC_APP_MPEG_CALIBRATION_INVALID;

    if (!pstvnc_mpeg_calibration_manual_prepare_plan(
            &calibration->manual_source,
            &plan))
        return PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION;

    if (plan.canvas_width != calibration->canvas_width ||
        plan.canvas_height != calibration->canvas_height)
        return PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION;

    if (!pstvnc_mpeg_calibration_manual_rasterize(
            &plan,
            calibration->frozen_desktop,
            calibration->canvas_pixel_count,
            calibration->work_surface,
            calibration->surface_capacity_pixels))
        return PSTVNC_APP_MPEG_CALIBRATION_RASTER_FAILED;

    if (pstvnc_ps2_graphics_present(
            calibration->work_surface,
            calibration->canvas_pixel_count,
            NULL) < 0)
        return PSTVNC_APP_MPEG_CALIBRATION_PRESENT_FAILED;

    return PSTVNC_APP_MPEG_CALIBRATION_OK;
}

static pstvnc_app_mpeg_calibration_result_t
complete_foreground_release(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_app_mpeg_calibration_service_result_t *result)
{
    int accepted;

    if (calibration == NULL ||
        result == NULL ||
        calibration->state != PSTVNC_APP_MPEG_CALIBRATION_ACTIVE ||
        !calibration->mouse_interpretation_suspended ||
        !calibration->rfb_flow_policy ||
        !calibration->input_runtime ||
        !calibration->presentation ||
        !calibration->rfb_flow_policy->frozen ||
        !presentation_is_exact_rfb_only(calibration->presentation) ||
        !manual_source_is_inactive(&calibration->manual_source) ||
        calibration->accepted_pending_release ==
            calibration->cancelled_pending_release)
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION);

    accepted = calibration->accepted_pending_release != 0u;

    if (accepted && !calibration->accepted_geometry_valid)
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION);

    if (pstvnc_ps2_graphics_present(
            calibration->frozen_desktop,
            calibration->canvas_pixel_count,
            NULL) < 0)
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_PRESENT_FAILED);

    if (pstvnc_input_runtime_resume_mouse_interpretation(
            calibration->input_runtime) < 0)
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_INPUT_RESUME_FAILED);

    calibration->mouse_interpretation_suspended = 0u;
    result->foreground_completed = 1u;

    if (accepted) {
        calibration->accepted_pending_release = 0u;
        calibration->state =
            PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED;
        calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
        result->accepted_protected = 1u;
        return PSTVNC_APP_MPEG_CALIBRATION_OK;
    }

    if (!pstvnc_rfb_flow_policy_set_frozen(
            calibration->rfb_flow_policy,
            0))
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_THAW_FAILED);

    calibration->cancelled_pending_release = 0u;
    calibration->accepted_geometry_valid = 0u;
    memset(
        &calibration->accepted_geometry,
        0,
        sizeof(calibration->accepted_geometry));

    calibration->state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    result->cancelled_completed = 1u;
    clear_live_owner_references(calibration);
    return PSTVNC_APP_MPEG_CALIBRATION_OK;
}

int pstvnc_app_mpeg_calibration_init(
    pstvnc_app_mpeg_calibration_t *calibration,
    int32_t canvas_width,
    int32_t canvas_height,
    uint16_t *frozen_desktop,
    uint16_t *work_surface,
    size_t surface_capacity_pixels)
{
    size_t required_pixels;

    if (calibration == NULL ||
        frozen_desktop == NULL ||
        work_surface == NULL ||
        !pixel_count_for_canvas(
            canvas_width,
            canvas_height,
            &required_pixels) ||
        surface_capacity_pixels < required_pixels ||
        pixel_ranges_overlap(
            frozen_desktop,
            required_pixels,
            work_surface,
            required_pixels))
        return 0;

    memset(calibration, 0, sizeof(*calibration));

    if (!pstvnc_mpeg_calibration_manual_init(
            &calibration->manual_source,
            canvas_width,
            canvas_height))
        return 0;

    calibration->frozen_desktop = frozen_desktop;
    calibration->work_surface = work_surface;
    calibration->surface_capacity_pixels = surface_capacity_pixels;
    calibration->canvas_pixel_count = required_pixels;
    calibration->canvas_width = canvas_width;
    calibration->canvas_height = canvas_height;
    calibration->state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    return 1;
}

pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_begin(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_rfb_session_t *rfb_session,
    const pstvnc_local_ui_t *local_ui,
    const pstvnc_mpeg_presentation_t *presentation,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char *published_click_buttons,
    const uint16_t *last_presented_desktop,
    size_t last_presented_pixel_count)
{
    pstvnc_app_mpeg_calibration_result_t present_result;

    if (calibration == NULL ||
        rfb_flow_policy == NULL ||
        input_runtime == NULL ||
        rfb_session == NULL ||
        local_ui == NULL ||
        presentation == NULL ||
        published_click_buttons == NULL ||
        last_presented_desktop == NULL)
        return PSTVNC_APP_MPEG_CALIBRATION_INVALID;

    if (calibration->state == PSTVNC_APP_MPEG_CALIBRATION_FAULTED)
        return PSTVNC_APP_MPEG_CALIBRATION_ALREADY_FAULTED;

    if (calibration->state != PSTVNC_APP_MPEG_CALIBRATION_IDLE ||
        local_ui->foreground != PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP ||
        local_ui->input_quarantined ||
        !presentation_is_exact_rfb_only(presentation) ||
        rfb_flow_policy->frozen ||
        !manual_source_is_inactive(&calibration->manual_source) ||
        last_presented_pixel_count < calibration->canvas_pixel_count ||
        pixel_ranges_overlap(
            last_presented_desktop,
            calibration->canvas_pixel_count,
            calibration->frozen_desktop,
            calibration->canvas_pixel_count) ||
        pixel_ranges_overlap(
            last_presented_desktop,
            calibration->canvas_pixel_count,
            calibration->work_surface,
            calibration->canvas_pixel_count))
        return PSTVNC_APP_MPEG_CALIBRATION_ADMISSION_REJECTED;

    calibration->rfb_flow_policy = rfb_flow_policy;
    calibration->input_runtime = input_runtime;
    calibration->rfb_session = rfb_session;
    calibration->local_ui = local_ui;
    calibration->presentation = presentation;
    calibration->accepted_pending_release = 0u;
    calibration->cancelled_pending_release = 0u;
    calibration->accepted_geometry_valid = 0u;
    memset(
        &calibration->accepted_geometry,
        0,
        sizeof(calibration->accepted_geometry));

    if (!pstvnc_rfb_flow_policy_set_frozen(
            rfb_flow_policy,
            1)) {
        clear_live_owner_references(calibration);
        calibration->last_result =
            PSTVNC_APP_MPEG_CALIBRATION_FREEZE_FAILED;
        return PSTVNC_APP_MPEG_CALIBRATION_FREEZE_FAILED;
    }

    calibration->state = PSTVNC_APP_MPEG_CALIBRATION_ACTIVE;

    if (pstvnc_input_runtime_suspend_mouse_interpretation(
            input_runtime) < 0)
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_INPUT_SUSPEND_FAILED);

    calibration->mouse_interpretation_suspended = 1u;

    if (*published_click_buttons != 0u) {
        if (published_cursor_x > UINT16_MAX ||
            published_cursor_y > UINT16_MAX ||
            !pstvnc_rfb_session_send_pointer_event(
                rfb_session,
                0,
                (uint16_t)published_cursor_x,
                (uint16_t)published_cursor_y))
            return fail_closed(
                calibration,
                PSTVNC_APP_MPEG_CALIBRATION_POINTER_RELEASE_FAILED);

        *published_click_buttons = 0u;
    }

    if (pstvnc_input_runtime_rebase_suspended_mouse_state(
            input_runtime,
            published_cursor_x,
            published_cursor_y,
            *published_click_buttons) < 0)
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_INPUT_REBASE_FAILED);

    if (last_presented_pixel_count < calibration->canvas_pixel_count)
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_SNAPSHOT_FAILED);

    memcpy(
        calibration->frozen_desktop,
        last_presented_desktop,
        calibration->canvas_pixel_count *
            sizeof(*calibration->frozen_desktop));

    if (!pstvnc_mpeg_calibration_manual_begin(
            &calibration->manual_source))
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_MANUAL_BEGIN_FAILED);

    present_result = present_visible_calibration(calibration);
    if (present_result != PSTVNC_APP_MPEG_CALIBRATION_OK)
        return fail_closed(calibration, present_result);

    calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    return PSTVNC_APP_MPEG_CALIBRATION_OK;
}

pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_service_controller(
    pstvnc_app_mpeg_calibration_t *calibration,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_app_mpeg_calibration_service_result_t *result)
{
    pstvnc_mpeg_calibration_manual_result_t manual_result;
    pstvnc_app_mpeg_calibration_result_t present_result;

    if (calibration == NULL ||
        controller_state == NULL ||
        result == NULL)
        return PSTVNC_APP_MPEG_CALIBRATION_INVALID;

    memset(result, 0, sizeof(*result));

    if (calibration->state == PSTVNC_APP_MPEG_CALIBRATION_FAULTED)
        return PSTVNC_APP_MPEG_CALIBRATION_ALREADY_FAULTED;

    if (calibration->state != PSTVNC_APP_MPEG_CALIBRATION_ACTIVE)
        return PSTVNC_APP_MPEG_CALIBRATION_NOT_ACTIVE;

    if (calibration->rfb_flow_policy == NULL ||
        calibration->input_runtime == NULL ||
        calibration->rfb_session == NULL ||
        calibration->local_ui == NULL ||
        calibration->presentation == NULL ||
        !calibration->rfb_flow_policy->frozen ||
        !calibration->mouse_interpretation_suspended ||
        !presentation_is_exact_rfb_only(calibration->presentation))
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION);

    if (!pstvnc_mpeg_calibration_manual_service_controller(
            &calibration->manual_source,
            controller_state,
            &manual_result))
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_MANUAL_SERVICE_FAILED);

    result->consume_controller_state =
        manual_result.consume_controller_state ? 1u : 0u;
    result->accepted_edge = manual_result.accepted ? 1u : 0u;
    result->cancelled_edge = manual_result.cancelled ? 1u : 0u;

    if (!result->consume_controller_state ||
        (manual_result.accepted && manual_result.cancelled) ||
        (manual_result.accepted &&
            (calibration->accepted_pending_release ||
             calibration->cancelled_pending_release)) ||
        (manual_result.cancelled &&
            (calibration->accepted_pending_release ||
             calibration->cancelled_pending_release)))
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION);

    if (manual_result.accepted) {
        if (!resolve_accepted_geometry(
                calibration,
                &manual_result.accepted_region))
            return fail_closed(
                calibration,
                PSTVNC_APP_MPEG_CALIBRATION_GEOMETRY_FAILED);

        calibration->accepted_pending_release = 1u;
    }

    if (manual_result.cancelled)
        calibration->cancelled_pending_release = 1u;

    if (manual_result.release_quarantine_complete)
        return complete_foreground_release(
            calibration,
            result);

    if (calibration->manual_source.calibration.screen ==
            PSTVNC_MPEG_CALIBRATION_INACTIVE) {
        if (!calibration->manual_source.calibration.release_quarantine_pending ||
            calibration->accepted_pending_release ==
                calibration->cancelled_pending_release)
            return fail_closed(
                calibration,
                PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION);

        calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
        return PSTVNC_APP_MPEG_CALIBRATION_OK;
    }

    present_result = present_visible_calibration(calibration);
    if (present_result != PSTVNC_APP_MPEG_CALIBRATION_OK)
        return fail_closed(calibration, present_result);

    calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    return PSTVNC_APP_MPEG_CALIBRATION_OK;
}

int pstvnc_app_mpeg_calibration_copy_accepted_geometry(
    const pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_mpeg_presentation_geometry_t *geometry)
{
    if (calibration == NULL ||
        geometry == NULL ||
        calibration->state !=
            PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED ||
        !calibration->accepted_geometry_valid ||
        calibration->rfb_flow_policy == NULL ||
        !calibration->rfb_flow_policy->frozen ||
        !presentation_is_exact_rfb_only(calibration->presentation))
        return 0;

    *geometry = calibration->accepted_geometry;
    return 1;
}

pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_abort_accepted(
    pstvnc_app_mpeg_calibration_t *calibration)
{
    if (calibration == NULL)
        return PSTVNC_APP_MPEG_CALIBRATION_INVALID;

    if (calibration->state == PSTVNC_APP_MPEG_CALIBRATION_FAULTED)
        return PSTVNC_APP_MPEG_CALIBRATION_ALREADY_FAULTED;

    if (calibration->state !=
            PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED)
        return PSTVNC_APP_MPEG_CALIBRATION_NOT_ACCEPTED_PROTECTED;

    if (calibration->rfb_flow_policy == NULL ||
        !calibration->rfb_flow_policy->frozen ||
        calibration->mouse_interpretation_suspended ||
        !calibration->accepted_geometry_valid ||
        !presentation_is_exact_rfb_only(calibration->presentation) ||
        !manual_source_is_inactive(&calibration->manual_source))
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION);

    if (!pstvnc_rfb_flow_policy_set_frozen(
            calibration->rfb_flow_policy,
            0))
        return fail_closed(
            calibration,
            PSTVNC_APP_MPEG_CALIBRATION_THAW_FAILED);

    calibration->accepted_geometry_valid = 0u;
    memset(
        &calibration->accepted_geometry,
        0,
        sizeof(calibration->accepted_geometry));

    calibration->state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    calibration->last_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    clear_live_owner_references(calibration);
    return PSTVNC_APP_MPEG_CALIBRATION_OK;
}

pstvnc_app_mpeg_calibration_state_t
pstvnc_app_mpeg_calibration_state(
    const pstvnc_app_mpeg_calibration_t *calibration)
{
    if (calibration == NULL)
        return PSTVNC_APP_MPEG_CALIBRATION_FAULTED;

    return calibration->state;
}
