#!/usr/bin/env bash
# File synopsis:
# Stop only observers demonstrably owned by one successor Issue #7 run.
#
# The resulting raw evidence is hashed before interpretation so later analysis
# cannot silently redefine the captured machine evidence.

set -euo pipefail

ROOT="$(
    cd "$(dirname "$0")/../.." &&
    pwd -P
)"

cd "$ROOT"

# shellcheck source=issue7-apparatus-common.sh
source scripts/testkit/issue7-apparatus-common.sh

[ "$#" -eq 1 ] ||
    issue7_die 'usage: issue7-stop-observers.sh RUN'

RUN="$(issue7_run_abs "$1")"
issue7_load_control "$RUN"

stop_udp()
{
    if ! issue7_pid_alive "$UDP_PID"; then
        return 0
    fi

    issue7_pid_owned "$UDP_PID" "$RUN" ||
        issue7_die \
            "refusing to stop unowned UDP observer pid=$UDP_PID"

    kill -TERM "$UDP_PID"

    for _attempt in $(seq 1 60); do
        issue7_pid_alive "$UDP_PID" || return 0
        sleep 0.05
    done

    issue7_pid_owned "$UDP_PID" "$RUN" ||
        issue7_die \
            'UDP observer ownership changed while stopping'

    kill -KILL "$UDP_PID"

    for _attempt in $(seq 1 40); do
        issue7_pid_alive "$UDP_PID" || return 0
        sleep 0.05
    done

    issue7_die 'UDP observer remained alive after SIGKILL'
}

stop_udp

if issue7_pcap_active "$PCAP_UNIT"; then
    issue7_pcap_owned "$PCAP_UNIT" "$PCAP_PATH" ||
        issue7_die \
            "refusing to stop unowned packet capture unit=$PCAP_UNIT"

    sudo -n systemctl stop "$PCAP_UNIT"
fi

issue7_pid_alive "$UDP_PID" &&
    issue7_die 'UDP observer still alive after stop'

issue7_pcap_active "$PCAP_UNIT" &&
    issue7_die 'packet capture still active after stop'

{
    printf 'STOP_RECORD_VERSION=1\n'
    printf 'TEST_ID=%q\n' "$TEST_ID"
    printf 'STOPPED_LOCAL=%q\n' "$(date --iso-8601=ns)"
} > "$RUN/STOPPED.env"

(
    cd "$RUN"

    find . \
        -type f \
        ! -name 'RAW-SHA256.txt' \
        ! -name 'ANALYSIS-RESULT.txt' \
        -print0 |
    sort -z |
    xargs -0 -r sha256sum
) > "$RUN/RAW-SHA256.txt"

(
    cd "$RUN"
    sha256sum -c RAW-SHA256.txt
)

if [ -f "$ISSUE7_ACTIVE_CONTROL" ]; then
    ACTIVE_RUN="$(
        bash -c '
            # shellcheck disable=SC1090
            source "$1"
            printf "%s\n" "${RUN:-}"
        ' _ "$ISSUE7_ACTIVE_CONTROL"
    )"

    if [ "$ACTIVE_RUN" = "$RUN" ]; then
        rm -f "$ISSUE7_ACTIVE_CONTROL"
    fi
fi

printf '%s\n' 'ISSUE7_OBSERVERS_STOPPED=PASS'
printf 'RUN=%s\n' "$RUN"
printf 'RAW_EVIDENCE_LEDGER=%s\n' "$RUN/RAW-SHA256.txt"
printf 'RAW_EVIDENCE_SEALED=YES\n'
printf 'NEXT_OPERATOR_ACTION=EVALUATE_RESULT\n'
