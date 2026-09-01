#include <stdio.h>
#include <string.h>

#include "framebuffer.h"

static int failures = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static void test_init_and_geometry(void)
{
    uint16_t pixels[12];
    pstvnc_framebuffer_t framebuffer;

    CHECK(!pstvnc_framebuffer_init(NULL, pixels, 12));
    CHECK(!pstvnc_framebuffer_init(&framebuffer, NULL, 12));
    CHECK(!pstvnc_framebuffer_init(&framebuffer, pixels, 0));
    CHECK(pstvnc_framebuffer_init(&framebuffer, pixels, 12));
    CHECK(framebuffer.pixels == pixels);
    CHECK(framebuffer.pixel_capacity == 12);
    CHECK(framebuffer.width == 0);
    CHECK(framebuffer.height == 0);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);

    CHECK(!pstvnc_framebuffer_set_geometry(&framebuffer, 0, 3));
    CHECK(!pstvnc_framebuffer_set_geometry(&framebuffer, 5, 3));
    CHECK(pstvnc_framebuffer_set_geometry(&framebuffer, 4, 3));
    CHECK(framebuffer.width == 4);
    CHECK(framebuffer.height == 3);
    CHECK(pstvnc_framebuffer_pixel_count(&framebuffer) == 12);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
    CHECK(pstvnc_framebuffer_pixel_count(NULL) == 0);
}

static void test_write_and_dirty(void)
{
    uint16_t pixels[20];
    static const uint16_t first[4] = {
        0x1111, 0x2222,
        0x3333, 0x4444
    };
    static const uint16_t second[2] = {
        0xaaaa, 0xbbbb
    };
    pstvnc_framebuffer_t framebuffer;
    pstvnc_framebuffer_rect_t dirty;

    memset(pixels, 0, sizeof(pixels));
    CHECK(pstvnc_framebuffer_init(&framebuffer, pixels, 20));
    CHECK(pstvnc_framebuffer_set_geometry(&framebuffer, 5, 4));

    CHECK(pstvnc_framebuffer_write_rect(
        &framebuffer, 1, 1, 2, 2, first, 2));
    CHECK(pixels[6] == 0x1111);
    CHECK(pixels[7] == 0x2222);
    CHECK(pixels[11] == 0x3333);
    CHECK(pixels[12] == 0x4444);
    CHECK(!framebuffer.valid);
    CHECK(pstvnc_framebuffer_get_dirty(&framebuffer, &dirty));
    CHECK(dirty.x == 1);
    CHECK(dirty.y == 1);
    CHECK(dirty.width == 2);
    CHECK(dirty.height == 2);

    CHECK(pstvnc_framebuffer_write_rect(
        &framebuffer, 3, 0, 2, 1, second, 2));
    CHECK(pixels[3] == 0xaaaa);
    CHECK(pixels[4] == 0xbbbb);
    CHECK(pstvnc_framebuffer_get_dirty(&framebuffer, &dirty));
    CHECK(dirty.x == 1);
    CHECK(dirty.y == 0);
    CHECK(dirty.width == 4);
    CHECK(dirty.height == 3);

    CHECK(!pstvnc_framebuffer_write_rect(
        &framebuffer, 4, 3, 2, 1, second, 2));
    CHECK(!pstvnc_framebuffer_write_rect(
        &framebuffer, 0, 0, 2, 1, second, 1));
    CHECK(!pstvnc_framebuffer_write_rect(
        &framebuffer, 0, 0, 2, 1, NULL, 2));

    pstvnc_framebuffer_clear_dirty(&framebuffer);
    CHECK(!framebuffer.dirty);
    CHECK(!pstvnc_framebuffer_get_dirty(&framebuffer, &dirty));
}

static void test_validity(void)
{
    uint16_t pixels[12];
    pstvnc_framebuffer_t framebuffer;

    CHECK(pstvnc_framebuffer_init(&framebuffer, pixels, 12));
    CHECK(!pstvnc_framebuffer_mark_valid(&framebuffer));
    CHECK(pstvnc_framebuffer_set_geometry(&framebuffer, 4, 3));
    CHECK(pstvnc_framebuffer_mark_valid(&framebuffer));
    CHECK(framebuffer.valid);

    pstvnc_framebuffer_invalidate(&framebuffer);
    CHECK(!framebuffer.valid);

    CHECK(pstvnc_framebuffer_mark_valid(&framebuffer));
    CHECK(pstvnc_framebuffer_set_geometry(&framebuffer, 3, 4));
    CHECK(!framebuffer.valid);
}

int main(void)
{
    test_init_and_geometry();
    test_write_and_dirty();
    test_validity();

    if (failures != 0) {
        fprintf(stderr, "framebuffer_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("framebuffer_test: PASS");
    return 0;
}
