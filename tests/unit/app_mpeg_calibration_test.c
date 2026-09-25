/*
 * File synopsis:
 * Focused host composition proof for A004 P9 Application-owned manual MPEG
 * CALIBRATION foreground sequencing.
 *
 * The test stubs only already-owned P8/input/RFB/Platform seams so event order
 * and failure containment are directly observable. P2 uses its real flow-policy
 * implementation. Existing canonical P8/P1, input-runtime, P3/compositor and
 * ordinary Application tests independently retain their mechanism coverage.
 */

#include "app_mpeg_calibration.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform/ps2_graphics.h"

#define TEST_WIDTH 64
#define TEST_HEIGHT 64
#define TEST_PIXELS ((size_t)TEST_WIDTH * (size_t)TEST_HEIGHT)

typedef enum test_event {
    EVENT_FREEZE_OBSERVED = 1,
    EVENT_SUSPEND,
    EVENT_POINTER_RELEASE,
    EVENT_REBASE,
    EVENT_SNAPSHOT_OBSERVED,
    EVENT_P8_BEGIN,
    EVENT_RASTER,
    EVENT_PRESENT_CALIBRATION,
    EVENT_PRESENT_RESTORE,
    EVENT_RESUME
} test_event_t;

typedef enum manual_step {
    MANUAL_STEP_EDIT = 0,
    MANUAL_STEP_REVIEW,
    MANUAL_STEP_ACCEPT,
    MANUAL_STEP_CANCEL,
    MANUAL_STEP_QUARANTINE_HOLD,
    MANUAL_STEP_RELEASE,
    MANUAL_STEP_NO_CONSUME
} manual_step_t;

typedef struct test_environment {
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_input_runtime_t input;
    pstvnc_rfb_session_t rfb;
    pstvnc_local_ui_t local_ui;
    pstvnc_mpeg_presentation_t presentation;

    uint16_t last_desktop[TEST_PIXELS];
    uint16_t last_desktop_before[TEST_PIXELS];
    uint16_t frozen[TEST_PIXELS];
    uint16_t work[TEST_PIXELS];

    unsigned char published_clicks;
} test_environment_t;

static test_event_t events[128];
static size_t event_count;

static pstvnc_rfb_flow_policy_t *witness_flow;
static const uint16_t *witness_snapshot_source;
static const uint16_t *witness_frozen;
static uint16_t *witness_work;
static size_t witness_pixels;

static int fail_suspend;
static int fail_pointer_release;
static int fail_rebase;
static int fail_resume;
static int fail_calibration_present;
static int fail_restore_present;
static int restore_presented;

static unsigned int pointer_release_calls;
static unsigned int calibration_present_calls;
static unsigned int restore_present_calls;
static unsigned int resolve_geometry_calls;

static manual_step_t next_manual_step;
static pstvnc_mpeg_calibration_region_t accepted_region;
static pstvnc_mpeg_calibration_geometry_t resolved_geometry;

static void record_event(test_event_t event)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count++] = event;
}

static void reset_fakes(void)
{
    memset(events, 0, sizeof(events));
    event_count = 0u;
    witness_flow = NULL;
    witness_snapshot_source = NULL;
    witness_frozen = NULL;
    witness_work = NULL;
    witness_pixels = 0u;
    fail_suspend = 0;
    fail_pointer_release = 0;
    fail_rebase = 0;
    fail_resume = 0;
    fail_calibration_present = 0;
    fail_restore_present = 0;
    restore_presented = 0;
    pointer_release_calls = 0u;
    calibration_present_calls = 0u;
    restore_present_calls = 0u;
    resolve_geometry_calls = 0u;
    next_manual_step = MANUAL_STEP_EDIT;
    memset(&accepted_region, 0, sizeof(accepted_region));
    memset(&resolved_geometry, 0, sizeof(resolved_geometry));
}

static void fill_desktop(test_environment_t *environment)
{
    size_t index;

    for (index = 0u; index < TEST_PIXELS; index++)
        environment->last_desktop[index] =
            (uint16_t)(0x8000u | (uint16_t)(index & 0x7fffu));

    memcpy(
        environment->last_desktop_before,
        environment->last_desktop,
        sizeof(environment->last_desktop));
}

