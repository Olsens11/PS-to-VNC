#!/usr/bin/env bash
# File synopsis:
# Remove the temporary Issue #7 Wayland/w0 provider-handoff apparatus and return
# the Pi to its normal Xtigervnc plus stock Raspberry Pi OS Wayland state.
#
# If Wayland/w0 is currently selected, this script first restores the canonical
# Xtigervnc socket target and waits for the experimental ELF to reconnect. It
# then removes the temporary proxy unit, stops and collects the transient w0
# backend, and restores NOOP-1 to the observed stock 1920x1080 geometry. If
# Xtigervnc is already selected, the durable socket is deliberately left alone.
# The hash-verified w0vncserver executable is intentionally retained on disk.
#
# Plain-language context: README.md
# Symbol and variable lookup: SYMBOLS.md
set -euo pipefail

W0="$HOME/.local/lib/ps-to-vnc/w0vncserver"
W0_SHA256='76dd46e40a3b1d82da8a86b3bff622b2f6c9c703c2d1ed38dee81c660d0ef804'
SOCKET='ps-to-vnc-rfb.socket'
XTIGER='ps-to-vnc-rfb-tigervnc.service'
W0_PROXY='ps-to-vnc-rfb-w0-proxy-test.service'
W0_BACKEND='ps-to-vnc-w0-backend-test.service'
PROXY_UNIT='/run/systemd/system/ps-to-vnc-rfb-w0-proxy-test.service'
DROPIN='/run/systemd/system/ps-to-vnc-rfb.socket.d/90-w0-proxy-test.conf'
RUNTIME_DIR="/run/user/$(id -u)"
WAYLAND_DISPLAY_NAME='wayland-0'

fail() {
    printf 'FAIL=%s\n' "$1" >&2
    exit 1
}

sha256() {
    sha256sum "$1" | awk '{print $1}'
}

front_connection() {
    sudo -n ss -H -ntp |
        grep -E '^ESTAB .*192\.168\.50\.1(%eth0)?:5900 +192\.168\.50\.2:' ||
        true
}

source_port() {
    sed -nE 's/.*192\.168\.50\.2:([0-9]+).*/\1/p' | head -1
}

printf '%s\n' '===== TEAR DOWN 480P WAYLAND/W0 RFB APPARATUS ====='
[ "$(id -un)" = 'ps2' ] || fail 'RUN_AS_PS2_USER'

FRONT_BEFORE="$(front_connection)"
SOURCE_BEFORE="$(printf '%s\n' "$FRONT_BEFORE" | source_port)"
TRIGGER_BEFORE="$(systemctl show "$SOCKET" -p Triggers --value)"

printf '%s\n' '===== RESTORE CANONICAL SOCKET TARGET IF NEEDED ====='
if [ -e "$DROPIN" ]; then
    sudo -n rm "$DROPIN"
    sudo -n systemctl daemon-reload
fi

TRIGGER="$(systemctl show "$SOCKET" -p Triggers --value)"
case " $TRIGGER " in
    *" $XTIGER "*) ;;
    *) fail 'CANONICAL_XTIGER_TRIGGER_NOT_RESTORED' ;;
esac

case " $TRIGGER_BEFORE " in
    *" $W0_PROXY "*)
        sudo -n systemctl restart "$SOCKET"
        ;;
    *)
        :
        ;;
esac

if [ -n "$SOURCE_BEFORE" ] && [[ " $TRIGGER_BEFORE " == *" $W0_PROXY "* ]]; then
    FOUND='NO'
    for I in $(seq 1 120); do
        FRONT_AFTER="$(front_connection)"
        SOURCE_AFTER="$(printf '%s\n' "$FRONT_AFTER" | source_port)"
        XT_STATE="$(systemctl is-active "$XTIGER" 2>/dev/null || true)"
        if [ -n "$SOURCE_AFTER" ] &&
           [ "$SOURCE_AFTER" != "$SOURCE_BEFORE" ] &&
           [ "$XT_STATE" = 'active' ]; then
            FOUND='YES'
            printf 'XTIGER_RECONNECT_DETECTED_AFTER_HALFSECONDS=%s\n' "$I"
            break
        fi
        sleep 0.5
    done
    [ "$FOUND" = 'YES' ] || fail 'XTIGER_RECONNECT_NOT_OBSERVED_DURING_TEARDOWN'
fi

printf '%s\n' '===== REMOVE SYSTEM W0 PROXY APPARATUS ====='
sudo -n systemctl stop "$W0_PROXY" 2>/dev/null || true
sudo -n systemctl reset-failed "$W0_PROXY" 2>/dev/null || true
sudo -n rm -f "$PROXY_UNIT"
sudo -n systemctl daemon-reload

