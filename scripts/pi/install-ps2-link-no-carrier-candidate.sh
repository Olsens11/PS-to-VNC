#!/usr/bin/env bash

# Install or remove the scoped NetworkManager no-carrier evaluation snippet.
# Use this ONLY after the read-only baseline proves that the existing static
# ps2-link profile does not own 192.168.50.1/24 before physical carrier.
#
# This script deliberately does NOT reload/restart NetworkManager and does NOT
# activate/deactivate a connection. A reboot or explicit later reload is a
# separate operator-visible experiment step so management connectivity is never
# changed as a side effect of file staging.

set -euo pipefail

ACTION="${1:-}"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)"
SOURCE="$REPO_ROOT/config/pi/NetworkManager/90-ps-to-vnc-ps2-link.conf"
DEST='/etc/NetworkManager/conf.d/90-ps-to-vnc-ps2-link.conf'

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

for tool in awk cmp dirname install rm sha256sum; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "ERROR: required tool missing: $tool" >&2
        exit 4
    fi
done

if [ ! -f "$SOURCE" ]; then
    echo "ERROR: tracked candidate missing: $SOURCE" >&2
    exit 5
fi

case "$ACTION" in
    install)
        if [ -e "$DEST" ] && ! cmp -s "$SOURCE" "$DEST"; then
            echo "ERROR: refusing to overwrite non-identical $DEST" >&2
            exit 10
        fi
        install -d -m 0755 "$(dirname "$DEST")"
        install -m 0644 "$SOURCE" "$DEST"
        echo "NETWORKMANAGER_SNIPPET_SHA256=$(sha256sum "$DEST" | awk '{print $1}')"
        echo 'PS2_LINK_NO_CARRIER_CANDIDATE_INSTALLED=YES'
        echo 'NETWORKMANAGER_RELOAD_PERFORMED=NO'
        echo 'CONNECTION_STATE_CHANGED=NO'
        echo 'NEXT=REBOOT_OR_EXPLICITLY_RELOAD_ONLY_AS_SEPARATE_TEST_STEP'
        ;;
    remove)
        if [ -e "$DEST" ]; then
            if ! cmp -s "$SOURCE" "$DEST"; then
                echo "ERROR: refusing to remove non-identical $DEST" >&2
                exit 20
            fi
            rm -f "$DEST"
        fi
        echo 'PS2_LINK_NO_CARRIER_CANDIDATE_PRESENT=NO'
        echo 'PS2_LINK_NO_CARRIER_CANDIDATE_REMOVED=YES'
        echo 'NETWORKMANAGER_RELOAD_PERFORMED=NO'
        echo 'CONNECTION_STATE_CHANGED=NO'
        ;;
esac
