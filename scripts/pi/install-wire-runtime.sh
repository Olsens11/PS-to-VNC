#!/usr/bin/env bash
# File synopsis:
# Stage, verify, or remove the tracked PS-to-VNC product Wire server runtime and
# its ordinary systemd service definition.
#
# This tool copies only exact tracked bytes. It never reloads the systemd
# manager, changes service enablement, changes service running state, or touches
# the qualified direct-RFB socket/provider definitions. Live activation and
# no-carrier behavior remain separate hardware-qualification work.
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

PROTOCOL_SOURCE="$REPO_ROOT/pi/wire_protocol.py"
RFB_RELAY_SOURCE="$REPO_ROOT/pi/rfb_relay.py"
RFB_ATTACHMENT_SOURCE="$REPO_ROOT/pi/rfb_attachment.py"
SERVER_SOURCE="$REPO_ROOT/pi/wire_server.py"
UNIT_SOURCE="$REPO_ROOT/systemd/pi/ps-to-vnc-wire.service"

PROTOCOL_DEST='/usr/lib/ps-to-vnc/wire_protocol.py'
RFB_RELAY_DEST='/usr/lib/ps-to-vnc/rfb_relay.py'
RFB_ATTACHMENT_DEST='/usr/lib/ps-to-vnc/rfb_attachment.py'
SERVER_DEST='/usr/lib/ps-to-vnc/wire_server.py'
UNIT_DEST='/etc/systemd/system/ps-to-vnc-wire.service'

usage()
{
    echo "Usage: sudo $0 {stage|verify|remove}" >&2
}

[ "$ACTION" = stage ] || [ "$ACTION" = verify ] || [ "$ACTION" = remove ] || {
    usage
    exit 2
}

[ "${EUID}" -eq 0 ] || {
    echo 'ERROR: Wire runtime staging requires root/sudo.' >&2
    exit 3
}

for command_name in     cmp     id     install     python3     rm     rmdir     sha256sum     stat     systemctl     systemd-analyze
do
    command -v "$command_name" >/dev/null 2>&1 || {
        echo "ERROR: required command missing: $command_name" >&2
        exit 4
    }
done

require_sources()
{
    local source

    for source in "$PROTOCOL_SOURCE" "$RFB_RELAY_SOURCE" "$RFB_ATTACHMENT_SOURCE" "$SERVER_SOURCE" "$UNIT_SOURCE"
    do
        [ -f "$source" ] || {
            echo "ERROR: tracked Wire source missing: $source" >&2
            exit 10
        }
    done

    [ -x "$PROTOCOL_SOURCE" ] || {
        echo 'ERROR: tracked Wire protocol source is not executable.' >&2
        exit 11
    }
    [ -x "$SERVER_SOURCE" ] || {
        echo 'ERROR: tracked Wire server source is not executable.' >&2
        exit 12
    }

    id ps2 >/dev/null 2>&1 || {
        echo 'ERROR: expected service user ps2 does not exist.' >&2
        exit 13
    }

    [ -x /usr/bin/python3 ] || {
        echo 'ERROR: /usr/bin/python3 is absent or not executable.' >&2
        exit 14
    }

    python3 - "$PROTOCOL_SOURCE" "$RFB_RELAY_SOURCE" "$RFB_ATTACHMENT_SOURCE" "$SERVER_SOURCE" <<'__PS2VNC_WIRE_SYNTAX_EOF__'
from pathlib import Path
import sys

for name in sys.argv[1:]:
    source = Path(name).read_text(encoding="utf-8")
    compile(source, name, "exec")
print("PI_WIRE_PYTHON_SYNTAX=PASS")
__PS2VNC_WIRE_SYNTAX_EOF__
}

assert_safe_target()
{
    local source="$1"
    local target="$2"

    if [ -e "$target" ] && ! cmp -s "$source" "$target"
    then
        echo "ERROR: refusing to overwrite non-identical target: $target" >&2
        exit 20
    fi
}

normalize_mode()
{
    local mode="$1"

    while [ "${#mode}" -gt 1 ] && [ "${mode#0}" != "$mode" ]
    do
        mode="${mode#0}"
    done

    printf '%s\n' "$mode"
}

