#!/usr/bin/env bash

# Read-only timing apparatus for the Issue #5 cold-boot / first-connect race.
# It records live state and packet timing but does not alter NetworkManager,
# systemd units, addresses, routes, processes, or listeners.
#
# Apparatus rule: prefer event-driven observers for the events that matter.
# The bounded sampler intentionally checks only cheap kernel-visible state so
# the observer itself does not materially change startup timing on the Pi.

set -euo pipefail

DURATION="${DURATION:-60}"
SAMPLE_INTERVAL="${SAMPLE_INTERVAL:-0.20}"
DEVICE="${DEVICE:-eth0}"
ADDRESS="${ADDRESS:-192.168.50.1/24}"
PORT="${PORT:-5900}"
RUN_ROOT="${RUN_ROOT:-/tmp}"
SOCKET_UNIT='ps-to-vnc-rfb.socket'
ACTIVATED_PROVIDER_UNIT='ps-to-vnc-rfb-tigervnc.service'
PERSISTENT_PROVIDER_UNIT='ps-to-vnc-rfb-tigervnc-persistent.service'
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
RUN_DIR="$RUN_ROOT/ps-to-vnc-rfb-activation-$STAMP"

if [ "${EUID}" -ne 0 ]; then
    echo 'ERROR: run through sudo/root so tcpdump and complete journal data are available.' >&2
    exit 2
fi

for tool in ip ss journalctl tcpdump timeout date nmcli systemctl sha256sum find sort xargs; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "ERROR: required tool missing: $tool" >&2
        exit 3
    fi
done

mkdir -m 0755 "$RUN_DIR"

cleanup() {
    local pid
    for pid in ${WATCH_PIDS:-}; do
        kill "$pid" 2>/dev/null || true
    done
    wait 2>/dev/null || true
}
trap cleanup EXIT INT TERM

{
    echo "UTC_START=$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)"
    echo "DURATION_SECONDS=$DURATION"
    echo "SAMPLE_INTERVAL_SECONDS=$SAMPLE_INTERVAL"
    echo "DEVICE=$DEVICE"
    echo "ADDRESS=$ADDRESS"
    echo "PORT=$PORT"
    echo "RFB_BOUNDARY_UNIT=$SOCKET_UNIT"
    echo "SOCKET_ACTIVATED_PROVIDER=$ACTIVATED_PROVIDER_UNIT"
    echo "PERSISTENT_CONTROL_PROVIDER=$PERSISTENT_PROVIDER_UNIT"
    echo "RUN_DIR=$RUN_DIR"
    echo "NETWORKMANAGER_VERSION=$(NetworkManager --version 2>/dev/null || true)"
    echo "SYSTEMD_VERSION=$(systemctl --version 2>/dev/null | head -1 || true)"
    echo "KERNEL=$(uname -r)"
} >"$RUN_DIR/metadata.env"

# Initial read-only snapshots.
ip -details link show dev "$DEVICE" >"$RUN_DIR/link-before.txt" 2>&1 || true
ip -4 addr show dev "$DEVICE" >"$RUN_DIR/address-before.txt" 2>&1 || true
ip -4 route show >"$RUN_DIR/routes-before.txt" 2>&1 || true
nmcli -f GENERAL.STATE,GENERAL.CONNECTION,GENERAL.DEVICE,WIRED-PROPERTIES.CARRIER device show "$DEVICE" \
    >"$RUN_DIR/nm-device-before.txt" 2>&1 || true
nmcli connection show ps2-link >"$RUN_DIR/nm-ps2-link-before.txt" 2>&1 || true
ss -ltnp "sport = :$PORT" >"$RUN_DIR/listener-before.txt" 2>&1 || true
systemctl status --no-pager \
    "$SOCKET_UNIT" "$ACTIVATED_PROVIDER_UNIT" "$PERSISTENT_PROVIDER_UNIT" \
    >"$RUN_DIR/systemd-before.txt" 2>&1 || true

WATCH_PIDS=''

# Kernel link/address/route changes are the authoritative low-level transition
# record. Timestamp each event as it is consumed.
(
    timeout "$DURATION" ip monitor link address route 2>&1 |
    while IFS= read -r line; do
        printf '%s %s\n' "$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)" "$line"
    done
) >"$RUN_DIR/ip-monitor.log" &
WATCH_PIDS+=" $!"

