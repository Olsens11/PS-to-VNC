/*
 * File synopsis:
 * Focused host proof for the A004 P8 UI-owned manual MPEG CALIBRATION region
 * source. Tests use the real accepted P1 core and neutral controller vocabulary
 * to prove explicit begin, semantic mapping, foreground/quarantine consumption,
 * one-shot copied acceptance, exact P1-resolved visual geometry and immutable
 * caller-owned CT16 raster preparation.
 */

#include "ui/mpeg_calibration_manual.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_WIDTH 64
#define TEST_HEIGHT 64
#define TEST_PIXELS ((size_t)TEST_WIDTH * (size_t)TEST_HEIGHT)

static pstvnc_controller_state_t controller_sample(
    uint16_t down,
    uint16_t pressed,
    uint16_t released)
{
    pstvnc_controller_state_t state;

    memset(&state, 0, sizeof(state));
    state.buttons_down = down;
    state.buttons_pressed = pressed;
    state.buttons_released = released;
    return state;
}

static void service_ok(
    pstvnc_mpeg_calibration_manual_source_t *source,
    uint16_t down,
    uint16_t pressed,
    uint16_t released,
    pstvnc_mpeg_calibration_manual_result_t *result)
{
    pstvnc_controller_state_t state =
        controller_sample(down, pressed, released);

    assert(pstvnc_mpeg_calibration_manual_service_controller(
        source,
        &state,
        result));
}

static void init_and_begin(
    pstvnc_mpeg_calibration_manual_source_t *source)
{
    assert(pstvnc_mpeg_calibration_manual_init(
        source,
        TEST_WIDTH,
        TEST_HEIGHT));
    assert(pstvnc_mpeg_calibration_manual_begin(source));
    assert(source->calibration.screen == PSTVNC_MPEG_CALIBRATION_EDIT);
    assert(pstvnc_mpeg_calibration_owns_foreground(
        &source->calibration));
}

static void test_explicit_begin_and_no_hardwired_entry_chord(void)
{
    pstvnc_mpeg_calibration_manual_source_t source;
    pstvnc_mpeg_calibration_manual_result_t result;
    uint16_t chord =
        PSTVNC_CONTROLLER_BUTTON_START |
        PSTVNC_CONTROLLER_BUTTON_SELECT;

    assert(pstvnc_mpeg_calibration_manual_init(
        &source,
        TEST_WIDTH,
        TEST_HEIGHT));

    service_ok(&source, chord, chord, 0u, &result);
    assert(!result.consume_controller_state);
    assert(!result.accepted);
    assert(!result.cancelled);
    assert(source.calibration.screen ==
        PSTVNC_MPEG_CALIBRATION_INACTIVE);
    assert(!pstvnc_mpeg_calibration_owns_foreground(
        &source.calibration));

    assert(pstvnc_mpeg_calibration_manual_begin(&source));
    assert(source.calibration.screen ==
        PSTVNC_MPEG_CALIBRATION_EDIT);
}