static void environment_init(test_environment_t *environment)
{
    reset_fakes();
    memset(environment, 0, sizeof(*environment));
    fill_desktop(environment);

    pstvnc_rfb_flow_policy_init(&environment->flow);

    environment->local_ui.foreground =
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP;
    environment->local_ui.input_quarantined = 0;

    environment->presentation.state =
        PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
    environment->presentation.snapshot_valid = 0u;
    environment->presentation.run_generation = 0u;

    environment->input.initialized = 1;
    environment->input.controller_thread_started = 1;

    assert(pstvnc_app_mpeg_calibration_init(
        &environment->calibration,
        TEST_WIDTH,
        TEST_HEIGHT,
        environment->frozen,
        environment->work,
        TEST_PIXELS));

    witness_flow = &environment->flow;
    witness_snapshot_source = environment->last_desktop;
    witness_frozen = environment->frozen;
    witness_work = environment->work;
    witness_pixels = TEST_PIXELS;
}

static pstvnc_app_mpeg_calibration_result_t begin_environment(
    test_environment_t *environment)
{
    return pstvnc_app_mpeg_calibration_begin(
        &environment->calibration,
        &environment->flow,
        &environment->input,
        &environment->rfb,
        &environment->local_ui,
        &environment->presentation,
        21u,
        17u,
        &environment->published_clicks,
        environment->last_desktop,
        TEST_PIXELS);
}

static pstvnc_controller_state_t controller_sample(void)
{
    pstvnc_controller_state_t sample;

    memset(&sample, 0, sizeof(sample));
    return sample;
}

static pstvnc_app_mpeg_calibration_service_result_t
service_ok(test_environment_t *environment)
{
    pstvnc_app_mpeg_calibration_service_result_t result;
    pstvnc_controller_state_t sample = controller_sample();

    assert(pstvnc_app_mpeg_calibration_service_controller(
        &environment->calibration,
        &sample,
        &result) == PSTVNC_APP_MPEG_CALIBRATION_OK);

    return result;
}

static size_t find_event(test_event_t event)
{
    size_t index;

    for (index = 0u; index < event_count; index++) {
        if (events[index] == event)
            return index;
    }

    return event_count;
}

/* ----- P8/P1 seams: composition witness only ----- */

int pstvnc_mpeg_calibration_manual_init(
    pstvnc_mpeg_calibration_manual_source_t *source,
    int32_t canvas_width,
    int32_t canvas_height)
{
    if (source == NULL ||
        canvas_width < 16 ||
        canvas_height < 16)
        return 0;

    memset(source, 0, sizeof(*source));
    source->calibration.canvas_width = canvas_width;
    source->calibration.canvas_height = canvas_height;
    source->calibration.screen = PSTVNC_MPEG_CALIBRATION_INACTIVE;
    return 1;
}

int pstvnc_mpeg_calibration_owns_foreground(
    const pstvnc_mpeg_calibration_t *calibration)
{
    return calibration != NULL &&
        (calibration->screen != PSTVNC_MPEG_CALIBRATION_INACTIVE ||
         calibration->release_quarantine_pending);
}

int pstvnc_mpeg_calibration_manual_begin(
    pstvnc_mpeg_calibration_manual_source_t *source)
{
    if (source == NULL ||
        source->calibration.screen != PSTVNC_MPEG_CALIBRATION_INACTIVE ||
        source->calibration.release_quarantine_pending)
        return 0;

    assert(witness_snapshot_source != NULL);
    assert(witness_frozen != NULL);
    assert(witness_pixels != 0u);
    assert(memcmp(
        witness_snapshot_source,
        witness_frozen,
        witness_pixels * sizeof(*witness_frozen)) == 0);

    record_event(EVENT_SNAPSHOT_OBSERVED);
    record_event(EVENT_P8_BEGIN);
    source->calibration.screen = PSTVNC_MPEG_CALIBRATION_EDIT;
    return 1;
}

