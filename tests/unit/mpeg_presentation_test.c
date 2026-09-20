/*
 * File synopsis:
 * Host contract tests for the clean Display-owned MPEG start-side visible
 * ownership state: resolved geometry snapshot, caller generation fencing,
 * WAIT_FIRST_FRAME protection, exact first-frame promotion and pending abort.
 */

#include "mpeg_presentation.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static pstvnc_mpeg_presentation_geometry_t valid_geometry(void)
{
    pstvnc_mpeg_presentation_geometry_t geometry;

    memset(&geometry, 0, sizeof(geometry));

    geometry.base.x = 176;
    geometry.base.y = 119;
    geometry.base.width = 352;
    geometry.base.height = 224;

    geometry.inner_content.x = 184;
    geometry.inner_content.y = 123;
    geometry.inner_content.width = 336;
    geometry.inner_content.height = 216;

    geometry.suppression.x = 164;
    geometry.suppression.y = 113;
    geometry.suppression.width = 376;
    geometry.suppression.height = 236;

    return geometry;
}

static void assert_geometry_equal(
    const pstvnc_mpeg_presentation_geometry_t *left,
    const pstvnc_mpeg_presentation_geometry_t *right)
{
    assert(memcmp(left, right, sizeof(*left)) == 0);
}

static void test_initial_rfb_only_state(void)
{
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t snapshot;
    uint32_t run_generation = 0u;

    pstvnc_mpeg_presentation_init(&presentation);

    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_RFB_ONLY);
    assert(pstvnc_mpeg_presentation_mode(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_DIRECT_RFB);
    assert(!pstvnc_mpeg_presentation_requires_global_rfb_protection(
        &presentation));
    assert(!pstvnc_mpeg_presentation_owns_mpeg_visual(&presentation));
    assert(!pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &run_generation));

    assert(pstvnc_mpeg_presentation_state(NULL) ==
        PSTVNC_MPEG_PRESENTATION_STATE_INVALID);
    assert(pstvnc_mpeg_presentation_mode(NULL) ==
        PSTVNC_MPEG_PRESENTATION_MODE_INVALID);
}

static void test_arm_snapshots_caller_generation_and_geometry(void)
{
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();
    pstvnc_mpeg_presentation_geometry_t expected = geometry;
    pstvnc_mpeg_presentation_geometry_t snapshot;
    uint32_t observed_generation = 0u;

    pstvnc_mpeg_presentation_init(&presentation);

    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        41u));

    geometry.base.x = 999;
    geometry.inner_content.x = 999;
    geometry.suppression.x = 999;

    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
    assert(pstvnc_mpeg_presentation_mode(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_FROZEN_RFB_DESKTOP);
    assert(pstvnc_mpeg_presentation_requires_global_rfb_protection(
        &presentation));
    assert(!pstvnc_mpeg_presentation_owns_mpeg_visual(&presentation));

    assert(pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &observed_generation));
    assert(observed_generation == 41u);
    assert_geometry_equal(&snapshot, &expected);

    /* Nested arm must not replace the live exact-run snapshot. */
    geometry = valid_geometry();
    geometry.base.x = 32;
    geometry.inner_content.x = 40;
    geometry.suppression.x = 20;

    assert(!pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        42u));

    assert(pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &observed_generation));
    assert(observed_generation == 41u);
    assert_geometry_equal(&snapshot, &expected);
}

static void test_only_exact_first_frame_promotes(void)
{
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();
    pstvnc_mpeg_presentation_geometry_t snapshot;
    uint32_t observed_generation = 0u;

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        77u));

    assert(!pstvnc_mpeg_presentation_first_frame_presented(
        &presentation,
        0u));
    assert(!pstvnc_mpeg_presentation_first_frame_presented(
        &presentation,
        76u));

    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
    assert(pstvnc_mpeg_presentation_requires_global_rfb_protection(
        &presentation));

    assert(pstvnc_mpeg_presentation_first_frame_presented(
        &presentation,
        77u));

    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_MPEG_OWNED);
    assert(pstvnc_mpeg_presentation_mode(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_COMPOSITED);
    assert(!pstvnc_mpeg_presentation_requires_global_rfb_protection(
        &presentation));
    assert(pstvnc_mpeg_presentation_owns_mpeg_visual(&presentation));

    assert(!pstvnc_mpeg_presentation_first_frame_presented(
        &presentation,
        77u));
    assert(!pstvnc_mpeg_presentation_abort_pending(
        &presentation,
        77u));

    /*
     * There is deliberately no active-stop transition in this P3 owner. The
     * exact active snapshot remains intact for later current-Q7 retirement.
     */
    assert(pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &observed_generation));
    assert(observed_generation == 77u);
    assert_geometry_equal(&snapshot, &geometry);
}

static void test_pending_abort_is_exact_and_releases_snapshot(void)
{
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = valid_geometry();
    pstvnc_mpeg_presentation_geometry_t snapshot;
    uint32_t observed_generation = 0u;

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        100u));

    assert(!pstvnc_mpeg_presentation_abort_pending(
        &presentation,
        99u));
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);

    assert(pstvnc_mpeg_presentation_abort_pending(
        &presentation,
        100u));

    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_RFB_ONLY);
    assert(pstvnc_mpeg_presentation_mode(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_DIRECT_RFB);
    assert(!pstvnc_mpeg_presentation_requires_global_rfb_protection(
        &presentation));
    assert(!pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &observed_generation));

    /*
     * Generation allocation belongs to the caller. A later distinct run may
     * arm cleanly; a stale event from the aborted run cannot promote it.
     */
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        101u));
    assert(!pstvnc_mpeg_presentation_first_frame_presented(
        &presentation,
        100u));
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
    assert(pstvnc_mpeg_presentation_first_frame_presented(
        &presentation,
        101u));
}

static void test_invalid_geometry_and_generation_fail_closed(void)
{
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry;

    pstvnc_mpeg_presentation_init(&presentation);

    geometry = valid_geometry();
    assert(!pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        0u));

    geometry = valid_geometry();
    geometry.base.width = 350;
    assert(!pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        1u));

    geometry = valid_geometry();
    geometry.base.x = -1;
    assert(!pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        1u));

    geometry = valid_geometry();
    geometry.inner_content.x++;
    assert(!pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        1u));

    geometry = valid_geometry();
    geometry.suppression.x = geometry.base.x + 1;
    assert(!pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        1u));

    geometry = valid_geometry();
    geometry.suppression.width = 0;
    assert(!pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        1u));

    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_RFB_ONLY);
    assert(pstvnc_mpeg_presentation_mode(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_DIRECT_RFB);
}

int main(void)
{
    test_initial_rfb_only_state();
    test_arm_snapshots_caller_generation_and_geometry();
    test_only_exact_first_frame_promotes();
    test_pending_abort_is_exact_and_releases_snapshot();
    test_invalid_geometry_and_generation_fail_closed();

    puts("MPEG_PRESENTATION_TEST=PASS");
    return 0;
}
