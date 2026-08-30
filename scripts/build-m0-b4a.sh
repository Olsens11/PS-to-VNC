#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd -P
)"

WORK="$ROOT/working/b4a"
AUTH="$ROOT/runtime/M0_BUILD_AUTHORITY.env"
FROZEN="$ROOT/baseline/frozen-b4a"

cd "$ROOT"

test -f "$AUTH"

# shellcheck disable=SC1090
source "$AUTH"

echo '===== PS-to-VNC M0 B4A CONTROLLED BUILD ====='

echo
echo '===== PRE-BUILD SOURCE AUTHORITY ====='

test "$(
    sha256sum "$WORK/ps2ip.c" |
    awk '{print $1}'
)" = "$M0_SOURCE_SHA256"

test "$(
    sha256sum "$WORK/Makefile" |
    awk '{print $1}'
)" = "$M0_MAKEFILE_SHA256"

test "$(
    sha256sum "$WORK/ps2vnc_gsHires.c" |
    awk '{print $1}'
)" = "$M0_GSHIRES_SOURCE_SHA256"

test "$(
    sha256sum "$WORK/ps2vnc_identity.c" |
    awk '{print $1}'
)" = "$M0_IDENTITY_SOURCE_SHA256"

echo 'WORKING_SOURCE_AUTHORITY=PASS'

echo
echo '===== PREPARE EXACT B4A NETWORK DEPENDENCY ====='

mkdir -p "$WORK/build/deps"

cp \
    "$FROZEN/libps2ip_mtu1458_wscale128.a" \
    "$WORK/build/deps/libps2ip_mtu1458_wscale128.a"

ACTUAL_LIB_SHA="$(
    sha256sum \
        "$WORK/build/deps/libps2ip_mtu1458_wscale128.a" |
    awk '{print $1}'
)"

echo "BUILD_LIBPS2IP_SHA256=$ACTUAL_LIB_SHA"

test "$ACTUAL_LIB_SHA" = "$M0_B4A_LIBPS2IP_SHA256"

echo 'B4A_NETWORK_DEPENDENCY=PASS'

echo
echo '===== REQUIRE EXACT PINNED DOCKER IMAGE ====='

IMAGE_ID="$(
    sudo docker image inspect \
        "$M0_DOCKER_IMAGE" \
        --format '{{.Id}}'
)"

echo "DOCKER_IMAGE=$M0_DOCKER_IMAGE"
echo "DOCKER_IMAGE_ID=$IMAGE_ID"

test "$IMAGE_ID" = "$M0_DOCKER_LOCAL_IMAGE_ID"

echo 'PINNED_DOCKER_IMAGE=PASS'

echo
echo '===== BUILD ====='

sudo docker run \
    --rm \
    --platform linux/arm64 \
    --security-opt seccomp=unconfined \
    -v "$WORK:/work" \
    -w /work \
    "$M0_DOCKER_IMAGE" \
    sh -c 'apk add --no-cache build-base >/dev/null && make clean && make'

sudo chown "$(id -u):$(id -g)" \
    "$WORK/PS2VNC.ELF" \
    "$WORK"/*.o \
    "$WORK/DEV9_irx.c" \
    "$WORK/NETMAN_irx.c" \
    "$WORK/SMAP_irx.c"

echo
echo '===== REFERENCE LADDER ====='

compare_file()
{
    local label="$1"
    local file="$2"
    local expected_sha="$3"

    local actual_sha
    local actual_bytes

    actual_sha="$(
        sha256sum "$file" |
        awk '{print $1}'
    )"

    actual_bytes="$(
        wc -c < "$file"
    )"

    echo "$label=$actual_sha"
    echo "${label}_BYTES=$actual_bytes"

    if [ "$actual_sha" = "$expected_sha" ]; then
        echo "${label}_MATCH=YES"
    else
        echo "${label}_MATCH=NO"
    fi
}

compare_file \
    'DEV9_IRX_C_SHA256' \
    "$WORK/DEV9_irx.c" \
    "$M0_REF_DEV9_IRX_C_SHA256"

compare_file \
    'DEV9_IRX_O_SHA256' \
    "$WORK/DEV9_irx.o" \
    "$M0_REF_DEV9_IRX_O_SHA256"

compare_file \
    'NETMAN_IRX_C_SHA256' \
    "$WORK/NETMAN_irx.c" \
    "$M0_REF_NETMAN_IRX_C_SHA256"

compare_file \
    'NETMAN_IRX_O_SHA256' \
    "$WORK/NETMAN_irx.o" \
    "$M0_REF_NETMAN_IRX_O_SHA256"

compare_file \
    'SMAP_IRX_C_SHA256' \
    "$WORK/SMAP_irx.c" \
    "$M0_REF_SMAP_IRX_C_SHA256"

compare_file \
    'SMAP_IRX_O_SHA256' \
    "$WORK/SMAP_irx.o" \
    "$M0_REF_SMAP_IRX_O_SHA256"

compare_file \
    'PS2IP_O_SHA256' \
    "$WORK/ps2ip.o" \
    "$M0_REF_PS2IP_O_SHA256"

compare_file \
    'IDENTITY_O_SHA256' \
    "$WORK/ps2vnc_identity.o" \
    "$M0_REF_IDENTITY_O_SHA256"

compare_file \
    'GSHIRES_O_SHA256' \
    "$WORK/ps2vnc_gsHires.o" \
    "$M0_REF_GSHIRES_O_SHA256"

compare_file \
    'ELF_SHA256' \
    "$WORK/PS2VNC.ELF" \
    "$M0_REFERENCE_ELF_SHA256"

ELF_SHA="$(
    sha256sum "$WORK/PS2VNC.ELF" |
    awk '{print $1}'
)"

ELF_BYTES="$(
    wc -c < "$WORK/PS2VNC.ELF"
)"

echo
echo '===== CLASSIFICATION ====='

if [ "$ELF_SHA" = "$M0_REFERENCE_ELF_SHA256" ] &&
   [ "$ELF_BYTES" -eq "$M0_REFERENCE_ELF_BYTES" ]
then
    echo 'M0_BUILD_CLASSIFICATION=OUTCOME_A_BYTE_EXACT'
else
    echo 'M0_BUILD_CLASSIFICATION=NON_IDENTICAL_REQUIRES_ANALYSIS'
fi

echo
echo '===== FINAL ====='
echo 'M0_CONTROLLED_BUILD=COMPLETE'
echo "ELF_SHA256=$ELF_SHA"
echo "ELF_BYTES=$ELF_BYTES"
