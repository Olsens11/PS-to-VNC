/*
 * File synopsis:
 * Deterministic host contracts for the Application-owned A004 P7 main-thread
 * MPEG frame consumer. MPEG worker and physical compositor calls are faked at
 * their public seams while real Presentation, scheduler, frame validation, and
 * session media-clock code exercise the accepted cross-domain contracts.
 */

#include "app_mpeg_frame.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_GENERATION 900u

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

typedef struct fake_worker {
    pstvnc_mpeg_worker_t *worker;
    pstvnc_mpeg_worker_frame_t frame;
    pstvnc_mpeg_worker_result_t stop_result;
    pstvnc_mpeg_worker_result_t release_result;
    uint32_t run_generation;
    int available;
    int claimed;
    int worker_finished;
    int claim_calls;
    int release_calls;
    int stop_calls;
    int status_calls;
} fake_worker_t;

typedef struct fake_compositor {
    pstvnc_mpeg_compositor_result_t result;
    uint64_t sync_tick;
    int failure_synchronized;
    int call_count;
    pstvnc_mpeg_compositor_frame_t last_frame;
} fake_compositor_t;

typedef struct fixture {
    pstvnc_app_mpeg_frame_consumer_t consumer;
    pstvnc_mpeg_worker_t worker;
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_scheduler_profile_t scheduler_profile;
    uint16_t pixels[512];
    int clock_locked;
} fixture_t;

static fake_worker_t g_worker;
static fake_compositor_t g_compositor;

static int clock_lock(void *context)
{
    int *locked = (int *)context;

    if (locked == NULL || *locked)
        return -1;

    *locked = 1;
    return 0;
}

static int clock_unlock(void *context)
{
    int *locked = (int *)context;

    if (locked == NULL || !*locked)
        return -1;

    *locked = 0;
    return 0;
}

static pstvnc_mpeg_presentation_geometry_t valid_geometry(void)
{
    pstvnc_mpeg_presentation_geometry_t geometry;

    memset(&geometry, 0, sizeof(geometry));

    geometry.base.x = 100;
    geometry.base.y = 120;
    geometry.base.width = 32;
    geometry.base.height = 16;

    geometry.inner_content = geometry.base;

    geometry.suppression.x = 96;
    geometry.suppression.y = 116;
    geometry.suppression.width = 40;
    geometry.suppression.height = 24;

    return geometry;
}

