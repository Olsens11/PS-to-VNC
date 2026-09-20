#!/usr/bin/env bash

# File synopsis:
# Read-only inspection of the effective PS-to-VNC RFB provider definition.
#
# This exists because systemd drop-ins can materially change a unit even when
# the tracked fragment itself still matches qualified source. Before the staged
# minimum desktop is attached to the real provider lifecycle, expose the merged
# manager view and every local drop-in byte-for-byte.
#
# This script performs no mutation.

set -u

SCRIPT_DIR="$(
    CDPATH= cd -- "$(dirname -- "$0")" &&
    pwd
)"
REPO_ROOT="$(
    CDPATH= cd -- "$SCRIPT_DIR/../.." &&
    pwd
)"

PROVIDER='ps-to-vnc-rfb-tigervnc.service'
SOCKET='ps-to-vnc-rfb.socket'
DESKTOP='ps-to-vnc-desktop.service'
DROPIN_DIR="/etc/systemd/system/$PROVIDER.d"

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
    echo 'ERROR: run through sudo/root for complete systemd evidence.' >&2
    exit 2
fi

for tool in     cat find git pgrep ps readlink sed sha256sum stat systemctl systemd-delta     tr xprop
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

section 'EFFECTIVE PROVIDER MANAGER VIEW'
run systemctl show     "$PROVIDER"     -p FragmentPath     -p SourcePath     -p DropInPaths     -p LoadState     -p UnitFileState     -p ActiveState     -p SubState     -p MainPID     -p User     -p Group     -p StandardInput     -p ExecStart     -p Requires     -p Requisite     -p BindsTo     -p Wants     -p After     -p TriggeredBy

section 'SYSTEMCTL CAT — MERGED PROVIDER DEFINITION'
run systemctl cat "$PROVIDER"

section 'LOCAL PROVIDER DROP-INS'
if [ -d "$DROPIN_DIR" ]; then
    echo "DROPIN_DIR_PRESENT=YES"
    while IFS= read -r path; do
        [ -n "$path" ] || continue
        echo "--- DROPIN=$path ---"
        run stat -c '%n|mode=%a|uid=%u|gid=%g|size=%s' "$path"
        run sha256sum "$path"
        echo '--- CONTENT BEGIN ---'
        cat "$path"
        echo '--- CONTENT END ---'
    done < <(find "$DROPIN_DIR" -maxdepth 1 -type f -name '*.conf' -print | sort)
else
    echo "DROPIN_DIR_PRESENT=NO"
fi

section 'SYSTEMD DELTA'
run systemd-delta --type=extended "$PROVIDER"

section 'TRACKED PROVIDER FRAGMENT'
run sha256sum "$REPO_ROOT/systemd/pi/$PROVIDER"
cat "$REPO_ROOT/systemd/pi/$PROVIDER"

section 'SOCKET AND DESKTOP RELATION'
run systemctl show     "$SOCKET"     -p FragmentPath     -p DropInPaths     -p LoadState     -p UnitFileState     -p ActiveState     -p SubState     -p Triggers
run systemctl show     "$DESKTOP"     -p FragmentPath     -p DropInPaths     -p LoadState     -p UnitFileState     -p ActiveState     -p SubState     -p Requisite     -p BindsTo     -p Wants     -p After

section 'DISPLAY OWNERSHIP WITNESS'
for display in :0 :1; do
    echo "--- DISPLAY=$display ---"
    run xprop -display "$display" -root _NET_SUPPORTING_WM_CHECK
    run xprop -display "$display" -root _NET_CLIENT_LIST
done

section 'GRAPHICAL PROCESS WITNESS'
for name in Xtigervnc Xorg Xwayland x0vncserver w0vncserver openbox lxpanel-pi; do
    echo "--- PROCESS_NAME=$name ---"
    run pgrep -a "$name"
done

for pid in $(pgrep -x Xtigervnc 2>/dev/null; pgrep -x x0vncserver 2>/dev/null; pgrep -x w0vncserver 2>/dev/null); do
    [ -r "/proc/$pid/status" ] || continue
    echo "--- PROVIDER_PID=$pid ---"
    run ps -o pid=,ppid=,user=,lstart=,cmd= -p "$pid"
    if [ -r "/proc/$pid/cgroup" ]; then
        echo "CGROUP_PID=$pid"
        sed 's/^/  /' "/proc/$pid/cgroup" 2>/dev/null || true
    fi
    if [ -r "/proc/$pid/environ" ]; then
        echo "SELECTED_ENV_PID=$pid"
        tr '\0' '\n' <"/proc/$pid/environ" 2>/dev/null |
            grep -E '^(DISPLAY|XDG_RUNTIME_DIR|DBUS_SESSION_BUS_ADDRESS|WAYLAND_DISPLAY)=' |
            sed 's/^/  /' || true
    fi
done

section 'CLASSIFICATION'
echo 'MUTATION_PERFORMED=NO'
echo 'SYSTEMCTL_DAEMON_RELOAD=NO'
echo 'UNIT_ENABLE_CHANGE=NO'
echo 'UNIT_START_STOP_CHANGE=NO'
echo 'RFB_PROVIDER_STATE_CHANGED=NO'
echo 'DISPLAY_STATE_CHANGED=NO'
echo 'NEXT=CLASSIFY_EFFECTIVE_PROVIDER_BEFORE_REAL_DISPLAY1_ACTIVATION'
echo 'RFB_EFFECTIVE_PROVIDER_INSPECTION=COMPLETE'
