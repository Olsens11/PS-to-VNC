/*
 * File synopsis:
 * Implements the A004 P10 protected P9 -> R21 ownership transfer.
 *
 * The coordinator is intentionally stateless with respect to MPEG generation:
 * app_mpeg_run remains the sole generation allocator/history owner. P10 only
 * sequences public Application seams and verifies owner facts before/after the
 * one R21 start invocation.
 */

#include "app_mpeg_activation.h"

#include <stdint.h>

static int rect_equal(
    const pstvnc_mpeg_presentation_rect_t *left,
    const pstvnc_mpeg_presentation_rect_t *right)
{
    return left != NULL &&
        right != NULL &&
        left->x == right->x &&
        left->y == right->y &&
        left->width == right->width &&
        left->height == right->height;
}

static int geometry_equal(
    const pstvnc_mpeg_presentation_geometry_t *left,
    const pstvnc_mpeg_presentation_geometry_t *right)
{
    return left != NULL &&
        right != NULL &&
        rect_equal(&left->base, &right->base) &&
        rect_equal(&left->inner_content, &right->inner_content) &&
        rect_equal(&left->suppression, &right->suppression);
}

static int exact_rfb_only_no_snapshot(
    const pstvnc_mpeg_presentation_t *presentation)
{
    pstvnc_mpeg_presentation_geometry_t geometry;
    uint32_t generation = 0u;

    if (presentation == NULL ||
        pstvnc_mpeg_presentation_state(presentation) !=
            PSTVNC_MPEG_PRESENTATION_RFB_ONLY)
        return 0;

    return !pstvnc_mpeg_presentation_snapshot(
        presentation,
        &geometry,
        &generation);
}

static int clean_pre_start_failure_proven(
    const pstvnc_app_mpeg_run_t *run,
    const pstvnc_mpeg_presentation_t *presentation)
{
    pstvnc_app_mpeg_run_status_t status;

    if (pstvnc_app_mpeg_run_status(run, &status) !=
            PSTVNC_APP_MPEG_RUN_OK)
        return 0;

    return status.state == PSTVNC_APP_MPEG_RUN_IDLE &&
        status.current_generation == 0u &&
        !status.session_teardown_required &&
        exact_rfb_only_no_snapshot(presentation);
}

static int exact_wait_first_frame_proven(
    const pstvnc_app_mpeg_run_t *run,
    const pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    const pstvnc_mpeg_presentation_t *presentation,
    const pstvnc_mpeg_presentation_geometry_t *expected_geometry,
    uint32_t *generation)
{
    pstvnc_app_mpeg_run_status_t status;
    pstvnc_mpeg_presentation_geometry_t actual_geometry;
    uint32_t actual_generation = 0u;

    if (generation == NULL ||
        pstvnc_app_mpeg_run_status(run, &status) !=
            PSTVNC_APP_MPEG_RUN_OK ||
        status.state !=
            PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME ||
        status.current_generation == 0u ||
        status.session_teardown_required ||
        rfb_flow_policy == NULL ||
        !rfb_flow_policy->frozen ||
        pstvnc_rfb_flow_policy_allows_remote_publication(
            rfb_flow_policy) ||
        presentation == NULL ||
        pstvnc_mpeg_presentation_state(presentation) !=
            PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||
        !pstvnc_mpeg_presentation_snapshot(
            presentation,
            &actual_geometry,
            &actual_generation) ||
        actual_generation != status.current_generation ||
        !geometry_equal(expected_geometry, &actual_geometry))
        return 0;

    *generation = status.current_generation;
    return 1;
}

static pstvnc_app_mpeg_activation_result_t
fault_protected_handoff(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_app_mpeg_activation_result_t result)
{
    if (calibration != NULL &&
        calibration->state ==
            PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED)
        (void)pstvnc_app_mpeg_calibration_fault_protected_handoff(
            calibration);

    return result;
}

