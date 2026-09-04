#!/usr/bin/env python3
"""File synopsis:
Run disposable regression tests for the clean source-dictionary tooling.

This self-test owns only validator fixtures; it does not inspect historical
source or claim exhaustive production symbol coverage.
"""

from pathlib import Path
import ast
import hashlib
import subprocess
import tempfile

TOOL = Path(__file__).with_name("source-dictionary.py")


def write(root: Path, relative: str, text: str) -> None:
    path = root / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def run(
    root: Path,
    expected: int,
    command: str = "check",
    require_complete: bool = False,
    strict: bool = False,
    long: bool = False,
    record_baseline: bool = False,
) -> str:
    arguments = ["python3", str(TOOL), command, "--root", str(root)]
    if require_complete:
        arguments.append("--require-complete")
    if strict:
        arguments.append("--strict")
    if long:
        arguments.append("--long")
    if record_baseline:
        arguments.append("--record-baseline")
    completed = subprocess.run(
        arguments,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    assert completed.returncode == expected, completed.stdout
    return completed.stdout


def semantic_anonymous_owner(
    source: str,
    node_type: type[ast.AST],
    kind: str,
    binding_names: tuple[str, ...],
    lexical_parent: str = "",
    occurrence: int = 0,
) -> str:
    """Return the expected location-independent owner for one fixture node."""
    tree = ast.parse(source)

    matches = [
        node
        for node in ast.walk(tree)
        if isinstance(node, node_type)
    ]

    node = matches[occurrence]

    digest = hashlib.sha256(
        ast.dump(
            node,
            annotate_fields=True,
            include_attributes=False,
        ).encode("utf-8")
    ).hexdigest()[:12]

    names = ",".join(binding_names) or "-"
    anonymous = f"<{kind}[{names}]#{digest}>"

    if lexical_parent:
        semantic_base = f"{lexical_parent}.{anonymous}"
    else:
        semantic_base = anonymous

    return f"{semantic_base}~{occurrence + 1}"


def dictionary(
    rows: str,
    coverage: str = "IN_PROGRESS",
    directory: str = "src/net",
) -> str:
    """Render one disposable directory-owned symbol dictionary."""
    return f"""# Symbols

DIRECTORY={directory}
COVERAGE={coverage}

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
""" + rows


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)
    source = """/*
 * File synopsis:
 * Test source.
 */
static int packet_count;
"""
    write(root, "src/net/io.c", source)
    row = "| packet_count | variable | src/net/io.c | io.c | private | Counts completed packets for this fixture. | fixture |\n"
    write(root, "src/net/SYMBOLS.md", dictionary(row))

    output = run(root, 0)
    assert "SOURCE_DICTIONARIES=PASS" in output
    assert "SOURCE_DICTIONARY_CHECK_MODE=LONG_FALLBACK" in output
    assert "SOURCE_DICTIONARY_BASELINE=UNSET" in output
    assert (
        "SOURCE_DICTIONARY_BASELINE_REASON=NO_TRUSTED_LONG_PASS"
        in output
    )
    assert "DEFINITION_DISCOVERY_STATUS=PENDING" in output

    output = run(root, 0, long=True)
    assert "SOURCE_DICTIONARY_CHECK_MODE=LONG" in output
    assert "SOURCE_DICTIONARY_BASELINE_REASON=EXPLICIT_LONG" in output
    assert "DEFINITION_DISCOVERY_STATUS=PENDING" in output

    record_output = run(
        root,
        2,
        require_complete=True,
        long=True,
        record_baseline=True,
    )
    assert (
        "--record-baseline is unavailable until "
        "project-definition discovery is implemented"
        in record_output
    )

    write(
        root,
        "runtime/SOURCE_DICTIONARY_STATE.env",
        "SOURCE_DICTIONARY_STATE_VERSION=999\n"
        "LAST_LONG_PASS_COMMIT=UNSET\n",
    )
    assert "SOURCE_DICTIONARY_STATE_VERSION" in run(root, 1)
    (root / "runtime/SOURCE_DICTIONARY_STATE.env").unlink()

    assert "INCOMPLETE_DICTIONARY_COVERAGE=src/net" in run(
        root, 0, require_complete=True
    )
    assert "SOURCE_DICTIONARIES=ATTENTION" in run(
        root, 0, require_complete=True
    )
    assert "SOURCE_DICTIONARIES=FAIL_STRICT" in run(
        root, 1, require_complete=True, strict=True
    )

    write(root, "src/net/SYMBOLS.md", dictionary(row, coverage="COMPLETE"))
    assert "SOURCE_DICTIONARIES=PASS" in run(
        root, 0, require_complete=True, strict=True
    )
    portal_a = run(root, 0, "portal")
    portal_b = run(root, 0, "portal")
    assert portal_a == portal_b
    assert "src/net/SYMBOLS.md" in portal_a
    assert "COMPLETE" in portal_a

    write(root, "src/net/SYMBOLS.md", dictionary(row, coverage="UNVERIFIED"))
    assert "COVERAGE must be one of" in run(root, 1)

    write(root, "src/net/SYMBOLS.md", dictionary(row + row))
    assert "duplicate entry" in run(root, 1)

    # Structural duplicate identity must be rejected before ordinary
    # missing-source drift. Otherwise both rows could skip the duplicate
    # check through the missing-source early return.
    missing_source_row = row.replace(
        "src/net/io.c",
        "src/net/missing.c",
    )

    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            missing_source_row
            + missing_source_row
        ),
    )

    missing_duplicate_output = run(root, 1)

    assert "duplicate entry" in missing_duplicate_output
    assert (
        "SOURCE_DICTIONARIES=FAIL:"
        in missing_duplicate_output
    )

    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            row.replace("packet_count", "stale_name")
            + row.replace("packet_count", "other_stale_name")
        ),
    )
    output = run(root, 0)
    assert "STALE_DICTIONARY_SYMBOL=" in output
    assert "stale_name" in output
    assert "other_stale_name" in output
    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/io.c:packet_count:variable:-:"
        in output
    )
    assert "ATTENTION_COUNT=3" in output
    assert "SOURCE_DICTIONARIES=FAIL_STRICT" in run(root, 1, strict=True)

    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            row.replace(
                "Counts completed packets for this fixture.",
                "TODO",
            )
        ),
    )
    assert "INADEQUATE_DICTIONARY_DESCRIPTION=" in run(root, 0)
    assert "SOURCE_DICTIONARIES=ATTENTION" in run(root, 0)
    assert "SOURCE_DICTIONARIES=FAIL_STRICT" in run(root, 1, strict=True)

    write(root, "src/net/SYMBOLS.md", dictionary(row))
    write(root, "src/net/extra.c", source.replace("packet_count", "extra_count"))
    assert "MISSING_DICTIONARY_FILE_COVERAGE=src/net/extra.c" in run(root, 0)
    assert "SOURCE_DICTIONARIES=ATTENTION" in run(root, 0)
    assert "SOURCE_DICTIONARIES=FAIL_STRICT" in run(root, 1, strict=True)

    (root / "src/net/extra.c").unlink()
    write(root, "evidence/old.c", source.replace("packet_count", "historical_count"))
    assert "SOURCE_DICTIONARIES=PASS" in run(root, 0)

    # Development machinery is deliberately outside the product-symbol
    # completeness boundary even when it carries a clean-generation synopsis.
    write(
        root,
        "scripts/tool.py",
        """#!/usr/bin/env python3
\\"\\"\\"File synopsis:
Development-tool scope fixture.
\\"\\"\\"
tool_value = 1
""",
    )
    write(
        root,
        "scripts/testkit/helper.sh",
        """#!/usr/bin/env bash
# File synopsis:
# TestKit scope fixture.
helper_value=1
""",
    )
    write(
        root,
        "tests/unit/helper_test.c",
        """/*
 * File synopsis:
 * Unit-test scope fixture.
 */
static int test_value;
""",
    )
    write(
        root,
        "mk/helper.mk",
        """TOOL_BUILD_DIR = .tool-build

tool-target:
\t@true
""",
    )

    tool_scope_output = run(root, 0, long=True)

    assert "scripts/tool.py" not in tool_scope_output
    assert "scripts/testkit/helper.sh" not in tool_scope_output
    assert "tests/unit/helper_test.c" not in tool_scope_output
    assert "mk/helper.mk" not in tool_scope_output
    assert "SOURCE_DICTIONARIES=PASS" in tool_scope_output

