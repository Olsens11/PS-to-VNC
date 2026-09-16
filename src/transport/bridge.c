/*
 * File synopsis:
 * Implements Transport's one cross-component bridge body. It coordinates the
 * application-owned session lifecycle and adapts RFB's logical byte-stream /
 * finite-quiesce process to the private Transport runtime without exposing the
 * physical PSTV descriptor or moving protocol policy into Transport.
 *
 * The single-session storage matches the product's one active PSTV connection.
 * Complete-RFB-message safe-boundary choice remains with application/RFB; this
 * bridge only executes the ordered Transport operations once asked.
 *
 * Context: docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md; docs/ledge/
 * LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include "bridge.h"
#include "runtime.h"

/* One product PSTV session is active at a time. */
static pstvnc_transport_runtime_t pstvnc_transport_bridge_runtime;
static int pstvnc_transport_bridge_session_active;

static pstvnc_transport_result_t pstvnc_transport_bridge_terminal_result(void)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_bridge_runtime.failed)
        return PSTVNC_TRANSPORT_FAILED;

    if (pstvnc_transport_bridge_runtime.receiver_done)
        return PSTVNC_TRANSPORT_CLOSED;

    return PSTVNC_TRANSPORT_FAILED;
}

/* ------------------------------------------------------------------------- */
/* Application session lifecycle process.                                    */
/* ------------------------------------------------------------------------- */

pstvnc_transport_result_t pstvnc_transport_session_open(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config)
{
    if (socket_fd == NULL || *socket_fd < 0 || config == NULL ||
        pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    /*
     * runtime_initialize() adopts the descriptor only as its final successful
     * initialization step. Therefore a false result leaves caller ownership
     * unambiguous and *socket_fd untouched.
     */
    if (!pstvnc_transport_runtime_initialize(
            &pstvnc_transport_bridge_runtime,
            *socket_fd,
            config))
        return PSTVNC_TRANSPORT_FAILED;

    /* From this point onward the caller must never close this descriptor. */
    *socket_fd = -1;

    if (!pstvnc_transport_runtime_start_receiver(
            &pstvnc_transport_bridge_runtime)) {
        /* No receiver thread is live after a failed start; retire the adopted
         * descriptor/resources here so failed open has one owner and one close.
         */
        (void)pstvnc_transport_runtime_release(
            &pstvnc_transport_bridge_runtime);
        return PSTVNC_TRANSPORT_FAILED;
    }

    pstvnc_transport_bridge_session_active = 1;
    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_session_abort(void)
{
    int released;

    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    /*
     * The stop request remains inside Transport: it publishes receiver stop
     * intent before shutdown() interrupts Transport's privately owned socket.
     * No resource is reclaimed until the normal completion event proves the
     * sole receiver can no longer touch session state.
     */
    if (!pstvnc_transport_runtime_request_stop(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_runtime_wait_receiver_done(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_FAILED;

    released = pstvnc_transport_runtime_release(
        &pstvnc_transport_bridge_runtime);
    pstvnc_transport_bridge_session_active = 0;

    return released
        ? PSTVNC_TRANSPORT_OK
        : PSTVNC_TRANSPORT_FAILED;
}

pstvnc_transport_result_t pstvnc_transport_session_wait_receiver_done(void)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    /*
     * Success means only that receiver completion is proven. The session may
     * still have failed; callers retain product-level failure classification.
     */
    if (!pstvnc_transport_runtime_wait_receiver_done(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_session_close(void)
{
    int released;

    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_bridge_runtime.receiver_thread_started &&
        !pstvnc_transport_bridge_runtime.receiver_done)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    released = pstvnc_transport_runtime_release(
        &pstvnc_transport_bridge_runtime);
    pstvnc_transport_bridge_session_active = 0;

    return released
        ? PSTVNC_TRANSPORT_OK
        : PSTVNC_TRANSPORT_FAILED;
}

/* ------------------------------------------------------------------------- */
/* Logical RFB byte-stream delivery process.                                 */
/* ------------------------------------------------------------------------- */

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    void *buffer,
    size_t count)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_read_exact(
            &pstvnc_transport_bridge_runtime,
            buffer,
            count))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(void)
{
    int result;

    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    result = pstvnc_transport_runtime_rfb_poll_receive(
        &pstvnc_transport_bridge_runtime);

    if (result > 0)
        return PSTVNC_TRANSPORT_OK;
    if (result == 0)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const void *buffer,
    size_t count)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_write_exact(
            &pstvnc_transport_bridge_runtime,
            buffer,
            count))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

/* ------------------------------------------------------------------------- */
/* Ordered logical-RFB finite-session quiesce process.                       */
/* ------------------------------------------------------------------------- */

pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(void)
{
    int requested;

    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    requested = pstvnc_transport_runtime_rfb_quiesce_requested(
        &pstvnc_transport_bridge_runtime);

    if (requested > 0)
        return PSTVNC_TRANSPORT_OK;
    if (requested == 0)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(void)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_send_quiesce_boundary(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(void)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_wait_quiesce_commit(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    size_t *residual_count)
{
    if (!pstvnc_transport_bridge_session_active || residual_count == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_snapshot_residual(
            &pstvnc_transport_bridge_runtime,
            residual_count))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    size_t expected_count,
    size_t *discarded_count)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_discard_quiesce_residual(
            &pstvnc_transport_bridge_runtime,
            expected_count,
            discarded_count))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(void)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_send_quiesce_complete(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}