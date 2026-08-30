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
case "$CURRENT_STAGE" in
    M1)
        test "$LAST_COMPLETE_STAGE" = 'M0'
        ;;

    M2)
        test "$LAST_COMPLETE_STAGE" = 'M1'
        ;;

    M3)
        test "$LAST_COMPLETE_STAGE" = 'M2'
        test "$CURRENT_STAGE_STATUS" = 'IN_PROGRESS'
        test "$NEXT_ACTION" = \
            'M3A_plan_accelerated_extraction_waves'
        ;;

    *)
        echo "ERROR=UNSUPPORTED_CURRENT_STAGE:$CURRENT_STAGE"
        exit 75
        ;;
esac

test "$BLOCKED_BY" = 'NONE'

echo "LAST_COMPLETE_STAGE=$LAST_COMPLETE_STAGE"
echo "CURRENT_STAGE=$CURRENT_STAGE"
echo "CURRENT_STAGE_STATUS=$CURRENT_STAGE_STATUS"
echo "NEXT_ACTION=$NEXT_ACTION"
echo "BLOCKED_BY=$BLOCKED_BY"
echo 'MACHINE_MIGRATION_STATE=PASS'

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
python3 - \
    "$LAST_COMPLETE_STAGE" \
    "$CURRENT_STAGE" \
    "$CURRENT_STAGE_STATUS" \
    "$CURRENT_WORKING_SOURCE" \
    "$CURRENT_SOURCE_HEAD" \
    "$LAST_HARDWARE_RESULT" \
    "$NEXT_ACTION" <<'PY2'
from pathlib import Path
import sys

expected = {
    "LAST_COMPLETE_STAGE": sys.argv[1],
    "CURRENT_STAGE": sys.argv[2],
    "CURRENT_STAGE_STATUS": sys.argv[3],
    "CURRENT_WORKING_SOURCE": sys.argv[4],
    "CURRENT_SOURCE_HEAD": sys.argv[5],
    "LAST_HARDWARE_RESULT": sys.argv[6],
    "NEXT_ACTION": sys.argv[7],
}

lines = Path("docs/MIGRATION_STATE.md").read_text().splitlines()

start = next(
    i for i, line in enumerate(lines)
    if line.strip() == "## Machine-state mirror"
)

end = len(lines)

for i in range(start + 1, len(lines)):
    if lines[i].startswith("## "):
        end = i
        break

actual = {}

for line in lines[start + 1:end]:
    text = line.strip()

    if "=" not in text:
        continue

    key, value = text.split("=", 1)

    if key in actual:
        raise SystemExit(
            f"duplicate machine-mirror field: {key}"
        )

    actual[key] = value

for key, value in expected.items():
    if actual.get(key) != value:
        raise SystemExit(
            f"mirror mismatch {key}: "
            f"{actual.get(key)!r} != {value!r}"
        )
PY2

echo 'HUMAN_MACHINE_DOCUMENTATION_COHERENCE=PASS'

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
test "$CURRENT_WORKING_SOURCE" = 'working/b4a/ps2ip.c'
test -f "$CURRENT_WORKING_SOURCE"