pstvnc_app_mpeg_activation_result_t
pstvnc_app_mpeg_activation_start_protected(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_app_mpeg_run_t *run,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_media_clock_t *media_clock)
{
    pstvnc_mpeg_presentation_geometry_t geometry;
    pstvnc_app_mpeg_run_status_t status;
    pstvnc_app_mpeg_run_result_t start_result;
    pstvnc_rfb_flow_policy_t *rfb_flow_policy;
    pstvnc_mpeg_presentation_t *presentation;
    uint32_t generation = 0u;

    if (calibration == NULL ||
        run == NULL ||
        transport_access == NULL ||
        media_clock == NULL)
        return PSTVNC_APP_MPEG_ACTIVATION_INVALID;

    if (pstvnc_app_mpeg_run_status(run, &status) !=
            PSTVNC_APP_MPEG_RUN_OK ||
        status.state != PSTVNC_APP_MPEG_RUN_IDLE ||
        status.current_generation != 0u ||
        status.session_teardown_required)
        return PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED;

    /*
     * P9 owns the protected geometry until successful handoff commit. This read
     * is deliberately completed before the one R21 mutation boundary.
     */
    if (!pstvnc_app_mpeg_calibration_copy_accepted_geometry(
            calibration,
            &geometry))
        return PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED;

    rfb_flow_policy = calibration->rfb_flow_policy;
    presentation = calibration->presentation;

    if (rfb_flow_policy == NULL ||
        !rfb_flow_policy->frozen ||
        pstvnc_rfb_flow_policy_allows_remote_publication(
            rfb_flow_policy) ||
        !exact_rfb_only_no_snapshot(presentation))
        return PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED;

    start_result = pstvnc_app_mpeg_run_start(
        run,
        &geometry,
        rfb_flow_policy,
        transport_access,
        presentation,
        media_clock);

    if (start_result != PSTVNC_APP_MPEG_RUN_OK) {
        if (rfb_flow_policy->frozen &&
            clean_pre_start_failure_proven(
                run,
                presentation)) {
            if (pstvnc_app_mpeg_calibration_abort_accepted(
                    calibration) ==
                    PSTVNC_APP_MPEG_CALIBRATION_OK)
                return PSTVNC_APP_MPEG_ACTIVATION_START_ROLLED_BACK;

            return fault_protected_handoff(
                calibration,
                PSTVNC_APP_MPEG_ACTIVATION_ROLLBACK_TEARDOWN_REQUIRED);
        }

        return fault_protected_handoff(
            calibration,
            PSTVNC_APP_MPEG_ACTIVATION_START_FAILURE_TEARDOWN_REQUIRED);
    }

    if (!exact_wait_first_frame_proven(
            run,
            rfb_flow_policy,
            presentation,
            &geometry,
            &generation))
        return fault_protected_handoff(
            calibration,
            PSTVNC_APP_MPEG_ACTIVATION_SUCCESS_PROOF_TEARDOWN_REQUIRED);

    if (pstvnc_app_mpeg_calibration_commit_protected_handoff(
            calibration,
            generation) != PSTVNC_APP_MPEG_CALIBRATION_OK)
        return fault_protected_handoff(
            calibration,
            PSTVNC_APP_MPEG_ACTIVATION_HANDOFF_COMMIT_TEARDOWN_REQUIRED);

    /*
     * P9 is now reusable IDLE and no longer holds accepted geometry. Re-prove
     * the downstream owner after commit so success cannot hide a handoff side
     * effect that disturbed P2/P3/run authority.
     */
    if (calibration->state != PSTVNC_APP_MPEG_CALIBRATION_IDLE ||
        calibration->accepted_geometry_valid ||
        !exact_wait_first_frame_proven(
            run,
            rfb_flow_policy,
            presentation,
            &geometry,
            &generation))
        return PSTVNC_APP_MPEG_ACTIVATION_HANDOFF_COMMIT_TEARDOWN_REQUIRED;

    return PSTVNC_APP_MPEG_ACTIVATION_OK;
}
