#!/usr/bin/env python3
"""File synopsis:
Reconcile the current ledge clean-source dictionary maintenance set.

This maintenance helper preserves dictionary rows that still map to a current
mechanically discovered definition, removes rows whose defining source/lexical
owner no longer exists, and appends current definitions that the canonical
source-dictionary validator reports as missing. Its explicit target set contains
only domains whose current definition forms are safe for this mechanical
rewriter. A003 R8 adds the maintained Python-only `pi/` product root to that
set; other product dictionaries remain governed by the canonical validator and
are not rewritten merely because they exist in the clean topology.

Run only with Universal Ctags available. After reconciliation, regenerate the
portal and run the canonical long/complete/strict source-dictionary check.
"""

from __future__ import annotations

import importlib.util
import sys
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
VALIDATOR_PATH = ROOT / "scripts" / "source-dictionary.py"
TARGET_DIRECTORIES = (
    Path("pi"),
    Path("src"),
    Path("src/audio"),
    Path("src/config"),
    Path("src/display"),
    Path("src/input"),
    Path("src/media"),
    Path("src/mpeg"),
    Path("src/platform"),
    Path("src/rfb"),
    Path("src/transport"),
)
TABLE_HEADER = "| Name | Kind | File | Owner | Scope | Description | Context |"
TABLE_SEPARATOR = "|---|---|---|---|---|---|---|"


@dataclass(frozen=True)
class RenderedEntry:
    name: str
    kind: str
    file: str
    owner: str
    scope: str
    description: str
    context: str


def load_validator():
    spec = importlib.util.spec_from_file_location(
        "pstvnc_source_dictionary_validator",
        VALIDATOR_PATH,
    )
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {VALIDATOR_PATH}")

    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def entry_from_existing(entry) -> RenderedEntry:
    return RenderedEntry(
        name=entry.name,
        kind=entry.kind,
        file=entry.file,
        owner=entry.owner,
        scope=entry.scope,
        description=entry.description,
        context=entry.context,
    )


def entry_maps_to_current_definition(validator, entry, definitions) -> bool:
    """Require an actual current definition, including lexical-owner identity."""
    return any(
        validator.definition_is_documented(definition, [entry])
        for definition in definitions
    )


def generated_entry(definition) -> RenderedEntry:
    source = Path(definition.file)
    if definition.owner:
        owner = definition.owner
        scope = "local"
    elif source.suffix == ".h":
        owner = f"{source.stem} interface"
        scope = "public"
    else:
        owner = source.stem
        scope = "file"

    return RenderedEntry(
        name=definition.name,
        kind=definition.kind,
        file=definition.file,
        owner=owner,
        scope=scope,
        description=(
            f"Defines {definition.name} as a current clean-source "
            f"{definition.kind}."
        ),
        context="mechanically reconciled current clean source",
    )


def escape_cell(value: str) -> str:
    return value.replace("|", "\\|")


def render_row(entry: RenderedEntry) -> str:
    cells = (
        entry.name,
        entry.kind,
        entry.file,
        entry.owner,
        entry.scope,
        entry.description,
        entry.context,
    )
    return "| " + " | ".join(escape_cell(cell) for cell in cells) + " |"


def replace_table(path: Path, entries: list[RenderedEntry]) -> None:
    lines = path.read_text(encoding="utf-8").splitlines()
    try:
        header_index = next(
            index
            for index, line in enumerate(lines)
            if line.strip() == TABLE_HEADER
        )
    except StopIteration as exc:
        raise RuntimeError(f"{path}: canonical dictionary table not found") from exc

    if (
        header_index + 1 >= len(lines)
        or lines[header_index + 1].strip() != TABLE_SEPARATOR
    ):
        raise RuntimeError(f"{path}: canonical dictionary separator not found")

    end_index = header_index + 2
    while end_index < len(lines) and lines[end_index].lstrip().startswith("|"):
        end_index += 1

    rendered = [
        *lines[:header_index],
        TABLE_HEADER,
        TABLE_SEPARATOR,
        *(render_row(entry) for entry in entries),
        *lines[end_index:],
    ]
    path.write_text("\n".join(rendered).rstrip() + "\n", encoding="utf-8")