case "$CURRENT_STAGE" in
    M1)
        source runtime/M1_SOURCE_AUTHORITY.env
        test "$M1_SOURCE_COMMIT" = "$CURRENT_SOURCE_HEAD"
        echo 'WORKING_SOURCE_GENERATION=M1B'
        ;;

    M2|M3)
        source runtime/M2_SOURCE_AUTHORITY.env

        test "$M2_SOURCE_STATUS" = 'EXTRACTED'
        test "$M2_SOURCE_COMMIT" = "$CURRENT_SOURCE_HEAD"

        test "$(
            sha256sum "$M2_MONOLITH_PATH" |
            awk '{print $1}'
        )" = "$M2_POST_MONOLITH_SHA256"

        test "$(
            sha256sum "$M2_CONFIG_TEXT_SOURCE_PATH" |
            awk '{print $1}'
        )" = "$M2_POST_CONFIG_TEXT_SOURCE_SHA256"

        test "$(
            sha256sum "$M2_CONFIG_TEXT_HEADER_PATH" |
            awk '{print $1}'
        )" = "$M2_POST_CONFIG_TEXT_HEADER_SHA256"

        test "$(
            sha256sum "$M2_MAKEFILE_PATH" |
            awk '{print $1}'
        )" = "$M2_MAKEFILE_SHA256"

        echo 'WORKING_SOURCE_GENERATION=M2B'
        echo "CURRENT_WORKING_SOURCE=$CURRENT_WORKING_SOURCE"
        echo "CURRENT_SOURCE_HEAD=$CURRENT_SOURCE_HEAD"

        if [ "$CURRENT_STAGE" = 'M3' ]; then
            echo 'M2_WORKING_SOURCE_AUTHORITY=HISTORICAL_BASE_FOR_M3'
        else
            echo 'M2_WORKING_SOURCE_AUTHORITY=PASS'
        fi
        ;;

    *)
        echo "ERROR=UNSUPPORTED_WORKING_SOURCE_STAGE:$CURRENT_STAGE"
        exit 76
        ;;
esac

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
python3 - \
    "$LAST_COMPLETE_STAGE" \
    "$CURRENT_STAGE" \
    "$CURRENT_STAGE_STATUS" \
    "$NEXT_ACTION" <<'PY2'
from pathlib import Path
import sys

expected = {
    "LAST_COMPLETE_STAGE": sys.argv[1],
    "CURRENT_STAGE": sys.argv[2],
    "CURRENT_STAGE_STATUS": sys.argv[3],
    "NEXT_ACTION": sys.argv[4],
}

lines = Path("docs/MIGRATION_STATE.md").read_text().splitlines()

start = next(
    i for i, line in enumerate(lines)
    if line.strip() == "## Machine-state mirror"
)

end = len(lines)

for i in range(start + 1, len(lines)):
    if lines[i].startswith("## "):
        end = i
        break

actual = {}

for line in lines[start + 1:end]:
    text = line.strip()

    if "=" in text:
        key, value = text.split("=", 1)
        actual[key] = value

for key, value in expected.items():
    if actual.get(key) != value:
        raise SystemExit(
            f"human migration mirror mismatch: {key}"
        )
PY2

echo 'HUMAN_MIGRATION_MIRROR=PASS'

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
    # Publication authority persists across migration stages.
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

    case "$M1_DUT_HARDWARE_STATUS" in
        NOT_RUN)
            echo 'M1_DUT_HARDWARE_AUTHORITY=NOT_YET_RUN'
            ;;

        PASS)
            test "$M1_DUT_HARDWARE_RESULT" = \
                'M1D_PASS_MACHINE_AND_PHYSICAL'

            test "$M1_DUT_MACHINE_RESULT" = 'PASS'
            test "$M1_DUT_PHYSICAL_RESULT" = 'PASS'
            test "$M1_DUT_STARTUP_GATE" = 'PASS'

            test -d "$M1_DUT_HARDWARE_EVIDENCE"
            test -f \
                "$M1_DUT_HARDWARE_EVIDENCE/MACHINE-RESULT.env"
            test -f \
                "$M1_DUT_HARDWARE_EVIDENCE/PHYSICAL-OBSERVATION.txt"
            test -f \
                "$M1_DUT_HARDWARE_EVIDENCE/SHA256SUMS.txt"

            test "$(
                sha256sum \
                    "$M1_DUT_HARDWARE_EVIDENCE/SHA256SUMS.txt" |
                awk '{print $1}'
            )" = \
                "$M1_DUT_HARDWARE_EVIDENCE_MANIFEST_SHA256"

            (
                cd "$M1_DUT_HARDWARE_EVIDENCE"
                sha256sum -c SHA256SUMS.txt >/dev/null
            )

            grep -Fxq \
                'MACHINE_RESULT=PASS' \
                "$M1_DUT_HARDWARE_EVIDENCE/MACHINE-RESULT.env"

            grep -Fxq \
                'PHYSICAL_RESULT=PASS' \
                "$M1_DUT_HARDWARE_EVIDENCE/PHYSICAL-OBSERVATION.txt"

            echo 'M1_DUT_HARDWARE_AUTHORITY=PASS'
            ;;

        *)
            echo \
                "ERROR=UNKNOWN_M1_DUT_HARDWARE_STATUS:$M1_DUT_HARDWARE_STATUS"
            exit 1
            ;;
    esac

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
    echo "M1_DUT_HARDWARE_STATUS=$M1_DUT_HARDWARE_STATUS"
    echo 'M1C_DUT_AUTHORITY=PASS'
