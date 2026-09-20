/*
 * File synopsis:
 * Host contract tests for the clean A004 MPEG CALIBRATION value/foreground
 * owner. These tests exercise geometry meanings, recovered edit controls,
 * held-X acceptance safety, exactly-once acceptance, cancellation, release
 * quarantine, and separation from presentation/ownership side effects.
 */

#include "mpeg_calibration.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static pstvnc_mpeg_calibration_input_t sample(
    uint32_t down,
    uint32_t pressed)
{
    pstvnc_mpeg_calibration_input_t input;

    input.down = down;
    input.pressed = pressed;
    return input;
}

static void update_ok(
    pstvnc_mpeg_calibration_t *calibration,
    uint32_t down,
    uint32_t pressed,
    pstvnc_mpeg_calibration_effects_t *effects)
{
    pstvnc_mpeg_calibration_input_t input = sample(down, pressed);

    assert(pstvnc_mpeg_calibration_update(
        calibration,
        &input,
        effects));
}

static void complete_release_quarantine(
    pstvnc_mpeg_calibration_t *calibration)
{
    pstvnc_mpeg_calibration_effects_t effects;

    update_ok(calibration, 0, 0, &effects);
    assert(effects.release_quarantine_complete);
    assert(!pstvnc_mpeg_calibration_owns_foreground(calibration));
}

static void test_default_and_edit_geometry(void)
{
    pstvnc_mpeg_calibration_t calibration;
    pstvnc_mpeg_calibration_effects_t effects;
    pstvnc_mpeg_calibration_geometry_t geometry;

    assert(pstvnc_mpeg_calibration_init(&calibration, 704, 462));

    assert(calibration.defaults.x == 176);
    assert(calibration.defaults.y == 119);
    assert(calibration.defaults.width == 352);
    assert(calibration.defaults.height == 224);

    assert(pstvnc_mpeg_calibration_begin(&calibration));
    assert(pstvnc_mpeg_calibration_owns_foreground(&calibration));

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_RIGHT,
        0,
        &effects);

    assert(calibration.current.x == 168);
    assert(calibration.current.width == 368);

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_MOVE |
            PSTVNC_MPEG_CALIBRATION_RIGHT |
            PSTVNC_MPEG_CALIBRATION_DOWN,
        0,
        &effects);

    assert(calibration.current.x == 169);
    assert(calibration.current.y == 120);

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_INNER_MATTE |
            PSTVNC_MPEG_CALIBRATION_RIGHT,
        0,
        &effects);

    assert(calibration.current.inner_matte_x == 1);
    assert(calibration.current.inner_matte_y == 1);

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_OUTER_MATTE |
            PSTVNC_MPEG_CALIBRATION_LEFT,
        0,
        &effects);

    assert(calibration.current.outer_matte_x == 1);
    assert(calibration.current.outer_matte_y == 1);

    assert(pstvnc_mpeg_calibration_resolve_geometry(
        &calibration,
        &calibration.current,
        &geometry));

    assert(geometry.base.x == 169);
    assert(geometry.base.y == 120);
    assert(geometry.base.width == 368);
    assert(geometry.base.height == 224);

    assert(geometry.inner_content.x == 170);
    assert(geometry.inner_content.y == 121);
    assert(geometry.inner_content.width == 366);
    assert(geometry.inner_content.height == 222);

    assert(geometry.suppression.x == 168);
    assert(geometry.suppression.y == 119);
    assert(geometry.suppression.width == 370);
    assert(geometry.suppression.height == 226);
}

static void test_geometry_bounds_and_overflow_rejection(void)
{
    pstvnc_mpeg_calibration_t calibration;
    pstvnc_mpeg_calibration_geometry_t geometry;
    pstvnc_mpeg_calibration_region_t region;

    assert(!pstvnc_mpeg_calibration_init(&calibration, 15, 462));
    assert(!pstvnc_mpeg_calibration_init(&calibration, 704, 15));
    assert(pstvnc_mpeg_calibration_init(&calibration, 704, 462));

    region = calibration.defaults;
    region.width = 351;
    assert(!pstvnc_mpeg_calibration_resolve_geometry(
        &calibration, &region, &geometry));

    region = calibration.defaults;
    region.x = INT32_MAX;
    assert(!pstvnc_mpeg_calibration_resolve_geometry(
        &calibration, &region, &geometry));

    region = calibration.defaults;
    region.outer_matte_x = INT32_MAX;
    assert(!pstvnc_mpeg_calibration_resolve_geometry(
        &calibration, &region, &geometry));

    region = calibration.defaults;
    region.x = 0;
    region.y = 0;
    region.outer_matte_x = 704;
    region.outer_matte_y = 462;

    assert(pstvnc_mpeg_calibration_resolve_geometry(
        &calibration, &region, &geometry));

    assert(geometry.suppression.x == 0);
    assert(geometry.suppression.y == 0);
    assert(geometry.suppression.width == 704);
    assert(geometry.suppression.height == 462);
}

