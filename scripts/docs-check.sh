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
# different job: it is the exhaustive inventory that makes maintained Markdown
# authorities and reference documents mechanically discoverable.
#
# Immutable ledge shift records are intentionally different. Their authoritative
# discovery mechanism is the docs/ledge/work-log/ directory plus its README
# contract; adding every append-only shift record to the shared docs/INDEX.md
# would recreate the write-contention point that the immutable-log design was
# introduced to remove. Index the work-log contract itself, but validate each
# individual record through scripts/work-log-check.py instead.
#
# Report the complete missing maintained-document set in one pass so the
# operator can repair the index coherently instead of discovering entries one at
# a time.
UNINDEXED_DOCUMENTS=()

while IFS= read -r FILE
do
    [ "$FILE" = 'docs/INDEX.md' ] && continue

    case "$FILE" in
        docs/ledge/work-log/*.md)
            [ "$FILE" = 'docs/ledge/work-log/README.md' ] || continue
            ;;
    esac

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
    echo "INDEX_ROLE=EXHAUSTIVE_MAINTAINED_DOCUMENT_INVENTORY"
    echo "REQUIRED_ACTION=Add each UNINDEXED_DOCUMENT to docs/INDEX.md in the appropriate section."
    echo "NOTE=Immutable docs/ledge/work-log shift records are validated by work-log-check.py and intentionally excluded from shared-index churn."
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
