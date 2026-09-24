/*
 * File synopsis:
 * Implements R21's Application-owned MPEG run-start composition transaction.
 * It validates one resolved geometry/profile snapshot, requires pre-existing
 * RFB publication protection, opens one clean Transport MPEG run, constructs
 * fresh PS2 execution owners, arms Presentation/P7, then sends START last.
 *
 * Every pre-START failure attempts reverse-order retirement. A cleanup failure
 * faults the coordinator instead of claiming IDLE. Once START is invoked, even
 * a non-OK result is treated as irreversible and requires outer session
 * teardown; pre-START abort is never used after that boundary.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-APPLICATION-MPEG-RUN-START-R21.
 */

#include "app_mpeg_run.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int pstvnc_app_mpeg_run_rect_end(
    const pstvnc_mpeg_presentation_rect_t *rect,
    int64_t *right,
    int64_t *bottom)
{
    if (rect == NULL ||
        right == NULL ||
        bottom == NULL ||
        rect->x < 0 ||
        rect->y < 0 ||
        rect->width <= 0 ||
        rect->height <= 0)
        return 0;

    *right = (int64_t)rect->x + (int64_t)rect->width;
    *bottom = (int64_t)rect->y + (int64_t)rect->height;
    return 1;
}

static int pstvnc_app_mpeg_run_geometry_valid(
    const pstvnc_mpeg_presentation_geometry_t *geometry,
    const pstvnc_config_mpeg_runtime_profile_t *profile)
{
    int64_t base_right;
    int64_t base_bottom;
    int64_t inner_right;
    int64_t inner_bottom;
    int64_t suppression_right;
    int64_t suppression_bottom;

    if (geometry == NULL ||
        profile == NULL ||
        geometry->base.width < 16 ||
        geometry->base.height < 16 ||
        (geometry->base.width & 15) != 0 ||
        (geometry->base.height & 15) != 0 ||
        (uint32_t)geometry->base.width > profile->decoder.max_width ||
        (uint32_t)geometry->base.height > profile->decoder.max_height)
        return 0;

    if (!pstvnc_app_mpeg_run_rect_end(
            &geometry->base, &base_right, &base_bottom) ||
        !pstvnc_app_mpeg_run_rect_end(
            &geometry->inner_content, &inner_right, &inner_bottom) ||
        !pstvnc_app_mpeg_run_rect_end(
            &geometry->suppression,
            &suppression_right,
            &suppression_bottom))
        return 0;

    if (geometry->inner_content.x < geometry->base.x ||
        geometry->inner_content.y < geometry->base.y ||
        inner_right > base_right ||
        inner_bottom > base_bottom)
        return 0;

    if ((int64_t)geometry->inner_content.x - geometry->base.x !=
            base_right - inner_right ||
        (int64_t)geometry->inner_content.y - geometry->base.y !=
            base_bottom - inner_bottom)
        return 0;

    if (geometry->suppression.x > geometry->base.x ||
        geometry->suppression.y > geometry->base.y ||
        suppression_right < base_right ||
        suppression_bottom < base_bottom)
        return 0;

    if (base_right > (int64_t)UINT32_MAX ||
        base_bottom > (int64_t)UINT32_MAX ||
        suppression_right > (int64_t)UINT32_MAX ||
        suppression_bottom > (int64_t)UINT32_MAX)
        return 0;

    return 1;
}

static int pstvnc_app_mpeg_run_geometry_equal(
    const pstvnc_mpeg_presentation_geometry_t *left,
    const pstvnc_mpeg_presentation_geometry_t *right)
{
    return left != NULL &&
        right != NULL &&
        left->base.x == right->base.x &&
        left->base.y == right->base.y &&
        left->base.width == right->base.width &&
        left->base.height == right->base.height &&
        left->inner_content.x == right->inner_content.x &&
        left->inner_content.y == right->inner_content.y &&
        left->inner_content.width == right->inner_content.width &&
        left->inner_content.height == right->inner_content.height &&
        left->suppression.x == right->suppression.x &&
        left->suppression.y == right->suppression.y &&
        left->suppression.width == right->suppression.width &&
        left->suppression.height == right->suppression.height;
}

static void pstvnc_app_mpeg_run_clear_attempt(
    pstvnc_app_mpeg_run_t *run)
{
    memset(&run->transport_access, 0, sizeof(run->transport_access));
    memset(&run->profile, 0, sizeof(run->profile));
    memset(&run->worker_runtime, 0, sizeof(run->worker_runtime));
    memset(&run->decoder_backend, 0, sizeof(run->decoder_backend));
    memset(&run->worker, 0, sizeof(run->worker));
    memset(&run->frame_consumer, 0, sizeof(run->frame_consumer));

    run->presentation = NULL;
    run->rfb_flow_policy = NULL;
    run->media_clock = NULL;

    run->current_generation = 0u;
    run->session_teardown_required = 0;
    run->transport_run_open = 0;
    run->worker_runtime_owned = 0;
    run->worker_started = 0;
    run->presentation_armed = 0;
    run->frame_consumer_initialized = 0;
    run->start_invoked = 0;
}

