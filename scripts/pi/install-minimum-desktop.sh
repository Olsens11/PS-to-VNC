#!/usr/bin/env bash

# File synopsis:
# Stage, verify, or remove the tracked PS-to-VNC Openbox + lxpanel-pi desktop
# layer.
#
# Deliberate lifecycle boundary:
#
#   stage  -> copies exact tracked files only
#   verify -> compares installed bytes and prerequisites
#   remove -> removes only exact matching inactive/disabled candidate files
#
# This tool NEVER:
#   - enables the desktop service;
#   - starts or stops the desktop service;
#   - starts or stops the RFB provider;
#   - calls systemctl daemon-reload;
#   - installs or removes packages.
#
# Live activation is a separate hardware-qualification transaction.

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

PROVENANCE_SOURCE="$REPO_ROOT/config/pi/desktop/PANEL_PROVENANCE.env"

OPENBOX_RC_SOURCE="$REPO_ROOT/config/pi/desktop/openbox/rc.xml"
OPENBOX_MENU_SOURCE="$REPO_ROOT/config/pi/desktop/openbox/menu.xml"

PANEL_DEFAULT_SOURCE="$REPO_ROOT/config/pi/desktop/xdg/lxpanel-pi/default"
PANEL_PROFILE_SOURCE="$REPO_ROOT/config/pi/desktop/xdg/lxpanel-pi/panels/panel"

SESSION_SOURCE="$REPO_ROOT/scripts/pi/run-desktop-session.sh"
UNIT_SOURCE="$REPO_ROOT/systemd/pi/ps-to-vnc-desktop.service"

PROVENANCE_DEST='/etc/ps-to-vnc/desktop/PANEL_PROVENANCE.env'

OPENBOX_RC_DEST='/etc/ps-to-vnc/desktop/openbox/rc.xml'
OPENBOX_MENU_DEST='/etc/ps-to-vnc/desktop/openbox/menu.xml'

PANEL_DEFAULT_DEST='/etc/ps-to-vnc/desktop/xdg/lxpanel-pi/default'
PANEL_PROFILE_DEST='/etc/ps-to-vnc/desktop/xdg/lxpanel-pi/panels/panel'

SESSION_DEST='/usr/lib/ps-to-vnc/run-desktop-session.sh'
UNIT_DEST='/etc/systemd/system/ps-to-vnc-desktop.service'

QUALIFIED_SOCKET_SOURCE="$REPO_ROOT/systemd/pi/ps-to-vnc-rfb.socket"
QUALIFIED_PROVIDER_SOURCE="$REPO_ROOT/systemd/pi/ps-to-vnc-rfb-tigervnc.service"
QUALIFIED_PERSISTENT_SOURCE="$REPO_ROOT/systemd/pi/ps-to-vnc-rfb-tigervnc-persistent.service"

QUALIFIED_SOCKET_DEST='/etc/systemd/system/ps-to-vnc-rfb.socket'
QUALIFIED_PROVIDER_DEST='/etc/systemd/system/ps-to-vnc-rfb-tigervnc.service'
QUALIFIED_PERSISTENT_DEST='/etc/systemd/system/ps-to-vnc-rfb-tigervnc-persistent.service'

usage()
{
    echo "Usage: sudo $0 {stage|verify|remove}" >&2
}

[ "$ACTION" = stage ] || \
[ "$ACTION" = verify ] || \
[ "$ACTION" = remove ] || {
    usage
    exit 2
}

[ "${EUID}" -eq 0 ] || {
    echo 'ERROR: installer actions require root/sudo.' >&2
    exit 3
}

for command_name in \
    cmp \
    dpkg-query \
    id \
    install \
    python3 \
    readlink \
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


