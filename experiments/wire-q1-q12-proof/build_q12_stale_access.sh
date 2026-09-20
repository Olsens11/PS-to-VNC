#!/usr/bin/env bash
# File synopsis:
# Builds the Q12 stale-access hardware proof against current product Transport
# in the pinned PS2DEV image.  No FTP or PS2 contact occurs.
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/../.." &&
    pwd
)"

BUILD_DIR="$ROOT/build/experiments/wire-q1-q12-proof/q12-stale-access"
DEP_DIR="$BUILD_DIR/deps"
ELF="$BUILD_DIR/PS2VNC-Wire-Q12-Stale-Access.ELF"

FROZEN_DEP="$ROOT/baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a"
BUILD_DEP="$DEP_DIR/libps2ip_mtu1458_wscale128.a"
EXPECTED_DEP_SHA='b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

IMAGE='ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11'
TOOLCHAIN_PATH='/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'

command -v docker >/dev/null 2>&1

python3 "$ROOT/experiments/wire-q1-q12-proof/q12_stale_access_server.py" --self-test

test "$(sha256sum "$FROZEN_DEP" | awk '{print $1}')" = "$EXPECTED_DEP_SHA"

rm -rf "$BUILD_DIR"
mkdir -p "$DEP_DIR"
cp "$FROZEN_DEP" "$BUILD_DEP"

HOST_UID="$(id -u)"
HOST_GID="$(id -g)"

timeout 180s docker run --rm \
    --entrypoint /bin/sh \
    -e HOST_UID="$HOST_UID" \
    -e HOST_GID="$HOST_GID" \
    -v "$ROOT:/repo" \
    -w /repo \
    "$IMAGE" \
    -lc "
        set -eu
        apk add --no-cache make >/dev/null
        export PATH='$TOOLCHAIN_PATH'
        make -f mk/wire-q1-q12-proof5-q12-stale-access.mk
        chown -R \"\$HOST_UID:\$HOST_GID\" build/experiments/wire-q1-q12-proof
    "

test -f "$ELF"

echo "Q12_STALE_ACCESS_BUILD=PASS"
echo "ELF_PATH=$ELF"
echo "ELF_BYTES=$(stat -c %s "$ELF")"
echo "ELF_SHA256=$(sha256sum "$ELF" | awk '{print $1}')"
"$ROOT/scripts/testkit/pt-load-fingerprint.sh" "$ELF"
