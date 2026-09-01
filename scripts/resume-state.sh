#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd -P
)"

PROJECT_STATE="$ROOT/runtime/PROJECT_STATE.env"
MIGRATION_STATE="$ROOT/runtime/MIGRATION_STATE.env"

test -f "$PROJECT_STATE"
test -f "$MIGRATION_STATE"

get_field()
{
    local file="$1"
    local field="$2"

    local value
    value="$(
        awk -F= \
            -v key="$field" \
            '$1 == key {
                sub(/^[^=]*=/, "")
                print
                exit
            }' \
            "$file"
    )"

    if [ -z "$value" ]; then
        echo "ERROR=UNKNOWN_OR_EMPTY_FIELD:$field" >&2
        return 2
    fi

    printf '%s\n' "$value"
}

# Compatibility contract:
# --field retains its established migration-state meaning.
if [ "${1:-}" = '--field' ]; then
    FIELD="${2:?field name required}"
    get_field "$MIGRATION_STATE" "$FIELD"
    exit
fi

# New clean-reconstruction current-project interface.
if [ "${1:-}" = '--project-field' ]; then
    FIELD="${2:?field name required}"
    get_field "$PROJECT_STATE" "$FIELD"
    exit
fi

echo '===== PS-to-VNC RESUME STATE ====='

echo "PROJECT_NAME=$(get_field "$PROJECT_STATE" PROJECT_NAME)"
echo "PROJECT_ROOT=$ROOT"

echo
echo '===== CURRENT PROJECT STATE ====='
echo "PHASE=$(get_field "$PROJECT_STATE" PHASE)"
echo "PROJECT_INTENT=$(get_field "$PROJECT_STATE" PROJECT_INTENT)"
echo "MACHINE_CURRENT_STATE=$(get_field "$PROJECT_STATE" MACHINE_CURRENT_STATE)"
echo "NEXT_ACTION=$(get_field "$PROJECT_STATE" NEXT_ACTION)"
echo "BLOCKED_BY=$(get_field "$PROJECT_STATE" BLOCKED_BY)"

echo
echo '===== RECONSTRUCTION PROGRESS ====='
echo "PRESERVATION_AND_RESET=$(get_field "$PROJECT_STATE" PRESERVATION_AND_RESET)"
echo "SEMANTIC_AUDIT=$(get_field "$PROJECT_STATE" SEMANTIC_AUDIT)"
echo "CLEAN_PS2_RECONSTRUCTION=$(get_field "$PROJECT_STATE" CLEAN_PS2_RECONSTRUCTION)"
echo "PI_REPRODUCIBILITY_PACKAGE=$(get_field "$PROJECT_STATE" PI_REPRODUCIBILITY_PACKAGE)"
echo "GITHUB_RECONCILIATION=$(get_field "$PROJECT_STATE" GITHUB_RECONCILIATION)"

echo
echo '===== PRESERVED EXPLORATORY AUTHORITY ====='
echo "MIGRATION_STATE_ROLE=$(get_field "$PROJECT_STATE" MIGRATION_STATE_ROLE)"
echo "LAST_COMPLETE_STAGE=$(get_field "$MIGRATION_STATE" LAST_COMPLETE_STAGE)"
echo "FINAL_EXPLORATORY_STAGE=$(get_field "$MIGRATION_STATE" CURRENT_STAGE)"
echo "FINAL_EXPLORATORY_STAGE_STATUS=$(get_field "$MIGRATION_STATE" CURRENT_STAGE_STATUS)"
echo "FORMER_NEXT_ACTION=$(get_field "$MIGRATION_STATE" NEXT_ACTION)"
echo "FINAL_SOURCE_HEAD=$(get_field "$MIGRATION_STATE" CURRENT_SOURCE_HEAD)"
echo "LAST_HARDWARE_RESULT=$(get_field "$MIGRATION_STATE" LAST_HARDWARE_RESULT)"
echo "LAST_VALIDATED_WORKING_ELF_SHA256=$(get_field "$MIGRATION_STATE" LAST_VALIDATED_WORKING_ELF_SHA256)"

echo
echo '===== FROZEN BASELINE ====='
echo "BASELINE_DUT=$(get_field "$MIGRATION_STATE" BASELINE_DUT)"
echo "BASELINE_SOURCE=$(get_field "$MIGRATION_STATE" BASELINE_SOURCE)"
echo "BASELINE_SOURCE_SHA256=$(get_field "$MIGRATION_STATE" BASELINE_SOURCE_SHA256)"
echo "BASELINE_ELF_SHA256=$(get_field "$MIGRATION_STATE" BASELINE_ELF_SHA256)"
echo "DEPENDENCY_MAP=$(get_field "$MIGRATION_STATE" DEPENDENCY_MAP)"
echo "DEPENDENCY_MAP_SHA256SUMS_SHA256=$(get_field "$MIGRATION_STATE" DEPENDENCY_MAP_SHA256SUMS_SHA256)"

echo
echo '===== SAFETY BOUNDARY ====='
echo "LEGACY_ROOT=$(get_field "$MIGRATION_STATE" LEGACY_ROOT)"
echo "LEGACY_MUTABLE=$(get_field "$MIGRATION_STATE" LEGACY_MUTABLE)"
echo "DO_NOT_TOUCH=$(get_field "$MIGRATION_STATE" DO_NOT_TOUCH)"

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
echo '===== DEVELOPMENT CONTINUITY ====='
echo 'SESSION_BOOTSTRAP=AGENTS.md'
echo 'CURRENT_HUMAN_STATE=docs/status.md'
echo 'CURRENT_MACHINE_STATE=runtime/PROJECT_STATE.env'
echo 'HISTORICAL_MIGRATION_STATE=runtime/MIGRATION_STATE.env'
echo 'DOCS_ROUTER=docs/README.md'
echo 'DEVELOPMENT_POLICY=docs/development/README.md'
echo 'TESTING_POLICY=docs/development/testing.md'
echo 'TOOLING_POLICY=docs/development/tooling.md'
echo 'PROJECT_CHECK=scripts/check.sh'

echo
echo '===== END PS-to-VNC RESUME STATE ====='
