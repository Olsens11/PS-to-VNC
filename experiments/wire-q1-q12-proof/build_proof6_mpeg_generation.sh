#!/usr/bin/env bash
# File synopsis:
# Generates Proof 6's finite MPEG-2 source stream and builds the PS2 DUT against
# current product Transport plus the clean MPEG decoder in the pinned PS2DEV
# image. No FTP or PS2 contact occurs.
set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd
)"

BUILD_DIR="$ROOT/build/experiments/wire-q1-q12-proof/proof6-mpeg-generation"
DEP_DIR="$BUILD_DIR/deps"
STREAM="$BUILD_DIR/proof6-source.m2v"
ELF="$BUILD_DIR/PS2VNC-Wire-Proof6-MPEG-Generation.ELF"
MANIFEST="$BUILD_DIR/ARTIFACT-MANIFEST.txt"

FROZEN_DEP="$ROOT/baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a"
BUILD_DEP="$DEP_DIR/libps2ip_mtu1458_wscale128.a"
EXPECTED_DEP_SHA='b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

IMAGE='ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11'
TOOLCHAIN_PATH='/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'

command -v docker >/dev/null 2>&1
command -v ffmpeg >/dev/null 2>&1
command -v ffprobe >/dev/null 2>&1

python3 -B \
    "$ROOT/experiments/wire-q1-q12-proof/proof6_mpeg_generation_server.py" \
    --self-test

test "$(sha256sum "$FROZEN_DEP" | awk '{print $1}')" = "$EXPECTED_DEP_SHA"

rm -rf "$BUILD_DIR"
mkdir -p "$DEP_DIR"
cp "$FROZEN_DEP" "$BUILD_DEP"

echo '===== GENERATE PROOF6 MPEG-2 STREAM ====='

ffmpeg \
    -hide_banner \
    -loglevel error \
    -y \
    -f lavfi \
    -i 'testsrc2=size=704x480:rate=30000/1001' \
    -t 12 \
    -an \
    -c:v mpeg2video \
    -profile:v main \
    -level:v 8 \
    -pix_fmt yuv420p \
    -g 15 \
    -bf 0 \
    -b:v 2500k \
    -maxrate 4000k \
    -bufsize 1835k \
    -f mpeg2video \
    "$STREAM"

test -s "$STREAM"

FRAMES="$(
    ffprobe \
        -v error \
        -count_frames \
        -select_streams v:0 \
        -show_entries stream=nb_read_frames \
        -of default=nokey=1:noprint_wrappers=1 \
        "$STREAM"
)"

test "$FRAMES" -ge 355
test "$FRAMES" -le 365

ffprobe \
    -v error \
    -select_streams v:0 \
    -show_entries \
stream=codec_name,profile,width,height,pix_fmt,r_frame_rate,level,nb_read_frames \
    -count_frames \
    -of default=noprint_wrappers=1 \
    "$STREAM"

HOST_UID="$(id -u)"
HOST_GID="$(id -g)"

timeout 240s docker run --rm \
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
        make -f mk/wire-q1-q12-proof6-mpeg-generation.mk
        chown -R \"\$HOST_UID:\$HOST_GID\" \
            build/experiments/wire-q1-q12-proof/proof6-mpeg-generation
    "

test -f "$ELF"

ELF_BYTES="$(stat -c %s "$ELF")"
ELF_SHA256="$(sha256sum "$ELF" | awk '{print $1}')"
STREAM_BYTES="$(stat -c %s "$STREAM")"
STREAM_SHA256="$(sha256sum "$STREAM" | awk '{print $1}')"

cat > "$MANIFEST" <<MANIFEST
PROOF6_SOURCE_HEAD=$(git -C "$ROOT" rev-parse HEAD)
PINNED_IMAGE=$IMAGE
PS2IP_SHA256=$EXPECTED_DEP_SHA

STREAM=$STREAM
STREAM_BYTES=$STREAM_BYTES
STREAM_SHA256=$STREAM_SHA256
STREAM_FRAMES=$FRAMES
STREAM_CODEC=mpeg2video
STREAM_SIZE=704x480
STREAM_RATE=30000/1001
STREAM_PIXEL_FORMAT=yuv420p

ELF=$ELF
ELF_BYTES=$ELF_BYTES
ELF_SHA256=$ELF_SHA256

PRODUCT_TRANSPORT=src/transport
CLEAN_MPEG_DECODER=src/mpeg/decoder.c
Q4_ESTABLISHMENT=PROOF_LOCAL
RFB_CONTROL_MARKERS=PROOF_LOCAL
PS2_LIBMPEG_GS_ADAPTER=DEVELOPMENT_APPARATUS
STOP_REQUEST_TARGET=DURING_ACTIVE_MPEG_PICTURE
MPEG_WORKER_PRIORITY=67
MPEG_QUEUE_BYTES=524288
MPEG_INITIAL_CREDIT_BYTES=524288
MANIFEST

cat "$MANIFEST"

echo "PROOF6_MPEG_BUILD=PASS"
echo "ELF_PATH=$ELF"
echo "ELF_BYTES=$ELF_BYTES"
echo "ELF_SHA256=$ELF_SHA256"
echo "STREAM_PATH=$STREAM"
echo "STREAM_SHA256=$STREAM_SHA256"
"$ROOT/scripts/testkit/pt-load-fingerprint.sh" "$ELF"
