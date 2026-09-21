#!/usr/bin/env bash
# File synopsis:
# Stage, verify, or remove only the selected Pi-local RFB provider socket/service.
#
# The internal endpoint is infrastructure for a later Wire RFB Relay attachment:
# 127.0.0.1:5900 -> systemd socket -> X0tigervnc -> existing LightDM/Xorg :0.
#
# This tool is deliberately manager-inert. It never stages the preserved direct
# RFB alternative, reloads systemd, changes enablement/running state, activates
# an endpoint, or mutates LightDM/Xorg. The preserved direct units are copied
# only into a temporary static-verification tree so mutual-exclusion composition
# is checked without modifying installed authority.
set -euo pipefail

ACTION="${1:-}"

SCRIPT_DIR="$(
    CDPATH= cd -- "$(dirname -- "$0")" &&
    pwd
)"
REPO_ROOT="$(
    CDPATH= cd -- "$SCRIPT_DIR/../.." &&
    pwd
)"

SOURCE_DIR="$REPO_ROOT/systemd/pi"
DEST_DIR='/etc/systemd/system'

INTERNAL_SOCKET_NAME='ps-to-vnc-rfb-internal.socket'
INTERNAL_SERVICE_NAME='ps-to-vnc-rfb-internal-x0tigervnc.service'

DIRECT_SOCKET_NAME='ps-to-vnc-rfb.socket'
DIRECT_SERVICE_NAME='ps-to-vnc-rfb-tigervnc.service'
DIRECT_PERSISTENT_NAME='ps-to-vnc-rfb-tigervnc-persistent.service'
DIRECT_DROPIN_NAME='90-native-x0vnc.conf'
DIRECT_DROPIN_RELATIVE="$DIRECT_SERVICE_NAME.d/$DIRECT_DROPIN_NAME"

INTERNAL_SOCKET_SOURCE="$SOURCE_DIR/$INTERNAL_SOCKET_NAME"
INTERNAL_SERVICE_SOURCE="$SOURCE_DIR/$INTERNAL_SERVICE_NAME"

DIRECT_SOCKET_SOURCE="$SOURCE_DIR/$DIRECT_SOCKET_NAME"
DIRECT_SERVICE_SOURCE="$SOURCE_DIR/$DIRECT_SERVICE_NAME"
DIRECT_PERSISTENT_SOURCE="$SOURCE_DIR/$DIRECT_PERSISTENT_NAME"
DIRECT_DROPIN_SOURCE="$SOURCE_DIR/$DIRECT_DROPIN_RELATIVE"

INTERNAL_SOCKET_DEST="$DEST_DIR/$INTERNAL_SOCKET_NAME"
INTERNAL_SERVICE_DEST="$DEST_DIR/$INTERNAL_SERVICE_NAME"

TMP_CANDIDATE=''

cleanup()
{
    if [ -n "$TMP_CANDIDATE" ] && [ -d "$TMP_CANDIDATE" ]
    then
        rm -rf "$TMP_CANDIDATE"
    fi
}
trap cleanup EXIT INT TERM

usage()
{
    echo "Usage: sudo $0 {stage|verify|remove}" >&2
}

[ "$ACTION" = stage ] || [ "$ACTION" = verify ] || [ "$ACTION" = remove ] || {
    usage
    exit 2
}

[ "${EUID}" -eq 0 ] || {
    echo 'ERROR: internal RFB provider staging requires root/sudo.' >&2
    exit 3
}

for command_name in \
    cmp \
    cp \
    id \
    install \
    mktemp \
    rm \
    sha256sum \
    stat \
    systemctl \
    systemd-analyze
do
    command -v "$command_name" >/dev/null 2>&1 || {
        echo "ERROR: required command missing: $command_name" >&2
        exit 4
    }
done

normalize_mode()
{
    local mode="$1"

    while [ "${#mode}" -gt 1 ] && [ "${mode#0}" != "$mode" ]
    do
        mode="${mode#0}"
    done

    printf '%s\n' "$mode"
}

