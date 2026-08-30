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

while IFS= read -r FILE
do
    [ "$FILE" = 'docs/INDEX.md' ] && continue

    if ! grep -Fq "$FILE" docs/INDEX.md; then
        echo "ERROR=UNINDEXED_DOCUMENT:$FILE"
        exit 30
    fi
done < <(
    find docs \
        -type f \
        -name '*.md' \
        -print |
    sort
)

echo 'ALL_DOCUMENTS_INDEXED=PASS'

echo
echo '===== RECOVERY REFERENCES ====='

grep -Fq 'START_HERE.md' docs/INDEX.md
grep -Fq 'docs/MIGRATION_STATE.md' docs/INDEX.md
grep -Fq 'runtime/MIGRATION_STATE.env' docs/INDEX.md
grep -Fq 'scripts/resume-state.sh' docs/INDEX.md
grep -Fq 'scripts/migration-check.sh' docs/INDEX.md

echo 'RECOVERY_REFERENCES=PASS'

echo
echo '===== MIGRATION COHERENCE ====='

scripts/migration-check.sh

echo
echo '===== FINAL ====='
echo 'PS_TO_VNC_DOCS_CHECK=PASS'
