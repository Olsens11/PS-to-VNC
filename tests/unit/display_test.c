#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "display.h"

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static void prepare_valid_framebuffer(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t *pixels)
{
    CHECK(pstvnc_framebuffer_init(
        framebuffer,
        pixels,
        PSTVNC_DISPLAY_PIXEL_COUNT));
    CHECK(pstvnc_framebuffer_set_geometry(
        framebuffer,
        PSTVNC_DISPLAY_WIDTH,
        PSTVNC_DISPLAY_HEIGHT));
    CHECK(pstvnc_framebuffer_mark_valid(framebuffer));
}

static void test_exact_conversion(void)
{
    static uint16_t remote[PSTVNC_DISPLAY_PIXEL_COUNT];
    static uint16_t gs[PSTVNC_DISPLAY_PIXEL_COUNT];
    pstvnc_framebuffer_t framebuffer;

    memset(remote, 0, sizeof(remote));
    memset(gs, 0, sizeof(gs));

    remote[0] = 0x0000;
    remote[1] = 0x001f;
    remote[2] = 0x03e0;
    remote[3] = 0x7c00;
    remote[PSTVNC_DISPLAY_PIXEL_COUNT - 1u] = 0x7fff;

    prepare_valid_framebuffer(&framebuffer, remote);

    CHECK(pstvnc_display_prepare_gs16(
        &framebuffer,
        gs,
        PSTVNC_DISPLAY_PIXEL_COUNT));

    CHECK(gs[0] == 0x8000);
    CHECK(gs[1] == 0x801f);
    CHECK(gs[2] == 0x83e0);
    CHECK(gs[3] == 0xfc00);
    CHECK(gs[PSTVNC_DISPLAY_PIXEL_COUNT - 1u] == 0xffff);

    CHECK(remote[0] == 0x0000);
    CHECK(remote[3] == 0x7c00);
}

static void test_requires_authoritative_target_geometry(void)
{
    static uint16_t remote[PSTVNC_DISPLAY_PIXEL_COUNT];
    static uint16_t gs[PSTVNC_DISPLAY_PIXEL_COUNT];
    pstvnc_framebuffer_t framebuffer;

    CHECK(pstvnc_framebuffer_init(
        &framebuffer,
        remote,
        PSTVNC_DISPLAY_PIXEL_COUNT));
    CHECK(pstvnc_framebuffer_set_geometry(
        &framebuffer,
        PSTVNC_DISPLAY_WIDTH,
        PSTVNC_DISPLAY_HEIGHT));

    CHECK(!pstvnc_display_prepare_gs16(
        &framebuffer,
        gs,
        PSTVNC_DISPLAY_PIXEL_COUNT));

    CHECK(pstvnc_framebuffer_mark_valid(&framebuffer));
    framebuffer.width = PSTVNC_DISPLAY_WIDTH - 1u;

    CHECK(!pstvnc_display_prepare_gs16(
        &framebuffer,
        gs,
        PSTVNC_DISPLAY_PIXEL_COUNT));
}

static void test_capacity_and_null_rejection(void)
{
    static uint16_t remote[PSTVNC_DISPLAY_PIXEL_COUNT];
    static uint16_t gs[PSTVNC_DISPLAY_PIXEL_COUNT];
    pstvnc_framebuffer_t framebuffer;

    prepare_valid_framebuffer(&framebuffer, remote);

    CHECK(!pstvnc_display_prepare_gs16(
        &framebuffer,
        gs,
        PSTVNC_DISPLAY_PIXEL_COUNT - 1u));
    CHECK(!pstvnc_display_prepare_gs16(
        NULL,
        gs,
        PSTVNC_DISPLAY_PIXEL_COUNT));
    CHECK(!pstvnc_display_prepare_gs16(
        &framebuffer,
        NULL,
        PSTVNC_DISPLAY_PIXEL_COUNT));
}

int main(void)
{
    test_exact_conversion();
    test_requires_authoritative_target_geometry();
    test_capacity_and_null_rejection();

    if (failures != 0) {
        fprintf(stderr, "display_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("display_test: PASS");
    return 0;
}
