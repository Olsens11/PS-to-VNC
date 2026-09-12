/*
 * File synopsis:
 * Strict host contract test for the experiment-local native MPEG-calibration
 * CT16 rasterizer.
 */
#include "h1_mpeg_calibration_raster.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST_WIDTH 704
#define TEST_HEIGHT 462
#define TEST_PIXELS ((size_t)TEST_WIDTH * (size_t)TEST_HEIGHT)

static uint16_t frozen[TEST_PIXELS];
static uint16_t surface[TEST_PIXELS];

static size_t pixel_at(int x, int y)
{
    return (size_t)y * (size_t)TEST_WIDTH + (size_t)x;
}

int main(void)
{
    pstvnc_h1_mpeg_calibration_render_plan_t plan;
    size_t i;

    for (i = 0; i < TEST_PIXELS; i++)
        frozen[i] = (uint16_t)(0x8000u | (i & 0x7fffu));

    memset(&plan, 0, sizeof(plan));
    plan.kind = PSTVNC_H1_MPEG_CAL_RENDER_EDIT;
    plan.region.x = 176;
    plan.region.y = 119;
    plan.region.width = 352;
    plan.region.height = 224;
    plan.region.inner_matte_x = 8;
    plan.region.inner_matte_y = 4;
    plan.region.outer_matte_x = 12;
    plan.region.outer_matte_y = 10;
    plan.suppression_rect.x = 164;
    plan.suppression_rect.y = 109;
    plan.suppression_rect.width = 376;
    plan.suppression_rect.height = 244;

    assert(pstvnc_h1_mpeg_calibration_rasterize(
        &plan, frozen, TEST_PIXELS, TEST_WIDTH, TEST_HEIGHT,
        surface, TEST_PIXELS));
    assert(surface[pixel_at(0, 0)] == frozen[pixel_at(0, 0)]);
    assert(surface[pixel_at(176, 119)] == PSTVNC_H1_MPEG_CAL_COLOR_ORANGE);
    assert(surface[pixel_at(187, 119)] == PSTVNC_H1_MPEG_CAL_COLOR_TEAL);
    assert(surface[pixel_at(180, 130)] == PSTVNC_H1_MPEG_CAL_COLOR_BLACK);
    assert(surface[pixel_at(165, 110)] != frozen[pixel_at(165, 110)]);
    assert(surface[pixel_at(250, 200)] == frozen[pixel_at(250, 200)]);

    plan.kind = PSTVNC_H1_MPEG_CAL_RENDER_CONTROLS;
    assert(pstvnc_h1_mpeg_calibration_rasterize(
        &plan, frozen, TEST_PIXELS, TEST_WIDTH, TEST_HEIGHT,
        surface, TEST_PIXELS));
    assert(surface[pixel_at(100, 100)] != frozen[pixel_at(100, 100)]);

    plan.kind = PSTVNC_H1_MPEG_CAL_RENDER_REVIEW;
    assert(pstvnc_h1_mpeg_calibration_rasterize(
        &plan, frozen, TEST_PIXELS, TEST_WIDTH, TEST_HEIGHT,
        surface, TEST_PIXELS));
    assert(surface[pixel_at(100, 100)] != frozen[pixel_at(100, 100)]);

    puts("MPEG_CALIBRATION_RASTER_HOST_TEST=PASS");
    return 0;
}
