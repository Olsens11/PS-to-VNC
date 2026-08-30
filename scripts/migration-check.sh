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

GIT_BRANCH="$(git branch --show-current)"
GIT_HEAD="$(git rev-parse HEAD)"
GIT_REMOTE_COUNT="$(git remote | wc -l)"

echo "GIT_BRANCH=$GIT_BRANCH"
echo "GIT_HEAD=$GIT_HEAD"
echo "GIT_REMOTE_COUNT=$GIT_REMOTE_COUNT"

test "$GIT_BRANCH" = 'main'

if [ "${GITHUB_PUBLICATION_STATUS:-}" = 'PRIVATE_PUBLISHED' ]; then
    test "$GIT_REMOTE_COUNT" -eq 1
    test "${GITHUB_OWNER:-}" = 'Olsens11'
    test "${GITHUB_REPOSITORY:-}" = 'Olsens11/PS-to-VNC'
    test "${GITHUB_REMOTE_NAME:-}" = 'origin'
    test "${GITHUB_REMOTE_URL:-}" = \
        'https://github.com/Olsens11/PS-to-VNC.git'
    test "${GITHUB_VISIBILITY:-}" = 'PRIVATE'
    test "${GITHUB_DEFAULT_BRANCH:-}" = 'main'

    test "$(git remote get-url origin)" = \
        "$GITHUB_REMOTE_URL"

    test "$(git remote get-url --push origin)" = \
        "$GITHUB_REMOTE_URL"

    echo 'GITHUB_PUBLICATION_STATUS=PRIVATE_PUBLISHED'
    echo 'GIT_AUTHORITY=PASS'
else
    test "$GIT_REMOTE_COUNT" -eq 0
    echo 'GIT_AUTHORITY=PASS'
fi

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
   [ "$NEXT_ACTION" != \
       'M0C_reconstruct_and_fingerprint_historical_build_environment' ]
then
    test -f runtime/M0_BUILD_AUTHORITY.env
fi

