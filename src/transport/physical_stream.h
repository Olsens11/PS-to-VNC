/*
 * File synopsis:
 * Defines the transport-internal owner for the one adopted physical PSTV
 * socket and its serialized framed-send state. This boundary owns only physical
 * stream transmission state; it does not own receive dispatch, logical-channel
 * queues, RFB semantics, media policy, or application lifecycle policy.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
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
 * Releases the adopted socket and send lock. The higher transport runtime must
 * prove receiver/dispatch quiescence before calling this once receive exists.
 */
void pstvnc_transport_physical_stream_release(
    pstvnc_transport_physical_stream_t *stream);

#endif