static void fake_worker_offer(
    fixture_t *fixture,
    uint32_t ordinal)
{
    memset(&g_worker.frame, 0, sizeof(g_worker.frame));

    g_worker.frame.picture.pixels = fixture->pixels;
    g_worker.frame.picture.byte_count = sizeof(fixture->pixels);
    g_worker.frame.picture.capacity_bytes = sizeof(fixture->pixels);
    g_worker.frame.picture.width = 32u;
    g_worker.frame.picture.height = 16u;
    g_worker.frame.picture.bytes_per_pixel = 2u;
    g_worker.frame.picture.picture_ordinal = ordinal;
    g_worker.frame.run_generation = TEST_GENERATION;
    g_worker.frame.claim_token = ordinal;
    g_worker.available = 1;
    g_worker.claimed = 0;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_claim(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    pstvnc_mpeg_worker_frame_t *frame)
{
    if (worker != g_worker.worker ||
        run_generation != g_worker.run_generation)
        return PSTVNC_MPEG_WORKER_WRONG_GENERATION;

    g_worker.claim_calls += 1;

    if (!g_worker.available || g_worker.claimed)
        return PSTVNC_MPEG_WORKER_FRAME_UNAVAILABLE;

    g_worker.available = 0;
    g_worker.claimed = 1;
    *frame = g_worker.frame;
    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_release_frame(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    uint32_t claim_token)
{
    if (worker != g_worker.worker ||
        run_generation != g_worker.run_generation)
        return PSTVNC_MPEG_WORKER_WRONG_GENERATION;

    g_worker.release_calls += 1;

    if (g_worker.release_result != PSTVNC_MPEG_WORKER_OK)
        return g_worker.release_result;

    if (!g_worker.claimed ||
        claim_token != g_worker.frame.claim_token)
        return PSTVNC_MPEG_WORKER_CLAIM_MISMATCH;

    g_worker.claimed = 0;
    return PSTVNC_MPEG_WORKER_OK;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_request_stop(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation)
{
    if (worker != g_worker.worker ||
        run_generation != g_worker.run_generation)
        return PSTVNC_MPEG_WORKER_WRONG_GENERATION;

    g_worker.stop_calls += 1;
    return g_worker.stop_result;
}

pstvnc_mpeg_worker_result_t pstvnc_mpeg_worker_status(
    pstvnc_mpeg_worker_t *worker,
    uint32_t run_generation,
    pstvnc_mpeg_worker_status_t *status)
{
    if (worker != g_worker.worker ||
        run_generation != g_worker.run_generation)
        return PSTVNC_MPEG_WORKER_WRONG_GENERATION;

    g_worker.status_calls += 1;
    memset(status, 0, sizeof(*status));
    status->worker_finished = g_worker.worker_finished;
    status->slot_state = g_worker.claimed
        ? PSTVNC_MPEG_WORKER_SLOT_CLAIMED
        : g_worker.available
            ? PSTVNC_MPEG_WORKER_SLOT_AVAILABLE
            : PSTVNC_MPEG_WORKER_SLOT_EMPTY;

    return PSTVNC_MPEG_WORKER_OK;
}

int pstvnc_mpeg_compositor_present(
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock,
    const pstvnc_mpeg_compositor_frame_t *frame,
    pstvnc_mpeg_compositor_effects_t *effects)
{
    int armed = 0;

    g_compositor.call_count += 1;
    g_compositor.last_frame = *frame;

    memset(effects, 0, sizeof(*effects));
    effects->observed_sync_tick = g_compositor.sync_tick;

    if (g_compositor.result != PSTVNC_MPEG_COMPOSITOR_OK) {
        effects->synchronized = g_compositor.failure_synchronized;
        return g_compositor.result;
    }

    effects->synchronized = 1u;

    if (pstvnc_mpeg_presentation_state(presentation) ==
            PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME) {
        CHECK(pstvnc_media_clock_is_armed(
            clock,
            &armed) == PSTVNC_MEDIA_CLOCK_OK);

        CHECK(pstvnc_media_clock_arm(
            clock,
            g_compositor.sync_tick) == PSTVNC_MEDIA_CLOCK_OK);

        effects->clock_armed_now = armed ? 0u : 1u;

        CHECK(pstvnc_mpeg_presentation_first_frame_presented(
            presentation,
            frame->run_generation));
        effects->first_frame_promoted = 1u;
    }

    return PSTVNC_MPEG_COMPOSITOR_OK;
}

static void fixture_init(fixture_t *fixture)
{
    pstvnc_config_media_clock_profile_t clock_profile;
    pstvnc_media_clock_sync_t clock_sync;
    pstvnc_mpeg_presentation_geometry_t geometry;

    memset(fixture, 0, sizeof(*fixture));
    memset(&g_worker, 0, sizeof(g_worker));
    memset(&g_compositor, 0, sizeof(g_compositor));

    g_worker.worker = &fixture->worker;
    g_worker.run_generation = TEST_GENERATION;
    g_worker.stop_result = PSTVNC_MPEG_WORKER_OK;
    g_worker.release_result = PSTVNC_MPEG_WORKER_OK;

    g_compositor.result = PSTVNC_MPEG_COMPOSITOR_OK;
    g_compositor.sync_tick = 1000u;

    memset(&clock_profile, 0, sizeof(clock_profile));
    memset(&clock_sync, 0, sizeof(clock_sync));
    clock_sync.lock = clock_lock;
    clock_sync.unlock = clock_unlock;
    clock_sync.context = &fixture->clock_locked;

    CHECK(pstvnc_media_clock_init(
        &fixture->clock,
        &clock_profile,
        1000u,
        &clock_sync) == PSTVNC_MEDIA_CLOCK_OK);

    geometry = valid_geometry();
    pstvnc_mpeg_presentation_init(&fixture->presentation);
    CHECK(pstvnc_mpeg_presentation_arm(
        &fixture->presentation,
        &geometry,
        TEST_GENERATION));

    fixture->scheduler_profile.fps_numerator = 10u;
    fixture->scheduler_profile.fps_denominator = 1u;
    fixture->scheduler_profile.drop_enabled = 1;
    fixture->scheduler_profile.drop_threshold_milliframes = 500u;

    CHECK(pstvnc_app_mpeg_frame_consumer_init(
        &fixture->consumer,
        TEST_GENERATION,
        &fixture->worker,
        &fixture->presentation,
        &fixture->clock,
        &fixture->scheduler_profile) == PSTVNC_APP_MPEG_FRAME_OK);
}

static void test_first_wait_present_drop(void)
{
    fixture_t fixture;
    pstvnc_app_mpeg_frame_service_result_t result;
    pstvnc_app_mpeg_frame_status_t status;
    int claim_calls_after_wait;
    int compositor_calls_after_present;

    fixture_init(&fixture);
    fake_worker_offer(&fixture, 1u);

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_PRESENTED);
    CHECK(result.picture_ordinal == 1u);
    CHECK(result.compositor_effects.synchronized);
    CHECK(result.compositor_effects.first_frame_promoted);
    CHECK(result.compositor_effects.observed_sync_tick == 1000u);
    CHECK(!result.claim_outstanding);
    CHECK(g_worker.release_calls == 1);
    CHECK(g_compositor.call_count == 1);
    CHECK(g_compositor.last_frame.surface.pixels == fixture.pixels);
    CHECK(pstvnc_mpeg_presentation_state(&fixture.presentation) ==
        PSTVNC_MPEG_PRESENTATION_MPEG_OWNED);

    CHECK(pstvnc_app_mpeg_frame_consumer_status(
        &fixture.consumer,
        TEST_GENERATION,
        &status) == PSTVNC_APP_MPEG_FRAME_OK);
    CHECK(status.last_consumed_ordinal == 1u);
    CHECK(status.presented_count == 1u);
    CHECK(status.scheduler_initialized);

    fake_worker_offer(&fixture, 2u);
    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        1050u,
        &result) == PSTVNC_APP_MPEG_FRAME_WAIT);
    CHECK(result.picture_ordinal == 2u);
    CHECK(result.deadline_tick == 1100u);
    CHECK(result.claim_outstanding);
    CHECK(g_worker.claimed);
    CHECK(g_worker.release_calls == 1);
    claim_calls_after_wait = g_worker.claim_calls;

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        1100u,
        &result) == PSTVNC_APP_MPEG_FRAME_PRESENTED);
    CHECK(g_worker.claim_calls == claim_calls_after_wait);
    CHECK(!result.claim_outstanding);
    CHECK(g_worker.release_calls == 2);
    CHECK(g_compositor.call_count == 2);
    compositor_calls_after_present = g_compositor.call_count;

    fake_worker_offer(&fixture, 3u);
    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        1260u,
        &result) == PSTVNC_APP_MPEG_FRAME_DROPPED);
    CHECK(result.scheduler_decision.decision ==
        PSTVNC_MPEG_SCHEDULER_DROP_PRESENTATION_ONLY);
    CHECK(g_compositor.call_count == compositor_calls_after_present);
    CHECK(g_worker.release_calls == 3);

    CHECK(pstvnc_app_mpeg_frame_consumer_status(
        &fixture.consumer,
        TEST_GENERATION,
        &status) == PSTVNC_APP_MPEG_FRAME_OK);
    CHECK(status.last_consumed_ordinal == 3u);
    CHECK(status.presented_count == 2u);
    CHECK(status.dropped_count == 1u);
    CHECK(!status.claim_outstanding);
}