require_source_tree()
{
    local path

    for path in \
        "$PROVENANCE_SOURCE" \
        "$OPENBOX_RC_SOURCE" \
        "$OPENBOX_MENU_SOURCE" \
        "$PANEL_DEFAULT_SOURCE" \
        "$PANEL_PROFILE_SOURCE" \
        "$SESSION_SOURCE" \
        "$UNIT_SOURCE"
    do
        [ -f "$path" ] || {
            echo "ERROR: tracked source missing: $path" >&2
            exit 10
        }
    done

    [ -x "$SESSION_SOURCE" ] || {
        echo 'ERROR: tracked session supervisor is not executable.' >&2
        exit 11
    }
}


require_runtime_dependencies()
{
    local path

    id ps2 >/dev/null 2>&1 || {
        echo 'ERROR: expected runtime user ps2 does not exist.' >&2
        exit 12
    }

    for path in \
        /usr/bin/dbus-run-session \
        /usr/bin/lxpanel-pi \
        /usr/bin/lxterminal \
        /usr/bin/openbox \
        /usr/bin/xprop
    do
        [ -x "$path" ] || {
            echo "ERROR: required runtime executable absent: $path" >&2
            exit 13
        }
    done
}


load_and_verify_provenance()
{
    # The provenance file contains only project-controlled NAME=value records.
    # shellcheck disable=SC1090
    . "$PROVENANCE_SOURCE"

    : "${LXPANEL_PI_PACKAGE_VERSION:?missing lxpanel-pi provenance}"
    : "${RPD_X_CORE_PACKAGE_VERSION:?missing rpd-x-core provenance}"
    : "${SOURCE_PANEL_DEFAULT_SHA256:?missing panel-default provenance}"
    : "${SOURCE_PANEL_PROFILE_SHA256:?missing panel-profile provenance}"

    local installed_lxpanel
    local installed_rpd
    local source_default_sha
    local source_panel_sha

    installed_lxpanel="$(
        dpkg-query \
            -W \
            -f='${Version}' \
            lxpanel-pi \
            2>/dev/null
    )" || {
        echo 'ERROR: lxpanel-pi is not installed.' >&2
        exit 14
    }

    installed_rpd="$(
        dpkg-query \
            -W \
            -f='${Version}' \
            rpd-x-core \
            2>/dev/null
    )" || {
        echo 'ERROR: rpd-x-core is not installed.' >&2
        exit 15
    }

    echo "EXPECTED_LXPANEL_PI_VERSION=$LXPANEL_PI_PACKAGE_VERSION"
    echo "INSTALLED_LXPANEL_PI_VERSION=$installed_lxpanel"

    echo "EXPECTED_RPD_X_CORE_VERSION=$RPD_X_CORE_PACKAGE_VERSION"
    echo "INSTALLED_RPD_X_CORE_VERSION=$installed_rpd"

    [ "$installed_lxpanel" = "$LXPANEL_PI_PACKAGE_VERSION" ] || {
        echo 'ERROR: lxpanel-pi package version differs from evaluated authority.' >&2
        exit 16
    }

    [ "$installed_rpd" = "$RPD_X_CORE_PACKAGE_VERSION" ] || {
        echo 'ERROR: rpd-x-core package version differs from evaluated authority.' >&2
        exit 17
    }

    source_default_sha="$(
        sha256sum "$PANEL_DEFAULT_SOURCE" |
        awk '{print $1}'
    )"

    source_panel_sha="$(
        sha256sum "$PANEL_PROFILE_SOURCE" |
        awk '{print $1}'
    )"

    [ "$source_default_sha" = "$SOURCE_PANEL_DEFAULT_SHA256" ] || {
        echo 'ERROR: tracked panel-default bytes differ from recorded provenance.' >&2
        exit 18
    }

    [ "$source_panel_sha" = "$SOURCE_PANEL_PROFILE_SHA256" ] || {
        echo 'ERROR: tracked panel-profile bytes differ from recorded provenance.' >&2
        exit 19
    }

    echo 'PANEL_PROVENANCE=PASS'
}


