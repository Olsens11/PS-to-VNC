/*
 * File synopsis:
 * Exercises live RFB server-message framing negatives, idle safe-boundary
 * service, and finite-session quiesce ordering against a logical bridge fixture.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Shared Raw
 * server-message parser"; docs/CLEAN_ARCHITECTURE.md, "RFB client/session";
 * docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rfb_session.h"
#include "transport/protocol.h"

static int failures = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                       \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

#define QUIESCE_NEVER ((size_t)-1)

static unsigned char input[512];
static unsigned char output[32];
static size_t input_size;
static size_t input_pos;
static size_t output_size;
static int force_poll_failure;
static int force_quiesce_request_failure;
static int force_quiesce_complete_failure;
static size_t quiesce_after_input_pos;
static int quiesce_complete_calls;

static void script_reset(void)
{
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    input_size = 0;
    input_pos = 0;
    output_size = 0;
    force_poll_failure = 0;
    force_quiesce_request_failure = 0;
    force_quiesce_complete_failure = 0;
    quiesce_after_input_pos = QUIESCE_NEVER;
    quiesce_complete_calls = 0;
}

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
    if (force_poll_failure)
        return -1;

    return input_pos < input_size ? 1 : 0;
}

int pstvnc_rfb_bridge_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count)
{
    (void)transport_access;

    if ((buffer == NULL && count != 0u) ||
        output_size + count > sizeof(output))
        return -1;

    if (count != 0u)
        memcpy(&output[output_size], buffer, count);
    output_size += count;
    return 0;
}

int pstvnc_rfb_bridge_provider_failure(
    const pstvnc_transport_access_t *transport_access,
    pstvnc_rfb_provider_failure_reason_t *reason)
{
    (void)transport_access;
    if (reason == NULL)
        return -1;
    *reason = PSTVNC_RFB_PROVIDER_FAILURE_NONE;
    return 0;
}

int pstvnc_rfb_bridge_quiesce_requested(
    const pstvnc_transport_access_t *transport_access)
{
    (void)transport_access;
    if (force_quiesce_request_failure)
        return -1;

    if (quiesce_after_input_pos != QUIESCE_NEVER &&
        input_pos >= quiesce_after_input_pos)
        return 1;

    return 0;
}

int pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(
    const pstvnc_transport_access_t *transport_access)
{
    (void)transport_access;
    quiesce_complete_calls++;

    if (force_quiesce_complete_failure)
        return -1;

    quiesce_after_input_pos = QUIESCE_NEVER;
    return 0;
}

static void prepare_live(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer,
    uint16_t pixels[12])
{
    pstvnc_rfb_session_init(session);
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

static void test_try_receive_idle_preserves_authority(void)
{
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);

    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_IDLE);

    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(framebuffer.valid);
    CHECK(!framebuffer.dirty);
    CHECK(input_pos == 0);
}

static void test_hw1_isolated_raw_incremental_reaches_message_boundary(void)
{
    static uint16_t pixels[PSTVNC_RFB_SESSION_MAX_FRAME_PIXELS];
    static const unsigned char update_header[4] = {
        0u, 0u, 0u, 1u
    };
    unsigned char rectangle_header[12] = {
        0x02u, 0xa9u, /* x = 681 */
        0x00u, 0x0bu, /* y = 11 */
        0x00u, 0x0cu, /* width = 12 */
        0x00u, 0x0du, /* height = 13 */
        0u, 0u, 0u, 0u /* Raw encoding */
    };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;
    pstvnc_framebuffer_rect_t dirty;
    size_t pixel_index;

    script_reset();
    memset(pixels, 0, sizeof(pixels));
    pstvnc_rfb_session_init(&session);
    session.transport_access.opaque_ticket = 1u;
    session.state = PSTVNC_RFB_SESSION_READY;
    session.server_init.width = 704u;
    session.server_init.height = 462u;

    CHECK(pstvnc_framebuffer_init(
        &framebuffer,
        pixels,
        PSTVNC_RFB_SESSION_MAX_FRAME_PIXELS));
    CHECK(pstvnc_framebuffer_set_geometry(&framebuffer, 704u, 462u));
    CHECK(pstvnc_framebuffer_mark_valid(&framebuffer));

    append_input(update_header, sizeof(update_header));
    append_input(rectangle_header, sizeof(rectangle_header));
    for (pixel_index = 0u; pixel_index < 12u * 13u; pixel_index++) {
        uint16_t value = (uint16_t)(0x2400u + pixel_index);
        unsigned char wire[2] = {
            (unsigned char)value,
            (unsigned char)(value >> 8)
        };

        append_input(wire, sizeof(wire));
    }

    /* Exact sealed HW1 RFB payload: 4 + 12 + (12 * 13 * 2) = 328 bytes. */
    CHECK(input_size == 328u);
    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_UPDATE);
    CHECK(input_pos == input_size);
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(pstvnc_framebuffer_get_dirty(&framebuffer, &dirty));
    CHECK(dirty.x == 681u);
    CHECK(dirty.y == 11u);
    CHECK(dirty.width == 12u);
    CHECK(dirty.height == 13u);
    CHECK(pixels[(size_t)11u * 704u + 681u] == 0x2400u);
    CHECK(pixels[(size_t)23u * 704u + 692u] == (uint16_t)(0x2400u + 155u));

    /*
     * No later server message is present. The complete isolated Raw response has
     * already returned to a true RFB message boundary, so the next incremental
     * request can serialize immediately from that state.
     */
    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_IDLE);
    CHECK(pstvnc_rfb_session_request_update(&session, 1));
    CHECK(output_size == PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE);
    CHECK(output[0] == 3u);
    CHECK(output[1] == 1u);
    CHECK(output[6] == 0x02u);
    CHECK(output[7] == 0xc0u);
    CHECK(output[8] == 0x01u);
    CHECK(output[9] == 0xceu);

    /*
     * Complete a second isolated interval after the first successor request.
     * No stale request/message state from the first Raw update may prevent the
     * same boundary from accepting another complete response and next request.
     */
    input_size = 0u;
    input_pos = 0u;
    append_input(update_header, sizeof(update_header));
    append_input(rectangle_header, sizeof(rectangle_header));
    for (pixel_index = 0u; pixel_index < 12u * 13u; pixel_index++) {
        uint16_t value = (uint16_t)(0x3400u + pixel_index);
        unsigned char wire[2] = {
            (unsigned char)value,
            (unsigned char)(value >> 8)
        };

        append_input(wire, sizeof(wire));
    }

    CHECK(input_size == 328u);
    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_UPDATE);
    CHECK(input_pos == input_size);
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(pixels[(size_t)11u * 704u + 681u] == 0x3400u);
    CHECK(pixels[(size_t)23u * 704u + 692u] == (uint16_t)(0x3400u + 155u));
    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_IDLE);
    CHECK(pstvnc_rfb_session_request_update(&session, 1));
    CHECK(output_size == 2u * PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE);
    CHECK(output[10] == 3u);
    CHECK(output[11] == 1u);
}

