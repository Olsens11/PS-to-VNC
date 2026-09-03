/*
 * File synopsis:
 * Implements deterministic DUT identity emission using a stampable ELF blob
 * and a linker sendto wrapper before the first diagnostic datagram.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "Clean diagnostics and
 * deterministic runtime identity"; docs/reconstruction/ISSUE7_DUT_MANIFEST.md.
 */

/*
 * PS2VNC runtime ELF identity transport.
 *
 * This file deliberately does not depend on experiment-specific ps2ip.c
 * source. The linker wraps sendto(). Immediately before the first
 * diagnostic UDP datagram to port 5999, the wrapper emits one identity
 * datagram using the same socket and destination.
 *
 * The identity blob is fixed-size so TestKit can stamp it after link
 * without changing ELF layout or addresses.
 *
 * Runtime message construction deliberately avoids printf-family formatting.
 * Hardware evidence showed the target formatter path truncating long identity
 * payloads to 106 bytes. Identity is infrastructure, so serialize it with
 * exact bounded byte copies instead.
 */

#include "diagnostics/identity.h"

static size_t pstvnc_identity_bounded_length(
    const char *text,
    size_t limit)
{
    size_t length;

    if (text == NULL)
        return limit + 1;

    for (length = 0; length < limit; length++) {
        if (text[length] == '\0')
            return length;
    }

    return limit + 1;
}

static int pstvnc_identity_append(
    char *message,
    size_t capacity,
    size_t *length,
    const char *source,
    size_t source_length)
{
    size_t index;

    if (message == NULL ||
        length == NULL ||
        source == NULL)
        return 0;

    if (*length > capacity)
        return 0;

    if (source_length > capacity - *length)
        return 0;

    for (index = 0; index < source_length; index++)
        message[*length + index] = source[index];

    *length += source_length;

    return 1;
}

static int pstvnc_identity_is_hex(char value)
{
    return
        (value >= '0' && value <= '9') ||
        (value >= 'a' && value <= 'f') ||
        (value >= 'A' && value <= 'F');
}

size_t pstvnc_diagnostics_identity_format_message(
    char *message,
    size_t capacity,
    const char *test_id,
    const char *digest)
{
    static const char prefix[] =
        "PS2VNC_ID version=1 test=";

    static const char separator[] =
        " digest=";

    size_t test_id_length;
    size_t digest_length;
    size_t length = 0;
    size_t index;

    if (message == NULL ||
        capacity == 0 ||
        test_id == NULL ||
        digest == NULL)
        return 0;

    message[0] = '\0';

    /*
     * The stamped blob owns char test_id[64], so a valid runtime value has
     * at most 63 visible bytes followed by NUL.
     */
    test_id_length =
        pstvnc_identity_bounded_length(test_id, 64);

    if (test_id_length == 0 ||
        test_id_length > 63)
        return 0;

    /*
     * The stamped digest field owns exactly 64 visible hex characters plus
     * its terminating NUL.
     */
    digest_length =
        pstvnc_identity_bounded_length(digest, 65);

    if (digest_length != 64)
        return 0;

    for (index = 0; index < digest_length; index++) {
        if (!pstvnc_identity_is_hex(digest[index]))
            return 0;
    }

    if (!pstvnc_identity_append(
            message,
            capacity,
            &length,
            prefix,
            sizeof(prefix) - 1))
        return 0;

    if (!pstvnc_identity_append(
            message,
            capacity,
            &length,
            test_id,
            test_id_length))
        return 0;

    if (!pstvnc_identity_append(
            message,
            capacity,
            &length,
            separator,
            sizeof(separator) - 1))
        return 0;

    if (!pstvnc_identity_append(
            message,
            capacity,
            &length,
            digest,
            digest_length))
        return 0;

    /*
     * The UDP payload itself does not need NUL, but keeping the local buffer
     * NUL-terminated makes the serialization contract directly testable and
     * safe for diagnostics.
     */
    if (length >= capacity) {
        message[0] = '\0';
        return 0;
    }

    message[length] = '\0';

    return length;
}

#ifndef PSTVNC_DIAGNOSTICS_IDENTITY_FORMAT_TEST

#include <ps2ip.h>

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
    PSTVNC_DIAGNOSTICS_IDENTITY_MAGIC,
    "UNSTAMPED",
    PS2VNC_IDENTITY_ZERO64
};

static int ps2vnc_identity_sent = 0;

/*
 * GNU ld --wrap redirects calls to sendto() through these symbols.
 *
 * Derive both linker-alias declarations directly from the sendto prototype
 * supplied by PS2SDK <sys/socket.h>. This keeps the aliases type-identical
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

        if (ntohs(in->sin_port) ==
            PSTVNC_DIAGNOSTICS_IDENTITY_UDP_PORT) {

            char message[192];

            size_t message_length =
                pstvnc_diagnostics_identity_format_message(
                    message,
                    sizeof(message),
                    ps2vnc_identity_blob.test_id,
                    ps2vnc_identity_blob.digest);

            if (message_length > 0) {
                ssize_t rc;

                rc = __real_sendto(
                    sock,
                    message,
                    message_length,
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

#endif
