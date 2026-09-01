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


def run(root: Path, expected: int, command: str = "check") -> str:
    completed = subprocess.run(
        ["python3", str(TOOL), command, "--root", str(root)],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    assert completed.returncode == expected, completed.stdout
    return completed.stdout


def dictionary(rows: str) -> str:
    return """# Symbols

DIRECTORY=src/net

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
    portal_a = run(root, 0, "portal")
    portal_b = run(root, 0, "portal")
    assert portal_a == portal_b
    assert "src/net/SYMBOLS.md" in portal_a

    write(root, "src/net/SYMBOLS.md", dictionary(row + row))
    assert "duplicate entry" in run(root, 1)

    write(root, "src/net/SYMBOLS.md", dictionary(row.replace("packet_count", "stale_name")))
    assert "stale symbol" in run(root, 1)

    write(root, "src/net/SYMBOLS.md", dictionary(row.replace("Counts completed packets for this fixture.", "TODO")))
    assert "inadequate description" in run(root, 1)

    write(root, "src/net/SYMBOLS.md", dictionary(row))
    write(root, "src/net/extra.c", source.replace("packet_count", "extra_count"))
    assert "without dictionary entries" in run(root, 1)

    (root / "src/net/extra.c").unlink()
    write(root, "evidence/old.c", source.replace("packet_count", "historical_count"))
    assert "SOURCE_DICTIONARIES=PASS" in run(root, 0)

print("SOURCE_DICTIONARY_SELF_TEST=PASS")
