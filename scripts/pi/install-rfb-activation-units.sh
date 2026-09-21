#!/usr/bin/env bash
# File synopsis:
# Stage, verify, or remove the tracked PS-to-VNC direct-RFB activation
# definitions plus the selected native-desktop X0tigervnc provider drop-in.
#
# This tool is deliberately manager-inert. It performs exact-byte/mode staging,
# static composed-unit validation, and read-only lifecycle checks only. It never
# reloads systemd, changes enablement, starts/stops/restarts a unit, or changes
# the live LightDM/Xorg desktop. Any live transition is a separate bounded
# qualification step.
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

SOCKET_NAME='ps-to-vnc-rfb.socket'
ACTIVATED_SERVICE_NAME='ps-to-vnc-rfb-tigervnc.service'
PERSISTENT_SERVICE_NAME='ps-to-vnc-rfb-tigervnc-persistent.service'
DROPIN_NAME='90-native-x0vnc.conf'
DROPIN_RELATIVE="$ACTIVATED_SERVICE_NAME.d/$DROPIN_NAME"

SOCKET_SOURCE="$SOURCE_DIR/$SOCKET_NAME"
ACTIVATED_SERVICE_SOURCE="$SOURCE_DIR/$ACTIVATED_SERVICE_NAME"
PERSISTENT_SERVICE_SOURCE="$SOURCE_DIR/$PERSISTENT_SERVICE_NAME"
DROPIN_SOURCE="$SOURCE_DIR/$DROPIN_RELATIVE"

SOCKET_DEST="$DEST_DIR/$SOCKET_NAME"
ACTIVATED_SERVICE_DEST="$DEST_DIR/$ACTIVATED_SERVICE_NAME"
PERSISTENT_SERVICE_DEST="$DEST_DIR/$PERSISTENT_SERVICE_NAME"
DROPIN_DEST="$DEST_DIR/$DROPIN_RELATIVE"

LEGACY_GENERIC_SERVICE_DEST="$DEST_DIR/ps-to-vnc-rfb.service"
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
    echo 'ERROR: RFB provider staging requires root/sudo.' >&2
    exit 3
}

for command_name in \
    cmp \
    cp \
    id \
    install \
    mktemp \
    rm \
    rmdir \
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
        "$SOCKET_SOURCE" \
        "$ACTIVATED_SERVICE_SOURCE" \
        "$PERSISTENT_SERVICE_SOURCE" \
        "$DROPIN_SOURCE"
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

    if [ -e "$LEGACY_GENERIC_SERVICE_DEST" ]
    then
        echo "ERROR: stale/unknown $LEGACY_GENERIC_SERVICE_DEST exists; inspect it before staging selected provider authority." >&2
        exit 14
    fi
}

refuse_live_rfb_lifecycle()
{
    local name

    for name in \
        "$SOCKET_NAME" \
        "$ACTIVATED_SERVICE_NAME" \
        "$PERSISTENT_SERVICE_NAME"
    do
        if systemctl is-active --quiet "$name" 2>/dev/null
        then
            echo "ERROR: $name is active; R11 staging is inactive-only." >&2
            exit 20
        fi

        if systemctl is-enabled --quiet "$name" 2>/dev/null
        then
            echo "ERROR: $name is enabled; R11 staging is inactive-only." >&2
            exit 21
        fi
    done

    echo 'RFB_SOCKET_ACTIVE=NO'
    echo 'RFB_SOCKET_ENABLED=NO'
    echo 'RFB_PROVIDER_ACTIVE=NO'
    echo 'RFB_PROVIDER_ENABLED=NO'
    echo 'RFB_PERSISTENT_CONTROL_ACTIVE=NO'
    echo 'RFB_PERSISTENT_CONTROL_ENABLED=NO'
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
        echo "ERROR: expected RFB authority absent: $target" >&2
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
    install -d -m 0755 "$TMP_CANDIDATE/$ACTIVATED_SERVICE_NAME.d"

    cp "$SOCKET_SOURCE" "$TMP_CANDIDATE/$SOCKET_NAME"
    cp "$ACTIVATED_SERVICE_SOURCE" "$TMP_CANDIDATE/$ACTIVATED_SERVICE_NAME"
    cp "$PERSISTENT_SERVICE_SOURCE" "$TMP_CANDIDATE/$PERSISTENT_SERVICE_NAME"
    cp "$DROPIN_SOURCE" "$TMP_CANDIDATE/$DROPIN_RELATIVE"

    SYSTEMD_UNIT_PATH="$TMP_CANDIDATE:" systemd-analyze verify \
        "$SOCKET_NAME" \
        "$ACTIVATED_SERVICE_NAME" \
        "$PERSISTENT_SERVICE_NAME"

    rm -rf "$TMP_CANDIDATE"
    TMP_CANDIDATE=''

    echo 'RFB_COMPOSED_SYSTEMD_STATIC_VERIFY=PASS'
}

