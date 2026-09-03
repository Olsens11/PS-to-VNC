#!/usr/bin/env bash
# File synopsis:
# Arm only the UDP and bounded PS2-facing packet observers required by Issue #7.
#
# Silent-stall recovery is deliberately absent: an unexplained DUT stall remains
# failed until the operator explicitly intervenes.

set -eEuo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd -P
)"

cd "$ROOT"

# shellcheck source=issue7-apparatus-common.sh
source scripts/testkit/issue7-apparatus-common.sh

[ "$#" -eq 1 ] ||
    issue7_die 'usage: issue7-arm-observers.sh DUT_MANIFEST'

MANIFEST="$(realpath -e "$1")"

issue7_validate_manifest_current "$MANIFEST"

for command in \
    python3 \
    ps \
    ss \
    sudo \
    systemctl
do
    command -v "$command" >/dev/null 2>&1 ||
        issue7_die "required command unavailable: $command"
done

for executable in \
    /usr/bin/tcpdump \
    /usr/bin/timeout
do
    [ -x "$executable" ] ||
        issue7_die "required executable unavailable: $executable"
done

mkdir -p "$ISSUE7_RUN_ROOT"

[ ! -e "$ISSUE7_ACTIVE_CONTROL" ] ||
    issue7_die \
        "active Issue #7 run already exists: $ISSUE7_ACTIVE_CONTROL"

if ss -H -lun |
   grep -Eq ':[5]999([[:space:]]|$)'
then
    issue7_die 'UDP port 5999 is already in use'
fi

STRAY_TCPDUMP="$(
    ps -eo pid=,args= |
        grep -E '[t]cpdump.*192\.168\.50\.2' ||
    true
)"

[ -z "$STRAY_TCPDUMP" ] || {
    printf '%s\n' "$STRAY_TCPDUMP" >&2
    issue7_die \
        'pre-existing PS2-facing tcpdump must be stopped before arming'
}

sudo -n true

STAMP="$(date '+%Y%m%d-%H%M%S')"
RUN="$ISSUE7_RUN_ROOT/$ISSUE7_TEST_ID-$STAMP"
OBS="$RUN/observers"

[ ! -e "$RUN" ] ||
    issue7_die "run already exists: $RUN"

mkdir -p "$OBS"

RUN="$(realpath -e "$RUN")"
OBS="$RUN/observers"

cp -p "$MANIFEST" "$RUN/manifest.env"

MANIFEST_SHA256="$(issue7_sha256 "$RUN/manifest.env")"

UDP_LOG="$OBS/udp5999.jsonl"
UDP_CONSOLE="$OBS/udp5999-console.log"
PCAP_PATH="$OBS/ps2-facing.pcap"

UNIT_TEST_ID="$(
    printf '%s' "$ISSUE7_TEST_ID" |
        tr '[:upper:]' '[:lower:]' |
        sed 's/[^a-z0-9_.-]/-/g'
)"

PCAP_UNIT="ps2vnc-i7-$UNIT_TEST_ID-$STAMP.service"

UDP_PID=''
PCAP_STARTED=0

cleanup_error()
{
    rc=$?
    trap - ERR
    set +e

    if [ -n "$UDP_PID" ] &&
       issue7_pid_owned "$UDP_PID" "$RUN"
    then
        kill -TERM "$UDP_PID" 2>/dev/null
    fi

    if [ "$PCAP_STARTED" -eq 1 ] &&
       issue7_pcap_active "$PCAP_UNIT" &&
       issue7_pcap_owned "$PCAP_UNIT" "$PCAP_PATH"
    then
        sudo -n systemctl stop "$PCAP_UNIT" >/dev/null 2>&1
    fi

    printf 'ARM_FAILURE_EXIT=%s\n' "$rc" \
        > "$RUN/ARM-FAILED.txt"

    exit "$rc"
}

trap cleanup_error ERR

: > "$UDP_CONSOLE"

nohup python3 \
    "$ROOT/scripts/testkit/issue7-udp-observer.py" \
    --log "$UDP_LOG" \
    --bind "$ISSUE7_PI_PRIVATE_IP" \
    --port "$ISSUE7_UDP_PORT" \
    --owner-token "$RUN" \
    >"$UDP_CONSOLE" \
    2>&1 \
    </dev/null &

UDP_PID=$!

sleep 0.5

issue7_pid_owned "$UDP_PID" "$RUN" ||
    issue7_die 'UDP observer failed startup ownership check'

sudo -n systemd-run \
    --unit="$PCAP_UNIT" \
    --collect \
    --property=Restart=no \
    /usr/bin/timeout \
    --signal=TERM \
    "$ISSUE7_CAPTURE_MAX_SECONDS" \
    /usr/bin/tcpdump \
    -ni "$ISSUE7_PCAP_INTERFACE" \
    -s 0 \
    -U \
    -w "$PCAP_PATH" \
    host "$ISSUE7_PS2_HOST"

PCAP_STARTED=1

sleep 0.5

issue7_pcap_active "$PCAP_UNIT" ||
    issue7_die 'PS2-facing packet capture failed startup'

issue7_pcap_owned "$PCAP_UNIT" "$PCAP_PATH" ||
    issue7_die 'packet capture ownership verification failed'

CONTROL="$RUN/control.env"

{
    printf 'CONTROL_VERSION=1\n'
    printf 'TEST_ID=%q\n' "$ISSUE7_TEST_ID"
    printf 'RUN=%q\n' "$RUN"
    printf 'ARMED_LOCAL=%q\n' "$(date --iso-8601=ns)"
    printf 'SOURCE_COMMIT=%q\n' "$ISSUE7_SOURCE_COMMIT"
    printf 'MANIFEST_SHA256=%q\n' "$MANIFEST_SHA256"
    printf 'UDP_PID=%q\n' "$UDP_PID"
    printf 'UDP_LOG=%q\n' "$UDP_LOG"
    printf 'PCAP_UNIT=%q\n' "$PCAP_UNIT"
    printf 'PCAP_PATH=%q\n' "$PCAP_PATH"
} > "$CONTROL"

mkdir -p "$ISSUE7_STATE_ROOT"
cp -p "$CONTROL" "$ISSUE7_ACTIVE_CONTROL"

trap - ERR

printf '%s\n' 'ISSUE7_OBSERVERS_ARMED=PASS'
printf 'TEST_ID=%s\n' "$ISSUE7_TEST_ID"
printf 'RUN=%s\n' "$RUN"
printf 'UDP_PID=%s\n' "$UDP_PID"
printf 'UDP_LOG=%s\n' "$UDP_LOG"
printf 'PCAP_UNIT=%s\n' "$PCAP_UNIT"
printf 'PCAP_PATH=%s\n' "$PCAP_PATH"
printf 'PCAP_MAX_SECONDS=%s\n' "$ISSUE7_CAPTURE_MAX_SECONDS"
printf 'NEXT_OPERATOR_ACTION=DEPLOY_EXACT_STAMPED_DUT\n'
