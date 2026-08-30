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
echo '===== FINAL ====='
echo 'PS_TO_VNC_MIGRATION_CHECK=PASS'
echo "LAST_COMPLETE_STAGE=$LAST_COMPLETE_STAGE"
echo "CURRENT_STAGE=$CURRENT_STAGE"
echo "CURRENT_STAGE_STATUS=$CURRENT_STAGE_STATUS"
echo "NEXT_ACTION=$NEXT_ACTION"
