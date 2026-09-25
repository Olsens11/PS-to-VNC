/*
 * File synopsis:
 * Implements the bounded read-only HTTP/1.0 management GET for /ps2vnc.conf.
 * This owner uses one fresh management descriptor, accepts only HTTP/1.0 or
 * HTTP/1.1 status 200, frames the body by clean peer close, and publishes raw
 * bytes atomically. It does not parse configuration or execute product policy.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * B11-READ-ONLY-HUMAN-CONFIG-GET-R31;
 * docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md.
 */

#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "config_get.h"
#include "platform/ps2_network.h"

#define PSTVNC_MANAGEMENT_CONFIG_REQUEST_MAX_BYTES 128u
#define PSTVNC_MANAGEMENT_RECEIVE_CHUNK_BYTES 256u

static const char config_request[] =
    "GET /ps2vnc.conf HTTP/1.0\r\n"
    "Host: 192.168.50.1\r\n"
    "Connection: close\r\n"
    "\r\n";

static int management_status_is_200(
    const unsigned char *header,
    size_t header_length)
{
    size_t line_end;

    if (header == NULL || header_length < 14u)
        return 0;

    if (memcmp(header, "HTTP/1.", 7u) != 0 ||
        (header[7] != '0' && header[7] != '1') ||
        header[8] != ' ' ||
        header[9] != '2' ||
        header[10] != '0' ||
        header[11] != '0')
        return 0;

    for (line_end = 12u; line_end + 1u < header_length; line_end++) {
        if (header[line_end] == '\r' &&
            header[line_end + 1u] == '\n')
            break;
    }

    if (line_end + 1u >= header_length)
        return 0;

    return header[12] == ' ' || header[12] == '\r';
}

static int management_send_request(int socket_fd)
{
    const size_t request_length = sizeof(config_request) - 1u;
    size_t sent = 0u;

    if (request_length > PSTVNC_MANAGEMENT_CONFIG_REQUEST_MAX_BYTES)
        return 0;

    while (sent < request_length) {
        ssize_t result = send(
            socket_fd,
            config_request + sent,
            request_length - sent,
            0);

        if (result <= 0 ||
            (size_t)result > request_length - sent)
            return 0;

        sent += (size_t)result;
    }

    return 1;
}

static int management_append_body(
    unsigned char *candidate,
    size_t *candidate_length,
    size_t capacity,
    const unsigned char *bytes,
    size_t byte_count)
{
    if (candidate == NULL ||
        candidate_length == NULL ||
        bytes == NULL)
        return 0;

    if (*candidate_length > capacity ||
        byte_count > capacity - *candidate_length)
        return 0;

    if (byte_count > 0u)
        memcpy(candidate + *candidate_length, bytes, byte_count);

    *candidate_length += byte_count;
    return 1;
}

int pstvnc_management_config_get(
    unsigned char *body,
    size_t body_capacity,
    size_t *body_length)
{
    unsigned char header[PSTVNC_MANAGEMENT_HTTP_MAX_HEADER_BYTES];
    unsigned char candidate_body[PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES];
    unsigned char receive_buffer[PSTVNC_MANAGEMENT_RECEIVE_CHUNK_BYTES];
    size_t header_length = 0u;
    size_t candidate_length = 0u;
    int header_complete = 0;
    int socket_fd = -1;
    int success = 0;

    if (body_length == NULL ||
        body_capacity > PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES ||
        (body_capacity > 0u && body == NULL))
        return -1;

    socket_fd = pstvnc_ps2_network_connect_management();
    if (socket_fd < 0)
        return -1;

    if (!management_send_request(socket_fd))
        goto done;

    for (;;) {
        ssize_t received = recv(
            socket_fd,
            receive_buffer,
            sizeof(receive_buffer),
            0);
        size_t cursor = 0u;

        if (received < 0)
            goto done;

        if (received == 0)
            break;

        if (!header_complete) {
            while (cursor < (size_t)received && !header_complete) {
                if (header_length >= sizeof(header))
                    goto done;

                header[header_length++] = receive_buffer[cursor++];

                if (header_length >= 4u &&
                    header[header_length - 4u] == '\r' &&
                    header[header_length - 3u] == '\n' &&
                    header[header_length - 2u] == '\r' &&
                    header[header_length - 1u] == '\n') {
                    header_complete = 1;

                    if (!management_status_is_200(
                            header,
                            header_length))
                        goto done;
                }
            }
        }

        if (header_complete && cursor < (size_t)received) {
            if (!management_append_body(
                    candidate_body,
                    &candidate_length,
                    body_capacity,
                    receive_buffer + cursor,
                    (size_t)received - cursor))
                goto done;
        }
    }

    if (!header_complete)
        goto done;

    if (candidate_length > 0u)
        memcpy(body, candidate_body, candidate_length);

    *body_length = candidate_length;
    success = 1;

done:
    pstvnc_ps2_network_close(socket_fd);
    return success ? 0 : -1;
}
