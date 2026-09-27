#!/usr/bin/env bash
# File synopsis:
# Builds both R45 PS2 wake-discriminator variants twice in the exact pinned PS2
# toolchain, proves per-variant reproducibility, proves baseline/control identity
# distinction, and reports exact ELF/PT_LOAD identities. Never contacts hardware.
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/../../.." &&
    pwd
)"

IMAGE='ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11'
TOOLCHAIN_PATH='/usr/local/ps2dev/bin:/usr/local/ps2dev/ee/bin:/usr/local/ps2dev/iop/bin:/usr/local/ps2dev/dvp/bin:/usr/local/ps2dev/ps2sdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'
FROZEN_DEP="$ROOT/baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a"
EXPECTED_DEP_SHA='b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'
MAKEFILE='tests/hardware/r45-wake-discriminator/Makefile.ps2'
OUT_ROOT="$ROOT/build/r45-wake-discriminator"
PT_TOOL="$ROOT/scripts/testkit/pt-load-fingerprint.sh"

BASELINE_DIR="$OUT_ROOT/baseline"
CONTROL_DIR="$OUT_ROOT/control"
PROOF_DIR="$OUT_ROOT/proof"

BASELINE_ONE="$PROOF_DIR/baseline-first.ELF"
BASELINE_TWO="$OUT_ROOT/baseline/R45-WAKE-baseline.ELF"
CONTROL_ONE="$PROOF_DIR/control-first.ELF"
CONTROL_TWO="$OUT_ROOT/control/R45-WAKE-control.ELF"

BASELINE_CANONICAL="$OUT_ROOT/R45-WAKE-BASELINE-1000US.ELF"
CONTROL_CANONICAL="$OUT_ROOT/R45-WAKE-CONTROL-ZERO.ELF"

die()
{
    echo "ERROR: $*" >&2
    exit 1
}

sha256()
{
    sha256sum "$1" | awk '{print $1}'
}

one_value()
{
    key="$1"
    file="$2"
    value="$(sed -n "s/^$key=//p" "$file")"
    count="$(printf '%s\n' "$value" | sed '/^$/d' | wc -l)"
    [ "$count" -eq 1 ] || die "expected exactly one $key in $file; found $count"
    printf '%s\n' "$value"
}

command -v docker >/dev/null 2>&1 || die 'docker is required'
[ -x "$PT_TOOL" ] || die "missing PT_LOAD tool: $PT_TOOL"
[ -f "$FROZEN_DEP" ] || die "missing frozen PS2IP archive: $FROZEN_DEP"
[ "$(sha256 "$FROZEN_DEP")" = "$EXPECTED_DEP_SHA" ] ||
    die 'frozen PS2IP identity mismatch'

rm -rf "$OUT_ROOT"
mkdir -p "$PROOF_DIR"

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
        apk add --no-cache bash build-base binutils >/dev/null
        export PATH='$TOOLCHAIN_PATH'

        build_variant()
        {
            name=\"\$1\"
            control=\"\$2\"
            output=\"build/r45-wake-discriminator/\$name\"

            rm -rf \"\$output\"
            mkdir -p \"\$output/deps\"
            cp baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a \
                \"\$output/deps/libps2ip_mtu1458_wscale128.a\"

            make -f '$MAKEFILE' \
                VARIANT_NAME=\"\$name\" \
                R45_CONTROL=\"\$control\" \
                PS2IP_LIB=\"\$output/deps/libps2ip_mtu1458_wscale128.a\"
        }

        build_variant baseline 0
        cp build/r45-wake-discriminator/baseline/R45-WAKE-baseline.ELF \
            build/r45-wake-discriminator/proof/baseline-first.ELF
        build_variant baseline 0

        build_variant control 1
        cp build/r45-wake-discriminator/control/R45-WAKE-control.ELF \
            build/r45-wake-discriminator/proof/control-first.ELF
        build_variant control 1

        chown -R \"\$HOST_UID:\$HOST_GID\" build/r45-wake-discriminator
    "

