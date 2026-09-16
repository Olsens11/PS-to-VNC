#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rfb_session.h"

static int failures = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static unsigned char input[4096];
static size_t input_size;
static size_t input_pos;

static void script_reset(void)
{
    memset(input, 0, sizeof(input));
    input_size = 0;
    input_pos = 0;
}

static void append_input(
    const void *bytes,
    size_t count)
{
    CHECK(input_size + count <= sizeof(input));
    if (input_size + count > sizeof(input))
        return;

    memcpy(&input[input_size], bytes, count);
    input_size += count;
}

int pstvnc_rfb_bridge_read_exact(
    void *buffer,
    size_t count)
{
    if (input_pos + count > input_size)
        return -1;

    memcpy(buffer, &input[input_pos], count);
    input_pos += count;
    return 0;
}

int pstvnc_rfb_bridge_poll_receive(void)
{
    return input_pos < input_size ? 1 : 0;
}

int pstvnc_rfb_bridge_write_exact(
    const void *buffer,
    size_t count)
{
    (void)buffer;
    (void)count;
    return 0;
}

int pstvnc_rfb_bridge_quiesce_requested(void)
{
    return 0;
}

int pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(void)
{
    return 0;
}

static void append_update_header(uint16_t rectangle_count)
{
    unsigned char update[4] = {
        0, 0,
        (unsigned char)(rectangle_count >> 8),
        (unsigned char)rectangle_count
    };

    append_input(update, sizeof(update));
}

static void append_rectangle(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint32_t encoding,
    const uint16_t *pixels)
{
    unsigned char header[12];
    uint16_t row;
    uint16_t column;

    memset(header, 0, sizeof(header));
    header[0] = (unsigned char)(x >> 8);
    header[1] = (unsigned char)x;
    header[2] = (unsigned char)(y >> 8);
    header[3] = (unsigned char)y;
    header[4] = (unsigned char)(width >> 8);
    header[5] = (unsigned char)width;
    header[6] = (unsigned char)(height >> 8);
    header[7] = (unsigned char)height;
    header[8] = (unsigned char)(encoding >> 24);
    header[9] = (unsigned char)(encoding >> 16);
    header[10] = (unsigned char)(encoding >> 8);
    header[11] = (unsigned char)encoding;

    append_input(header, sizeof(header));

    if (encoding != 0)
        return;

    for (row = 0; row < height; row++) {
        for (column = 0; column < width; column++) {
            uint16_t value = pixels[(size_t)row * width + column];
            unsigned char wire[2] = {
                (unsigned char)value,
                (unsigned char)(value >> 8)
            };

            append_input(wire, sizeof(wire));
        }
    }
}

static void prepare(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer,
    uint16_t *pixels)
{
    pstvnc_rfb_session_init(session);
    session->state = PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME;
    session->server_init.width = 4;
    session->server_init.height = 3;

    CHECK(pstvnc_framebuffer_init(framebuffer, pixels, 12));
    CHECK(pstvnc_framebuffer_set_geometry(framebuffer, 4, 3));
}

static void prepare_live(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer,
    uint16_t *pixels)
{
    prepare(session, framebuffer, pixels);
    CHECK(pstvnc_framebuffer_mark_valid(framebuffer));
    session->state = PSTVNC_RFB_SESSION_READY;
}

