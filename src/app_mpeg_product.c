/*
 * File synopsis:
 * Implements R34's ordinary MPEG product composition using only accepted
 * Application/lower-owner public seams. It owns semantic routing and
 * cross-domain sequencing, not gesture recognition, calibration mechanics,
 * generation identity, decoding, presentation mechanisms, Transport validity,
 * or normal retirement/reveal.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34.
 */

#include <string.h>

#include "app_mpeg_product.h"

static int run_state_is_live_serviceable(
    pstvnc_app_mpeg_run_state_t state)
{
    return state == PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME ||
        state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED;
}

int pstvnc_app_mpeg_product_init(
    pstvnc_app_mpeg_product_t *product,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_media_clock_t *media_clock,
    int32_t canvas_width,
    int32_t canvas_height,
    uint16_t *frozen_desktop,
    uint16_t *work_surface,
    size_t surface_capacity_pixels)
{
    if (product == NULL ||
        transport_access == NULL ||
        transport_access->opaque_ticket == 0u ||
        media_clock == NULL)
        return 0;

    memset(product, 0, sizeof(*product));
    product->transport_access = *transport_access;
    product->media_clock = media_clock;

    pstvnc_mpeg_presentation_init(&product->presentation);
    pstvnc_app_mpeg_run_init(&product->run);

    if (!pstvnc_app_mpeg_calibration_init(
            &product->calibration,
            canvas_width,
            canvas_height,
            frozen_desktop,
            work_surface,
            surface_capacity_pixels))
        return 0;

    product->initialized = 1;
    return 1;
}

int pstvnc_app_mpeg_product_desktop_action_eligible(
    const pstvnc_app_mpeg_product_t *product,
    const pstvnc_local_ui_t *local_ui)
{
    if (product == NULL ||
        local_ui == NULL ||
        !product->initialized)
        return 0;

    return local_ui->foreground == PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP &&
        !pstvnc_local_ui_input_is_quarantined(local_ui) &&
        pstvnc_app_mpeg_calibration_state(&product->calibration) ==
            PSTVNC_APP_MPEG_CALIBRATION_IDLE &&
        product->run.state == PSTVNC_APP_MPEG_RUN_IDLE &&
        pstvnc_mpeg_presentation_state(&product->presentation) ==
            PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
}

pstvnc_app_mpeg_product_result_t
pstvnc_app_mpeg_product_route_action(
    pstvnc_app_mpeg_product_t *product,
    pstvnc_product_action_t action,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_rfb_session_t *rfb_session,
    const pstvnc_local_ui_t *local_ui,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char *published_click_buttons,
    const uint16_t *last_presented_desktop,
    size_t last_presented_pixel_count)
{
    if (product == NULL ||
        !product->initialized ||
        !pstvnc_product_action_is_valid(action))
        return PSTVNC_APP_MPEG_PRODUCT_INVALID;

    /*
     * R34 owns only MPEG_CALIBRATION. A semantic action observed while another
     * calibration/run owns the lifecycle is consumed as inadmissible, never
     * translated into overlapping work.
     */
    if (action != PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION)
        return PSTVNC_APP_MPEG_PRODUCT_INVALID;

    if (!pstvnc_app_mpeg_product_desktop_action_eligible(
            product,
            local_ui))
        return PSTVNC_APP_MPEG_PRODUCT_OK;

    if (pstvnc_app_mpeg_calibration_begin(
            &product->calibration,
            rfb_flow_policy,
            input_runtime,
            rfb_session,
            local_ui,
            &product->presentation,
            published_cursor_x,
            published_cursor_y,
            published_click_buttons,
            last_presented_desktop,
            last_presented_pixel_count) !=
        PSTVNC_APP_MPEG_CALIBRATION_OK)
        return PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE;

    return PSTVNC_APP_MPEG_PRODUCT_OK;
}

pstvnc_app_mpeg_product_result_t
pstvnc_app_mpeg_product_service_controller(
    pstvnc_app_mpeg_product_t *product,
    const pstvnc_controller_state_t *controller_state,
    int *consumed)
{
    pstvnc_app_mpeg_calibration_service_result_t service_result;
    pstvnc_app_mpeg_activation_result_t activation_result;

    if (product == NULL ||
        controller_state == NULL ||
        consumed == NULL ||
        !product->initialized)
        return PSTVNC_APP_MPEG_PRODUCT_INVALID;

    *consumed = 0;

    if (pstvnc_app_mpeg_calibration_state(&product->calibration) !=
        PSTVNC_APP_MPEG_CALIBRATION_ACTIVE)
        return PSTVNC_APP_MPEG_PRODUCT_OK;

    memset(&service_result, 0, sizeof(service_result));

    if (pstvnc_app_mpeg_calibration_service_controller(
            &product->calibration,
            controller_state,
            &service_result) != PSTVNC_APP_MPEG_CALIBRATION_OK)
        return PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE;

    *consumed = service_result.consume_controller_state ? 1 : 0;

    if (!service_result.accepted_protected)
        return PSTVNC_APP_MPEG_PRODUCT_OK;

    activation_result = pstvnc_app_mpeg_activation_start_protected(
        &product->calibration,
        &product->run,
        &product->transport_access,
        product->media_clock);

    if (activation_result == PSTVNC_APP_MPEG_ACTIVATION_OK ||
        activation_result ==
            PSTVNC_APP_MPEG_ACTIVATION_START_ROLLED_BACK)
        return PSTVNC_APP_MPEG_PRODUCT_OK;

    return PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE;
}

pstvnc_app_mpeg_product_result_t
pstvnc_app_mpeg_product_service_live(
    pstvnc_app_mpeg_product_t *product,
    uint64_t current_tick)
{
    pstvnc_app_mpeg_frame_service_result_t service_result;

    if (product == NULL || !product->initialized)
        return PSTVNC_APP_MPEG_PRODUCT_INVALID;

    if (!run_state_is_live_serviceable(product->run.state))
        return PSTVNC_APP_MPEG_PRODUCT_OK;

    memset(&service_result, 0, sizeof(service_result));

    if (pstvnc_app_mpeg_run_service(
            &product->run,
            current_tick,
            &service_result) != PSTVNC_APP_MPEG_RUN_OK)
        return PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE;

    return PSTVNC_APP_MPEG_PRODUCT_OK;
}

int pstvnc_app_mpeg_product_has_started_run(
    const pstvnc_app_mpeg_product_t *product)
{
    if (product == NULL || !product->initialized)
        return 0;

    return product->run.current_generation != 0u &&
        product->run.transport_run_open &&
        product->run.worker_runtime_owned &&
        product->run.worker_started &&
        product->run.frame_consumer_initialized &&
        product->run.presentation_armed &&
        product->run.start_invoked;
}

pstvnc_app_mpeg_product_result_t
pstvnc_app_mpeg_product_service_session_abort(
    pstvnc_app_mpeg_product_t *product,
    int *abort_ready)
{
    if (product == NULL ||
        abort_ready == NULL ||
        !product->initialized ||
        !pstvnc_app_mpeg_product_has_started_run(product))
        return PSTVNC_APP_MPEG_PRODUCT_INVALID;

    *abort_ready = 0;

    if (pstvnc_app_mpeg_run_session_abort_service(
            &product->run) != PSTVNC_APP_MPEG_RUN_OK)
        return PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE;

    if (product->run.state == PSTVNC_APP_MPEG_RUN_SESSION_ABORT_READY)
        *abort_ready = 1;

    return PSTVNC_APP_MPEG_PRODUCT_OK;
}
