#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd
)"

cd "$ROOT"

PREP="$ROOT/scripts/testkit/prepare-hardware-elf.sh"
MANIFEST="$ROOT/scripts/testkit/issue7-dut-manifest.py"
PRISTINE="$ROOT/build/reconstruction/issue7/PS-to-VNC-Issue7.ELF"
TEST_ID="${TESTKIT_MANIFEST_SELF_TEST_ID:-ISSUE7-MANIFEST-SELFTEST}"

[ -x "$PREP" ] || {
    echo "required executable missing: $PREP" >&2
    exit 1
}

[ -f "$MANIFEST" ] || {
    echo "manifest tool missing: $MANIFEST" >&2
    exit 1
}

[ -f "$PRISTINE" ] || {
    echo \
        "canonical clean Issue 7 ELF missing: $PRISTINE" \
        >&2
    echo \
        'run the linked reproducibility gate before this self-test' \
        >&2
    exit 1
}

python3 - "$MANIFEST" <<'PYCOMPILE'
from pathlib import Path
import sys

path = Path(sys.argv[1])

compile(
    path.read_text(encoding="utf-8"),
    str(path),
    "exec",
)

print("ISSUE7_DUT_MANIFEST_PYTHON_SYNTAX=PASS")
PYCOMPILE

TMP="$(mktemp -d)"
cleanup()
{
    rm -rf "$TMP"
}
trap cleanup EXIT

"$PREP" \
    "$PRISTINE" \
    "$TEST_ID" \
    "$TMP/dut.ELF" \
    > "$TMP/preparation.env"

python3 "$MANIFEST" \
    "$TMP/preparation.env" \
    "$TMP/manifest-a.env" \
    > "$TMP/manifest-a.log"

python3 "$MANIFEST" \
    "$TMP/preparation.env" \
    "$TMP/manifest-b.env" \
    > "$TMP/manifest-b.log"

cmp -s "$TMP/manifest-a.env" "$TMP/manifest-b.env"

grep -qx 'ISSUE7_DUT_MANIFEST_VERSION=1' "$TMP/manifest-a.env"
grep -qx 'QUALIFICATION_SCOPE=PRE_HARDWARE_IDENTITY_ONLY' "$TMP/manifest-a.env"
grep -qx 'HARDWARE_QUALIFIED=NO' "$TMP/manifest-a.env"
grep -qx 'SOURCE_REPOSITORY=Olsens11/PS-to-VNC' "$TMP/manifest-a.env"
grep -qx 'SOURCE_BRANCH=reconstruct/issue7-minimal-core' "$TMP/manifest-a.env"
grep -qx \
    'PRISTINE_BUILD_PATH=build/reconstruction/issue7/PS-to-VNC-Issue7.ELF' \
    "$TMP/manifest-a.env"
grep -qx "HARDWARE_TEST_ID=$TEST_ID" "$TMP/manifest-a.env"
grep -qx 'IDENTITY_STAMP_REPRODUCIBLE=YES' "$TMP/manifest-a.env"
grep -qx 'DEPLOYED=NO' "$TMP/manifest-a.env"
grep -qx 'RUNTIME_IDENTITY_OBSERVED=NO' "$TMP/manifest-a.env"
grep -qx 'MACHINE_EVIDENCE_RESULT=NOT_RUN' "$TMP/manifest-a.env"
grep -qx 'OPERATOR_RESULT=NOT_RUN' "$TMP/manifest-a.env"

grep -Eq '^SOURCE_COMMIT=[0-9a-f]{40}$' "$TMP/manifest-a.env"
grep -Eq '^PS2DEV_IMAGE=ps2dev/ps2dev@sha256:[0-9a-f]{64}$' "$TMP/manifest-a.env"
grep -Eq '^PS2IP_SHA256=[0-9a-f]{64}$' "$TMP/manifest-a.env"

for key in \
    SCRIPTS_TESTKIT_VERIFY_ELF_IDENTITY_SH_SHA256 \
    SCRIPTS_TESTKIT_ISSUE7_DUT_MANIFEST_PY_SHA256 \
    SCRIPTS_TESTKIT_ISSUE7_APPARATUS_COMMON_SH_SHA256 \
    SCRIPTS_TESTKIT_ISSUE7_ARM_OBSERVERS_SH_SHA256 \
    SCRIPTS_TESTKIT_ISSUE7_STOP_OBSERVERS_SH_SHA256 \
    SCRIPTS_TESTKIT_ISSUE7_UDP_OBSERVER_PY_SHA256 \
    SCRIPTS_TESTKIT_ISSUE7_RESULT_PY_SHA256 \
    SCRIPTS_TESTKIT_ISSUE7_APPARATUS_SELF_TEST_SH_SHA256
do
    grep -Eq \
        "^${key}=[0-9a-f]{64}$" \
        "$TMP/manifest-a.env"
done

if python3 "$MANIFEST" \
    "$TMP/preparation.env" \
    "$TMP/manifest-a.env" \
    > "$TMP/existing-output.log" 2>&1
then
    echo 'manifest tool unexpectedly overwrote existing output' >&2
    exit 1
fi

printf '\001' >> "$TMP/dut.ELF"
if python3 "$MANIFEST" \
    "$TMP/preparation.env" \
    "$TMP/tampered.env" \
    > "$TMP/tampered.log" 2>&1
then
    echo 'manifest tool accepted a stamped ELF changed after preparation' >&2
    exit 1
fi

sha_a="$(sha256sum "$TMP/manifest-a.env" | awk '{print $1}')"
sha_b="$(sha256sum "$TMP/manifest-b.env" | awk '{print $1}')"
[ "$sha_a" = "$sha_b" ]

echo 'ISSUE7_DUT_MANIFEST_SELF_TEST_VERSION=2'
echo "MANIFEST_SHA256=$sha_a"
echo 'CANONICAL_CLEAN_BUILD_BINDING=PASS'
echo 'DETERMINISTIC_MANIFEST=PASS'
echo 'SOURCE_AUTHORITY_FIELDS=PASS'
echo 'PREPARATION_EVIDENCE_FIELDS=PASS'
echo 'NON_QUALIFICATION_DEFAULTS=PASS'
echo 'EXISTING_OUTPUT_FAIL_CLOSED=PASS'
echo 'POST_PREPARATION_TAMPER_FAIL_CLOSED=PASS'
echo 'ISSUE7_DUT_MANIFEST_SELF_TEST=PASS'