# NetworkManager and systemd provide their own event records. -n 0 prevents
# pre-capture journal history from being mixed into this bounded observation.
(timeout "$DURATION" journalctl -n 0 -f -o short-monotonic \
    -u NetworkManager \
    -u "$SOCKET_UNIT" \
    -u "$ACTIVATED_PROVIDER_UNIT" \
    -u "$PERSISTENT_PROVIDER_UNIT" \
    --no-pager 2>&1 || true) >"$RUN_DIR/journal.log" &
WATCH_PIDS+=" $!"

# Wire evidence decides the ordering of ARP and TCP establishment. Store raw
# packets rather than doing protocol work in the observer process.
(timeout "$DURATION" tcpdump -i "$DEVICE" -nn -U -s 0 \
    -w "$RUN_DIR/ps2-link.pcap" "arp or tcp port $PORT" \
    2>"$RUN_DIR/tcpdump.stderr" || true) &
WATCH_PIDS+=" $!"

# Low-overhead periodic witness. NetworkManager and service transitions are
# already event-driven above, so do not spawn nmcli/systemctl at high frequency.
# This sampler only provides convenient correlation points for three cheap,
# product-significant states.
(
    end=$((SECONDS + DURATION))
    printf 'utc\tcarrier\taddress_present\tlisten5900\n'
    while [ "$SECONDS" -lt "$end" ]; do
        carrier='?'
        [ -r "/sys/class/net/$DEVICE/carrier" ] && \
            carrier="$(cat "/sys/class/net/$DEVICE/carrier" 2>/dev/null || printf '?')"

        if ip -4 addr show dev "$DEVICE" 2>/dev/null | grep -Fq "inet $ADDRESS"; then
            address_present=1
        else
            address_present=0
        fi

        if ss -H -ltn "sport = :$PORT" 2>/dev/null | grep -q .; then
            listen5900=1
        else
            listen5900=0
        fi

        printf '%s\t%s\t%s\t%s\n' \
            "$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)" \
            "$carrier" "$address_present" "$listen5900"

        sleep "$SAMPLE_INTERVAL"
    done
) >"$RUN_DIR/state-sampler.tsv" &
WATCH_PIDS+=" $!"

cat <<EOF
RFB_ACTIVATION_TIMELINE_CAPTURE=RUNNING
RUN_DIR=$RUN_DIR
DURATION_SECONDS=$DURATION
SAMPLE_INTERVAL_SECONDS=$SAMPLE_INTERVAL

During the next $DURATION seconds, perform only the physical PS2 launch required
by the current test step. This apparatus does not change Pi configuration.
EOF

wait
WATCH_PIDS=''
trap - EXIT INT TERM

# Final snapshots.
ip -details link show dev "$DEVICE" >"$RUN_DIR/link-after.txt" 2>&1 || true
ip -4 addr show dev "$DEVICE" >"$RUN_DIR/address-after.txt" 2>&1 || true
ip -4 route show >"$RUN_DIR/routes-after.txt" 2>&1 || true
nmcli -f GENERAL.STATE,GENERAL.CONNECTION,GENERAL.DEVICE,WIRED-PROPERTIES.CARRIER device show "$DEVICE" \
    >"$RUN_DIR/nm-device-after.txt" 2>&1 || true
ss -ltnp "sport = :$PORT" >"$RUN_DIR/listener-after.txt" 2>&1 || true
systemctl status --no-pager \
    "$SOCKET_UNIT" "$ACTIVATED_PROVIDER_UNIT" "$PERSISTENT_PROVIDER_UNIT" \
    >"$RUN_DIR/systemd-after.txt" 2>&1 || true
printf 'UTC_END=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)" >>"$RUN_DIR/metadata.env"

# Hash the finalized evidence set. The manifest intentionally excludes itself.
find "$RUN_DIR" -maxdepth 1 -type f ! -name SHA256SUMS.txt -print0 \
    | sort -z \
    | xargs -0 sha256sum >"$RUN_DIR/SHA256SUMS.txt"

echo "RFB_ACTIVATION_TIMELINE_CAPTURE=COMPLETE"
echo "RUN_DIR=$RUN_DIR"
echo 'MUTATION_PERFORMED=NO'