require_sources()
{
    local source

    for source in \
        "$INTERNAL_SOCKET_SOURCE" \
        "$INTERNAL_SERVICE_SOURCE" \
        "$DIRECT_SOCKET_SOURCE" \
        "$DIRECT_SERVICE_SOURCE" \
        "$DIRECT_PERSISTENT_SOURCE" \
        "$DIRECT_DROPIN_SOURCE"
    do
        [ -f "$source" ] || {
            echo "ERROR: tracked RFB authority missing: $source" >&2
            exit 10
        }
    done

    id ps2 >/dev/null 2>&1 || {
        echo 'ERROR: expected service user ps2 does not exist.' >&2
        exit 11
    }

    [ -x /usr/bin/X0tigervnc ] || {
        echo 'ERROR: selected native provider /usr/bin/X0tigervnc is absent or not executable.' >&2
        exit 12
    }

    [ -d /home/ps2 ] || {
        echo 'ERROR: expected ps2 home /home/ps2 is absent.' >&2
        exit 13
    }
}

refuse_live_internal_lifecycle()
{
    local name

    for name in "$INTERNAL_SOCKET_NAME" "$INTERNAL_SERVICE_NAME"
    do
        if systemctl is-active --quiet "$name" 2>/dev/null
        then
            echo "ERROR: $name is active; R12 staging is inactive-only." >&2
            exit 20
        fi

        if systemctl is-enabled --quiet "$name" 2>/dev/null
        then
            echo "ERROR: $name is enabled; R12 staging is inactive-only." >&2
            exit 21
        fi
    done

    echo 'RFB_INTERNAL_SOCKET_ACTIVE=NO'
    echo 'RFB_INTERNAL_SOCKET_ENABLED=NO'
    echo 'RFB_INTERNAL_PROVIDER_ACTIVE=NO'
    echo 'RFB_INTERNAL_PROVIDER_ENABLED=NO'
}

assert_safe_target()
{
    local source="$1"
    local target="$2"

    if [ -e "$target" ] && ! cmp -s "$source" "$target"
    then
        echo "ERROR: refusing to overwrite non-identical target: $target" >&2
        exit 30
    fi
}

install_file()
{
    local mode="$1"
    local source="$2"
    local target="$3"

    install -D -m "$mode" "$source" "$target"

    cmp -s "$source" "$target" || {
        echo "ERROR: staged bytes differ: $target" >&2
        exit 31
    }

    echo "INSTALLED_IDENTITY=PASS|$target"
    echo "INSTALLED_SHA256=$(sha256sum "$target" | awk '{print $1}')"
}

verify_file()
{
    local expected_mode="$1"
    local source="$2"
    local target="$3"
    local actual_mode

    [ -f "$target" ] || {
        echo "ERROR: expected internal RFB provider file absent: $target" >&2
        exit 40
    }

    cmp -s "$source" "$target" || {
        echo "ERROR: installed bytes differ from tracked source: $target" >&2
        exit 41
    }

    actual_mode="$(stat -c '%a' "$target")"
    [ "$(normalize_mode "$actual_mode")" = "$(normalize_mode "$expected_mode")" ] || {
        echo "ERROR: wrong mode on $target: actual=$actual_mode expected=$expected_mode" >&2
        exit 42
    }

    echo "INSTALLED_IDENTITY=PASS|$target"
}

verify_composed_candidate()
{
    TMP_CANDIDATE="$(mktemp -d)"
    install -d -m 0755 "$TMP_CANDIDATE/$DIRECT_SERVICE_NAME.d"

    cp "$INTERNAL_SOCKET_SOURCE" "$TMP_CANDIDATE/$INTERNAL_SOCKET_NAME"
    cp "$INTERNAL_SERVICE_SOURCE" "$TMP_CANDIDATE/$INTERNAL_SERVICE_NAME"

    # The direct files are validation inputs only. They are never installation
    # targets of this R12 tool.
    cp "$DIRECT_SOCKET_SOURCE" "$TMP_CANDIDATE/$DIRECT_SOCKET_NAME"
    cp "$DIRECT_SERVICE_SOURCE" "$TMP_CANDIDATE/$DIRECT_SERVICE_NAME"
    cp "$DIRECT_PERSISTENT_SOURCE" "$TMP_CANDIDATE/$DIRECT_PERSISTENT_NAME"
    cp "$DIRECT_DROPIN_SOURCE" "$TMP_CANDIDATE/$DIRECT_DROPIN_RELATIVE"

    SYSTEMD_UNIT_PATH="$TMP_CANDIDATE:" systemd-analyze verify \
        "$INTERNAL_SOCKET_NAME" \
        "$INTERNAL_SERVICE_NAME" \
        "$DIRECT_SOCKET_NAME" \
        "$DIRECT_SERVICE_NAME" \
        "$DIRECT_PERSISTENT_NAME"

    rm -rf "$TMP_CANDIDATE"
    TMP_CANDIDATE=''

    echo 'RFB_INTERNAL_COMPOSED_SYSTEMD_STATIC_VERIFY=PASS'
}