int pstvnc_mpeg_calibration_manual_service_controller(
    pstvnc_mpeg_calibration_manual_source_t *source,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_mpeg_calibration_manual_result_t *result)
{
    (void)controller_state;

    if (source == NULL || result == NULL)
        return 0;

    memset(result, 0, sizeof(*result));

    switch (next_manual_step) {
        case MANUAL_STEP_EDIT:
            source->calibration.screen =
                PSTVNC_MPEG_CALIBRATION_EDIT;
            result->consume_controller_state = 1u;
            return 1;

        case MANUAL_STEP_REVIEW:
            source->calibration.screen =
                PSTVNC_MPEG_CALIBRATION_REVIEW;
            result->consume_controller_state = 1u;
            return 1;

        case MANUAL_STEP_ACCEPT:
            source->calibration.screen =
                PSTVNC_MPEG_CALIBRATION_INACTIVE;
            source->calibration.release_quarantine_pending = 1u;
            result->consume_controller_state = 1u;
            result->accepted = 1u;
            result->accepted_region = accepted_region;
            return 1;

        case MANUAL_STEP_CANCEL:
            source->calibration.screen =
                PSTVNC_MPEG_CALIBRATION_INACTIVE;
            source->calibration.release_quarantine_pending = 1u;
            result->consume_controller_state = 1u;
            result->cancelled = 1u;
            return 1;

        case MANUAL_STEP_QUARANTINE_HOLD:
            source->calibration.screen =
                PSTVNC_MPEG_CALIBRATION_INACTIVE;
            source->calibration.release_quarantine_pending = 1u;
            result->consume_controller_state = 1u;
            return 1;

        case MANUAL_STEP_RELEASE:
            source->calibration.screen =
                PSTVNC_MPEG_CALIBRATION_INACTIVE;
            source->calibration.release_quarantine_pending = 0u;
            result->consume_controller_state = 1u;
            result->release_quarantine_complete = 1u;
            return 1;

        case MANUAL_STEP_NO_CONSUME:
            source->calibration.screen =
                PSTVNC_MPEG_CALIBRATION_EDIT;
            return 1;

        default:
            return 0;
    }
}

int pstvnc_mpeg_calibration_manual_prepare_plan(
    const pstvnc_mpeg_calibration_manual_source_t *source,
    pstvnc_mpeg_calibration_manual_plan_t *plan)
{
    if (source == NULL ||
        plan == NULL ||
        (source->calibration.screen != PSTVNC_MPEG_CALIBRATION_EDIT &&
         source->calibration.screen != PSTVNC_MPEG_CALIBRATION_REVIEW))
        return 0;

    memset(plan, 0, sizeof(*plan));
    plan->kind =
        source->calibration.screen == PSTVNC_MPEG_CALIBRATION_EDIT
            ? PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_EDIT
            : PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_REVIEW;
    plan->canvas_width = source->calibration.canvas_width;
    plan->canvas_height = source->calibration.canvas_height;
    plan->geometry.base.x = 16;
    plan->geometry.base.y = 16;
    plan->geometry.base.width = 32;
    plan->geometry.base.height = 32;
    plan->geometry.inner_content = plan->geometry.base;
    plan->geometry.suppression = plan->geometry.base;
    return 1;
}

int pstvnc_mpeg_calibration_manual_rasterize(
    const pstvnc_mpeg_calibration_manual_plan_t *plan,
    const uint16_t *frozen_desktop,
    size_t frozen_pixel_count,
    uint16_t *surface_pixels,
    size_t surface_capacity_pixels)
{
    size_t bytes;

    if (plan == NULL ||
        frozen_desktop == NULL ||
        surface_pixels == NULL ||
        frozen_pixel_count < witness_pixels ||
        surface_capacity_pixels < witness_pixels ||
        frozen_desktop != witness_frozen ||
        surface_pixels != witness_work)
        return 0;

    bytes = witness_pixels * sizeof(*surface_pixels);
    memcpy(surface_pixels, frozen_desktop, bytes);
    surface_pixels[witness_pixels - 1u] ^= 0x001fu;
    record_event(EVENT_RASTER);
    return 1;
}