static void test_normalized_controller_mapping(void)
{
    pstvnc_mpeg_calibration_manual_source_t source;
    pstvnc_mpeg_calibration_manual_result_t result;

    init_and_begin(&source);
    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_RIGHT,
        0u,
        0u,
        &result);
    assert(result.consume_controller_state);
    assert(source.calibration.current.width == 48);
    assert(source.calibration.current.x == 8);

    assert(pstvnc_mpeg_calibration_manual_init(
        &source,
        TEST_WIDTH,
        TEST_HEIGHT));
    assert(pstvnc_mpeg_calibration_manual_begin(&source));
    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_LEFT,
        0u,
        0u,
        &result);
    assert(source.calibration.current.width == 16);
    assert(source.calibration.current.x == 24);

    assert(pstvnc_mpeg_calibration_manual_init(
        &source,
        TEST_WIDTH,
        TEST_HEIGHT));
    assert(pstvnc_mpeg_calibration_manual_begin(&source));
    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_UP,
        0u,
        0u,
        &result);
    assert(source.calibration.current.height == 48);
    assert(source.calibration.current.y == 8);

    assert(pstvnc_mpeg_calibration_manual_init(
        &source,
        TEST_WIDTH,
        TEST_HEIGHT));
    assert(pstvnc_mpeg_calibration_manual_begin(&source));
    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_DOWN,
        0u,
        0u,
        &result);
    assert(source.calibration.current.height == 16);
    assert(source.calibration.current.y == 24);

    assert(pstvnc_mpeg_calibration_manual_init(
        &source,
        TEST_WIDTH,
        TEST_HEIGHT));
    assert(pstvnc_mpeg_calibration_manual_begin(&source));
    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_R1 |
            PSTVNC_CONTROLLER_BUTTON_RIGHT |
            PSTVNC_CONTROLLER_BUTTON_DOWN,
        0u,
        0u,
        &result);
    assert(source.calibration.current.x == 17);
    assert(source.calibration.current.y == 17);
    assert(source.calibration.current.width == 32);
    assert(source.calibration.current.height == 32);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_R2 |
            PSTVNC_CONTROLLER_BUTTON_RIGHT,
        0u,
        0u,
        &result);
    assert(source.calibration.current.inner_matte_x == 1);
    assert(source.calibration.current.inner_matte_y == 1);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_L2 |
            PSTVNC_CONTROLLER_BUTTON_LEFT,
        0u,
        0u,
        &result);
    assert(source.calibration.current.outer_matte_x == 1);
    assert(source.calibration.current.outer_matte_y == 1);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_START,
        PSTVNC_CONTROLLER_BUTTON_START,
        0u,
        &result);
    assert(source.calibration.current.x == source.calibration.defaults.x);
    assert(source.calibration.current.y == source.calibration.defaults.y);
    assert(source.calibration.current.width ==
        source.calibration.defaults.width);
    assert(source.calibration.current.height ==
        source.calibration.defaults.height);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        0u,
        &result);
    assert(source.calibration.screen == PSTVNC_MPEG_CALIBRATION_REVIEW);
    assert(!result.accepted);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        0u,
        &result);
    assert(source.calibration.screen == PSTVNC_MPEG_CALIBRATION_EDIT);
    assert(!result.cancelled);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        0u,
        &result);
    assert(result.cancelled);
    assert(!result.accepted);
    assert(source.calibration.screen == PSTVNC_MPEG_CALIBRATION_INACTIVE);
}

static void test_one_shot_accept_and_release_quarantine(void)
{
    pstvnc_mpeg_calibration_manual_source_t source;
    pstvnc_mpeg_calibration_manual_result_t result;
    pstvnc_mpeg_calibration_region_t accepted;

    init_and_begin(&source);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_R1 |
            PSTVNC_CONTROLLER_BUTTON_RIGHT,
        0u,
        0u,
        &result);
    assert(source.calibration.current.x == 17);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        0u,
        &result);
    assert(result.consume_controller_state);
    assert(!result.accepted);
    assert(source.calibration.screen == PSTVNC_MPEG_CALIBRATION_REVIEW);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        0u,
        0u,
        &result);
    assert(result.consume_controller_state);
    assert(!result.accepted);
    assert(!source.calibration.review_accept_armed);

    service_ok(
        &source,
        0u,
        0u,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        &result);
    assert(result.consume_controller_state);
    assert(!result.accepted);
    assert(source.calibration.review_accept_armed);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        0u,
        &result);
    assert(result.consume_controller_state);
    assert(result.accepted);
    assert(!result.cancelled);
    assert(!result.release_quarantine_complete);
    accepted = result.accepted_region;
    assert(accepted.x == 17);
    assert(source.calibration.release_quarantine_pending);
    assert(source.calibration.screen == PSTVNC_MPEG_CALIBRATION_INACTIVE);

    source.calibration.committed.x = 33;
    assert(result.accepted_region.x == accepted.x);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        0u,
        0u,
        &result);
    assert(result.consume_controller_state);
    assert(!result.accepted);
    assert(!result.release_quarantine_complete);

    service_ok(
        &source,
        0u,
        0u,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        &result);
    assert(result.consume_controller_state);
    assert(!result.accepted);
    assert(result.release_quarantine_complete);
    assert(!pstvnc_mpeg_calibration_owns_foreground(
        &source.calibration));

    service_ok(&source, 0u, 0u, 0u, &result);
    assert(!result.consume_controller_state);
    assert(!result.accepted);
    assert(!result.release_quarantine_complete);
}

