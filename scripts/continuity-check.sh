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
    docs/development/source-naming-and-symbols.md
    docs/development/source-topology.md
    docs/development/documentation.md
    docs/development/testing.md
    docs/development/tooling.md
    docs/development/lessons-learned.md
    docs/adr/README.md
    docs/adr/0001-development-continuity-baseline.md
    docs/reference/SOURCE_SYMBOL_DICTIONARIES.md
    runtime/DEVELOPMENT_SYSTEM.env
    runtime/PROJECT_STATE.env
    scripts/check.sh
    scripts/docs-check.sh
    scripts/continuity-check.sh
    scripts/check-clean-ps2-compile.sh
    scripts/check-historical-issue7-reproducibility.sh
)

for path in "${required[@]}"
do
    require_file "$path"
    echo "PRESENT=$path"
done

echo 'CURRENT_CONTINUITY_SURFACES=PASS'


echo
echo '===== EXECUTABLE DEVELOPMENT TOOL MODES ====='

for executable in \
    scripts/check.sh \
    scripts/docs-check.sh \
    scripts/continuity-check.sh \
    scripts/resume-state.sh \
    scripts/check-clean-ps2-compile.sh \
    scripts/build-issue7-clean.sh \
    scripts/check-issue7-linked-reproducibility.sh \
    scripts/check-historical-issue7-reproducibility.sh \
    scripts/testkit/self-test.sh
do
    if [ ! -x "$executable" ]; then
        echo "NOT_EXECUTABLE=$executable" >&2
        exit 1
    fi

    echo "EXECUTABLE=$executable"
done

echo 'EXECUTABLE_DEVELOPMENT_TOOL_MODES=PASS'


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
require_env "$PROJECT" CURRENT_RECONSTRUCTION_STAGE 'POST_ISSUE39_PRE_ISSUE40_PI_DESKTOP_PREREQUISITE'
require_env "$PROJECT" ISSUE39_STATUS 'CLOSED_COMPLETED'
require_env "$PROJECT" SOURCE_TOPOLOGY_STATUS 'ACTIVE_DOMAIN_DIRECTORIES'
require_env "$PROJECT" SOURCE_TOPOLOGY_ADOPTED_DATE '2026-09-05'
require_env "$PROJECT" SOURCE_TOPOLOGY_POLICY 'docs/development/source-topology.md'
require_env "$PROJECT" SOURCE_TOPOLOGY_RECORD 'docs/reconstruction/2026-09-05_SOURCE_TOPOLOGY_ADOPTION.md'
require_env "$PROJECT" SOURCE_DICTIONARY_PORTAL 'docs/reference/SOURCE_SYMBOL_DICTIONARIES.md'

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
require_env "$DEV" TOOL_INVOCATION_RULE 'PURPOSE_APPLICABILITY_EVIDENCE_NEED_BEFORE_INVOCATION'
require_env "$DEV" TOOL_PORTABILITY_RULE 'GENERALIZE_PORTABILITY_PRESERVE_PURPOSE'
require_env "$DEV" TOOL_DICTIONARY_ROLE 'INVENTORY_NOT_INVOCATION_LIST'
require_env "$DEV" TOOL_SELECTION_SEQUENCE 'INVENTORY_APPLICABILITY_EVIDENCE_NEED_INVOCATION'
require_env "$DEV" SOURCE_TOPOLOGY_POLICY 'docs/development/source-topology.md'
require_env "$DEV" ISSUE7_STAGE_LOCAL_REPRO_TOOL 'scripts/check-issue7-linked-reproducibility.sh'
require_env "$DEV" ISSUE7_STAGE_LOCAL_REPRO_AUTHORITY 'd94c9280035e288ccbec692ea21e89d3ffb4ffec'
require_env "$DEV" ISSUE7_STAGE_LOCAL_REPRO_OUTSIDE_STAGE 'SKIP_NOT_APPLICABLE_EXIT_0_NO_BUILD'
require_env "$DEV" ISSUE7_LATER_STAGE_REPRO_TOOL 'scripts/check-historical-issue7-reproducibility.sh'
require_env "$DEV" ISSUE7_LATER_STAGE_REPRO_REQUIREMENT 'EXPLICIT_PASS_REQUIRED_WHEN_ISSUE7_EVIDENCE_IS_REQUESTED'
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

require_text \
    'source-topology.md' \
    AGENTS.md \
    SOURCE_TOPOLOGY_BOOTSTRAP

