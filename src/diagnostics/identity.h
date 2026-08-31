#ifndef PSTVNC_DIAGNOSTICS_IDENTITY_H
#define PSTVNC_DIAGNOSTICS_IDENTITY_H

#include <stddef.h>

/*
 * Stable build/runtime identity transport contract.
 *
 * Identity is emitted through the linker sendto() wrapper immediately before
 * the first diagnostic UDP datagram. The fixed blob layout is intentionally
 * discoverable and stampable by the established hardware TestKit.
 *
 * Runtime message serialization is deterministic and does not depend on
 * printf-family formatter return semantics.
 *
 * This module owns identity transport only. It exposes no application-domain
 * state or product-policy API.
 */

#define PSTVNC_DIAGNOSTICS_IDENTITY_UDP_PORT 5999
#define PSTVNC_DIAGNOSTICS_IDENTITY_MAGIC "PS2VNCIDv1!BLOB!"

/*
 * Serialize:
 *
 *     PS2VNC_ID version=1 test=<test_id> digest=<64 hex chars>
 *
 * Returns the exact byte count excluding the terminating NUL.
 * Returns 0 on invalid input or insufficient output capacity.
 */
size_t pstvnc_diagnostics_identity_format_message(
    char *message,
    size_t capacity,
    const char *test_id,
    const char *digest);

#endif
