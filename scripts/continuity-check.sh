#!/usr/bin/env bash

set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.."
    pwd
)"

cd "$ROOT"

echo '===== DEVELOPMENT CONTINUITY CHECK ====='

required=(
    README.md
    START_HERE.md
    AGENTS.md
    CONTRIBUTING.md
    docs/README.md
    docs/INDEX.md
    docs/status.md
    docs/reference/CURRENT_TOOL_AND_CONTEXT_MAP.md
    docs/development/README.md
    docs/development/SESSION_RECONSTRUCTION.md
    docs/development/TEMPORAL_STATE_SEMANTICS.md
    docs/development/BRANCH_LIFECYCLE.md
    docs/development/BRANCH_WORKSTREAM_INDEX.md
    docs/development/CONTINUITY_FOLLOWUPS.md
    docs/development/PS2VNC_CONTINUITY_HARVEST.md
    docs/development/documentation.md
    docs/development/testing.md
    docs/development/tooling.md
    docs/development/lessons-learned.md
    docs/adr/README.md
    docs/adr/0001-development-continuity-baseline.md
    runtime/DEVELOPMENT_SYSTEM.env
    runtime/PROJECT_STATE.env
)

for path in "${required[@]}"
do
    test -f "$path"
done

get_env()
{
    local file="$1"
    local key="$2"

    local count
    count="$(
        awk -F= -v key="$key" \
            '$1==key{n++} END{print n+0}' \
            "$file"
    )"

    test "$count" -eq 1

    awk -F= -v key="$key" \
        '$1==key{sub(/^[^=]*=/,""); print}' \
        "$file"
}

require_iso8601_timestamp()
{
    local value="$1"

    printf '%s\n' "$value" |
        grep -Eq '^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}(Z|[+-][0-9]{2}:[0-9]{2})$'
}

MIG='runtime/MIGRATION_STATE.env'
M4='runtime/M4_SOURCE_AUTHORITY.env'
DEV='runtime/DEVELOPMENT_SYSTEM.env'
PROJECT='runtime/PROJECT_STATE.env'

# Committed mutable state is a timestamped snapshot, not timeless live
# authority. Durable historical identities still receive exact checks below.
test "$(get_env "$PROJECT" PROJECT_STATE_VERSION)" = '2'
test "$(get_env "$PROJECT" PROJECT_NAME)" = 'PS-to-VNC'
test "$(get_env "$PROJECT" STATE_TEMPORAL_ROLE)" = 'SNAPSHOT'
require_iso8601_timestamp "$(get_env "$PROJECT" STATE_RECORDED_AT)"
test "$(get_env "$PROJECT" STATE_FRESHNESS_POLICY)" = \
    'docs/development/TEMPORAL_STATE_SEMANTICS.md'
test "$(get_env "$PROJECT" PROJECT_STATE_ROLE)" = \
    'CURRENT_AT_RECORDED_TIME'

test "$(get_env "$PROJECT" MACHINE_CURRENT_STATE)" = \
    'runtime/PROJECT_STATE.env'

test "$(get_env "$PROJECT" HUMAN_CURRENT_STATE)" = \
    'docs/status.md'

test "$(get_env "$PROJECT" MIGRATION_STATE_ROLE)" = \
    'HISTORICAL_REFERENCE'

test "$(get_env "$PROJECT" EXPLORATORY_MIGRATION_STATE)" = \
    'runtime/MIGRATION_STATE.env'

grep -q 'Temporal role: `SNAPSHOT`' docs/status.md
STATUS_RECORDED_AT="$(
    sed -n 's/^Recorded at: `\([^`]*\)`$/\1/p' docs/status.md |
    head -n 1
)"
test -n "$STATUS_RECORDED_AT"
require_iso8601_timestamp "$STATUS_RECORDED_AT"
grep -q 'TEMPORAL_STATE_SEMANTICS.md' docs/status.md

grep -q 'Status: `BRANCH_INVENTORY_SNAPSHOT`' \
    docs/development/BRANCH_WORKSTREAM_INDEX.md
