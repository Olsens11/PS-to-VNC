#!/usr/bin/env bash

# PS-to-VNC current development-continuity consistency check.
#
# This tool validates the active repository-centered development contract:
# canonical entry points, current human/machine state, development policy, and
# bootstrap/tool-reuse rules.
#
# Completed migration stages, M4 hardware history, and legacy TestKit topology
# are historical/evidence responsibilities and are deliberately not re-audited
# here.

set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.."
    pwd
)"

cd "$ROOT"

echo '===== DEVELOPMENT CONTINUITY CHECK ====='


fail()
{
    local error="$1"
    shift

    echo "ERROR=$error"

    while [ "$#" -gt 0 ]
    do
        echo "$1"
        shift
    done

    exit 40
}


require_file()
{
    local path="$1"

    test -f "$path" ||
        fail \
            "MISSING_CONTINUITY_FILE:$path" \
            "REQUIRED_ACTION=Restore or deliberately update the current continuity contract."
}


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

    if [ "$count" -ne 1 ]; then
        fail \
            "INVALID_MACHINE_STATE_FIELD:$file:$key" \
            "MATCH_COUNT=$count" \
            "REQUIRED_ACTION=Keep exactly one authoritative value for this current-state field."
    fi

    awk -F= -v key="$key" \
        '$1==key{sub(/^[^=]*=/,""); print}' \
        "$file"
}


require_env()
{
    local file="$1"
    local key="$2"
    local expected="$3"
    local actual

    actual="$(get_env "$file" "$key")"

    if [ "$actual" != "$expected" ]; then
        fail \
            "MACHINE_STATE_MISMATCH:$file:$key" \
            "EXPECTED=$expected" \
            "ACTUAL=$actual"
    fi
}


require_status_field()
{
    local field="$1"
    local value

    value="$(get_env runtime/PROJECT_STATE.env "$field")"

    grep -Fq "${field}=${value}" docs/status.md ||
        fail \
            "CURRENT_STATUS_MIRROR_MISSING:$field" \
            "EXPECTED=${field}=${value}" \
            "REQUIRED_ACTION=Reconcile runtime/PROJECT_STATE.env with docs/status.md; do not promote historical-only fields into current status."
}


require_text()
{
    local needle="$1"
    local file="$2"
    local label="$3"

    grep -Fq "$needle" "$file" ||
        fail \
            "CONTINUITY_RULE_MISSING:$label" \
            "FILE=$file" \
            "EXPECTED_TEXT=$needle"
}


echo
echo '===== CURRENT CONTINUITY SURFACES ====='

required=(
    AGENTS.md
    CONTRIBUTING.md
    docs/README.md
    docs/status.md
    docs/PROJECT_INTENT.md
    docs/CLEAN_ARCHITECTURE.md
    docs/development/README.md
    docs/development/documentation.md
    docs/development/testing.md
    docs/development/tooling.md
    docs/development/lessons-learned.md
    docs/adr/README.md
    docs/adr/0001-development-continuity-baseline.md
    runtime/DEVELOPMENT_SYSTEM.env
    runtime/PROJECT_STATE.env
    scripts/check.sh
    scripts/docs-check.sh
    scripts/continuity-check.sh
)

for path in "${required[@]}"
do
    require_file "$path"
    echo "PRESENT=$path"
done

echo 'CURRENT_CONTINUITY_SURFACES=PASS'


echo
echo '===== CURRENT PROJECT STATE CONTRACT ====='

PROJECT='runtime/PROJECT_STATE.env'

require_env "$PROJECT" PROJECT_NAME 'PS-to-VNC'
require_env "$PROJECT" PROJECT_STATE_ROLE 'CURRENT'
require_env "$PROJECT" PROJECT_INTENT 'docs/PROJECT_INTENT.md'
require_env "$PROJECT" HUMAN_CURRENT_STATE 'docs/status.md'
require_env "$PROJECT" MACHINE_CURRENT_STATE 'runtime/PROJECT_STATE.env'
require_env "$PROJECT" CLEAN_ARCHITECTURE 'docs/CLEAN_ARCHITECTURE.md'
require_env "$PROJECT" MIGRATION_STATE_ROLE 'HISTORICAL_REFERENCE'

