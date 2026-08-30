#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd -P
)"

STATE="$ROOT/runtime/MIGRATION_STATE.env"

cd "$ROOT"

test -f "$STATE"

# shellcheck disable=SC1090
source "$STATE"

echo '===== PS-to-VNC MIGRATION COHERENCE CHECK ====='

echo
echo '===== A. REPOSITORY IDENTITY ====='

test "$PROJECT_NAME" = 'PS-to-VNC'
test "$PROJECT_ROOT" = '/home/ps2/PS-to-VNC'
test "$ROOT" = "$PROJECT_ROOT"

test "$LEGACY_ROOT" = '/home/ps2/ps2vnc'
test "$LEGACY_MUTABLE" = 'NO'
test "$DO_NOT_TOUCH" = "$LEGACY_ROOT"

test -d "$ROOT/.git"
test -d "$LEGACY_ROOT/.git"

NEW_GIT="$(
    readlink -f "$ROOT/.git"
)"

LEGACY_GIT="$(
    readlink -f "$LEGACY_ROOT/.git"
)"

test "$NEW_GIT" != "$LEGACY_GIT"

echo "PROJECT_ROOT=$PROJECT_ROOT"
echo "NEW_GIT_DIR=$NEW_GIT"
echo "LEGACY_GIT_DIR=$LEGACY_GIT"
echo 'REPOSITORY_IDENTITY=PASS'

echo
echo '===== B. MIGRATION STATE ====='

case "$CURRENT_STAGE_STATUS" in
    NOT_STARTED|IN_PROGRESS|VALIDATING|BLOCKED|COMPLETE)
        ;;
    *)
        echo "ERROR=INVALID_CURRENT_STAGE_STATUS:$CURRENT_STAGE_STATUS"
        exit 20
        ;;
esac

test -n "$LAST_COMPLETE_STAGE"
test -n "$CURRENT_STAGE"
test -n "$NEXT_ACTION"
test -n "$BLOCKED_BY"

echo "LAST_COMPLETE_STAGE=$LAST_COMPLETE_STAGE"
echo "CURRENT_STAGE=$CURRENT_STAGE"
echo "CURRENT_STAGE_STATUS=$CURRENT_STAGE_STATUS"
echo "NEXT_ACTION=$NEXT_ACTION"
echo "BLOCKED_BY=$BLOCKED_BY"
echo 'MACHINE_MIGRATION_STATE=PASS'

echo
echo '===== C. FROZEN B4A AUTHORITY ====='

test "$BASELINE_DUT" = 'D17AL-F8J2-B4A'
test -f "$BASELINE_SOURCE"

ACTUAL_SOURCE_SHA="$(
    sha256sum "$BASELINE_SOURCE" |
    awk '{print $1}'
)"

ACTUAL_SOURCE_LINES="$(
    wc -l < "$BASELINE_SOURCE"
)"

echo "BASELINE_SOURCE_SHA_EXPECTED=$BASELINE_SOURCE_SHA256"
echo "BASELINE_SOURCE_SHA_ACTUAL=$ACTUAL_SOURCE_SHA"
echo "BASELINE_SOURCE_LINES_EXPECTED=$BASELINE_SOURCE_LINES"
echo "BASELINE_SOURCE_LINES_ACTUAL=$ACTUAL_SOURCE_LINES"

test "$ACTUAL_SOURCE_SHA" = "$BASELINE_SOURCE_SHA256"
test "$ACTUAL_SOURCE_LINES" -eq "$BASELINE_SOURCE_LINES"

test -f baseline/frozen-b4a/PS2VNC.ELF

ACTUAL_ELF_SHA="$(
    sha256sum baseline/frozen-b4a/PS2VNC.ELF |
    awk '{print $1}'
)"

echo "BASELINE_ELF_SHA_EXPECTED=$BASELINE_ELF_SHA256"
echo "BASELINE_ELF_SHA_ACTUAL=$ACTUAL_ELF_SHA"

test "$ACTUAL_ELF_SHA" = "$BASELINE_ELF_SHA256"

echo 'FROZEN_B4A_AUTHORITY=PASS'

echo
echo '===== D. DEPENDENCY MAP AUTHORITY ====='

