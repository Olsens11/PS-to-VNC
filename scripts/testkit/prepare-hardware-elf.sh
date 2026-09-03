#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd
)"

STAMP_TOOL="$ROOT/scripts/testkit/stamp-elf-identity.sh"
VERIFY_TOOL="$ROOT/scripts/testkit/verify-elf-identity.sh"
PT_TOOL="$ROOT/scripts/testkit/pt-load-fingerprint.sh"

usage()
{
    echo \
        'usage: prepare-hardware-elf.sh PRISTINE_ELF TEST_ID OUTPUT_ELF' \
        >&2
    exit 2
}

[ "$#" -eq 3 ] || usage

PRISTINE="$1"
TEST_ID="$2"
OUTPUT="$3"

[ -f "$PRISTINE" ] || {
    echo "pristine ELF not found: $PRISTINE" >&2
    exit 1
}

[ -n "$TEST_ID" ] || {
    echo 'TEST_ID must not be empty' >&2
    exit 1
}

[ ! -e "$OUTPUT" ] || {
    echo "output already exists: $OUTPUT" >&2
    exit 1
}

for tool in \
    "$STAMP_TOOL" \
    "$VERIFY_TOOL" \
    "$PT_TOOL"
do
    [ -x "$tool" ] || {
        echo "required tool unavailable: $tool" >&2
        exit 1
    }
done

OUT_DIR="$(dirname -- "$OUTPUT")"

[ -d "$OUT_DIR" ] || {
    echo "output directory not found: $OUT_DIR" >&2
    exit 1
}

# Normalize caller-supplied file operands so preparation is independent of the
# caller's working directory and every tool sees one unambiguous path.
PRISTINE_DIR="$(dirname -- "$PRISTINE")"
PRISTINE_BASE="$(basename -- "$PRISTINE")"
OUTPUT_BASE="$(basename -- "$OUTPUT")"

PRISTINE="$(
    cd "$PRISTINE_DIR" &&
    printf '%s/%s\n' "$(pwd -P)" "$PRISTINE_BASE"
)"

OUTPUT="$(
    cd "$OUT_DIR" &&
    printf '%s/%s\n' "$(pwd -P)" "$OUTPUT_BASE"
)"

TMP="$(mktemp -d)"

cleanup()
{
    rm -rf "$TMP"
}

trap cleanup EXIT

one_value()
{
    key="$1"
    file="$2"

    values="$(
        sed -n "s/^${key}=//p" "$file"
    )"

    count="$(
        printf '%s\n' "$values" |
        sed '/^$/d' |
        wc -l
    )"

    [ "$count" -eq 1 ] || {
        echo \
            "expected exactly one $key in $file; found $count" \
            >&2
        exit 1
    }

    printf '%s\n' "$values"
}

sha256()
{
    sha256sum "$1" |
    awk '{print $1}'
}

cp "$PRISTINE" "$OUTPUT"

STAMP_LOG="$TMP/stamp.txt"

"$STAMP_TOOL" \
    "$OUTPUT" \
    "$TEST_ID" \
    | tee "$STAMP_LOG"

IDENTITY_SHA="$(
    one_value ELF_IDENTITY_SHA256 "$STAMP_LOG"
)"

STAMP_REPORTED_FINAL="$(
    one_value ELF_FINAL_SHA256 "$STAMP_LOG"
)"

STAMPED_SHA="$(
    sha256 "$OUTPUT"
)"

[ "$STAMP_REPORTED_FINAL" = "$STAMPED_SHA" ] || {
    echo \
        'stamp tool final SHA does not match output file' \
        >&2
    exit 1
}

VERIFY_LOG="$TMP/verify.txt"

"$VERIFY_TOOL" \
    "$OUTPUT" \
    "$TEST_ID" \
    "$IDENTITY_SHA" \
    | tee "$VERIFY_LOG"

grep -qx \
    'TESTKIT_VERIFY_ELF_IDENTITY=PASS' \
    "$VERIFY_LOG"

SECOND="$TMP/second.ELF"

cp "$PRISTINE" "$SECOND"

"$STAMP_TOOL" \
    "$SECOND" \
    "$TEST_ID" \
    > "$TMP/stamp-second.txt"

SECOND_IDENTITY="$(
    one_value \
        ELF_IDENTITY_SHA256 \
        "$TMP/stamp-second.txt"
)"

SECOND_FINAL="$(
    one_value \
        ELF_FINAL_SHA256 \
        "$TMP/stamp-second.txt"
)"

[ "$SECOND_IDENTITY" = "$IDENTITY_SHA" ]
[ "$SECOND_FINAL" = "$STAMPED_SHA" ]

