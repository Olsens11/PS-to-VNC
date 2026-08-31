#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd
)"

TMP="$(
    mktemp -d \
        /tmp/ps-to-vnc-identity-message-selftest.XXXXXX
)"

cleanup()
{
    rm -rf "$TMP"
}

trap cleanup EXIT

cat > "$TMP/test.c" <<'C'
#include "diagnostics/identity.h"

#include <stdio.h>
#include <string.h>

static int require_message(
    const char *test_id,
    const char *digest,
    const char *expected)
{
    char message[192];

    size_t length =
        pstvnc_diagnostics_identity_format_message(
            message,
            sizeof(message),
            test_id,
            digest);

    if (length != strlen(expected))
        return 0;

    if (strcmp(message, expected) != 0)
        return 0;

    if (message[length] != '\0')
        return 0;

    return 1;
}

int main(void)
{
    static const char digest[] =
        "28dbe5d985cfcbdcc2721e10e8b816fc"
        "fe37eb1e20e0b4d0e97c217299c515b8";

    static const char current_expected[] =
        "PS2VNC_ID version=1 test=M4I-C2-HW1 "
        "digest="
        "28dbe5d985cfcbdcc2721e10e8b816fc"
        "fe37eb1e20e0b4d0e97c217299c515b8";

    static const char historical_expected[] =
        "PS2VNC_ID version=1 test=T15D2B2BT30 "
        "digest="
        "28dbe5d985cfcbdcc2721e10e8b816fc"
        "fe37eb1e20e0b4d0e97c217299c515b8";

    char max_test_id[64];
    char max_expected[192];
    char exact[108];
    char too_small[107];

    size_t length;
    int index;

    if (strlen(digest) != 64)
        return 10;

    if (!require_message(
            "M4I-C2-HW1",
            digest,
            current_expected))
        return 11;

    if (strlen(current_expected) != 107)
        return 12;

    if (!require_message(
            "T15D2B2BT30",
            digest,
            historical_expected))
        return 13;

    if (strlen(historical_expected) != 108)
        return 14;

    for (index = 0; index < 63; index++)
        max_test_id[index] = 'A';

    max_test_id[63] = '\0';

    snprintf(
        max_expected,
        sizeof(max_expected),
        "PS2VNC_ID version=1 test=%s digest=%s",
        max_test_id,
        digest);

    if (!require_message(
            max_test_id,
            digest,
            max_expected))
        return 15;

    if (strlen(max_expected) != 160)
        return 16;

    length =
        pstvnc_diagnostics_identity_format_message(
            exact,
            sizeof(exact),
            "M4I-C2-HW1",
            digest);

    if (length != 107)
        return 17;

    length =
        pstvnc_diagnostics_identity_format_message(
            too_small,
            sizeof(too_small),
            "M4I-C2-HW1",
            digest);

    if (length != 0)
        return 18;

    length =
        pstvnc_diagnostics_identity_format_message(
            exact,
            sizeof(exact),
            "M4I-C2-HW1",
            "1234");

    if (length != 0)
        return 19;

    length =
        pstvnc_diagnostics_identity_format_message(
            exact,
            sizeof(exact),
            "M4I-C2-HW1",
            "28dbe5d985cfcbdcc2721e10e8b816fc"
            "fe37eb1e20e0b4d0e97c217299c515bg");

    if (length != 0)
        return 20;

    puts(
        "IDENTITY_CURRENT_107_BYTE_MESSAGE=PASS"
    );

    puts(
        "IDENTITY_HISTORICAL_108_BYTE_MESSAGE=PASS"
    );

    puts(
        "IDENTITY_MAX_TEST_ID_160_BYTE_MESSAGE=PASS"
    );

    puts(
        "IDENTITY_EXACT_64_HEX_DIGEST=PASS"
    );

    puts(
        "IDENTITY_INSUFFICIENT_CAPACITY_FAIL_CLOSED=PASS"
    );

    puts(
        "IDENTITY_MALFORMED_DIGEST_FAIL_CLOSED=PASS"
    );

    puts(
        "PS_TO_VNC_IDENTITY_RUNTIME_MESSAGE_SELF_TEST=PASS"
    );

    return 0;
}
C

cc \
    -std=c99 \
    -Wall \
    -Wextra \
    -Werror \
    -DPSTVNC_DIAGNOSTICS_IDENTITY_FORMAT_TEST \
    -I"$ROOT/src" \
    "$ROOT/src/diagnostics/identity.c" \
    "$TMP/test.c" \
    -o "$TMP/test"

"$TMP/test"
