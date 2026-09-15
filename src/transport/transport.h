/*
 * File synopsis:
 * Defines the ledge shared-PSTV transport owner's public lifecycle and logical
 * RFB byte-stream boundary. The transport owns the physical stream; protocol
 * components never receive from or send to that socket directly.
 *
 * Context: docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md, "A001 transport ownership
 * reconciliation".
 */

#ifndef PSTVNC_TRANSPORT_H
#define PSTVNC_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

typedef enum pstvnc_transport_result {
    PSTVNC_TRANSPORT_OK = 0,
    PSTVNC_TRANSPORT_WOULD_BLOCK = 1,
    PSTVNC_TRANSPORT_CLOSED = -1,
    PSTVNC_TRANSPORT_INVALID = -2
} pstvnc_transport_result_t;

/*
 * Logical RFB stream operations exposed by the transport owner.
 *
 * The physical socket descriptor is deliberately absent. Reads consume bytes
 * already dispatched by the sole transport receiver. Writes are serialized by
 * the transport before becoming physical PSTV frames.
 */
int pstvnc_transport_rfb_read_exact(void *buffer, size_t count);
int pstvnc_transport_rfb_poll_receive(void);
int pstvnc_transport_rfb_write_exact(const void *buffer, size_t count);

/*
 * Session lifecycle is application-requested but transport-owned. Quiesce must
 * not report completion while a receiver dispatch can still touch channel
 * resources; diagnostics may observe that fact but are not synchronization.
 */
int pstvnc_transport_session_open(int socket_fd, int enable_rfb);
int pstvnc_transport_session_quiesce(void);
void pstvnc_transport_session_close(void);

#endif