static void test_retiring_drain_and_sealed_fence(void)
{
    fixture_t fixture;
    pstvnc_app_mpeg_frame_service_result_t result;
    int claims_before_seal;

    fixture_init(&fixture);
    fake_worker_offer(&fixture, 1u);
    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_PRESENTED);

    CHECK(pstvnc_mpeg_presentation_begin_retirement(
        &fixture.presentation,
        TEST_GENERATION));

    fake_worker_offer(&fixture, 2u);
    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        1100u,
        &result) == PSTVNC_APP_MPEG_FRAME_PRESENTED);
    CHECK(g_compositor.call_count == 2);

    CHECK(pstvnc_mpeg_presentation_seal_retirement(
        &fixture.presentation,
        TEST_GENERATION));

    fake_worker_offer(&fixture, 3u);
    claims_before_seal = g_worker.claim_calls;

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        1200u,
        &result) == PSTVNC_APP_MPEG_FRAME_PRESENTATION_STATE);
    CHECK(g_worker.claim_calls == claims_before_seal);
    CHECK(!result.claim_outstanding);
}

static void test_invalid_mapping_and_ordinal_containment(void)
{
    fixture_t fixture;
    pstvnc_app_mpeg_frame_service_result_t result;
    pstvnc_app_mpeg_frame_status_t status;

    fixture_init(&fixture);
    fake_worker_offer(&fixture, 1u);
    g_worker.frame.picture.bytes_per_pixel = 4u;

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_FRAME_INVALID);
    CHECK(result.stop_attempted);
    CHECK(result.release_attempted);
    CHECK(g_worker.stop_calls == 1);
    CHECK(g_worker.release_calls == 1);
    CHECK(!result.claim_outstanding);

    CHECK(pstvnc_app_mpeg_frame_consumer_status(
        &fixture.consumer,
        TEST_GENERATION,
        &status) == PSTVNC_APP_MPEG_FRAME_OK);
    CHECK(status.faulted);
    CHECK(!status.claim_outstanding);

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_FAULTED);

    fixture_init(&fixture);
    fake_worker_offer(&fixture, 2u);

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_ORDINAL_INVALID);
    CHECK(g_worker.stop_calls == 1);
    CHECK(g_worker.release_calls == 1);
    CHECK(g_compositor.call_count == 0);
}