require_text \
    'SKIPPED` / `NOT_APPLICABLE`' \
    AGENTS.md \
    ISSUE7_STAGE_LOCAL_AGENT_SKIP_POLICY

require_text \
    'Issue #7 stage-local reproducibility command' \
    START_HERE.md \
    ISSUE7_STAGE_LOCAL_START_HERE_POLICY

require_text \
    'ISSUE7_LINKED_REPRODUCIBILITY_APPLICABILITY=NOT_APPLICABLE' \
    START_HERE.md \
    ISSUE7_STAGE_LOCAL_START_HERE_APPLICABILITY

require_text \
    'ISSUE7_LINKED_REPRODUCIBILITY=SKIPPED' \
    START_HERE.md \
    ISSUE7_STAGE_LOCAL_START_HERE_SKIP_RESULT

require_text \
    'ISSUE7_LINKED_REPRODUCIBILITY_BUILD_RUN=NO' \
    START_HERE.md \
    ISSUE7_STAGE_LOCAL_START_HERE_NO_BUILD

require_text \
    'ISSUE7_LINKED_REPRODUCIBILITY_WORKFLOW_BLOCKING=NO' \
    START_HERE.md \
    ISSUE7_STAGE_LOCAL_START_HERE_NONBLOCKING

require_text \
    'successful no-op' \
    docs/development/tooling.md \
    ISSUE7_STAGE_LOCAL_TOOLING_SKIP_POLICY

require_text \
    '**Issue #7 operational note:**' \
    scripts/SYMBOLS.md \
    ISSUE7_SCRIPTS_DICTIONARY_OPERATIONAL_NOTE

require_text \
    'ISSUE7_LINKED_REPRODUCIBILITY_APPLICABILITY=NOT_APPLICABLE' \
    scripts/check-issue7-linked-reproducibility.sh \
    ISSUE7_STAGE_LOCAL_APPLICABILITY_MARKER

require_text \
    'ISSUE7_LINKED_REPRODUCIBILITY=SKIPPED' \
    scripts/check-issue7-linked-reproducibility.sh \
    ISSUE7_STAGE_LOCAL_SKIP_MARKER

require_text \
    'ISSUE7_LINKED_REPRODUCIBILITY_BUILD_RUN=NO' \
    scripts/check-issue7-linked-reproducibility.sh \
    ISSUE7_STAGE_LOCAL_NO_BUILD_MARKER

require_text \
    'ISSUE7_LINKED_REPRODUCIBILITY_WORKFLOW_BLOCKING=NO' \
    scripts/check-issue7-linked-reproducibility.sh \
    ISSUE7_STAGE_LOCAL_NONBLOCKING_MARKER

require_text \
    'check-historical-issue7-reproducibility.sh' \
    scripts/check-issue7-linked-reproducibility.sh \
    ISSUE7_STAGE_LOCAL_SAFE_WRAPPER_REDIRECT

require_text \
    'Generalize portability; preserve purpose.' \
    AGENTS.md \
    TOOL_PORTABILITY_PURPOSE_POLICY

require_text \
    'what specific claim or uncertainty' \
    AGENTS.md \
    TOOL_INVOCATION_EVIDENCE_POLICY

require_text \
    'inventory -> applicability -> evidence need -> invocation' \
    START_HERE.md \
    TOOL_SELECTION_SEQUENCE_BOOTSTRAP

require_text \
    'inventory and orientation surface, not an execution' \
    scripts/SYMBOLS.md \
    TOOL_DICTIONARY_INVENTORY_ROLE

require_text \
    'Issue #7 stage-specific compile' \
    docs/development/tooling.md \
    ISSUE7_STRICT_COMPILE_STAGE_PURPOSE

require_text \
    'Compile every clean Issue #7 C translation unit' \
    scripts/check-clean-ps2-compile.sh \
    ISSUE7_STRICT_COMPILE_ORIGINAL_SYNOPSIS

echo 'TOOL_SELECTION_APPLICABILITY_POLICY=PASS'
echo 'ISSUE7_STAGE_LOCAL_TOOL_SAFETY=PASS'
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

require_text \
    'source-topology.md' \
    docs/development/README.md \
    SOURCE_TOPOLOGY_DEVELOPMENT_POLICY

echo 'DEVELOPMENT_MODEL=PASS'
echo 'LEGACY_TESTKIT_INHERITANCE=RETIRED'