static void test_held_review_control_requires_release_repress(void)
{
    pstvnc_mpeg_calibration_t calibration;
    pstvnc_mpeg_calibration_effects_t effects;
    pstvnc_mpeg_calibration_region_t expected;

    assert(pstvnc_mpeg_calibration_init(&calibration, 704, 462));
    assert(pstvnc_mpeg_calibration_begin(&calibration));

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_MOVE |
            PSTVNC_MPEG_CALIBRATION_RIGHT,
        0,
        &effects);

    expected = calibration.current;

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        &effects);

    assert(calibration.screen == PSTVNC_MPEG_CALIBRATION_REVIEW);
    assert(!effects.accepted);
    assert(!calibration.review_accept_armed);

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        0,
        &effects);

    assert(!effects.accepted);
    assert(!calibration.review_accept_armed);

    update_ok(&calibration, 0, 0, &effects);
    assert(calibration.review_accept_armed);
    assert(!effects.accepted);

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        &effects);

    assert(effects.accepted);
    assert(!effects.cancelled);
    assert(memcmp(
        &effects.accepted_region,
        &expected,
        sizeof(expected)) == 0);

    assert(calibration.has_committed);
    assert(memcmp(
        &calibration.committed,
        &expected,
        sizeof(expected)) == 0);

    assert(calibration.screen == PSTVNC_MPEG_CALIBRATION_INACTIVE);
    assert(pstvnc_mpeg_calibration_owns_foreground(&calibration));

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        0,
        &effects);

    assert(!effects.accepted);
    assert(!effects.cancelled);

    complete_release_quarantine(&calibration);
}

static void test_cancel_does_not_publish_acceptance(void)
{
    pstvnc_mpeg_calibration_t calibration;
    pstvnc_mpeg_calibration_effects_t effects;
    pstvnc_mpeg_calibration_region_t original;

    assert(pstvnc_mpeg_calibration_init(&calibration, 704, 462));
    original = calibration.committed;

    assert(pstvnc_mpeg_calibration_begin(&calibration));

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_RIGHT,
        0,
        &effects);

    assert(memcmp(
        &calibration.current,
        &original,
        sizeof(original)) != 0);

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_CANCEL,
        PSTVNC_MPEG_CALIBRATION_CANCEL,
        &effects);

    assert(effects.cancelled);
    assert(!effects.accepted);
    assert(!calibration.has_committed);
    assert(memcmp(
        &calibration.committed,
        &original,
        sizeof(original)) == 0);

    assert(!pstvnc_mpeg_calibration_begin(&calibration));
    complete_release_quarantine(&calibration);

    assert(pstvnc_mpeg_calibration_begin(&calibration));
    assert(memcmp(
        &calibration.current,
        &calibration.defaults,
        sizeof(calibration.current)) == 0);
}

static void test_recalibration_starts_from_committed_value(void)
{
    pstvnc_mpeg_calibration_t calibration;
    pstvnc_mpeg_calibration_effects_t effects;
    pstvnc_mpeg_calibration_region_t committed;

    assert(pstvnc_mpeg_calibration_init(&calibration, 704, 462));
    assert(pstvnc_mpeg_calibration_begin(&calibration));

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_MOVE |
            PSTVNC_MPEG_CALIBRATION_RIGHT,
        0,
        &effects);

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        &effects);

    update_ok(&calibration, 0, 0, &effects);

    update_ok(
        &calibration,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT,
        &effects);

    assert(effects.accepted);
    committed = effects.accepted_region;

    complete_release_quarantine(&calibration);

    assert(pstvnc_mpeg_calibration_begin(&calibration));
    assert(memcmp(
        &calibration.current,
        &committed,
        sizeof(committed)) == 0);
}

int main(void)
{
    test_default_and_edit_geometry();
    test_geometry_bounds_and_overflow_rejection();
    test_held_review_control_requires_release_repress();
    test_cancel_does_not_publish_acceptance();
    test_recalibration_starts_from_committed_value();

    puts("MPEG_CALIBRATION_UNIT=PASS");
    return 0;
}