install_file()
{
    local mode="$1"
    local source="$2"
    local target="$3"

    install -D -m "$mode" "$source" "$target"

    cmp -s "$source" "$target" || {
        echo "ERROR: staged bytes differ: $target" >&2
        exit 21
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
        echo "ERROR: expected Wire runtime file absent: $target" >&2
        exit 30
    }

    cmp -s "$source" "$target" || {
        echo "ERROR: installed bytes differ from tracked source: $target" >&2
        exit 31
    }

    actual_mode="$(stat -c '%a' "$target")"

    [ "$(normalize_mode "$actual_mode")" = "$(normalize_mode "$expected_mode")" ] || {
        echo "ERROR: wrong mode on $target: actual=$actual_mode expected=$expected_mode" >&2
        exit 32
    }

    echo "INSTALLED_IDENTITY=PASS|$target"
}

refuse_live_wire_lifecycle()
{
    if systemctl is-active --quiet ps-to-vnc-wire.service 2>/dev/null
    then
        echo 'ERROR: ps-to-vnc-wire.service is active.' >&2
        exit 40
    fi

    if systemctl is-enabled --quiet ps-to-vnc-wire.service 2>/dev/null
    then
        echo 'ERROR: ps-to-vnc-wire.service is enabled.' >&2
        exit 41
    fi

    echo 'WIRE_SERVICE_ACTIVE=NO'
    echo 'WIRE_SERVICE_ENABLED=NO'
}

stage_candidate()
{
    require_sources
    refuse_live_wire_lifecycle

    assert_safe_target "$PROTOCOL_SOURCE" "$PROTOCOL_DEST"
    assert_safe_target "$RFB_RELAY_SOURCE" "$RFB_RELAY_DEST"
    assert_safe_target "$RFB_ATTACHMENT_SOURCE" "$RFB_ATTACHMENT_DEST"
    assert_safe_target "$SERVER_SOURCE" "$SERVER_DEST"
    assert_safe_target "$UNIT_SOURCE" "$UNIT_DEST"

    install_file 0755 "$PROTOCOL_SOURCE" "$PROTOCOL_DEST"
    install_file 0644 "$RFB_RELAY_SOURCE" "$RFB_RELAY_DEST"
    install_file 0644 "$RFB_ATTACHMENT_SOURCE" "$RFB_ATTACHMENT_DEST"
    install_file 0755 "$SERVER_SOURCE" "$SERVER_DEST"
    install_file 0644 "$UNIT_SOURCE" "$UNIT_DEST"

    systemd-analyze verify "$UNIT_DEST"
    echo 'SYSTEMD_UNIT_STATIC_VERIFY=PASS'
    echo 'SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED'
    echo 'WIRE_SERVICE_ENABLEMENT=NOT_CHANGED'
    echo 'WIRE_SERVICE_RUNNING_STATE=NOT_CHANGED'
    echo 'RFB_PROVIDER_STATE_CHANGED=NO'
    echo 'PI_WIRE_RUNTIME_STAGE=PASS'
}

verify_candidate()
{
    require_sources
    refuse_live_wire_lifecycle

    verify_file 0755 "$PROTOCOL_SOURCE" "$PROTOCOL_DEST"
    verify_file 0644 "$RFB_RELAY_SOURCE" "$RFB_RELAY_DEST"
    verify_file 0644 "$RFB_ATTACHMENT_SOURCE" "$RFB_ATTACHMENT_DEST"
    verify_file 0755 "$SERVER_SOURCE" "$SERVER_DEST"
    verify_file 0644 "$UNIT_SOURCE" "$UNIT_DEST"

    systemd-analyze verify "$UNIT_DEST"
    echo 'SYSTEMD_UNIT_STATIC_VERIFY=PASS'
    echo 'PI_WIRE_RUNTIME_VERIFY=PASS'
}

remove_candidate()
{
    local source target

    require_sources
    refuse_live_wire_lifecycle

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
    done <<__PS2VNC_WIRE_REMOVE_EOF__
$PROTOCOL_SOURCE|$PROTOCOL_DEST
$RFB_RELAY_SOURCE|$RFB_RELAY_DEST
$RFB_ATTACHMENT_SOURCE|$RFB_ATTACHMENT_DEST
$SERVER_SOURCE|$SERVER_DEST
$UNIT_SOURCE|$UNIT_DEST
__PS2VNC_WIRE_REMOVE_EOF__

    rmdir /usr/lib/ps-to-vnc 2>/dev/null || true

    echo 'SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED'
    echo 'RFB_PROVIDER_STATE_CHANGED=NO'
    echo 'PI_WIRE_RUNTIME_REMOVE=PASS'
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