stage_candidate()
{
    require_sources
    refuse_live_rfb_lifecycle
    verify_composed_candidate

    assert_safe_target "$SOCKET_SOURCE" "$SOCKET_DEST"
    assert_safe_target "$ACTIVATED_SERVICE_SOURCE" "$ACTIVATED_SERVICE_DEST"
    assert_safe_target "$PERSISTENT_SERVICE_SOURCE" "$PERSISTENT_SERVICE_DEST"
    assert_safe_target "$DROPIN_SOURCE" "$DROPIN_DEST"

    install_file 0644 "$SOCKET_SOURCE" "$SOCKET_DEST"
    install_file 0644 "$ACTIVATED_SERVICE_SOURCE" "$ACTIVATED_SERVICE_DEST"
    install_file 0644 "$PERSISTENT_SERVICE_SOURCE" "$PERSISTENT_SERVICE_DEST"
    install_file 0644 "$DROPIN_SOURCE" "$DROPIN_DEST"

    echo 'SELECTED_RFB_PROVIDER=X0tigervnc_existing_display_0'
    echo 'SELECTED_RFB_PROVIDER_EXECUTABLE=/usr/bin/X0tigervnc'
    echo 'SELECTED_RFB_PROVIDER_DISPLAY=:0'
    echo 'SELECTED_RFB_PROVIDER_XAUTHORITY=/home/ps2/.Xauthority'
    echo 'SELECTED_RFB_PROVIDER_INHERITED_SOCKET=YES'
    echo 'SELECTED_RFB_PROVIDER_SECOND_LISTENER=NO'
    echo 'SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED'
    echo 'RFB_UNIT_ENABLEMENT=NOT_CHANGED'
    echo 'RFB_UNIT_RUNNING_STATE=NOT_CHANGED'
    echo 'LIGHTDM_XORG_STATE=NOT_CHANGED'
    echo 'RFB_NATIVE_PROVIDER_STAGE=PASS'
}

verify_candidate()
{
    require_sources
    refuse_live_rfb_lifecycle

    verify_file 0644 "$SOCKET_SOURCE" "$SOCKET_DEST"
    verify_file 0644 "$ACTIVATED_SERVICE_SOURCE" "$ACTIVATED_SERVICE_DEST"
    verify_file 0644 "$PERSISTENT_SERVICE_SOURCE" "$PERSISTENT_SERVICE_DEST"
    verify_file 0644 "$DROPIN_SOURCE" "$DROPIN_DEST"

    verify_composed_candidate

    echo 'SELECTED_RFB_PROVIDER=X0tigervnc_existing_display_0'
    echo 'SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED'
    echo 'RFB_NATIVE_PROVIDER_VERIFY=PASS'
}

remove_candidate()
{
    local source target

    require_sources
    refuse_live_rfb_lifecycle

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
    done <<__PS2VNC_RFB_REMOVE_EOF__
$DROPIN_SOURCE|$DROPIN_DEST
$PERSISTENT_SERVICE_SOURCE|$PERSISTENT_SERVICE_DEST
$ACTIVATED_SERVICE_SOURCE|$ACTIVATED_SERVICE_DEST
$SOCKET_SOURCE|$SOCKET_DEST
__PS2VNC_RFB_REMOVE_EOF__

    rmdir "$DEST_DIR/$ACTIVATED_SERVICE_NAME.d" 2>/dev/null || true

    echo 'SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED'
    echo 'RFB_UNIT_ENABLEMENT=NOT_CHANGED'
    echo 'RFB_UNIT_RUNNING_STATE=NOT_CHANGED'
    echo 'LIGHTDM_XORG_STATE=NOT_CHANGED'
    echo 'RFB_NATIVE_PROVIDER_REMOVE=PASS'
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