def discover_directory_definitions(validator, directory: Path):
    definitions = set()
    clean_sources = sorted(
        path
        for path in validator.clean_files(ROOT)
        if path.parent == directory
    )

    for source in clean_sources:
        if source.suffix in {".c", ".h"}:
            definitions.update(
                validator.discover_c_definitions(ROOT, source)
            )
        elif source.suffix == ".py":
            definitions.update(
                validator.discover_python_definitions(ROOT, source)
            )
        else:
            raise RuntimeError(
                f"{source}: reconciliation helper has no safe adapter "
                "for this current target"
            )

    return sorted(definitions)


def reconcile_dictionary(validator, directory: Path) -> tuple[int, int, int]:
    dictionary = ROOT / directory / "SYMBOLS.md"
    if not dictionary.is_file():
        raise RuntimeError(f"missing target dictionary: {dictionary}")

    declared_directory, coverage, parsed_entries = validator.parse_dictionary(
        dictionary,
        ROOT,
    )
    if declared_directory != directory.as_posix():
        raise RuntimeError(
            f"{dictionary}: DIRECTORY={declared_directory}, expected {directory}"
        )
    if coverage != "COMPLETE" and not (
        directory == Path("pi") and coverage == "IN_PROGRESS"
    ):
        raise RuntimeError(
            f"{dictionary}: expected COMPLETE coverage, or the explicitly "
            "adopted pi/ IN_PROGRESS bootstrap"
        )

    definitions = discover_directory_definitions(validator, directory)
    kept_parsed = [
        entry
        for entry in parsed_entries
        if entry_maps_to_current_definition(validator, entry, definitions)
    ]
    removed = len(parsed_entries) - len(kept_parsed)
    rendered = [entry_from_existing(entry) for entry in kept_parsed]

    added = 0
    for definition in definitions:
        if validator.definition_is_documented(definition, kept_parsed):
            continue

        new_entry = generated_entry(definition)
        rendered.append(new_entry)

        # Make the newly generated row visible to subsequent definitions using
        # the same canonical matcher without relying on a second tool pass.
        kept_parsed.append(
            validator.Entry(
                name=new_entry.name,
                kind=new_entry.kind,
                file=new_entry.file,
                owner=new_entry.owner,
                scope=new_entry.scope,
                description=new_entry.description,
                context=new_entry.context,
                dictionary=dictionary,
                line=0,
            )
        )
        added += 1

    # Existing current rows retain their semantic wording/order. Newly
    # discovered rows arrive in deterministic Definition ordering.
    replace_table(dictionary, rendered)

    if coverage == "IN_PROGRESS":
        text = dictionary.read_text(encoding="utf-8")
        marker = "COVERAGE=IN_PROGRESS"
        if text.count(marker) != 1:
            raise RuntimeError(
                f"{dictionary}: cannot promote ambiguous coverage metadata"
            )
        dictionary.write_text(
            text.replace(marker, "COVERAGE=COMPLETE", 1),
            encoding="utf-8",
        )

    return len(parsed_entries), removed, added


def main() -> int:
    validator = load_validator()
    total_removed = 0
    total_added = 0

    for directory in TARGET_DIRECTORIES:
        before, removed, added = reconcile_dictionary(validator, directory)
        total_removed += removed
        total_added += added
        print(
            "DICTIONARY_RECONCILED="
            f"{directory.as_posix()} before={before} "
            f"removed={removed} added={added}"
        )

    print(f"DICTIONARY_RECONCILIATION_REMOVED={total_removed}")
    print(f"DICTIONARY_RECONCILIATION_ADDED={total_added}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