with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """/*
 * File synopsis:
 * C definition-discovery fixture.
 */
#define PROJECT_LIMIT 4

typedef struct project_record {
    int value;
} project_record_t;

static int project_helper(int amount)
{
    int result = external_api(amount);
    return result;
}
"""

    rows = """| PROJECT_LIMIT | constant | src/net/io.c | fixture constants | private | Defines the fixture limit used to prove macro normalization. | fixture |
| project_record | structure | src/net/io.c | fixture types | private | Defines the fixture record used to prove structure discovery. | fixture |
| value | field | src/net/io.c | project_record | private | Stores the fixture record value used to prove member ownership. | fixture |
| project_record_t | type | src/net/io.c | fixture types | private | Names the fixture record type used to prove typedef discovery. | fixture |
| project_helper | function | src/net/io.c | fixture helper | private | Implements the fixture helper used to prove function discovery. | fixture |
| amount | parameter | src/net/io.c | project_helper | function | Supplies the fixture helper input used to prove parameter ownership. | fixture |
| result | variable | src/net/io.c | project_helper | local | Stores the fixture helper result used to prove local discovery. | fixture |
"""

    write(root, "src/net/io.c", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows, coverage="COMPLETE"),
    )

    output = run(root, 0, long=True)
    assert "SOURCE_DICTIONARIES=PASS" in output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output
    assert "external_api" not in output

    header = """/*
 * File synopsis:
 * Prototype parameter compatibility fixture.
 */
int project_public(int amount);
"""
    header_row = (
        "| project_public | function declaration | src/net/io.h | "
        "fixture interface | public | Declares the fixture public operation "
        "used to prove header discovery. | fixture |\n"
        "| amount | parameter | src/net/io.h | project_public | public | "
        "Names the fixture public argument while proving legacy generic "
        "parameter compatibility. | fixture |\n"
    )

    write(root, "src/net/io.h", header)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows + header_row, coverage="COMPLETE"),
    )

    output = run(root, 0, long=True)
    assert "SOURCE_DICTIONARIES=PASS" in output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output

    write(
        root,
        "src/net/io.c",
        source
        + """
static int project_new(int input)
{
    return input;
}
""",
    )

    output = run(root, 0, long=True)
    missing = {
        line
        for line in output.splitlines()
        if line.startswith("MISSING_DICTIONARY_SYMBOL=")
    }

    assert any(
        ":project_new:function:-:" in line
        for line in missing
    ), output
    assert any(
        ":input:parameter:project_new:" in line
        for line in missing
    ), output
    assert len(missing) == 2, output
    assert "external_api" not in output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """#!/usr/bin/env python3
\"\"\"File synopsis:
Python definition-discovery fixture.
\"\"\"

from external_package import external_api

PROJECT_LIMIT = 4
project_value = 1


class ProjectRecord:
    value: int

    def compute(self, amount: int) -> int:
        result = external_api(amount)
        for item in range(amount):
            result += item
        self.value = result
        return result


def project_helper(input_value: int) -> int:
    return external_api(input_value)
"""

    rows = """| PROJECT_LIMIT | constant | src/net/tool.py | fixture constants | private | Defines the fixture limit used to prove Python constant discovery. | fixture |
| project_value | variable | src/net/tool.py | fixture module | private | Stores one fixture module value used to prove Python variable discovery. | fixture |
| ProjectRecord | type | src/net/tool.py | fixture type | private | Defines the fixture class used to prove Python type discovery. | fixture |
| value | field | src/net/tool.py | ProjectRecord | private | Stores the fixture class value used to prove Python field discovery. | fixture |
| compute | function | src/net/tool.py | ProjectRecord | private | Implements the fixture method used to prove class-owned Python function discovery. | fixture |
| self | parameter | src/net/tool.py | ProjectRecord.compute | local | Names the current fixture instance for the method. | fixture |
| amount | parameter | src/net/tool.py | ProjectRecord.compute | local | Supplies the fixture method input. | fixture |
| result | variable | src/net/tool.py | ProjectRecord.compute | local | Stores the computed fixture result. | fixture |
| item | variable | src/net/tool.py | ProjectRecord.compute | local | Iterates fixture values used to prove loop-target discovery. | fixture |
| project_helper | function | src/net/tool.py | fixture helper | private | Implements the module helper used to prove Python function discovery. | fixture |
| input_value | parameter | src/net/tool.py | project_helper | local | Supplies the fixture module helper input. | fixture |
"""

    write(root, "src/net/tool.py", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows, coverage="COMPLETE"),
    )

    output = run(root, 0, long=True)
    assert "SOURCE_DICTIONARIES=PASS" in output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output
    assert "external_api" not in output
    assert "external_package" not in output

    write(
        root,
        "src/net/tool.py",
        source
        + """

def project_new(new_value: int) -> int:
    new_result = external_api(new_value)
    return new_result
""",
    )

    output = run(root, 0, long=True)
    missing = {
        line
        for line in output.splitlines()
        if line.startswith("MISSING_DICTIONARY_SYMBOL=")
    }

    assert any(
        ":project_new:function:-:" in line
        for line in missing
    ), output
    assert any(
        ":new_value:parameter:project_new:" in line
        for line in missing
    ), output
    assert any(
        ":new_result:variable:project_new:" in line
        for line in missing
    ), output
    assert len(missing) == 3, output
    assert "external_api" not in output
    assert "external_package" not in output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """#!/usr/bin/env python3
\"\"\"File synopsis:
Duplicate method-name ownership fixture.
\"\"\"


class First:
    def run(self, value):
        result = value
        return result


class Second:
    def run(self, value):
        result = value
        return result
"""

    rows = """| First | type | src/net/owners.py | fixture types | private | Defines the first fixture class used to prove lexical method ownership. | fixture |
| run | function | src/net/owners.py | First | private | Implements the first class method used to prove duplicate-name disambiguation. | fixture |
| self | parameter | src/net/owners.py | First.run | local | Names the first fixture instance. | fixture |
| value | parameter | src/net/owners.py | First.run | local | Supplies the first fixture method input. | fixture |
| result | variable | src/net/owners.py | First.run | local | Stores the first fixture method result. | fixture |
| Second | type | src/net/owners.py | fixture types | private | Defines the second fixture class used to prove lexical method ownership. | fixture |
| run | function | src/net/owners.py | Second | private | Implements the second class method using the same source name safely. | fixture |
| self | parameter | src/net/owners.py | Second.run | local | Names the second fixture instance. | fixture |
| value | parameter | src/net/owners.py | Second.run | local | Supplies the second fixture method input. | fixture |
| result | variable | src/net/owners.py | Second.run | local | Stores the second fixture method result. | fixture |
"""

    write(root, "src/net/owners.py", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows, coverage="COMPLETE"),
    )

    output = run(root, 0, long=True)
    assert "SOURCE_DICTIONARIES=PASS" in output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output

    # Remove only the second class's result row. If lexical ownership were
    # collapsed by method name, the first class's result row could hide it.
    rows_without_second_result = rows.replace(
        "| result | variable | src/net/owners.py | Second.run | local | "
        "Stores the second fixture method result. | fixture |\n",
        "",
    )
    assert rows_without_second_result != rows
    assert (
        "| result | variable | src/net/owners.py | Second.run | local |"
        not in rows_without_second_result
    )
    assert (
        "| result | variable | src/net/owners.py | First.run | local |"
        in rows_without_second_result
    )
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            rows_without_second_result,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 0, long=True)

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/owners.py:result:variable:Second.run:"
        in output
    ), output
    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/owners.py:result:variable:First.run:"
        not in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """/*
 * File synopsis:
 * Git baseline fixture source.
 */
static int packet_count;
"""
    row = (
        "| packet_count | variable | src/net/io.c | io.c | private | "
        "Counts completed packets for this fixture. | fixture |\n"
    )

    write(root, "src/net/io.c", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(row, coverage="COMPLETE"),
    )
    write(
        root,
        "runtime/SOURCE_DICTIONARY_STATE.env",
        "SOURCE_DICTIONARY_STATE_VERSION=1\n"
        "LAST_LONG_PASS_COMMIT=UNSET\n",
    )

    subprocess.run(
        ["git", "init", "-q"],
        cwd=root,
        check=True,
    )
    subprocess.run(
        ["git", "config", "user.name", "PS-to-VNC Self Test"],
        cwd=root,
        check=True,
    )
    subprocess.run(
        [
            "git",
            "config",
            "user.email",
            "self-test@example.invalid",
        ],
        cwd=root,
        check=True,
    )
    subprocess.run(
        ["git", "add", "."],
        cwd=root,
        check=True,
    )
    subprocess.run(
        ["git", "commit", "-q", "-m", "fixture"],
        cwd=root,
        check=True,
    )

    baseline_head = subprocess.check_output(
        ["git", "rev-parse", "HEAD"],
        cwd=root,
        text=True,
    ).strip()

    write(
        root,
        "runtime/SOURCE_DICTIONARY_STATE.env",
        "SOURCE_DICTIONARY_STATE_VERSION=1\n"
        f"LAST_LONG_PASS_COMMIT={baseline_head}\n",
    )

    subprocess.run(
        ["git", "add", "runtime/SOURCE_DICTIONARY_STATE.env"],
        cwd=root,
        check=True,
    )
    subprocess.run(
        ["git", "commit", "-q", "-m", "trusted baseline state"],
        cwd=root,
        check=True,
    )

    write(
        root,
        "src/net/io.c",
        source + "\n/* changed after trusted baseline */\n",
    )

    output = run(root, 0)
    assert "SOURCE_DICTIONARY_CHECK_MODE=INCREMENTAL" in output
    assert (
        "SOURCE_DICTIONARY_BASELINE_REASON=TRUSTED_BASELINE"
        in output
    )
    assert "DEFINITION_SCOPE_COUNT=1" in output
    assert "DEFINITION_SCOPE_PATH=src/net/io.c" in output
    assert "DEFINITION_DISCOVERY_STATUS=PENDING" in output

    write(
        root,
        "runtime/SOURCE_DICTIONARY_STATE.env",
        "SOURCE_DICTIONARY_STATE_VERSION=1\n"
        "LAST_LONG_PASS_COMMIT="
        "0000000000000000000000000000000000000000\n",
    )

    output = run(root, 0)
    assert "SOURCE_DICTIONARY_CHECK_MODE=LONG_FALLBACK" in output
    assert (
        "SOURCE_DICTIONARY_BASELINE_REASON="
        "BASELINE_COMMIT_UNRESOLVABLE"
        in output
    )



