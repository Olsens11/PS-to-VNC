#!/usr/bin/env python3
"""File synopsis:
Run disposable regression tests for the clean source-dictionary tooling.

This self-test owns only validator fixtures; it does not inspect historical
source or claim exhaustive production symbol coverage.
"""

from pathlib import Path
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


def dictionary(rows: str, coverage: str = "IN_PROGRESS") -> str:
    return f"""# Symbols

DIRECTORY=src/net
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

    write(
        root,
        "src/net/SYMBOLS.md",
        dictionary(
            row.replace("packet_count", "stale_name")
            + row.replace("packet_count", "other_stale_name")
        ),
    )
    assert "STALE_DICTIONARY_SYMBOL=" in run(root, 0)
    assert "stale_name" in run(root, 0)
    assert "other_stale_name" in run(root, 0)
    assert "ATTENTION_COUNT=2" in run(root, 0)
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


print("SOURCE_DICTIONARY_SELF_TEST=PASS")
