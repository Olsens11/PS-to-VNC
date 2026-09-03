#!/usr/bin/env bash
# File synopsis:
# Exercise successor Issue #7 apparatus logic without PS2, FTP, sudo, or tcpdump.
#
# The test covers shell parsing, Python syntax, the real UDP receiver, and
# positive/negative startup-diagnostic ordering.

set -euo pipefail

# Dynamic import tests must not leave bytecode in the tracked source tree.
export PYTHONDONTWRITEBYTECODE=1

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd -P
)"

cd "$ROOT"

COMMON='scripts/testkit/issue7-apparatus-common.sh'
ARM='scripts/testkit/issue7-arm-observers.sh'
DEPLOY='scripts/testkit/issue7-deploy-elf.sh'
STOP='scripts/testkit/issue7-stop-observers.sh'
UDP='scripts/testkit/issue7-udp-observer.py'
RESULT='scripts/testkit/issue7-result.py'

for script in \
    "$COMMON" \
    "$ARM" \
    "$DEPLOY" \
    "$STOP" \
    "$0"
do
    bash -n "$script"
done

python3 - "$UDP" "$RESULT" <<'PY'
from pathlib import Path
import sys

for raw in sys.argv[1:]:
    path = Path(raw)
    compile(
        path.read_text(encoding="utf-8"),
        str(path),
        "exec",
    )

print("ISSUE7_APPARATUS_PYTHON_SYNTAX=PASS")
PY

TMP="$(mktemp -d)"

cleanup()
{
    if [ -n "${UDP_PID:-}" ]; then
        kill -TERM "$UDP_PID" 2>/dev/null || true
        wait "$UDP_PID" 2>/dev/null || true
    fi

    rm -rf "$TMP"
}

trap cleanup EXIT

MANIFEST="$TMP/manifest.env"

cat > "$MANIFEST" <<'MANIFEST'
ISSUE7_DUT_MANIFEST_VERSION=1
QUALIFICATION_SCOPE=PRE_HARDWARE_IDENTITY_ONLY
HARDWARE_QUALIFIED=NO
SOURCE_REPOSITORY=Olsens11/PS-to-VNC
SOURCE_BRANCH=reconstruct/issue7-minimal-core
SOURCE_COMMIT=1111111111111111111111111111111111111111
PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74
HARDWARE_TEST_ID=ISSUE7-SELFTEST
HARDWARE_ELF_IDENTITY_SHA256=2222222222222222222222222222222222222222222222222222222222222222
HARDWARE_STAMPED_ELF_SHA256=3333333333333333333333333333333333333333333333333333333333333333
MANIFEST

VALUE="$(
    bash -c '
        source "$1"
        issue7_manifest_value HARDWARE_TEST_ID "$2"
    ' _ "$COMMON" "$MANIFEST"
)"

[ "$VALUE" = 'ISSUE7-SELFTEST' ]

PORT="$(
    python3 - <<'PY'
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("127.0.0.1", 0))
print(sock.getsockname()[1])
sock.close()
PY
)"

UDP_LOG="$TMP/udp.jsonl"
UDP_CONSOLE="$TMP/udp-console.txt"

python3 "$UDP" \
    --log "$UDP_LOG" \
    --bind 127.0.0.1 \
    --port "$PORT" \
    --owner-token "$TMP" \
    >"$UDP_CONSOLE" \
    2>&1 &

UDP_PID=$!

sleep 0.4

python3 - "$PORT" <<'PY'
import socket
import sys
import time

port = int(sys.argv[1])

payloads = [
    "PS2VNC_ID version=1 test=ISSUE7-SELFTEST "
    "digest=" + ("2" * 64),
    "PSTVNC_STAGE NET_READY",
    "PSTVNC_STAGE GS_READY",
    "PSTVNC_STAGE DESKTOP_READY",
]

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

for payload in payloads:
    sock.sendto(
        payload.encode("ascii"),
        ("127.0.0.1", port),
    )
    time.sleep(0.03)

sock.close()
PY

sleep 0.2

kill -TERM "$UDP_PID"
wait "$UDP_PID"
UDP_PID=''

grep -q 'ISSUE7_UDP_OBSERVER_STARTED=YES' \
    "$UDP_CONSOLE"

grep -q 'ISSUE7_UDP_OBSERVER_STOPPED=YES' \
    "$UDP_CONSOLE"

python3 - \
    "$RESULT" \
    "$UDP_LOG" <<'PY'
from importlib.util import (
    module_from_spec,
    spec_from_file_location,
)
import json
from pathlib import Path
import sys

result_path = Path(sys.argv[1])
udp_log = Path(sys.argv[2])

spec = spec_from_file_location(
    "issue7_result",
    result_path,
)

module = module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(module)

manifest = {
    "HARDWARE_TEST_ID": "ISSUE7-SELFTEST",
    "HARDWARE_ELF_IDENTITY_SHA256": "2" * 64,
}

payloads = [
    json.loads(line)["payload"]
    for line in udp_log.read_text(
        encoding="utf-8",
    ).splitlines()
    if line
]

passed, counts = module.evaluate_payloads(
    manifest,
    payloads,
)

assert passed
assert counts[
    "PS2VNC_ID version=1 test=ISSUE7-SELFTEST "
    + "digest="
    + ("2" * 64)
] == 1
assert counts["PSTVNC_STAGE NET_READY"] == 1
assert counts["PSTVNC_STAGE GS_READY"] == 1
assert counts["PSTVNC_STAGE DESKTOP_READY"] == 1
assert counts["PSTVNC_STAGE FATAL"] == 0

bad_order = payloads.copy()
bad_order[1], bad_order[2] = (
    bad_order[2],
    bad_order[1],
)

passed, _ = module.evaluate_payloads(
    manifest,
    bad_order,
)

assert not passed

fatal = payloads + ["PSTVNC_STAGE FATAL"]

passed, _ = module.evaluate_payloads(
    manifest,
    fatal,
)

assert not passed

duplicate = payloads + [payloads[0]]

passed, _ = module.evaluate_payloads(
    manifest,
    duplicate,
)

assert not passed

print("ISSUE7_UDP_RECEIVER_SELF_TEST=PASS")
print("ISSUE7_DIAGNOSTIC_ORDER_SELF_TEST=PASS")
print("ISSUE7_NEGATIVE_DIAGNOSTIC_TESTS=PASS")
PY

printf '%s\n' 'ISSUE7_APPARATUS_SELF_TEST=PASS'
printf '%s\n' 'NETWORK_CONTACT=LOOPBACK_ONLY'
printf '%s\n' 'FTP_USED=NO'
printf '%s\n' 'SUDO_USED=NO'
printf '%s\n' 'PS2_CONTACT=NO'