int pstvnc_mpeg_calibration_resolve_geometry(
    const pstvnc_mpeg_calibration_t *calibration,
    const pstvnc_mpeg_calibration_region_t *region,
    pstvnc_mpeg_calibration_geometry_t *geometry)
{
    if (calibration == NULL || region == NULL || geometry == NULL)
        return 0;

    resolve_geometry_calls++;
    *geometry = resolved_geometry;
    return 1;
}

/* ----- Input/RFB/Platform seams ----- */

int pstvnc_input_runtime_suspend_mouse_interpretation(
    pstvnc_input_runtime_t *runtime)
{
    assert(runtime != NULL);
    assert(witness_flow != NULL);
    assert(witness_flow->frozen);

    record_event(EVENT_FREEZE_OBSERVED);
    record_event(EVENT_SUSPEND);

    if (fail_suspend)
        return -1;

    runtime->mouse_interpretation_suspend_requested = 1;
    runtime->mouse_interpretation_suspended = 1;
    return 0;
}

int pstvnc_input_runtime_rebase_suspended_mouse_state(
    pstvnc_input_runtime_t *runtime,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char published_click_buttons)
{
    assert(runtime != NULL);
    assert(runtime->mouse_interpretation_suspended);
    assert(published_cursor_x == 21u);
    assert(published_cursor_y == 17u);
    assert(published_click_buttons == 0u);
    record_event(EVENT_REBASE);

    if (fail_rebase)
        return -1;

    runtime->suspended_mouse_state_rebased = 1;
    return 0;
}

int pstvnc_input_runtime_resume_mouse_interpretation(
    pstvnc_input_runtime_t *runtime)
{
    assert(runtime != NULL);
    assert(restore_presented);
    record_event(EVENT_RESUME);

    if (fail_resume)
        return -1;

    runtime->mouse_interpretation_suspend_requested = 0;
    runtime->mouse_interpretation_suspended = 0;
    runtime->suspended_mouse_state_rebased = 0;
    return 0;
}

int pstvnc_rfb_session_send_pointer_event(
    pstvnc_rfb_session_t *session,
    uint8_t button_mask,
    uint16_t x,
    uint16_t y)
{
    assert(session != NULL);
    assert(button_mask == 0u);
    assert(x == 21u);
    assert(y == 17u);

    pointer_release_calls++;
    record_event(EVENT_POINTER_RELEASE);

    return fail_pointer_release ? 0 : 1;
}

int pstvnc_ps2_graphics_present(
    const uint16_t *desktop_pixels,
    size_t desktop_pixel_count,
    const pstvnc_ps2_graphics_overlay_t *local_overlay)
{
    assert(desktop_pixel_count == witness_pixels);
    assert(local_overlay == NULL);

    if (desktop_pixels == witness_work) {
        calibration_present_calls++;
        record_event(EVENT_PRESENT_CALIBRATION);
        return fail_calibration_present ? -1 : 0;
    }

    if (desktop_pixels == witness_frozen) {
        restore_present_calls++;
        record_event(EVENT_PRESENT_RESTORE);

        if (fail_restore_present)
            return -1;

        restore_presented = 1;
        return 0;
    }

    return -1;
}

/* ----- Focused P9 proofs ----- */

