#!/usr/bin/env bash
# File synopsis:
# Move one live PS2 RFB session from the temporary Wayland/w0 proxy back to the
# canonical socket-activated Xtigervnc provider.
#
# This script removes the temporary socket Service= override, rearms the durable
# 192.168.50.1:5900 socket for Xtigervnc, and waits for the experimental ELF to
# reconnect with a new PS2 TCP source port. The w0 backend and 704x462 Wayland
# state deliberately remain prepared so another bidirectional switch is cheap.
#
# Plain-language context: README.md
# Symbol and variable lookup: SYMBOLS.md
set -euo pipefail

SOCKET='ps-to-vnc-rfb.socket'
XTIGER='ps-to-vnc-rfb-tigervnc.service'
W0_PROXY='ps-to-vnc-rfb-w0-proxy-test.service'
W0_BACKEND='ps-to-vnc-w0-backend-test.service'
DROPIN='/run/systemd/system/ps-to-vnc-rfb.socket.d/90-w0-proxy-test.conf'

fail() {
    printf 'FAIL=%s\n' "$1" >&2
    exit 1
}

front_connection() {
    sudo -n ss -H -ntp |
        grep -E '^ESTAB .*192\.168\.50\.1(%eth0)?:5900 +192\.168\.50\.2:' ||
        true
}

source_port() {
    sed -nE 's/.*192\.168\.50\.2:([0-9]+).*/\1/p' | head -1
}

printf '%s\n' '===== SWITCH RFB PROVIDER: WAYLAND/W0 -> XTIGERVNC ====='
[ "$(id -un)" = 'ps2' ] || fail 'RUN_AS_PS2_USER'
[ -e "$DROPIN" ] || fail 'WAYLAND_PROVIDER_DROPIN_MISSING'
[ "$(systemctl --user is-active "$W0_BACKEND" 2>/dev/null || true)" = 'active' ] || fail 'W0_BACKEND_INACTIVE'

TRIGGER="$(systemctl show "$SOCKET" -p Triggers --value)"
case " $TRIGGER " in
    *" $W0_PROXY "*) ;;
    *" $XTIGER "*) fail 'XTIGER_PROVIDER_ALREADY_SELECTED' ;;
    *) fail 'UNEXPECTED_SOCKET_TRIGGER' ;;
esac

FRONT_BEFORE="$(front_connection)"
[ -n "$FRONT_BEFORE" ] || fail 'NO_LIVE_PS2_RFB_SESSION'
SOURCE_BEFORE="$(printf '%s\n' "$FRONT_BEFORE" | source_port)"
[ -n "$SOURCE_BEFORE" ] || fail 'PS2_SOURCE_PORT_NOT_FOUND'
printf 'PS2_SOURCE_PORT_BEFORE=%s\n' "$SOURCE_BEFORE"

printf '%s\n' '===== RESTORE CANONICAL XTIGER SOCKET TARGET ====='
sudo -n rm "$DROPIN"
sudo -n systemctl daemon-reload

TRIGGER="$(systemctl show "$SOCKET" -p Triggers --value)"
case " $TRIGGER " in
    *" $XTIGER "*) ;;
    *) fail 'SOCKET_NOT_RETARGETED_TO_XTIGER' ;;
esac

sudo -n systemctl restart "$SOCKET"

FOUND='NO'
SOURCE_AFTER=''
XTIGER_PID=''
for I in $(seq 1 120); do
    FRONT_AFTER="$(front_connection)"
    SOURCE_AFTER="$(printf '%s\n' "$FRONT_AFTER" | source_port)"
    XT_STATE="$(systemctl is-active "$XTIGER" 2>/dev/null || true)"
    XTIGER_PID="$(systemctl show "$XTIGER" -p MainPID --value 2>/dev/null || true)"

    if [ -n "$SOURCE_AFTER" ] &&
       [ "$SOURCE_AFTER" != "$SOURCE_BEFORE" ] &&
       [ "$XT_STATE" = 'active' ] &&
       [ -n "$XTIGER_PID" ] &&
       [ "$XTIGER_PID" != '0' ]; then
        FOUND='YES'
        printf 'RECONNECT_DETECTED_AFTER_HALFSECONDS=%s\n' "$I"
        break
    fi
    sleep 0.5
done

[ "$FOUND" = 'YES' ] || fail 'XTIGER_RECONNECT_NOT_OBSERVED'
sudo -n systemctl stop "$W0_PROXY" 2>/dev/null || true
sudo -n systemctl reset-failed "$W0_PROXY" 2>/dev/null || true

printf 'PS2_SOURCE_PORT_AFTER=%s\n' "$SOURCE_AFTER"
printf 'XTIGER_PID=%s\n' "$XTIGER_PID"
printf 'SOCKET_TRIGGER=%s\n' "$(systemctl show "$SOCKET" -p Triggers --value)"
printf 'XTIGER_STATE=%s\n' "$(systemctl is-active "$XTIGER")"
printf 'W0_BACKEND_STATE=%s\n' "$(systemctl --user is-active "$W0_BACKEND")"
printf 'RFB_PROVIDER=XTIGERVNC\n'
printf 'WAYLAND_APPARATUS_RETAINED=YES\n'
printf 'RFB_SWITCH_TO_XTIGER=PASS\n'
printf 'ELF_RELAUNCH_REQUIRED=NO\n'
printf 'FTP_USED=NO\n'
