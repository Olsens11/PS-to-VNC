#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "osk.h"
#include "osk_render.h"

#define EXPECTED_DEFAULT_FNV64 \
    UINT64_C(0x2ccd6dca0a40ea6a)

#define EXPECTED_SHIFT_CTRL_ALT_FNV64 \
    UINT64_C(0xf732b31ed8dac4ee)

#define EXPECTED_FUNC_CTRL_ALT_FNV64 \
    UINT64_C(0xc52903eff4da984e)

static uint16_t render_surface[
    PSTVNC_OSK_SURFACE_PIXEL_COUNT];

static uint16_t expected_surface_color(
    unsigned int red,
    unsigned int green,
    unsigned int blue)
{
    return (uint16_t)(
        0x8000u |
        ((blue & 0x1fu) << 10) |
        ((green & 0x1fu) << 5) |
        (red & 0x1fu));
}

static uint64_t calculate_surface_fnv64(
    const uint16_t *pixels,
    size_t pixel_count)
{
    uint64_t value =
        UINT64_C(0xcbf29ce484222325);

    const uint64_t prime =
        UINT64_C(0x100000001b3);

    size_t pixel_index;

    for (pixel_index = 0;
         pixel_index < pixel_count;
         pixel_index++) {

        value ^=
            (uint64_t)(
                pixels[pixel_index] &
                0xffu);

        value *= prime;

        value ^=
            (uint64_t)(
                (
                    pixels[pixel_index] >>
                    8
                ) &
                0xffu);

        value *= prime;
    }

    return value;
}

static void test_default_visual_identity(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);

    memset(
        render_surface,
        0,
        sizeof(render_surface));

    assert(
        pstvnc_osk_render_surface(
            &osk,
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT));

    assert(
        calculate_surface_fnv64(
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT) ==
        EXPECTED_DEFAULT_FNV64);

    /*
     * Default selected key: ABC row 1 column 0.
     * Historical geometry places its fill at x=9, y=37.
     */
    assert(
        render_surface[
            37u * PSTVNC_OSK_WIDTH +
            9u] ==
        expected_surface_color(
            31u, 24u, 3u));

    /*
     * Border surrounding that selected key.
     */
    assert(
        render_surface[
            36u * PSTVNC_OSK_WIDTH +
            8u] ==
        expected_surface_color(
            22u, 22u, 22u));

    /*
     * ABC page indicator remains active on the utility row.
     */
    assert(
        render_surface[
            136u * PSTVNC_OSK_WIDTH +
            10u] ==
        expected_surface_color(
            6u, 20u, 8u));

    assert(
        render_surface[0] ==
        expected_surface_color(
            3u, 3u, 4u));
}

static void test_modifier_visual_identity(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);

    osk.row = 0u;
    osk.col = 0u;
    osk.shift = 1;
    osk.ctrl = 1;
    osk.alt = 1;

    assert(
        pstvnc_osk_render_surface(
            &osk,
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT));

    assert(
        calculate_surface_fnv64(
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT) ==
        EXPECTED_SHIFT_CTRL_ALT_FNV64);
}

static void test_function_page_visual_identity(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);

    osk.page =
        PSTVNC_OSK_PAGE_FUNC;

    osk.row = 2u;
    osk.col = 1u;
    osk.ctrl = 1;
    osk.alt = 1;

    assert(
        pstvnc_osk_render_surface(
            &osk,
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT));

    assert(
        calculate_surface_fnv64(
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT) ==
        EXPECTED_FUNC_CTRL_ALT_FNV64);
}

static void test_invalid_render_contract(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);

    assert(
        !pstvnc_osk_render_surface(
            NULL,
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT));

    assert(
        !pstvnc_osk_render_surface(
            &osk,
            NULL,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT));

    assert(
        !pstvnc_osk_render_surface(
            &osk,
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT - 1u));

    osk.page =
        PSTVNC_OSK_PAGE_COUNT;

    assert(
        !pstvnc_osk_render_surface(
            &osk,
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT));

    pstvnc_osk_reset_for_open(&osk);
    osk.row = PSTVNC_OSK_ROWS;

    assert(
        !pstvnc_osk_render_surface(
            &osk,
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT));

    pstvnc_osk_reset_for_open(&osk);

    osk.col =
        pstvnc_osk_row_length(
            osk.page,
            osk.row);

    assert(
        !pstvnc_osk_render_surface(
            &osk,
            render_surface,
            PSTVNC_OSK_SURFACE_PIXEL_COUNT));
}

int main(void)
{
    test_default_visual_identity();
    test_modifier_visual_identity();
    test_function_page_visual_identity();
    test_invalid_render_contract();

    puts("OSK_RENDER_TEST=PASS");
    return 0;
}
