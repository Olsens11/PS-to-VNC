#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

TMP="$(mktemp -d)"

cleanup()
{
    rm -rf "$TMP"
}

trap cleanup EXIT

cc \
    -std=c11 \
    -Wall \
    -Wextra \
    -Werror \
    -I"$ROOT/src" \
    "$ROOT/src/rfb_protocol.c" \
    "$ROOT/tests/unit/rfb_protocol_test.c" \
    -o "$TMP/rfb_protocol_test"

"$TMP/rfb_protocol_test"

echo 'RECONSTRUCTION_M1_HOST_TESTS=PASS'
