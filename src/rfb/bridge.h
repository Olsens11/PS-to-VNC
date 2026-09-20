/*
 * File synopsis:
 * Defines RFB's single cross-component byte-stream bridge to the ledge Transport
 * owner. Each ordinary RFB session carries one opaque Transport access ticket on
 * every logical operation so stale work cannot be redirected through a
 * replacement Wire Session.
 *
 * This bridge does not parse RFB, own Transport lifecycle, choose application
 * recovery policy, or decide when a complete RFB message boundary has occurred.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md;
 * docs/development/module-lifecycle.md.
 */

#ifndef PSTVNC_RFB_BRIDGE_H
#define PSTVNC_RFB_BRIDGE_H

#include "transport/transport.h"

#include <stddef.h>

int pstvnc_rfb_bridge_acquire(
    pstvnc_transport_access_t *transport_access);
int pstvnc_rfb_bridge_read_exact(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t count);
int pstvnc_rfb_bridge_poll_receive(
    const pstvnc_transport_access_t *transport_access);
int pstvnc_rfb_bridge_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count);
int pstvnc_rfb_bridge_quiesce_requested(
    const pstvnc_transport_access_t *transport_access);
int pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(
    const pstvnc_transport_access_t *transport_access);

#endif
