#!/usr/bin/env bash
# File synopsis:
# Re-run the final Issue #7 linked reproducibility proof from its exact recorded
# source authority without compiling a later reconstruction stage against the
# historical Issue #7 object set.
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

ISSUE7_SOURCE_COMMIT='d94c9280035e288ccbec692ea21e89d3ffb4ffec'

EXPECTED_ELF_SHA='39984bfb5e30a796c07c4cfabf1521084efd6dfad0dbb657f9747bbb322ae796'
EXPECTED_PT_LOAD_SHA='17a36c3ce541b5dd0b309163e0ecfd4b9f9d8284bbbc5ca1787b2738fb6e4f5e'
EXPECTED_PT_LOAD_BYTES='335240'

TMP="$(
    mktemp -d \
        /tmp/ps2vnc-historical-issue7.XXXXXX
)"

SNAP="$TMP/repo"

cleanup()
{
    rm -rf "$TMP"
}

trap cleanup EXIT

git -C "$ROOT" \
    cat-file \
    -e \
    "$ISSUE7_SOURCE_COMMIT^{commit}" || {
        echo \
            "ERROR: recorded Issue #7 source commit is not available: $ISSUE7_SOURCE_COMMIT" \
            >&2
        exit 1
    }

mkdir -p "$SNAP"

git -C "$ROOT" \
    archive \
    --format=tar \
    "$ISSUE7_SOURCE_COMMIT" |
tar \
    -xf - \
    -C "$SNAP"

[ -x "$SNAP/scripts/check-issue7-linked-reproducibility.sh" ] || {
    echo \
        'ERROR: recorded Issue #7 reproducibility tool is unavailable or non-executable.' \
        >&2
    exit 1
}

echo "HISTORICAL_STAGE=ISSUE7"
echo "HISTORICAL_SOURCE_COMMIT=$ISSUE7_SOURCE_COMMIT"

(
    cd "$SNAP"

    ./scripts/check-issue7-linked-reproducibility.sh
) | tee "$TMP/repro.log"

grep -Fq \
    'ISSUE7_LINKED_REPRODUCIBILITY=PASS' \
    "$TMP/repro.log" || {
        echo \
            'ERROR: recorded Issue #7 reproducibility proof failed.' \
            >&2
        exit 1
    }

ELF="$SNAP/build/reconstruction/issue7/PS-to-VNC-Issue7.ELF"

[ -f "$ELF" ] || {
    echo \
        'ERROR: recorded Issue #7 reproducibility run did not produce its ELF.' \
        >&2
    exit 1
}

ACTUAL_ELF_SHA="$(
    sha256sum "$ELF" |
    awk '{print $1}'
)"

"$SNAP/scripts/testkit/pt-load-fingerprint.sh" \
    "$ELF" \
    >"$TMP/pt-load.env"

ACTUAL_PT_LOAD_SHA="$(
    sed -n \
        's/^PT_LOAD_SHA256=//p' \
        "$TMP/pt-load.env"
)"

ACTUAL_PT_LOAD_BYTES="$(
    sed -n \
        's/^PT_LOAD_BYTES=//p' \
        "$TMP/pt-load.env"
)"

[ "$ACTUAL_ELF_SHA" = "$EXPECTED_ELF_SHA" ] || {
    echo \
        'ERROR: historical Issue #7 whole-ELF identity mismatch.' \
        >&2
    echo "EXPECTED=$EXPECTED_ELF_SHA" >&2
    echo "ACTUAL=$ACTUAL_ELF_SHA" >&2
    exit 1
}

[ "$ACTUAL_PT_LOAD_SHA" = "$EXPECTED_PT_LOAD_SHA" ] || {
    echo \
        'ERROR: historical Issue #7 PT_LOAD identity mismatch.' \
        >&2
    echo "EXPECTED=$EXPECTED_PT_LOAD_SHA" >&2
    echo "ACTUAL=$ACTUAL_PT_LOAD_SHA" >&2
    exit 1
}

[ "$ACTUAL_PT_LOAD_BYTES" = "$EXPECTED_PT_LOAD_BYTES" ] || {
    echo \
        'ERROR: historical Issue #7 PT_LOAD byte count mismatch.' \
        >&2
    echo "EXPECTED=$EXPECTED_PT_LOAD_BYTES" >&2
    echo "ACTUAL=$ACTUAL_PT_LOAD_BYTES" >&2
    exit 1
}

echo "ISSUE7_HISTORICAL_ELF_SHA256=$ACTUAL_ELF_SHA"
echo "ISSUE7_HISTORICAL_PT_LOAD_SHA256=$ACTUAL_PT_LOAD_SHA"
echo "ISSUE7_HISTORICAL_PT_LOAD_BYTES=$ACTUAL_PT_LOAD_BYTES"
echo 'ISSUE7_HISTORICAL_REPRODUCIBILITY=PASS'
