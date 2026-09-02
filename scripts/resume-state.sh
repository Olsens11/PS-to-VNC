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

# Clean-reconstruction committed snapshot interface.
if [ "${1:-}" = '--project-field' ]; then
    FIELD="${2:?field name required}"
    get_field "$PROJECT_STATE" "$FIELD"
    exit
fi

STATE_RECORDED_AT="$(get_field "$PROJECT_STATE" STATE_RECORDED_AT)"
STATE_TEMPORAL_ROLE="$(get_field "$PROJECT_STATE" STATE_TEMPORAL_ROLE)"

if git -C "$ROOT" rev-parse --verify HEAD >/dev/null 2>&1; then
    GIT_HEAD="$(git -C "$ROOT" rev-parse HEAD)"
    GIT_BRANCH="$(git -C "$ROOT" branch --show-current)"
    GIT_HEAD_COMMITTED_AT="$(git -C "$ROOT" show -s --format=%cI HEAD)"
    NEWER_LOCAL_COMMITS="$(
        git -C "$ROOT" rev-list --count --all --since="$STATE_RECORDED_AT" 2>/dev/null ||
        printf 'UNKNOWN\n'
    )"
else
    GIT_HEAD='UNBORN'
    GIT_BRANCH=''
    GIT_HEAD_COMMITTED_AT='UNKNOWN'
    NEWER_LOCAL_COMMITS='UNKNOWN'
fi

echo '===== PS-to-VNC RESUME STATE ====='
echo "PROJECT_NAME=$(get_field "$PROJECT_STATE" PROJECT_NAME)"
echo "PROJECT_ROOT=$ROOT"

echo
echo '===== COMMITTED PROJECT STATE SNAPSHOT ====='
echo "STATE_TEMPORAL_ROLE=$STATE_TEMPORAL_ROLE"
echo "STATE_RECORDED_AT=$STATE_RECORDED_AT"
echo "STATE_FRESHNESS=$(get_field "$PROJECT_STATE" STATE_FRESHNESS)"
echo "STATE_FRESHNESS_POLICY=$(get_field "$PROJECT_STATE" STATE_FRESHNESS_POLICY)"
echo "PROJECT_STATE_ROLE=$(get_field "$PROJECT_STATE" PROJECT_STATE_ROLE)"
echo "PHASE_RECORDED_AT_SNAPSHOT=$(get_field "$PROJECT_STATE" PHASE)"
echo "PROJECT_INTENT=$(get_field "$PROJECT_STATE" PROJECT_INTENT)"
echo "MACHINE_STATE_SNAPSHOT=$PROJECT_STATE"
echo "NEXT_ACTION_RECORDED_AT_SNAPSHOT=$(get_field "$PROJECT_STATE" NEXT_ACTION)"
echo "BLOCKED_BY_RECORDED_AT_SNAPSHOT=$(get_field "$PROJECT_STATE" BLOCKED_BY)"
echo 'PRESENT_TENSE_STATE_AUTHORITY=REQUIRES_GITHUB_AND_LIVE_RECONCILIATION'

echo
echo '===== SNAPSHOT FRESHNESS CONTEXT ====='
echo "CURRENT_GIT_HEAD=$GIT_HEAD"
echo "CURRENT_GIT_BRANCH=$GIT_BRANCH"
echo "CURRENT_GIT_HEAD_COMMITTED_AT=$GIT_HEAD_COMMITTED_AT"
echo "LOCAL_COMMITS_AFTER_STATE_SNAPSHOT=$NEWER_LOCAL_COMMITS"

if [ "$NEWER_LOCAL_COMMITS" != 'UNKNOWN' ] && [ "$NEWER_LOCAL_COMMITS" -gt 0 ]; then
    echo 'STATE_FRESHNESS_LOCAL=NEWER_GIT_ACTIVITY_PRESENT'
    echo 'RECORDED_NEXT_ACTION_USABLE_AS_CURRENT=NO_WITHOUT_RECONCILIATION'
else
    echo 'STATE_FRESHNESS_LOCAL=NO_NEWER_LOCAL_COMMIT_PROVEN'
    echo 'RECORDED_NEXT_ACTION_USABLE_AS_CURRENT=NOT_PROVEN'
fi

echo
echo '===== RECONSTRUCTION PROGRESS RECORDED AT SNAPSHOT ====='
echo "REFERENCE_PRESERVATION=$(get_field "$PROJECT_STATE" REFERENCE_PRESERVATION)"
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
echo '===== GIT STATUS ====='
git -C "$ROOT" status --short

echo
echo '===== DEVELOPMENT CONTINUITY ====='
echo 'SESSION_BOOTSTRAP=AGENTS.md'
echo 'HUMAN_STATE_SNAPSHOT=docs/status.md'
echo 'MACHINE_STATE_SNAPSHOT=runtime/PROJECT_STATE.env'
echo 'TEMPORAL_STATE_POLICY=docs/development/TEMPORAL_STATE_SEMANTICS.md'
echo 'BRANCH_WORKSTREAM_INDEX=docs/development/BRANCH_WORKSTREAM_INDEX.md'
echo 'CONTINUITY_FOLLOWUPS=docs/development/CONTINUITY_FOLLOWUPS.md'
echo 'HISTORICAL_MIGRATION_STATE=runtime/MIGRATION_STATE.env'
echo 'DOCS_ROUTER=docs/README.md'
echo 'DEVELOPMENT_POLICY=docs/development/README.md'
echo 'TESTING_POLICY=docs/development/testing.md'
echo 'TOOLING_POLICY=docs/development/tooling.md'
echo 'PROJECT_CHECK=scripts/check.sh'

echo
echo '===== END PS-to-VNC RESUME STATE ====='