with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """#!/usr/bin/env bash
# File synopsis:
# Shell definition-discovery fixture.

ROOT="/tmp/project"
OPTIONS=(
    one
    two
)

export PROJECT_EXPORTED=1
export PYTHONDONTWRITEBYTECODE=1

project_shell()
{
    local item="$1"
    local declared

    for value in one two
    do
        result="$value"
    done

    : "${PROJECT_DEFAULT:=fallback}"
    : "${PROJECT_ALT=fallback}"

    TEMP_ONLY=1 EXTERNAL_ONLY=2 external_command "$item"
}
"""

    rows = """| ROOT | variable | src/net/tool.sh | fixture root | private | Stores the fixture root used to prove standalone shell assignment discovery. | fixture |
| OPTIONS | array | src/net/tool.sh | fixture options | private | Stores the fixture array used to prove Bash array discovery. | fixture |
| PROJECT_EXPORTED | variable | src/net/tool.sh | fixture environment | private | Stores the project-owned exported fixture variable. | fixture |
| project_shell | function | src/net/tool.sh | fixture operation | private | Implements the shell fixture operation. | fixture |
| item | variable | src/net/tool.sh | project_shell | local | Stores the first shell fixture argument. | fixture |
| declared | variable | src/net/tool.sh | project_shell | local | Declares a shell fixture local without an initial value. | fixture |
| value | variable | src/net/tool.sh | project_shell | local | Iterates shell fixture values. | fixture |
| result | variable | src/net/tool.sh | project_shell | local | Stores the shell fixture loop result. | fixture |
| PROJECT_DEFAULT | variable | src/net/tool.sh | project_shell | local | Stores a colon-default side-effect assignment in the fixture. | fixture |
| PROJECT_ALT | variable | src/net/tool.sh | project_shell | local | Stores a non-colon default side-effect assignment in the fixture. | fixture |
"""

    write(root, "src/net/tool.sh", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows, coverage="COMPLETE"),
    )

    output = run(root, 0, long=True)

    assert "SOURCE_DICTIONARIES=PASS" in output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output
    assert "TEMP_ONLY" not in output
    assert "EXTERNAL_ONLY" not in output
    assert "PYTHONDONTWRITEBYTECODE" not in output

    source += """
PROJECT_NEW=1
PROJECT_NEW_ARRAY=(alpha beta)
"""
    write(root, "src/net/tool.sh", source)

    output = run(root, 0, long=True)

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/tool.sh:PROJECT_NEW:variable:-:"
        in output
    ), output
    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/tool.sh:PROJECT_NEW_ARRAY:array:-:"
        in output
    ), output
    assert "TEMP_ONLY" not in output
    assert "EXTERNAL_ONLY" not in output
    assert "PYTHONDONTWRITEBYTECODE" not in output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """#!/usr/bin/env bash
# File synopsis:
# Supported shell-read binding fixture.

project_reader()
{
    read -r PROJECT_READ PROJECT_PATH
}
"""

    rows = """| project_reader | function | src/net/tool.sh | shell read fixture | private | Implements the supported shell read-binding fixture. | fixture |
"""

    write(root, "src/net/tool.sh", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows, coverage="COMPLETE"),
    )

    output = run(root, 0, long=True)

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/tool.sh:PROJECT_READ:variable:"
        "project_reader:"
        in output
    ), output
    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/tool.sh:PROJECT_PATH:variable:"
        "project_reader:"
        in output
    ), output

    rows += """| PROJECT_READ | variable | src/net/tool.sh | project_reader | local | Stores the first project-owned value populated by the supported read fixture. | fixture |
| PROJECT_PATH | variable | src/net/tool.sh | project_reader | local | Stores the second project-owned value populated by the supported read fixture. | fixture |
"""

    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows, coverage="COMPLETE"),
    )

    output = run(root, 0, long=True)

    assert "SOURCE_DICTIONARIES=PASS" in output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """#!/usr/bin/env bash
# File synopsis:
# Unsupported shell-read grammar fixture.

project_reader()
{
    read -a PROJECT_VALUES
}
"""

    rows = """| project_reader | function | src/net/tool.sh | unsupported read fixture | private | Implements the fail-closed unsupported shell read fixture. | fixture |
"""

    write(root, "src/net/tool.sh", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows, coverage="COMPLETE"),
    )

    output = run(root, 1, long=True)

    assert "unsupported shell read grammar" in output, output
    assert "read -a PROJECT_VALUES" in output, output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """#!/usr/bin/env bash
# File synopsis:
# Unsupported arithmetic-definition fixture.

PROJECT_COUNTER=0
((PROJECT_COUNTER += 1))
"""

    rows = """| PROJECT_COUNTER | variable | src/net/tool.sh | fixture counter | private | Stores the arithmetic fixture counter. | fixture |
"""

    write(root, "src/net/tool.sh", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(rows, coverage="COMPLETE"),
    )

    output = run(root, 1, long=True)

    assert (
        "unsupported arithmetic shell definition"
        in output
    ), output