static void assert_admission_rejected_without_freeze(
    test_environment_t *environment)
{
    assert(begin_environment(environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_ADMISSION_REJECTED);
    assert(!environment->flow.frozen);
    assert(event_count == 0u);
    assert(pstvnc_app_mpeg_calibration_state(
        &environment->calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_IDLE);
}

static void test_admission_requires_exact_rfb_only_desktop_state(void)
{
    test_environment_t environment;

    environment_init(&environment);
    environment.local_ui.foreground = PSTVNC_LOCAL_UI_FOREGROUND_OSK;
    assert_admission_rejected_without_freeze(&environment);

    environment_init(&environment);
    environment.local_ui.input_quarantined = 1;
    assert_admission_rejected_without_freeze(&environment);

    environment_init(&environment);
    environment.presentation.state =
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
    environment.presentation.snapshot_valid = 1u;
    environment.presentation.run_generation = 7u;
    assert_admission_rejected_without_freeze(&environment);

    environment_init(&environment);
    environment.presentation.snapshot_valid = 1u;
    assert_admission_rejected_without_freeze(&environment);

    environment_init(&environment);
    environment.flow.frozen = 1u;
    assert_admission_rejected_without_freeze(&environment);

    environment_init(&environment);
    environment.calibration.manual_source.calibration.screen =
        PSTVNC_MPEG_CALIBRATION_EDIT;
    assert_admission_rejected_without_freeze(&environment);

    environment_init(&environment);
    environment.calibration.state =
        PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED;
    assert_admission_rejected_without_freeze(&environment);
}

static void test_begin_order_and_immutable_snapshot(void)
{
    test_environment_t environment;

    environment_init(&environment);
    environment.published_clicks = 1u;

    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_OK);

    assert(environment.flow.frozen);
    assert(environment.published_clicks == 0u);
    assert(pointer_release_calls == 1u);
    assert(environment.calibration.mouse_interpretation_suspended);
    assert(memcmp(
        environment.last_desktop,
        environment.last_desktop_before,
        sizeof(environment.last_desktop)) == 0);
    assert(memcmp(
        environment.frozen,
        environment.last_desktop_before,
        sizeof(environment.frozen)) == 0);
    assert(environment.work[TEST_PIXELS - 1u] !=
        environment.frozen[TEST_PIXELS - 1u]);

    assert(event_count == 8u);
    assert(events[0] == EVENT_FREEZE_OBSERVED);
    assert(events[1] == EVENT_SUSPEND);
    assert(events[2] == EVENT_POINTER_RELEASE);
    assert(events[3] == EVENT_REBASE);
    assert(events[4] == EVENT_SNAPSHOT_OBSERVED);
    assert(events[5] == EVENT_P8_BEGIN);
    assert(events[6] == EVENT_RASTER);
    assert(events[7] == EVENT_PRESENT_CALIBRATION);

    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_ACTIVE);
}

static void test_neutral_pointer_skips_release_serialization(void)
{
    test_environment_t environment;

    environment_init(&environment);
    environment.published_clicks = 0u;

    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_OK);
    assert(pointer_release_calls == 0u);
    assert(find_event(EVENT_POINTER_RELEASE) == event_count);
    assert(find_event(EVENT_REBASE) <
        find_event(EVENT_SNAPSHOT_OBSERVED));
}

static void test_failed_pointer_release_fails_closed(void)
{
    test_environment_t environment;

    environment_init(&environment);
    environment.published_clicks = 1u;
    fail_pointer_release = 1;

    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_POINTER_RELEASE_FAILED);
    assert(environment.flow.frozen);
    assert(environment.published_clicks == 1u);
    assert(environment.calibration.mouse_interpretation_suspended);
    assert(find_event(EVENT_REBASE) == event_count);
    assert(find_event(EVENT_P8_BEGIN) == event_count);
    assert(find_event(EVENT_RESUME) == event_count);
    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_FAULTED);
}

static void test_platform_failure_contains_foreground(void)
{
    test_environment_t environment;

    environment_init(&environment);
    fail_calibration_present = 1;

    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_PRESENT_FAILED);
    assert(environment.flow.frozen);
    assert(environment.calibration.mouse_interpretation_suspended);
    assert(find_event(EVENT_RESUME) == event_count);
    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_FAULTED);
}