if [ -f runtime/M0_BUILD_AUTHORITY.env ]; then
    # M0_BUILD_AUTHORITY.env is historical authority once M0 is complete.
    #
    # LAST_BUILD_RESULT and LAST_VALIDATED_WORKING_ELF_SHA256 describe the
    # currently active generation and therefore legitimately advance in M1.
    # They are only required to equal M0 while M0 is still the active source
    # generation.

    # shellcheck disable=SC1091
    source runtime/M0_BUILD_AUTHORITY.env

    test "$M0_REFERENCE_DUT" = 'D17AL-F8J2-B4A'

    test "$M0_SOURCE_SHA256" = \
        "$BASELINE_SOURCE_SHA256"

    test "$M0_REFERENCE_ELF_SHA256" = \
        "$BASELINE_ELF_SHA256"

    test "$M0_B4A_LIBPS2IP_SHA256" = \
        'b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

    ACTUAL_M0_LIB_SHA="$(
        sha256sum \
            baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a |
        awk '{print $1}'
    )"

    test "$ACTUAL_M0_LIB_SHA" = \
        "$M0_B4A_LIBPS2IP_SHA256"

    test "$M0_B4A_LIBPS2IP_AUTHORITY" = \
        'FROZEN_MATCHES_HISTORICAL_B4A_WORKTREE'

    test "$M0_STALE_LEGACY_ARCHIVE_STATUS" = \
        'NOT_B4A_LINK_INPUT'

    test "$M0_REPRODUCTION_TARGET" = 'BYTE_EXACT'

    echo "M0_REFERENCE_DUT=$M0_REFERENCE_DUT"
    echo "M0_B4A_LIBPS2IP_SHA256=$M0_B4A_LIBPS2IP_SHA256"
    echo "M0_REFERENCE_ELF_SHA256=$M0_REFERENCE_ELF_SHA256"
    echo "M0_REPRODUCTION_CLASSIFICATION=$M0_REPRODUCTION_CLASSIFICATION"

    if [ "$M0_REPRODUCTION_CLASSIFICATION" = \
         'OUTCOME_A_BYTE_EXACT_ELF' ]
    then
        test "$M0D2_OBSERVED_ELF_SHA256" = \
            "$M0_REFERENCE_ELF_SHA256"

        test "$M0D2_OBSERVED_ELF_BYTES" = \
            "$M0_REFERENCE_ELF_BYTES"

        test "$M0D2_GSHIRES_LTO_PAYLOADS" = \
            'BYTE_IDENTICAL'

        test "$M0D2_GSHIRES_VARIANCE" = \
            'SLIM_LTO_SECTION_NAME_METADATA_ONLY'

        test "$M0D2_GSHIRES_FINAL_ELF_EFFECT" = \
            'NONE'

        test "$M0_FINAL_ELF_IDENTITY" = \
            'BYTE_EXACT'

        test -f docs/M0_BUILD_RESULT.md
        test -f evidence/m0/m0d2/SHA256SUMS.txt

        ACTUAL_M0D2_EVIDENCE_MANIFEST_SHA="$(
            sha256sum \
                evidence/m0/m0d2/SHA256SUMS.txt |
            awk '{print $1}'
        )"

        test "$ACTUAL_M0D2_EVIDENCE_MANIFEST_SHA" = \
            "$M0D2_EVIDENCE_MANIFEST_SHA256"

        (
            cd evidence/m0/m0d2
            sha256sum -c SHA256SUMS.txt >/dev/null
        )

        if [ "${M0_STAGE_STATUS:-}" = 'COMPLETE' ]; then
            test "$M0_COMPLETION_RESULT" = 'PASS'

            test "$M0_COMPLETION_ELF_SHA256" = \
                "$M0_REFERENCE_ELF_SHA256"

            test "$M0_COMPLETION_ELF_IDENTITY" = \
                'BYTE_EXACT_B4A'

            test "$M0_COMPLETION_SOURCE_HEAD" = \
                'd1c0d6a4829c03f3a062095afd00859188e13dfe'
        fi

        if [ "$CURRENT_SOURCE_HEAD" = \
             "${M0_COMPLETION_SOURCE_HEAD:-NONE}" ]
        then
            test "$LAST_BUILD_RESULT" = \
                'OUTCOME_A_BYTE_EXACT_ELF'

            test "$LAST_VALIDATED_WORKING_ELF_SHA256" = \
                "$M0_REFERENCE_ELF_SHA256"

            if [ -f working/b4a/PS2VNC.ELF ]; then
                test "$(
                    sha256sum working/b4a/PS2VNC.ELF |
                    awk '{print $1}'
                )" = "$M0_REFERENCE_ELF_SHA256"
            fi

            echo 'M0_ACTIVE_GENERATION_CURRENT_STATE=PASS'
        else
            echo 'M0_ACTIVE_GENERATION_CURRENT_STATE=HISTORICAL_ONLY'
        fi

        echo 'M0_OUTCOME_A_AUTHORITY=PASS'
    fi

    echo 'M0_BUILD_AUTHORITY=PASS'
else
    echo 'M0_BUILD_AUTHORITY=PRE_M0C'
fi


echo '===== K. WORKING SOURCE AUTHORITY ====='

if [ "$CURRENT_WORKING_SOURCE" = 'NONE' ]; then
    echo 'WORKING_SOURCE_AUTHORITY=NOT_YET_ACTIVE'