test -d "$DEPENDENCY_MAP"
test -f "$DEPENDENCY_MAP/SHA256SUMS.txt"

ACTUAL_DEPENDENCY_SHA="$(
    sha256sum "$DEPENDENCY_MAP/SHA256SUMS.txt" |
    awk '{print $1}'
)"

echo "DEPENDENCY_MAP_SHA_EXPECTED=$DEPENDENCY_MAP_SHA256SUMS_SHA256"
echo "DEPENDENCY_MAP_SHA_ACTUAL=$ACTUAL_DEPENDENCY_SHA"

test "$ACTUAL_DEPENDENCY_SHA" = "$DEPENDENCY_MAP_SHA256SUMS_SHA256"

echo 'DEPENDENCY_MAP_AUTHORITY=PASS'

echo
echo '===== E. BOOTSTRAP PRESERVATION ====='

test -n "$PRE_RECOVERY_INDEX_TREE"
test -n "$PRE_RECOVERY_ARCHIVE"
test -n "$PRE_RECOVERY_ARCHIVE_SHA256"

git cat-file -e "$PRE_RECOVERY_INDEX_TREE^{tree}"

test -f "$PRE_RECOVERY_ARCHIVE"

ACTUAL_ARCHIVE_SHA="$(
    sha256sum "$PRE_RECOVERY_ARCHIVE" |
    awk '{print $1}'
)"

echo "PRE_RECOVERY_INDEX_TREE=$PRE_RECOVERY_INDEX_TREE"
echo "PRE_RECOVERY_ARCHIVE=$PRE_RECOVERY_ARCHIVE"
echo "PRE_RECOVERY_ARCHIVE_SHA_EXPECTED=$PRE_RECOVERY_ARCHIVE_SHA256"
echo "PRE_RECOVERY_ARCHIVE_SHA_ACTUAL=$ACTUAL_ARCHIVE_SHA"

test "$ACTUAL_ARCHIVE_SHA" = "$PRE_RECOVERY_ARCHIVE_SHA256"

echo 'BOOTSTRAP_PRESERVATION=PASS'

echo
echo '===== F. HUMAN / MACHINE DOCUMENTATION COHERENCE ====='

for FILE in \
    START_HERE.md \
    docs/INDEX.md \
    docs/PROJECT_STATE.md \
    docs/MIGRATION_STATE.md \
    docs/MODULARIZATION.md \
    docs/reference/FILE_AND_SERVICE_MAP.md \
    scripts/resume-state.sh \
    scripts/migration-check.sh
do
    test -f "$FILE"
done

grep -Fq "$LAST_COMPLETE_STAGE" docs/MIGRATION_STATE.md
grep -Fq "$CURRENT_STAGE" docs/MIGRATION_STATE.md
grep -Fq "$CURRENT_STAGE_STATUS" docs/MIGRATION_STATE.md

grep -Fq "$CURRENT_STAGE" docs/PROJECT_STATE.md

grep -Fq 'runtime/MIGRATION_STATE.env' START_HERE.md
grep -Fq 'docs/MIGRATION_STATE.md' START_HERE.md
grep -Fq 'scripts/resume-state.sh' START_HERE.md
grep -Fq 'scripts/migration-check.sh' START_HERE.md

echo 'HUMAN_MACHINE_DOCUMENTATION_COHERENCE=PASS'

echo
echo '===== G. GIT AUTHORITY ====='

echo "GIT_BRANCH=$(git branch --show-current)"
test "$(git branch --show-current)" = 'main'

if git rev-parse --verify HEAD >/dev/null 2>&1; then
    HEAD_NOW="$(
        git rev-parse HEAD
    )"

    echo "GIT_HEAD=$HEAD_NOW"

    test "$CURRENT_DOC_HEAD" != 'PENDING_INITIAL_BOOTSTRAP_COMMIT'

    git cat-file -e "$CURRENT_DOC_HEAD^{commit}"

    if [ "$CURRENT_SOURCE_HEAD" != 'NONE' ]; then
        git cat-file -e "$CURRENT_SOURCE_HEAD^{commit}"
    fi
