/*
 * File synopsis:
 * Defines RFB's single cross-component byte-stream bridge to the ledge Transport
 * owner. It preserves RFB's simple exact-read / readiness / exact-write contract
 * while removing physical socket identity from the protocol-facing interface.
 *
 * This bridge does not parse RFB, own Transport lifecycle, choose application
 * recovery policy, or expose the physical PSTV descriptor.
 *
 * Context: docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md, "RFB ownership under
 * shared transport"; docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#ifndef PSTVNC_RFB_BRIDGE_H
#define PSTVNC_RFB_BRIDGE_H

#include <stddef.h>

/* Return 0 on exact success and -1 on terminal/transport failure. */
int pstvnc_rfb_bridge_read_exact(
    void *buffer,
    size_t count);

/* Return 1 when bytes are ready, 0 while idle, and -1 on terminal failure. */
int pstvnc_rfb_bridge_poll_receive(void);

/* Return 0 only when every requested client byte was accepted by Transport. */
int pstvnc_rfb_bridge_write_exact(
    const void *buffer,
    size_t count);

#endif