require_qualified_issue5_foundation()
{
    local source
    local target

    while IFS='|' read -r source target
    do
        [ -f "$source" ] || {
            echo "ERROR: qualified tracked source absent: $source" >&2
            exit 20
        }

        [ -f "$target" ] || {
            echo "ERROR: qualified installed unit absent: $target" >&2
            exit 21
        }

        cmp -s "$source" "$target" || {
            echo "ERROR: installed Issue5 unit differs from tracked authority: $target" >&2
            exit 22
        }

        echo "QUALIFIED_ISSUE5_IDENTITY=PASS|$target"
    done <<__PS2VNC_ISSUE5_IDENTITY_EOF__
$QUALIFIED_SOCKET_SOURCE|$QUALIFIED_SOCKET_DEST
$QUALIFIED_PROVIDER_SOURCE|$QUALIFIED_PROVIDER_DEST
$QUALIFIED_PERSISTENT_SOURCE|$QUALIFIED_PERSISTENT_DEST
__PS2VNC_ISSUE5_IDENTITY_EOF__
}


refuse_live_desktop_lifecycle()
{
    if systemctl \
           is-active \
           --quiet \
           ps-to-vnc-desktop.service \
           2>/dev/null
    then
        echo 'ERROR: ps-to-vnc-desktop.service is active.' >&2
        exit 23
    fi

    if systemctl \
           is-enabled \
           --quiet \
           ps-to-vnc-desktop.service \
           2>/dev/null
    then
        echo 'ERROR: ps-to-vnc-desktop.service is enabled.' >&2
        exit 24
    fi

    echo 'DESKTOP_SERVICE_ACTIVE=NO'
    echo 'DESKTOP_SERVICE_ENABLED=NO'
}


assert_safe_target()
{
    local source="$1"
    local target="$2"

    if [ -e "$target" ] && \
       ! cmp -s "$source" "$target"
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

    install \
        -D \
        -m "$mode" \
        "$source" \
        "$target"

    cmp -s "$source" "$target" || {
        echo "ERROR: installed bytes differ: $target" >&2
        exit 31
    }

    echo "INSTALLED=$target"
    echo "SHA256=$(sha256sum "$target" | awk '{print $1}')"
    echo "MODE=$(stat -c '%a' "$target")"
}


verify_file()
{
    local expected_mode="$1"
    local source="$2"
    local target="$3"

    [ -f "$target" ] || {
        echo "ERROR: expected installed file absent: $target" >&2
        exit 40
    }

    cmp -s "$source" "$target" || {
        echo "ERROR: installed bytes differ from tracked source: $target" >&2
        exit 41
    }

    local actual_mode

    actual_mode="$(
        stat \
            -c '%a' \
            "$target"
    )"

    [ "$actual_mode" = "$expected_mode" ] || {
        echo "ERROR: wrong installed mode on $target: $actual_mode" >&2
        exit 42
    }

    echo "INSTALLED_IDENTITY=PASS|$target"
    echo "INSTALLED_SHA256=$(sha256sum "$target" | awk '{print $1}')"
}