static void test_success_with_async_messages(void)
{
    static const unsigned char bell = 2;
    static const unsigned char cut_text[] = {
        3,
        0, 0, 0,
        0, 0, 0, 3,
        'a', 'b', 'c'
    };
    static const unsigned char color_map[] = {
        1,
        0,
        0, 0,
        0, 1,
        0, 0, 0, 0, 0, 0
    };
    static const uint16_t source[12] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;
    pstvnc_framebuffer_rect_t dirty;

    script_reset();
    prepare(&session, &framebuffer, pixels);

    append_input(&bell, 1);
    append_input(cut_text, sizeof(cut_text));
    append_input(color_map, sizeof(color_map));
    append_update_header(1);
    append_rectangle(0, 0, 4, 3, 0, source);

    CHECK(pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(framebuffer.valid);
    CHECK(memcmp(pixels, source, sizeof(source)) == 0);
    CHECK(pstvnc_framebuffer_get_dirty(&framebuffer, &dirty));
    CHECK(dirty.x == 0);
    CHECK(dirty.y == 0);
    CHECK(dirty.width == 4);
    CHECK(dirty.height == 3);
    CHECK(input_pos == input_size);
}

static void test_multiple_rectangles(void)
{
    static const uint16_t top[8] = {
        1, 2, 3, 4,
        5, 6, 7, 8
    };
    static const uint16_t bottom[4] = {
        9, 10, 11, 12
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare(&session, &framebuffer, pixels);

    append_update_header(2);
    append_rectangle(0, 0, 4, 2, 0, top);
    append_rectangle(0, 2, 4, 1, 0, bottom);

    CHECK(pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(framebuffer.valid);
    CHECK(pixels[0] == 1);
    CHECK(pixels[11] == 12);
}

static void test_partial_full_frame_rejected(void)
{
    static const uint16_t half[6] = {
        1, 2,
        3, 4,
        5, 6
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare(&session, &framebuffer, pixels);

    append_update_header(1);
    append_rectangle(0, 0, 2, 3, 0, half);

    CHECK(!pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_SIZE);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

static void test_protocol_failures(void)
{
    static const uint16_t one_pixel[2] = { 1, 2 };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare(&session, &framebuffer, pixels);
    append_update_header(0);
    CHECK(!pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_EMPTY_UPDATE);

    script_reset();
    prepare(&session, &framebuffer, pixels);
    append_update_header(1);
    append_rectangle(0, 0, 1, 1, 5, one_pixel);
    CHECK(!pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_ENCODING);

    script_reset();
    prepare(&session, &framebuffer, pixels);
    append_update_header(1);
    append_rectangle(3, 2, 2, 1, 0, one_pixel);
    CHECK(!pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_RECTANGLE_BOUNDS);

    script_reset();
    prepare(&session, &framebuffer, pixels);
    {
        unsigned char unsupported = 99;
        append_input(&unsupported, 1);
    }
    CHECK(!pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(session.error ==
        PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_SERVER_MESSAGE);
}

static void test_short_pixel_payload_invalidates(void)
{
    static const unsigned char rectangle_header[12] = {
        0, 0,
        0, 0,
        0, 4,
        0, 3,
        0, 0, 0, 0
    };
    static const unsigned char one_pixel[2] = { 1, 0 };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare(&session, &framebuffer, pixels);

    append_update_header(1);
    append_input(rectangle_header, sizeof(rectangle_header));
    append_input(one_pixel, sizeof(one_pixel));

    CHECK(!pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_IO);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

static void test_live_partial_update(void)
{
    static const uint16_t changed[2] = { 0x1234, 0x5678 };
    uint16_t pixels[12];
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;
    pstvnc_framebuffer_rect_t dirty;
    unsigned int i;

    for (i = 0; i < 12; i++)
        pixels[i] = 0x1111;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);

    append_update_header(1);
    append_rectangle(1, 1, 2, 1, 0, changed);

    CHECK(pstvnc_rfb_session_receive_update(&session, &framebuffer));
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(framebuffer.valid);
    CHECK(pixels[5] == changed[0]);
    CHECK(pixels[6] == changed[1]);
    CHECK(pixels[0] == 0x1111);
    CHECK(pixels[11] == 0x1111);
    CHECK(pstvnc_framebuffer_get_dirty(&framebuffer, &dirty));
    CHECK(dirty.x == 1);
    CHECK(dirty.y == 1);
    CHECK(dirty.width == 2);
    CHECK(dirty.height == 1);
}

static void test_live_dirty_resets_per_update(void)
{
    static const uint16_t first = 0x0101;
    static const uint16_t second = 0x0202;
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;
    pstvnc_framebuffer_rect_t dirty;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);

    append_update_header(2);
    append_rectangle(0, 0, 1, 1, 0, &first);
    append_rectangle(3, 2, 1, 1, 0, &second);
    append_update_header(0);

    CHECK(pstvnc_rfb_session_receive_update(&session, &framebuffer));
    CHECK(pstvnc_framebuffer_get_dirty(&framebuffer, &dirty));
    CHECK(dirty.x == 0);
    CHECK(dirty.y == 0);
    CHECK(dirty.width == 4);
    CHECK(dirty.height == 3);

    CHECK(pstvnc_rfb_session_receive_update(&session, &framebuffer));
    CHECK(framebuffer.valid);
    CHECK(!framebuffer.dirty);
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(input_pos == input_size);
}

static void test_live_failure_invalidates(void)
{
    static const unsigned char rectangle_header[12] = {
        0, 1,
        0, 1,
        0, 2,
        0, 1,
        0, 0, 0, 0
    };
    static const unsigned char one_pixel[2] = { 0x34, 0x12 };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);

    append_update_header(1);
    append_input(rectangle_header, sizeof(rectangle_header));
    append_input(one_pixel, sizeof(one_pixel));

    CHECK(!pstvnc_rfb_session_receive_update(&session, &framebuffer));
    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_IO);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

int main(void)
{
    test_success_with_async_messages();
    test_multiple_rectangles();
    test_partial_full_frame_rejected();
    test_protocol_failures();
    test_short_pixel_payload_invalidates();
    test_live_partial_update();
    test_live_dirty_resets_per_update();
    test_live_failure_invalidates();

    if (failures != 0) {
        fprintf(
            stderr,
            "rfb_initial_frame_test: %d failure(s)\n",
            failures);
        return 1;
    }

    puts("rfb_initial_frame_test: PASS");
    return 0;
}
