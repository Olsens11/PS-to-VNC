#!/usr/bin/env bash

set -euo pipefail

EXPECTED_TIGERVNC='1.15.0+dfsg-2.1~deb13u1'
PROFILE='ps2-link'
DEVICE='eth0'
ADDRESS='192.168.50.1/24'

fail() {
    echo "FOUNDATION_VERIFY=FAIL reason=$1" >&2
    exit 1
}

installed="$(dpkg-query -W -f='${Version}' tigervnc-standalone-server 2>/dev/null || true)"
[ "$installed" = "$EXPECTED_TIGERVNC" ] || fail "tigervnc-version:${installed:-NONE}"
command -v Xtigervnc >/dev/null 2>&1 || fail 'Xtigervnc-missing'

nmcli -t -f NAME connection show | grep -Fxq "$PROFILE" || fail 'ps2-link-missing'

profile_value() {
    nmcli -g "$1" connection show "$PROFILE" 2>/dev/null | tr -d '\r'
}

[ "$(profile_value connection.interface-name)" = "$DEVICE" ] || fail 'ps2-link-device'
[ "$(profile_value connection.autoconnect)" = yes ] || fail 'ps2-link-autoconnect'
[ "$(profile_value connection.autoconnect-priority)" = 100 ] || fail 'ps2-link-priority'
[ "$(profile_value ipv4.method)" = manual ] || fail 'ps2-link-ipv4-method'
[ "$(profile_value ipv4.addresses)" = "$ADDRESS" ] || fail 'ps2-link-address'
[ "$(profile_value ipv4.never-default)" = yes ] || fail 'ps2-link-never-default'
[ "$(profile_value ipv6.method)" = disabled ] || fail 'ps2-link-ipv6'

if ip route show default | grep -q ' dev eth0 '; then
    fail 'eth0-default-route'
fi

state="$(nmcli -g GENERAL.STATE device show "$DEVICE" 2>/dev/null || true)"
connection="$(nmcli -g GENERAL.CONNECTION device show "$DEVICE" 2>/dev/null || true)"
active_address="$(nmcli -g IP4.ADDRESS device show "$DEVICE" 2>/dev/null || true)"

if [ "$connection" = "$PROFILE" ] && [ "$active_address" != "$ADDRESS" ]; then
    fail "active-address:${active_address:-NONE}"
fi

echo "TIGERVNC_VERSION=$installed"
echo "PS2_LINK_PROFILE=$PROFILE"
echo "PS2_LINK_EXPECTED_ADDRESS=$ADDRESS"
echo "ETH0_STATE=${state:-UNKNOWN}"
echo "ETH0_CONNECTION=${connection:---}"
echo "ETH0_ACTIVE_ADDRESS=${active_address:-NONE}"
echo 'FOUNDATION_VERIFY=PASS'
