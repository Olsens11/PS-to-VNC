#!/usr/bin/env bash

# Install or remove the tracked Issue #5 RFB socket-activation unit candidate.
# This script deliberately does NOT enable, start, stop, restart, or mask units.
# Service activation remains a separate operator-visible experiment step.

set -euo pipefail

ACTION="${1:-}"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)"
SOURCE_DIR="$REPO_ROOT/systemd/pi"
DEST_DIR='/etc/systemd/system'
SOCKET_NAME='ps-to-vnc-rfb.socket'
SERVICE_NAME='ps-to-vnc-rfb.service'

usage() {
    echo "Usage: sudo $0 {install|remove}" >&2
}

if [ "$ACTION" != install ] && [ "$ACTION" != remove ]; then
    usage
    exit 2
fi

if [ "${EUID}" -ne 0 ]; then
    echo 'ERROR: run this script through sudo/root.' >&2
    exit 3
fi

for tool in cmp install systemd-analyze systemctl sha256sum id; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "ERROR: required tool missing: $tool" >&2
        exit 4
    fi
done

for name in "$SOCKET_NAME" "$SERVICE_NAME"; do
    if [ ! -f "$SOURCE_DIR/$name" ]; then
        echo "ERROR: tracked candidate missing: $SOURCE_DIR/$name" >&2
        exit 5
    fi
done

install_candidate() {
    local tmpdir target name

    if ! id ps2 >/dev/null 2>&1; then
        echo 'ERROR: expected service user ps2 does not exist.' >&2
        exit 10
    fi
    if [ ! -x /usr/bin/Xtigervnc ]; then
        echo 'ERROR: /usr/bin/Xtigervnc is absent or not executable.' >&2
        exit 11
    fi

    # Validate the exact candidate pair before touching /etc. Copying to a
    # temporary directory preserves the real unit basenames for dependency
    # resolution during systemd-analyze verify.
    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' EXIT
    cp "$SOURCE_DIR/$SOCKET_NAME" "$tmpdir/$SOCKET_NAME"
    cp "$SOURCE_DIR/$SERVICE_NAME" "$tmpdir/$SERVICE_NAME"
    systemd-analyze verify "$tmpdir/$SOCKET_NAME" "$tmpdir/$SERVICE_NAME"

    # Fail closed on any pre-existing definition that is not byte-for-byte the
    # candidate being installed. Never overwrite unknown or hand-edited units.
    for name in "$SOCKET_NAME" "$SERVICE_NAME"; do
        target="$DEST_DIR/$name"
        if [ -e "$target" ] && ! cmp -s "$SOURCE_DIR/$name" "$target"; then
            echo "ERROR: refusing to overwrite non-identical $target" >&2
            exit 12
        fi
    done

    install -d -m 0755 "$DEST_DIR"
    install -m 0644 "$SOURCE_DIR/$SOCKET_NAME" "$DEST_DIR/$SOCKET_NAME"
    install -m 0644 "$SOURCE_DIR/$SERVICE_NAME" "$DEST_DIR/$SERVICE_NAME"
    systemctl daemon-reload

    echo "SOCKET_SHA256=$(sha256sum "$DEST_DIR/$SOCKET_NAME" | awk '{print $1}')"
    echo "SERVICE_SHA256=$(sha256sum "$DEST_DIR/$SERVICE_NAME" | awk '{print $1}')"
    echo 'RFB_ACTIVATION_UNITS_INSTALLED=YES'
    echo 'RFB_ACTIVATION_UNITS_ENABLED=NO_CHANGE'
    echo 'RFB_ACTIVATION_UNITS_STARTED=NO_CHANGE'
    echo 'NEXT=VERIFY_PORT_AND_DISPLAY_ARE_FREE_THEN_START_SOCKET_EXPLICITLY'
}

remove_candidate() {
    local target name

    # Removal is equally fail closed. An active unit must be stopped as its own
    # explicit experiment/rollback step before its definition is removed.
    for name in "$SOCKET_NAME" "$SERVICE_NAME"; do
        if systemctl is-active --quiet "$name" 2>/dev/null; then
            echo "ERROR: $name is active; stop it explicitly before removal." >&2
            exit 20
        fi
    done

    for name in "$SOCKET_NAME" "$SERVICE_NAME"; do
        target="$DEST_DIR/$name"
        if [ -e "$target" ]; then
            if ! cmp -s "$SOURCE_DIR/$name" "$target"; then
                echo "ERROR: refusing to remove non-identical $target" >&2
                exit 21
            fi
            rm -f "$target"
        fi
    done

    systemctl daemon-reload
    echo 'RFB_ACTIVATION_UNITS_PRESENT=NO'
    echo 'RFB_ACTIVATION_UNITS_REMOVED=YES'
}

case "$ACTION" in
    install) install_candidate ;;
    remove) remove_candidate ;;
esac
