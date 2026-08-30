#ifndef PSTVNC_DIAGNOSTICS_IDENTITY_H
#define PSTVNC_DIAGNOSTICS_IDENTITY_H

/*
 * Stable build/runtime identity transport contract.
 *
 * Identity is emitted through the linker sendto() wrapper immediately before
 * the first diagnostic UDP datagram.  The fixed blob layout is intentionally
 * discoverable and stampable by the established hardware TestKit.
 *
 * This module owns identity transport only.  It exposes no application-domain
 * state or product-policy API.
 */

#define PSTVNC_DIAGNOSTICS_IDENTITY_UDP_PORT 5999
#define PSTVNC_DIAGNOSTICS_IDENTITY_MAGIC "PS2VNCIDv1!BLOB!"

#endif
