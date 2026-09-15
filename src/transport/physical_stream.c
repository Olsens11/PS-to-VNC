/*
 * File synopsis:
 * Implements ownership of the one adopted physical PSTV socket and the single
 * serialized framed-send path. This file deliberately does not receive frames,
 * dispatch logical channels, parse RFB, manage media policy, or decide product
 * lifecycle; those responsibilities remain with their documented owners.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include "physical_stream.h"

#include <kernel.h>
#include <ps2ip.h>

#include <sys/socket.h>
#include <unistd.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int pstvnc_transport_physical_stream_send_exact(
    int socket_fd,
    const uint8_t *bytes,
    size_t byte_count)
{
    size_t sent_total = 0;

    while (sent_total < byte_count) {
        int sent = send(
            socket_fd,
            bytes + sent_total,
            byte_count - sent_total,
            0);

        if (sent <= 0)
            return 0;

        sent_total += (size_t)sent;
    }

    return 1;
}

int pstvnc_transport_physical_stream_adopt(
    pstvnc_transport_physical_stream_t *stream,
    int socket_fd)
{
    ee_sema_t send_mutex;
    int send_semaphore_id;

    if (stream == NULL || socket_fd < 0)
        return 0;

    stream->socket_fd = -1;
    stream->send_semaphore_id = -1;
    stream->next_send_sequence = 1u;

    memset(&send_mutex, 0, sizeof(send_mutex));
    send_mutex.init_count = 1;
    send_mutex.max_count = 1;
    send_mutex.option = 0;

    send_semaphore_id = CreateSema(&send_mutex);
    if (send_semaphore_id < 0)
        return 0;

    stream->socket_fd = socket_fd;
    stream->send_semaphore_id = send_semaphore_id;
    return 1;
}

int pstvnc_transport_physical_stream_send_frame(
    pstvnc_transport_physical_stream_t *stream,
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const void *payload,
    size_t payload_length)
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    pstvnc_transport_header_t header;
    int send_succeeded = 0;

    if (stream == NULL || stream->socket_fd < 0 ||
        stream->send_semaphore_id < 0 ||
        payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD ||
        (payload_length > 0 && payload == NULL))
        return 0;

    if (WaitSema(stream->send_semaphore_id) < 0)
        return 0;

    memset(&header, 0, sizeof(header));
    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = kind;
    header.channel = channel;
    header.flags = flags;
    header.sequence = stream->next_send_sequence;
    header.payload_length = (uint32_t)payload_length;

    if (pstvnc_transport_header_encode(wire_header, &header) &&
        pstvnc_transport_physical_stream_send_exact(
            stream->socket_fd,
            wire_header,
            sizeof(wire_header)) &&
        (payload_length == 0 ||
         pstvnc_transport_physical_stream_send_exact(
             stream->socket_fd,
             (const uint8_t *)payload,
             payload_length))) {
        /* Advance only after the complete frame has reached the socket API. */
        stream->next_send_sequence++;
        send_succeeded = 1;
    }

    if (SignalSema(stream->send_semaphore_id) < 0)
        return 0;

    return send_succeeded;
}

void pstvnc_transport_physical_stream_release(
    pstvnc_transport_physical_stream_t *stream)
{
    if (stream == NULL)
        return;

    if (stream->socket_fd >= 0)
        close(stream->socket_fd);
    if (stream->send_semaphore_id >= 0)
        DeleteSema(stream->send_semaphore_id);

    stream->socket_fd = -1;
    stream->send_semaphore_id = -1;
    stream->next_send_sequence = 1u;
}