with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Exercise Python lambda, default-expression, and pattern bindings.

module_lambda = lambda lambda_value: lambda_value


def outer(argument=(default_binding := 7)):
    match argument:
        case {"item": captured, **remaining}:
            return captured, remaining
        case [head, *tail]:
            return head, tail
    return None
"""

    lambda_owner = semantic_anonymous_owner(
        source,
        ast.Lambda,
        "lambda",
        ("lambda_value",),
    )

    rows = f"""| module_lambda | variable | src/net/python_bindings.py | fixture module | private | Stores the fixture lambda used to prove lambda parameter discovery. | fixture |
| lambda_value | parameter | src/net/python_bindings.py | {lambda_owner} | local | Names the input parameter owned by the module-level fixture lambda. | fixture |
| outer | function | src/net/python_bindings.py | fixture functions | private | Exercises default-expression and structural-pattern binding discovery. | fixture |
| argument | parameter | src/net/python_bindings.py | outer | local | Supplies the value inspected by the fixture match statement. | fixture |
| default_binding | variable | src/net/python_bindings.py | fixture module | private | Captures the named expression evaluated in the function default expression. | fixture |
| captured | variable | src/net/python_bindings.py | outer | local | Captures the named item from the fixture mapping pattern. | fixture |
| remaining | variable | src/net/python_bindings.py | outer | local | Captures remaining items from the fixture mapping pattern. | fixture |
| head | variable | src/net/python_bindings.py | outer | local | Captures the leading item from the fixture sequence pattern. | fixture |
| tail | variable | src/net/python_bindings.py | outer | local | Captures remaining items from the fixture sequence pattern. | fixture |
"""

    write(
        root,
        "src/net/python_bindings.py",
        source,
    )
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            rows,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 0, long=True)

    assert "SOURCE_DICTIONARIES=PASS" in output, output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output, output

    reduced_rows = rows.replace(
        f"| lambda_value | parameter | src/net/python_bindings.py | {lambda_owner} | local | Names the input parameter owned by the module-level fixture lambda. | fixture |\n",
        "",
    ).replace(
        "| captured | variable | src/net/python_bindings.py | outer | local | Captures the named item from the fixture mapping pattern. | fixture |\n",
        "",
    )

    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            reduced_rows,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 0, long=True)

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/python_bindings.py:"
        f"lambda_value:parameter:{lambda_owner}:"
        in output
    ), output

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/python_bindings.py:"
        "captured:variable:outer:"
        in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Prove augmented assignment is mutation rather than a declaration.

sentinel = 0
counter += 1
"""

    row = "| sentinel | variable | src/net/augassign.py | fixture module | private | Provides the only actual variable declaration in the augmented-assignment fixture. | fixture |\n"

    write(root, "src/net/augassign.py", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            row,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 0, long=True)

    assert "SOURCE_DICTIONARIES=PASS" in output, output
    assert ":counter:" not in output, output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Unsupported global-ownership fixture.

