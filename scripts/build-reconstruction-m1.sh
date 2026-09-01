#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

BUILD_DIR="$ROOT/build/reconstruction/m1a"
DEP_DIR="$BUILD_DIR/deps"
ELF="$BUILD_DIR/PS-to-VNC-M1A.ELF"

FROZEN_DEP="$ROOT/baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a"
BUILD_DEP="$DEP_DIR/libps2ip_mtu1458_wscale128.a"
EXPECTED_DEP_SHA='b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

IMAGE='ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11'
TOOLCHAIN_PATH='/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'

"$ROOT/scripts/test-reconstruction-m1-host.sh"

test -f "$FROZEN_DEP"

actual_dep_sha="$(
    sha256sum "$FROZEN_DEP" |
    awk '{print $1}'
)"

test "$actual_dep_sha" = "$EXPECTED_DEP_SHA"

sudo rm -rf "$BUILD_DIR"
mkdir -p "$DEP_DIR"
cp "$FROZEN_DEP" "$BUILD_DEP"

test "$(
    sha256sum "$BUILD_DEP" |
    awk '{print $1}'
)" = "$EXPECTED_DEP_SHA"

sudo docker run --rm \
    --platform linux/arm64 \
    --security-opt seccomp=unconfined \
    --entrypoint /bin/sh \
    -v "$ROOT:/repo" \
    -w /repo \
    "$IMAGE" \
    -lc "
        set -eu
        apk add --no-cache build-base >/dev/null
        export PATH='$TOOLCHAIN_PATH'
        make -f mk/reconstruction-m1.mk
    "

sudo chown -R \
    "$(id -u):$(id -g)" \
    "$BUILD_DIR"

test -f "$ELF"

echo 'RECONSTRUCTION_MILESTONE=M1A'
echo 'BUILD_RESULT=PASS'
echo "ELF_PATH=$ELF"
echo "PS2IP_SHA256=$EXPECTED_DEP_SHA"
sha256sum "$ELF"
