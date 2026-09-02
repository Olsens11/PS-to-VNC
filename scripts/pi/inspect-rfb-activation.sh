#!/usr/bin/env bash

# Read-only preflight for the Issue #5 RFB endpoint-lifecycle experiment.
# This script must not change NetworkManager, systemd, packages, files, routes,
# processes, or listeners.

set -u

PROFILE='ps2-link'
DEVICE='eth0'
ADDRESS='192.168.50.1/24'
PORT='5900'
SOCKET_UNIT='ps-to-vnc-rfb.socket'
ACTIVATED_PROVIDER_UNIT='ps-to-vnc-rfb-tigervnc.service'
PERSISTENT_PROVIDER_UNIT='ps-to-vnc-rfb-tigervnc-persistent.service'
LEGACY_GENERIC_UNIT='ps-to-vnc-rfb.service'

section() {
    printf '\n===== %s =====\n' "$1"
}

run() {
    printf '+ '
    printf '%q ' "$@"
    printf '\n'
    "$@" 2>&1 || printf 'COMMAND_EXIT=%d\n' "$?"
}

section 'IDENTITY'
date -u '+UTC=%Y-%m-%dT%H:%M:%SZ'
printf 'HOSTNAME=%s\n' "$(hostname 2>/dev/null || printf UNKNOWN)"
run uname -a
if [ -r /etc/os-release ]; then
    run cat /etc/os-release
fi

section 'PACKAGE / TOOL VERSIONS'
run NetworkManager --version
run systemctl --version
run dpkg-query -W -f='${Package}=${Version}\n' network-manager systemd tigervnc-standalone-server
run dpkg-query -S /usr/bin/Xtigervnc
if [ -x /usr/bin/Xtigervnc ]; then
    run /usr/bin/Xtigervnc -version
else
    echo 'XTIGERVNC_EXECUTABLE=ABSENT'
fi

section 'NETWORKMANAGER EFFECTIVE CONFIG'
run NetworkManager --print-config

section 'PS2-LINK PROFILE'
if nmcli -t -f NAME connection show 2>/dev/null | grep -Fxq "$PROFILE"; then
    echo 'PS2_LINK_PROFILE_PRESENT=YES'
    run nmcli -f \
connection.id,connection.interface-name,connection.autoconnect,connection.autoconnect-priority,\
ipv4.method,ipv4.addresses,ipv4.gateway,ipv4.never-default,ipv6.method \
connection show "$PROFILE"
else
    echo 'PS2_LINK_PROFILE_PRESENT=NO'
fi

section 'ETH0 LIVE STATE'
run nmcli -f GENERAL.STATE,GENERAL.CONNECTION,GENERAL.DEVICE,WIRED-PROPERTIES.CARRIER device show "$DEVICE"
if [ -r "/sys/class/net/$DEVICE/carrier" ]; then
    printf 'SYSFS_CARRIER='
    cat "/sys/class/net/$DEVICE/carrier" 2>/dev/null || true
fi
run ip -details link show dev "$DEVICE"
run ip -4 addr show dev "$DEVICE"
run ip -4 route show

if ip -4 addr show dev "$DEVICE" 2>/dev/null | grep -Fq "inet $ADDRESS"; then
    echo 'PS2_PRIVATE_ADDRESS_PRESENT=YES'
else
    echo 'PS2_PRIVATE_ADDRESS_PRESENT=NO'
fi

section 'RFB LISTENER / PROCESS STATE'
run ss -ltnp "sport = :$PORT"
run pgrep -a Xtigervnc
run pgrep -a w0vncserver
run pgrep -af 'ps-to-vnc-vnc-session|wayvnc|Xtigervnc|w0vncserver'

section 'SYSTEMD RFB BOUNDARY / PROVIDER STATE'
for unit in \
    "$SOCKET_UNIT" \
    "$ACTIVATED_PROVIDER_UNIT" \
    "$PERSISTENT_PROVIDER_UNIT" \
    "$LEGACY_GENERIC_UNIT"; do
    run systemctl status --no-pager "$unit"
    run systemctl is-enabled "$unit"
    run systemctl is-active "$unit"
done

section 'DISPLAY :1 COLLISION CHECK'
for path in /tmp/.X1-lock /tmp/.X11-unix/X1; do
    if [ -e "$path" ]; then
        printf 'PRESENT=%s\n' "$path"
        run ls -ld "$path"
    else
        printf 'ABSENT=%s\n' "$path"
    fi
done

section 'EXPLORATORY WAYLAND ARTIFACT CHECK'
for path in \
    "$HOME/.local/lib/ps-to-vnc/w0vncserver" \
    "$HOME/.local/bin/ps-to-vnc-vnc-session" \
    "$HOME/.config/kanshi/config"; do
    if [ -e "$path" ]; then
        printf 'PRESENT=%s\n' "$path"
        run ls -l "$path"
        run sha256sum "$path"
    else
        printf 'ABSENT=%s\n' "$path"
    fi
done

section 'READ-ONLY PRECHECK SUMMARY'
echo "EXPECTED_DEVICE=$DEVICE"
echo "EXPECTED_ADDRESS=$ADDRESS"
echo "EXPECTED_RFB_PORT=$PORT"
echo "RFB_BOUNDARY_UNIT=$SOCKET_UNIT"
echo "CURRENT_SOCKET_ACTIVATED_PROVIDER=$ACTIVATED_PROVIDER_UNIT"
echo "PERSISTENT_CONTROL_PROVIDER=$PERSISTENT_PROVIDER_UNIT"
echo 'MUTATION_PERFORMED=NO'
echo 'RFB_ACTIVATION_PREFLIGHT=COMPLETE'
