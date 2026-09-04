#!/usr/bin/env bash
set -euo pipefail

ROOT="$(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
    pwd -P
)"

cd "$ROOT"

echo '===== PS-to-VNC DOCUMENTATION CHECK ====='

for FILE in \
    START_HERE.md \
    docs/INDEX.md \
    docs/PROJECT_IDENTITY.md \
    docs/PROJECT_STATE.md \
    docs/MIGRATION_STATE.md \
    docs/LEGACY_HANDOFF.md \
    docs/MODULARIZATION.md \
    docs/ROADMAP.md \
    docs/architecture/ARCHITECTURE.md \
    docs/reference/FILE_AND_SERVICE_MAP.md
do
    test -f "$FILE"
    echo "PRESENT=$FILE"
done

echo
echo '===== DOCUMENT INDEX ====='

# docs/README.md is the curated documentation router. docs/INDEX.md has a
# different job: it is the exhaustive inventory that makes every Markdown
# document mechanically discoverable, including historical/reference material.
#
# Report the complete missing set in one pass so the operator can repair the
# index coherently instead of discovering missing entries one at a time.
UNINDEXED_DOCUMENTS=()

while IFS= read -r FILE
do
    [ "$FILE" = 'docs/INDEX.md' ] && continue

    if ! grep -Fq "$FILE" docs/INDEX.md; then
        UNINDEXED_DOCUMENTS+=("$FILE")
    fi
done < <(
    find docs \
        -type f \
        -name '*.md' \
        -print |
    sort
)

if [ "${#UNINDEXED_DOCUMENTS[@]}" -ne 0 ]; then
    for FILE in "${UNINDEXED_DOCUMENTS[@]}"
    do
        echo "UNINDEXED_DOCUMENT=$FILE"
    done

    echo "ERROR=DOCUMENT_INDEX_INCOMPLETE"
    echo "MISSING_DOCUMENT_COUNT=${#UNINDEXED_DOCUMENTS[@]}"
    echo "INDEX=docs/INDEX.md"
    echo "INDEX_ROLE=EXHAUSTIVE_DOCUMENT_INVENTORY"
    echo "REQUIRED_ACTION=Add each UNINDEXED_DOCUMENT to docs/INDEX.md in the appropriate section."
    echo "NOTE=docs/README.md is the curated current-authority router; docs/INDEX.md is the exhaustive inventory."
    exit 30
fi

echo 'ALL_DOCUMENTS_INDEXED=PASS'

echo
echo '===== RECOVERY REFERENCES ====='

grep -Fq 'START_HERE.md' docs/INDEX.md
grep -Fq 'docs/MIGRATION_STATE.md' docs/INDEX.md
grep -Fq 'runtime/MIGRATION_STATE.env' docs/INDEX.md
grep -Fq 'scripts/resume-state.sh' docs/INDEX.md

echo 'RECOVERY_REFERENCES=PASS'

echo
echo '===== FINAL ====='
echo 'PS_TO_VNC_DOCS_CHECK=PASS'