static void test_visible_edit_review_service_and_no_leakage(void)
{
    test_environment_t environment;
    pstvnc_app_mpeg_calibration_service_result_t result;
    pstvnc_controller_state_t sample = controller_sample();

    environment_init(&environment);
    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_OK);
    assert(calibration_present_calls == 1u);

    next_manual_step = MANUAL_STEP_REVIEW;
    result = service_ok(&environment);
    assert(result.consume_controller_state);
    assert(!result.accepted_edge);
    assert(!result.cancelled_edge);
    assert(calibration_present_calls == 2u);
    assert(memcmp(
        environment.last_desktop,
        environment.last_desktop_before,
        sizeof(environment.last_desktop)) == 0);

    next_manual_step = MANUAL_STEP_EDIT;
    result = service_ok(&environment);
    assert(result.consume_controller_state);
    assert(calibration_present_calls == 3u);

    next_manual_step = MANUAL_STEP_NO_CONSUME;
    assert(pstvnc_app_mpeg_calibration_service_controller(
        &environment.calibration,
        &sample,
        &result) ==
        PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION);
    assert(environment.flow.frozen);
    assert(environment.calibration.mouse_interpretation_suspended);
    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_FAULTED);
}

static void seed_exact_geometry(void)
{
    accepted_region.x = 11;
    accepted_region.y = 13;
    accepted_region.width = 32;
    accepted_region.height = 16;
    accepted_region.inner_matte_x = 3;
    accepted_region.inner_matte_y = 2;
    accepted_region.outer_matte_x = 5;
    accepted_region.outer_matte_y = 7;

    resolved_geometry.base.x = 11;
    resolved_geometry.base.y = 13;
    resolved_geometry.base.width = 32;
    resolved_geometry.base.height = 16;

    resolved_geometry.inner_content.x = 14;
    resolved_geometry.inner_content.y = 15;
    resolved_geometry.inner_content.width = 26;
    resolved_geometry.inner_content.height = 12;

    resolved_geometry.suppression.x = 6;
    resolved_geometry.suppression.y = 6;
    resolved_geometry.suppression.width = 42;
    resolved_geometry.suppression.height = 30;
}

static void test_accept_waits_for_release_then_stays_protected(void)
{
    test_environment_t environment;
    pstvnc_app_mpeg_calibration_service_result_t result;
    pstvnc_mpeg_presentation_geometry_t copied;
    size_t events_before_copy;

    environment_init(&environment);
    seed_exact_geometry();

    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_OK);

    next_manual_step = MANUAL_STEP_ACCEPT;
    result = service_ok(&environment);
    assert(result.consume_controller_state);
    assert(result.accepted_edge);
    assert(!result.foreground_completed);
    assert(resolve_geometry_calls == 1u);
    assert(environment.flow.frozen);
    assert(environment.calibration.mouse_interpretation_suspended);
    assert(!pstvnc_app_mpeg_calibration_copy_accepted_geometry(
        &environment.calibration,
        &copied));

    next_manual_step = MANUAL_STEP_QUARANTINE_HOLD;
    result = service_ok(&environment);
    assert(result.consume_controller_state);
    assert(!result.foreground_completed);
    assert(environment.flow.frozen);
    assert(environment.calibration.mouse_interpretation_suspended);
    assert(restore_present_calls == 0u);

    restore_presented = 0;
    next_manual_step = MANUAL_STEP_RELEASE;
    result = service_ok(&environment);
    assert(result.consume_controller_state);
    assert(result.foreground_completed);
    assert(result.accepted_protected);
    assert(!result.cancelled_completed);
    assert(environment.flow.frozen);
    assert(!environment.calibration.mouse_interpretation_suspended);
    assert(environment.presentation.state ==
        PSTVNC_MPEG_PRESENTATION_RFB_ONLY);
    assert(!environment.presentation.snapshot_valid);
    assert(environment.presentation.run_generation == 0u);
    assert(find_event(EVENT_PRESENT_RESTORE) <
        find_event(EVENT_RESUME));
    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED);

    events_before_copy = event_count;
    assert(pstvnc_app_mpeg_calibration_copy_accepted_geometry(
        &environment.calibration,
        &copied));
    assert(memcmp(
        &copied.base,
        &resolved_geometry.base,
        sizeof(copied.base)) == 0);
    assert(memcmp(
        &copied.inner_content,
        &resolved_geometry.inner_content,
        sizeof(copied.inner_content)) == 0);
    assert(memcmp(
        &copied.suppression,
        &resolved_geometry.suppression,
        sizeof(copied.suppression)) == 0);
    assert(event_count == events_before_copy);
    assert(environment.flow.frozen);

    assert(pstvnc_app_mpeg_calibration_abort_accepted(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_OK);
    assert(!environment.flow.frozen);
    assert(environment.flow.full_refresh_pending);
    assert(pstvnc_rfb_flow_policy_next_request(
        &environment.flow) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);
    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_IDLE);
    assert(!pstvnc_app_mpeg_calibration_copy_accepted_geometry(
        &environment.calibration,
        &copied));
}

