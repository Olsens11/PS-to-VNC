#!/usr/bin/env bash

# Read-only timing apparatus for the Issue #5 cold-boot / first-connect race.
# It records live state and packet timing but does not alter NetworkManager,
# systemd units, addresses, routes, processes, or listeners.

set -euo pipefail

DURATION="${DURATION:-60}"
DEVICE="${DEVICE:-eth0}"
PORT="${PORT:-5900}"
RUN_ROOT="${RUN_ROOT:-/tmp}"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
RUN_DIR="$RUN_ROOT/ps-to-vnc-rfb-activation-$STAMP"

if [ "${EUID}" -ne 0 ]; then
    echo 'ERROR: run through sudo/root so tcpdump and complete journal data are available.' >&2
    exit 2
fi

for tool in ip ss journalctl tcpdump timeout date awk sed; do
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
    echo "DEVICE=$DEVICE"
    echo "PORT=$PORT"
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
systemctl status --no-pager ps-to-vnc-rfb.socket ps-to-vnc-rfb.service \
    >"$RUN_DIR/systemd-before.txt" 2>&1 || true

WATCH_PIDS=''

# Kernel/network state transitions with monotonic-ish timestamps supplied by
# userspace date at each line. stdbuf is used when available to avoid buffering.
(
    timeout "$DURATION" ip monitor link address route 2>&1 |
    while IFS= read -r line; do
        printf '%s %s\n' "$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)" "$line"
    done
) >"$RUN_DIR/ip-monitor.log" &
WATCH_PIDS+=" $!"

# NetworkManager and candidate systemd unit events. short-monotonic preserves
# ordering against boot-relative timestamps; precise wall time is also present
# in the packet capture and sampler.
(timeout "$DURATION" journalctl -f -o short-monotonic -u NetworkManager \
    -u ps-to-vnc-rfb.socket -u ps-to-vnc-rfb.service --no-pager 2>&1 || true) \
    >"$RUN_DIR/journal.log" &
WATCH_PIDS+=" $!"

# Capture ARP and RFB TCP establishment/data on the dedicated private link.
(timeout "$DURATION" tcpdump -i "$DEVICE" -nn -tttt -U -s 0 \
    -w "$RUN_DIR/ps2-link.pcap" "arp or tcp port $PORT" 2>"$RUN_DIR/tcpdump.stderr" || true) &
WATCH_PIDS+=" $!"

# High-frequency state sampler. 50 ms is intentionally short for this narrow
# startup experiment and runs only for the bounded capture window.
(
    end=$((SECONDS + DURATION))
    printf 'utc\tcarrier\taddress_present\tnm_state\tnm_connection\tlisten5900\tsocket_state\tservice_state\n'
    while [ "$SECONDS" -lt "$end" ]; do
        carrier='?'
        [ -r "/sys/class/net/$DEVICE/carrier" ] && carrier="$(cat "/sys/class/net/$DEVICE/carrier" 2>/dev/null || printf '?')"

        if ip -4 addr show dev "$DEVICE" 2>/dev/null | grep -Fq 'inet 192.168.50.1/24'; then
            address_present=1
        else
            address_present=0
        fi

        nm_state="$(nmcli -g GENERAL.STATE device show "$DEVICE" 2>/dev/null | tr '\t ' '__' || true)"
        nm_connection="$(nmcli -g GENERAL.CONNECTION device show "$DEVICE" 2>/dev/null | tr '\t ' '__' || true)"

        if ss -H -ltn "sport = :$PORT" 2>/dev/null | grep -q .; then
            listen5900=1
        else
            listen5900=0
        fi

        socket_state="$(systemctl is-active ps-to-vnc-rfb.socket 2>/dev/null || true)"
        service_state="$(systemctl is-active ps-to-vnc-rfb.service 2>/dev/null || true)"

        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)" \
            "$carrier" "$address_present" "$nm_state" "$nm_connection" \
            "$listen5900" "$socket_state" "$service_state"

        sleep 0.05
    done
) >"$RUN_DIR/state-sampler.tsv" &
WATCH_PIDS+=" $!"

cat <<EOF
RFB_ACTIVATION_TIMELINE_CAPTURE=RUNNING
RUN_DIR=$RUN_DIR
DURATION_SECONDS=$DURATION

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
ss -ltnp "sport = :$PORT" >"$RUN_DIR/listener-after.txt" 2>&1 || true
systemctl status --no-pager ps-to-vnc-rfb.socket ps-to-vnc-rfb.service \
    >"$RUN_DIR/systemd-after.txt" 2>&1 || true

sha256sum "$RUN_DIR"/* >"$RUN_DIR/SHA256SUMS.txt" 2>/dev/null || true

printf 'UTC_END=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)" >>"$RUN_DIR/metadata.env"

echo "RFB_ACTIVATION_TIMELINE_CAPTURE=COMPLETE"
echo "RUN_DIR=$RUN_DIR"
echo 'MUTATION_PERFORMED=NO'
