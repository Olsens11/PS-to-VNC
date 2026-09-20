/*
 * File synopsis:
 * Implements RFB's logical byte-stream bridge over one opaque Transport access
 * ticket. RFB keeps exact I/O and quiesce vocabulary while Transport alone owns
 * Wire-session validity and stale-ticket rejection.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md;
 * docs/development/module-lifecycle.md.
 */

#include "bridge.h"
#include "../transport/bridge.h"

int pstvnc_rfb_bridge_acquire(
    pstvnc_transport_access_t *transport_access)
{
    return pstvnc_transport_access_acquire(transport_access) ==
        PSTVNC_TRANSPORT_OK ? 0 : -1;
}

int pstvnc_rfb_bridge_read_exact(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t count)
{
    return pstvnc_transport_rfb_read_exact(
        transport_access, buffer, count) == PSTVNC_TRANSPORT_OK ? 0 : -1;
}

int pstvnc_rfb_bridge_poll_receive(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t result =
        pstvnc_transport_rfb_poll_receive(transport_access);
    if (result == PSTVNC_TRANSPORT_OK)
        return 1;
    if (result == PSTVNC_TRANSPORT_WOULD_BLOCK)
        return 0;
    return -1;
}

int pstvnc_rfb_bridge_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count)
{
    return pstvnc_transport_rfb_write_exact(
        transport_access, buffer, count) == PSTVNC_TRANSPORT_OK ? 0 : -1;
}

int pstvnc_rfb_bridge_quiesce_requested(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t result =
        pstvnc_transport_rfb_quiesce_requested(transport_access);
    if (result == PSTVNC_TRANSPORT_OK)
        return 1;
    if (result == PSTVNC_TRANSPORT_WOULD_BLOCK)
        return 0;
    return -1;
}

int pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(
    const pstvnc_transport_access_t *transport_access)
{
    size_t residual_count = 0u;
    size_t discarded_count = 0u;

    if (pstvnc_transport_rfb_send_quiesce_boundary(
            transport_access) != PSTVNC_TRANSPORT_OK)
        return -1;
    if (pstvnc_transport_rfb_wait_quiesce_commit(
            transport_access) != PSTVNC_TRANSPORT_OK)
        return -1;
    if (pstvnc_transport_rfb_snapshot_quiesce_residual(
            transport_access, &residual_count) != PSTVNC_TRANSPORT_OK)
        return -1;
    if (pstvnc_transport_rfb_discard_quiesce_residual(
            transport_access, residual_count, &discarded_count) !=
            PSTVNC_TRANSPORT_OK)
        return -1;
    if (discarded_count != residual_count)
        return -1;
    if (pstvnc_transport_rfb_send_quiesce_complete(
            transport_access) != PSTVNC_TRANSPORT_OK)
        return -1;
    return 0;
}
