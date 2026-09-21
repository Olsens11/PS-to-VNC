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
SMS_VENDOR="$ROOT/vendor/sms-libmpeg"
SMS_INC="$SMS_VENDOR/include"
SMS_SRC="$SMS_VENDOR/src"

if [ ! -x "$CC" ]; then
    echo "PS2 compiler not found: $CC" >&2
    echo 'REQUIRED_CONTEXT=PINNED_PS2DEV_CONTAINER_OR_EQUIVALENT_HOST_TOOLCHAIN' >&2
    echo 'CLASSIFICATION=DEVELOPMENT_ENVIRONMENT_PRECONDITION' >&2
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
    -I"$SMS_INC"
    -I"$ROOT/src"
    -I"$ROOT/src/audio"
    -I"$ROOT/src/config"
    -I"$ROOT/src/media"
    -I"$ROOT/src/mpeg"
    -I"$ROOT/src/input"
    -I"$ROOT/src/ui"
    -I"$ROOT/src/rfb"
    -I"$ROOT/src/framebuffer"
    -I"$ROOT/src/display"
    -I"$ROOT/src/diagnostics"
    -I"$ROOT/src/platform"
    -I"$ROOT/src/transport"
    -I"$PS2SDK/ee/include"
    -I"$PS2SDK/common/include"
    -I"$GSKIT/include"
)

SOURCES=(
    src/main.c
    src/app.c
    src/audio/playback.c
    src/audio/audsrv_service.c
    src/audio/session.c
    src/config/profile.c
    src/config/text.c
    src/media/clock.c
    src/mpeg/decoder.c
    src/mpeg/ps2_decoder_backend.c
    src/diagnostics/diagnostics.c
    src/diagnostics/identity.c
    src/rfb/rfb.c
    src/framebuffer/framebuffer.c
    src/rfb/bridge.c
    src/rfb/rfb_session.c
    src/display/display.c
    src/display/mpeg_frame.c
    src/display/mpeg_presentation.c
    src/display/mpeg_scheduler.c
    src/display/mpeg_compositor.c
    src/input/input.c
    src/input/mouse.c
    src/input/input_runtime.c
    src/input/keyboard.c
    src/input/pad.c
    src/ui/local_controller.c
    src/ui/local_ui.c
    src/ui/osk.c
    src/ui/osk_render.c
    src/ui/local_ui_presentation.c
    src/transport/protocol.c
    src/transport/physical_stream.c
    src/transport/rfb_channel.c
    src/transport/audio_channel.c
    src/transport/mpeg_channel.c
    src/transport/runtime.c
    src/transport/quiesce.c
    src/transport/bridge.c
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

declare -A SMS_EXPECTED_BLOBS=(
    ["include/libmpeg.h"]="ee2195b52dc3a7112537046426a80aa0e603fa6c"
    ["include/libmpeg_internal.h"]="c2f80a9104380634c3ef6749baa2e5f0acb5c13e"
    ["src/libmpeg.c"]="f9e5f11689fa6ed3759365249c2d7cfb7335e2fb"
    ["src/libmpeg_core.S"]="93638fd62e58bfac8c6ed1c5fc84ef119d318438"
)

for relative_path in "${!SMS_EXPECTED_BLOBS[@]}"; do
    actual_blob="$(git -C "$ROOT" hash-object "$SMS_VENDOR/$relative_path")"
    expected_blob="${SMS_EXPECTED_BLOBS[$relative_path]}"
    if [ "$actual_blob" != "$expected_blob" ]; then
        echo "SMS_VENDOR_IDENTITY_MISMATCH=$relative_path:$actual_blob:$expected_blob" >&2
        exit 1
    fi
    echo "SMS_VENDOR_IDENTITY_PASS=$relative_path:$actual_blob"
done

SMS_FLAGS=(
    -D_EE
    -O2
    -G8192
    -mgpopt
    -mno-abicalls
    -Wall
    -mno-check-zero-division
    -I"$SMS_INC"
    -I"$PS2SDK/ee/include"
    -I"$PS2SDK/common/include"
)

echo 'PS2_SMS_COMPILE=vendor/sms-libmpeg/src/libmpeg.c'
"$CC" "${SMS_FLAGS[@]}" \
    -c "$SMS_SRC/libmpeg.c" \
    -o "$BUILD_DIR/sms_libmpeg.o"

echo 'PS2_SMS_COMPILE=vendor/sms-libmpeg/src/libmpeg_core.S'
"$CC" "${SMS_FLAGS[@]}" \
    -c "$SMS_SRC/libmpeg_core.S" \
    -o "$BUILD_DIR/sms_libmpeg_core.o"

echo 'SMS_DEDICATED_COMPILE_CHECK=PASS'
echo 'CLEAN_PS2_COMPILE_CHECK=PASS'