static void test_cancel_owns_quarantine_without_acceptance(void)
{
    pstvnc_mpeg_calibration_manual_source_t source;
    pstvnc_mpeg_calibration_manual_result_t result;

    init_and_begin(&source);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        0u,
        &result);
    assert(result.consume_controller_state);
    assert(result.cancelled);
    assert(!result.accepted);
    assert(source.calibration.release_quarantine_pending);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        0u,
        0u,
        &result);
    assert(result.consume_controller_state);
    assert(!result.release_quarantine_complete);
    assert(!result.accepted);

    service_ok(
        &source,
        0u,
        0u,
        PSTVNC_CONTROLLER_BUTTON_CIRCLE,
        &result);
    assert(result.consume_controller_state);
    assert(result.release_quarantine_complete);
    assert(!result.accepted);
    assert(!pstvnc_mpeg_calibration_owns_foreground(
        &source.calibration));
}

static void test_visual_plan_uses_exact_p1_geometry(void)
{
    pstvnc_mpeg_calibration_manual_source_t source;
    pstvnc_mpeg_calibration_manual_result_t result;
    pstvnc_mpeg_calibration_manual_plan_t plan;
    pstvnc_mpeg_calibration_geometry_t expected;

    init_and_begin(&source);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_R1 |
            PSTVNC_CONTROLLER_BUTTON_RIGHT,
        0u,
        0u,
        &result);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_R2 |
            PSTVNC_CONTROLLER_BUTTON_RIGHT,
        0u,
        0u,
        &result);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_L2 |
            PSTVNC_CONTROLLER_BUTTON_LEFT,
        0u,
        0u,
        &result);

    assert((source.calibration.current.width & 15) == 0);
    assert((source.calibration.current.height & 15) == 0);
    assert(source.calibration.current.x == 17);

    assert(pstvnc_mpeg_calibration_resolve_geometry(
        &source.calibration,
        &source.calibration.current,
        &expected));

    assert(pstvnc_mpeg_calibration_manual_prepare_plan(
        &source,
        &plan));
    assert(plan.kind == PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_EDIT);
    assert(memcmp(
        &plan.candidate_region,
        &source.calibration.current,
        sizeof(plan.candidate_region)) == 0);
    assert(memcmp(
        &plan.geometry,
        &expected,
        sizeof(plan.geometry)) == 0);
    assert(plan.geometry.base.x == 17);
    assert(plan.geometry.inner_content.x == 18);
    assert(plan.geometry.inner_content.y == 17);
    assert(plan.geometry.suppression.x == 16);
    assert(plan.title != NULL);
    assert(plan.help_primary != NULL);
    assert(plan.help_secondary != NULL);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        0u,
        &result);
    assert(pstvnc_mpeg_calibration_manual_prepare_plan(
        &source,
        &plan));
    assert(plan.kind == PSTVNC_MPEG_CALIBRATION_MANUAL_VIEW_REVIEW);
    assert(!plan.review_accept_armed);

    service_ok(
        &source,
        0u,
        0u,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        &result);
    assert(pstvnc_mpeg_calibration_manual_prepare_plan(
        &source,
        &plan));
    assert(plan.review_accept_armed);
}

static void grow_inner_and_outer_mattes(
    pstvnc_mpeg_calibration_manual_source_t *source)
{
    pstvnc_mpeg_calibration_manual_result_t result;
    int i;

    for (i = 0; i < 3; i++) {
        service_ok(
            source,
            PSTVNC_CONTROLLER_BUTTON_R2 |
                PSTVNC_CONTROLLER_BUTTON_RIGHT,
            0u,
            0u,
            &result);
        service_ok(
            source,
            PSTVNC_CONTROLLER_BUTTON_L2 |
                PSTVNC_CONTROLLER_BUTTON_LEFT,
            0u,
            0u,
            &result);
    }
}