else
    test "$CURRENT_WORKING_SOURCE" = 'working/b4a/ps2ip.c'

    test -f "$CURRENT_WORKING_SOURCE"
    test -f working/b4a/Makefile
    test -f working/b4a/ps2vnc_identity.c
    test -f working/b4a/ps2vnc_gsHires.c

    test "$(
        sha256sum working/b4a/ps2vnc_identity.c |
        awk '{print $1}'
    )" = "$M0_IDENTITY_SOURCE_SHA256"

    test "$(
        sha256sum working/b4a/ps2vnc_gsHires.c |
        awk '{print $1}'
    )" = "$M0_GSHIRES_SOURCE_SHA256"

    if [ "$CURRENT_SOURCE_HEAD" = "$M0_COMPLETION_SOURCE_HEAD" ]; then
        test "$(
            sha256sum "$CURRENT_WORKING_SOURCE" |
            awk '{print $1}'
        )" = "$M0_SOURCE_SHA256"

        test "$(
            sha256sum working/b4a/Makefile |
            awk '{print $1}'
        )" = "$M0_MAKEFILE_SHA256"

        git cat-file -e "$CURRENT_SOURCE_HEAD^{commit}"

        test "$(
            git show \
                "${CURRENT_SOURCE_HEAD}:${CURRENT_WORKING_SOURCE}" |
            sha256sum |
            awk '{print $1}'
        )" = "$M0_SOURCE_SHA256"

        echo 'WORKING_SOURCE_GENERATION=M0'
        echo 'M0_WRITABLE_SOURCE_AUTHORITY=PASS'

    elif [ "$CURRENT_STAGE" = 'M1' ]; then
        test -f runtime/M1_SOURCE_AUTHORITY.env

        # shellcheck disable=SC1091
        source runtime/M1_SOURCE_AUTHORITY.env

        test "$M1_SOURCE_STAGE" = 'M1B'
        test "$M1_SOURCE_STATUS" = 'EXTRACTED'
        test "$M1_SOURCE_COMMIT" = "$CURRENT_SOURCE_HEAD"

        test "$M1_MONOLITH_PATH" = \
            'working/b4a/ps2ip.c'

        test "$M1_CONFIG_TEXT_SOURCE_PATH" = \
            'working/b4a/ps2vnc_config_text.c'

        test "$M1_CONFIG_TEXT_HEADER_PATH" = \
            'working/b4a/ps2vnc_config_text.h'

        test -f "$M1_CONFIG_TEXT_SOURCE_PATH"
        test -f "$M1_CONFIG_TEXT_HEADER_PATH"

        test "$(
            sha256sum "$M1_MONOLITH_PATH" |
            awk '{print $1}'
        )" = "$M1_MONOLITH_SHA256"

        test "$(
            sha256sum "$M1_CONFIG_TEXT_SOURCE_PATH" |
            awk '{print $1}'
        )" = "$M1_CONFIG_TEXT_SOURCE_SHA256"

        test "$(
            sha256sum "$M1_CONFIG_TEXT_HEADER_PATH" |
            awk '{print $1}'
        )" = "$M1_CONFIG_TEXT_HEADER_SHA256"

        test "$(
            sha256sum working/b4a/Makefile |
            awk '{print $1}'
        )" = "$M1_MAKEFILE_SHA256"

        test "$M1_MONOLITH_SHA256" != "$M0_SOURCE_SHA256"
        test "$M1_MAKEFILE_SHA256" != "$M0_MAKEFILE_SHA256"

        git cat-file -e "$M1_SOURCE_COMMIT^{commit}"
        git merge-base --is-ancestor "$M1_SOURCE_COMMIT" HEAD

        test "$(
            git show \
                "${M1_SOURCE_COMMIT}:${M1_MONOLITH_PATH}" |
            sha256sum |
            awk '{print $1}'
        )" = "$M1_MONOLITH_SHA256"

        test "$(
            git show \
                "${M1_SOURCE_COMMIT}:${M1_CONFIG_TEXT_SOURCE_PATH}" |
            sha256sum |
            awk '{print $1}'
        )" = "$M1_CONFIG_TEXT_SOURCE_SHA256"

        test "$(
            git show \
                "${M1_SOURCE_COMMIT}:${M1_CONFIG_TEXT_HEADER_PATH}" |
            sha256sum |
            awk '{print $1}'
        )" = "$M1_CONFIG_TEXT_HEADER_SHA256"

        test "$(
            git show \
                "${M1_SOURCE_COMMIT}:working/b4a/Makefile" |
            sha256sum |
            awk '{print $1}'
        )" = "$M1_MAKEFILE_SHA256"

        test "$(
            grep -Fxc \
                '#include "ps2vnc_config_text.h"' \
                "$M1_MONOLITH_PATH"
        )" -eq 1

        if grep -Fq \
            'static char *ps2vnc_config_trim_left' \
            "$M1_MONOLITH_PATH"
        then
            echo 'ERROR=M1B_TRIM_LEFT_STILL_PRIVATE_IN_MONOLITH'
            exit 75
        fi

        if grep -Fq \
            'static void ps2vnc_config_trim_right' \
            "$M1_MONOLITH_PATH"
        then
            echo 'ERROR=M1B_TRIM_RIGHT_STILL_PRIVATE_IN_MONOLITH'
            exit 76
        fi

        grep -Fq \
            'ps2vnc_config_text.o' \
            working/b4a/Makefile

        echo 'WORKING_SOURCE_GENERATION=M1B'
        echo "CURRENT_WORKING_SOURCE=$CURRENT_WORKING_SOURCE"
        echo "CURRENT_SOURCE_HEAD=$CURRENT_SOURCE_HEAD"
        echo 'M1_WORKING_SOURCE_AUTHORITY=PASS'
    else
        echo "ERROR=UNKNOWN_WORKING_SOURCE_GENERATION:$CURRENT_STAGE"
        exit 77
    fi
