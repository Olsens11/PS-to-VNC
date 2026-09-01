#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

: "${PS2DEV:=/usr/local/ps2dev}"
: "${PS2SDK:=$PS2DEV/ps2sdk}"

CC="$PS2DEV/ee/bin/mips64r5900el-ps2-elf-gcc"

if [ ! -x "$CC" ]; then
    echo "PS2 compiler not found: $CC" >&2
    exit 1
fi

if [ ! -d "$PS2SDK/ee/include" ] || [ ! -d "$PS2SDK/common/include" ]; then
    echo "PS2SDK headers not found under: $PS2SDK" >&2
    exit 1
fi

BUILD_DIR="$(mktemp -d)"
trap 'rm -rf "$BUILD_DIR"' EXIT

COMMON_FLAGS=(
    -D_EE
    -G0
    -march=r5900
    -mhard-float
    -msingle-float
    -mno-llsc
    -mno-shared
    -mplt
    -O2
    -g
    -std=c99
    -Wall
    -Wextra
    -Werror
    -I"$ROOT/src"
    -I"$ROOT/src/platform"
    -I"$PS2SDK/ee/include"
    -I"$PS2SDK/common/include"
)

SOURCES=(
    src/rfb.c
    src/framebuffer.c
    src/rfb_session.c
    src/platform/ps2_system.c
    src/platform/ps2_network.c
)

for source in "${SOURCES[@]}"; do
    object="$BUILD_DIR/$(basename "${source%.c}").o"
    echo "PS2_COMPILE=$source"
    "$CC" \
        "${COMMON_FLAGS[@]}" \
        -c "$ROOT/$source" \
        -o "$object"
done

echo 'CLEAN_PS2_COMPILE_CHECK=PASS'
