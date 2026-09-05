#!/usr/bin/env bash

set -euo pipefail

PROFILE='ps2-link'
DEVICE='eth0'
ADDRESS='192.168.50.1/24'
BASELINE_PROFILE='netplan-eth0'

if [ "${EUID}" -ne 0 ]; then
    echo 'ERROR: run this script through sudo/root.' >&2
    exit 2
fi

if ! command -v nmcli >/dev/null 2>&1; then
    echo 'ERROR: NetworkManager/nmcli is required.' >&2
    exit 10
fi

baseline_present=NO
if nmcli -t -f NAME connection show | grep -Fxq "$BASELINE_PROFILE"; then
    baseline_present=YES
fi

profile_value() {
    nmcli -g "$1" connection show "$PROFILE" 2>/dev/null | tr -d '\r'
}

require_value() {
    field="$1"
    expected="$2"
    actual="$(profile_value "$field")"

    if [ "$actual" != "$expected" ]; then
        echo "ERROR: $PROFILE field mismatch: $field" >&2
        echo "EXPECTED=$expected" >&2
        echo "ACTUAL=${actual:-EMPTY}" >&2
        exit 20
    fi
}

if nmcli -t -f NAME connection show | grep -Fxq "$PROFILE"; then
    echo 'PS2_LINK_PROFILE_ALREADY_EXISTS=YES'
else
    nmcli connection add \
        type ethernet \
        ifname "$DEVICE" \
        con-name "$PROFILE" \
        connection.autoconnect yes \
        connection.autoconnect-priority 100 \
        ipv4.method manual \
        ipv4.addresses "$ADDRESS" \
        ipv4.never-default yes \
        ipv6.method disabled
fi

require_value connection.id "$PROFILE"
require_value connection.interface-name "$DEVICE"
require_value connection.autoconnect yes
require_value connection.autoconnect-priority 100
require_value ipv4.method manual
require_value ipv4.addresses "$ADDRESS"
require_value ipv4.never-default yes
require_value ipv6.method disabled

if [ "$baseline_present" = YES ]; then
    if ! nmcli -t -f NAME connection show | grep -Fxq "$BASELINE_PROFILE"; then
        echo 'ERROR: pre-existing netplan-eth0 profile disappeared.' >&2
        exit 21
    fi
fi

if ip route show default | grep -q ' dev eth0 '; then
    echo 'ERROR: eth0 unexpectedly owns a default route.' >&2
    exit 22
fi

echo "PS2_LINK_PROFILE=$PROFILE"
echo "PS2_LINK_DEVICE=$DEVICE"
echo "PS2_LINK_ADDRESS=$ADDRESS"
echo 'PS2_LINK_NEVER_DEFAULT=YES'
echo "BASELINE_NETPLAN_ETH0_PRESERVED=$baseline_present"
echo 'PS2_LINK_ACTIVATED_BY_SCRIPT=NO'
echo 'PS2_LINK_PROFILE_CONFIGURE=PASS'