fi

echo '===== L. M0 HARDWARE RESOLUTION ====='

if { [ "$CURRENT_STAGE" = 'M0' ] &&
       [ "$NEXT_ACTION" = 'M0G_close_M0_and_define_M1' ]; } ||
   [ "${M0_STAGE_STATUS:-}" = 'COMPLETE' ]
then
    # M0 hardware resolution is permanent historical authority once M0 is
    # complete.
    #
    # LAST_HARDWARE_RESULT and LAST_VALIDATED_WORKING_ELF_SHA256 describe
    # the current project generation. They are required to contain the M0
    # values only while the completed M0 source generation is still active.

    test "$M0_REPRODUCTION_CLASSIFICATION" = \
        'OUTCOME_A_BYTE_EXACT_ELF'

    test "$M0_HARDWARE_VALIDATION_REQUIREMENT" = \
        'NOT_REQUIRED_BYTE_EXACT_DUT'

    test "$M0_HARDWARE_VALIDATION_BASIS" = \
        'HISTORICAL_B4A_VALIDATION_APPLIES_TO_IDENTICAL_ELF_BYTES'

    test "$M0_NEW_HARDWARE_RUN_PERFORMED" = 'NO'
    test "$M0_NEW_HARDWARE_RESULT" = 'NONE'
    test "$M0_HARDWARE_RESOLUTION" = 'PASS'

    test -f docs/M0_HARDWARE_RESOLUTION.md

    if [ "$CURRENT_SOURCE_HEAD" = \
         "${M0_COMPLETION_SOURCE_HEAD:-NONE}" ]
    then
        test "$LAST_HARDWARE_RESULT" = \
            'M0_NO_NEW_RUN_BYTE_EXACT_DUT_B4A_HISTORICAL_VALIDATION_APPLIES'

        test "$LAST_VALIDATED_WORKING_ELF_SHA256" = \
            "$M0_REFERENCE_ELF_SHA256"

        echo 'M0_HARDWARE_CURRENT_STATE=ACTIVE_GENERATION_PASS'
    else
        echo 'M0_HARDWARE_CURRENT_STATE=HISTORICAL_ONLY'
    fi

    echo 'M0_NEW_HARDWARE_RUN_PERFORMED=NO'
    echo 'M0_HARDWARE_VALIDATION_REQUIREMENT=NOT_REQUIRED_BYTE_EXACT_DUT'
    echo 'M0_HARDWARE_RESOLUTION=PASS'
else
    echo 'M0_HARDWARE_RESOLUTION=NOT_YET_FINALIZED'
fi


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