stage_candidate()
{
    require_source_tree
    require_runtime_dependencies
    load_and_verify_provenance
    require_qualified_issue5_foundation
    refuse_live_desktop_lifecycle

    while IFS='|' read -r source target
    do
        assert_safe_target "$source" "$target"
    done <<__PS2VNC_STAGE_TARGETS_EOF__
$PROVENANCE_SOURCE|$PROVENANCE_DEST
$OPENBOX_RC_SOURCE|$OPENBOX_RC_DEST
$OPENBOX_MENU_SOURCE|$OPENBOX_MENU_DEST
$PANEL_DEFAULT_SOURCE|$PANEL_DEFAULT_DEST
$PANEL_PROFILE_SOURCE|$PANEL_PROFILE_DEST
$SESSION_SOURCE|$SESSION_DEST
$UNIT_SOURCE|$UNIT_DEST
__PS2VNC_STAGE_TARGETS_EOF__

    install_file 0644 "$PROVENANCE_SOURCE" "$PROVENANCE_DEST"

    install_file 0644 "$OPENBOX_RC_SOURCE" "$OPENBOX_RC_DEST"
    install_file 0644 "$OPENBOX_MENU_SOURCE" "$OPENBOX_MENU_DEST"

    install_file 0644 "$PANEL_DEFAULT_SOURCE" "$PANEL_DEFAULT_DEST"
    install_file 0644 "$PANEL_PROFILE_SOURCE" "$PANEL_PROFILE_DEST"

    install_file 0755 "$SESSION_SOURCE" "$SESSION_DEST"

    install_file 0644 "$UNIT_SOURCE" "$UNIT_DEST"

    # Validate the fully staged unit while its ExecStart target now exists.
    systemd-analyze \
        verify \
        "$UNIT_DEST"

    echo 'SYSTEMD_UNIT_STATIC_VERIFY=PASS'

    echo 'SYSTEMCTL_DAEMON_RELOAD=NOT_PERFORMED'
    echo 'SYSTEMCTL_ENABLE=NOT_PERFORMED'
    echo 'SYSTEMCTL_START=NOT_PERFORMED'
    echo 'RFB_PROVIDER_STATE_CHANGED=NO'
    echo 'DESKTOP_CANDIDATE_STAGE=PASS'
}


verify_candidate()
{
    require_source_tree
    require_runtime_dependencies
    load_and_verify_provenance
    require_qualified_issue5_foundation
    refuse_live_desktop_lifecycle

    verify_file 0644 "$PROVENANCE_SOURCE" "$PROVENANCE_DEST"

    verify_file 0644 "$OPENBOX_RC_SOURCE" "$OPENBOX_RC_DEST"
    verify_file 0644 "$OPENBOX_MENU_SOURCE" "$OPENBOX_MENU_DEST"

    verify_file 0644 "$PANEL_DEFAULT_SOURCE" "$PANEL_DEFAULT_DEST"
    verify_file 0644 "$PANEL_PROFILE_SOURCE" "$PANEL_PROFILE_DEST"

    verify_file 0755 "$SESSION_SOURCE" "$SESSION_DEST"

    verify_file 0644 "$UNIT_SOURCE" "$UNIT_DEST"

    systemd-analyze \
        verify \
        "$UNIT_DEST"

    echo 'SYSTEMD_UNIT_STATIC_VERIFY=PASS'

    echo 'SYSTEMCTL_DAEMON_RELOAD=NOT_REQUIRED_FOR_BYTE_VERIFY'
    echo 'DESKTOP_CANDIDATE_VERIFY=PASS'
}


remove_candidate()
{
    require_source_tree
    refuse_live_desktop_lifecycle

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
    done <<__PS2VNC_REMOVE_TARGETS_EOF__
$PROVENANCE_SOURCE|$PROVENANCE_DEST
$OPENBOX_RC_SOURCE|$OPENBOX_RC_DEST
$OPENBOX_MENU_SOURCE|$OPENBOX_MENU_DEST
$PANEL_DEFAULT_SOURCE|$PANEL_DEFAULT_DEST
$PANEL_PROFILE_SOURCE|$PANEL_PROFILE_DEST
$SESSION_SOURCE|$SESSION_DEST
$UNIT_SOURCE|$UNIT_DEST
__PS2VNC_REMOVE_TARGETS_EOF__

    # Remove only empty project-owned directories.
    rmdir \
        /etc/ps-to-vnc/desktop/xdg/lxpanel-pi/panels \
        /etc/ps-to-vnc/desktop/xdg/lxpanel-pi \
        /etc/ps-to-vnc/desktop/xdg \
        /etc/ps-to-vnc/desktop/openbox \
        /etc/ps-to-vnc/desktop \
        /usr/lib/ps-to-vnc \
        2>/dev/null \
        || true

    echo 'SYSTEMCTL_DAEMON_RELOAD=NOT_PERFORMED'
    echo 'DESKTOP_CANDIDATE_REMOVE=PASS'
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
