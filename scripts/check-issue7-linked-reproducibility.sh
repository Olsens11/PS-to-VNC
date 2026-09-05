#!/usr/bin/env bash
# File synopsis:
# Issue #7 stage-local linked reproducibility proof.
#
# At the exact final Issue #7 source authority this builds twice and requires
# whole-ELF plus PT_LOAD reproducibility.
#
# At any other reconstruction stage it is intentionally a successful no-op:
# it reports SKIPPED / NOT_APPLICABLE, performs no build, and does not interrupt
# an unrelated workflow. Use check-historical-issue7-reproducibility.sh when
# actual Issue #7 evidence is required from a later checkout.
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd
)"

# STAGE_LOCAL_ONLY_ISSUE7
#
# This is the original Issue #7 stage-local reproducibility mechanism.
# It is applicable only when the checkout itself is the exact final Issue #7
# source authority.
#
# Later reconstruction stages legitimately contain source and modules that do
# not belong to the historical Issue #7 object set. Encountering this check in
# such a workflow is therefore NOT_APPLICABLE, not a project failure.
#
# Outside the exact Issue #7 authority this script:
#
#   - performs no build;
#   - changes no build output;
#   - reports an explicit SKIPPED / NOT_APPLICABLE result;
#   - exits successfully so an unrelated later-stage workflow can continue.
#
# When an actual historical Issue #7 proof is required from a later checkout,
# use:
#
#     ./scripts/check-historical-issue7-reproducibility.sh
#
# Never add later-stage objects to the Issue #7 makefile merely to make this
# stage-local mechanism operate on current source.

ISSUE7_STAGE_LOCAL_SOURCE_AUTHORITY='d94c9280035e288ccbec692ea21e89d3ffb4ffec'

CURRENT_SOURCE_COMMIT="$(
    git -C "$ROOT"         rev-parse HEAD
)"

if [ "$CURRENT_SOURCE_COMMIT" != "$ISSUE7_STAGE_LOCAL_SOURCE_AUTHORITY" ]
then
    echo 'ISSUE7_LINKED_REPRODUCIBILITY_APPLICABILITY=NOT_APPLICABLE'
    echo 'ISSUE7_LINKED_REPRODUCIBILITY=SKIPPED'
    echo 'ISSUE7_LINKED_REPRODUCIBILITY_BUILD_RUN=NO'
    echo 'ISSUE7_LINKED_REPRODUCIBILITY_WORKFLOW_BLOCKING=NO'
    echo 'ISSUE7_LINKED_REPRODUCIBILITY_SKIP_REASON=CHECKOUT_IS_NOT_FINAL_ISSUE7_AUTHORITY'
    echo "CURRENT_SOURCE_COMMIT=$CURRENT_SOURCE_COMMIT"
    echo "REQUIRED_ISSUE7_SOURCE_COMMIT=$ISSUE7_STAGE_LOCAL_SOURCE_AUTHORITY"
    echo 'HISTORICAL_REPRO_COMMAND=./scripts/check-historical-issue7-reproducibility.sh'
    echo 'NOTE=Issue #7 stage-local reproducibility is not applicable to this checkout; no build was run.'
    exit 0
fi

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