stage_candidate()
{
    require_sources
    refuse_live_internal_lifecycle
    verify_composed_candidate

    assert_safe_target "$INTERNAL_SOCKET_SOURCE" "$INTERNAL_SOCKET_DEST"
    assert_safe_target "$INTERNAL_SERVICE_SOURCE" "$INTERNAL_SERVICE_DEST"

    install_file 0644 "$INTERNAL_SOCKET_SOURCE" "$INTERNAL_SOCKET_DEST"
    install_file 0644 "$INTERNAL_SERVICE_SOURCE" "$INTERNAL_SERVICE_DEST"

    echo 'SELECTED_RFB_INTERNAL_ENDPOINT=127.0.0.1:5900'
    echo 'SELECTED_RFB_INTERNAL_PROVIDER=X0tigervnc_existing_display_0'
    echo 'SELECTED_RFB_INTERNAL_PROVIDER_EXECUTABLE=/usr/bin/X0tigervnc'
    echo 'SELECTED_RFB_INTERNAL_PROVIDER_DISPLAY=:0'
    echo 'SELECTED_RFB_INTERNAL_PROVIDER_XAUTHORITY=/home/ps2/.Xauthority'
    echo 'SELECTED_RFB_INTERNAL_PROVIDER_INHERITED_SOCKET=YES'
    echo 'SELECTED_RFB_INTERNAL_PROVIDER_SECOND_LISTENER=NO'
    echo 'DIRECT_RFB_AUTHORITY_STAGED_BY_THIS_TOOL=NO'
    echo 'SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED'
    echo 'RFB_INTERNAL_ENABLEMENT=NOT_CHANGED'
    echo 'RFB_INTERNAL_RUNNING_STATE=NOT_CHANGED'
    echo 'LIGHTDM_XORG_STATE=NOT_CHANGED'
    echo 'RFB_INTERNAL_PROVIDER_STAGE=PASS'
}

verify_candidate()
{
    require_sources
    refuse_live_internal_lifecycle

    verify_file 0644 "$INTERNAL_SOCKET_SOURCE" "$INTERNAL_SOCKET_DEST"
    verify_file 0644 "$INTERNAL_SERVICE_SOURCE" "$INTERNAL_SERVICE_DEST"

    verify_composed_candidate

    echo 'SELECTED_RFB_INTERNAL_ENDPOINT=127.0.0.1:5900'
    echo 'DIRECT_RFB_AUTHORITY_STAGED_BY_THIS_TOOL=NO'
    echo 'SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED'
    echo 'RFB_INTERNAL_PROVIDER_VERIFY=PASS'
}

remove_candidate()
{
    local source target

    require_sources
    refuse_live_internal_lifecycle

    while IFS='|' read -r source target
    do
        if [ ! -e "$target" ]
        then
            echo "ALREADY_ABSENT=$target"
            continue
        fi

        cmp -s "$source" "$target" || {
            echo "ERROR: refusing to remove non-identical target: $target" >&2
            exit 50
        }

        rm -f "$target"
        echo "REMOVED=$target"
    done <<__PS2VNC_RFB_INTERNAL_REMOVE_EOF__
$INTERNAL_SERVICE_SOURCE|$INTERNAL_SERVICE_DEST
$INTERNAL_SOCKET_SOURCE|$INTERNAL_SOCKET_DEST
__PS2VNC_RFB_INTERNAL_REMOVE_EOF__

    echo 'DIRECT_RFB_AUTHORITY_STAGED_BY_THIS_TOOL=NO'
    echo 'SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED'
    echo 'RFB_INTERNAL_ENABLEMENT=NOT_CHANGED'
    echo 'RFB_INTERNAL_RUNNING_STATE=NOT_CHANGED'
    echo 'LIGHTDM_XORG_STATE=NOT_CHANGED'
    echo 'RFB_INTERNAL_PROVIDER_REMOVE=PASS'
}

case "$ACTION" in
    stage)
        stage_candidate
        ;;
    verify)
        verify_candidate
        ;;
    remove)
        remove_candidate
        ;;
esac
