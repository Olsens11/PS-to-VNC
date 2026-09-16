/*
 * File synopsis:
 * Defines RFB's single cross-component byte-stream bridge to the ledge Transport
 * owner. It preserves RFB's exact-read / readiness / exact-write contract and
 * coordinates finite-session quiescence without exposing physical socket identity.
 *
 * This bridge does not parse RFB, own Transport lifecycle, choose application
 * recovery policy, or decide when a complete RFB message boundary has occurred.
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

/*
 * Finite-session quiesce process.
 *
 * requested() reports Transport's REQUEST state without consuming RFB bytes.
 * complete_quiesce_at_message_boundary() may be called only after the RFB
 * parser has reached a complete server-message boundary. It then performs the
 * required BOUNDARY -> COMMIT -> residual snapshot/discard -> COMPLETE ordering.
 * Residual discard is Transport-owned cleanup and never becomes parser credit.
 */
int pstvnc_rfb_bridge_quiesce_requested(void);
int pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(void);

#endif
