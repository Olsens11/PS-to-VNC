/*
 * PS2VNC runtime ELF identity transport.
 *
 * This file deliberately does not depend on experiment-specific ps2ip.c
 * source.  The linker wraps sendto().  Immediately before the first
 * diagnostic UDP datagram to port 5999, the wrapper emits one identity
 * datagram using the same socket and destination.
 *
 * The identity blob is fixed-size so TestKit can stamp it after link
 * without changing ELF layout or addresses.
 */

#include <stdio.h>
#include <ps2ip.h>

#define PS2VNC_IDENTITY_UDP_PORT 5999

#define PS2VNC_IDENTITY_ZERO64 \
    "0000000000000000" \
    "0000000000000000" \
    "0000000000000000" \
    "0000000000000000"

struct __attribute__((packed)) ps2vnc_identity_blob_v1 {
    char magic[17];
    char test_id[64];
    char digest[65];
};

static const struct ps2vnc_identity_blob_v1
ps2vnc_identity_blob __attribute__((used)) = {
    "PS2VNCIDv1!BLOB!",
    "UNSTAMPED",
    PS2VNC_IDENTITY_ZERO64
};

static int ps2vnc_identity_sent = 0;

/*
 * GNU ld --wrap redirects calls to sendto() through these symbols.
 *
 * Derive both linker-alias declarations directly from the sendto prototype
 * supplied by PS2SDK <sys/socket.h>.  This keeps the aliases type-identical
 * to the SDK interface and makes the compiler reject wrapper drift.
 */
extern __typeof__(sendto) __real_sendto;
extern __typeof__(sendto) __wrap_sendto;

ssize_t __wrap_sendto(
    int sock,
    const void *data,
    size_t len,
    int flags,
    const struct sockaddr *to,
    socklen_t tolen)
{
    if (!ps2vnc_identity_sent &&
        to != NULL &&
        tolen >= (int)sizeof(struct sockaddr_in) &&
        to->sa_family == AF_INET) {

        const struct sockaddr_in *in =
            (const struct sockaddr_in *)to;

        if (ntohs(in->sin_port) == PS2VNC_IDENTITY_UDP_PORT) {
            char msg[192];
            int msg_len;

            msg_len = snprintf(
                msg,
                sizeof(msg),
                "PS2VNC_ID version=1 test=%s digest=%s",
                ps2vnc_identity_blob.test_id,
                ps2vnc_identity_blob.digest
            );

            if (msg_len > 0) {
                ssize_t rc;

                if (msg_len >= (int)sizeof(msg))
                    msg_len = sizeof(msg) - 1;

                rc = __real_sendto(
                    sock,
                    msg,
                    msg_len,
                    flags,
                    to,
                    tolen
                );

                if (rc >= 0)
                    ps2vnc_identity_sent = 1;
            }
        }
    }

    return __real_sendto(
        sock,
        data,
        len,
        flags,
        to,
        tolen
    );
}
