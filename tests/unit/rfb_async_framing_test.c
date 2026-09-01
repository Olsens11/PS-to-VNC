/*
 * File synopsis:
 * Exercises live RFB server-message framing negatives that must fail closed
 * after framebuffer authority has already been established.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Shared Raw
 * server-message parser"; docs/CLEAN_ARCHITECTURE.md, "RFB client/session".
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rfb_session.h"

static int failures = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                       \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static unsigned char input[128];
static size_t input_size;
static size_t input_pos;

static void script_reset(void)
{
    memset(input, 0, sizeof(input));
    input_size = 0;
    input_pos = 0;
}

static void append_input(const void *bytes, size_t count)
{
    CHECK(input_size + count <= sizeof(input));
    if (input_size + count > sizeof(input))
        return;

    memcpy(&input[input_size], bytes, count);
    input_size += count;
}

int pstvnc_rfb_io_read_exact(int socket_fd, void *buffer, size_t count)
{
    (void)socket_fd;

    if (input_pos + count > input_size)
        return -1;

    memcpy(buffer, &input[input_pos], count);
    input_pos += count;
    return 0;
}

int pstvnc_rfb_io_write_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    (void)socket_fd;
    (void)buffer;
    (void)count;
    return 0;
}

static void prepare_live(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer,
    uint16_t pixels[12])
{
    pstvnc_rfb_session_init(session);
    session->socket_fd = 7;
    session->state = PSTVNC_RFB_SESSION_READY;
    session->server_init.width = 4;
    session->server_init.height = 3;

    CHECK(pstvnc_framebuffer_init(framebuffer, pixels, 12));
    CHECK(pstvnc_framebuffer_set_geometry(framebuffer, 4, 3));
    CHECK(pstvnc_framebuffer_mark_valid(framebuffer));
}

static void check_io_failure(const unsigned char *bytes, size_t count)
{
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    append_input(bytes, count);

    CHECK(!pstvnc_rfb_session_receive_update(&session, &framebuffer));
    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_IO);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

static void test_server_cut_text_truncation(void)
{
    static const unsigned char short_header[] = {
        3,
        0, 0, 0,
        0, 0, 0
    };
    static const unsigned char short_payload[] = {
        3,
        0, 0, 0,
        0, 0, 0, 3,
        'a', 'b'
    };

    check_io_failure(short_header, sizeof(short_header));
    check_io_failure(short_payload, sizeof(short_payload));
}

static void test_color_map_truncation(void)
{
    static const unsigned char short_header[] = {
        1,
        0,
        0, 0,
        0
    };
    static const unsigned char short_payload[] = {
        1,
        0,
        0, 0,
        0, 1,
        0, 0, 0, 0, 0
    };

    check_io_failure(short_header, sizeof(short_header));
    check_io_failure(short_payload, sizeof(short_payload));
}

static void test_unsupported_message_invalidates(void)
{
    static const unsigned char unsupported = 99;
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    append_input(&unsupported, 1);

    CHECK(!pstvnc_rfb_session_receive_update(&session, &framebuffer));
    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error ==
        PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_SERVER_MESSAGE);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

static void test_bell_then_empty_update_preserves_authority(void)
{
    static const unsigned char stream[] = {
        2,
        0, 0, 0, 0
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    append_input(stream, sizeof(stream));

    CHECK(pstvnc_rfb_session_receive_update(&session, &framebuffer));
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(framebuffer.valid);
    CHECK(!framebuffer.dirty);
    CHECK(input_pos == input_size);
}

int main(void)
{
    test_server_cut_text_truncation();
    test_color_map_truncation();
    test_unsupported_message_invalidates();
    test_bell_then_empty_update_preserves_authority();

    if (failures != 0) {
        fprintf(stderr, "rfb_async_framing_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("rfb_async_framing_test: PASS");
    return 0;
}