else
    echo 'M1C_DUT_AUTHORITY=NOT_AT_M1D_GATE'
fi


echo
echo '===== T. M1 COMPLETION AUTHORITY ====='

if [ "$LAST_COMPLETE_STAGE" = 'M1' ]; then
    test -f runtime/M1_COMPLETION_AUTHORITY.env
    test -f runtime/M1_SOURCE_AUTHORITY.env
    test -f runtime/M1_DUT_AUTHORITY.env
    test -f docs/M1_COMPLETION.md

    # shellcheck disable=SC1091
    source runtime/M1_COMPLETION_AUTHORITY.env

    test "$M1_STAGE" = 'M1'
    test "$M1_STAGE_STATUS" = 'COMPLETE'
    test "$M1_COMPLETION_RESULT" = 'PASS'

    test "$M1_SOURCE_COMMIT" = \
        '0f1b88ddf7821c935b68aabe6d65180bf02b074f'

    test "$M1_DUT_SHA256" = \
        '26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b'

    test "$M1_BUILD_RESULT" = \
        'M1C_BUILD_PASS_NEW_DUT'

    test "$M1_HARDWARE_RESULT" = \
        'M1D_PASS_MACHINE_AND_PHYSICAL'

    test "$M1_MACHINE_RESULT" = 'PASS'
    test "$M1_PHYSICAL_RESULT" = 'PASS'
    test "$M1_STARTUP_GATE" = 'PASS'

    test -d "$M1_HARDWARE_EVIDENCE"
    test -f "$M1_HARDWARE_EVIDENCE/SHA256SUMS.txt"

    (
        cd "$M1_HARDWARE_EVIDENCE"
        sha256sum -c SHA256SUMS.txt >/dev/null
    )

    test "$M1_NEXT_STAGE" = 'M2'

    echo 'M1_COMPLETION_AUTHORITY=PASS'
else
    echo 'M1_COMPLETION_AUTHORITY=NOT_COMPLETE_STAGE'
fi

echo
echo '===== U. M2A EXTRACTION BOUNDARY ====='
if [ "$CURRENT_STAGE" = 'M2' ]; then
    test -f runtime/M2_BOUNDARY_AUTHORITY.env
    test -f docs/M2A_EXTRACTION_BOUNDARY.md

    # shellcheck disable=SC1091
    source runtime/M2_BOUNDARY_AUTHORITY.env

    test "$M2_STAGE" = 'M2A'
    test "$M2_BOUNDARY_STATUS" = 'SELECTED'
    test "$M2_SOURCE_MUTATION" = 'NO'

    test "$M2_SELECTION_BASE_SOURCE_HEAD" = \
        '0f1b88ddf7821c935b68aabe6d65180bf02b074f'

    test "$M2_TARGET_MODULE" = \
        'working/b4a/ps2vnc_config_text.c'

    test "$M2_SELECTED_FUNCTION_1" = \
        'ps2vnc_config_parse_int'

    test "$M2_SELECTED_FUNCTION_2" = \
        'ps2vnc_config_parse_bool'

    test "$M2_SELECTED_GLOBAL_REFS" = '0'
    test "$M2_SELECTED_EXTERNAL_CALLEES" = '0'
    test "$M2_SELECTED_CROSS_MODULE_INCOMING_EDGES" = '1'
    test "$M2_SELECTED_TOTAL_CALL_SITES" = '7'

    echo 'M2A_EXTRACTION_BOUNDARY=PASS'
