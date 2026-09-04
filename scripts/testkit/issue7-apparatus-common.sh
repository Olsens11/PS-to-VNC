#!/usr/bin/env bash

# File synopsis:
# Shared fail-closed contracts for the successor Issue #7 hardware apparatus.
#
# These helpers own Issue #7 observer/capture constants, current-manifest
# validation, and observer ownership checks. Generic ELF deployment is a
# separate TestKit responsibility.
#
# They do not implement product recovery or historical M4 freeze/sampler
# behavior.

ISSUE7_ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/../.." &&
    pwd -P
)"

ISSUE7_EXPECTED_BRANCH='reconstruct/issue7-minimal-core'
ISSUE7_EXPECTED_REPOSITORY='Olsens11/PS-to-VNC'
ISSUE7_EXPECTED_PS2IP_SHA256='b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

ISSUE7_PS2_HOST='192.168.50.2'
ISSUE7_PI_PRIVATE_IP='192.168.50.1'
ISSUE7_UDP_PORT='5999'
ISSUE7_PCAP_INTERFACE='eth0'
ISSUE7_RFB_PORT='5900'
ISSUE7_CAPTURE_MAX_SECONDS='1800'

ISSUE7_STATE_HOME="${XDG_STATE_HOME:-$HOME/.local/state}"
ISSUE7_STATE_ROOT="$ISSUE7_STATE_HOME/ps-to-vnc/issue7-hardware"
ISSUE7_RUN_ROOT="$ISSUE7_STATE_ROOT/runs"
ISSUE7_ACTIVE_CONTROL="$ISSUE7_STATE_ROOT/active.env"

issue7_die()
{
    printf 'ISSUE7_APPARATUS_ERROR=%s\n' "$*" >&2
    exit 1
}

issue7_sha256()
{
    local file="$1"

    sha256sum "$file" |
        awk '{print $1}'
}

issue7_manifest_value()
{
    local key="$1"
    local manifest="$2"
    local value

    value="$(
        awk -F= -v wanted="$key" '
            $1 == wanted {
                count++
                sub(/^[^=]*=/, "")
                value = $0
            }
            END {
                if (count != 1 || value == "")
                    exit 1
                print value
            }
        ' "$manifest"
    )" ||
        issue7_die "expected exactly one non-empty $key in $manifest"

    printf '%s\n' "$value"
}

issue7_validate_manifest_current()
{
    local manifest="$1"
    local version
    local scope
    local qualified
    local repository
    local branch
    local ps2ip_sha

    [ -f "$manifest" ] ||
        issue7_die "manifest not found: $manifest"

    version="$(issue7_manifest_value ISSUE7_DUT_MANIFEST_VERSION "$manifest")"
    scope="$(issue7_manifest_value QUALIFICATION_SCOPE "$manifest")"
    qualified="$(issue7_manifest_value HARDWARE_QUALIFIED "$manifest")"
    repository="$(issue7_manifest_value SOURCE_REPOSITORY "$manifest")"
    branch="$(issue7_manifest_value SOURCE_BRANCH "$manifest")"

    ISSUE7_SOURCE_COMMIT="$(
        issue7_manifest_value SOURCE_COMMIT "$manifest"
    )"

    ps2ip_sha="$(
        issue7_manifest_value PS2IP_SHA256 "$manifest"
    )"

    ISSUE7_TEST_ID="$(
        issue7_manifest_value HARDWARE_TEST_ID "$manifest"
    )"

    ISSUE7_IDENTITY_SHA256="$(
        issue7_manifest_value HARDWARE_ELF_IDENTITY_SHA256 "$manifest"
    )"

    ISSUE7_STAMPED_ELF_SHA256="$(
        issue7_manifest_value HARDWARE_STAMPED_ELF_SHA256 "$manifest"
    )"

    [ "$version" = '1' ] ||
        issue7_die "unsupported DUT manifest version: $version"

    [ "$scope" = 'PRE_HARDWARE_IDENTITY_ONLY' ] ||
        issue7_die "unexpected DUT manifest qualification scope: $scope"

    [ "$qualified" = 'NO' ] ||
        issue7_die 'input DUT manifest already claims hardware qualification'

    [ "$repository" = "$ISSUE7_EXPECTED_REPOSITORY" ] ||
        issue7_die "source repository mismatch: $repository"

    [ "$branch" = "$ISSUE7_EXPECTED_BRANCH" ] ||
        issue7_die "source branch mismatch: $branch"

    [ "$ps2ip_sha" = "$ISSUE7_EXPECTED_PS2IP_SHA256" ] ||
        issue7_die "qualified PS2IP identity mismatch"

    printf '%s\n' "$ISSUE7_SOURCE_COMMIT" |
        grep -Eq '^[0-9a-f]{40}$' ||
        issue7_die 'SOURCE_COMMIT is malformed'

    printf '%s\n' "$ISSUE7_IDENTITY_SHA256" |
        grep -Eq '^[0-9a-f]{64}$' ||
        issue7_die 'runtime identity SHA256 is malformed'

    printf '%s\n' "$ISSUE7_STAMPED_ELF_SHA256" |
        grep -Eq '^[0-9a-f]{64}$' ||
        issue7_die 'stamped ELF SHA256 is malformed'

    printf '%s\n' "$ISSUE7_TEST_ID" |
        grep -Eq '^[A-Za-z0-9._-]{1,63}$' ||
        issue7_die 'hardware test ID is unsafe'

    [ "$(git -C "$ISSUE7_ROOT" branch --show-current)" = \
      "$ISSUE7_EXPECTED_BRANCH" ] ||
        issue7_die 'current checkout is not the Issue #7 branch'

    [ "$(git -C "$ISSUE7_ROOT" rev-parse HEAD)" = \
      "$ISSUE7_SOURCE_COMMIT" ] ||
        issue7_die 'current checkout does not match manifest SOURCE_COMMIT'

    [ -z "$(
        git -C "$ISSUE7_ROOT" status \
            --porcelain \
            --untracked-files=no
    )" ] ||
        issue7_die 'tracked source tree is not clean'

    while read -r hash_key relative_path; do
        expected_hash="$(
            issue7_manifest_value "$hash_key" "$manifest"
        )"

        actual_hash="$(
            issue7_sha256 "$ISSUE7_ROOT/$relative_path"
        )"

        [ "$actual_hash" = "$expected_hash" ] ||
            issue7_die \
                "apparatus hash mismatch: $relative_path"
    done <<'APPARATUS_HASHES'