def mutate():
    global PROJECT_STATE
    PROJECT_STATE = 1
"""

    row = "| mutate | function | src/net/global_binding.py | fixture functions | private | Provides the function surrounding the unsupported global ownership declaration. | fixture |\n"

    write(root, "src/net/global_binding.py", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            row,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 1, long=True)

    assert (
        "Python global binding ownership is not yet supported"
        in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Supported one-parent nonlocal ownership fixture.

def outer():
    state = 0

    def inner():
        nonlocal state
        state = 1

    return inner
"""

    rows = """| outer | function | src/net/nonlocal_binding.py | fixture functions | private | Provides the outer scope owning the fixture nonlocal state. | fixture |
| state | variable | src/net/nonlocal_binding.py | outer | local | Stores the outer local updated through the nested nonlocal declaration. | fixture |
| inner | function | src/net/nonlocal_binding.py | outer | private | Provides the nested function that updates its enclosing state. | fixture |
"""

    write(
        root,
        "src/net/nonlocal_binding.py",
        source,
    )
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            rows,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 0, long=True)

    assert "SOURCE_DICTIONARIES=PASS" in output, output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output, output
    assert ":state:variable:outer.inner:" not in output, output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Ambiguous deeper nonlocal ownership fixture.

def outer():
    state = 0

    def middle():
        def inner():
            nonlocal state
            state = 1

        return inner

    return middle
