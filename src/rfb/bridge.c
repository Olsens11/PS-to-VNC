/*
 * File synopsis:
 * Implements RFB's one cross-component bridge to Transport's logical channel-1
 * byte stream. It translates Transport's richer result vocabulary into RFB's
 * intentionally small exact-read / readiness / exact-write I/O contract.
 *
 * No physical socket identity crosses this file. Transport lifecycle and
 * complete-RFB-message safe-boundary policy remain with their documented owners.
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