static void pstvnc_app_mpeg_run_fault(
    pstvnc_app_mpeg_run_t *run,
    pstvnc_app_mpeg_run_result_t result)
{
    run->state = PSTVNC_APP_MPEG_RUN_FAULTED;
    run->last_result = result;
    run->session_teardown_required = 1;
}

static int pstvnc_app_mpeg_run_unwind_pre_start(
    pstvnc_app_mpeg_run_t *run)
{
    int clean = 1;
    int worker_safe = 1;
    pstvnc_mpeg_worker_outcome_t outcome;

    if (run->frame_consumer_initialized) {
        memset(&run->frame_consumer, 0, sizeof(run->frame_consumer));
        run->frame_consumer_initialized = 0;
    }

    if (run->presentation_armed) {
        if (!pstvnc_mpeg_presentation_abort_pending(
                run->presentation,
                run->current_generation)) {
            clean = 0;
        } else {
            run->presentation_armed = 0;
        }
    }

    if (run->worker_started) {
        if (pstvnc_mpeg_worker_request_stop(
                &run->worker,
                run->current_generation) != PSTVNC_MPEG_WORKER_OK) {
            clean = 0;
            worker_safe = 0;
        }

        if (worker_safe &&
            pstvnc_mpeg_worker_join(
                &run->worker,
                run->current_generation) != PSTVNC_MPEG_WORKER_OK) {
            clean = 0;
            worker_safe = 0;
        }

        if (worker_safe) {
            memset(&outcome, 0, sizeof(outcome));
            if (pstvnc_mpeg_worker_outcome(
                    &run->worker,
                    run->current_generation,
                    &outcome) != PSTVNC_MPEG_WORKER_OK ||
                outcome.run_generation != run->current_generation ||
                outcome.kind == PSTVNC_MPEG_WORKER_OUTCOME_NONE)
                clean = 0;

            if (pstvnc_mpeg_worker_release(
                    &run->worker,
                    run->current_generation) != PSTVNC_MPEG_WORKER_OK) {
                clean = 0;
                worker_safe = 0;
            } else {
                run->worker_started = 0;
            }
        }
    } else if (run->worker.initialized) {
        /*
         * A lower-layer start failure retained partial worker ownership that
         * cannot be proven joined through the ordinary R4 contract.
         */
        clean = 0;
        worker_safe = 0;
    }

    if (!worker_safe) {
        pstvnc_app_mpeg_run_fault(
            run,
            PSTVNC_APP_MPEG_RUN_CLEANUP_FAILED);
        return 0;
    }

    if (run->worker_runtime_owned) {
        if (pstvnc_mpeg_ps2_worker_runtime_release(
                &run->worker_runtime) != 0) {
            clean = 0;
        } else {
            run->worker_runtime_owned = 0;
        }
    }

    if (run->transport_run_open) {
        if (pstvnc_transport_mpeg_run_abort_pre_start(
                &run->transport_access) != PSTVNC_TRANSPORT_OK) {
            clean = 0;
        } else {
            run->transport_run_open = 0;
        }
    }

    if (!clean) {
        pstvnc_app_mpeg_run_fault(
            run,
            PSTVNC_APP_MPEG_RUN_CLEANUP_FAILED);
        return 0;
    }

    pstvnc_app_mpeg_run_clear_attempt(run);
    run->state = PSTVNC_APP_MPEG_RUN_IDLE;
    return 1;
}

static pstvnc_app_mpeg_run_result_t
pstvnc_app_mpeg_run_fail_pre_start(
    pstvnc_app_mpeg_run_t *run,
    pstvnc_app_mpeg_run_result_t result)
{
    run->last_result = result;

    if (!pstvnc_app_mpeg_run_unwind_pre_start(run))
        return PSTVNC_APP_MPEG_RUN_CLEANUP_FAILED;

    run->last_result = result;
    return result;
}

void pstvnc_app_mpeg_run_init(
    pstvnc_app_mpeg_run_t *run)
{
    if (run == NULL)
        return;

    memset(run, 0, sizeof(*run));
    run->state = PSTVNC_APP_MPEG_RUN_IDLE;
    run->last_result = PSTVNC_APP_MPEG_RUN_OK;
}

pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_start(
    pstvnc_app_mpeg_run_t *run,
    const pstvnc_mpeg_presentation_geometry_t *geometry,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *media_clock)
{
    const pstvnc_config_mpeg_runtime_profile_t *selected_profile;
    pstvnc_mpeg_decoder_memory_ops_t decoder_memory_ops;
    pstvnc_mpeg_decoder_sync_ops_t decoder_sync_ops;
    pstvnc_mpeg_worker_memory_ops_t worker_memory_ops;
    pstvnc_mpeg_worker_thread_ops_t worker_thread_ops;
    pstvnc_mpeg_worker_sync_ops_t worker_sync_ops;
    pstvnc_mpeg_worker_event_ops_t frame_event_ops;
    pstvnc_mpeg_decoder_platform_ops_t decoder_platform_ops;
    pstvnc_mpeg_presentation_geometry_t presentation_geometry;
    pstvnc_transport_mpeg_start_request_t start_request;
    uint32_t presentation_generation;
    pstvnc_app_mpeg_run_result_t failure;

    if (run == NULL ||
        geometry == NULL ||
        rfb_flow_policy == NULL ||
        transport_access == NULL ||
        presentation == NULL ||
        media_clock == NULL)
        return PSTVNC_APP_MPEG_RUN_INVALID;

    if (run->state == PSTVNC_APP_MPEG_RUN_FAULTED)
        return PSTVNC_APP_MPEG_RUN_FAULTED;

    if (run->state != PSTVNC_APP_MPEG_RUN_IDLE ||
        run->current_generation != 0u)
        return PSTVNC_APP_MPEG_RUN_NOT_IDLE;

    selected_profile = pstvnc_config_mpeg_runtime_profile_selected();
    if (selected_profile == NULL) {
        run->last_result = PSTVNC_APP_MPEG_RUN_PROFILE_UNAVAILABLE;
        return run->last_result;
    }

    if (!pstvnc_app_mpeg_run_geometry_valid(
            geometry,
            selected_profile)) {
        run->last_result = PSTVNC_APP_MPEG_RUN_GEOMETRY_INVALID;
        return run->last_result;
    }

    if (pstvnc_rfb_flow_policy_allows_remote_publication(
            rfb_flow_policy)) {
        run->last_result = PSTVNC_APP_MPEG_RUN_RFB_NOT_PROTECTED;
        return run->last_result;
    }

    if (pstvnc_mpeg_presentation_state(presentation) !=
        PSTVNC_MPEG_PRESENTATION_RFB_ONLY) {
        run->last_result = PSTVNC_APP_MPEG_RUN_PRESENTATION_NOT_IDLE;
        return run->last_result;
    }

    if (run->last_allocated_generation == UINT32_MAX) {
        run->state = PSTVNC_APP_MPEG_RUN_FAULTED;
        run->last_result = PSTVNC_APP_MPEG_RUN_GENERATION_EXHAUSTED;
        return run->last_result;
    }

    run->last_allocated_generation++;
    run->current_generation = run->last_allocated_generation;
    run->transport_access = *transport_access;
    run->profile = *selected_profile;
    run->presentation = presentation;
    run->rfb_flow_policy = rfb_flow_policy;
    run->media_clock = media_clock;

    if (pstvnc_transport_mpeg_run_open(
            &run->transport_access) != PSTVNC_TRANSPORT_OK) {
        failure = PSTVNC_APP_MPEG_RUN_TRANSPORT_OPEN_FAILED;
        return pstvnc_app_mpeg_run_fail_pre_start(run, failure);
    }
    run->transport_run_open = 1;

    if (pstvnc_mpeg_ps2_worker_runtime_init(
            &run->worker_runtime,
            &run->profile.ps2_worker_runtime) != 0) {
        run->worker_runtime_owned =
            run->worker_runtime.resources_owned != 0;
        failure = PSTVNC_APP_MPEG_RUN_RUNTIME_INIT_FAILED;
        return pstvnc_app_mpeg_run_fail_pre_start(run, failure);
    }
    run->worker_runtime_owned = 1;

    memset(&decoder_memory_ops, 0, sizeof(decoder_memory_ops));
    memset(&decoder_sync_ops, 0, sizeof(decoder_sync_ops));
    memset(&worker_memory_ops, 0, sizeof(worker_memory_ops));
    memset(&worker_thread_ops, 0, sizeof(worker_thread_ops));
    memset(&worker_sync_ops, 0, sizeof(worker_sync_ops));
    memset(&frame_event_ops, 0, sizeof(frame_event_ops));

    if (pstvnc_mpeg_ps2_worker_runtime_operations(
            &run->worker_runtime,
            &decoder_memory_ops,
            &decoder_sync_ops,
            &worker_memory_ops,
            &worker_thread_ops,
            &worker_sync_ops,
            &frame_event_ops) != 0) {
        failure = PSTVNC_APP_MPEG_RUN_RUNTIME_OPS_FAILED;
        return pstvnc_app_mpeg_run_fail_pre_start(run, failure);
    }

    pstvnc_ps2_mpeg_decoder_backend_init(&run->decoder_backend);
    memset(&decoder_platform_ops, 0, sizeof(decoder_platform_ops));
    if (pstvnc_ps2_mpeg_decoder_backend_platform_ops(
            &run->decoder_backend,
            &decoder_platform_ops) != 0) {
        failure = PSTVNC_APP_MPEG_RUN_BACKEND_OPS_FAILED;
        return pstvnc_app_mpeg_run_fail_pre_start(run, failure);
    }

    if (pstvnc_mpeg_worker_start(
            &run->worker,
            run->current_generation,
            &run->profile.worker,
            &run->profile.decoder,
            &decoder_memory_ops,
            &decoder_sync_ops,
            &decoder_platform_ops,
            &worker_memory_ops,
            &worker_thread_ops,
            &worker_sync_ops,
            &frame_event_ops) != PSTVNC_MPEG_WORKER_OK) {
        failure = PSTVNC_APP_MPEG_RUN_WORKER_START_FAILED;
        return pstvnc_app_mpeg_run_fail_pre_start(run, failure);
    }
    run->worker_started = 1;

    if (!pstvnc_mpeg_presentation_arm(
            run->presentation,
            geometry,
            run->current_generation)) {
        failure = PSTVNC_APP_MPEG_RUN_PRESENTATION_ARM_FAILED;
        return pstvnc_app_mpeg_run_fail_pre_start(run, failure);
    }
    run->presentation_armed = 1;

    memset(&presentation_geometry, 0, sizeof(presentation_geometry));
    presentation_generation = 0u;
    if (!pstvnc_mpeg_presentation_snapshot(
            run->presentation,
            &presentation_geometry,
            &presentation_generation) ||
        presentation_generation != run->current_generation ||
        !pstvnc_app_mpeg_run_geometry_equal(
            geometry,
            &presentation_geometry)) {
        failure = PSTVNC_APP_MPEG_RUN_PRESENTATION_SNAPSHOT_FAILED;
        return pstvnc_app_mpeg_run_fail_pre_start(run, failure);
    }

    if (pstvnc_app_mpeg_frame_consumer_init(
            &run->frame_consumer,
            run->current_generation,
            &run->worker,
            run->presentation,
            run->media_clock,
            &run->profile.scheduler) != PSTVNC_APP_MPEG_FRAME_OK) {
        failure = PSTVNC_APP_MPEG_RUN_FRAME_CONSUMER_INIT_FAILED;
        return pstvnc_app_mpeg_run_fail_pre_start(run, failure);
    }
    run->frame_consumer_initialized = 1;

    memset(&start_request, 0, sizeof(start_request));
    start_request.generation = run->current_generation;
    start_request.base_x = (uint32_t)presentation_geometry.base.x;
    start_request.base_y = (uint32_t)presentation_geometry.base.y;
    start_request.base_width = (uint32_t)presentation_geometry.base.width;
    start_request.base_height = (uint32_t)presentation_geometry.base.height;
    start_request.suppression_x =
        (uint32_t)presentation_geometry.suppression.x;
    start_request.suppression_y =
        (uint32_t)presentation_geometry.suppression.y;
    start_request.suppression_width =
        (uint32_t)presentation_geometry.suppression.width;
    start_request.suppression_height =
        (uint32_t)presentation_geometry.suppression.height;

    /*
     * Irreversible R21 boundary. From this point on, a non-OK result does not
     * prove that START bytes were absent, so pre-START abort is forbidden.
     */
    run->start_invoked = 1;
    if (pstvnc_transport_mpeg_send_start(
            &run->transport_access,
            &start_request) != PSTVNC_TRANSPORT_OK) {
        pstvnc_app_mpeg_run_fault(
            run,
            PSTVNC_APP_MPEG_RUN_START_FAILED);
        return run->last_result;
    }

    run->state = PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME;
    run->last_result = PSTVNC_APP_MPEG_RUN_OK;
    return PSTVNC_APP_MPEG_RUN_OK;
}

pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_status(
    const pstvnc_app_mpeg_run_t *run,
    pstvnc_app_mpeg_run_status_t *status)
{
    if (run == NULL || status == NULL)
        return PSTVNC_APP_MPEG_RUN_INVALID;

    status->state = run->state;
    status->last_result = run->last_result;
    status->last_allocated_generation = run->last_allocated_generation;
    status->current_generation = run->current_generation;
    status->session_teardown_required = run->session_teardown_required;
    return PSTVNC_APP_MPEG_RUN_OK;
}