else
    echo 'GIT_HEAD=UNBORN'

    test "$CURRENT_DOC_HEAD" = 'PENDING_INITIAL_BOOTSTRAP_COMMIT'
    test "$CURRENT_SOURCE_HEAD" = 'NONE'
fi

echo "GIT_REMOTE_COUNT=$(git remote | wc -l)"

echo 'GIT_AUTHORITY=PASS'

echo
echo '===== H. LEGACY SAFETY BOUNDARY ====='

LEGACY_HEAD_NOW="$(
    git -C "$LEGACY_ROOT" rev-parse HEAD
)"

echo "LEGACY_HEAD_EXPECTED=$LEGACY_SEPARATION_HEAD"
echo "LEGACY_HEAD_ACTUAL=$LEGACY_HEAD_NOW"

test "$LEGACY_HEAD_NOW" = "$LEGACY_SEPARATION_HEAD"

git -C "$LEGACY_ROOT" diff --quiet --
git -C "$LEGACY_ROOT" diff --cached --quiet --

echo 'LEGACY_TRACKED_STATE_UNCHANGED=PASS'

echo
echo '===== I. CURRENT WORKING AUTHORITY ====='

if [ "$CURRENT_WORKING_SOURCE" = 'NONE' ]; then
    test "$CURRENT_STAGE" = 'M0'

    case "$CURRENT_STAGE_STATUS" in
        NOT_STARTED|IN_PROGRESS)
            ;;
        *)
            echo "ERROR=NO_WORKING_SOURCE_INVALID_STAGE_STATUS:$CURRENT_STAGE_STATUS"
            exit 70
            ;;
    esac
fi

echo "CURRENT_WORKING_SOURCE=$CURRENT_WORKING_SOURCE"
echo "CURRENT_SOURCE_HEAD=$CURRENT_SOURCE_HEAD"
echo "CURRENT_DOC_HEAD=$CURRENT_DOC_HEAD"

echo 'CURRENT_WORKING_AUTHORITY=PASS'

echo
echo '===== J. M0 BUILD AUTHORITY ====='

if [ "$CURRENT_STAGE" = 'M0' ] &&
   [ "$CURRENT_STAGE_STATUS" = 'IN_PROGRESS' ] &&
   [ "$NEXT_ACTION" != 'M0C_reconstruct_and_fingerprint_historical_build_environment' ]
then
    test -f runtime/M0_BUILD_AUTHORITY.env
fi

if [ -f runtime/M0_BUILD_AUTHORITY.env ]; then
    # shellcheck disable=SC1091
    source runtime/M0_BUILD_AUTHORITY.env

    test "$M0_REFERENCE_DUT" = 'D17AL-F8J2-B4A'

    test "$M0_SOURCE_SHA256" = "$BASELINE_SOURCE_SHA256"
    test "$M0_REFERENCE_ELF_SHA256" = "$BASELINE_ELF_SHA256"

    test "$M0_B4A_LIBPS2IP_SHA256" = \
        'b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

    ACTUAL_M0_LIB_SHA="$(
        sha256sum baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a |
        awk '{print $1}'
    )"

    test "$ACTUAL_M0_LIB_SHA" = "$M0_B4A_LIBPS2IP_SHA256"

    test "$M0_B4A_LIBPS2IP_AUTHORITY" = \
        'FROZEN_MATCHES_HISTORICAL_B4A_WORKTREE'

    test "$M0_STALE_LEGACY_ARCHIVE_STATUS" = \
        'NOT_B4A_LINK_INPUT'

    test "$M0_REPRODUCTION_TARGET" = 'BYTE_EXACT'

    echo "M0_REFERENCE_DUT=$M0_REFERENCE_DUT"
    echo "M0_B4A_LIBPS2IP_SHA256=$M0_B4A_LIBPS2IP_SHA256"
    echo "M0_REFERENCE_ELF_SHA256=$M0_REFERENCE_ELF_SHA256"
    echo "M0_REPRODUCTION_CLASSIFICATION=$M0_REPRODUCTION_CLASSIFICATION"
    if [ "$M0_REPRODUCTION_CLASSIFICATION" = 'OUTCOME_A_BYTE_EXACT_ELF' ]; then
        test "$M0D2_OBSERVED_ELF_SHA256" = "$M0_REFERENCE_ELF_SHA256"
        test "$M0D2_OBSERVED_ELF_BYTES" = "$M0_REFERENCE_ELF_BYTES"

        test "$M0D2_GSHIRES_LTO_PAYLOADS" = 'BYTE_IDENTICAL'
        test "$M0D2_GSHIRES_VARIANCE" = \
            'SLIM_LTO_SECTION_NAME_METADATA_ONLY'
        test "$M0D2_GSHIRES_FINAL_ELF_EFFECT" = 'NONE'

        test "$M0_FINAL_ELF_IDENTITY" = 'BYTE_EXACT'

        test "$LAST_BUILD_RESULT" = 'OUTCOME_A_BYTE_EXACT_ELF'
        test "$LAST_VALIDATED_WORKING_ELF_SHA256" = \
            "$M0_REFERENCE_ELF_SHA256"

        test -f docs/M0_BUILD_RESULT.md
        test -f evidence/m0/m0d2/SHA256SUMS.txt

        ACTUAL_M0D2_EVIDENCE_MANIFEST_SHA="$(
            sha256sum evidence/m0/m0d2/SHA256SUMS.txt |
            awk '{print $1}'
        )"

        test "$ACTUAL_M0D2_EVIDENCE_MANIFEST_SHA" = \
            "$M0D2_EVIDENCE_MANIFEST_SHA256"

        (
            cd evidence/m0/m0d2
            sha256sum -c SHA256SUMS.txt >/dev/null
        )

        if [ -f working/b4a/PS2VNC.ELF ]; then
            test "$(
                sha256sum working/b4a/PS2VNC.ELF |
                awk '{print $1}'
            )" = "$M0_REFERENCE_ELF_SHA256"
        fi

        echo 'M0_OUTCOME_A_AUTHORITY=PASS'
    fi

    echo 'M0_BUILD_AUTHORITY=PASS'
