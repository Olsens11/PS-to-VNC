#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd -P
)"

STATE="$ROOT/runtime/MIGRATION_STATE.env"

test -f "$STATE"

if [ "${1:-}" = '--field' ]; then
    FIELD="${2:?field name required}"

    VALUE="$(
        awk -F= \
            -v key="$FIELD" \
            '$1 == key {
                sub(/^[^=]*=/, "")
                print
                exit
            }' \
            "$STATE"
    )"

    if [ -z "$VALUE" ]; then
        echo "ERROR=UNKNOWN_OR_EMPTY_FIELD:$FIELD" >&2
        exit 2
    fi

    printf '%s\n' "$VALUE"
    exit 0
fi

# shellcheck disable=SC1090
source "$STATE"

echo '===== PS-to-VNC RESUME STATE ====='

echo "PROJECT_NAME=$PROJECT_NAME"
echo "PROJECT_ROOT=$ROOT"

echo
echo '===== MIGRATION ====='
echo "LAST_COMPLETE_STAGE=$LAST_COMPLETE_STAGE"
echo "CURRENT_STAGE=$CURRENT_STAGE"
echo "CURRENT_STAGE_STATUS=$CURRENT_STAGE_STATUS"
echo "NEXT_ACTION=$NEXT_ACTION"
echo "BLOCKED_BY=$BLOCKED_BY"

echo
echo '===== SOURCE / VALIDATION ====='
echo "CURRENT_WORKING_SOURCE=$CURRENT_WORKING_SOURCE"
echo "CURRENT_SOURCE_HEAD=$CURRENT_SOURCE_HEAD"
echo "CURRENT_DOC_HEAD=$CURRENT_DOC_HEAD"
echo "LAST_BUILD_RESULT=$LAST_BUILD_RESULT"
echo "LAST_HARDWARE_RESULT=$LAST_HARDWARE_RESULT"
echo "LAST_VALIDATED_WORKING_ELF_SHA256=$LAST_VALIDATED_WORKING_ELF_SHA256"

echo
echo '===== FROZEN BASELINE ====='
echo "BASELINE_DUT=$BASELINE_DUT"
echo "BASELINE_SOURCE=$BASELINE_SOURCE"
echo "BASELINE_SOURCE_SHA256=$BASELINE_SOURCE_SHA256"
echo "BASELINE_ELF_SHA256=$BASELINE_ELF_SHA256"
echo "DEPENDENCY_MAP=$DEPENDENCY_MAP"
echo "DEPENDENCY_MAP_SHA256SUMS_SHA256=$DEPENDENCY_MAP_SHA256SUMS_SHA256"

echo
echo '===== SAFETY BOUNDARY ====='
echo "LEGACY_ROOT=$LEGACY_ROOT"
echo "LEGACY_MUTABLE=$LEGACY_MUTABLE"
echo "DO_NOT_TOUCH=$DO_NOT_TOUCH"

echo
echo '===== GIT ====='

if git -C "$ROOT" rev-parse --verify HEAD >/dev/null 2>&1; then
    echo "GIT_HEAD=$(git -C "$ROOT" rev-parse HEAD)"
else
    echo 'GIT_HEAD=UNBORN'
fi

echo "GIT_BRANCH=$(git -C "$ROOT" branch --show-current)"
echo "GIT_REMOTE_COUNT=$(git -C "$ROOT" remote | wc -l)"

echo
echo '===== GIT STATUS ====='
git -C "$ROOT" status --short

echo
echo '===== END PS-to-VNC RESUME STATE ====='

# DEVELOPMENT_CONTINUITY_ROUTES_V1
echo
echo '===== DEVELOPMENT CONTINUITY ====='
echo 'SESSION_BOOTSTRAP=AGENTS.md'
echo 'CURRENT_HUMAN_STATE=docs/status.md'
echo 'DOCS_ROUTER=docs/README.md'
echo 'DEVELOPMENT_POLICY=docs/development/README.md'
echo 'TESTING_POLICY=docs/development/testing.md'
echo 'TOOLING_POLICY=docs/development/tooling.md'
echo 'PROJECT_CHECK=scripts/check.sh'
