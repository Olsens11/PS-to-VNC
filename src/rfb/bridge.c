/*
 * File synopsis:
 * Implements RFB's one cross-component bridge to Transport's logical channel-1
 * byte stream. It translates Transport's richer result vocabulary into RFB's
 * exact I/O vocabulary and coordinates the ordered finite-session quiesce
 * sequence once the RFB parser has selected a complete-message boundary.
 *
 * No physical socket identity crosses this file. Transport lifecycle remains
 * Transport-owned and complete-RFB-message boundary choice remains RFB-owned.
 *
 * Context: docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md, "RFB ownership under
 * shared transport"; docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include "bridge.h"

#include "../transport/bridge.h"

int pstvnc_rfb_bridge_read_exact(
    void *buffer,
    size_t count)
{
    return pstvnc_transport_rfb_read_exact(buffer, count) ==
        PSTVNC_TRANSPORT_OK ? 0 : -1;
}

int pstvnc_rfb_bridge_poll_receive(void)
{
    pstvnc_transport_result_t result =
        pstvnc_transport_rfb_poll_receive();

    if (result == PSTVNC_TRANSPORT_OK)
        return 1;
    if (result == PSTVNC_TRANSPORT_WOULD_BLOCK)
        return 0;

    return -1;
}

int pstvnc_rfb_bridge_write_exact(
    const void *buffer,
    size_t count)
{
    return pstvnc_transport_rfb_write_exact(buffer, count) ==
        PSTVNC_TRANSPORT_OK ? 0 : -1;
}

int pstvnc_rfb_bridge_quiesce_requested(void)
{
    pstvnc_transport_result_t result =
        pstvnc_transport_rfb_quiesce_requested();

    if (result == PSTVNC_TRANSPORT_OK)
        return 1;
    if (result == PSTVNC_TRANSPORT_WOULD_BLOCK)
        return 0;

    return -1;
}

int pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(void)
{
    size_t residual_count = 0;
    size_t discarded_count = 0;

    /*
     * The caller is the RFB parser and therefore owns proof that no partial
     * server message has been consumed. From that proven boundary onward the
     * bridge owns only cross-component ordering, not protocol interpretation.
     */
    if (pstvnc_transport_rfb_send_quiesce_boundary() !=
        PSTVNC_TRANSPORT_OK)
        return -1;

    if (pstvnc_transport_rfb_wait_quiesce_commit() !=
        PSTVNC_TRANSPORT_OK)
        return -1;

    if (pstvnc_transport_rfb_snapshot_quiesce_residual(
            &residual_count) != PSTVNC_TRANSPORT_OK)
        return -1;

    if (pstvnc_transport_rfb_discard_quiesce_residual(
            residual_count,
            &discarded_count) != PSTVNC_TRANSPORT_OK)
        return -1;

    if (discarded_count != residual_count)
        return -1;

    if (pstvnc_transport_rfb_send_quiesce_complete() !=
        PSTVNC_TRANSPORT_OK)
        return -1;

    return 0;
}