SCRIPTS_TESTKIT_VERIFY_ELF_IDENTITY_SH_SHA256 scripts/testkit/verify-elf-identity.sh
SCRIPTS_TESTKIT_ISSUE7_DUT_MANIFEST_PY_SHA256 scripts/testkit/issue7-dut-manifest.py
SCRIPTS_TESTKIT_ISSUE7_APPARATUS_COMMON_SH_SHA256 scripts/testkit/issue7-apparatus-common.sh
SCRIPTS_TESTKIT_ISSUE7_ARM_OBSERVERS_SH_SHA256 scripts/testkit/issue7-arm-observers.sh
SCRIPTS_TESTKIT_ISSUE7_STOP_OBSERVERS_SH_SHA256 scripts/testkit/issue7-stop-observers.sh
SCRIPTS_TESTKIT_ISSUE7_UDP_OBSERVER_PY_SHA256 scripts/testkit/issue7-udp-observer.py
SCRIPTS_TESTKIT_ISSUE7_RESULT_PY_SHA256 scripts/testkit/issue7-result.py
SCRIPTS_TESTKIT_ISSUE7_APPARATUS_SELF_TEST_SH_SHA256 scripts/testkit/issue7-apparatus-self-test.sh
APPARATUS_HASHES
}

issue7_run_abs()
{
    local raw="$1"
    local abs
    local run_root

    [ -d "$raw" ] ||
        issue7_die "run directory not found: $raw"

    abs="$(realpath -e "$raw")"
    run_root="$(realpath -e "$ISSUE7_RUN_ROOT")"

    case "$abs/" in
        "$run_root"/*/)
            ;;
        *)
            issue7_die "run is outside Issue #7 state root: $abs"
            ;;
    esac

    printf '%s\n' "$abs"
}

issue7_load_control()
{
    local run="$1"
    local control="$run/control.env"

    [ -f "$control" ] ||
        issue7_die "run control missing: $control"

    # This file is generated only by issue7-arm-observers.sh inside the
    # successor-owned state root and is therefore trusted apparatus state.
    # shellcheck disable=SC1090
    source "$control"

    [ "${CONTROL_VERSION:-}" = '1' ] ||
        issue7_die 'unsupported run-control version'

    [ "${RUN:-}" = "$run" ] ||
        issue7_die 'run control does not own requested run'
}

issue7_pid_alive()
{
    local pid="$1"

    [ -n "$pid" ] &&
        kill -0 "$pid" 2>/dev/null
}

issue7_pid_owned()
{
    local pid="$1"
    local run="$2"
    local cmd

    issue7_pid_alive "$pid" ||
        return 1

    cmd="$(
        tr '\0' ' ' < "/proc/$pid/cmdline" 2>/dev/null ||
        true
    )"

    printf '%s\n' "$cmd" |
        grep -Fq "$ISSUE7_ROOT/scripts/testkit/issue7-udp-observer.py" &&
    printf '%s\n' "$cmd" |
        grep -Fq -- '--owner-token' &&
    printf '%s\n' "$cmd" |
        grep -Fq "$run"
}

issue7_pcap_active()
{
    local unit="$1"

    [ -n "$unit" ] &&
        systemctl is-active --quiet "$unit" 2>/dev/null
}

issue7_pcap_owned()
{
    local unit="$1"
    local pcap="$2"
    local exec_start

    case "$unit" in
        ps2vnc-i7-*.service)
            ;;
        *)
            return 1
            ;;
    esac

    exec_start="$(
        systemctl show \
            --property=ExecStart \
            --value \
            "$unit" 2>/dev/null ||
        true
    )"

    printf '%s\n' "$exec_start" |
        grep -Fq '/usr/bin/tcpdump' &&
    printf '%s\n' "$exec_start" |
        grep -Fq "$pcap"
}
