#!/usr/bin/env bash
# File synopsis:
# Deploy one exact Issue #7 stamped DUT to unique and rolling PS2 paths.
#
# Both FTP targets are downloaded after upload and must hash back to the
# manifest's exact stamped ELF identity before deployment is accepted.

set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd -P
)"

cd "$ROOT"

# shellcheck source=issue7-apparatus-common.sh
source scripts/testkit/issue7-apparatus-common.sh

[ "$#" -eq 3 ] ||
    issue7_die \
        'usage: issue7-deploy-elf.sh DUT_MANIFEST STAMPED_ELF RUN'

MANIFEST="$(realpath -e "$1")"
ELF="$(realpath -e "$2")"

issue7_validate_manifest_current "$MANIFEST"

RUN="$(issue7_run_abs "$3")"
issue7_load_control "$RUN"

[ "$(issue7_sha256 "$MANIFEST")" = \
  "$(issue7_sha256 "$RUN/manifest.env")" ] ||
    issue7_die 'deployment manifest differs from armed-run manifest'

[ "$TEST_ID" = "$ISSUE7_TEST_ID" ] ||
    issue7_die 'armed-run test ID differs from deployment manifest'

issue7_pid_owned "$UDP_PID" "$RUN" ||
    issue7_die 'UDP observer is not alive and owned before deployment'

issue7_pcap_active "$PCAP_UNIT" ||
    issue7_die 'packet capture is not active before deployment'

issue7_pcap_owned "$PCAP_UNIT" "$PCAP_PATH" ||
    issue7_die 'packet capture ownership mismatch before deployment'

[ "$(issue7_sha256 "$ELF")" = \
  "$ISSUE7_STAMPED_ELF_SHA256" ] ||
    issue7_die 'stamped ELF SHA256 does not match DUT manifest'

"$ROOT/scripts/testkit/verify-elf-identity.sh" \
    "$ELF" \
    "$ISSUE7_TEST_ID" \
    "$ISSUE7_IDENTITY_SHA256"

command -v curl >/dev/null 2>&1 ||
    issue7_die 'required command unavailable: curl'

SHORT_SHA="${ISSUE7_STAMPED_ELF_SHA256:0:8}"

UNIQUE_REMOTE="/mass/0/PS2VNC-$ISSUE7_TEST_ID-$SHORT_SHA.ELF"
ROLLING_REMOTE='/mass/0/PS2VNC.ELF'

BASE="ftp://$ISSUE7_PS2_HOST:$ISSUE7_PS2_FTP_PORT"

TMP="$(mktemp -d)"

cleanup()
{
    rm -rf "$TMP"
}

trap cleanup EXIT

curl \
    --fail \
    --silent \
    --show-error \
    --upload-file "$ELF" \
    "$BASE$UNIQUE_REMOTE"

curl \
    --fail \
    --silent \
    --show-error \
    --upload-file "$ELF" \
    "$BASE$ROLLING_REMOTE"

curl \
    --fail \
    --silent \
    --show-error \
    "$BASE$UNIQUE_REMOTE" \
    -o "$TMP/unique.ELF"

curl \
    --fail \
    --silent \
    --show-error \
    "$BASE$ROLLING_REMOTE" \
    -o "$TMP/rolling.ELF"

UNIQUE_READBACK_SHA256="$(
    issue7_sha256 "$TMP/unique.ELF"
)"

ROLLING_READBACK_SHA256="$(
    issue7_sha256 "$TMP/rolling.ELF"
)"

[ "$UNIQUE_READBACK_SHA256" = \
  "$ISSUE7_STAMPED_ELF_SHA256" ] ||
    issue7_die 'unique FTP readback SHA256 mismatch'

[ "$ROLLING_READBACK_SHA256" = \
  "$ISSUE7_STAMPED_ELF_SHA256" ] ||
    issue7_die 'rolling FTP readback SHA256 mismatch'

python3 - \
    "$RUN/deployment.json" \
    "$ISSUE7_TEST_ID" \
    "$ELF" \
    "$ISSUE7_STAMPED_ELF_SHA256" \
    "$UNIQUE_REMOTE" \
    "$ROLLING_REMOTE" \
    "$UNIQUE_READBACK_SHA256" \
    "$ROLLING_READBACK_SHA256" <<'PY'
from datetime import datetime, timezone
import json
from pathlib import Path
import sys

(
    output,
    test_id,
    elf,
    elf_sha,
    unique,
    rolling,
    unique_sha,
    rolling_sha,
) = sys.argv[1:]

record = {
    "version": 1,
    "deployed_at": (
        datetime.now(timezone.utc)
        .astimezone()
        .isoformat()
    ),
    "test_id": test_id,
    "elf_path": elf,
    "elf_sha256": elf_sha,
    "unique_remote": unique,
    "rolling_remote": rolling,
    "unique_readback_sha256": unique_sha,
    "rolling_readback_sha256": rolling_sha,
}

Path(output).write_text(
    json.dumps(
        record,
        sort_keys=True,
        indent=2,
    )
    + "\n",
    encoding="utf-8",
)
PY

printf '%s\n' 'ISSUE7_DEPLOYMENT=PASS'
printf 'TEST_ID=%s\n' "$ISSUE7_TEST_ID"
printf 'ELF_SHA256=%s\n' "$ISSUE7_STAMPED_ELF_SHA256"
printf 'UNIQUE_REMOTE=%s\n' "$UNIQUE_REMOTE"
printf 'ROLLING_REMOTE=%s\n' "$ROLLING_REMOTE"
printf 'UNIQUE_READBACK_SHA256=%s\n' "$UNIQUE_READBACK_SHA256"
printf 'ROLLING_READBACK_SHA256=%s\n' "$ROLLING_READBACK_SHA256"
printf 'NEXT_OPERATOR_ACTION=LAUNCH_PS2VNC\n'
