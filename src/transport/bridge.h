/*
 * File synopsis:
 * Defines Transport's single cross-component bridge. The bridge exposes the
 * application-requested session lifecycle and the logical RFB byte-stream /
 * quiesce processes while keeping the physical PSTV descriptor and receiver
 * runtime private to Transport.
 *
 * The bridge does not parse RFB, invent configuration defaults, decide the
 * application's complete-RFB-message safe boundary, or perform recovery policy.
 *
 * Context: docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md; docs/ledge/
 * LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#ifndef PSTVNC_TRANSPORT_BRIDGE_H
#define PSTVNC_TRANSPORT_BRIDGE_H

#include "transport.h"

#include <stddef.h>

/*
 * Application lifecycle process.
 *
 * On success, ownership of *socket_fd transfers permanently to Transport and
 * *socket_fd is set to -1. A failure before adoption leaves the caller's value
 * unchanged. A failure after adoption consumes/closes the descriptor and also
 * leaves *socket_fd at -1, preventing ambiguous double-close ownership.
 */
pstvnc_transport_result_t pstvnc_transport_session_open(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config);

/*
 * Application-local fatal convergence. Transport first requests its sole
 * receiver to stop and interrupts its own blocking physical I/O, then waits for
 * receiver completion, and only then reclaims the session. This is deliberately
 * distinct from the server-driven finite-RFB quiesce process below.
 */
pstvnc_transport_result_t pstvnc_transport_session_abort(void);

/* Wait for explicit sole-receiver completion; this is not timer polling. */
pstvnc_transport_result_t pstvnc_transport_session_wait_receiver_done(void);

/*
 * Release Transport-owned session resources only after receiver completion.
 * WOULD_BLOCK means receiver-visible state is still live and nothing is freed.
 */
pstvnc_transport_result_t pstvnc_transport_session_close(void);

/* Logical RFB delivery process. No operation exposes the physical descriptor. */
pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    void *buffer,
    size_t count);
pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(void);
pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const void *buffer,
    size_t count);

/*
 * Ordered finite-RFB quiesce process.
 *
 * REQUEST detection does not choose the safe boundary. The application/RFB
 * owner must call send_quiesce_boundary() only after reaching a complete RFB
 * message boundary. Residual discard is explicit and never becomes parser
 * consumption credit.
 */
pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(void);
pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(void);
pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(void);
pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    size_t *residual_count);
pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    size_t expected_count,
    size_t *discarded_count);
pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(void);

#endif