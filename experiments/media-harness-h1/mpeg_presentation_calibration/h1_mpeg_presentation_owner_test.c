/*
 * File synopsis:
 * Strict host contract for accepted calibration -> first MPEG frame -> shared
 * presentation ownership and return-to-RFB lifecycle.
 */
#include "h1_mpeg_presentation_owner.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static pstvnc_mpeg_cal_region_t region(void)
{
    pstvnc_mpeg_cal_region_t value;
    memset(&value, 0, sizeof(value));
    value.x = 176;
    value.y = 119;
    value.width = 352;
    value.height = 224;
    value.inner_matte_x = 4;
    value.inner_matte_y = 3;
    value.outer_matte_x = 6;
    value.outer_matte_y = 7;
    return value;
}

static pstvnc_mpeg_cal_rect_t suppression(void)
{
    pstvnc_mpeg_cal_rect_t value;
    value.x = 170;
    value.y = 112;
    value.width = 364;
    value.height = 238;
    return value;
}

static void test_initial_rfb_only_contract(void)
{
    pstvnc_h1_mpeg_presentation_owner_t owner;

    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    assert(pstvnc_h1_mpeg_presentation_owner_state(&owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(pstvnc_h1_mpeg_presentation_owner_allows_rfb_request(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_remote_present_mode(&owner) ==
        PSTVNC_H1_MPEG_REMOTE_PRESENT_DIRECT_RFB);
    assert(!pstvnc_h1_mpeg_presentation_owner_has_mpeg(&owner));
    assert(!pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_region(&owner) == NULL);
    assert(pstvnc_h1_mpeg_presentation_owner_suppression_rect(&owner) == NULL);
}

static void test_arm_freezes_until_exact_first_frame_generation(void)
{
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_mpeg_cal_region_t r = region();
    pstvnc_mpeg_cal_rect_t s = suppression();
    uint32_t generation = 0;

    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    assert(pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &r, &s, &generation));
    assert(generation != 0u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(&owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
    assert(!pstvnc_h1_mpeg_presentation_owner_allows_rfb_request(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_remote_present_mode(&owner) ==
        PSTVNC_H1_MPEG_REMOTE_PRESENT_FROZEN_DESKTOP);
    assert(!pstvnc_h1_mpeg_presentation_owner_has_mpeg(&owner));
    assert(!pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_region(&owner) != NULL);
    assert(pstvnc_h1_mpeg_presentation_owner_suppression_rect(&owner) != NULL);

    assert(!pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
        &owner, generation + 1u));
    assert(pstvnc_h1_mpeg_presentation_owner_state(&owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME);

    assert(pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
        &owner, generation));
    assert(pstvnc_h1_mpeg_presentation_owner_state(&owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED);
    assert(pstvnc_h1_mpeg_presentation_owner_allows_rfb_request(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_remote_present_mode(&owner) ==
        PSTVNC_H1_MPEG_REMOTE_PRESENT_COMPOSITED);
    assert(pstvnc_h1_mpeg_presentation_owner_has_mpeg(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&owner));
    assert(!pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&owner));
}

static void test_abort_pending_returns_full_rfb(void)
{
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_mpeg_cal_region_t r = region();
    pstvnc_mpeg_cal_rect_t s = suppression();
    uint32_t generation = 0;

    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    assert(pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &r, &s, &generation));
    assert(pstvnc_h1_mpeg_presentation_owner_abort_start(
        &owner, generation));
    assert(pstvnc_h1_mpeg_presentation_owner_state(&owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(pstvnc_h1_mpeg_presentation_owner_allows_rfb_request(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_remote_present_mode(&owner) ==
        PSTVNC_H1_MPEG_REMOTE_PRESENT_DIRECT_RFB);
    assert(!pstvnc_h1_mpeg_presentation_owner_has_mpeg(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_region(&owner) == NULL);
}

static void test_stop_active_returns_full_rfb(void)
{
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_mpeg_cal_region_t r = region();
    pstvnc_mpeg_cal_rect_t s = suppression();
    uint32_t generation = 0;

    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    assert(pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &r, &s, &generation));
    assert(pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
        &owner, generation));
    assert(pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&owner));
    assert(pstvnc_h1_mpeg_presentation_owner_stop(&owner, generation));
    assert(pstvnc_h1_mpeg_presentation_owner_state(&owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&owner));
}

static void test_invalid_or_nested_arm_is_rejected(void)
{
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_mpeg_cal_region_t r = region();
    pstvnc_mpeg_cal_rect_t s = suppression();
    pstvnc_mpeg_cal_rect_t too_small = s;
    uint32_t generation = 0;
    uint32_t second_generation = 0;

    too_small.x = r.x + 1;

    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    assert(!pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &r, &too_small, &generation));
    assert(pstvnc_h1_mpeg_presentation_owner_state(&owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);

    assert(pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &r, &s, &generation));
    assert(!pstvnc_h1_mpeg_presentation_owner_arm(
        &owner, &r, &s, &second_generation));
    assert(second_generation == 0u);
}

static void test_stale_generation_cannot_control_later_attempt(void)
{
    pstvnc_h1_mpeg_presentation_owner_t owner;
    pstvnc_mpeg_cal_region_t r = region();
    pstvnc_mpeg_cal_rect_t s = suppression();
    uint32_t first = 0;
    uint32_t second = 0;

    pstvnc_h1_mpeg_presentation_owner_init(&owner);
    assert(pstvnc_h1_mpeg_presentation_owner_arm(&owner, &r, &s, &first));
    assert(pstvnc_h1_mpeg_presentation_owner_abort_start(&owner, first));
    assert(pstvnc_h1_mpeg_presentation_owner_take_full_refresh(&owner));

    assert(pstvnc_h1_mpeg_presentation_owner_arm(&owner, &r, &s, &second));
    assert(second != first);
    assert(!pstvnc_h1_mpeg_presentation_owner_first_frame_presented(
        &owner, first));
    assert(!pstvnc_h1_mpeg_presentation_owner_abort_start(&owner, first));
    assert(pstvnc_h1_mpeg_presentation_owner_state(&owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
    assert(pstvnc_h1_mpeg_presentation_owner_abort_start(&owner, second));
}

int main(void)
{
    test_initial_rfb_only_contract();
    test_arm_freezes_until_exact_first_frame_generation();
    test_abort_pending_returns_full_rfb();
    test_stop_active_returns_full_rfb();
    test_invalid_or_nested_arm_is_rejected();
    test_stale_generation_cannot_control_later_attempt();

    puts("MPEG_PRESENTATION_OWNER_HOST_TEST=PASS");
    return 0;
}