if [ "$CURRENT_STAGE" = 'M1' ]; then
    test -f docs/M1A_EXTRACTION_BOUNDARY.md

    grep -Fq \
        'ps2vnc_config_trim_left' \
        docs/M1A_EXTRACTION_BOUNDARY.md

    grep -Fq \
        'ps2vnc_config_trim_right' \
        docs/M1A_EXTRACTION_BOUNDARY.md

    echo 'M1A_SELECTED_FUNCTION_1=ps2vnc_config_trim_left'
    echo 'M1A_SELECTED_FUNCTION_2=ps2vnc_config_trim_right'
    echo 'M1A_EXTRACTION_BOUNDARY=PASS'
else
    echo 'M1A_EXTRACTION_BOUNDARY=NOT_CURRENT_STAGE'
fi


echo '===== P. GITHUB PUBLICATION AUTHORITY ====='

if [ "${GITHUB_PUBLICATION_STATUS:-}" = 'PRIVATE_PUBLISHED' ]; then
    test "$CURRENT_STAGE" = 'M1'
    test "$CURRENT_STAGE_STATUS" = 'IN_PROGRESS'

    test "$GITHUB_REPOSITORY" = \
        'Olsens11/PS-to-VNC'

    test "$GITHUB_VISIBILITY" = 'PRIVATE'

    test "$GITHUB_REMOTE_URL" = \
        'https://github.com/Olsens11/PS-to-VNC.git'

    test -f docs/GITHUB_PUBLICATION.md
    test -f docs/M1A_EXTRACTION_BOUNDARY.md

    echo 'GITHUB_REPOSITORY=Olsens11/PS-to-VNC'
    echo 'GITHUB_VISIBILITY=PRIVATE'
    echo 'GITHUB_PUBLICATION_AUTHORITY=PASS'
else
    echo 'GITHUB_PUBLICATION_AUTHORITY=NOT_YET_PUBLISHED'
fi

echo
echo '===== Q. M1 LIFECYCLE ACTION AUTHORITY ====='

if [ "$CURRENT_STAGE" = 'M1' ]; then
    test "$CURRENT_STAGE_STATUS" = 'IN_PROGRESS'

    case "$NEXT_ACTION" in
        PUBLISH_create_verify_and_push_new_PS-to-VNC_GitHub_repository)
            test "$CURRENT_SOURCE_HEAD" = \
                "$M0_COMPLETION_SOURCE_HEAD"

            test -f docs/M1A_EXTRACTION_BOUNDARY.md

            echo 'M1_ACTION=PUBLICATION_GATE'
            ;;

        M1B_mechanically_extract_config_text_module)
            test "${GITHUB_PUBLICATION_STATUS:-}" = \
                'PRIVATE_PUBLISHED'

            test "$CURRENT_SOURCE_HEAD" = \
                "$M0_COMPLETION_SOURCE_HEAD"

            test -f docs/M1A_EXTRACTION_BOUNDARY.md
            test -f docs/GITHUB_PUBLICATION.md

            echo 'M1_ACTION=M1B_EXTRACTION'
            ;;

        M1C_build_and_characterize_first_modularized_DUT)
            test "${GITHUB_PUBLICATION_STATUS:-}" = \
                'PRIVATE_PUBLISHED'

            test -f runtime/M1_SOURCE_AUTHORITY.env
            test -f docs/M1B_EXTRACTION_RESULT.md

            echo 'M1_ACTION=M1C_BUILD_CHARACTERIZATION'
            ;;

        M1D_run_required_hardware_regression)
            test "${GITHUB_PUBLICATION_STATUS:-}" = \
                'PRIVATE_PUBLISHED'

            test -f runtime/M1_SOURCE_AUTHORITY.env
            test -f runtime/M1_DUT_AUTHORITY.env
            test -f docs/M1C_DUT_CHARACTERIZATION.md

            echo 'M1_ACTION=M1D_HARDWARE_REGRESSION'
            ;;

        *)
            echo "ERROR=UNSUPPORTED_M1_NEXT_ACTION:$NEXT_ACTION"
            exit 78
            ;;
    esac

    echo 'M1_LIFECYCLE_ACTION_AUTHORITY=PASS'
