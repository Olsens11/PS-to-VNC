/*
 * File synopsis:
 * Implements the H1 experiment-local foreground ownership sequence that
 * surrounds MPEG presentation calibration.
 */

#include "h1_mpeg_calibration_foreground.h"

#include <stddef.h>
#include <string.h>

void pstvnc_h1_mpeg_calibration_foreground_init(
    pstvnc_h1_mpeg_calibration_foreground_t *foreground,
    int canvas_width,
    int canvas_height)
{
    if (foreground == NULL)
        return;

    memset(foreground, 0, sizeof(*foreground));
    pstvnc_h1_mpeg_calibration_adapter_init(
        &foreground->adapter,
        canvas_width,
        canvas_height);
}

int pstvnc_h1_mpeg_calibration_foreground_service_controller(
    pstvnc_h1_mpeg_calibration_foreground_t *foreground,
    const pstvnc_controller_state_t *controller_state,
    const pstvnc_h1_mpeg_calibration_foreground_ops_t *ops,
    void *ops_context,
    pstvnc_h1_mpeg_calibration_foreground_result_t *result)
{
    pstvnc_h1_mpeg_calibration_adapter_result_t adapter_result;

    if (foreground == NULL || controller_state == NULL ||
        ops == NULL || result == NULL ||
        ops->suspend_mouse_interpretation == NULL ||
        ops->neutralize_published_pointer == NULL ||
        ops->rebase_suspended_mouse_state == NULL ||
        ops->resume_mouse_interpretation == NULL)
        return 0;

    memset(result, 0, sizeof(*result));

    if (!pstvnc_h1_mpeg_calibration_adapter_service_controller(
            &foreground->adapter,
            controller_state,
            &adapter_result))
        return 0;

    result->adapter_result = adapter_result;
    result->consume_controller_state =
        adapter_result.consume_controller_state;

    if (adapter_result.calibration_effects.enter_foreground) {
        /*
         * The adapter has already claimed the entry sample, so fail closed from
         * this point onward. RFB freeze is established before invoking the
         * input-runtime transition and remains asserted on callback failure.
         */
        foreground->rfb_visuals_frozen = 1;

        if (!ops->suspend_mouse_interpretation(ops_context))
            return 0;

        foreground->mouse_interpretation_suspended = 1;

        if (!ops->neutralize_published_pointer(ops_context) ||
            !ops->rebase_suspended_mouse_state(ops_context))
            return 0;
    }

    if (adapter_result.freeze_rfb_visuals)
        foreground->rfb_visuals_frozen = 1;

    if (adapter_result.calibration_effects.release_quarantine_complete) {
        /*
         * Physical release is proven. Only now may ordinary mouse semantics
         * resume. If resume fails, keep both safety facts asserted.
         */
        if (foreground->mouse_interpretation_suspended) {
            if (!ops->resume_mouse_interpretation(ops_context))
                return 0;

            foreground->mouse_interpretation_suspended = 0;
        }

        foreground->rfb_visuals_frozen = 0;
    }

    result->freeze_rfb_visuals = foreground->rfb_visuals_frozen;
    return 1;
}