else
    echo 'M0_BUILD_AUTHORITY=PRE_M0C'
fi


echo
echo '===== K. M0 WRITABLE SOURCE AUTHORITY ====='

if [ "$CURRENT_WORKING_SOURCE" = 'NONE' ]; then
    echo 'M0_WRITABLE_SOURCE_AUTHORITY=NOT_YET_ACTIVE'
else
    case "$CURRENT_STAGE" in
        M0)
            ;;
        M1)
            case "$CURRENT_STAGE_STATUS" in
                NOT_STARTED)
                    ;;
                IN_PROGRESS)
                    case "$NEXT_ACTION" in
                        PUBLISH_create_verify_and_push_new_PS-to-VNC_GitHub_repository)
                            test -f docs/M1A_EXTRACTION_BOUNDARY.md
                            ;;
                        *)
                            echo "ERROR=M1_IN_PROGRESS_ACTION_NOT_YET_SUPPORTED:$NEXT_ACTION"
                            exit 73
                            ;;
                    esac
                    ;;
                *)
                    echo "ERROR=M1_STAGE_STATUS_NOT_SUPPORTED:$CURRENT_STAGE_STATUS"
                    exit 74
                    ;;
            esac
            ;;
        *)
            echo "ERROR=WORKING_SOURCE_STAGE_NOT_YET_SUPPORTED:$CURRENT_STAGE"
            exit 71
            ;;
    esac

    test "$CURRENT_WORKING_SOURCE" = 'working/b4a/ps2ip.c'

    test -f "$CURRENT_WORKING_SOURCE"
    test -f working/b4a/Makefile
    test -f working/b4a/ps2vnc_identity.c
    test -f working/b4a/ps2vnc_gsHires.c
    test -x scripts/build-m0-b4a.sh

    test "$(
        sha256sum "$CURRENT_WORKING_SOURCE" |
        awk '{print $1}'
    )" = "$M0_SOURCE_SHA256"

    test "$(
        sha256sum working/b4a/Makefile |
        awk '{print $1}'
    )" = "$M0_MAKEFILE_SHA256"

    test "$(
        sha256sum working/b4a/ps2vnc_identity.c |
        awk '{print $1}'
    )" = "$M0_IDENTITY_SOURCE_SHA256"

    test "$(
        sha256sum working/b4a/ps2vnc_gsHires.c |
        awk '{print $1}'
    )" = "$M0_GSHIRES_SOURCE_SHA256"

    if [ "$CURRENT_SOURCE_HEAD" != 'NONE' ]; then
        git cat-file -e "$CURRENT_SOURCE_HEAD^{commit}"
        git merge-base --is-ancestor "$CURRENT_SOURCE_HEAD" HEAD

        COMMITTED_WORKING_SHA="$(
            git show "$CURRENT_SOURCE_HEAD:$CURRENT_WORKING_SOURCE" |
            sha256sum |
            awk '{print $1}'
        )"

        test "$COMMITTED_WORKING_SHA" = "$M0_SOURCE_SHA256"
    fi

    echo "CURRENT_WORKING_SOURCE=$CURRENT_WORKING_SOURCE"
    echo "CURRENT_SOURCE_HEAD=$CURRENT_SOURCE_HEAD"
    echo 'M0_WRITABLE_SOURCE_AUTHORITY=PASS'