for elf in "$BASELINE_ONE" "$BASELINE_TWO" "$CONTROL_ONE" "$CONTROL_TWO"; do
    [ -f "$elf" ] || die "expected discriminator ELF missing: $elf"
done

cmp -s "$BASELINE_ONE" "$BASELINE_TWO" ||
    die 'baseline discriminator is not byte-reproducible'
cmp -s "$CONTROL_ONE" "$CONTROL_TWO" ||
    die 'control discriminator is not byte-reproducible'

cp "$BASELINE_ONE" "$BASELINE_CANONICAL"
cp "$CONTROL_ONE" "$CONTROL_CANONICAL"

cmp -s "$BASELINE_CANONICAL" "$CONTROL_CANONICAL" &&
    die 'baseline and control whole-ELF identities are unexpectedly identical'

strings "$BASELINE_CANONICAL" |
    grep -Fx 'R45_WAKE_DISCRIMINATOR_V1:BASELINE_1000US' >/dev/null ||
    die 'baseline variant identity stamp missing'
strings "$CONTROL_CANONICAL" |
    grep -Fx 'R45_WAKE_DISCRIMINATOR_V1:CONTROL_ZERO_TIMEOUT' >/dev/null ||
    die 'control variant identity stamp missing'

"$PT_TOOL" "$BASELINE_CANONICAL" > "$OUT_ROOT/baseline.ptload"
"$PT_TOOL" "$CONTROL_CANONICAL" > "$OUT_ROOT/control.ptload"

BASELINE_ELF_SHA="$(sha256 "$BASELINE_CANONICAL")"
CONTROL_ELF_SHA="$(sha256 "$CONTROL_CANONICAL")"
BASELINE_PT_SHA="$(one_value PT_LOAD_SHA256 "$OUT_ROOT/baseline.ptload")"
CONTROL_PT_SHA="$(one_value PT_LOAD_SHA256 "$OUT_ROOT/control.ptload")"
BASELINE_PT_BYTES="$(one_value PT_LOAD_BYTES "$OUT_ROOT/baseline.ptload")"
CONTROL_PT_BYTES="$(one_value PT_LOAD_BYTES "$OUT_ROOT/control.ptload")"

[ "$BASELINE_ELF_SHA" != "$CONTROL_ELF_SHA" ] ||
    die 'baseline/control ELF SHA identities must differ'
[ "$BASELINE_PT_SHA" != "$CONTROL_PT_SHA" ] ||
    die 'baseline/control PT_LOAD identities must differ'

cat > "$OUT_ROOT/BUILD-AUTHORITY.env" <<EOF
R45_DISCRIMINATOR_BUILD_VERSION=1
R45_TOOLCHAIN_IMAGE=$IMAGE
R45_PS2IP_SHA256=$EXPECTED_DEP_SHA
R45_CYCLE_COUNT=4096
R45_PEER_APPLICATION_BYTES_SENT=0
R45_BASELINE_VARIANT=BASELINE_1000US
R45_BASELINE_READINESS_TIMEOUT_US=1000
R45_BASELINE_ELF_SHA256=$BASELINE_ELF_SHA
R45_BASELINE_PT_LOAD_SHA256=$BASELINE_PT_SHA
R45_BASELINE_PT_LOAD_BYTES=$BASELINE_PT_BYTES
R45_BASELINE_REPRODUCIBLE=YES
R45_CONTROL_VARIANT=CONTROL_ZERO_TIMEOUT
R45_CONTROL_READINESS_TIMEOUT_US=0
R45_CONTROL_ELF_SHA256=$CONTROL_ELF_SHA
R45_CONTROL_PT_LOAD_SHA256=$CONTROL_PT_SHA
R45_CONTROL_PT_LOAD_BYTES=$CONTROL_PT_BYTES
R45_CONTROL_REPRODUCIBLE=YES
R45_VARIANTS_DISTINCT=YES
R45_HARDWARE_EXECUTED=NO
R45_DISCRIMINATOR_BUILD=PASS
EOF

cat "$OUT_ROOT/BUILD-AUTHORITY.env"