static void test_stop_failure_retains_borrow(void)
{
    fixture_t fixture;
    pstvnc_app_mpeg_frame_service_result_t result;
    pstvnc_app_mpeg_frame_status_t status;

    fixture_init(&fixture);
    fake_worker_offer(&fixture, 1u);
    g_worker.frame.picture.bytes_per_pixel = 4u;
    g_worker.stop_result = PSTVNC_MPEG_WORKER_SYNC_FAILED;

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_FRAME_INVALID);
    CHECK(result.stop_attempted);
    CHECK(!result.release_attempted);
    CHECK(result.worker_stop_result == PSTVNC_MPEG_WORKER_SYNC_FAILED);
    CHECK(result.claim_outstanding);
    CHECK(g_worker.claimed);
    CHECK(g_worker.release_calls == 0);

    CHECK(pstvnc_app_mpeg_frame_consumer_status(
        &fixture.consumer,
        TEST_GENERATION,
        &status) == PSTVNC_APP_MPEG_FRAME_OK);
    CHECK(status.faulted);
    CHECK(status.claim_outstanding);
    CHECK(status.held_ordinal == 1u);
}

static void test_failure_preserves_physical_effects(void)
{
    fixture_t fixture;
    pstvnc_app_mpeg_frame_service_result_t result;

    fixture_init(&fixture);
    fake_worker_offer(&fixture, 1u);

    g_compositor.result = PSTVNC_MPEG_COMPOSITOR_CLOCK_FAILED;
    g_compositor.failure_synchronized = 1;
    g_compositor.sync_tick = 1777u;

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_COMPOSITOR_FAILED);
    CHECK(result.compositor_result == PSTVNC_MPEG_COMPOSITOR_CLOCK_FAILED);
    CHECK(result.compositor_effects.synchronized);
    CHECK(result.compositor_effects.observed_sync_tick == 1777u);
    CHECK(result.stop_attempted);
    CHECK(result.release_attempted);
    CHECK(!result.claim_outstanding);
}

static void test_scheduler_init_failure_after_first_sync(void)
{
    fixture_t fixture;
    pstvnc_app_mpeg_frame_service_result_t result;

    fixture_init(&fixture);
    fixture.consumer.scheduler_profile.fps_numerator = 0u;
    fake_worker_offer(&fixture, 1u);

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_SCHEDULER_FAILED);
    CHECK(result.compositor_effects.synchronized);
    CHECK(result.compositor_effects.first_frame_promoted);
    CHECK(pstvnc_mpeg_presentation_state(&fixture.presentation) ==
        PSTVNC_MPEG_PRESENTATION_MPEG_OWNED);
    CHECK(result.stop_attempted);
    CHECK(result.release_attempted);
    CHECK(!result.claim_outstanding);
}

static void test_idle_reports_worker_finished_without_outcome(void)
{
    fixture_t fixture;
    pstvnc_app_mpeg_frame_service_result_t result;

    fixture_init(&fixture);
    g_worker.worker_finished = 1;

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_IDLE);
    CHECK(result.worker_finished);
    CHECK(g_worker.claim_calls == 1);
    CHECK(g_worker.status_calls >= 2);
    CHECK(g_compositor.call_count == 0);
}

static void test_wrong_generation_fails_closed(void)
{
    fixture_t fixture;
    pstvnc_app_mpeg_frame_service_result_t result;
    int claim_calls;

    fixture_init(&fixture);
    fake_worker_offer(&fixture, 1u);
    claim_calls = g_worker.claim_calls;

    CHECK(pstvnc_app_mpeg_frame_consumer_service(
        &fixture.consumer,
        TEST_GENERATION + 1u,
        0u,
        &result) == PSTVNC_APP_MPEG_FRAME_WRONG_GENERATION);
    CHECK(g_worker.claim_calls == claim_calls);
    CHECK(!g_worker.claimed);
}

int main(void)
{
    test_first_wait_present_drop();
    test_retiring_drain_and_sealed_fence();
    test_invalid_mapping_and_ordinal_containment();
    test_stop_failure_retains_borrow();
    test_failure_preserves_physical_effects();
    test_scheduler_init_failure_after_first_sync();
    test_idle_reports_worker_finished_without_outcome();
    test_wrong_generation_fails_closed();

    if (failures != 0) {
        fprintf(stderr, "app_mpeg_frame_test: FAIL (%d)\n", failures);
        return 1;
    }

    puts("APP_MPEG_FRAME_TEST=PASS");
    return 0;
}