fi


echo
echo '===== L. M0 HARDWARE RESOLUTION ====='

if { [ "$CURRENT_STAGE" = 'M0' ] &&
       [ "$NEXT_ACTION" = 'M0G_close_M0_and_define_M1' ]; } ||
   [ "${M0_STAGE_STATUS:-}" = 'COMPLETE' ]
then
    test "$M0_REPRODUCTION_CLASSIFICATION" = \
        'OUTCOME_A_BYTE_EXACT_ELF'

    test "$M0_HARDWARE_VALIDATION_REQUIREMENT" = \
        'NOT_REQUIRED_BYTE_EXACT_DUT'

    test "$M0_HARDWARE_VALIDATION_BASIS" = \
        'HISTORICAL_B4A_VALIDATION_APPLIES_TO_IDENTICAL_ELF_BYTES'

    test "$M0_NEW_HARDWARE_RUN_PERFORMED" = 'NO'
    test "$M0_NEW_HARDWARE_RESULT" = 'NONE'
    test "$M0_HARDWARE_RESOLUTION" = 'PASS'

    test "$LAST_HARDWARE_RESULT" = \
        'M0_NO_NEW_RUN_BYTE_EXACT_DUT_B4A_HISTORICAL_VALIDATION_APPLIES'

    test -f docs/M0_HARDWARE_RESOLUTION.md

    test "$LAST_VALIDATED_WORKING_ELF_SHA256" = \
        "$M0_REFERENCE_ELF_SHA256"

    echo 'M0_NEW_HARDWARE_RUN_PERFORMED=NO'
    echo 'M0_HARDWARE_VALIDATION_REQUIREMENT=NOT_REQUIRED_BYTE_EXACT_DUT'
    echo 'M0_HARDWARE_RESOLUTION=PASS'
else
    echo 'M0_HARDWARE_RESOLUTION=NOT_YET_FINALIZED'
fi

echo

echo
echo '===== M. M0 COMPLETION AUTHORITY ====='

if [ "${M0_STAGE_STATUS:-}" = 'COMPLETE' ]; then
    test "$M0_COMPLETION_RESULT" = 'PASS'

    test "$M0_COMPLETION_WORKING_SOURCE" = \
        'working/b4a/ps2ip.c'

    test "$M0_COMPLETION_SOURCE_HEAD" = \
        'd1c0d6a4829c03f3a062095afd00859188e13dfe'

    test "$M0_COMPLETION_ELF_SHA256" = \
        "$M0_REFERENCE_ELF_SHA256"

    test "$M0_COMPLETION_ELF_IDENTITY" = \
        'BYTE_EXACT_B4A'

    test "$M0_COMPLETION_HARDWARE_REQUIREMENT" = \
        'NOT_REQUIRED_BYTE_EXACT_DUT'

    test "$M0_COMPLETION_NEW_HARDWARE_RUN" = 'NO'
    test "$M0_COMPLETION_NEXT_STAGE" = 'M1'

    test -f docs/M1_EXTRACTION_CONTRACT.md

    git cat-file -e \
        "$M0_COMPLETION_SOURCE_HEAD^{commit}"

    COMMITTED_M0_SOURCE_SHA="$(
        git show \
            "$M0_COMPLETION_SOURCE_HEAD:$M0_COMPLETION_WORKING_SOURCE" |
        sha256sum |
        awk '{print $1}'
    )"

    test "$COMMITTED_M0_SOURCE_SHA" = \
        "$M0_SOURCE_SHA256"

    echo 'M0_STAGE_STATUS=COMPLETE'
    echo "M0_COMPLETION_SOURCE_HEAD=$M0_COMPLETION_SOURCE_HEAD"
    echo 'M0_COMPLETION_AUTHORITY=PASS'
