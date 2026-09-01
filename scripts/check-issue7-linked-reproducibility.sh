#!/usr/bin/env bash
# File synopsis:
# Build the clean Issue #7 ELF twice and require byte-for-byte whole-ELF plus
# PT_LOAD digest/length reproducibility before accepting the linked artifact.
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

BUILD="$ROOT/scripts/build-issue7-clean.sh"
PT="$ROOT/scripts/testkit/pt-load-fingerprint.sh"
ELF="$ROOT/build/reconstruction/issue7/PS-to-VNC-Issue7.ELF"

for tool in "$BUILD" "$PT"; do
    [ -x "$tool" ] || {
        echo "required executable missing: $tool" >&2
        exit 1
    }
done

TMP="$(mktemp -d)"

cleanup()
{
    rm -rf "$TMP"
}

trap cleanup EXIT

sha256()
{
    sha256sum "$1" | awk '{print $1}'
}

value_from_file()
{
    key="$1"
    file="$2"

    value="$(sed -n "s/^${key}=//p" "$file")"
    count="$(printf '%s\n' "$value" | sed '/^$/d' | wc -l)"

    [ "$count" -eq 1 ] || {
        echo "expected exactly one $key in $file; found $count" >&2
        exit 1
    }

    printf '%s\n' "$value"
}

echo '[ISSUE7 REPRO] build 1'
"$BUILD" > "$TMP/build1.log"
test -f "$ELF"
cp "$ELF" "$TMP/build1.ELF"

"$PT" "$TMP/build1.ELF" > "$TMP/build1.pt"

BUILD1_SHA="$(sha256 "$TMP/build1.ELF")"
BUILD1_PT_SHA="$(value_from_file PT_LOAD_SHA256 "$TMP/build1.pt")"
BUILD1_PT_BYTES="$(value_from_file PT_LOAD_BYTES "$TMP/build1.pt")"

echo '[ISSUE7 REPRO] build 2'
"$BUILD" > "$TMP/build2.log"
test -f "$ELF"

"$PT" "$ELF" > "$TMP/build2.pt"

BUILD2_SHA="$(sha256 "$ELF")"
BUILD2_PT_SHA="$(value_from_file PT_LOAD_SHA256 "$TMP/build2.pt")"
BUILD2_PT_BYTES="$(value_from_file PT_LOAD_BYTES "$TMP/build2.pt")"

[ "$BUILD1_SHA" = "$BUILD2_SHA" ] || {
    echo 'linked ELF is not reproducible across two clean builds' >&2
    echo "BUILD1_ELF_SHA256=$BUILD1_SHA" >&2
    echo "BUILD2_ELF_SHA256=$BUILD2_SHA" >&2
    exit 1
}

cmp -s "$TMP/build1.ELF" "$ELF" || {
    echo 'linked ELF byte comparison failed despite SHA check' >&2
    exit 1
}

[ "$BUILD1_PT_SHA" = "$BUILD2_PT_SHA" ] || {
    echo 'PT_LOAD fingerprint is not reproducible across two clean builds' >&2
    exit 1
}

[ "$BUILD1_PT_BYTES" = "$BUILD2_PT_BYTES" ] || {
    echo 'PT_LOAD byte count changed across two clean builds' >&2
    exit 1
}

echo 'ISSUE7_LINKED_REPRODUCIBILITY_VERSION=1'
echo "BUILD1_ELF_SHA256=$BUILD1_SHA"
echo "BUILD2_ELF_SHA256=$BUILD2_SHA"
echo 'WHOLE_ELF_REPRODUCIBLE=YES'
echo "BUILD1_PT_LOAD_SHA256=$BUILD1_PT_SHA"
echo "BUILD2_PT_LOAD_SHA256=$BUILD2_PT_SHA"
echo "PT_LOAD_BYTES=$BUILD1_PT_BYTES"
echo 'PT_LOAD_REPRODUCIBLE=YES'
echo 'ISSUE7_LINKED_REPRODUCIBILITY=PASS'
