#ifndef PSTVNC_FRAMEBUFFER_H
#define PSTVNC_FRAMEBUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct pstvnc_framebuffer_rect {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} pstvnc_framebuffer_rect_t;

typedef struct pstvnc_framebuffer {
    uint16_t *pixels;
    size_t pixel_capacity;
    uint16_t width;
    uint16_t height;
    int valid;
    int dirty;
    pstvnc_framebuffer_rect_t dirty_rect;
} pstvnc_framebuffer_t;

int pstvnc_framebuffer_init(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t *pixels,
    size_t pixel_capacity);

int pstvnc_framebuffer_set_geometry(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t width,
    uint16_t height);

size_t pstvnc_framebuffer_pixel_count(
    const pstvnc_framebuffer_t *framebuffer);

void pstvnc_framebuffer_invalidate(
    pstvnc_framebuffer_t *framebuffer);

int pstvnc_framebuffer_mark_valid(
    pstvnc_framebuffer_t *framebuffer);

int pstvnc_framebuffer_write_rect(
    pstvnc_framebuffer_t *framebuffer,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    const uint16_t *source,
    size_t source_stride);

void pstvnc_framebuffer_clear_dirty(
    pstvnc_framebuffer_t *framebuffer);

int pstvnc_framebuffer_get_dirty(
    const pstvnc_framebuffer_t *framebuffer,
    pstvnc_framebuffer_rect_t *rect);

#endif