grep -q 'Temporal role: `SNAPSHOT`' \
    docs/development/BRANCH_WORKSTREAM_INDEX.md
BRANCH_RECORDED_AT="$(
    sed -n 's/^Recorded at: `\([^`]*\)`$/\1/p' \
        docs/development/BRANCH_WORKSTREAM_INDEX.md |
    head -n 1
)"
test -n "$BRANCH_RECORDED_AT"
require_iso8601_timestamp "$BRANCH_RECORDED_AT"

grep -q 'Temporal role: `SNAPSHOT_BACKLOG`' \
    docs/development/CONTINUITY_FOLLOWUPS.md
FOLLOWUPS_RECORDED_AT="$(
    sed -n 's/^Recorded at: `\([^`]*\)`$/\1/p' \
        docs/development/CONTINUITY_FOLLOWUPS.md |
    head -n 1
)"
test -n "$FOLLOWUPS_RECORDED_AT"
require_iso8601_timestamp "$FOLLOWUPS_RECORDED_AT"

echo 'TEMPORAL_STATE_CONTRACT=PASS'

# Durable exploratory/provenance identity remains exact even though mutable
# NEXT/CURRENT fields are no longer mirrored as timeless present-tense truth.
test "$(get_env "$PROJECT" EXPLORATORY_FINAL_SOURCE_HEAD)" = \
    "$(get_env "$MIG" CURRENT_SOURCE_HEAD)"

test "$(get_env "$PROJECT" EXPLORATORY_FINAL_HARDWARE_HEAD)" = \
    "$(get_env "$M4" M4I_FINAL_HARDWARE_HEAD)"

test "$(get_env "$PROJECT" EXPLORATORY_FINAL_ELF_SHA256)" = \
    "$(get_env "$MIG" LAST_VALIDATED_WORKING_ELF_SHA256)"

test "$(get_env "$PROJECT" EXPLORATORY_FINAL_PT_LOAD_SHA256)" = \
    "$(get_env "$M4" LAST_VALIDATED_PT_LOAD_SHA256)"

test "$(get_env "$PROJECT" EXPLORATORY_FORMER_NEXT_ACTION)" = \
    "$(get_env "$MIG" NEXT_ACTION)"

echo 'EXPLORATORY_STATE_PRESERVATION=PASS'

test "$(get_env "$DEV" SESSION_BOOTSTRAP)" = 'AGENTS.md'
test "$(get_env "$DEV" DOCS_ROUTER)" = 'docs/README.md'
test "$(get_env "$DEV" HUMAN_CURRENT_STATE)" = 'docs/status.md'

test "$(get_env "$DEV" MACHINE_CURRENT_STATE)" = \
    'runtime/PROJECT_STATE.env'

test "$(get_env "$DEV" HISTORICAL_MIGRATION_STATE)" = \
    'runtime/MIGRATION_STATE.env'

test "$(get_env "$DEV" CANONICAL_TOOL_REUSE)" = 'REQUIRED'
test "$(get_env "$DEV" REAL_PROJECT_AGITATION)" = 'ENABLED'

echo 'DEVELOPMENT_SYSTEM_CONTRACT=PASS'

M4_SOURCE_COMMIT="$(
    get_env "$M4" CURRENT_SOURCE_COMMIT
)"

MIG_SOURCE_COMMIT="$(
    get_env "$MIG" CURRENT_SOURCE_HEAD
)"

test "$M4_SOURCE_COMMIT" = "$MIG_SOURCE_COMMIT"

M4_HW_HEAD="$(
    get_env "$M4" M4I_FINAL_HARDWARE_HEAD
)"

test "$(
    get_env "$M4" CURRENT_HARDWARE_AUTHORITY_COMMIT
)" = "$M4_HW_HEAD"

git cat-file -e "$M4_HW_HEAD^{commit}"

test "$(
    get_env "$M4" LAST_DIRECT_HARDWARE_AUTHORITY
)" = 'M4I-FINAL'

