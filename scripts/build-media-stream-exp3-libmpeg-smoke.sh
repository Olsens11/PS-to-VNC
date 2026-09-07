#!/usr/bin/env bash
# File synopsis:
# Build the standalone Media Stream EXP3 embedded MPEG-2 hardware smoke ELF
# with the same pinned PS2 toolchain image used by qualified reconstruction
# builds. The test MPEG-2 elementary stream is generated on the Pi, validated,
# embedded as an EE binary object, and linked with PS2SDK libmpeg.
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

BUILD_DIR="$ROOT/build/experiments/media-stream-exp3/embedded-libmpeg-smoke"
STREAM="$BUILD_DIR/test.bin"
ELF="$BUILD_DIR/PS2VNC-EXP3-EmbeddedMPEG-Smoke.ELF"
MANIFEST="$BUILD_DIR/ARTIFACT-MANIFEST.txt"

IMAGE='ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11'

TOOLCHAIN_PATH='/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'

command -v docker >/dev/null
command -v ffmpeg >/dev/null
command -v ffprobe >/dev/null

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo '===== GENERATE MPEG-2 ELEMENTARY STREAM ====='

ffmpeg \
    -hide_banner \
    -loglevel error \
    -y \
    -f lavfi \
    -i 'testsrc2=size=704x480:rate=30000/1001' \
    -t 20 \
    -an \
    -c:v mpeg2video \
    -profile:v main \
    -level:v 8 \
    -pix_fmt yuv420p \
    -g 15 \
    -bf 0 \
    -b:v 3500k \
    -maxrate 5000k \
    -bufsize 1835k \
    -f mpeg2video \
    "$STREAM"

test -s "$STREAM"

echo '===== VALIDATE GENERATED STREAM ====='

ffprobe \
    -v error \
    -count_frames \
    -select_streams v:0 \
    -show_entries \
stream=codec_name,profile,width,height,pix_fmt,r_frame_rate,level,nb_read_frames \
    -of default=noprint_wrappers=1 \
    "$STREAM"

frames="$(
    ffprobe \
        -v error \
        -count_frames \
        -select_streams v:0 \
        -show_entries stream=nb_read_frames \
        -of default=nokey=1:noprint_wrappers=1 \
        "$STREAM"
)"

test "$frames" -ge 595
test "$frames" -le 605

echo "GENERATED_FRAMES=$frames"

echo '===== BUILD WITH PINNED PS2 TOOLCHAIN ====='

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

        apk add \
            --no-cache \
            bash \
            build-base \
            >/dev/null

        export PATH='$TOOLCHAIN_PATH'

        make \
            -f mk/media-stream-exp3-libmpeg-smoke.mk \
            all

        test -s \
            build/experiments/media-stream-exp3/embedded-libmpeg-smoke/PS2VNC-EXP3-EmbeddedMPEG-Smoke.ELF

        chown -R \
            \"\$HOST_UID:\$HOST_GID\" \
            build/experiments/media-stream-exp3/embedded-libmpeg-smoke
    "

test -s "$ELF"

echo '===== ELF SIZE / IDENTITY ====='

ELF_BYTES="$(
    stat -c '%s' "$ELF"
)"

ELF_SHA256="$(
    sha256sum "$ELF" |
        awk '{print $1}'
)"

STREAM_BYTES="$(
    stat -c '%s' "$STREAM"
)"

STREAM_SHA256="$(
    sha256sum "$STREAM" |
        awk '{print $1}'
)"

SOURCE_SHA256="$(
    sha256sum \
        "$ROOT/experiments/media-stream-exp3/libmpeg_embedded_smoke.c" |
        awk '{print $1}'
)"

printf 'ELF=%s\n' "$ELF"
printf 'ELF_BYTES=%s\n' "$ELF_BYTES"
printf 'ELF_SHA256=%s\n' "$ELF_SHA256"

printf 'STREAM=%s\n' "$STREAM"
printf 'STREAM_BYTES=%s\n' "$STREAM_BYTES"
printf 'STREAM_SHA256=%s\n' "$STREAM_SHA256"

printf 'SOURCE_SHA256=%s\n' "$SOURCE_SHA256"

echo '===== VERIFY EMBEDDED INPUT SYMBOLS WERE RESOLVED ====='

docker run --rm \
    --entrypoint /bin/sh \
    -v "$ROOT:/repo:ro" \
    -w /repo \
    "$IMAGE" \
    -lc "
        set -eu
        export PATH='$TOOLCHAIN_PATH'

        mips64r5900el-ps2-elf-readelf \
            -h \
            build/experiments/media-stream-exp3/embedded-libmpeg-smoke/PS2VNC-EXP3-EmbeddedMPEG-Smoke.ELF

        if mips64r5900el-ps2-elf-nm \
            -u \
            build/experiments/media-stream-exp3/embedded-libmpeg-smoke/PS2VNC-EXP3-EmbeddedMPEG-Smoke.ELF |
            grep -q '_binary_test_bin_'
        then
            echo 'ERROR: unresolved embedded-stream symbol' >&2
            exit 1
        fi
    "

cat > "$MANIFEST" <<MANIFEST
EXP3_HEAD=$(git -C "$ROOT" rev-parse HEAD)
PINNED_IMAGE=$IMAGE

SOURCE=experiments/media-stream-exp3/libmpeg_embedded_smoke.c
SOURCE_SHA256=$SOURCE_SHA256

MPEG_TEST_STREAM=$STREAM
MPEG_TEST_STREAM_BYTES=$STREAM_BYTES
MPEG_TEST_STREAM_SHA256=$STREAM_SHA256
MPEG_TEST_FRAMES=$frames

VIDEO_CODEC=mpeg2video
VIDEO_PROFILE=main
VIDEO_LEVEL=main
VIDEO_SIZE=704x480
VIDEO_PIXEL_FORMAT=yuv420p
VIDEO_RATE=30000/1001
VIDEO_GOP=15
VIDEO_B_FRAMES=0
VIDEO_DURATION_SECONDS=20

ELF=$ELF
ELF_BYTES=$ELF_BYTES
ELF_SHA256=$ELF_SHA256

VISIBLE_SETUP_STATE=BLUE
VISIBLE_SUCCESS_STATE=GREEN
VISIBLE_FAILURE_STATE=RED
MIN_SUCCESS_PICTURES=550
MANIFEST

cat "$MANIFEST"

echo 'EXP3_EMBEDDED_LIBMPEG_BUILD=PASS'