static void test_try_receive_empty_update_completes(void)
{
    static const unsigned char empty_update[] = {
        0, 0, 0, 0
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    append_input(empty_update, sizeof(empty_update));

    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_UPDATE);

    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(framebuffer.valid);
    CHECK(!framebuffer.dirty);
    CHECK(input_pos == input_size);
}

static void test_try_receive_bell_then_idle_yields_at_boundary(void)
{
    static const unsigned char bell = 2;
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    append_input(&bell, sizeof(bell));

    /*
     * Bell is one complete server message. The service call may consume it,
     * then return IDLE only after reaching the next complete message boundary.
     */
    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_IDLE);

    CHECK(input_pos == input_size);
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

static void test_try_receive_never_yields_mid_message(void)
{
    static const unsigned char truncated_cut_text[] = {
        3,
        0, 0, 0,
        0, 0, 0
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    append_input(
        truncated_cut_text,
        sizeof(truncated_cut_text));

    /*
     * Once type 3 has begun, the missing remainder is an I/O failure. It must
     * never be converted into the benign IDLE scheduling result.
     */
    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_FAILED);

    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_IO);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

static void test_try_receive_poll_failure_fails_closed(void)
{
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    force_poll_failure = 1;

    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_FAILED);

    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_IO);
    CHECK(!framebuffer.valid);
    CHECK(!framebuffer.dirty);
}

static void test_quiesce_after_complete_message_stops_before_next_byte(void)
{
    static const unsigned char stream[] = {
        2,
        99
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    append_input(stream, sizeof(stream));
    quiesce_after_input_pos = 1;

    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_IDLE);

    CHECK(quiesce_complete_calls == 1);
    CHECK(input_pos == 1);
    CHECK(input[1] == 99);
    CHECK(session.state == PSTVNC_RFB_SESSION_READY);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_NONE);
    CHECK(framebuffer.valid);
}

static void test_quiesce_completion_failure_fails_closed_at_boundary(void)
{
    static const unsigned char stream[] = {
        2,
        99
    };
    uint16_t pixels[12] = { 0 };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    append_input(stream, sizeof(stream));
    quiesce_after_input_pos = 1;
    force_quiesce_complete_failure = 1;

    CHECK(
        pstvnc_rfb_session_try_receive_update(
            &session,
            &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_FAILED);

    CHECK(quiesce_complete_calls == 1);
    CHECK(input_pos == 1);
    CHECK(session.state == PSTVNC_RFB_SESSION_FAILED);
    CHECK(session.error == PSTVNC_RFB_SESSION_ERROR_IO);
    CHECK(!framebuffer.valid);
}

int main(void)
{
    test_server_cut_text_truncation();
    test_color_map_truncation();
    test_unsupported_message_invalidates();
    test_bell_then_empty_update_preserves_authority();
    test_try_receive_idle_preserves_authority();
    test_hw1_isolated_raw_incremental_reaches_message_boundary();
    test_try_receive_empty_update_completes();
    test_try_receive_bell_then_idle_yields_at_boundary();
    test_try_receive_never_yields_mid_message();
    test_try_receive_poll_failure_fails_closed();
    test_quiesce_after_complete_message_stops_before_next_byte();
    test_quiesce_completion_failure_fails_closed_at_boundary();

    if (failures != 0) {
        fprintf(stderr, "rfb_async_framing_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("rfb_async_framing_test: PASS");
    return 0;
}
