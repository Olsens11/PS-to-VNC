/*
 * File synopsis:
 * Defines the transport-internal owner for the one adopted physical PSTV
 * socket and its serialized framed-send and sole-receive sequencing state.
 * This boundary owns physical framing only; it does not own logical-channel
 * queues, RFB semantics, exact-generation lifecycle state, media policy, or
 * application lifecycle policy.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md; docs/ledge/
 * LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#ifndef PSTVNC_TRANSPORT_PHYSICAL_STREAM_H
#define PSTVNC_TRANSPORT_PHYSICAL_STREAM_H

#include "protocol.h"

#include <stddef.h>
#include <stdint.h>

typedef struct pstvnc_transport_physical_stream {
    int socket_fd;
    int send_semaphore_id;
    uint32_t next_send_sequence;
    uint32_t expected_receive_sequence;
} pstvnc_transport_physical_stream_t;

/* Ownership of socket_fd transfers to stream only when this call succeeds. */
int pstvnc_transport_physical_stream_adopt(
    pstvnc_transport_physical_stream_t *stream,
    int socket_fd);

/*
 * Sends one complete PSTV frame under the stream's single send lock.
 * Sequence advances only after the entire header and payload are committed.
 */
int pstvnc_transport_physical_stream_send_frame(
    pstvnc_transport_physical_stream_t *stream,
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const void *payload,
    size_t payload_length);

/*
 * PS2-side outbound START seam. This is framing only: exact-generation state
 * stays with application coordination, while physical_stream keeps the one
 * ordered send path and sequence space for the full-duplex PSTV connection.
 */
static inline int pstvnc_transport_physical_stream_send_mpeg_start(
    pstvnc_transport_physical_stream_t *stream,
    const pstvnc_mpeg_start_payload_t *start)
{
    uint8_t payload[PSTVNC_MPEG_START_PAYLOAD_SIZE];

    if (stream == NULL ||
        !pstvnc_mpeg_start_payload_encode(payload, start))
        return 0;

    return pstvnc_transport_physical_stream_send_frame(
        stream,
        PSTVNC_TRANSPORT_FRAME_MPEG_START,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0u,
        payload,
        sizeof(payload));
}

/*
 * Receives exactly one complete PSTV frame from the sole physical socket.
 * The caller supplies storage large enough for the accepted payload ceiling.
 * Inbound sequence advances only after the complete payload has been read.
 */
/*
 * Wait for inbound socket readability without consuming Wire bytes.
 * Returns 1 when readable, 0 on timeout, and -1 on readiness failure.
 */
int pstvnc_transport_physical_stream_wait_readable(
    pstvnc_transport_physical_stream_t *stream,
    uint32_t timeout_us);

int pstvnc_transport_physical_stream_receive_frame(
    pstvnc_transport_physical_stream_t *stream,
    pstvnc_transport_header_t *header,
    void *payload,
    size_t payload_capacity);

/*
 * Interrupt current-session physical I/O without transferring or releasing
 * descriptor ownership. Transport uses this only to make a blocked sole
 * receiver converge after an application-local fatal-session request.
 */
int pstvnc_transport_physical_stream_shutdown_io(
    pstvnc_transport_physical_stream_t *stream);

/*
 * Releases the adopted socket and send lock. The higher transport runtime must
 * prove receiver/dispatch quiescence before calling this once receive exists.
 */
void pstvnc_transport_physical_stream_release(
    pstvnc_transport_physical_stream_t *stream);

#endif