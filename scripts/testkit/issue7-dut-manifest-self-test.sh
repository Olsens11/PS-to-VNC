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

# TRACKED_INPUTS in issue7-dut-manifest.py is the provenance authority.
# Derive this regression from that list instead of maintaining a second,
# incomplete list of manifest hash fields here.
python3 - \
    "$MANIFEST" \
    "$TMP/manifest-a.env" <<'PYHASH'
from pathlib import Path
import ast
import hashlib
import sys


manifest_source = Path(
    sys.argv[1]
).resolve()

generated_manifest = Path(
    sys.argv[2]
).resolve()

tree = ast.parse(
    manifest_source.read_text(
        encoding="utf-8"
    )
)

tracked_inputs = None

for node in tree.body:
    if not isinstance(node, ast.Assign):
        continue

    if any(
        isinstance(target, ast.Name)
        and target.id == "TRACKED_INPUTS"
        for target in node.targets
    ):
        tracked_inputs = ast.literal_eval(
            node.value
        )
        break

if tracked_inputs is None:
    raise SystemExit(
        "TRACKED_INPUTS not found in DUT manifest tool"
    )

if not isinstance(tracked_inputs, tuple):
    raise SystemExit(
        "TRACKED_INPUTS is not a tuple"
    )

root = manifest_source.parents[2]

values = {}

for number, raw in enumerate(
    generated_manifest.read_text(
        encoding="utf-8"
    ).splitlines(),
    start=1,
):
    if not raw or raw.startswith("#"):
        continue

    if "=" not in raw:
        raise SystemExit(
            "malformed generated manifest line "
            f"{number}: {raw!r}"
        )

    key, value = raw.split("=", 1)

    if key in values:
        raise SystemExit(
            f"duplicate generated manifest field: {key}"
        )

    values[key] = value


def manifest_hash_key(relative: str) -> str:
    return (
        relative.upper()
        .replace("/", "_")
        .replace("-", "_")
        .replace(".", "_")
        + "_SHA256"
    )


def sha256(path: Path) -> str:
    digest = hashlib.sha256()

    with path.open("rb") as stream:
        for block in iter(
            lambda: stream.read(1024 * 1024),
            b"",
        ):
            digest.update(block)

    return digest.hexdigest()


checked = 0

for relative in tracked_inputs:
    source = root / relative

    if not source.is_file():
        raise SystemExit(
            f"tracked manifest input missing: {relative}"
        )

    key = manifest_hash_key(
        relative
    )

    expected = sha256(
        source
    )

    actual = values.get(
        key
    )

    if actual != expected:
        raise SystemExit(
            "tracked-input manifest hash mismatch "
            f"{key}: expected={expected} actual={actual}"
        )

    checked += 1

obsolete = (
    "SCRIPTS_TESTKIT_ISSUE7_DEPLOY_ELF_SH_SHA256"
)

if obsolete in values:
    raise SystemExit(
        "obsolete Issue #7 deployer hash field "
        "reappeared in generated manifest"
    )

print(
    f"ISSUE7_MANIFEST_TRACKED_INPUT_COUNT={checked}"
)

print(
    "ISSUE7_MANIFEST_TRACKED_INPUT_HASHES=PASS"
)

print(
    "OBSOLETE_ISSUE7_DEPLOY_HASH_FIELD=0"
)
PYHASH

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

echo 'ISSUE7_DUT_MANIFEST_SELF_TEST_VERSION=3'
echo "MANIFEST_SHA256=$sha_a"
echo 'CANONICAL_CLEAN_BUILD_BINDING=PASS'
echo 'DETERMINISTIC_MANIFEST=PASS'
echo 'SOURCE_AUTHORITY_FIELDS=PASS'
echo 'PREPARATION_EVIDENCE_FIELDS=PASS'
echo 'NON_QUALIFICATION_DEFAULTS=PASS'
echo 'EXISTING_OUTPUT_FAIL_CLOSED=PASS'
echo 'POST_PREPARATION_TAMPER_FAIL_CLOSED=PASS'
echo 'ISSUE7_DUT_MANIFEST_SELF_TEST=PASS'
