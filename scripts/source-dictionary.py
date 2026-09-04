#!/usr/bin/env python3
"""File synopsis:
Validate and aggregate directory-owned PS-to-VNC symbol dictionaries.

This tool owns clean-generation dictionary validation and deterministic views;
it does not define product symbols or include historical/pre-refresh source.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path

CLEAN_MARKER = "File synopsis:"
DICT_NAME = "SYMBOLS.md"
EXCLUDED_PARTS = {"baseline", "evidence", "working", "build", ".git"}
PLACEHOLDERS = {"todo", "tbd", "unknown", "placeholder", "describe me"}
HEADER = ("Name", "Kind", "File", "Owner", "Scope", "Description", "Context")
COVERAGE_STATES = {"IN_PROGRESS", "COMPLETE"}


class DictionaryError(RuntimeError):
    pass


@dataclass(frozen=True)
class Entry:
    name: str
    kind: str
    file: str
    owner: str
    scope: str
    description: str
    context: str
    dictionary: Path
    line: int


def is_excluded(path: Path, root: Path) -> bool:
    return any(part in EXCLUDED_PARTS for part in path.relative_to(root).parts)


def clean_files(root: Path) -> set[Path]:
    result: set[Path] = set()
    for suffix in ("*.c", "*.h", "*.py", "*.sh"):
        for path in root.rglob(suffix):
            if is_excluded(path, root) or not path.is_file():
                continue
            if CLEAN_MARKER in path.read_text(encoding="utf-8", errors="strict")[:2048]:
                result.add(path.relative_to(root))
    return result


def unescape(cell: str) -> str:
    return cell.strip().replace("\\|", "|").replace("`", "")


def parse_dictionary(path: Path, root: Path) -> tuple[str, str, list[Entry]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    directory = None
    coverage = None
    entries: list[Entry] = []
    in_table = False
    for number, line in enumerate(lines, 1):
        if line.startswith("DIRECTORY="):
            directory = line.split("=", 1)[1].strip()
        if line.startswith("COVERAGE="):
            coverage = line.split("=", 1)[1].strip()
        if line.strip().startswith("| Name | Kind | File |"):
            cells = tuple(unescape(x) for x in line.strip().strip("|").split("|"))
            if cells != HEADER:
                raise DictionaryError(f"{path}:{number}: unexpected table columns")
            in_table = True
            continue
        if in_table and re.fullmatch(r"\s*\|(?:\s*:?-+:?\s*\|){7}\s*", line):
            continue
        if in_table and line.lstrip().startswith("|"):
            cells = [unescape(x) for x in line.strip().strip("|").split("|")]
            if len(cells) != 7:
                raise DictionaryError(f"{path}:{number}: expected seven cells")
            entries.append(Entry(*cells, dictionary=path, line=number))
        elif in_table and line.strip():
            in_table = False
    if not directory:
        raise DictionaryError(f"{path}: missing DIRECTORY metadata")
    if coverage not in COVERAGE_STATES:
        allowed = ", ".join(sorted(COVERAGE_STATES))
        raise DictionaryError(f"{path}: COVERAGE must be one of {allowed}")
    expected = path.parent.relative_to(root).as_posix() or "."
    if directory != expected:
        raise DictionaryError(f"{path}: DIRECTORY={directory}, expected {expected}")
    return directory, coverage, entries


def validate(root: Path, require_complete: bool = False) -> tuple[list[tuple[str, str, Path, list[Entry]]], set[Path], list[str]]:
    dictionaries = []
    covered: set[Path] = set()
    keys: set[tuple[str, str, str, str]] = set()
    incomplete: list[str] = []
    attention: list[str] = []
    for path in sorted(root.rglob(DICT_NAME)):
        if is_excluded(path, root):
            continue
        directory, coverage, entries = parse_dictionary(path, root)
        if coverage != "COMPLETE":
            incomplete.append(directory)
        if not entries:
            raise DictionaryError(f"{path}: dictionary has no entries")
        for entry in entries:
            source = root / entry.file
            try:
                source.relative_to(path.parent)
            except ValueError as exc:
                raise DictionaryError(
                    f"{path}:{entry.line}: {entry.file} is outside dictionary directory"
                ) from exc
            if not source.is_file():
                attention.append(
                    f"MISSING_DICTIONARY_SOURCE={path}:{entry.line}:{entry.file}"
                )
                continue
            source_text = source.read_text(encoding="utf-8")
            if CLEAN_MARKER not in source_text[:2048]:
                raise DictionaryError(
                    f"{path}:{entry.line}: {entry.file} is not clean-generation source"
                )
            if not re.search(rf"(?<![A-Za-z0-9_]){re.escape(entry.name)}(?![A-Za-z0-9_])", source_text):
                attention.append(
                    f"STALE_DICTIONARY_SYMBOL={path}:{entry.line}:{entry.file}:{entry.name}"
                )
            if len(entry.description) < 12 or entry.description.lower() in PLACEHOLDERS:
                attention.append(
                    f"INADEQUATE_DICTIONARY_DESCRIPTION={path}:{entry.line}:{entry.file}:{entry.name}"
                )
            key = (entry.file, entry.owner, entry.kind, entry.name)
            if key in keys:
                raise DictionaryError(f"{path}:{entry.line}: duplicate entry {key}")
            keys.add(key)
            covered.add(Path(entry.file))
        dictionaries.append((directory, coverage, path, entries))
    missing = clean_files(root) - covered
    for source in sorted(missing):
        attention.append(
            f"MISSING_DICTIONARY_FILE_COVERAGE={source.as_posix()}"
        )
    if require_complete:
        for directory in sorted(incomplete):
            attention.append(
                f"INCOMPLETE_DICTIONARY_COVERAGE={directory}"
            )
    return dictionaries, covered, attention


def render_portal(root: Path, dictionaries) -> str:
    lines = [
        "# Clean Source Symbol Dictionaries", "",
        "This portal is generated from directory-owned `SYMBOLS.md` files.", "",
        "| Directory | Dictionary | Coverage | Symbols |", "|---|---|---|---:|",
    ]
    for directory, coverage, path, entries in dictionaries:
        rel = path.relative_to(root).as_posix()
        lines.append(f"| `{directory}` | [`{rel}`](../../{rel}) | {coverage} | {len(entries)} |")
    lines += ["", "Generate the comprehensive view with:", "", "```sh",
              "python3 scripts/source-dictionary.py aggregate", "```", ""]
    return "\n".join(lines)


def render_aggregate(dictionaries) -> str:
    lines = ["# Comprehensive Clean Source Symbol Index", "",
             "Generated from directory-owned dictionaries; do not edit here.", ""]
    for directory, coverage, _path, entries in dictionaries:
        lines += [f"## `{directory}`", "", f"Coverage: `{coverage}`", "",
                  "| Name | Kind | File | Owner | Scope | Description | Context |",
                  "|---|---|---|---|---|---|---|"]
        for entry in sorted(entries, key=lambda item: (item.file, item.owner, item.kind, item.name)):
            values = [entry.name, entry.kind, entry.file, entry.owner, entry.scope,
                      entry.description, entry.context]
            values = [value.replace("|", "\\|") for value in values]
            lines.append("| " + " | ".join(values) + " |")
        lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("check", "portal", "aggregate"))
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--output", type=Path)
    parser.add_argument(
        "--require-complete",
        action="store_true",
        help="report directories that still declare COVERAGE=IN_PROGRESS",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="exit nonzero when ordinary dictionary-maintenance findings exist",
    )
    args = parser.parse_args()
    try:
        dictionaries, _, attention = validate(
            args.root.resolve(),
            args.require_complete,
        )

        if attention:
            if args.command == "check":
                for finding in attention:
                    print(finding)
                print(f"ATTENTION_COUNT={len(attention)}")
                if args.strict:
                    print("SOURCE_DICTIONARIES=FAIL_STRICT")
                    return 1
                print("SOURCE_DICTIONARIES=ATTENTION")
                print("CHECK_CONTINUES=YES")
                return 0

            for finding in attention:
                print(finding, file=sys.stderr)
            print(
                f"SOURCE_DICTIONARIES_ATTENTION_COUNT={len(attention)}",
                file=sys.stderr,
            )
            if args.strict:
                print("SOURCE_DICTIONARIES=FAIL_STRICT", file=sys.stderr)
                return 1

        if args.command == "check":
            print("SOURCE_DICTIONARIES=PASS")
            return 0
        rendered = (
            render_portal(args.root.resolve(), dictionaries)
            if args.command == "portal"
            else render_aggregate(dictionaries)
        )
        if args.output:
            args.output.write_text(rendered, encoding="utf-8")
        else:
            sys.stdout.write(rendered)
        return 0
    except DictionaryError as exc:
        print(f"SOURCE_DICTIONARIES=FAIL: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
