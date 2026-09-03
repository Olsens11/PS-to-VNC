#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd
)"

cd "$ROOT"

PREP="$ROOT/scripts/testkit/prepare-hardware-elf.sh"
PT="$ROOT/scripts/testkit/pt-load-fingerprint.sh"
STAMP="$ROOT/scripts/testkit/stamp-elf-identity.sh"
VERIFY="$ROOT/scripts/testkit/verify-elf-identity.sh"

FIXTURE="${TESTKIT_FIXTURE_ELF:-$ROOT/working/b4a/PS2VNC.ELF}"
TEST_ID="${TESTKIT_SELF_TEST_ID:-PS-TO-VNC-TESTKIT-SELFTEST}"

for tool in \
    "$PREP" \
    "$PT" \
    "$STAMP" \
    "$VERIFY"
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
bash -n "$STAMP"
bash -n "$VERIFY"

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

grep -qx \
    'IDENTITY_TOOL_AUTHORITY=SUCCESSOR_REPOSITORY' \
    "$ABS_TMP/absolute.txt"

echo '[SELFTEST] relative pristine + relative output'

"$PREP" \
    working/b4a/PS2VNC.ELF \
    "$TEST_ID" \
    "$REL_TMP/relative.ELF" \
    > "$ABS_TMP/relative.txt"

grep -qx \
    'TESTKIT_PREPARE_HARDWARE_ELF=PASS' \
    "$ABS_TMP/relative.txt"

cmp -s \
    "$ABS_TMP/absolute.ELF" \
    "$REL_TMP/relative.ELF"

prepared_sha="$(
    sha256sum "$ABS_TMP/absolute.ELF" |
    awk '{print $1}'
)"

relative_sha="$(
    sha256sum "$REL_TMP/relative.ELF" |
    awk '{print $1}'
)"

[ "$prepared_sha" = "$relative_sha" ]

identity="$(
    sed -n \
        's/^HARDWARE_ELF_IDENTITY_SHA256=//p' \
        "$ABS_TMP/absolute.txt" |
    tail -1
)"

[ -n "$identity" ]

"$VERIFY" \
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

echo 'TESTKIT_SELF_TEST_VERSION=3'
echo 'TESTKIT_SELF_TEST_SCOPE=SUCCESSOR_IDENTITY_PREPARATION'
echo "FIXTURE_ELF_SHA256=$before"
echo "SELF_TEST_ID=$TEST_ID"
echo "PREPARED_ELF_SHA256=$prepared_sha"
echo 'ABSOLUTE_OUTPUT_PATH=PASS'
echo 'RELATIVE_PRISTINE_PATH=PASS'
echo 'RELATIVE_OUTPUT_PATH=PASS'
echo 'SUCCESSOR_IDENTITY_TOOLING=PASS'
echo 'DETERMINISTIC_PREPARATION=PASS'
echo 'CORRECT_THREE_ARGUMENT_IDENTITY_VERIFY=PASS'
echo 'EXISTING_OUTPUT_FAIL_CLOSED=PASS'
echo 'FIXTURE_IMMUTABLE=PASS'

echo '[SELFTEST] sealed historical identity compatibility'
./scripts/testkit/successor-identity-compat-self-test.py

echo '[SELFTEST] runtime identity message serialization'
./scripts/testkit/identity-runtime-message-self-test.sh

# Checkpoint activation remains a separate historical/current-state concern.
# Issue #7 hardware execution is now owned entirely by successor tooling.
echo 'CHECKPOINT_ACTIVATION_SELF_TEST=SEPARATE_SCOPE'
echo 'LEGACY_HARDWARE_BRIDGE=RETIRED'

echo '[SELFTEST] successor Issue #7 hardware apparatus'
./scripts/testkit/issue7-apparatus-self-test.sh
echo 'ISSUE7_HARDWARE_APPARATUS_SELF_TEST=PASS'

echo 'PS_TO_VNC_TESTKIT_SELF_TEST=PASS'
