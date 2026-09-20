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

static unsigned char input[256];
static size_t input_size;
static size_t input_pos;

static void append_input(const void *bytes, size_t count)
{
    CHECK(input_size + count <= sizeof(input));
    if (input_size + count > sizeof(input))
        return;

    memcpy(&input[input_size], bytes, count);
    input_size += count;
}

int pstvnc_rfb_bridge_acquire(
    pstvnc_transport_access_t *transport_access)
{
    if (transport_access == NULL)
        return -1;
    transport_access->opaque_ticket = 1u;
    return 0;
}

int pstvnc_rfb_bridge_read_exact(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t count)
{
    (void)transport_access;
    if (input_pos + count > input_size)
        return -1;

    memcpy(buffer, &input[input_pos], count);
    input_pos += count;
    return 0;
}

int pstvnc_rfb_bridge_poll_receive(
    const pstvnc_transport_access_t *transport_access)
{
    (void)transport_access;
    return input_pos < input_size ? 1 : 0;
}

int pstvnc_rfb_bridge_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count)
{
    (void)transport_access;
    (void)buffer;
    (void)count;
    return 0;
}

int pstvnc_rfb_bridge_quiesce_requested(
    const pstvnc_transport_access_t *transport_access)
{
    (void)transport_access;
    return 0;
}

int pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(
    const pstvnc_transport_access_t *transport_access)
{
    (void)transport_access;
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

static void append_raw_rectangle(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
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
    append_input(header, sizeof(header));

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

static void test_equal_byte_count_overlap_is_not_authoritative(void)
{
    static const uint16_t first[8] = {
        1, 2, 3, 4,
        5, 6, 7, 8
    };
    static const uint16_t overlap[4] = {
        9, 10, 11, 12
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    input_size = 0;
    input_pos = 0;
    memset(input, 0, sizeof(input));

    pstvnc_rfb_session_init(&session);
    session.state = PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME;
    session.server_init.width = 4;
    session.server_init.height = 3;

    CHECK(pstvnc_framebuffer_init(&framebuffer, pixels, 12));
    CHECK(pstvnc_framebuffer_set_geometry(&framebuffer, 4, 3));

    /*
     * Twelve Raw pixels are announced in total, exactly matching 4x3, but the
     * second rectangle rewrites row 1 while row 2 is never supplied. A byte
     * total alone would incorrectly promote this framebuffer to authoritative.
     */
    append_update_header(2);
    append_raw_rectangle(0, 0, 4, 2, first);
    append_raw_rectangle(0, 1, 4, 1, overlap);

    CHECK(!pstvnc_rfb_session_receive_initial_frame(
        &session, &framebuffer));
    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_COVERAGE);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

int main(void)
{
    test_equal_byte_count_overlap_is_not_authoritative();

    if (failures != 0) {
        fprintf(
            stderr,
            "rfb_initial_coverage_test: %d failure(s)\n",
            failures);
        return 1;
    }

    puts("rfb_initial_coverage_test: PASS");
    return 0;
}
