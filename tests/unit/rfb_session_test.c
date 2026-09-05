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

static unsigned char input[1024];
static size_t input_size;
static size_t input_pos;
static unsigned char output[1024];
static size_t output_size;
static int force_write_failure;

static void script_reset(void)
{
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    input_size = 0;
    input_pos = 0;
    output_size = 0;
    force_write_failure = 0;
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

int pstvnc_rfb_io_read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
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

    if (force_write_failure)
        return -1;

    if (output_size + count > sizeof(output))
        return -1;

    memcpy(&output[output_size], buffer, count);
    output_size += count;
    return 0;
}

static void append_server_init(
    uint16_t width,
    uint16_t height,
    uint32_t name_length)
{
    unsigned char bytes[24] = {
        0, 0, 0, 0,
        32, 24, 0, 1,
        0, 255,
        0, 255,
        0, 255,
        16, 8, 0,
        0, 0, 0,
        0, 0, 0, 0
    };

    bytes[0] = (unsigned char)(width >> 8);
    bytes[1] = (unsigned char)width;
    bytes[2] = (unsigned char)(height >> 8);
    bytes[3] = (unsigned char)height;
    bytes[20] = (unsigned char)(name_length >> 24);
    bytes[21] = (unsigned char)(name_length >> 16);
    bytes[22] = (unsigned char)(name_length >> 8);
    bytes[23] = (unsigned char)name_length;

    append_input(bytes, sizeof(bytes));
}

static void build_success_input(
    const unsigned char *name,
    size_t name_length)
{
    static const unsigned char banner[12] = "RFB 003.008\n";
    static const unsigned char security[] = { 2, 2, 1 };
    static const unsigned char result[4] = { 0, 0, 0, 0 };

    append_input(banner, sizeof(banner));
    append_input(security, sizeof(security));
    append_input(result, sizeof(result));
    append_server_init(704, 462, (uint32_t)name_length);
    append_input(name, name_length);
}

static void test_success(void)
{
    static const unsigned char name[] = "clean-desktop";
    static const unsigned char expected_prefix[14] = {
        'R', 'F', 'B', ' ', '0', '0', '3', '.', '0', '0', '8', '\n',
        1, 1
    };
    static const unsigned char pixel_format[20] = {
        0, 0, 0, 0,
        16, 15, 0, 1,
        0, 31,
        0, 31,
        0, 31,
        0, 5, 10,
        0, 0, 0
    };
    static const unsigned char raw[8] = {
        2, 0,
        0, 1,
        0, 0, 0, 0
    };
    static const unsigned char full[10] = {
        3, 0,
        0, 0,
        0, 0,
        0x02, 0xc0,
        0x01, 0xce
    };
    pstvnc_rfb_session_t session;

    script_reset();
    build_success_input(name, sizeof(name) - 1);

    CHECK(pstvnc_rfb_session_start(&session, 7, 704, 462));
    CHECK(session.state == PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(session.server_major == 3);
    CHECK(session.server_minor == 8);
    CHECK(session.server_init.width == 704);
    CHECK(session.server_init.height == 462);
    CHECK(strcmp(session.desktop_name, "clean-desktop") == 0);
    CHECK(input_pos == input_size);

    CHECK(output_size == 52);
    CHECK(memcmp(
        output,
        expected_prefix,
        sizeof(expected_prefix)) == 0);
    CHECK(memcmp(
        output + 14,
        pixel_format,
        sizeof(pixel_format)) == 0);
    CHECK(memcmp(output + 34, raw, sizeof(raw)) == 0);
    CHECK(memcmp(output + 42, full, sizeof(full)) == 0);
}

static void test_live_update_request(void)
{
    static const unsigned char incremental[10] = {
        3, 1,
        0, 0,
        0, 0,
        0x02, 0xc0,
        0x01, 0xce
    };
    static const unsigned char full[10] = {
        3, 0,
        0, 0,
        0, 0,
        0x02, 0xc0,
        0x01, 0xce
    };
    pstvnc_rfb_session_t session;

    script_reset();
    pstvnc_rfb_session_init(&session);
    session.socket_fd = 7;
    session.state = PSTVNC_RFB_SESSION_READY;
    session.server_init.width = 704;
    session.server_init.height = 462;

    CHECK(pstvnc_rfb_session_request_update(&session, 1));
    CHECK(output_size == sizeof(incremental));
    CHECK(memcmp(output, incremental, sizeof(incremental)) == 0);

    script_reset();
    CHECK(pstvnc_rfb_session_request_update(&session, 0));
    CHECK(output_size == sizeof(full));
    CHECK(memcmp(output, full, sizeof(full)) == 0);

    script_reset();
    session.state = PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME;
    CHECK(!pstvnc_rfb_session_request_update(&session, 1));
    CHECK(output_size == 0);
}


static void test_pointer_event_send(void)
{
    static const unsigned char expected[6] = {
        5,
        0x04,
        0x02, 0xbf,
        0x01, 0xcd
    };
    pstvnc_rfb_session_t session;

    script_reset();
    pstvnc_rfb_session_init(&session);

    session.socket_fd = 7;
    session.state = PSTVNC_RFB_SESSION_READY;
    session.server_init.width = 704;
    session.server_init.height = 462;

    CHECK(
        pstvnc_rfb_session_send_pointer_event(
            &session,
            PSTVNC_RFB_POINTER_BUTTON_RIGHT,
            703,
            461));

    CHECK(output_size == sizeof(expected));
    CHECK(memcmp(output, expected, sizeof(expected)) == 0);
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);

    /*
     * Session geometry is the final wire-side guard. Invalid coordinates must
     * produce no bytes and must not convert a healthy session into failure.
     */
    script_reset();

    CHECK(
        !pstvnc_rfb_session_send_pointer_event(
            &session,
            0,
            704,
            461));

    CHECK(
        !pstvnc_rfb_session_send_pointer_event(
            &session,
            0,
            703,
            462));

    CHECK(output_size == 0);
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);

    script_reset();
    session.state = PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME;

    CHECK(
        !pstvnc_rfb_session_send_pointer_event(
            &session,
            0,
            100,
            100));

    CHECK(output_size == 0);

    CHECK(
        !pstvnc_rfb_session_send_pointer_event(
            NULL,
            0,
            0,
            0));

    /*
     * A transport failure is a session failure, matching the existing
     * FramebufferUpdateRequest send contract.
     */
    script_reset();
    pstvnc_rfb_session_init(&session);

    session.socket_fd = 7;
    session.state = PSTVNC_RFB_SESSION_READY;
    session.server_init.width = 704;
    session.server_init.height = 462;

    force_write_failure = 1;

    CHECK(
        !pstvnc_rfb_session_send_pointer_event(
            &session,
            PSTVNC_RFB_POINTER_BUTTON_LEFT,
            100,
            100));

    CHECK(output_size == 0);
    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_IO);
}

