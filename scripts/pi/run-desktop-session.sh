#!/usr/bin/env bash

# File synopsis:
# Run the PS-to-VNC X11 desktop contents inside one already-created private
# runtime directory and one DBus session.
#
# Expected outer lifecycle:
#
#   systemd
#     -> dbus-run-session
#        -> this script
#           -> Openbox
#           -> lxpanel-pi
#
# The RFB/X server remains a separate qualified Issue #5 responsibility.

set -euo pipefail

OPENBOX_RC="${PS2VNC_OPENBOX_RC:-/etc/ps-to-vnc/desktop/openbox/rc.xml}"

: "${DISPLAY:?DISPLAY must identify the dedicated PS-to-VNC X server}"
: "${XDG_RUNTIME_DIR:?XDG_RUNTIME_DIR must identify the private session runtime}"
: "${XDG_CONFIG_HOME:?XDG_CONFIG_HOME must be set}"
: "${XDG_CACHE_HOME:?XDG_CACHE_HOME must be set}"
: "${XDG_DATA_HOME:?XDG_DATA_HOME must be set}"
: "${XDG_STATE_HOME:?XDG_STATE_HOME must be set}"
: "${DBUS_SESSION_BUS_ADDRESS:?run this script under dbus-run-session}"

[ -f "$OPENBOX_RC" ] || {
    echo "ERROR: Openbox configuration missing: $OPENBOX_RC" >&2
    exit 10
}

[ -d "$XDG_RUNTIME_DIR" ] || {
    echo "ERROR: private runtime directory missing: $XDG_RUNTIME_DIR" >&2
    exit 11
}

RUNTIME_UID="$(
    stat \
        -c '%u' \
        "$XDG_RUNTIME_DIR"
)"

RUNTIME_MODE="$(
    stat \
        -c '%a' \
        "$XDG_RUNTIME_DIR"
)"

[ "$RUNTIME_UID" -eq "$(id -u)" ] || {
    echo 'ERROR: private runtime directory has wrong owner.' >&2
    exit 12
}

[ "$RUNTIME_MODE" = 700 ] || {
    echo 'ERROR: private runtime directory must have mode 0700.' >&2
    exit 13
}

mkdir -p \
    "$XDG_CONFIG_HOME" \
    "$XDG_CACHE_HOME" \
    "$XDG_DATA_HOME" \
    "$XDG_STATE_HOME"

echo "DESKTOP_SESSION_DISPLAY=$DISPLAY"
echo "DESKTOP_SESSION_RUNTIME=$XDG_RUNTIME_DIR"
echo "DESKTOP_SESSION_DBUS=$DBUS_SESSION_BUS_ADDRESS"

X_READY=NO

for attempt in $(seq 1 50)
do
    if /usr/bin/xprop \
           -display "$DISPLAY" \
           -root \
           >/dev/null 2>&1
    then
        X_READY=YES
        echo "DESKTOP_X_READY_ATTEMPT=$attempt"
        break
    fi

    /usr/bin/sleep 0.10
done

[ "$X_READY" = YES ] || {
    echo "ERROR: X display did not become ready: $DISPLAY" >&2
    exit 14
}

OPENBOX_PID=''
PANEL_PID=''

cleanup()
{
    local rc=$?

    trap - EXIT INT TERM

    for pid in \
        "${PANEL_PID:-}" \
        "${OPENBOX_PID:-}"
    do
        [ -n "$pid" ] || continue

        if kill -0 "$pid" 2>/dev/null
        then
            kill \
                -TERM \
                "$pid" \
                2>/dev/null \
                || true
        fi
    done

    [ -n "${PANEL_PID:-}" ] && \
        wait "$PANEL_PID" 2>/dev/null || true

    [ -n "${OPENBOX_PID:-}" ] && \
        wait "$OPENBOX_PID" 2>/dev/null || true

    exit "$rc"
}

trap cleanup EXIT
trap 'exit 143' TERM
trap 'exit 130' INT

/usr/bin/openbox \
    --config-file "$OPENBOX_RC" \
    --sm-disable &

OPENBOX_PID=$!

WM_READY=NO

for attempt in $(seq 1 50)
do
    if ! kill -0 "$OPENBOX_PID" 2>/dev/null
    then
        wait "$OPENBOX_PID" || true
        echo 'ERROR: Openbox exited before becoming window manager.' >&2
        exit 15
    fi

    if /usr/bin/xprop \
           -display "$DISPLAY" \
           -root \
           _NET_SUPPORTING_WM_CHECK \
           2>/dev/null |
       grep -q \
           'window id'
    then
        WM_READY=YES
        echo "OPENBOX_READY_ATTEMPT=$attempt"
        break
    fi

    /usr/bin/sleep 0.10
done

[ "$WM_READY" = YES ] || {
    echo 'ERROR: Openbox never published EWMH window-manager ownership.' >&2
    exit 16
}

/usr/bin/lxpanel-pi &

PANEL_PID=$!

/usr/bin/sleep 1

if ! kill -0 "$PANEL_PID" 2>/dev/null
then
    set +e
    wait "$PANEL_PID"
    PANEL_RC=$?
    set -e

    echo "ERROR: lxpanel-pi exited during startup; rc=$PANEL_RC" >&2
    exit "$PANEL_RC"
fi

echo "OPENBOX_PID=$OPENBOX_PID"
echo "LXPANEL_PI_PID=$PANEL_PID"
echo 'PS_TO_VNC_DESKTOP_CONTENTS_READY=YES'

set +e

wait \
    -n \
    "$OPENBOX_PID" \
    "$PANEL_PID"

CHILD_RC=$?

set -e

if ! kill -0 "$OPENBOX_PID" 2>/dev/null
then
    echo "DESKTOP_CHILD_EXIT=OPENBOX|RC=$CHILD_RC"
elif ! kill -0 "$PANEL_PID" 2>/dev/null
then
    echo "DESKTOP_CHILD_EXIT=LXPANEL_PI|RC=$CHILD_RC"
else
    echo "DESKTOP_CHILD_EXIT=UNKNOWN|RC=$CHILD_RC"
fi

exit "$CHILD_RC"