"""

    rows = """| outer | function | src/net/deep_nonlocal.py | fixture functions | private | Provides the outer scope in the deliberately unsupported deep nonlocal fixture. | fixture |
| state | variable | src/net/deep_nonlocal.py | outer | local | Stores the outer state referenced across more than one nested function boundary. | fixture |
| middle | function | src/net/deep_nonlocal.py | outer | private | Provides the intermediate function that makes lexical ownership nontrivial. | fixture |
| inner | function | src/net/deep_nonlocal.py | outer.middle | private | Provides the deeply nested function whose nonlocal ownership must fail closed. | fixture |
"""

    write(
        root,
        "src/net/deep_nonlocal.py",
        source,
    )
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            rows,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 1, long=True)

    assert (
        "nonlocal binding requires unsupported "
        "multi-level lexical ownership resolution"
        in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Unsupported modern Python type-binding fixture.

sentinel = 1
type ProjectAlias = int
"""

    row = "| sentinel | variable | src/net/type_alias.py | fixture module | private | Keeps the dictionary structurally valid while type-alias fail-closed behavior is tested. | fixture |\n"

    write(root, "src/net/type_alias.py", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            row,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 1, long=True)

    assert (
        "Python type aliases are not yet supported"
        in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Unsupported Python type-parameter fixture.

def generic[T](value: T):
    return value
"""

    row = "| generic | function | src/net/type_parameter.py | fixture functions | private | Provides the generic function whose type parameter must fail closed. | fixture |\n"

    write(
        root,
        "src/net/type_parameter.py",
        source,
    )
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            row,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 1, long=True)

    assert (
        "Python type parameters are not yet supported"
        in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Exercise comprehension ownership and starred assignment targets.

module_values = [module_item for module_item in (1, 2)]


def build():
    return {local_item for local_item in (3, 4)}


head, *tail = (1, 2, 3)
"""

    module_comp_owner = semantic_anonymous_owner(
        source,
        ast.ListComp,
        "listcomp",
        ("module_item",),
    )

    local_comp_owner = semantic_anonymous_owner(
        source,
        ast.SetComp,
        "setcomp",
        ("local_item",),
        lexical_parent="build",
    )

    rows = f"""| module_values | variable | src/net/comprehension_scope.py | fixture module | private | Stores the module-level list-comprehension result. | fixture |
| module_item | variable | src/net/comprehension_scope.py | {module_comp_owner} | local | Names the iteration value local to the module-level list comprehension. | fixture |
| build | function | src/net/comprehension_scope.py | fixture functions | private | Returns a set built by a function-local comprehension. | fixture |
| local_item | variable | src/net/comprehension_scope.py | {local_comp_owner} | local | Names the iteration value local to the function-level set comprehension. | fixture |
| head | variable | src/net/comprehension_scope.py | fixture module | private | Receives the leading value from the starred assignment fixture. | fixture |
| tail | variable | src/net/comprehension_scope.py | fixture module | private | Receives the starred remainder from the assignment fixture. | fixture |
"""

    write(
        root,
        "src/net/comprehension_scope.py",
        source,
    )
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            rows,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 0, long=True)

    assert "SOURCE_DICTIONARIES=PASS" in output, output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output, output

    reduced_rows = rows.replace(
        f"| module_item | variable | src/net/comprehension_scope.py | {module_comp_owner} | local | Names the iteration value local to the module-level list comprehension. | fixture |\n",
        "",
    ).replace(
        f"| local_item | variable | src/net/comprehension_scope.py | {local_comp_owner} | local | Names the iteration value local to the function-level set comprehension. | fixture |\n",
        "",
    ).replace(
        "| tail | variable | src/net/comprehension_scope.py | fixture module | private | Receives the starred remainder from the assignment fixture. | fixture |\n",
        "",
    )

    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            reduced_rows,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 0, long=True)

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/comprehension_scope.py:"
        f"module_item:variable:{module_comp_owner}:"
        in output
    ), output

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/comprehension_scope.py:"
        f"local_item:variable:{local_comp_owner}:"
        in output
    ), output

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/comprehension_scope.py:"
        "tail:variable:-:"
        in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Assignment expression in a comprehension must fail closed.