static void test_none_missing(void)
{
    static const unsigned char banner[12] = "RFB 003.008\n";
    static const unsigned char security[] = { 2, 2, 16 };
    pstvnc_rfb_session_t session;

    script_reset();
    append_input(banner, sizeof(banner));
    append_input(security, sizeof(security));

    CHECK(!pstvnc_rfb_session_start(&session, 7, 704, 462));
    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error ==
        PSTVNC_RFB_SESSION_ERROR_SECURITY_NONE_UNAVAILABLE);
    CHECK(output_size == 12);
}

static void test_server_rejection(void)
{
    static const unsigned char banner[12] = "RFB 003.008\n";
    static const unsigned char rejection_header[] = {
        0,
        0, 0, 0, 5
    };
    static const unsigned char reason[] = "nope!";
    pstvnc_rfb_session_t session;

    script_reset();
    append_input(banner, sizeof(banner));
    append_input(rejection_header, sizeof(rejection_header));
    append_input(reason, 5);

    CHECK(!pstvnc_rfb_session_start(&session, 7, 704, 462));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_SERVER_REJECTED);
    CHECK(strcmp(session.server_rejection, "nope!") == 0);
    CHECK(input_pos == input_size);
}

static void test_geometry_mismatch(void)
{
    static const unsigned char banner[12] = "RFB 003.008\n";
    static const unsigned char security[] = { 1, 1 };
    static const unsigned char result[4] = { 0, 0, 0, 0 };
    pstvnc_rfb_session_t session;

    script_reset();
    append_input(banner, sizeof(banner));
    append_input(security, sizeof(security));
    append_input(result, sizeof(result));
    append_server_init(640, 480, 0);

    CHECK(!pstvnc_rfb_session_start(&session, 7, 704, 462));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_GEOMETRY);
}

static void test_long_name_consumed(void)
{
    unsigned char name[160];
    pstvnc_rfb_session_t session;
    size_t i;

    for (i = 0; i < sizeof(name); i++)
        name[i] = (unsigned char)('a' + (i % 26));

    script_reset();
    build_success_input(name, sizeof(name));

    CHECK(pstvnc_rfb_session_start(&session, 7, 704, 462));
    CHECK(strlen(session.desktop_name) == PSTVNC_RFB_SESSION_TEXT_MAX);
    CHECK(input_pos == input_size);
}

static void test_bad_version_and_short_io(void)
{
    static const unsigned char old_banner[12] = "RFB 003.003\n";
    pstvnc_rfb_session_t session;

    script_reset();
    append_input(old_banner, sizeof(old_banner));
    CHECK(!pstvnc_rfb_session_start(&session, 7, 704, 462));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_PROTOCOL_VERSION);

    script_reset();
    CHECK(!pstvnc_rfb_session_start(&session, 7, 704, 462));
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_IO);
}

int main(void)
{
    test_success();
    test_live_update_request();
    test_pointer_event_send();
    test_none_missing();
    test_server_rejection();
    test_geometry_mismatch();
    test_long_name_consumed();
    test_bad_version_and_short_io();

    if (failures != 0) {
        fprintf(stderr, "rfb_session_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("rfb_session_test: PASS");
    return 0;
}