else
    echo 'M2A_EXTRACTION_BOUNDARY=NOT_CURRENT_STAGE'
fi


echo
echo '===== V. M2B SOURCE AUTHORITY ====='

if [ "$CURRENT_STAGE" = 'M2' ] &&
   [ "$NEXT_ACTION" = \
       'M2C_build_and_characterize_scalar_parser_DUT' ]
then
    test -f runtime/M2_SOURCE_AUTHORITY.env
    test -f docs/M2B_EXTRACTION_RESULT.md

    # shellcheck disable=SC1091
    source runtime/M2_SOURCE_AUTHORITY.env

    test "$M2_SOURCE_STAGE" = 'M2B'
    test "$M2_SOURCE_STATUS" = 'EXTRACTED'
    test "$M2_SOURCE_AUTHORITY_ROLE" = 'SOURCE_ONLY'

    test "$M2_SELECTED_FUNCTION_1" = \
        'ps2vnc_config_parse_int'

    test "$M2_SELECTED_FUNCTION_2" = \
        'ps2vnc_config_parse_bool'

    test "$M2_CALLER_MUTATION" = 'NO'
    test "$M2_BODY_MUTATION" = 'NO'
    test "$M2_STATIC_LINKAGE_REMOVAL_ONLY" = 'YES'
    test "$M2_MAKEFILE_MUTATION" = 'NO'
    test "$M2_NEW_OBJECT" = 'NO'
    test "$M2_EXISTING_OBJECT" = 'ps2vnc_config_text.o'

    test "$M2_BUILD_STATUS" = 'NOT_RUN'
    test "$M2_HARDWARE_STATUS" = 'NOT_RUN'

    test "$M2_SOURCE_COMMIT" = "$CURRENT_SOURCE_HEAD"

    if grep -Eq \
        '^[[:space:]]*static[[:space:]]+int[[:space:]]+ps2vnc_config_parse_(int|bool)[[:space:]]*\(' \
        "$M2_MONOLITH_PATH"
    then
        echo 'ERROR=M2B_PRIVATE_DEFINITION_REMAINS'
        exit 80
    fi

    test "$(
        grep -Ec \
            '^[[:space:]]*int[[:space:]]+ps2vnc_config_parse_int[[:space:]]*\(' \
            "$M2_CONFIG_TEXT_SOURCE_PATH"
    )" -eq 1

    test "$(
        grep -Ec \
            '^[[:space:]]*int[[:space:]]+ps2vnc_config_parse_bool[[:space:]]*\(' \
            "$M2_CONFIG_TEXT_SOURCE_PATH"
    )" -eq 1

    grep -Fq \
        'int ps2vnc_config_parse_int(' \
        "$M2_CONFIG_TEXT_HEADER_PATH"

    grep -Fq \
        'int ps2vnc_config_parse_bool(' \
        "$M2_CONFIG_TEXT_HEADER_PATH"

    echo "M2B_SOURCE_COMMIT=$M2_SOURCE_COMMIT"
    echo 'M2B_SOURCE_AUTHORITY=PASS'
else
    echo 'M2B_SOURCE_AUTHORITY=NOT_ACTIVE'
fi

echo
echo '===== W. M2C DUT AUTHORITY ====='

if [ "$LAST_COMPLETE_STAGE" = 'M2' ] ||
   [ "$CURRENT_STAGE" = 'M2' ]