else
    echo 'M1_LIFECYCLE_ACTION_AUTHORITY=NOT_CURRENT_STAGE'
fi


echo
echo '===== R. M1B SOURCE AUTHORITY ====='

if [ "$CURRENT_STAGE" = 'M1' ] &&
   [ "$CURRENT_SOURCE_HEAD" != "$M0_COMPLETION_SOURCE_HEAD" ]
then
    test -f runtime/M1_SOURCE_AUTHORITY.env

    # shellcheck disable=SC1091
    source runtime/M1_SOURCE_AUTHORITY.env

    test "$M1_SOURCE_STAGE" = 'M1B'
    test "$M1_SOURCE_STATUS" = 'EXTRACTED'
    test "$M1_SOURCE_AUTHORITY_ROLE" = 'SOURCE_ONLY'

    test "$M1_SOURCE_COMMIT" = "$CURRENT_SOURCE_HEAD"

    test "$M1_SELECTED_FUNCTION_1" = \
        'ps2vnc_config_trim_left'

    test "$M1_SELECTED_FUNCTION_2" = \
        'ps2vnc_config_trim_right'

    test "$M1_SELECTED_GLOBAL_REFS" = '0'
    test "$M1_SELECTED_EXTERNAL_CALLEES" = '0'
    test "$M1_SELECTED_EXTERNAL_CALLERS" = '3'

    echo "M1B_SOURCE_COMMIT=$M1_SOURCE_COMMIT"
    echo 'M1B_SOURCE_AUTHORITY=PASS'
else
    echo 'M1B_SOURCE_AUTHORITY=NOT_ACTIVE'
fi


echo
echo '===== S. M1C DUT AUTHORITY ====='

if [ "$CURRENT_STAGE" = 'M1' ] &&
   [ "$NEXT_ACTION" = 'M1D_run_required_hardware_regression' ]
