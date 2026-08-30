#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

WORK="$ROOT/working/b4a"

FROZEN_DEP="$ROOT/baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a"
BUILD_DEP="$WORK/build/deps/libps2ip_mtu1458_wscale128.a"

EXPECTED_DEP_SHA='b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

IMAGE='ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11'

TOOLCHAIN_PATH='/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'

test -d "$WORK"
test -f "$WORK/Makefile"
test -f "$FROZEN_DEP"
test -f "$ROOT/src/config/text.c"
test -f "$ROOT/src/config/text.h"

ACTUAL_DEP_SHA="$(
    sha256sum "$FROZEN_DEP" |
    awk '{print $1}'
)"

test "$ACTUAL_DEP_SHA" = "$EXPECTED_DEP_SHA"

mkdir -p "$WORK/build/deps"

cp -f \
    "$FROZEN_DEP" \
    "$BUILD_DEP"

test "$(
    sha256sum "$BUILD_DEP" |
    awk '{print $1}'
)" = "$EXPECTED_DEP_SHA"

sudo rm -f \
    "$WORK"/*.o \
    "$WORK/PS2VNC.ELF" \
    "$WORK/DEV9_irx.c" \
    "$WORK/NETMAN_irx.c" \
    "$WORK/SMAP_irx.c"

sudo docker run --rm \
    --platform linux/arm64 \
    --security-opt seccomp=unconfined \
    --entrypoint /bin/sh \
    -v "$ROOT:/repo" \
    -w /repo/working/b4a \
    "$IMAGE" \
    -lc "
        set -eu
        apk add --no-cache build-base >/dev/null
        export PATH='$TOOLCHAIN_PATH'
        make
    "

sudo chown -R \
    "$(id -u):$(id -g)" \
    "$WORK"

test -f "$WORK/PS2VNC.ELF"

echo "BUILD_RESULT=PASS"
echo "ELF_PATH=$WORK/PS2VNC.ELF"
sha256sum "$WORK/PS2VNC.ELF"