test "$(
    get_env "$M4" LAST_DIRECT_HARDWARE_RESULT
)" = 'M4I_FINAL_PASS_MACHINE_AND_PHYSICAL'

test "$(
    get_env "$M4" NEXT_ACTION
)" = "$(
    get_env "$MIG" NEXT_ACTION
)"

test "$(
    get_env "$M4" BLOCKED_BY
)" = "$(
    get_env "$MIG" BLOCKED_BY
)"

if [ "$(
    get_env "$M4" CURRENT_WORKING_ELF_STATUS
)" = 'HARDWARE_QUALIFIED' ]
then
    test "$(
        get_env "$M4" CURRENT_WORKING_ELF_SHA256
    )" = "$(
        get_env "$MIG" LAST_VALIDATED_WORKING_ELF_SHA256
    )"

    test "$(
        get_env "$M4" CURRENT_WORKING_PT_LOAD_SHA256
    )" = "$(
        get_env "$M4" LAST_VALIDATED_PT_LOAD_SHA256
    )"

    test "$(
        get_env "$M4" CURRENT_HARDWARE_QUALIFICATION
    )" = 'PASS_MACHINE_AND_PHYSICAL'
fi

test "$(
    get_env "$M4" M4I_FINAL_HARDWARE_QUALIFIED
)" = 'YES'

echo 'M4_HISTORICAL_AUTHORITY_COHERENCE=PASS'

# There must be one obvious new-session entrance even though compatibility
# entry points remain for migration-era tools.
grep -q 'AGENTS.md' README.md
grep -q 'Canonical development-session bootstrap: `AGENTS.md`' START_HERE.md
grep -q 'CANONICAL_SESSION_BOOTSTRAP=AGENTS.md' START_HERE.md
grep -q 'scripts/resume-state.sh' AGENTS.md
grep -q 'scripts/check.sh' AGENTS.md
grep -q 'TEMPORAL_STATE_SEMANTICS.md' AGENTS.md
grep -q 'SESSION_RECONSTRUCTION.md' AGENTS.md
grep -q 'uncommitted work' AGENTS.md
grep -q 'canonical saved' AGENTS.md
grep -q 'last proven result' AGENTS.md
grep -q 'next intended test' AGENTS.md

grep -q 'CURRENT_TOOL_AND_CONTEXT_MAP.md' docs/README.md
grep -q 'CURRENT_TOOL_AND_CONTEXT_MAP.md' docs/INDEX.md
grep -q 'PS2VNC_CONTINUITY_HARVEST.md' docs/README.md
grep -q 'PS2VNC_CONTINUITY_HARVEST.md' docs/INDEX.md

echo 'BOOTSTRAP_AND_CONTEXT_ROUTING=PASS'

grep -q 'PS2VNC' docs/development/README.md
grep -q 'PS-to-VNC' docs/development/README.md
grep -q 'Real-project agitation' docs/development/README.md

echo 'AGITATION_MODEL=PASS'

grep -q '/home/ps2/ps2vnc/scripts/testkit/' \
    docs/development/tooling.md

echo 'LEGACY_TESTKIT_ROUTING=PASS'

grep -q 'explicit user approval' \
    docs/development/BRANCH_LIFECYCLE.md
grep -q 'cold storage' \
    docs/development/BRANCH_LIFECYCLE.md

echo 'BRANCH_LIFECYCLE_CONTRACT=PASS'

git diff --check -- \
    README.md \
    START_HERE.md \
    AGENTS.md \
    CONTRIBUTING.md \
    docs/README.md \
    docs/INDEX.md \
    docs/status.md \
    docs/development \
    docs/reference/CURRENT_TOOL_AND_CONTEXT_MAP.md \
    docs/reference/FILE_AND_SERVICE_MAP.md \
    docs/adr \
    runtime/DEVELOPMENT_SYSTEM.env \
    runtime/PROJECT_STATE.env \
    scripts/continuity-check.sh \
    scripts/resume-state.sh

echo 'DEVELOPMENT_CONTINUITY_CHECK=PASS'
