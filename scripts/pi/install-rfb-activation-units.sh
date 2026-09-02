#!/usr/bin/env bash

# Install or remove the tracked Issue #5 RFB lifecycle evaluation units.
# This script deliberately does NOT enable, start, stop, restart, or mask units.
# Service activation remains a separate operator-visible experiment step.

set -euo pipefail

ACTION="${1:-}"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)"
SOURCE_DIR="$REPO_ROOT/systemd/pi"
DEST_DIR='/etc/systemd/system'
SOCKET_NAME='ps-to-vnc-rfb.socket'
ACTIVATED_SERVICE_NAME='ps-to-vnc-rfb-tigervnc.service'
PERSISTENT_SERVICE_NAME='ps-to-vnc-rfb-tigervnc-persistent.service'
LEGACY_GENERIC_SERVICE_NAME='ps-to-vnc-rfb.service'
TMP_CANDIDATE=''

cleanup() {
    if [ -n "$TMP_CANDIDATE" ] && [ -d "$TMP_CANDIDATE" ]; then
        rm -rf "$TMP_CANDIDATE"
    fi
}
trap cleanup EXIT INT TERM

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

for tool in awk cmp cp id install mktemp rm sha256sum systemctl systemd-analyze; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "ERROR: required tool missing: $tool" >&2
        exit 4
    fi
done

UNIT_NAMES=(
    "$SOCKET_NAME"
    "$ACTIVATED_SERVICE_NAME"
    "$PERSISTENT_SERVICE_NAME"
)

for name in "${UNIT_NAMES[@]}"; do
    if [ ! -f "$SOURCE_DIR/$name" ]; then
        echo "ERROR: tracked candidate missing: $SOURCE_DIR/$name" >&2
        exit 5
    fi
done

install_candidate() {
    local target name

    if ! id ps2 >/dev/null 2>&1; then
        echo 'ERROR: expected service user ps2 does not exist.' >&2
        exit 10
    fi
    if [ ! -x /usr/bin/Xtigervnc ]; then
        echo 'ERROR: /usr/bin/Xtigervnc is absent or not executable.' >&2
        exit 11
    fi
    if [ -e "$DEST_DIR/$LEGACY_GENERIC_SERVICE_NAME" ]; then
        echo "ERROR: stale/unknown $DEST_DIR/$LEGACY_GENERIC_SERVICE_NAME exists; inspect it before staging the provider-specific units." >&2
        exit 12
    fi

    # Validate the exact candidate set before touching /etc. Copying to a
    # temporary directory preserves real unit basenames for dependency
    # resolution during systemd-analyze verify.
    TMP_CANDIDATE="$(mktemp -d)"
    for name in "${UNIT_NAMES[@]}"; do
        cp "$SOURCE_DIR/$name" "$TMP_CANDIDATE/$name"
    done
    systemd-analyze verify \
        "$TMP_CANDIDATE/$SOCKET_NAME" \
        "$TMP_CANDIDATE/$ACTIVATED_SERVICE_NAME" \
        "$TMP_CANDIDATE/$PERSISTENT_SERVICE_NAME"
    rm -rf "$TMP_CANDIDATE"
    TMP_CANDIDATE=''

    # Fail closed on any pre-existing definition that is not byte-for-byte the
    # candidate being installed. Never overwrite unknown or hand-edited units.
    for name in "${UNIT_NAMES[@]}"; do
        target="$DEST_DIR/$name"
        if [ -e "$target" ] && ! cmp -s "$SOURCE_DIR/$name" "$target"; then
            echo "ERROR: refusing to overwrite non-identical $target" >&2
            exit 13
        fi
    done

    install -d -m 0755 "$DEST_DIR"
    for name in "${UNIT_NAMES[@]}"; do
        install -m 0644 "$SOURCE_DIR/$name" "$DEST_DIR/$name"
        echo "UNIT_SHA256_${name//[^A-Za-z0-9]/_}=$(sha256sum "$DEST_DIR/$name" | awk '{print $1}')"
    done
    systemctl daemon-reload

    echo 'RFB_LIFECYCLE_UNITS_INSTALLED=YES'
    echo 'RFB_LIFECYCLE_UNITS_ENABLED=NO_CHANGE'
    echo 'RFB_LIFECYCLE_UNITS_STARTED=NO_CHANGE'
    echo 'NEXT=VERIFY_PORT_AND_DISPLAY_ARE_FREE_THEN_CHOOSE_EXACTLY_ONE_CONTROL_OR_SOCKET_ACTIVATION_PATH'
}

remove_candidate() {
    local target name

    # Removal is equally fail closed. An active unit must be stopped as its own
    # explicit experiment/rollback step before its definition is removed.
    for name in "${UNIT_NAMES[@]}"; do
        if systemctl is-active --quiet "$name" 2>/dev/null; then
            echo "ERROR: $name is active; stop it explicitly before removal." >&2
            exit 20
        fi
    done

    for name in "${UNIT_NAMES[@]}"; do
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
    echo 'RFB_LIFECYCLE_UNITS_PRESENT=NO'
    echo 'RFB_LIFECYCLE_UNITS_REMOVED=YES'
}

case "$ACTION" in
    install) install_candidate ;;
    remove) remove_candidate ;;
esac
