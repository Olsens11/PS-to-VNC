#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

BUILD_DIR="$ROOT/build/reconstruction/issue7"
DEP_DIR="$BUILD_DIR/deps"
ELF="$BUILD_DIR/PS-to-VNC-Issue7.ELF"

FROZEN_DEP="$ROOT/baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a"
BUILD_DEP="$DEP_DIR/libps2ip_mtu1458_wscale128.a"
EXPECTED_DEP_SHA='b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

IMAGE='ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11'
TOOLCHAIN_PATH='/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'

if ! command -v docker >/dev/null 2>&1; then
    echo 'ERROR: docker is required for the pinned clean Issue #7 build.' >&2
    exit 1
fi

if [ ! -f "$FROZEN_DEP" ]; then
    echo "ERROR: qualified PS2IP archive not found: $FROZEN_DEP" >&2
    exit 1
fi

actual_dep_sha="$(sha256sum "$FROZEN_DEP" | awk '{print $1}')"
if [ "$actual_dep_sha" != "$EXPECTED_DEP_SHA" ]; then
    echo 'ERROR: qualified PS2IP archive identity mismatch.' >&2
    echo "EXPECTED=$EXPECTED_DEP_SHA" >&2
    echo "ACTUAL=$actual_dep_sha" >&2
    exit 1
fi

rm -rf "$BUILD_DIR"
mkdir -p "$DEP_DIR"
cp "$FROZEN_DEP" "$BUILD_DEP"

test "$(sha256sum "$BUILD_DEP" | awk '{print $1}')" = "$EXPECTED_DEP_SHA"

HOST_UID="$(id -u)"
HOST_GID="$(id -g)"

docker run --rm \
    --entrypoint /bin/sh \
    -e HOST_UID="$HOST_UID" \
    -e HOST_GID="$HOST_GID" \
    -v "$ROOT:/repo" \
    -w /repo \
    "$IMAGE" \
    -lc "
        set -eu
        apk add --no-cache bash build-base >/dev/null
        export PATH='$TOOLCHAIN_PATH'
        make -f mk/issue7-clean.mk
        chown -R \"\$HOST_UID:\$HOST_GID\" build/reconstruction/issue7
    "

test -f "$ELF"

"$ROOT/scripts/check-issue7-identity-blob.py" "$ELF"
"$ROOT/scripts/testkit/pt-load-fingerprint.sh" "$ELF"

echo 'ISSUE7_LINKED_BUILD=PASS'
echo 'ISSUE7_RUNTIME_IDENTITY_LINKED=YES'
echo "PS2IP_SHA256=$EXPECTED_DEP_SHA"
echo "ELF_PATH=$ELF"
sha256sum "$ELF"
