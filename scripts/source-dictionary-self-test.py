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
) -> str:
    arguments = ["python3", str(TOOL), command, "--root", str(root)]
    if require_complete:
        arguments.append("--require-complete")
    if strict:
        arguments.append("--strict")
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

    assert "SOURCE_DICTIONARIES=PASS" in run(root, 0)
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

print("SOURCE_DICTIONARY_SELF_TEST=PASS")
