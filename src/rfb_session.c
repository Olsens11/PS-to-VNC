#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "rfb_io.h"
#include "rfb_session.h"

static uint32_t read_be32(const uint8_t bytes[4])
{
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}

static int read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    return pstvnc_rfb_io_read_exact(socket_fd, buffer, count) == 0;
}

static int write_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    return pstvnc_rfb_io_write_exact(socket_fd, buffer, count) == 0;
}

static int read_bounded_text(
    int socket_fd,
    uint32_t length,
    char out[PSTVNC_RFB_SESSION_TEXT_MAX + 1u])
{
    uint8_t discard[64];
    size_t take = length;
    uint32_t remaining;

    if (take > PSTVNC_RFB_SESSION_TEXT_MAX)
        take = PSTVNC_RFB_SESSION_TEXT_MAX;

    if (take > 0 && !read_exact(socket_fd, out, take))
        return 0;

    out[take] = '\0';
    remaining = length - (uint32_t)take;

    while (remaining > 0) {
        size_t chunk = remaining;

        if (chunk > sizeof(discard))
            chunk = sizeof(discard);

        if (!read_exact(socket_fd, discard, chunk))
            return 0;

        remaining -= (uint32_t)chunk;
    }

    return 1;
}

static int fail(
    pstvnc_rfb_session_t *session,
    pstvnc_rfb_session_error_t error)
{
    session->state = PSTVNC_RFB_SESSION_FAILED;
    session->error = error;
    return 0;
}

void pstvnc_rfb_session_init(
    pstvnc_rfb_session_t *session)
{
    if (session == NULL)
        return;

    memset(session, 0, sizeof(*session));
    session->socket_fd = -1;
    session->state = PSTVNC_RFB_SESSION_NEW;
}

int pstvnc_rfb_session_start(
    pstvnc_rfb_session_t *session,
    int socket_fd,
    uint16_t expected_width,
    uint16_t expected_height)
{
    uint8_t banner[PSTVNC_RFB_PROTOCOL_VERSION_SIZE];
    uint8_t security_count;
    uint8_t security_types[255];
    uint8_t security_choice;
    uint8_t security_result[4];
    uint8_t shared_flag;
    uint8_t server_init_bytes[PSTVNC_RFB_SERVER_INIT_SIZE];
    uint8_t message[PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE];
    uint8_t reason_length_bytes[4];
    uint32_t reason_length;

    if (session == NULL || socket_fd < 0 ||
        expected_width == 0 || expected_height == 0)
        return 0;

    pstvnc_rfb_session_init(session);
    session->socket_fd = socket_fd;

    if (!read_exact(socket_fd, banner, sizeof(banner)))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!pstvnc_rfb_parse_protocol_version(
            banner,
            &session->server_major,
            &session->server_minor) ||
        session->server_major != 3 ||
        session->server_minor < 8)
        return fail(
            session,
            PSTVNC_RFB_SESSION_ERROR_PROTOCOL_VERSION);

    pstvnc_rfb_build_client_version(banner);
    if (!write_exact(socket_fd, banner, sizeof(banner)))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!read_exact(socket_fd, &security_count, 1))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (security_count == 0) {
        if (!read_exact(socket_fd, reason_length_bytes, 4))
            return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

        reason_length = read_be32(reason_length_bytes);
        if (!read_bounded_text(
                socket_fd,
                reason_length,
                session->server_rejection))
            return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

        return fail(
            session,
            PSTVNC_RFB_SESSION_ERROR_SERVER_REJECTED);
    }

    if (!read_exact(
            socket_fd,
            security_types,
            security_count))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!pstvnc_rfb_choose_security_none(
            security_types,
            security_count,
            &security_choice))
        return fail(
            session,
            PSTVNC_RFB_SESSION_ERROR_SECURITY_NONE_UNAVAILABLE);

    if (!write_exact(socket_fd, &security_choice, 1))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!read_exact(
            socket_fd,
            security_result,
            sizeof(security_result)))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!pstvnc_rfb_security_result_ok(security_result))
        return fail(
            session,
            PSTVNC_RFB_SESSION_ERROR_SECURITY_RESULT);

    shared_flag = pstvnc_rfb_client_init_shared();
    if (!write_exact(socket_fd, &shared_flag, 1))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!read_exact(
            socket_fd,
            server_init_bytes,
            sizeof(server_init_bytes)))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!pstvnc_rfb_parse_server_init(
            server_init_bytes,
            &session->server_init))
        return fail(
            session,
            PSTVNC_RFB_SESSION_ERROR_SERVER_INIT);

    if (session->server_init.width != expected_width ||
        session->server_init.height != expected_height)
        return fail(session, PSTVNC_RFB_SESSION_ERROR_GEOMETRY);

    if (!read_bounded_text(
            socket_fd,
            session->server_init.name_length,
            session->desktop_name))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    pstvnc_rfb_build_set_pixel_format_gs555(message);
    if (!write_exact(
            socket_fd,
            message,
            PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    pstvnc_rfb_build_set_encodings_raw(message);
    if (!write_exact(
            socket_fd,
            message,
            PSTVNC_RFB_SET_ENCODINGS_RAW_SIZE))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    pstvnc_rfb_build_framebuffer_update_request(
        message,
        0,
        0,
        0,
        expected_width,
        expected_height);
    if (!write_exact(
            socket_fd,
            message,
            PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    session->state = PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME;
    session->error = PSTVNC_RFB_SESSION_ERROR_NONE;
    return 1;
}
