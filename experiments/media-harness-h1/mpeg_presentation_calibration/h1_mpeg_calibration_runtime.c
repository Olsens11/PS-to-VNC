/*
 * File synopsis:
 * Composes the experiment-local MPEG calibration foreground bridge, RFB gate,
 * and render-plan preparation into the narrow runtime seam consumed by H1.
 */
#include "h1_mpeg_calibration_runtime.h"

#include <string.h>

void pstvnc_h1_mpeg_calibration_runtime_init(
    pstvnc_h1_mpeg_calibration_runtime_t *runtime,
    int canvas_width,
    int canvas_height)
{
    if (runtime == NULL)
        return;

    memset(runtime, 0, sizeof(*runtime));
    pstvnc_h1_mpeg_calibration_foreground_init(
        &runtime->foreground,
        canvas_width,
        canvas_height);
    pstvnc_h1_mpeg_calibration_rfb_gate_init(&runtime->rfb_gate);
}

int pstvnc_h1_mpeg_calibration_runtime_service_controller(
    pstvnc_h1_mpeg_calibration_runtime_t *runtime,
    const pstvnc_controller_state_t *controller_state,
    const pstvnc_h1_mpeg_calibration_foreground_ops_t *foreground_ops,
    void *foreground_context,
    pstvnc_h1_mpeg_calibration_runtime_result_t *result)
{
    pstvnc_h1_mpeg_calibration_foreground_result_t foreground_result;

    if (runtime == NULL || controller_state == NULL ||
        foreground_ops == NULL || result == NULL)
        return 0;

    memset(result, 0, sizeof(*result));
    memset(&foreground_result, 0, sizeof(foreground_result));

    if (!pstvnc_h1_mpeg_calibration_foreground_service_controller(
            &runtime->foreground,
            controller_state,
            foreground_ops,
            foreground_context,
            &foreground_result))
        return 0;

    /*
     * This state transition is recorded immediately, but the RFB owner queries
     * and acts on the gate only at its already-existing safe scheduling point.
     */
    if (!pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
            &runtime->rfb_gate,
            foreground_result.freeze_rfb_visuals))
        return 0;

    result->consume_controller_state =
        foreground_result.consume_controller_state;
    result->freeze_rfb_visuals = foreground_result.freeze_rfb_visuals;
    result->calibration_visible =
        pstvnc_mpeg_calibration_owns_foreground(
            &runtime->foreground.adapter.calibration) &&
        runtime->foreground.adapter.calibration.screen !=
            PSTVNC_MPEG_CAL_INACTIVE;

    /*
     * Preserve the existing one-shot acceptance fact rather than reconstructing
     * it later from committed geometry or foreground transitions.
     */
    result->accepted =
        foreground_result.adapter_result.calibration_effects.accepted;

    return 1;
}

int pstvnc_h1_mpeg_calibration_runtime_prepare_render_plan(
    const pstvnc_h1_mpeg_calibration_runtime_t *runtime,
    pstvnc_h1_mpeg_calibration_render_plan_t *plan)
{
    if (runtime == NULL || plan == NULL)
        return 0;

    return pstvnc_h1_mpeg_calibration_prepare_render_plan(
        &runtime->foreground.adapter.calibration,
        plan);
}

int pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(
    const pstvnc_h1_mpeg_calibration_runtime_t *runtime)
{
    if (runtime == NULL)
        return 0;

    return pstvnc_h1_mpeg_calibration_rfb_gate_allows_request(
        &runtime->rfb_gate);
}

int pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(
    const pstvnc_h1_mpeg_calibration_runtime_t *runtime)
{
    if (runtime == NULL)
        return 0;

    return pstvnc_h1_mpeg_calibration_rfb_gate_allows_remote_present(
        &runtime->rfb_gate);
}

int pstvnc_h1_mpeg_calibration_runtime_take_full_refresh(
    pstvnc_h1_mpeg_calibration_runtime_t *runtime)
{
    if (runtime == NULL)
        return 0;

    return pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(
        &runtime->rfb_gate);
}

const pstvnc_mpeg_cal_region_t *
pstvnc_h1_mpeg_calibration_runtime_committed_region(
    const pstvnc_h1_mpeg_calibration_runtime_t *runtime)
{
    const pstvnc_mpeg_calibration_t *calibration;

    if (runtime == NULL)
        return NULL;

    calibration = &runtime->foreground.adapter.calibration;
    if (!calibration->has_committed)
        return NULL;

    return &calibration->committed;
}