echo
echo '===== CLEAN PRODUCT SOURCE TOPOLOGY ====='

python3 - <<'__PS2VNC_TOPOLOGY_CHECK_PY_EOF__'
from pathlib import Path
import re
import subprocess
import sys

CLEAN_MARKER = "File synopsis:"

EXPECTED_DIRECTORIES = {
    "pi",
    "src",
    "src/audio",
    "src/config",
    "src/diagnostics",
    "src/display",
    "src/framebuffer",
    "src/input",
    "src/media",
    "src/mpeg",
    "src/platform",
    "src/rfb",
    "src/transport",
    "src/ui",
}

ROOT_SOURCE_ALLOWLIST = {
    "src/main.c",
    "src/app.c",
    "src/app.h",
    "src/app_mpeg_calibration.c",
    "src/app_mpeg_calibration.h",
    "src/app_mpeg_frame.c",
    "src/app_mpeg_frame.h",
    "src/app_mpeg_run.c",
    "src/app_mpeg_run.h",
}

SOURCE_SUFFIXES = {
    ".c",
    ".h",
    ".py",
    ".sh",
    ".mk",
}


def is_make_source(path: Path) -> bool:
    return (
        path.name == "Makefile"
        or path.suffix == ".mk"
    )


def split_dictionary_row(line: str) -> list[str]:
    body = line.strip().strip("|")

    return [
        cell.strip()
        .replace(r"\|", "|")
        .replace("`", "")
        for cell in re.split(
            r"(?<!\\)\|",
            body,
        )
    ]


clean_files: set[Path] = set()

for product_root in (Path("src"), Path("pi")):
    for path in sorted(product_root.rglob("*")):
        if not path.is_file():
            continue

        if (
            path.suffix not in SOURCE_SUFFIXES
            and path.name != "Makefile"
        ):
            continue

        text = path.read_text(
            encoding="utf-8",
            errors="strict",
        )

        if (
            CLEAN_MARKER in text[:2048]
            or is_make_source(path)
        ):
            clean_files.add(path)

actual_directories = {
    path.parent.as_posix()
    for path in clean_files
}

if actual_directories != EXPECTED_DIRECTORIES:
    print(
        "ERROR=SOURCE_TOPOLOGY_DIRECTORY_SET_MISMATCH"
    )
    print(
        "EXPECTED="
        + ",".join(sorted(EXPECTED_DIRECTORIES))
    )
    print(
        "ACTUAL="
        + ",".join(sorted(actual_directories))
    )
    sys.exit(41)

root_sources = {
    path.as_posix()
    for path in clean_files
    if path.parent == Path("src")
}

if root_sources != ROOT_SOURCE_ALLOWLIST:
    print(
        "ERROR=SOURCE_TOPOLOGY_ROOT_ALLOWLIST_MISMATCH"
    )
    print(
        "EXPECTED="
        + ",".join(sorted(ROOT_SOURCE_ALLOWLIST))
    )
    print(
        "ACTUAL="
        + ",".join(sorted(root_sources))
    )
    sys.exit(42)

dictionary_paths = sorted(
    list(Path("src").rglob("SYMBOLS.md")) +
    list(Path("pi").rglob("SYMBOLS.md"))
)

dictionary_directories = {
    path.parent.as_posix()
    for path in dictionary_paths
}

if dictionary_directories != EXPECTED_DIRECTORIES:
    print(
        "ERROR=SOURCE_TOPOLOGY_DICTIONARY_SET_MISMATCH"
    )
    print(
        "EXPECTED="
        + ",".join(sorted(EXPECTED_DIRECTORIES))
    )
    print(
        "ACTUAL="
        + ",".join(sorted(dictionary_directories))
    )
    sys.exit(43)

covered_files: set[Path] = set()