cmp -s "$OUTPUT" "$SECOND"

PRISTINE_SHA="$(
    sha256 "$PRISTINE"
)"

PRISTINE_BYTES="$(
    wc -c < "$PRISTINE"
)"

STAMPED_BYTES="$(
    wc -c < "$OUTPUT"
)"

PRISTINE_LOAD="$TMP/pristine.load"
STAMPED_LOAD="$TMP/stamped.load"

PRISTINE_FP="$(
    "$PT_TOOL" \
        "$PRISTINE" \
        "$PRISTINE_LOAD"
)"

STAMPED_FP="$(
    "$PT_TOOL" \
        "$OUTPUT" \
        "$STAMPED_LOAD"
)"

value_from_text()
{
    key="$1"
    text="$2"

    printf '%s\n' "$text" |
    sed -n "s/^${key}=//p"
}

PRISTINE_LOAD_SHA="$(
    value_from_text \
        PT_LOAD_SHA256 \
        "$PRISTINE_FP"
)"

PRISTINE_LOAD_BYTES="$(
    value_from_text \
        PT_LOAD_BYTES \
        "$PRISTINE_FP"
)"

STAMPED_LOAD_SHA="$(
    value_from_text \
        PT_LOAD_SHA256 \
        "$STAMPED_FP"
)"

STAMPED_LOAD_BYTES="$(
    value_from_text \
        PT_LOAD_BYTES \
        "$STAMPED_FP"
)"

python3 - \
    "$PRISTINE" \
    "$OUTPUT" \
    "$PRISTINE_LOAD" \
    "$STAMPED_LOAD" <<'PY'
from pathlib import Path
import sys

def report(label, left_name, right_name):
    left = Path(left_name).read_bytes()
    right = Path(right_name).read_bytes()

    if len(left) != len(right):
        print(f"{label}_SIZE_CHANGED=YES")
        print(f"{label}_OLD_BYTES={len(left)}")
        print(f"{label}_NEW_BYTES={len(right)}")
        return

    changed = [
        i
        for i, (a, b)
        in enumerate(zip(left, right))
        if a != b
    ]

    print(f"{label}_SIZE_CHANGED=NO")
    print(
        f"{label}_CHANGED_BYTE_COUNT="
        f"{len(changed)}"
    )

    if changed:
        print(
            f"{label}_FIRST_CHANGED_OFFSET="
            f"{changed[0]}"
        )
        print(
            f"{label}_LAST_CHANGED_OFFSET="
            f"{changed[-1]}"
        )
    else:
        print(
            f"{label}_FIRST_CHANGED_OFFSET=NONE"
        )
        print(
            f"{label}_LAST_CHANGED_OFFSET=NONE"
        )

report(
    "WHOLE_ELF",
    sys.argv[1],
    sys.argv[2],
)

report(
    "PT_LOAD",
    sys.argv[3],
    sys.argv[4],
)
PY

echo "TESTKIT_PREPARE_HARDWARE_ELF_VERSION=2"
echo "IDENTITY_TOOL_AUTHORITY=SUCCESSOR_REPOSITORY"
echo "PRISTINE_ELF=$PRISTINE"
echo "PRISTINE_ELF_SHA256=$PRISTINE_SHA"
echo "PRISTINE_ELF_BYTES=$PRISTINE_BYTES"

echo "HARDWARE_TEST_ID=$TEST_ID"
echo "HARDWARE_ELF_IDENTITY_SHA256=$IDENTITY_SHA"

echo "HARDWARE_STAMPED_ELF=$OUTPUT"
echo "HARDWARE_STAMPED_ELF_SHA256=$STAMPED_SHA"
echo "HARDWARE_STAMPED_ELF_BYTES=$STAMPED_BYTES"

echo "PRISTINE_PT_LOAD_SHA256=$PRISTINE_LOAD_SHA"
echo "PRISTINE_PT_LOAD_BYTES=$PRISTINE_LOAD_BYTES"

echo "HARDWARE_STAMPED_PT_LOAD_SHA256=$STAMPED_LOAD_SHA"
echo "HARDWARE_STAMPED_PT_LOAD_BYTES=$STAMPED_LOAD_BYTES"

if cmp -s "$PRISTINE_LOAD" "$STAMPED_LOAD"; then
    echo 'IDENTITY_STAMP_PT_LOAD_RELATION=BYTE_EXACT'
else
    echo 'IDENTITY_STAMP_PT_LOAD_RELATION=NONIDENTICAL'
fi

echo 'IDENTITY_STAMP_REPRODUCIBLE=YES'
echo 'TESTKIT_PREPARE_HARDWARE_ELF=PASS'
