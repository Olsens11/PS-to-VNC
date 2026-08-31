#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd
)"

cd "$ROOT"

PREP="$ROOT/scripts/testkit/prepare-hardware-elf.sh"
PT="$ROOT/scripts/testkit/pt-load-fingerprint.sh"

LEGACY="${PS2VNC_LEGACY_TESTKIT_ROOT:-/home/ps2/ps2vnc/scripts/testkit}"

FIXTURE="${TESTKIT_FIXTURE_ELF:-$ROOT/working/b4a/PS2VNC.ELF}"
TEST_ID="${TESTKIT_SELF_TEST_ID:-PS-TO-VNC-TESTKIT-SELFTEST}"

for tool in \
    "$PREP" \
    "$PT" \
    "$LEGACY/stamp-elf-identity.sh" \
    "$LEGACY/verify-elf-identity.sh"
do
    [ -x "$tool" ] || {
        echo "required executable missing: $tool" >&2
        exit 1
    }
done

[ -f "$FIXTURE" ] || {
    echo "fixture ELF missing: $FIXTURE" >&2
    exit 1
}

bash -n "$PREP"
bash -n "$PT"

ABS_TMP="$(mktemp -d)"
REL_TMP="$(mktemp -d .testkit-self-test.XXXXXX)"

cleanup()
{
    rm -rf "$ABS_TMP" "$REL_TMP"
}

trap cleanup EXIT

before="$(
    sha256sum "$FIXTURE" |
    awk '{print $1}'
)"

"$PT" \
    "$FIXTURE" \
    "$ABS_TMP/pristine.load" \
    > "$ABS_TMP/pt.txt"

grep -qx \
    'TESTKIT_PT_LOAD_FINGERPRINT=PASS' \
    "$ABS_TMP/pt.txt"

echo '[SELFTEST] absolute output path'

"$PREP" \
    "$FIXTURE" \
    "$TEST_ID" \
    "$ABS_TMP/absolute.ELF" \
    > "$ABS_TMP/absolute.txt"

grep -qx \
    'TESTKIT_PREPARE_HARDWARE_ELF=PASS' \
    "$ABS_TMP/absolute.txt"

echo '[SELFTEST] relative pristine + relative output across legacy delegation'

"$PREP" \
    working/b4a/PS2VNC.ELF \
    "$TEST_ID" \
    "$REL_TMP/relative.ELF" \
    > "$ABS_TMP/relative.txt"

grep -qx \
    'TESTKIT_PREPARE_HARDWARE_ELF=PASS' \
    "$ABS_TMP/relative.txt"

echo '[SELFTEST] explicit legacy TestKit override'

PS2VNC_LEGACY_TESTKIT_ROOT="$LEGACY" \
"$PREP" \
    "$FIXTURE" \
    "$TEST_ID" \
    "$ABS_TMP/override.ELF" \
    > "$ABS_TMP/override.txt"

grep -qx \
    'TESTKIT_PREPARE_HARDWARE_ELF=PASS' \
    "$ABS_TMP/override.txt"

cmp -s \
    "$ABS_TMP/absolute.ELF" \
    "$REL_TMP/relative.ELF"

cmp -s \
    "$ABS_TMP/absolute.ELF" \
    "$ABS_TMP/override.ELF"

prepared_sha="$(
    sha256sum "$ABS_TMP/absolute.ELF" |
    awk '{print $1}'
)"

relative_sha="$(
    sha256sum "$REL_TMP/relative.ELF" |
    awk '{print $1}'
)"

override_sha="$(
    sha256sum "$ABS_TMP/override.ELF" |
    awk '{print $1}'
)"

[ "$prepared_sha" = "$relative_sha" ]
[ "$prepared_sha" = "$override_sha" ]

identity="$(
    sed -n \
        's/^HARDWARE_ELF_IDENTITY_SHA256=//p' \
        "$ABS_TMP/absolute.txt" |
    tail -1
)"

[ -n "$identity" ]

"$LEGACY/verify-elf-identity.sh" \
    "$ABS_TMP/absolute.ELF" \
    "$TEST_ID" \
    "$identity" \
    > "$ABS_TMP/verify.txt"

grep -qx \
    'TESTKIT_VERIFY_ELF_IDENTITY=PASS' \
    "$ABS_TMP/verify.txt"

if "$PREP" \
    "$FIXTURE" \
    "$TEST_ID" \
    "$ABS_TMP/absolute.ELF" \
    > "$ABS_TMP/existing-output.txt" 2>&1
then
    echo \
        'prepare-hardware-elf unexpectedly overwrote existing output' \
        >&2
    exit 1
fi

after="$(
    sha256sum "$FIXTURE" |
    awk '{print $1}'
)"

[ "$before" = "$after" ]

echo 'TESTKIT_SELF_TEST_VERSION=2'
echo "FIXTURE_ELF_SHA256=$before"
echo "SELF_TEST_ID=$TEST_ID"
echo "PREPARED_ELF_SHA256=$prepared_sha"
echo 'ABSOLUTE_OUTPUT_PATH=PASS'
echo 'RELATIVE_PRISTINE_PATH=PASS'
echo 'RELATIVE_OUTPUT_PATH=PASS'
echo 'CROSS_REPOSITORY_DELEGATION_PATH_NORMALIZATION=PASS'
echo 'DEFAULT_LEGACY_TESTKIT_PATH=PASS'
echo 'OVERRIDE_LEGACY_TESTKIT_PATH=PASS'
echo 'DETERMINISTIC_PREPARATION=PASS'
echo 'CORRECT_THREE_ARGUMENT_IDENTITY_VERIFY=PASS'
echo 'EXISTING_OUTPUT_FAIL_CLOSED=PASS'
echo 'FIXTURE_IMMUTABLE=PASS'
echo '[SELFTEST] M4 hardware-checkpoint activation'
python3 scripts/testkit/activation-self-test.py

echo 'PS_TO_VNC_TESTKIT_SELF_TEST=PASS'