values = [(captured := item) for item in (1, 2)]
"""

    row = "| values | variable | src/net/comprehension_namedexpr.py | fixture module | private | Stores the result surrounding the unsupported comprehension assignment expression. | fixture |\n"

    write(
        root,
        "src/net/comprehension_namedexpr.py",
        source,
    )
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            row,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 1, long=True)

    assert (
        "assignment expressions inside Python comprehensions "
        "require explicit containing-scope ownership support"
        in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# File synopsis:
# Identical same-parent lambdas must not collapse to one anonymous owner.

first = lambda value: value
second = lambda value: value
"""

    first_lambda_owner = semantic_anonymous_owner(
        source,
        ast.Lambda,
        "lambda",
        ("value",),
        occurrence=0,
    )

    second_lambda_owner = semantic_anonymous_owner(
        source,
        ast.Lambda,
        "lambda",
        ("value",),
        occurrence=1,
    )

    assert first_lambda_owner != second_lambda_owner
    assert first_lambda_owner.endswith("~1")
    assert second_lambda_owner.endswith("~2")

    rows = f"""| first | variable | src/net/lambda_collision.py | fixture module | private | Stores the first anonymous function in the identical-lambda occurrence fixture. | fixture |
| value | parameter | src/net/lambda_collision.py | {first_lambda_owner} | local | Names the parameter of the first syntactically identical anonymous function. | fixture |
| second | variable | src/net/lambda_collision.py | fixture module | private | Stores the second syntactically identical anonymous function. | fixture |
| value | parameter | src/net/lambda_collision.py | {second_lambda_owner} | local | Names the parameter of the second syntactically identical anonymous function. | fixture |
"""

    write(
        root,
        "src/net/lambda_collision.py",
        source,
    )
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            rows,
            coverage="COMPLETE",
        ),
    )

    output = run(root, 0, long=True)

    assert "SOURCE_DICTIONARIES=PASS" in output, output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output, output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# Make definition fixture.

