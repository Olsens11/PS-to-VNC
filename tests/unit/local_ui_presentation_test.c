#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "local_ui.h"
#include "local_ui_presentation.h"
#include "osk.h"
#include "osk_render.h"

static uint16_t overlay_pixels[
    PSTVNC_OSK_SURFACE_PIXEL_COUNT];

static void test_desktop_has_no_local_overlay(void)
{
    pstvnc_local_ui_t ui;
    pstvnc_local_ui_presentation_t presentation;

    pstvnc_local_ui_init(&ui);

    memset(
        overlay_pixels,
        0xa5,
        sizeof(overlay_pixels));

    assert(
        pstvnc_local_ui_prepare_presentation(
            &ui,
            NULL,
            NULL,
            0u,
            &presentation));

    assert(!presentation.overlay_visible);
    assert(presentation.overlay_pixels == NULL);
    assert(presentation.overlay_pixel_count == 0u);
    assert(presentation.overlay_width == 0u);
    assert(presentation.overlay_height == 0u);
    assert(presentation.overlay_x == 0u);
    assert(presentation.overlay_y == 0u);
    assert(presentation.generation == 0u);
}

static void test_historical_osk_fixed_mode_placement(void)
{
    pstvnc_local_ui_t ui;
    pstvnc_osk_t osk;
    pstvnc_local_ui_presentation_t presentation;

    pstvnc_local_ui_init(&ui);
    pstvnc_osk_reset_for_open(&osk);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    assert(
        pstvnc_local_ui_prepare_presentation(
            &ui,
            &osk,
            overlay_pixels,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT,
            &presentation));

    assert(presentation.overlay_visible);

    assert(
        presentation.overlay_pixels ==
        overlay_pixels);

    assert(
        presentation.overlay_pixel_count ==
        PSTVNC_OSK_SURFACE_PIXEL_COUNT);

    assert(
        presentation.overlay_width ==
        600u);

    assert(
        presentation.overlay_height ==
        178u);

    /*
     * Historical normal keyboard in the current fixed 704x462 mode:
     *
     *   x = (704 - 600) / 2 = 52
     *   y = 462 - 178 - 6 = 278
     */
    assert(
        presentation.overlay_x ==
        52u);

    assert(
        presentation.overlay_y ==
        278u);

    assert(
        presentation.generation ==
        pstvnc_local_ui_generation(
            &ui));

    assert(
        pstvnc_local_ui_needs_present(
            &ui));
}

static void test_new_local_generation_is_not_silently_acknowledged(void)
{
    pstvnc_local_ui_t ui;
    pstvnc_osk_t osk;
    pstvnc_local_ui_presentation_t presentation;
    uint32_t first_generation;

    pstvnc_local_ui_init(&ui);
    pstvnc_osk_reset_for_open(&osk);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    first_generation =
        pstvnc_local_ui_generation(
            &ui);

    assert(
        pstvnc_local_ui_prepare_presentation(
            &ui,
            &osk,
            overlay_pixels,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT,
            &presentation));

    assert(
        presentation.generation ==
        first_generation);

    /*
     * Preparation is not presentation acknowledgement.
     */
    assert(
        pstvnc_local_ui_needs_present(
            &ui));

    osk.shift = 1;

    assert(
        pstvnc_local_ui_mark_local_change(
            &ui));

    assert(
        pstvnc_local_ui_generation(
            &ui) !=
        first_generation);

    assert(
        pstvnc_local_ui_prepare_presentation(
            &ui,
            &osk,
            overlay_pixels,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT,
            &presentation));

    assert(
        presentation.generation ==
        pstvnc_local_ui_generation(
            &ui));

    assert(
        pstvnc_local_ui_needs_present(
            &ui));
}

static void test_close_describes_overlay_removal(void)
{
    pstvnc_local_ui_t ui;
    pstvnc_osk_t osk;
    pstvnc_local_ui_presentation_t presentation;

    pstvnc_local_ui_init(&ui);
    pstvnc_osk_reset_for_open(&osk);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    assert(
        pstvnc_local_ui_close_osk(
            &ui));

    assert(
        pstvnc_local_ui_needs_present(
            &ui));

    assert(
        pstvnc_local_ui_prepare_presentation(
            &ui,
            NULL,
            NULL,
            0u,
            &presentation));

    assert(!presentation.overlay_visible);

    assert(
        presentation.generation ==
        pstvnc_local_ui_generation(
            &ui));
}

static void test_invalid_contract(void)
{
    pstvnc_local_ui_t ui;
    pstvnc_osk_t osk;
    pstvnc_local_ui_presentation_t presentation;

    pstvnc_local_ui_init(&ui);
    pstvnc_osk_reset_for_open(&osk);

    assert(
        !pstvnc_local_ui_prepare_presentation(
            NULL,
            &osk,
            overlay_pixels,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT,
            &presentation));

    assert(
        !pstvnc_local_ui_prepare_presentation(
            &ui,
            &osk,
            overlay_pixels,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT,
            NULL));

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    assert(
        !pstvnc_local_ui_prepare_presentation(
            &ui,
            NULL,
            overlay_pixels,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT,
            &presentation));

    assert(
        !pstvnc_local_ui_prepare_presentation(
            &ui,
            &osk,
            NULL,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT,
            &presentation));

    assert(
        !pstvnc_local_ui_prepare_presentation(
            &ui,
            &osk,
            overlay_pixels,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT - 1u,
            &presentation));
}

int main(void)
{
    test_desktop_has_no_local_overlay();
    test_historical_osk_fixed_mode_placement();
    test_new_local_generation_is_not_silently_acknowledged();
    test_close_describes_overlay_removal();
    test_invalid_contract();

    puts("LOCAL_UI_PRESENTATION_TEST=PASS");
    return 0;
}
