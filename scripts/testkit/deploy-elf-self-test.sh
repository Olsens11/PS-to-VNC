#!/usr/bin/env bash

# File synopsis:
# Prove generic repository selection, dry-run isolation, authorization,
# deterministic dual-target FTP deployment, collision protection, exact
# readback identity, and evidence creation without contacting a real PS2.

set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd -P
)"

TOOL="$ROOT/scripts/testkit/deploy-elf.py"

python3 - "$TOOL" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])

compile(
    path.read_text(encoding="utf-8"),
    str(path),
    "exec",
)
PY

mkdir -p "$ROOT/build/testkit"

TMP="$(
    mktemp -d \
        "$ROOT/build/testkit/.deploy-self-test.XXXXXX"
)"

cleanup()
{
    rm -rf "$TMP"
}

trap cleanup EXIT

ELF="$TMP/DUT.ELF"
FAKE_FTP="$TMP/fake-ftp"
FAKE_CURL="$TMP/fake-curl.py"
CONTACTS="$TMP/ftp-contacts.log"
EVIDENCE="$TMP/deployment.json"
SECOND_EVIDENCE="$TMP/deployment-second.json"
THIRD_EVIDENCE="$TMP/deployment-third.json"
TEST_ID='GENERIC-DEPLOY-SELFTEST'

printf '%s\n' \
    'generic PS-to-VNC deployment fixture' \
    > "$ELF"

ELF_SHA="$(
    sha256sum "$ELF" |
    awk '{print $1}'
)"

ELF_BYTES="$(
    wc -c < "$ELF"
)"

ELF_REL="${ELF#"$ROOT/"}"
EVIDENCE_REL="${EVIDENCE#"$ROOT/"}"

cat > "$FAKE_CURL" <<'PY'
#!/usr/bin/env python3

from pathlib import Path
import os
import shutil
import sys
from urllib.parse import urlparse


arguments = sys.argv[1:]

root = Path(
    os.environ["FAKE_FTP_ROOT"]
)

contact_log = Path(
    os.environ["FAKE_FTP_CONTACT_LOG"]
)

contact_log.parent.mkdir(
    parents=True,
    exist_ok=True,
)

with contact_log.open(
    "a",
    encoding="utf-8",
) as stream:
    stream.write("CONTACT\n")


urls = [
    argument
    for argument in arguments
    if argument.startswith("ftp://")
]

if len(urls) != 1:
    raise SystemExit(90)

url = urls[0]
remote = (
    root
    / urlparse(url).path.lstrip("/")
)

if "--upload-file" in arguments:
    index = arguments.index(
        "--upload-file"
    )

    source = Path(
        arguments[index + 1]
    )

    remote.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    shutil.copyfile(
        source,
        remote,
    )

    raise SystemExit(0)


if "-o" not in arguments:
    raise SystemExit(91)

index = arguments.index("-o")

output = Path(
    arguments[index + 1]
)

if not remote.is_file():
    raise SystemExit(78)

shutil.copyfile(
    remote,
    output,
)
PY

chmod 755 "$FAKE_CURL"

run_tool()
{
    TESTKIT_SELFTEST=1 \
    PS2VNC_DEPLOY_CURL="$FAKE_CURL" \
    FAKE_FTP_ROOT="$FAKE_FTP" \
    FAKE_FTP_CONTACT_LOG="$CONTACTS" \
    python3 "$TOOL" "$@"
}

echo '[SELFTEST] explicit repository dry-run'

run_tool \
    --repo "$ROOT" \
    --elf "$ELF_REL" \
    --test-id "$TEST_ID" \
    --expected-sha256 "$ELF_SHA" \
    --expected-bytes "$ELF_BYTES" \
    --evidence "$EVIDENCE_REL" \
    --dry-run \
    > "$TMP/dry-run-explicit.log"

grep -qx \
    "REPOSITORY_CONTEXT=$ROOT" \
    "$TMP/dry-run-explicit.log"

grep -qx \
    'REPOSITORY_CONTEXT_SOURCE=EXPLICIT' \
    "$TMP/dry-run-explicit.log"

grep -qx \
    'FTP_CONTACT=NO' \
    "$TMP/dry-run-explicit.log"

grep -qx \
    'DEPLOY_ELF_DRY_RUN=PASS' \
    "$TMP/dry-run-explicit.log"

