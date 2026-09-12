/*
 * File synopsis:
 * Strict host contract tests for the explicit CP2P committed-region -> MPEG
 * start handoff and its first-frame ownership lifecycle.
 */
#include "h1_mpeg_start_handoff.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static pstvnc_mpeg_cal_region_t region_with_mattes(void)
{
    pstvnc_mpeg_cal_region_t region;

    memset(&region, 0, sizeof(region));
    region.x = 176;
    region.y = 119;
    region.width = 352;
    region.height = 224;
    region.inner_matte_x = 8;
    region.inner_matte_y = 4;
    region.outer_matte_x = 12;
    region.outer_matte_y = 6;
    return region;
}

static void test_prepare_preserves_base_geometry_and_separates_mattes(void)
{
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_start_contract_t contract;
    pstvnc_mpeg_cal_region_t region = region_with_mattes();
    const pstvnc_mpeg_cal_rect_t *suppression;

    pstvnc_h1_mpeg_start_handoff_init(&handoff, 704, 462);

    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &region, &contract));

    /* Locked product rule: neither matte changes capture/base MPEG geometry. */
    assert(contract.draw_x == 176);
    assert(contract.draw_y == 119);
    assert(contract.draw_width == 352);
    assert(contract.draw_height == 224);
    assert(contract.inner_matte_x == 8);
    assert(contract.inner_matte_y == 4);

    assert(contract.suppression_rect.x == 164);
    assert(contract.suppression_rect.y == 113);
    assert(contract.suppression_rect.width == 376);
    assert(contract.suppression_rect.height == 236);
    assert(contract.generation != 0u);

    assert(!pstvnc_h1_mpeg_start_handoff_allows_rfb_request(&handoff));
    assert(pstvnc_h1_mpeg_start_handoff_remote_present_mode(&handoff) ==
        PSTVNC_H1_MPEG_REMOTE_PRESENT_FROZEN_DESKTOP);
    assert(!pstvnc_h1_mpeg_start_handoff_take_full_refresh(&handoff));

    suppression = pstvnc_h1_mpeg_start_handoff_suppression_rect(&handoff);
    assert(suppression != NULL);
    assert(memcmp(
        suppression,
        &contract.suppression_rect,
        sizeof(*suppression)) == 0);
}

static void test_first_frame_and_stop_define_visible_ownership_boundaries(void)
{
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_start_contract_t contract;
    pstvnc_mpeg_cal_region_t region = region_with_mattes();

    pstvnc_h1_mpeg_start_handoff_init(&handoff, 704, 462);
    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &region, &contract));

    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(
        &handoff, contract.generation));
    assert(pstvnc_h1_mpeg_start_handoff_allows_rfb_request(&handoff));
    assert(pstvnc_h1_mpeg_start_handoff_remote_present_mode(&handoff) ==
        PSTVNC_H1_MPEG_REMOTE_PRESENT_COMPOSITED);
    assert(pstvnc_h1_mpeg_start_handoff_take_full_refresh(&handoff));
    assert(!pstvnc_h1_mpeg_start_handoff_take_full_refresh(&handoff));

    assert(pstvnc_h1_mpeg_start_handoff_stop(
        &handoff, contract.generation));
    assert(pstvnc_h1_mpeg_start_handoff_allows_rfb_request(&handoff));
    assert(pstvnc_h1_mpeg_start_handoff_remote_present_mode(&handoff) ==
        PSTVNC_H1_MPEG_REMOTE_PRESENT_DIRECT_RFB);
    assert(pstvnc_h1_mpeg_start_handoff_take_full_refresh(&handoff));
}

static void test_abort_retains_external_committed_geometry_for_retry(void)
{
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_start_contract_t first;
    pstvnc_h1_mpeg_start_contract_t second;
    pstvnc_mpeg_cal_region_t committed = region_with_mattes();

    pstvnc_h1_mpeg_start_handoff_init(&handoff, 704, 462);
    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &committed, &first));
    assert(pstvnc_h1_mpeg_start_handoff_abort_start(
        &handoff, first.generation));
    assert(pstvnc_h1_mpeg_start_handoff_take_full_refresh(&handoff));

    /* Committed calibration is external durable runtime data, so retry is legal. */
    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &committed, &second));
    assert(second.generation != 0u);
    assert(second.generation != first.generation);

    /* A late callback from the aborted start cannot seize presentation. */
    assert(!pstvnc_h1_mpeg_start_handoff_first_frame_presented(
        &handoff, first.generation));
    assert(pstvnc_h1_mpeg_start_handoff_remote_present_mode(&handoff) ==
        PSTVNC_H1_MPEG_REMOTE_PRESENT_FROZEN_DESKTOP);
}

static void test_invalid_capture_geometry_is_rejected(void)
{
    pstvnc_h1_mpeg_start_handoff_t handoff;
    pstvnc_h1_mpeg_start_contract_t contract;
    pstvnc_mpeg_cal_region_t region = region_with_mattes();

    pstvnc_h1_mpeg_start_handoff_init(&handoff, 704, 462);

    region.width = 350;
    assert(!pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &region, &contract));

    region = region_with_mattes();
    region.x = 400;
    assert(!pstvnc_h1_mpeg_start_handoff_prepare_start(
        &handoff, &region, &contract));
}

int main(void)
{
    test_prepare_preserves_base_geometry_and_separates_mattes();
    test_first_frame_and_stop_define_visible_ownership_boundaries();
    test_abort_retains_external_committed_geometry_for_retry();
    test_invalid_capture_geometry_is_rejected();

    puts("H1_MPEG_START_HANDOFF_HOST_TEST=PASS");
    return 0;
}
