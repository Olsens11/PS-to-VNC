#!/usr/bin/env bash
# File synopsis:
# Move one live PS2 RFB session from canonical Xtigervnc to the already-prepared
# 704x462 Wayland/w0 backend while keeping 192.168.50.1:5900 as the PS2 endpoint.
#
# This script installs the temporary socket Service= override, restarts the
# durable socket to deliberately break the old RFB connection, and then waits
# for the reconnect-capable experimental ELF to create a new source-port
# connection through systemd-socket-proxyd to w0vncserver on 127.0.0.1:5901.
# It does not start or configure the Wayland/w0 backend; setup owns that work.
#
# Plain-language context: README.md
# Symbol and variable lookup: SYMBOLS.md
set -euo pipefail

SOCKET='ps-to-vnc-rfb.socket'
XTIGER='ps-to-vnc-rfb-tigervnc.service'
W0_PROXY='ps-to-vnc-rfb-w0-proxy-test.service'
W0_BACKEND='ps-to-vnc-w0-backend-test.service'
PROXY_UNIT='/run/systemd/system/ps-to-vnc-rfb-w0-proxy-test.service'
DROPIN_DIR='/run/systemd/system/ps-to-vnc-rfb.socket.d'
DROPIN="$DROPIN_DIR/90-w0-proxy-test.conf"
RUNTIME_DIR="/run/user/$(id -u)"
WAYLAND_DISPLAY_NAME='wayland-0'

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

wayland_is_480p() {
    env \
        XDG_RUNTIME_DIR="$RUNTIME_DIR" \
        WAYLAND_DISPLAY="$WAYLAND_DISPLAY_NAME" \
        wlr-randr | awk '
            /^NOOP-1 / { inside=1; next }
            /^[^[:space:]]/ { if (inside) exit }
            inside && /704x462 px \(current\)/ { found=1 }
            END { exit found ? 0 : 1 }
        '
}

printf '%s\n' '===== SWITCH RFB PROVIDER: XTIGERVNC -> WAYLAND/W0 ====='
[ "$(id -un)" = 'ps2' ] || fail 'RUN_AS_PS2_USER'
[ -f "$PROXY_UNIT" ] || fail 'RUN_SETUP_FIRST_PROXY_UNIT_MISSING'
[ "$(systemctl --user is-active "$W0_BACKEND" 2>/dev/null || true)" = 'active' ] || fail 'RUN_SETUP_FIRST_W0_BACKEND_INACTIVE'
ss -H -lntp | grep -E '127\.0\.0\.1:5901 .*w0vncserver' >/dev/null || fail 'W0_BACKEND_NOT_LISTENING_5901'
wayland_is_480p || fail 'WAYLAND_NOOP1_NOT_704X462'

TRIGGER="$(systemctl show "$SOCKET" -p Triggers --value)"
case " $TRIGGER " in
    *" $XTIGER "*) ;;
    *" $W0_PROXY "*) fail 'WAYLAND_PROVIDER_ALREADY_SELECTED' ;;
    *) fail 'UNEXPECTED_SOCKET_TRIGGER' ;;
esac

FRONT_BEFORE="$(front_connection)"
[ -n "$FRONT_BEFORE" ] || fail 'NO_LIVE_PS2_RFB_SESSION'
SOURCE_BEFORE="$(printf '%s\n' "$FRONT_BEFORE" | source_port)"
[ -n "$SOURCE_BEFORE" ] || fail 'PS2_SOURCE_PORT_NOT_FOUND'
printf 'PS2_SOURCE_PORT_BEFORE=%s\n' "$SOURCE_BEFORE"

printf '%s\n' '===== RETARGET DURABLE SOCKET TO W0 PROXY ====='
sudo -n mkdir -p "$DROPIN_DIR"
sudo -n tee "$DROPIN" >/dev/null <<'DROPIN'
[Socket]
Service=ps-to-vnc-rfb-w0-proxy-test.service
DROPIN
sudo -n systemctl reset-failed "$W0_PROXY" 2>/dev/null || true
sudo -n systemctl daemon-reload

TRIGGER="$(systemctl show "$SOCKET" -p Triggers --value)"
case " $TRIGGER " in
    *" $W0_PROXY "*) ;;
    *) fail 'SOCKET_NOT_RETARGETED_TO_W0_PROXY' ;;
esac

sudo -n systemctl restart "$SOCKET"

FOUND='NO'
SOURCE_AFTER=''
for I in $(seq 1 120); do
    FRONT_AFTER="$(front_connection)"
    SOURCE_AFTER="$(printf '%s\n' "$FRONT_AFTER" | source_port)"
    BACK_AFTER="$(
        sudo -n ss -H -ntp |
            grep -E '^ESTAB .*127\.0\.0\.1:[0-9]+ +127\.0\.0\.1:5901' ||
            true
    )"
    PROXY_STATE="$(systemctl is-active "$W0_PROXY" 2>/dev/null || true)"

    if [ -n "$SOURCE_AFTER" ] &&
       [ "$SOURCE_AFTER" != "$SOURCE_BEFORE" ] &&
       [ -n "$BACK_AFTER" ] &&
       [ "$PROXY_STATE" = 'active' ]; then
        FOUND='YES'
        printf 'RECONNECT_DETECTED_AFTER_HALFSECONDS=%s\n' "$I"
        break
    fi
    sleep 0.5
done

[ "$FOUND" = 'YES' ] || fail 'WAYLAND_RECONNECT_NOT_OBSERVED'
sudo -n systemctl reset-failed "$XTIGER" 2>/dev/null || true

printf 'PS2_SOURCE_PORT_AFTER=%s\n' "$SOURCE_AFTER"
printf 'SOCKET_TRIGGER=%s\n' "$(systemctl show "$SOCKET" -p Triggers --value)"
printf 'W0_PROXY_STATE=%s\n' "$(systemctl is-active "$W0_PROXY")"
printf 'W0_BACKEND_STATE=%s\n' "$(systemctl --user is-active "$W0_BACKEND")"
printf 'RFB_PROVIDER=WAYLAND_W0\n'
printf 'RFB_SWITCH_TO_WAYLAND=PASS\n'
printf 'ELF_RELAUNCH_REQUIRED=NO\n'
printf 'FTP_USED=NO\n'