else
    echo 'M0_COMPLETION_AUTHORITY=NOT_YET_COMPLETE'
fi


echo
echo '===== N. HUMAN MIGRATION MIRROR ====='

HUMAN_MIRROR_FILE="$(
    mktemp /tmp/pstvnc-human-mirror.XXXXXX
)"

trap 'rm -f "$HUMAN_MIRROR_FILE"' EXIT

awk '
    $0 == "## Machine-state mirror" {
        inside = 1
        next
    }

    inside && /^## / {
        exit
    }

    inside {
        print
    }
' docs/MIGRATION_STATE.md > "$HUMAN_MIRROR_FILE"

human_mirror_require()
{
    local key="$1"
    local value="$2"
    local expected="    ${key}=${value}"
    local count

    count="$(
        {
            grep -Fxc \
                "$expected" \
                "$HUMAN_MIRROR_FILE" \
                || true
        }
    )"

    if [ "$count" -ne 1 ]; then
        echo "ERROR=HUMAN_MIRROR_FIELD_COUNT:${key}:${count}"
        exit 72
    fi
}

human_mirror_require \
    LAST_COMPLETE_STAGE \
    "$LAST_COMPLETE_STAGE"

human_mirror_require \
    CURRENT_STAGE \
    "$CURRENT_STAGE"

human_mirror_require \
    CURRENT_STAGE_STATUS \
    "$CURRENT_STAGE_STATUS"

human_mirror_require \
    CURRENT_WORKING_SOURCE \
    "$CURRENT_WORKING_SOURCE"

human_mirror_require \
    CURRENT_SOURCE_HEAD \
    "$CURRENT_SOURCE_HEAD"

human_mirror_require \
    NEXT_ACTION \
    "$NEXT_ACTION"

echo 'HUMAN_MIGRATION_MIRROR=PASS'


echo
echo '===== O. M1A EXTRACTION BOUNDARY ====='

if [ "$CURRENT_STAGE" = 'M1' ] &&
   [ "$CURRENT_STAGE_STATUS" = 'IN_PROGRESS' ] &&
   [ "$NEXT_ACTION" = \
       'PUBLISH_create_verify_and_push_new_PS-to-VNC_GitHub_repository' ]
then
    test -f docs/M1A_EXTRACTION_BOUNDARY.md

    grep -Fq \
        'ps2vnc_config_trim_left' \
        docs/M1A_EXTRACTION_BOUNDARY.md

    grep -Fq \
        'ps2vnc_config_trim_right' \
        docs/M1A_EXTRACTION_BOUNDARY.md

    test "$CURRENT_WORKING_SOURCE" = \
        'working/b4a/ps2ip.c'

    test "$CURRENT_SOURCE_HEAD" = \
        'd1c0d6a4829c03f3a062095afd00859188e13dfe'

    echo 'M1A_SELECTED_FUNCTION_1=ps2vnc_config_trim_left'
    echo 'M1A_SELECTED_FUNCTION_2=ps2vnc_config_trim_right'
    echo 'M1A_SOURCE_MUTATION=NO'
    echo 'M1A_EXTRACTION_BOUNDARY=PASS'
else
    echo 'M1A_EXTRACTION_BOUNDARY=NOT_AT_PUBLICATION_GATE'
fi

echo '===== FINAL ====='
echo 'PS_TO_VNC_MIGRATION_CHECK=PASS'
echo "LAST_COMPLETE_STAGE=$LAST_COMPLETE_STAGE"
echo "CURRENT_STAGE=$CURRENT_STAGE"
echo "CURRENT_STAGE_STATUS=$CURRENT_STAGE_STATUS"
echo "NEXT_ACTION=$NEXT_ACTION"