[ ! -e "$CONTACTS" ]
[ ! -e "$EVIDENCE" ]

echo '[SELFTEST] auto-discovered repository dry-run'

(
    cd "$ROOT"

    run_tool \
        --elf "$ELF_REL" \
        --test-id "$TEST_ID" \
        --expected-sha256 "$ELF_SHA" \
        --expected-bytes "$ELF_BYTES" \
        --evidence "$EVIDENCE_REL" \
        --dry-run \
        > "$TMP/dry-run-auto.log"
)

grep -qx \
    'REPOSITORY_CONTEXT_SOURCE=AUTO_DISCOVERED' \
    "$TMP/dry-run-auto.log"

grep -qx \
    'FTP_CONTACT=NO' \
    "$TMP/dry-run-auto.log"

[ ! -e "$CONTACTS" ]

echo '[SELFTEST] live action requires authorization'

if run_tool \
    --repo "$ROOT" \
    --elf "$ELF_REL" \
    --test-id "$TEST_ID" \
    --expected-sha256 "$ELF_SHA" \
    --expected-bytes "$ELF_BYTES" \
    --evidence "$EVIDENCE_REL" \
    > "$TMP/unauthorized.log" 2>&1
then
    echo \
        'live deployment accepted without authorization' \
        >&2
    exit 1
fi

[ ! -e "$CONTACTS" ]

echo '[SELFTEST] expected identity fails before FTP'

if run_tool \
    --repo "$ROOT" \
    --elf "$ELF_REL" \
    --test-id "$TEST_ID" \
    --expected-sha256 \
        0000000000000000000000000000000000000000000000000000000000000000 \
    --evidence "$EVIDENCE_REL" \
    --operator-authorized \
    > "$TMP/bad-sha.log" 2>&1
then
    echo \
        'deployment accepted incorrect expected SHA256' \
        >&2
    exit 1
fi

[ ! -e "$CONTACTS" ]

echo '[SELFTEST] exact dual-target deployment'

run_tool \
    --repo "$ROOT" \
    --elf "$ELF_REL" \
    --test-id "$TEST_ID" \
    --expected-sha256 "$ELF_SHA" \
    --expected-bytes "$ELF_BYTES" \
    --evidence "$EVIDENCE_REL" \
    --operator-authorized \
    > "$TMP/deploy.log"

[ "$(wc -l < "$CONTACTS")" -eq 5 ]

grep -qx \
    'FTP_CONTACT=YES' \
    "$TMP/deploy.log"

grep -qx \
    'DEPLOY_ELF=PASS' \
    "$TMP/deploy.log"

UNIQUE="$(
    sed -n \
        's/^UNIQUE_REMOTE=//p' \
        "$TMP/deploy.log" |
    tail -1
)"

ROLLING="$(
    sed -n \
        's/^ROLLING_REMOTE=//p' \
        "$TMP/deploy.log" |
    tail -1
)"

[ "$ROLLING" = '/mass/0/PS2VNC.ELF' ]

UNIQUE_FILE="$FAKE_FTP${UNIQUE}"
ROLLING_FILE="$FAKE_FTP${ROLLING}"

[ "$(
    sha256sum "$UNIQUE_FILE" |
    awk '{print $1}'
)" = "$ELF_SHA" ]

[ "$(
    sha256sum "$ROLLING_FILE" |
    awk '{print $1}'
)" = "$ELF_SHA" ]

[ "$(
    wc -c < "$UNIQUE_FILE"
)" -eq "$ELF_BYTES" ]

[ "$(
    wc -c < "$ROLLING_FILE"
)" -eq "$ELF_BYTES" ]

python3 - \
    "$EVIDENCE" \
    "$ROOT" \
    "$ELF_SHA" \
    "$ELF_BYTES" \
    "$UNIQUE" \
    "$ROLLING" <<'PY'
import json
from pathlib import Path
import sys


(
    evidence_raw,
    repository,
    expected_sha,
    expected_bytes_raw,
    unique,
    rolling,
) = sys.argv[1:]

record = json.loads(
    Path(evidence_raw).read_text(
        encoding="utf-8"
    )
)

expected_bytes = int(
    expected_bytes_raw
)