PROJECT_DIR ?= build
PROJECT_APPEND += appended
override PROJECT_OVERRIDE := overridden

define PROJECT_BLOCK
fixture body
endef

CC ?= cc
CFLAGS ?=
EE_BIN = external-interface-name

.PHONY: all
all: $(PROJECT_DIR)/artifact PROJECT_PREREQUISITE

$(PROJECT_DIR)/artifact:
	@echo artifact

project-specific: PROJECT_TARGET_SPECIFIC := value
project-specific:
	@echo target-specific
"""

    rows = """| PROJECT_DIR | variable | src/net/Makefile | make fixture | private | Stores the project-owned output directory used by the Make fixture. | fixture |
| PROJECT_APPEND | variable | src/net/Makefile | make fixture | private | Exercises append-assignment discovery for a project-owned Make variable. | fixture |
| PROJECT_OVERRIDE | variable | src/net/Makefile | make fixture | private | Exercises prefixed simple-assignment discovery for a project-owned Make variable. | fixture |
| PROJECT_BLOCK | variable | src/net/Makefile | make fixture | private | Exercises GNU Make define-block discovery. | fixture |
| PROJECT_TARGET_SPECIFIC | variable | src/net/Makefile | make fixture | private | Exercises target-specific Make variable discovery. | fixture |
| all | target | src/net/Makefile | make fixture | private | Names the fixture aggregate target. | fixture |
| $(PROJECT_DIR)/artifact | target | src/net/Makefile | make fixture | private | Names the fixture artifact target. | fixture |
| project-specific | target | src/net/Makefile | make fixture | private | Names the fixture target carrying target-specific state. | fixture |
"""

    write(root, "src/net/Makefile", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            rows,
            coverage="COMPLETE",
            directory="src/net",
        ),
    )

    output = run(root, 0, long=True)

    assert "SOURCE_DICTIONARIES=PASS" in output, output
    assert "MISSING_DICTIONARY_SYMBOL=" not in output, output

    for external_name in (
        "CC",
        "CFLAGS",
        "EE_BIN",
        "PROJECT_PREREQUISITE",
        ".PHONY",
    ):
        assert external_name not in (
            line.split("MISSING_DICTIONARY_SYMBOL=", 1)[1]
            for line in output.splitlines()
            if line.startswith("MISSING_DICTIONARY_SYMBOL=")
        ), output

    reduced_rows = rows.replace(
        "| PROJECT_APPEND | variable | src/net/Makefile | make fixture | private | Exercises append-assignment discovery for a project-owned Make variable. | fixture |\n",
        "",
    ).replace(
        "| project-specific | target | src/net/Makefile | make fixture | private | Names the fixture target carrying target-specific state. | fixture |\n",
        "",
    )

    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            reduced_rows,
            coverage="COMPLETE",
            directory="src/net",
        ),
    )

    output = run(root, 0, long=True)

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/Makefile:PROJECT_APPEND:variable:-:"
        in output
    ), output

    assert (
        "MISSING_DICTIONARY_SYMBOL="
        "src/net/Makefile:project-specific:target:-:"
        in output
    ), output


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)

    source = """# Unsupported Make definition fixture.

$(PROJECT_DYNAMIC) := value
"""

    rows = """| sentinel | target | src/net/Makefile | make fixture | private | Keeps the structural dictionary nonempty while unsupported syntax is tested. | fixture |
"""

    write(root, "src/net/Makefile", source)
    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            rows,
            coverage="COMPLETE",
            directory="src/net",
        ),
    )

    output = run(root, 1, long=True)

    assert "unsupported Make assignment grammar" in output, output
    assert "$(PROJECT_DYNAMIC) := value" in output, output


print("SOURCE_DICTIONARY_SELF_TEST=PASS")
