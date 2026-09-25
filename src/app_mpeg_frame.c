/*
 * File synopsis:
 * Implements the Application/main-thread bridge from one exact A003 worker
 * borrow to A004 first-frame composition and post-first-frame scheduling.
 *
 * The consumer never blocks for time. WAIT keeps the exact claim pinned and
 * returns the absolute deadline to the caller's main loop. PRESENT and DROP
 * release only after their Application-owned accounting path is complete.
 * Mapping/scheduler/compositor failures request exact worker stop before
 * releasing the failed borrow; if stop cannot be established, the claim remains
 * outstanding and visible in status/result state.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A004-MAIN-THREAD-FRAME-CONSUMER-P7.
 */

#include "app_mpeg_frame.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "display/mpeg_frame.h"

static int pstvnc_app_mpeg_frame_generation_valid(
    const pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation)
{
    return consumer != NULL &&
        consumer->initialized &&
        consumer->run_generation != 0u &&
        run_generation != 0u &&
        consumer->run_generation == run_generation;
}

static void pstvnc_app_mpeg_frame_clear_held(
    pstvnc_app_mpeg_frame_consumer_t *consumer)
{
    memset(&consumer->held_frame, 0, sizeof(consumer->held_frame));
    consumer->claim_outstanding = 0;
}

static int pstvnc_app_mpeg_frame_expected_ordinal(
    const pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t *expected_ordinal)
{
    if (consumer == NULL ||
        expected_ordinal == NULL ||
        consumer->last_consumed_ordinal == UINT32_MAX)
        return 0;

    *expected_ordinal = consumer->last_consumed_ordinal + 1u;
    return *expected_ordinal != 0u;
}

static int pstvnc_app_mpeg_frame_map(
    const pstvnc_app_mpeg_frame_consumer_t *consumer,
    pstvnc_mpeg_compositor_frame_t *frame)
{
    const pstvnc_mpeg_decoded_picture_t *picture;
    size_t pixel_capacity;

    if (consumer == NULL ||
        frame == NULL ||
        !consumer->claim_outstanding)
        return 0;

    picture = &consumer->held_frame.picture;

    if (consumer->held_frame.run_generation != consumer->run_generation ||
        consumer->held_frame.claim_token == 0u ||
        picture->picture_ordinal == 0u ||
        consumer->held_frame.claim_token != picture->picture_ordinal ||
        picture->pixels == NULL ||
        ((uintptr_t)picture->pixels % sizeof(uint16_t)) != 0u ||
        picture->bytes_per_pixel != sizeof(uint16_t) ||
        picture->byte_count == 0u ||
        (picture->byte_count % sizeof(uint16_t)) != 0u ||
        picture->byte_count > picture->capacity_bytes ||
        picture->width > UINT_MAX ||
        picture->height > UINT_MAX)
        return 0;

    pixel_capacity = picture->byte_count / sizeof(uint16_t);

    memset(frame, 0, sizeof(*frame));
    frame->surface.pixels = (const uint16_t *)picture->pixels;
    frame->surface.pixel_capacity = pixel_capacity;
    frame->surface.width = (unsigned int)picture->width;
    frame->surface.height = (unsigned int)picture->height;
    frame->run_generation = consumer->run_generation;

    return pstvnc_mpeg_rgb16_macroblock_surface_valid(&frame->surface);
}

static void pstvnc_app_mpeg_frame_result_init(
    pstvnc_app_mpeg_frame_service_result_t *result)
{
    memset(result, 0, sizeof(*result));
    result->result = PSTVNC_APP_MPEG_FRAME_INVALID;
    result->worker_claim_result = PSTVNC_MPEG_WORKER_OK;
    result->worker_status_result = PSTVNC_MPEG_WORKER_OK;
    result->worker_stop_result = PSTVNC_MPEG_WORKER_OK;
    result->worker_release_result = PSTVNC_MPEG_WORKER_OK;
    result->scheduler_result = PSTVNC_MPEG_SCHEDULER_OK;
    result->scheduler_decision.decision =
        PSTVNC_MPEG_SCHEDULER_DECISION_INVALID;
    result->compositor_result = PSTVNC_MPEG_COMPOSITOR_OK;
}