static void test_cancel_restores_then_resumes_then_thaws_with_full_debt(void)
{
    test_environment_t environment;
    pstvnc_app_mpeg_calibration_service_result_t result;

    environment_init(&environment);
    environment.flow.request_outstanding = 1u;

    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_OK);

    next_manual_step = MANUAL_STEP_CANCEL;
    result = service_ok(&environment);
    assert(result.cancelled_edge);
    assert(!result.foreground_completed);
    assert(environment.flow.frozen);
    assert(environment.calibration.mouse_interpretation_suspended);

    next_manual_step = MANUAL_STEP_QUARANTINE_HOLD;
    result = service_ok(&environment);
    assert(!result.foreground_completed);
    assert(environment.flow.frozen);

    restore_presented = 0;
    next_manual_step = MANUAL_STEP_RELEASE;
    result = service_ok(&environment);
    assert(result.foreground_completed);
    assert(result.cancelled_completed);
    assert(!result.accepted_protected);
    assert(!environment.flow.frozen);
    assert(environment.flow.full_refresh_pending);
    assert(!environment.calibration.mouse_interpretation_suspended);
    assert(find_event(EVENT_PRESENT_RESTORE) <
        find_event(EVENT_RESUME));
    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_IDLE);

    assert(pstvnc_rfb_flow_policy_next_request(
        &environment.flow) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);
    assert(pstvnc_rfb_flow_policy_record_update_complete(
        &environment.flow));
    assert(pstvnc_rfb_flow_policy_next_request(
        &environment.flow) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);
}

static void test_restore_or_p3_failure_never_silently_releases(void)
{
    test_environment_t environment;
    pstvnc_app_mpeg_calibration_service_result_t result;
    pstvnc_controller_state_t sample = controller_sample();

    environment_init(&environment);
    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_OK);
    next_manual_step = MANUAL_STEP_CANCEL;
    (void)service_ok(&environment);
    fail_restore_present = 1;
    next_manual_step = MANUAL_STEP_RELEASE;

    assert(pstvnc_app_mpeg_calibration_service_controller(
        &environment.calibration,
        &sample,
        &result) ==
        PSTVNC_APP_MPEG_CALIBRATION_PRESENT_FAILED);
    assert(environment.flow.frozen);
    assert(environment.calibration.mouse_interpretation_suspended);
    assert(find_event(EVENT_RESUME) == event_count);
    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_FAULTED);

    environment_init(&environment);
    assert(begin_environment(&environment) ==
        PSTVNC_APP_MPEG_CALIBRATION_OK);
    environment.presentation.state =
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME;
    next_manual_step = MANUAL_STEP_EDIT;

    assert(pstvnc_app_mpeg_calibration_service_controller(
        &environment.calibration,
        &sample,
        &result) ==
        PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION);
    assert(environment.flow.frozen);
    assert(environment.calibration.mouse_interpretation_suspended);
    assert(pstvnc_app_mpeg_calibration_state(
        &environment.calibration) ==
        PSTVNC_APP_MPEG_CALIBRATION_FAULTED);
}

int main(void)
{
    test_admission_requires_exact_rfb_only_desktop_state();
    test_begin_order_and_immutable_snapshot();
    test_neutral_pointer_skips_release_serialization();
    test_failed_pointer_release_fails_closed();
    test_platform_failure_contains_foreground();
    test_visible_edit_review_service_and_no_leakage();
    test_accept_waits_for_release_then_stays_protected();
    test_cancel_restores_then_resumes_then_thaws_with_full_debt();
    test_restore_or_p3_failure_never_silently_releases();

    puts("APP_MPEG_CALIBRATION_TEST=PASS");
    return 0;
}
