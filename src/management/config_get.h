/*
 * File synopsis:
 * Declares the read-only PS2 management operation that retrieves the human
 * configuration document as bounded raw bytes. The returned bytes remain
 * untrusted; Configuration parsing, Input binding installation and Application
 * policy are intentionally outside this owner.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * B11-READ-ONLY-HUMAN-CONFIG-GET-R31;
 * docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md.
 */

#ifndef PSTVNC_MANAGEMENT_CONFIG_GET_H
#define PSTVNC_MANAGEMENT_CONFIG_GET_H

#include <stddef.h>

#define PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES 4096u
#define PSTVNC_MANAGEMENT_HTTP_MAX_HEADER_BYTES 1024u

/*
 * Retrieve /ps2vnc.conf through the fixed private management endpoint.
 *
 * Success publishes the exact connection-close body and byte count.
 * Failure leaves body bytes and *body_length unchanged.
 */
int pstvnc_management_config_get(
    unsigned char *body,
    size_t body_capacity,
    size_t *body_length);

#endif /* PSTVNC_MANAGEMENT_CONFIG_GET_H */