static void pstvnc_app_mpeg_frame_result_refresh_claim(
    const pstvnc_app_mpeg_frame_consumer_t *consumer,
    pstvnc_app_mpeg_frame_service_result_t *result)
{
    result->claim_outstanding = consumer->claim_outstanding;
    if (consumer->claim_outstanding)
        result->picture_ordinal =
            consumer->held_frame.picture.picture_ordinal;
}

static void pstvnc_app_mpeg_frame_contain_failure(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    pstvnc_app_mpeg_frame_service_result_t *result)
{
    pstvnc_mpeg_worker_result_t worker_result;

    consumer->faulted = 1;

    if (!consumer->claim_outstanding) {
        pstvnc_app_mpeg_frame_result_refresh_claim(consumer, result);
        return;
    }

    result->stop_attempted = 1;
    worker_result = pstvnc_mpeg_worker_request_stop(
        consumer->worker,
        consumer->run_generation);
    result->worker_stop_result = worker_result;

    if (worker_result != PSTVNC_MPEG_WORKER_OK) {
        pstvnc_app_mpeg_frame_result_refresh_claim(consumer, result);
        return;
    }

    result->release_attempted = 1;
    worker_result = pstvnc_mpeg_worker_release_frame(
        consumer->worker,
        consumer->run_generation,
        consumer->held_frame.claim_token);
    result->worker_release_result = worker_result;

    if (worker_result == PSTVNC_MPEG_WORKER_OK)
        pstvnc_app_mpeg_frame_clear_held(consumer);

    pstvnc_app_mpeg_frame_result_refresh_claim(consumer, result);
}

static int pstvnc_app_mpeg_frame_release_success(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    pstvnc_app_mpeg_frame_service_result_t *result)
{
    pstvnc_mpeg_worker_result_t worker_result;

    result->release_attempted = 1;
    worker_result = pstvnc_mpeg_worker_release_frame(
        consumer->worker,
        consumer->run_generation,
        consumer->held_frame.claim_token);
    result->worker_release_result = worker_result;

    if (worker_result == PSTVNC_MPEG_WORKER_OK) {
        pstvnc_app_mpeg_frame_clear_held(consumer);
        result->claim_outstanding = 0;
        return 1;
    }

    /*
     * Exact release failed. Preserve the held metadata, fault the coordinator,
     * and stop the worker so no later decode may intentionally advance. The
     * higher-level owner still sees claim_outstanding because release was not
     * proven successful.
     */
    consumer->faulted = 1;
    result->stop_attempted = 1;
    result->worker_stop_result = pstvnc_mpeg_worker_request_stop(
        consumer->worker,
        consumer->run_generation);
    pstvnc_app_mpeg_frame_result_refresh_claim(consumer, result);
    return 0;
}

static int pstvnc_app_mpeg_frame_presentation_allows_new_claim(
    const pstvnc_app_mpeg_frame_consumer_t *consumer,
    pstvnc_mpeg_presentation_state_t state)
{
    if (state == PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME)
        return consumer->last_consumed_ordinal == 0u;

    if (state == PSTVNC_MPEG_PRESENTATION_MPEG_OWNED ||
        state == PSTVNC_MPEG_PRESENTATION_RETIRING)
        return consumer->last_consumed_ordinal != 0u;

    return 0;
}

pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_init(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation,
    pstvnc_mpeg_worker_t *worker,
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock,
    const pstvnc_mpeg_scheduler_profile_t *scheduler_profile)
{
    pstvnc_mpeg_presentation_geometry_t geometry;
    pstvnc_mpeg_worker_status_t worker_status;
    uint32_t presentation_generation;
    pstvnc_mpeg_worker_result_t worker_result;

    if (consumer == NULL ||
        worker == NULL ||
        presentation == NULL ||
        clock == NULL ||
        scheduler_profile == NULL ||
        run_generation == 0u)
        return PSTVNC_APP_MPEG_FRAME_INVALID;

    if (pstvnc_mpeg_presentation_state(presentation) !=
            PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||
        !pstvnc_mpeg_presentation_snapshot(
            presentation,
            &geometry,
            &presentation_generation) ||
        presentation_generation != run_generation)
        return PSTVNC_APP_MPEG_FRAME_PRESENTATION_STATE;

    (void)geometry;

    worker_result = pstvnc_mpeg_worker_status(
        worker,
        run_generation,
        &worker_status);

    if (worker_result == PSTVNC_MPEG_WORKER_WRONG_GENERATION)
        return PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION;

    if (worker_result != PSTVNC_MPEG_WORKER_OK)
        return PSTVNC_APP_MPEG_FRAME_WORKER_FAILED;

    memset(consumer, 0, sizeof(*consumer));
    consumer->worker = worker;
    consumer->presentation = presentation;
    consumer->clock = clock;
    consumer->scheduler_profile = *scheduler_profile;
    consumer->run_generation = run_generation;
    consumer->initialized = 1;

    return PSTVNC_APP_MPEG_FRAME_OK;
}

pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_service(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation,
    uint64_t current_tick,
    pstvnc_app_mpeg_frame_service_result_t *result)
{
    pstvnc_mpeg_presentation_state_t presentation_state;
    pstvnc_mpeg_compositor_frame_t compositor_frame;
    pstvnc_mpeg_worker_status_t worker_status;
    pstvnc_mpeg_worker_result_t worker_result;
    uint32_t expected_ordinal;
    uint32_t ordinal;
    int scheduler_result;
    int compositor_result;

    if (result == NULL)
        return PSTVNC_APP_MPEG_FRAME_INVALID;

    pstvnc_app_mpeg_frame_result_init(result);

    if (consumer == NULL || !consumer->initialized) {
        result->result = PSTVNC_APP_MPEG_FRAME_INVALID;
        return result->result;
    }

    if (run_generation == 0u ||
        run_generation != consumer->run_generation) {
        result->result = PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION;
        return result->result;
    }

    if (consumer->faulted) {
        result->result = PSTVNC_APP_MPEG_FRAME_FAULTED;
        pstvnc_app_mpeg_frame_result_refresh_claim(consumer, result);
        return result->result;
    }

    presentation_state =
        pstvnc_mpeg_presentation_state(consumer->presentation);

    if (!consumer->claim_outstanding) {
        if (!pstvnc_app_mpeg_frame_presentation_allows_new_claim(
                consumer,
                presentation_state)) {
            result->result =
                PSTVNC_APP_MPEG_FRAME_PRESENTATION_STATE;
            return result->result;
        }

        worker_result = pstvnc_mpeg_worker_claim(
            consumer->worker,
            consumer->run_generation,
            &consumer->held_frame);
        result->worker_claim_result = worker_result;

        if (worker_result == PSTVNC_MPEG_WORKER_FRAME_UNAVAILABLE) {
            worker_result = pstvnc_mpeg_worker_status(
                consumer->worker,
                consumer->run_generation,
                &worker_status);
            result->worker_status_result = worker_result;

            if (worker_result == PSTVNC_MPEG_WORKER_OK) {
                result->worker_finished = worker_status.worker_finished;
                result->result = PSTVNC_APP_MPEG_FRAME_IDLE;
                return result->result;
            }

            result->result = worker_result ==
                    PSTVNC_MPEG_WORKER_WRONG_GENERATION
                ? PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION
                : PSTVNC_APP_MPEG_FRAME_WORKER_FAILED;
            return result->result;
        }

        if (worker_result != PSTVNC_MPEG_WORKER_OK) {
            result->result = worker_result ==
                    PSTVNC_MPEG_WORKER_WRONG_GENERATION
                ? PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION
                : PSTVNC_APP_MPEG_FRAME_WORKER_FAILED;
            return result->result;
        }

        consumer->claim_outstanding = 1;
    }

    /*
     * A higher-level owner must prove no P7 claim before P5 seal/reveal. If
     * Presentation nevertheless becomes sealed while a claim is held, fail
     * closed and contain the borrow rather than presenting through that state.
     */
    presentation_state =
        pstvnc_mpeg_presentation_state(consumer->presentation);

    if (presentation_state == PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING ||
        presentation_state == PSTVNC_MPEG_PRESENTATION_RFB_ONLY) {
        result->result = PSTVNC_APP_MPEG_FRAME_PRESENTATION_STATE;
        pstvnc_app_mpeg_frame_contain_failure(consumer, result);
        return result->result;
    }

    if (!pstvnc_app_mpeg_frame_map(consumer, &compositor_frame)) {
        result->result = PSTVNC_APP_MPEG_FRAME_FRAME_INVALID;
        pstvnc_app_mpeg_frame_contain_failure(consumer, result);
        return result->result;
    }

    ordinal = consumer->held_frame.picture.picture_ordinal;
    result->picture_ordinal = ordinal;

    if (!pstvnc_app_mpeg_frame_expected_ordinal(
            consumer,
            &expected_ordinal) ||
        ordinal != expected_ordinal) {
        result->result = PSTVNC_APP_MPEG_FRAME_ORDINAL_INVALID;
        pstvnc_app_mpeg_frame_contain_failure(consumer, result);
        return result->result;
    }

    if (ordinal == 1u) {
        if (presentation_state !=
                PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||
            consumer->scheduler_initialized) {
            result->result =
                PSTVNC_APP_MPEG_FRAME_PRESENTATION_STATE;
            pstvnc_app_mpeg_frame_contain_failure(consumer, result);
            return result->result;
        }

        compositor_result = pstvnc_mpeg_compositor_present(
            consumer->presentation,
            consumer->clock,
            &compositor_frame,
            &result->compositor_effects);
        result->compositor_result =
            (pstvnc_mpeg_compositor_result_t)compositor_result;

        if (compositor_result != PSTVNC_MPEG_COMPOSITOR_OK) {
            result->result = PSTVNC_APP_MPEG_FRAME_COMPOSITOR_FAILED;
            pstvnc_app_mpeg_frame_contain_failure(consumer, result);
            return result->result;
        }

        if (!result->compositor_effects.synchronized ||
            !result->compositor_effects.first_frame_promoted) {
            result->result =
                PSTVNC_APP_MPEG_FRAME_COMPOSITOR_EFFECTS_INVALID;
            pstvnc_app_mpeg_frame_contain_failure(consumer, result);
            return result->result;
        }

        scheduler_result =
            pstvnc_mpeg_scheduler_init_after_first_sync(
                &consumer->scheduler,
                consumer->clock,
                &consumer->scheduler_profile,
                consumer->run_generation,
                result->compositor_effects.observed_sync_tick);
        result->scheduler_result =
            (pstvnc_mpeg_scheduler_result_t)scheduler_result;

        if (scheduler_result != PSTVNC_MPEG_SCHEDULER_OK) {
            result->result = PSTVNC_APP_MPEG_FRAME_SCHEDULER_FAILED;
            pstvnc_app_mpeg_frame_contain_failure(consumer, result);
            return result->result;
        }

        consumer->scheduler_initialized = 1;

        if (!pstvnc_app_mpeg_frame_release_success(
                consumer,
                result)) {
            result->result = PSTVNC_APP_MPEG_FRAME_RELEASE_FAILED;
            return result->result;
        }

        consumer->last_consumed_ordinal = 1u;
        consumer->presented_count += 1u;
        result->result = PSTVNC_APP_MPEG_FRAME_PRESENTED;
        return result->result;
    }

    if (!consumer->scheduler_initialized ||
        (presentation_state != PSTVNC_MPEG_PRESENTATION_MPEG_OWNED &&
         presentation_state != PSTVNC_MPEG_PRESENTATION_RETIRING)) {
        result->result = PSTVNC_APP_MPEG_FRAME_PRESENTATION_STATE;
        pstvnc_app_mpeg_frame_contain_failure(consumer, result);
        return result->result;
    }

    scheduler_result = pstvnc_mpeg_scheduler_decide(
        &consumer->scheduler,
        consumer->run_generation,
        (uint64_t)ordinal,
        current_tick,
        &result->scheduler_decision);
    result->scheduler_result =
        (pstvnc_mpeg_scheduler_result_t)scheduler_result;

    if (scheduler_result != PSTVNC_MPEG_SCHEDULER_OK) {
        result->result = PSTVNC_APP_MPEG_FRAME_SCHEDULER_FAILED;
        pstvnc_app_mpeg_frame_contain_failure(consumer, result);
        return result->result;
    }

    if (result->scheduler_decision.decision ==
            PSTVNC_MPEG_SCHEDULER_WAIT_UNTIL_DEADLINE) {
        result->deadline_tick =
            result->scheduler_decision.deadline_tick;
        result->claim_outstanding = 1;
        result->result = PSTVNC_APP_MPEG_FRAME_WAIT;
        return result->result;
    }

    if (result->scheduler_decision.decision ==
            PSTVNC_MPEG_SCHEDULER_DROP_PRESENTATION_ONLY) {
        if (!pstvnc_app_mpeg_frame_release_success(
                consumer,
                result)) {
            result->result = PSTVNC_APP_MPEG_FRAME_RELEASE_FAILED;
            return result->result;
        }

        consumer->last_consumed_ordinal = ordinal;
        consumer->dropped_count += 1u;
        result->result = PSTVNC_APP_MPEG_FRAME_DROPPED;
        return result->result;
    }

    if (result->scheduler_decision.decision !=
            PSTVNC_MPEG_SCHEDULER_PRESENT_NOW) {
        result->result = PSTVNC_APP_MPEG_FRAME_SCHEDULER_FAILED;
        pstvnc_app_mpeg_frame_contain_failure(consumer, result);
        return result->result;
    }

    compositor_result = pstvnc_mpeg_compositor_present(
        consumer->presentation,
        consumer->clock,
        &compositor_frame,
        &result->compositor_effects);
    result->compositor_result =
        (pstvnc_mpeg_compositor_result_t)compositor_result;

    if (compositor_result != PSTVNC_MPEG_COMPOSITOR_OK) {
        result->result = PSTVNC_APP_MPEG_FRAME_COMPOSITOR_FAILED;
        pstvnc_app_mpeg_frame_contain_failure(consumer, result);
        return result->result;
    }

    if (!result->compositor_effects.synchronized) {
        result->result =
            PSTVNC_APP_MPEG_FRAME_COMPOSITOR_EFFECTS_INVALID;
        pstvnc_app_mpeg_frame_contain_failure(consumer, result);
        return result->result;
    }

    if (!pstvnc_app_mpeg_frame_release_success(
            consumer,
            result)) {
        result->result = PSTVNC_APP_MPEG_FRAME_RELEASE_FAILED;
        return result->result;
    }

    consumer->last_consumed_ordinal = ordinal;
    consumer->presented_count += 1u;
    result->result = PSTVNC_APP_MPEG_FRAME_PRESENTED;
    return result->result;
}

pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_abandon_claim(
    pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation)
{
    pstvnc_mpeg_worker_result_t worker_result;

    if (!pstvnc_app_mpeg_frame_generation_valid(
            consumer,
            run_generation))
        return consumer != NULL && consumer->initialized
            ? PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION
            : PSTVNC_APP_MPEG_FRAME_INVALID;

    /*
     * Crossing this seam is terminal for this P7 consumer. Even when no claim
     * exists, prevent any later service from acquiring/presenting a new frame
     * while the enclosing Wire Session is being destroyed.
     */
    consumer->faulted = 1;

    if (!consumer->claim_outstanding)
        return PSTVNC_APP_MPEG_FRAME_OK;

    worker_result = pstvnc_mpeg_worker_release_frame(
        consumer->worker,
        consumer->run_generation,
        consumer->held_frame.claim_token);
    if (worker_result != PSTVNC_MPEG_WORKER_OK)
        return PSTVNC_APP_MPEG_FRAME_RELEASE_FAILED;

    pstvnc_app_mpeg_frame_clear_held(consumer);
    return PSTVNC_APP_MPEG_FRAME_OK;
}

pstvnc_app_mpeg_frame_result_t pstvnc_app_mpeg_frame_consumer_status(
    const pstvnc_app_mpeg_frame_consumer_t *consumer,
    uint32_t run_generation,
    pstvnc_app_mpeg_frame_status_t *status)
{
    if (status == NULL)
        return PSTVNC_APP_MPEG_FRAME_INVALID;

    memset(status, 0, sizeof(*status));

    if (!pstvnc_app_mpeg_frame_generation_valid(
            consumer,
            run_generation))
        return consumer != NULL && consumer->initialized
            ? PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION
            : PSTVNC_APP_MPEG_FRAME_INVALID;

    status->run_generation = consumer->run_generation;
    status->last_consumed_ordinal =
        consumer->last_consumed_ordinal;
    status->presented_count = consumer->presented_count;
    status->dropped_count = consumer->dropped_count;
    status->scheduler_initialized =
        consumer->scheduler_initialized;
    status->claim_outstanding = consumer->claim_outstanding;
    status->faulted = consumer->faulted;

    if (consumer->claim_outstanding)
        status->held_ordinal =
            consumer->held_frame.picture.picture_ordinal;

    return PSTVNC_APP_MPEG_FRAME_OK;
}