then
    test -f runtime/M1_DUT_AUTHORITY.env

    # shellcheck disable=SC1091
    source runtime/M1_DUT_AUTHORITY.env

    test "$M1_DUT_STAGE" = 'M1C'

    test "$M1_DUT_STATUS" = \
        'BUILT_CHARACTERIZED_NOT_HARDWARE_VALIDATED'

    test "$M1_DUT_SOURCE_COMMIT" = \
        "$CURRENT_SOURCE_HEAD"

    test "$M1_DUT_ELF_PATH" = \
        'working/b4a/PS2VNC.ELF'

    test -f "$M1_DUT_ELF_PATH"

    test "$(
        sha256sum "$M1_DUT_ELF_PATH" |
        awk '{print $1}'
    )" = "$M1_DUT_ELF_SHA256"

    test "$(
        wc -c < "$M1_DUT_ELF_PATH"
    )" -eq "$M1_DUT_ELF_BYTES"

    test "$M1_DUT_ELF_SHA256" = \
        "$LAST_VALIDATED_WORKING_ELF_SHA256"

    test "$M1_DUT_M0_REFERENCE_ELF_SHA256" = \
        "$M0_REFERENCE_ELF_SHA256"

    test "$M1_DUT_IDENTITY_RELATION" = \
        'NEW_NONIDENTICAL_DUT'

    test "$M1_DUT_SECOND_BUILD_SHA256" = \
        "$M1_DUT_ELF_SHA256"

    test "$M1_DUT_SECOND_BUILD_REPRODUCTION" = \
        'BYTE_EXACT'

    test "$M1_DUT_TRANSLATION_UNIT_LINKAGE" = 'PASS'
    test "$M1_DUT_HOST_CONFIG_TEXT_PARITY" = 'PASS'

    test "$M1_DUT_HARDWARE_STATUS" = 'NOT_RUN'

    case "$M1_DUT_DEPLOYMENT_STATUS" in
        NOT_DEPLOYED)
            echo 'M1_DUT_DEPLOYMENT_AUTHORITY=NOT_YET_DEPLOYED'
            ;;

        DEPLOYED_VERIFIED)
            test "$M1_DUT_DEPLOYED_ELF_SHA256" = \
                "$M1_DUT_ELF_SHA256"

            test "$M1_DUT_DEPLOYMENT_METHOD" = \
                'CURL_FTP_DUAL_TARGET_READBACK_SHA256'

            test -d "$M1_DUT_DEPLOYMENT_EVIDENCE"

            test -f \
                "$M1_DUT_DEPLOYMENT_EVIDENCE/DEPLOYMENT-RESULT.env"

            test -f \
                "$M1_DUT_DEPLOYMENT_EVIDENCE/SHA256SUMS.txt"

            test "$(
                sha256sum \
                    "$M1_DUT_DEPLOYMENT_EVIDENCE/SHA256SUMS.txt" |
                awk '{print $1}'
            )" = \
                "$M1_DUT_DEPLOYMENT_EVIDENCE_MANIFEST_SHA256"

            (
                cd "$M1_DUT_DEPLOYMENT_EVIDENCE"
                sha256sum -c SHA256SUMS.txt >/dev/null
            )

            # shellcheck disable=SC1090
            source \
                "$M1_DUT_DEPLOYMENT_EVIDENCE/DEPLOYMENT-RESULT.env"

            test "$DEPLOYMENT_RESULT" = 'PASS'
            test "$LOCAL_ELF_SHA256" = "$M1_DUT_ELF_SHA256"
            test "$UNIQUE_READBACK_SHA256" = "$M1_DUT_ELF_SHA256"
            test "$ROLLING_READBACK_SHA256" = "$M1_DUT_ELF_SHA256"
            test "$HARDWARE_RUN" = 'NO'

            echo 'M1_DUT_DEPLOYMENT_AUTHORITY=PASS'
            ;;

        *)
            echo \
                "ERROR=UNKNOWN_M1_DUT_DEPLOYMENT_STATUS:$M1_DUT_DEPLOYMENT_STATUS"
            exit 79
            ;;
    esac

    test -f "$M1_DUT_EVIDENCE_ELF"

    test "$(
        sha256sum "$M1_DUT_EVIDENCE_ELF" |
        awk '{print $1}'
    )" = "$M1_DUT_ELF_SHA256"

    test "$(
        sha256sum \
            "$M1_DUT_M1C1_EVIDENCE/SHA256SUMS.txt" |
        awk '{print $1}'
    )" = "$M1_DUT_M1C1_EVIDENCE_MANIFEST_SHA256"

    test "$(
        sha256sum \
            "$M1_DUT_M1C2_EVIDENCE/SHA256SUMS.txt" |
        awk '{print $1}'
    )" = "$M1_DUT_M1C2_EVIDENCE_MANIFEST_SHA256"

    (
        cd "$M1_DUT_M1C1_EVIDENCE"
        sha256sum -c SHA256SUMS.txt >/dev/null
    )

    (
        cd "$M1_DUT_M1C2_EVIDENCE"
        sha256sum -c SHA256SUMS.txt >/dev/null
    )

    test "$LAST_BUILD_RESULT" = \
        'M1C_BUILD_PASS_NEW_DUT'

    echo "M1_DUT_ELF_SHA256=$M1_DUT_ELF_SHA256"
    echo 'M1_DUT_SECOND_BUILD_REPRODUCTION=BYTE_EXACT'
    echo 'M1_DUT_HOST_CONFIG_TEXT_PARITY=PASS'
    echo 'M1_DUT_HARDWARE_STATUS=NOT_RUN'
    echo 'M1C_DUT_AUTHORITY=PASS'
else
    echo 'M1C_DUT_AUTHORITY=NOT_AT_M1D_GATE'
fi


echo '===== FINAL ====='
echo 'PS_TO_VNC_MIGRATION_CHECK=PASS'
echo "LAST_COMPLETE_STAGE=$LAST_COMPLETE_STAGE"
echo "CURRENT_STAGE=$CURRENT_STAGE"
echo "CURRENT_STAGE_STATUS=$CURRENT_STAGE_STATUS"
echo "NEXT_ACTION=$NEXT_ACTION"
