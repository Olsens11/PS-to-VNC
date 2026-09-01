/*
 * File synopsis:
 * Owns the synchronized RFB handshake, requests, server-message framing, Raw
 * decoding, initial coverage proof, and fail-closed state.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "rfb_io.h"
#include "rfb_session.h"

#define PSTVNC_RFB_INITIAL_COVERAGE_BYTES \
    ((PSTVNC_RFB_SESSION_MAX_FRAME_PIXELS + 7u) / 8u)

/*
 * Temporary parser scratch for the single main-thread-owned Issue #7 session.
 * This is not product state: it is cleared before each strict initial frame and
 * exists only to prove that every authoritative pixel is written exactly once.
 */
static uint8_t initial_frame_coverage[PSTVNC_RFB_INITIAL_COVERAGE_BYTES];

static uint32_t read_be32(const uint8_t bytes[4])
{
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}

static uint16_t read_be16(const uint8_t bytes[2])
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static int read_exact(int socket_fd, void *buffer, size_t count)
{
    return pstvnc_rfb_io_read_exact(socket_fd, buffer, count) == 0;
}

static int write_exact(int socket_fd, const void *buffer, size_t count)
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

    /*
     * Retain only bounded diagnostic text, but consume the server's entire
     * declared field. Truncating storage must not leave unread bytes to be
     * mistaken for the next RFB message.
     */
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

