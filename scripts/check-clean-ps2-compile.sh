#!/usr/bin/env bash
# File synopsis:
# Compile every clean Issue #7 C translation unit with the strict R5900 flags
# and pinned SDK/include expectations used by CI, without linking or deploying.
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

: "${PS2DEV:=/usr/local/ps2dev}"
: "${PS2SDK:=$PS2DEV/ps2sdk}"
: "${GSKIT:=$PS2DEV/gsKit}"

CC="$PS2DEV/ee/bin/mips64r5900el-ps2-elf-gcc"

if [ ! -x "$CC" ]; then
    echo "PS2 compiler not found: $CC" >&2
    exit 1
fi

if [ ! -d "$PS2SDK/ee/include" ] || [ ! -d "$PS2SDK/common/include" ]; then
    echo "PS2SDK headers not found under: $PS2SDK" >&2
    exit 1
fi

if [ ! -d "$GSKIT/include" ]; then
    echo "gsKit headers not found under: $GSKIT" >&2
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
    -I"$GSKIT/include"
)

SOURCES=(
    src/main.c
    src/app.c
    src/diagnostics.c
    src/diagnostics/identity.c
    src/rfb.c
    src/framebuffer.c
    src/rfb_session.c
    src/display.c
    src/platform/ps2_system.c
    src/platform/ps2_network.c
    src/platform/ps2_graphics.c
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
