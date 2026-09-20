#!/usr/bin/env bash

# File synopsis:
# Read-only preflight for the first controlled application of the staged
# PS-to-VNC minimum desktop to the real qualified DISPLAY=:1 provider.
#
# This script deliberately does NOT:
#   - call systemctl daemon-reload;
#   - enable, start, stop, restart, kill, or reset any unit/process;
#   - alter the RFB listener/provider;
#   - alter the staged desktop files;
#   - create evidence files.
#
# Its purpose is to expose the exact live fixture that the later activation
# transaction must replace without weakening the qualified Issue #5 boundary.

set -u

SCRIPT_DIR="$(
    CDPATH= cd -- "$(dirname -- "$0")" &&
    pwd
)"
REPO_ROOT="$(
    CDPATH= cd -- "$SCRIPT_DIR/../.." &&
    pwd
)"

DESKTOP_UNIT='ps-to-vnc-desktop.service'
RFB_SOCKET='ps-to-vnc-rfb.socket'
RFB_PROVIDER='ps-to-vnc-rfb-tigervnc.service'
RFB_PERSISTENT='ps-to-vnc-rfb-tigervnc-persistent.service'

section()
{
    printf '\n===== %s =====\n' "$1"
}

run()
{
    printf '+ '
    printf '%q ' "$@"
    printf '\n'
    "$@" 2>&1 || printf 'COMMAND_EXIT=%d\n' "$?"
}

if [ "${EUID}" -ne 0 ]; then
    echo 'ERROR: run through sudo/root for complete process and systemd evidence.' >&2
    exit 2
fi

for tool in     git grep loginctl pgrep ps sed sha256sum ss stat systemctl     systemd-analyze tr xprop
do
    command -v "$tool" >/dev/null 2>&1 || {
        echo "ERROR: required tool missing: $tool" >&2
        exit 3
    }
done

section 'REPOSITORY AUTHORITY'
echo "REPO_ROOT=$REPO_ROOT"
run git -C "$REPO_ROOT" branch --show-current
run git -C "$REPO_ROOT" rev-parse HEAD
run git -C "$REPO_ROOT" status --short --branch

section 'STAGED DESKTOP BYTE / FOUNDATION VERIFY'
run "$REPO_ROOT/scripts/pi/install-minimum-desktop.sh" verify

section 'SYSTEMD MANAGER VIEW — NO DAEMON RELOAD'
for unit in     "$DESKTOP_UNIT"     "$RFB_SOCKET"     "$RFB_PROVIDER"     "$RFB_PERSISTENT"
do
    echo "--- UNIT=$unit ---"
    run systemctl show         "$unit"         -p LoadState         -p UnitFileState         -p ActiveState         -p SubState         -p MainPID
    run systemctl is-enabled "$unit"
    run systemctl is-active "$unit"
done

section 'STAGED DESKTOP UNIT STATIC VERIFY'
run systemd-analyze verify /etc/systemd/system/ps-to-vnc-desktop.service
run sha256sum /etc/systemd/system/ps-to-vnc-desktop.service
run sha256sum "$REPO_ROOT/systemd/pi/ps-to-vnc-desktop.service"

section 'QUALIFIED RFB ENDPOINT LIVE STATE'
run ss -ltnp 'sport = :5900'
run pgrep -a Xtigervnc
run systemctl status --no-pager "$RFB_SOCKET" "$RFB_PROVIDER"

section 'DISPLAY :1 KERNEL / X STATE'
for path in /tmp/.X1-lock /tmp/.X11-unix/X1; do
    if [ -e "$path" ]; then
        echo "PRESENT=$path"
        run stat -c '%n|mode=%a|uid=%u|gid=%g|size=%s' "$path"
    else
        echo "ABSENT=$path"
    fi
done
run xprop -display :1 -root _NET_SUPPORTING_WM_CHECK
run xprop -display :1 -root _NET_CLIENT_LIST

section 'CURRENT DESKTOP PROCESS OWNERSHIP'
for name in openbox lxpanel-pi lxterminal; do
    echo "--- PROCESS_NAME=$name ---"
    run pgrep -a "$name"
done

for pid in $(pgrep -x openbox 2>/dev/null; pgrep -x lxpanel-pi 2>/dev/null); do
    [ -r "/proc/$pid/status" ] || continue

    echo "--- GRAPHICAL_PID=$pid ---"
    run ps -o pid=,ppid=,user=,lstart=,cmd= -p "$pid"

    if [ -r "/proc/$pid/cgroup" ]; then
        echo "CGROUP_PID=$pid"
        sed 's/^/  /' "/proc/$pid/cgroup" 2>/dev/null || true
    fi

    if [ -r "/proc/$pid/environ" ]; then
        echo "SELECTED_ENV_PID=$pid"
        tr '\0' '\n' <"/proc/$pid/environ" 2>/dev/null |
            grep -E '^(DISPLAY|XDG_RUNTIME_DIR|DBUS_SESSION_BUS_ADDRESS|WAYLAND_DISPLAY|XDG_CONFIG_HOME|XDG_CACHE_HOME|XDG_DATA_HOME|XDG_STATE_HOME)=' |
            sed 's/^/  /' || true
    fi
done

section 'PHYSICAL PI DESKTOP COEXISTENCE WITNESS'
run pgrep -a lightdm
run pgrep -a labwc
run pgrep -a wf-panel-pi
run loginctl list-sessions --no-legend

section 'ACTIVATION PREFLIGHT SUMMARY'
echo 'MUTATION_PERFORMED=NO'
echo 'SYSTEMCTL_DAEMON_RELOAD=NO'
echo 'DESKTOP_SERVICE_ENABLE_CHANGE=NO'
echo 'DESKTOP_SERVICE_START_CHANGE=NO'
echo 'RFB_PROVIDER_STATE_CHANGED=NO'
echo 'REAL_DISPLAY1_MUTATION=NO'
echo 'NEXT=DESIGN_EXACT_STOP_RELOAD_ENABLE_RETRIGGER_TRANSACTION_FROM_THIS_LIVE_FIXTURE'
echo 'MINIMUM_DESKTOP_DISPLAY1_PREFLIGHT=COMPLETE'