assert record["deployment_result"] == "PASS"
assert record["repository_context"] == repository
assert record["repository_context_source"] == "EXPLICIT"
assert record["elf_sha256"] == expected_sha
assert record["elf_bytes"] == expected_bytes
assert record["unique_remote"] == unique
assert record["rolling_remote"] == rolling
assert record["unique_readback_sha256"] == expected_sha
assert record["unique_readback_bytes"] == expected_bytes
assert record["rolling_readback_sha256"] == expected_sha
assert record["rolling_readback_bytes"] == expected_bytes
assert record["ftp_readback"] == "PASS"
assert record["hardware_run"] is False
PY

echo '[SELFTEST] existing evidence fails before FTP'

BEFORE_CONTACTS="$(
    wc -l < "$CONTACTS"
)"

if run_tool \
    --repo "$ROOT" \
    --elf "$ELF_REL" \
    --test-id "$TEST_ID" \
    --expected-sha256 "$ELF_SHA" \
    --expected-bytes "$ELF_BYTES" \
    --evidence "$EVIDENCE_REL" \
    --operator-authorized \
    > "$TMP/existing-evidence.log" 2>&1
then
    echo \
        'deployment accepted existing evidence path' \
        >&2
    exit 1
fi

[ "$BEFORE_CONTACTS" -eq "$(
    wc -l < "$CONTACTS"
)" ]

echo '[SELFTEST] unique archival collision fails closed'

SECOND_REL="${SECOND_EVIDENCE#"$ROOT/"}"

BEFORE_CONTACTS="$(
    wc -l < "$CONTACTS"
)"

if run_tool \
    --repo "$ROOT" \
    --elf "$ELF_REL" \
    --test-id "$TEST_ID" \
    --expected-sha256 "$ELF_SHA" \
    --expected-bytes "$ELF_BYTES" \
    --evidence "$SECOND_REL" \
    --operator-authorized \
    > "$TMP/unique-collision.log" 2>&1
then
    echo \
        'deployment overwrote existing unique archival target' \
        >&2
    exit 1
fi

AFTER_CONTACTS="$(
    wc -l < "$CONTACTS"
)"

[ "$AFTER_CONTACTS" -eq "$((BEFORE_CONTACTS + 1))" ]
[ ! -e "$SECOND_EVIDENCE" ]

echo '[SELFTEST] post-plan local mutation fails before FTP'

printf '\001' >> "$ELF"

THIRD_REL="${THIRD_EVIDENCE#"$ROOT/"}"

BEFORE_CONTACTS="$(
    wc -l < "$CONTACTS"
)"

if run_tool \
    --repo "$ROOT" \
    --elf "$ELF_REL" \
    --test-id 'GENERIC-DEPLOY-TAMPER' \
    --expected-sha256 "$ELF_SHA" \
    --expected-bytes "$ELF_BYTES" \
    --evidence "$THIRD_REL" \
    --operator-authorized \
    > "$TMP/tampered.log" 2>&1
then
    echo \
        'deployment accepted locally mutated ELF' \
        >&2
    exit 1
fi

[ "$BEFORE_CONTACTS" -eq "$(
    wc -l < "$CONTACTS"
)" ]

echo 'DEPLOY_ELF_SELF_TEST_VERSION=1'
echo 'EXPLICIT_REPOSITORY_CONTEXT=PASS'
echo 'AUTO_DISCOVERED_REPOSITORY_CONTEXT=PASS'
echo 'RELATIVE_ELF_PATH=PASS'
echo 'RELATIVE_EVIDENCE_PATH=PASS'
echo 'DRY_RUN_FTP_CONTACT=NO'
echo 'OPERATOR_AUTHORIZATION=FAIL_CLOSED'
echo 'EXPECTED_IDENTITY=FAIL_CLOSED'
echo 'UNIQUE_ARCHIVAL_COLLISION=FAIL_CLOSED'
echo 'UNIQUE_DEPLOY_READBACK_SHA256_BYTES=PASS'
echo 'ROLLING_DEPLOY_READBACK_SHA256_BYTES=PASS'
echo 'ROLLING_PS2VNC_ELF_IS_NORMAL_DEPLOYMENT=PASS'
echo 'GENERIC_DEPLOYMENT_EVIDENCE=PASS'
echo 'POST_PLAN_MUTATION_FAILS_BEFORE_FTP=PASS'
echo 'REAL_PS2_CONTACT=NO'
echo 'DEPLOY_ELF_SELF_TEST=PASS'