static void test_raster_is_caller_owned_and_geometry_driven(void)
{
    pstvnc_mpeg_calibration_manual_source_t source;
    pstvnc_mpeg_calibration_manual_result_t result;
    pstvnc_mpeg_calibration_manual_plan_t plan;
    pstvnc_mpeg_calibration_manual_plan_t invalid_plan;
    uint16_t frozen[TEST_PIXELS];
    uint16_t frozen_before[TEST_PIXELS];
    uint16_t surface[TEST_PIXELS];
    uint16_t sentinel[TEST_PIXELS];
    size_t i;
    size_t outer_index;
    size_t inner_index;
    size_t matte_index;
    size_t selector_index;

    init_and_begin(&source);
    grow_inner_and_outer_mattes(&source);

    assert(pstvnc_mpeg_calibration_manual_prepare_plan(
        &source,
        &plan));

    for (i = 0u; i < TEST_PIXELS; i++)
        frozen[i] = (uint16_t)(0x8000u | (uint16_t)(i & 0x7fffu));

    memcpy(frozen_before, frozen, sizeof(frozen));

    assert(pstvnc_mpeg_calibration_manual_rasterize(
        &plan,
        frozen,
        TEST_PIXELS,
        surface,
        TEST_PIXELS));

    assert(memcmp(frozen, frozen_before, sizeof(frozen)) == 0);

    outer_index =
        (size_t)plan.geometry.suppression.y * TEST_WIDTH +
        (size_t)plan.geometry.suppression.x;
    assert(surface[outer_index] != frozen[outer_index]);

    inner_index =
        (size_t)(plan.geometry.inner_content.y +
            plan.geometry.inner_content.height / 2) * TEST_WIDTH +
        (size_t)(plan.geometry.inner_content.x +
            plan.geometry.inner_content.width / 2);
    assert(surface[inner_index] == frozen[inner_index]);

    matte_index =
        (size_t)(plan.geometry.base.y + 5) * TEST_WIDTH +
        (size_t)(plan.geometry.base.x + 2);
    assert(surface[matte_index] ==
        PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_BLACK);

    selector_index =
        (size_t)plan.geometry.base.y * TEST_WIDTH +
        (size_t)plan.geometry.base.x;
    assert(surface[selector_index] ==
        PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_ORANGE);
    assert(surface[(size_t)2 * TEST_WIDTH + 2u] ==
        PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_ORANGE);

    service_ok(
        &source,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        PSTVNC_CONTROLLER_BUTTON_CROSS,
        0u,
        &result);
    assert(pstvnc_mpeg_calibration_manual_prepare_plan(
        &source,
        &plan));
    assert(pstvnc_mpeg_calibration_manual_rasterize(
        &plan,
        frozen,
        TEST_PIXELS,
        surface,
        TEST_PIXELS));
    assert(surface[(size_t)2 * TEST_WIDTH + 2u] ==
        PSTVNC_MPEG_CALIBRATION_MANUAL_COLOR_TEAL);

    memset(sentinel, 0x5a, sizeof(sentinel));
    assert(!pstvnc_mpeg_calibration_manual_rasterize(
        &plan,
        frozen,
        TEST_PIXELS - 1u,
        sentinel,
        TEST_PIXELS));
    for (i = 0u; i < TEST_PIXELS; i++)
        assert(sentinel[i] == (uint16_t)0x5a5au);

    assert(!pstvnc_mpeg_calibration_manual_rasterize(
        &plan,
        frozen,
        TEST_PIXELS,
        sentinel,
        TEST_PIXELS - 1u));

    assert(!pstvnc_mpeg_calibration_manual_rasterize(
        &plan,
        frozen,
        TEST_PIXELS,
        (uint16_t *)frozen,
        TEST_PIXELS));

    invalid_plan = plan;
    invalid_plan.geometry.base.width = TEST_WIDTH + 1;
    assert(!pstvnc_mpeg_calibration_manual_rasterize(
        &invalid_plan,
        frozen,
        TEST_PIXELS,
        sentinel,
        TEST_PIXELS));
}

static void test_invalid_controller_fact_fails_without_state_change(void)
{
    pstvnc_mpeg_calibration_manual_source_t source;
    pstvnc_mpeg_calibration_manual_result_t result;
    pstvnc_controller_state_t invalid;
    pstvnc_mpeg_calibration_region_t before;

    init_and_begin(&source);
    before = source.calibration.current;

    invalid = controller_sample(
        0u,
        PSTVNC_CONTROLLER_BUTTON_RIGHT,
        0u);

    assert(!pstvnc_mpeg_calibration_manual_service_controller(
        &source,
        &invalid,
        &result));
    assert(memcmp(
        &source.calibration.current,
        &before,
        sizeof(before)) == 0);
}

int main(void)
{
    test_explicit_begin_and_no_hardwired_entry_chord();
    test_normalized_controller_mapping();
    test_one_shot_accept_and_release_quarantine();
    test_cancel_owns_quarantine_without_acceptance();
    test_visual_plan_uses_exact_p1_geometry();
    test_raster_is_caller_owned_and_geometry_driven();
    test_invalid_controller_fact_fails_without_state_change();

    puts("MPEG_CALIBRATION_MANUAL_TEST=PASS");
    return 0;
}