static int discard_exact(int socket_fd, uint32_t count)
{
    uint8_t discard[64];

    while (count > 0) {
        size_t chunk = count;

        if (chunk > sizeof(discard))
            chunk = sizeof(discard);

        if (!read_exact(socket_fd, discard, chunk))
            return 0;

        count -= (uint32_t)chunk;
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

static int fail_frame(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer,
    pstvnc_rfb_session_error_t error)
{
    /*
     * Rectangle decoding writes directly into the owned framebuffer. Any error
     * may therefore occur after partial mutation; invalidating authority keeps
     * later code from presenting a mixture of old and incomplete new state.
     */
    pstvnc_framebuffer_invalidate(framebuffer);
    return fail(session, error);
}

static int framebuffer_matches_session(
    const pstvnc_rfb_session_t *session,
    const pstvnc_framebuffer_t *framebuffer)
{
    return session != NULL &&
           framebuffer != NULL &&
           session->socket_fd >= 0 &&
           framebuffer->width != 0 &&
           framebuffer->height != 0 &&
           framebuffer->width == session->server_init.width &&
           framebuffer->height == session->server_init.height;
}

static int region_fits(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint16_t framebuffer_width,
    uint16_t framebuffer_height)
{
    return (uint32_t)x + width <= framebuffer_width &&
           (uint32_t)y + height <= framebuffer_height;
}

static int mark_initial_frame_coverage(
    const pstvnc_framebuffer_t *framebuffer,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    size_t *covered_pixels)
{
    uint16_t row;
    uint16_t column;

    /*
     * Byte totals alone cannot prove a complete initial desktop: overlapping
     * rectangles can duplicate pixels while leaving an equal-sized gap. This
     * bitmap rejects every duplicate and counts unique pixel authority.
     */
    for (row = 0; row < height; row++) {
        for (column = 0; column < width; column++) {
            size_t pixel_index =
                (size_t)(y + row) * framebuffer->width + (x + column);
            size_t byte_index = pixel_index >> 3;
            uint8_t bit = (uint8_t)(1u << (pixel_index & 7u));

            if ((initial_frame_coverage[byte_index] & bit) != 0)
                return 0;

            initial_frame_coverage[byte_index] |= bit;
            (*covered_pixels)++;
        }
    }

    return 1;
}

static int read_raw_row(
    pstvnc_rfb_session_t *session,
    uint16_t width)
{
    uint8_t *bytes = (uint8_t *)session->row_scratch;
    uint16_t column;
    size_t byte_count = (size_t)width * 2u;

    if (!read_exact(session->socket_fd, bytes, byte_count))
        return 0;

    for (column = 0; column < width; column++) {
        session->row_scratch[column] =
            (uint16_t)bytes[(size_t)column * 2u] |
            (uint16_t)((uint16_t)bytes[(size_t)column * 2u + 1u] << 8);
    }

    return 1;
}

static int receive_framebuffer_update(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer,
    int require_full)
{
    /*
     * Bell, clipboard, and color-map messages may legally arrive before the
     * requested framebuffer update. They are consumed to exact boundaries so
     * the single blocking stream remains synchronized; only a completed type-0
     * update returns control to the application.
     */
    for (;;) {
        uint8_t message_type;

        if (!read_exact(session->socket_fd, &message_type, 1))
            return fail_frame(
                session,
                framebuffer,
                PSTVNC_RFB_SESSION_ERROR_IO);

        if (message_type == 2)
            continue;

        if (message_type == 3) {
            uint8_t cut_header[7];
            uint32_t text_length;

            if (!read_exact(
                    session->socket_fd,
                    cut_header,
                    sizeof(cut_header)))
                return fail_frame(
                    session,
                    framebuffer,
                    PSTVNC_RFB_SESSION_ERROR_IO);

            text_length = read_be32(&cut_header[3]);
            if (!discard_exact(session->socket_fd, text_length))
                return fail_frame(
                    session,
                    framebuffer,
                    PSTVNC_RFB_SESSION_ERROR_IO);

            continue;
        }

        if (message_type == 1) {
            uint8_t color_header[5];
            uint16_t color_count;
            uint32_t payload_length;

            if (!read_exact(
                    session->socket_fd,
                    color_header,
                    sizeof(color_header)))
                return fail_frame(
                    session,
                    framebuffer,
                    PSTVNC_RFB_SESSION_ERROR_IO);

            color_count = read_be16(&color_header[3]);
            payload_length = (uint32_t)color_count * 6u;

            if (!discard_exact(session->socket_fd, payload_length))
                return fail_frame(
                    session,
                    framebuffer,
                    PSTVNC_RFB_SESSION_ERROR_IO);

            continue;
        }

        if (message_type == 0) {
            uint8_t update_header[3];
            uint16_t rectangle_count;
            uint16_t rectangle_index;
            size_t covered_pixels = 0;
            size_t total_pixel_bytes = 0;
            size_t framebuffer_pixels =
                pstvnc_framebuffer_pixel_count(framebuffer);
            size_t required_pixel_bytes = framebuffer_pixels * 2u;

            if (require_full) {
                size_t coverage_bytes;

                if (framebuffer_pixels >
                    PSTVNC_RFB_SESSION_MAX_FRAME_PIXELS)
                    return fail_frame(
                        session,
                        framebuffer,
                        PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_COVERAGE);

                coverage_bytes = (framebuffer_pixels + 7u) / 8u;
                memset(initial_frame_coverage, 0, coverage_bytes);
            }

            if (!read_exact(
                    session->socket_fd,
                    update_header,
                    sizeof(update_header)))
                return fail_frame(
                    session,
                    framebuffer,
                    PSTVNC_RFB_SESSION_ERROR_IO);

            rectangle_count = read_be16(&update_header[1]);
            pstvnc_framebuffer_clear_dirty(framebuffer);

            if (require_full && rectangle_count == 0)
                return fail_frame(
                    session,
                    framebuffer,
                    PSTVNC_RFB_SESSION_ERROR_EMPTY_UPDATE);

            for (rectangle_index = 0;
                 rectangle_index < rectangle_count;
                 rectangle_index++) {
                uint8_t rectangle_header[12];
                uint16_t x;
                uint16_t y;
                uint16_t width;
                uint16_t height;
                uint32_t encoding;
                uint16_t row;

                if (!read_exact(
                        session->socket_fd,
                        rectangle_header,
                        sizeof(rectangle_header)))
                    return fail_frame(
                        session,
                        framebuffer,
                        PSTVNC_RFB_SESSION_ERROR_IO);

                x = read_be16(&rectangle_header[0]);
                y = read_be16(&rectangle_header[2]);
                width = read_be16(&rectangle_header[4]);
                height = read_be16(&rectangle_header[6]);
                encoding = read_be32(&rectangle_header[8]);

                if (encoding != (uint32_t)PSTVNC_RFB_ENCODING_RAW)
                    return fail_frame(
                        session,
                        framebuffer,
                        PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_ENCODING);

                if (!region_fits(
                        x,
                        y,
                        width,
                        height,
                        framebuffer->width,
                        framebuffer->height))
                    return fail_frame(
                        session,
                        framebuffer,
                        PSTVNC_RFB_SESSION_ERROR_RECTANGLE_BOUNDS);

                if (require_full &&
                    !mark_initial_frame_coverage(
                        framebuffer,
                        x,
                        y,
                        width,
                        height,
                        &covered_pixels))
                    return fail_frame(
                        session,
                        framebuffer,
                        PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_COVERAGE);

                for (row = 0; row < height; row++) {
                    if (!read_raw_row(session, width))
                        return fail_frame(
                            session,
                            framebuffer,
                            PSTVNC_RFB_SESSION_ERROR_IO);

                    if (width > 0 &&
                        !pstvnc_framebuffer_write_rect(
                            framebuffer,
                            x,
                            (uint16_t)(y + row),
                            width,
                            1,
                            session->row_scratch,
                            width))
                        return fail_frame(
                            session,
                            framebuffer,
                            PSTVNC_RFB_SESSION_ERROR_RECTANGLE_BOUNDS);

                    total_pixel_bytes += (size_t)width * 2u;
                }
            }

            if (require_full) {
                /*
                 * The size check catches excess/short Raw payload accounting;
                 * the independent coverage check proves pixel identity. Both
                 * must pass before framebuffer.valid becomes authoritative.
                 */
                if (total_pixel_bytes != required_pixel_bytes)
                    return fail_frame(
                        session,
                        framebuffer,
                        PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_SIZE);

                if (covered_pixels != framebuffer_pixels)
                    return fail_frame(
                        session,
                        framebuffer,
                        PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_COVERAGE);

                if (!pstvnc_framebuffer_mark_valid(framebuffer))
                    return fail_frame(
                        session,
                        framebuffer,
                        PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_COVERAGE);
            }

            session->error = PSTVNC_RFB_SESSION_ERROR_NONE;
            return 1;
        }

        return fail_frame(
            session,
            framebuffer,
            PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_SERVER_MESSAGE);
    }
}

void pstvnc_rfb_session_init(pstvnc_rfb_session_t *session)
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
            banner, &session->server_major, &session->server_minor) ||
        session->server_major != 3 || session->server_minor < 8)
        return fail(session, PSTVNC_RFB_SESSION_ERROR_PROTOCOL_VERSION);

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
                socket_fd, reason_length, session->server_rejection))
            return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

        return fail(session, PSTVNC_RFB_SESSION_ERROR_SERVER_REJECTED);
    }

    if (!read_exact(socket_fd, security_types, security_count))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!pstvnc_rfb_choose_security_none(
            security_types, security_count, &security_choice))
        return fail(
            session,
            PSTVNC_RFB_SESSION_ERROR_SECURITY_NONE_UNAVAILABLE);

    if (!write_exact(socket_fd, &security_choice, 1))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!read_exact(socket_fd, security_result, sizeof(security_result)))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!pstvnc_rfb_security_result_ok(security_result))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_SECURITY_RESULT);

    shared_flag = pstvnc_rfb_client_init_shared();
    if (!write_exact(socket_fd, &shared_flag, 1))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!read_exact(socket_fd, server_init_bytes, sizeof(server_init_bytes)))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    if (!pstvnc_rfb_parse_server_init(
            server_init_bytes, &session->server_init))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_SERVER_INIT);

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
            socket_fd, message, PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    pstvnc_rfb_build_set_encodings_raw(message);
    if (!write_exact(
            socket_fd, message, PSTVNC_RFB_SET_ENCODINGS_RAW_SIZE))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    /*
     * The first request is deliberately non-incremental. READY is withheld
     * until its response proves a complete authoritative desktop; incremental
     * updates are meaningful only after that baseline exists.
     */
    pstvnc_rfb_build_framebuffer_update_request(
        message, 0, 0, 0, expected_width, expected_height);
    if (!write_exact(
            socket_fd, message, PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    session->state = PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME;
    session->error = PSTVNC_RFB_SESSION_ERROR_NONE;
    return 1;
}

int pstvnc_rfb_session_request_update(
    pstvnc_rfb_session_t *session,
    int incremental)
{
    uint8_t message[PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE];

    if (session == NULL ||
        session->socket_fd < 0 ||
        session->state != PSTVNC_RFB_SESSION_READY ||
        session->server_init.width == 0 ||
        session->server_init.height == 0)
        return 0;

    pstvnc_rfb_build_framebuffer_update_request(
        message,
        incremental,
        0,
        0,
        session->server_init.width,
        session->server_init.height);

    if (!write_exact(
            session->socket_fd,
            message,
            sizeof(message)))
        return fail(session, PSTVNC_RFB_SESSION_ERROR_IO);

    session->error = PSTVNC_RFB_SESSION_ERROR_NONE;
    return 1;
}

int pstvnc_rfb_session_receive_initial_frame(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer)
{
    if (!framebuffer_matches_session(session, framebuffer) ||
        session->state != PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME)
        return 0;

    if (framebuffer->width > PSTVNC_RFB_SESSION_MAX_ROW_PIXELS)
        return fail_frame(
            session,
            framebuffer,
            PSTVNC_RFB_SESSION_ERROR_ROW_WIDTH);

    pstvnc_framebuffer_invalidate(framebuffer);

    if (!receive_framebuffer_update(session, framebuffer, 1))
        return 0;

    session->state = PSTVNC_RFB_SESSION_READY;
    return 1;
}

int pstvnc_rfb_session_receive_update(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer)
{
    if (!framebuffer_matches_session(session, framebuffer) ||
        session->state != PSTVNC_RFB_SESSION_READY ||
        !framebuffer->valid)
        return 0;

    if (framebuffer->width > PSTVNC_RFB_SESSION_MAX_ROW_PIXELS)
        return fail_frame(
            session,
            framebuffer,
            PSTVNC_RFB_SESSION_ERROR_ROW_WIDTH);

    return receive_framebuffer_update(session, framebuffer, 0);
}