then
    test -f runtime/M2_DUT_AUTHORITY.env

    source runtime/M2_DUT_AUTHORITY.env

    test "$M2_DUT_STAGE" = 'M2C'
    test "$M2_DUT_STATUS" = 'BUILT_CHARACTERIZED'
    test "$M2_DUT_MACHINE_RESULT" = 'PASS'

    test "$M2_DUT_ELF_SHA256" = \
        'af245ae9f1145b5750377f83375fa9ec640f19ecd320461dd5ef80809186b84e'

    test "$M2_DUT_ELF_BYTES" = '2927920'

    test "$M2_DUT_SECOND_BUILD_REPRODUCTION" = 'BYTE_EXACT'
    test "$M2_DUT_HOST_SCALAR_PARSER_PARITY" = 'PASS'
    test "$M2_DUT_TRANSLATION_UNIT_LINKAGE" = 'PASS'

    test "$M2_DUT_PS2IP_SHA256" = \
        'b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74'

    test "$M2_DUT_HARDWARE_STATUS" = 'NOT_REQUIRED'
    test "$M2_DUT_HARDWARE_RUN" = 'NO'

    test "$(
        sha256sum "$M2_DUT_ELF_PATH" |
        awk '{print $1}'
    )" = "$M2_DUT_ELF_SHA256"

    test "$(
        sha256sum "$M2_DUT_EVIDENCE/SHA256SUMS.txt" |
        awk '{print $1}'
    )" = "$M2_DUT_EVIDENCE_MANIFEST_SHA256"

    (
        cd "$M2_DUT_EVIDENCE"
        sha256sum -c SHA256SUMS.txt >/dev/null
    )

    echo 'M2C_DUT_AUTHORITY=PASS'
else
    echo 'M2C_DUT_AUTHORITY=NOT_REQUIRED_YET'
fi


echo
echo '===== X. M2 COMPLETION AUTHORITY ====='

if [ "$LAST_COMPLETE_STAGE" = 'M2' ]; then
    test -f runtime/M2_COMPLETION_AUTHORITY.env
    test -f docs/M2_COMPLETION.md

    source runtime/M2_COMPLETION_AUTHORITY.env

    test "$M2_STAGE" = 'M2'
    test "$M2_STAGE_STATUS" = 'COMPLETE'
    test "$M2_COMPLETION_RESULT" = 'PASS'

    test "$M2_SOURCE_COMMIT" = \
        '58d22cba30174f92ebc50418da30080cef68d7c1'

    test "$M2_DUT_SHA256" = \
        'af245ae9f1145b5750377f83375fa9ec640f19ecd320461dd5ef80809186b84e'

    test "$M2_BUILD_RESULT" = \
        'M2C_BUILD_PASS_REPRODUCIBLE_LOW_RISK_DUT'

    test "$M2_HOST_PARITY" = 'PASS'
    test "$M2_TRANSLATION_UNIT_LINKAGE" = 'PASS'
    test "$M2_REPRODUCIBILITY" = 'BYTE_EXACT'

    test "$M2_HARDWARE_RUN" = 'NO'
    test "$M2_MACHINE_RESULT" = 'PASS'

    test "$M2_NEXT_STAGE" = 'M3'
    test "$M2_NEXT_ACTION" = \
        'M3A_plan_accelerated_extraction_waves'

    echo 'M2_COMPLETION_AUTHORITY=PASS'
else
    echo 'M2_COMPLETION_AUTHORITY=NOT_COMPLETE_STAGE'
fi

echo '===== FINAL ====='
echo 'PS_TO_VNC_MIGRATION_CHECK=PASS'
echo "LAST_COMPLETE_STAGE=$LAST_COMPLETE_STAGE"
echo "CURRENT_STAGE=$CURRENT_STAGE"
echo "CURRENT_STAGE_STATUS=$CURRENT_STAGE_STATUS"
echo "NEXT_ACTION=$NEXT_ACTION"