for dictionary in dictionary_paths:
    expected_directory = dictionary.parent.as_posix()
    in_table = False

    for line_number, line in enumerate(
        dictionary.read_text(
            encoding="utf-8",
            errors="strict",
        ).splitlines(),
        1,
    ):
        if line.startswith(
            "| Name | Kind | File | Owner |"
        ):
            in_table = True
            continue

        if not in_table:
            continue

        if line.startswith("|---"):
            continue

        if not line.startswith("| "):
            if line.strip():
                in_table = False
            continue

        cells = split_dictionary_row(line)

        if len(cells) != 7:
            print(
                "ERROR=SOURCE_TOPOLOGY_DICTIONARY_ROW_SHAPE"
            )
            print(
                f"DICTIONARY={dictionary}"
            )
            print(
                f"LINE={line_number}"
            )
            sys.exit(44)

        file_name = cells[2]
        source = Path(file_name)

        if source.parent.as_posix() != expected_directory:
            print(
                "ERROR=SOURCE_TOPOLOGY_NONLOCAL_DICTIONARY_ROW"
            )
            print(
                f"DICTIONARY={dictionary}"
            )
            print(
                f"SOURCE={file_name}"
            )
            sys.exit(45)

        covered_files.add(source)

missing_file_coverage = sorted(
    clean_files - covered_files
)

if missing_file_coverage:
    for path in missing_file_coverage:
        print(
            "SOURCE_TOPOLOGY_MISSING_LOCAL_FILE_COVERAGE="
            f"{path.as_posix()}"
        )

    sys.exit(46)

generated_portal = subprocess.run(
    [
        "python3",
        "scripts/source-dictionary.py",
        "portal",
    ],
    check=True,
    text=True,
    stdout=subprocess.PIPE,
).stdout

portal_path = Path(
    "docs/reference/SOURCE_SYMBOL_DICTIONARIES.md"
)

committed_portal = portal_path.read_text(
    encoding="utf-8"
)

if generated_portal != committed_portal:
    print(
        "ERROR=SOURCE_DICTIONARY_PORTAL_OUT_OF_DATE"
    )
    print(
        "REQUIRED_ACTION="
        "Regenerate docs/reference/SOURCE_SYMBOL_DICTIONARIES.md "
        "with source-dictionary.py portal."
    )
    sys.exit(47)

counts: dict[str, int] = {}

pattern = re.compile(
    r"^\| `(?P<directory>(?:src(?:/[^`]+)?|pi))` "
    r"\| .* \| COMPLETE \| (?P<count>[0-9]+) \|$"
)

for line in committed_portal.splitlines():
    match = pattern.match(line)

    if match:
        counts[
            match.group("directory")
        ] = int(
            match.group("count")
        )

if set(counts) != EXPECTED_DIRECTORIES:
    print(
        "ERROR=SOURCE_DICTIONARY_PORTAL_DIRECTORY_SET_MISMATCH"
    )
    sys.exit(48)

print(
    "SOURCE_TOPOLOGY_DIRECTORIES="
    + ",".join(sorted(EXPECTED_DIRECTORIES))
)

print(
    "SOURCE_TOPOLOGY_ROOT_FILES="
    + ",".join(sorted(ROOT_SOURCE_ALLOWLIST))
)

print(
    "SOURCE_TOPOLOGY_CLEAN_FILE_COUNT="
    f"{len(clean_files)}"
)

for directory in sorted(counts):
    print(
        f"SOURCE_DICTIONARY_COUNT="
        f"{directory}:{counts[directory]}"
    )

print(
    "SOURCE_DICTIONARY_TOTAL="
    f"{sum(counts.values())}"
)

print(
    "SOURCE_TOPOLOGY_ADOPTED_DATE="
    "2026-09-05"
)

print(
    "SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS"
)

print(
    "SOURCE_TOPOLOGY_CONTRACT=PASS"
)

print(
    "SOURCE_DICTIONARY_PORTAL_SYNC=PASS"
)
__PS2VNC_TOPOLOGY_CHECK_PY_EOF__

echo 'CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS'


echo
echo '===== DIFF INTEGRITY ====='

git diff --check -- \
    AGENTS.md \
    START_HERE.md \
    CONTRIBUTING.md \
    docs/README.md \
    docs/INDEX.md \
    docs/status.md \
    docs/CLEAN_ARCHITECTURE.md \
    docs/ROADMAP.md \
    docs/development \
    docs/reconstruction \
    docs/reference/FILE_AND_SERVICE_MAP.md \
    docs/adr \
    runtime/DEVELOPMENT_SYSTEM.env \
    runtime/PROJECT_STATE.env \
    scripts/continuity-check.sh \
    scripts/check-clean-ps2-compile.sh

echo 'CONTINUITY_DIFF_INTEGRITY=PASS'


echo
echo '===== FINAL ====='
echo 'DEVELOPMENT_CONTINUITY_CHECK=PASS'
