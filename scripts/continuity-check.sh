#!/usr/bin/env bash

set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.."
    pwd
)"

cd "$ROOT"

echo '===== DEVELOPMENT CONTINUITY CHECK ====='

required=(
    AGENTS.md
    CONTRIBUTING.md
    docs/README.md
    docs/status.md
    docs/development/README.md
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

MIG='runtime/MIGRATION_STATE.env'
M4='runtime/M4_SOURCE_AUTHORITY.env'
DEV='runtime/DEVELOPMENT_SYSTEM.env'
PROJECT='runtime/PROJECT_STATE.env'

test "$(get_env "$PROJECT" PROJECT_NAME)" = 'PS-to-VNC'
test "$(get_env "$PROJECT" PROJECT_STATE_ROLE)" = 'CURRENT'

test "$(get_env "$PROJECT" MACHINE_CURRENT_STATE)" = \
    'runtime/PROJECT_STATE.env'

test "$(get_env "$PROJECT" HUMAN_CURRENT_STATE)" = \
    'docs/status.md'

test "$(get_env "$PROJECT" MIGRATION_STATE_ROLE)" = \
    'HISTORICAL_REFERENCE'

test "$(get_env "$PROJECT" EXPLORATORY_MIGRATION_STATE)" = \
    'runtime/MIGRATION_STATE.env'

for field in \
    PHASE \
    MACHINE_CURRENT_STATE \
    MIGRATION_STATE_ROLE \
    RECONCILED_MAIN \
    REFERENCE_PRESERVATION \
    SEMANTIC_AUDIT \
    CLEAN_PS2_RECONSTRUCTION \
    PI_REPRODUCIBILITY_PACKAGE \
    GITHUB_RECONCILIATION \
    NEXT_ACTION \
    BLOCKED_BY
do
    value="$(get_env "$PROJECT" "$field")"
    grep -q "${field}=${value}" docs/status.md
done

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

echo 'CURRENT_PROJECT_STATE_MIRROR=PASS'
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

test "$M4_SOURCE_COMMIT" = \
    "$MIG_SOURCE_COMMIT"

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

echo 'M4_CURRENT_AUTHORITY_COHERENCE=PASS'


grep -q 'scripts/resume-state.sh' AGENTS.md
grep -q 'scripts/check.sh' AGENTS.md
grep -q 'uncommitted work' AGENTS.md
grep -q 'canonical saved' AGENTS.md
grep -q 'last proven result' AGENTS.md
grep -q 'next intended test' AGENTS.md

echo 'BOOTSTRAP_RULES=PASS'

grep -q 'PS2VNC' docs/development/README.md
grep -q 'PS-to-VNC' docs/development/README.md
grep -q 'Real-project agitation' docs/development/README.md

echo 'AGITATION_MODEL=PASS'

grep -q '/home/ps2/ps2vnc/scripts/testkit/' \
    docs/development/tooling.md

echo 'LEGACY_TESTKIT_ROUTING=PASS'

git diff --check -- \
    AGENTS.md \
    CONTRIBUTING.md \
    docs/README.md \
    docs/status.md \
    docs/development \
    docs/adr \
    runtime/DEVELOPMENT_SYSTEM.env \
    scripts/continuity-check.sh

echo 'DEVELOPMENT_CONTINUITY_CHECK=PASS'