printf '%s\n' '===== STOP TRANSIENT W0 BACKEND ====='
systemctl --user stop "$W0_BACKEND" 2>/dev/null || true
systemctl --user reset-failed "$W0_BACKEND" 2>/dev/null || true

for I in $(seq 1 20); do
    BACKEND_LOAD="$(systemctl --user show "$W0_BACKEND" -p LoadState --value 2>/dev/null || true)"
    [ -z "$BACKEND_LOAD" ] || [ "$BACKEND_LOAD" = 'not-found' ] && break
    sleep 0.1
done
BACKEND_LOAD="$(systemctl --user show "$W0_BACKEND" -p LoadState --value 2>/dev/null || true)"
[ -z "$BACKEND_LOAD" ] || [ "$BACKEND_LOAD" = 'not-found' ] || fail 'W0_BACKEND_TRANSIENT_UNIT_REMAINS'

printf '%s\n' '===== RESTORE STOCK RASPBERRY PI WAYLAND OUTPUT ====='
env \
    XDG_RUNTIME_DIR="$RUNTIME_DIR" \
    WAYLAND_DISPLAY="$WAYLAND_DISPLAY_NAME" \
    wlr-randr \
        --output NOOP-1 \
        --on \
        --custom-mode 1920x1080 \
        --pos 0,0 \
        --scale 1

printf '%s\n' '===== VERIFY CLEAN EXPERIMENT STATE ====='
[ ! -e "$DROPIN" ] || fail 'W0_SOCKET_OVERRIDE_REMAINS'
[ ! -e "$PROXY_UNIT" ] || fail 'W0_PROXY_UNIT_FILE_REMAINS'
[ "$(systemctl show "$W0_PROXY" -p LoadState --value 2>/dev/null || true)" != 'loaded' ] || fail 'W0_PROXY_UNIT_STILL_LOADED'
! ss -H -lntp | grep -q '127\.0\.0\.1:5901' || fail 'PORT_5901_STILL_LISTENING'

env \
    XDG_RUNTIME_DIR="$RUNTIME_DIR" \
    WAYLAND_DISPLAY="$WAYLAND_DISPLAY_NAME" \
    wlr-randr |
    awk '
        /^NOOP-1 / { inside=1; next }
        /^[^[:space:]]/ { if (inside) exit }
        inside && /Enabled: yes/ { enabled=1 }
        inside && /1920x1080 px \(current\)/ { mode=1 }
        inside && /Position: 0,0/ { position=1 }
        inside && /Transform: normal/ { transform=1 }
        inside && /Scale: 1\.000000/ { scale=1 }
        inside && /Adaptive Sync: disabled/ { adaptive=1 }
        END {
            exit enabled && mode && position &&
                 transform && scale && adaptive ? 0 : 1
        }
    ' || fail 'STOCK_WAYLAND_NOOP1_NOT_RESTORED'

printf '%s\n' '===== RESTORED STOCK WAYLAND STATE ====='
env \
    XDG_RUNTIME_DIR="$RUNTIME_DIR" \
    WAYLAND_DISPLAY="$WAYLAND_DISPLAY_NAME" \
    wlr-randr

TRIGGER="$(systemctl show "$SOCKET" -p Triggers --value)"
case " $TRIGGER " in
    *" $XTIGER "*) ;;
    *) fail 'FINAL_SOCKET_TRIGGER_NOT_XTIGER' ;;
esac
[ "$(systemctl is-active "$SOCKET")" = 'active' ] || fail 'FINAL_SOCKET_NOT_ACTIVE'
[ -x "$W0" ] || fail 'W0_BINARY_WAS_REMOVED'
[ "$(sha256 "$W0")" = "$W0_SHA256" ] || fail 'RETAINED_W0_BINARY_HASH_MISMATCH'

printf 'SOCKET_TRIGGER=%s\n' "$TRIGGER"
printf 'W0_BACKEND_STATE=%s\n' "$(systemctl --user is-active "$W0_BACKEND" 2>/dev/null || true)"
printf 'W0_BINARY_RETAINED=%s\n' "$W0"
printf 'W0_SHA256=%s\n' "$(sha256 "$W0")"
printf 'WAYLAND_STOCK_GEOMETRY=1920x1080\n'
printf 'WAYLAND_W0_APPARATUS_TEARDOWN=PASS\n'
printf 'FTP_USED=NO\n'
