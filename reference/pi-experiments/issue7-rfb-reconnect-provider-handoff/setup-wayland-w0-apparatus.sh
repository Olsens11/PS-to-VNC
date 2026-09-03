#!/usr/bin/env bash
# File synopsis:
# Prepare the temporary 480p Wayland/w0 side of the Issue #7 RFB provider-
# handoff experiment without changing the currently active PS2-facing provider.
#
# This script verifies the preserved w0vncserver binary, changes the existing
# labwc NOOP-1 output to the tested 704x462 geometry, starts w0vncserver on
# loopback port 5901, and installs the temporary socket-proxy service definition.
# It deliberately does not retarget or restart the durable 192.168.50.1:5900
# socket; switch-rfb-to-wayland.sh owns that live handoff.
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

wayland_state() {
    env \
        XDG_RUNTIME_DIR="$RUNTIME_DIR" \
        WAYLAND_DISPLAY="$WAYLAND_DISPLAY_NAME" \
        wlr-randr
}

wayland_is_480p() {
    wayland_state | awk '
        /^NOOP-1 / { inside=1; next }
        /^[^[:space:]]/ { if (inside) exit }
        inside && /704x462 px \(current\)/ { found=1 }
        END { exit found ? 0 : 1 }
    '
}

printf '%s\n' '===== SET UP 480P WAYLAND/W0 RFB APPARATUS ====='

[ "$(id -un)" = 'ps2' ] || fail 'RUN_AS_PS2_USER'
command -v wlr-randr >/dev/null || fail 'WLR_RANDR_MISSING'
command -v systemd-run >/dev/null || fail 'SYSTEMD_RUN_MISSING'
[ -x /usr/lib/systemd/systemd-socket-proxyd ] || fail 'SYSTEMD_SOCKET_PROXYD_MISSING'
[ -f /etc/systemd/system/ps-to-vnc-rfb.socket ] || fail 'CANONICAL_RFB_SOCKET_MISSING'
[ -f /etc/systemd/system/ps-to-vnc-rfb-tigervnc.service ] || fail 'CANONICAL_XTIGER_PROVIDER_MISSING'
[ ! -e "$DROPIN" ] || fail 'WAYLAND_PROVIDER_ALREADY_SELECTED_RUN_SWITCH_OR_TEARDOWN'

TRIGGER="$(systemctl show "$SOCKET" -p Triggers --value)"
case " $TRIGGER " in
    *" $XTIGER "*) ;;
    *) fail 'CANONICAL_XTIGER_TRIGGER_NOT_ACTIVE' ;;
esac

[ -x "$W0" ] || fail 'W0_BINARY_MISSING_RESTORE_FROM_ARCHIVE_BRANCH'
[ "$(sha256 "$W0")" = "$W0_SHA256" ] || fail 'W0_BINARY_HASH_MISMATCH'

printf '%s\n' '===== FORCE WAYLAND TO THE TESTED 480P GEOMETRY ====='
env \
    XDG_RUNTIME_DIR="$RUNTIME_DIR" \
    WAYLAND_DISPLAY="$WAYLAND_DISPLAY_NAME" \
    wlr-randr \
        --output NOOP-1 \
        --on \
        --custom-mode 704x462 \
        --pos 0,0 \
        --scale 1

wayland_is_480p || fail 'WAYLAND_NOOP1_NOT_704X462'
wayland_state

printf '%s\n' '===== START W0 BACKEND ON LOOPBACK 5901 ====='
if [ "$(systemctl --user is-active "$W0_BACKEND" 2>/dev/null || true)" != 'active' ]; then
    systemctl --user reset-failed "$W0_BACKEND" 2>/dev/null || true

    if [ "$(systemctl --user show "$W0_BACKEND" -p LoadState --value 2>/dev/null || true)" = 'loaded' ]; then
        systemctl --user start "$W0_BACKEND"
    else
        systemd-run \
            --user \
            --quiet \
            --collect \
            --unit="$W0_BACKEND" \
            --setenv="XDG_RUNTIME_DIR=$RUNTIME_DIR" \
            --setenv="WAYLAND_DISPLAY=$WAYLAND_DISPLAY_NAME" \
            --setenv="DBUS_SESSION_BUS_ADDRESS=unix:path=$RUNTIME_DIR/bus" \
            "$W0" \
                -interface 127.0.0.1 \
                -rfbport 5901 \
                -SecurityTypes None \
                -AcceptKeyEvents=0 \
                -AcceptPointerEvents=0 \
                -RememberDisplayChoice Never \
                -FrameRate 30
    fi
fi

W0_READY='NO'

for ATTEMPT in $(seq 1 50); do
    if [ "$(systemctl --user is-active "$W0_BACKEND" 2>/dev/null || true)" = 'active' ] &&
       ss -H -lntp |
           grep -E '127\.0\.0\.1:5901 .*w0vncserver' >/dev/null
    then
        W0_READY='YES'
        break
    fi

    sleep 0.1
done

[ "$W0_READY" = 'YES' ] || fail 'W0_BACKEND_NOT_READY_5901'
[ "$(systemctl --user is-active "$W0_BACKEND")" = 'active' ] || fail 'W0_BACKEND_NOT_ACTIVE'
ss -H -lntp | grep -E '127\.0\.0\.1:5901 .*w0vncserver' >/dev/null || fail 'W0_BACKEND_NOT_LISTENING_5901'

printf '%s\n' '===== INSTALL TEMPORARY SOCKET-PROXY PROVIDER UNIT ====='
TMP="$(mktemp)"
trap 'rm -f "$TMP"' EXIT
cat > "$TMP" <<'UNIT'
[Unit]
Description=Temporary PS-to-VNC w0vncserver proxy provider
Requires=ps-to-vnc-rfb.socket
After=ps-to-vnc-rfb.socket

[Service]
Type=simple
User=ps2
Group=ps2
WorkingDirectory=/home/ps2
UMask=0077
ExecStart=/usr/lib/systemd/systemd-socket-proxyd 127.0.0.1:5901
UNIT

sudo -n install -m 0644 "$TMP" "$PROXY_UNIT"
sudo -n systemctl daemon-reload
sudo -n systemctl stop "$W0_PROXY" 2>/dev/null || true
sudo -n systemctl reset-failed "$W0_PROXY" 2>/dev/null || true

[ "$(systemctl show "$W0_PROXY" -p LoadState --value)" = 'loaded' ] || fail 'W0_PROXY_UNIT_NOT_LOADED'
[ ! -e "$DROPIN" ] || fail 'SETUP_UNEXPECTEDLY_SELECTED_WAYLAND_PROVIDER'
TRIGGER="$(systemctl show "$SOCKET" -p Triggers --value)"
case " $TRIGGER " in
    *" $XTIGER "*) ;;
    *) fail 'SETUP_CHANGED_ACTIVE_PROVIDER' ;;
esac

printf 'W0_SHA256=%s\n' "$(sha256 "$W0")"
printf 'WAYLAND_GEOMETRY=704x462\n'
printf 'W0_BACKEND=127.0.0.1:5901\n'
printf 'ACTIVE_PROVIDER=XTIGERVNC\n'
printf 'WAYLAND_APPARATUS_SETUP=PASS\n'
printf 'FTP_USED=NO\n'