for field in \
    PHASE \
    MACHINE_CURRENT_STATE \
    MIGRATION_STATE_ROLE \
    REFERENCE_PRESERVATION \
    SEMANTIC_AUDIT \
    CLEAN_PS2_RECONSTRUCTION \
    PI_REPRODUCIBILITY_PACKAGE \
    GITHUB_RECONCILIATION \
    NEXT_ACTION \
    BLOCKED_BY
do
    require_status_field "$field"
done

echo 'CURRENT_PROJECT_STATE_MIRROR=PASS'


echo
echo '===== DEVELOPMENT SYSTEM CONTRACT ====='

DEV='runtime/DEVELOPMENT_SYSTEM.env'

require_env "$DEV" DEVELOPMENT_SYSTEM_STATUS 'ACTIVE'
require_env "$DEV" SESSION_BOOTSTRAP 'AGENTS.md'
require_env "$DEV" CONTRIBUTING_GUIDE 'CONTRIBUTING.md'
require_env "$DEV" DOCS_ROUTER 'docs/README.md'
require_env "$DEV" HUMAN_CURRENT_STATE 'docs/status.md'
require_env "$DEV" MACHINE_CURRENT_STATE 'runtime/PROJECT_STATE.env'
require_env "$DEV" DEVELOPMENT_POLICY 'docs/development/README.md'
require_env "$DEV" DOCUMENTATION_POLICY 'docs/development/documentation.md'
require_env "$DEV" TESTING_POLICY 'docs/development/testing.md'
require_env "$DEV" TOOLING_POLICY 'docs/development/tooling.md'
require_env "$DEV" LESSONS_LEARNED 'docs/development/lessons-learned.md'
require_env "$DEV" DECISION_RECORD_ROOT 'docs/adr'
require_env "$DEV" CURRENT_PROJECT_ADAPTER 'PS-to-VNC'
require_env "$DEV" REAL_PROJECT_AGITATION 'ENABLED'
require_env "$DEV" CANONICAL_TOOL_REUSE 'REQUIRED'
require_env "$DEV" PORTABLE_FRAMEWORK_STATUS 'EMBEDDED_NOT_EXTRACTED'
require_env "$DEV" LEGACY_TESTKIT_INHERITANCE 'RETIRED'

echo 'DEVELOPMENT_SYSTEM_CONTRACT=PASS'


echo
echo '===== BOOTSTRAP RULES ====='

require_text \
    'scripts/resume-state.sh' \
    AGENTS.md \
    RESUME_TOOL

require_text \
    'scripts/check.sh' \
    AGENTS.md \
    PROJECT_CHECK

require_text \
    'uncommitted work' \
    AGENTS.md \
    DIRTY_STATE_PROTECTION

require_text \
    'canonical saved' \
    AGENTS.md \
    CANONICAL_TOOL_REUSE

require_text \
    'last proven result' \
    AGENTS.md \
    TEST_CONTINUITY

require_text \
    'next intended test' \
    AGENTS.md \
    TEST_CONTINUITY

echo 'BOOTSTRAP_RULES=PASS'


echo
echo '===== DEVELOPMENT MODEL ====='

require_text \
    'PS2VNC' \
    docs/development/README.md \
    PROJECT_HISTORY

require_text \
    'PS-to-VNC' \
    docs/development/README.md \
    CURRENT_PROJECT

require_text \
    'Real-project agitation' \
    docs/development/README.md \
    REAL_PROJECT_AGITATION

require_text \
    'must not depend on their checkout' \
    docs/development/tooling.md \
    LEGACY_TESTKIT_RETIREMENT

echo 'DEVELOPMENT_MODEL=PASS'
echo 'LEGACY_TESTKIT_INHERITANCE=RETIRED'


echo
echo '===== DIFF INTEGRITY ====='

git diff --check -- \
    AGENTS.md \
    CONTRIBUTING.md \
    docs/README.md \
    docs/status.md \
    docs/development \
    docs/adr \
    runtime/DEVELOPMENT_SYSTEM.env \
    runtime/PROJECT_STATE.env \
    scripts/continuity-check.sh

echo 'CONTINUITY_DIFF_INTEGRITY=PASS'


echo
echo '===== FINAL ====='
echo 'DEVELOPMENT_CONTINUITY_CHECK=PASS'
