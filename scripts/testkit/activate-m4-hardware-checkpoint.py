#!/usr/bin/env python3

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import sys
import tempfile


SHA256_HEX = re.compile(r"^[0-9a-f]{64}$")
GIT_OID_HEX = re.compile(r"^(?:[0-9a-f]{40}|[0-9a-f]{64})$")


def die(message: str) -> "NoReturn":
    raise SystemExit(f"TESTKIT_ERROR={message}")


def parse_manifest(path: Path) -> dict[str, str]:
    if not path.is_file():
        die(f"manifest missing: {path}")

    result: dict[str, str] = {}

    for number, raw in enumerate(
        path.read_text(encoding="utf-8").splitlines(),
        start=1,
    ):
        line = raw.strip()

        if not line or line.startswith("#"):
            continue

        if "=" not in raw:
            die(
                f"manifest malformed line {number}: "
                f"{raw!r}"
            )

        key, value = raw.split("=", 1)
        key = key.strip()
        value = value.strip()

        if not re.fullmatch(r"[A-Z0-9_]+", key):
            die(f"invalid manifest key: {key}")

        if key in result:
            die(f"duplicate manifest key: {key}")

        result[key] = value

    return result


def require_fields(
    manifest: dict[str, str],
    names: list[str],
) -> None:
    for name in names:
        if not manifest.get(name):
            die(f"required manifest field missing: {name}")


def require_sha256(
    manifest: dict[str, str],
    name: str,
) -> None:
    value = manifest[name]

    if not SHA256_HEX.fullmatch(value):
        die(f"invalid SHA256 field {name}: {value}")


def require_git_oid(
    manifest: dict[str, str],
    name: str,
) -> None:
    value = manifest[name]

    if not GIT_OID_HEX.fullmatch(value):
        die(f"invalid Git object ID field {name}: {value}")


def read_lines(path: Path) -> list[str]:
    if not path.is_file():
        die(f"authority file missing: {path}")

    return path.read_text(
        encoding="utf-8"
    ).splitlines()


def get_last(
    path: Path,
    key: str,
) -> str:
    prefix = key + "="
    found: str | None = None

    for line in read_lines(path):
        if line.startswith(prefix):
            found = line[len(prefix):]

    if found is None:
        die(f"authority field missing: {path}:{key}")

    return found


def set_last(
    path: Path,
    key: str,
    value: str,
) -> None:
    lines = read_lines(path)
    prefix = key + "="
    indexes = [
        index
        for index, line in enumerate(lines)
        if line.startswith(prefix)
    ]

    if indexes:
        lines[indexes[-1]] = prefix + value
    else:
        if lines and lines[-1] != "":
            lines.append("")
        lines.append(prefix + value)

    atomic_write_lines(path, lines)


def atomic_write_lines(
    path: Path,
    lines: list[str],
) -> None:
    data = "\n".join(lines) + "\n"

    fd, temp_name = tempfile.mkstemp(
        prefix=f".{path.name}.",
        dir=str(path.parent),
        text=True,
    )

    temp = Path(temp_name)

    try:
        with os.fdopen(
            fd,
            "w",
            encoding="utf-8",
            newline="\n",
        ) as handle:
            handle.write(data)

        os.chmod(temp, path.stat().st_mode)
        os.replace(temp, path)

    finally:
        if temp.exists():
            temp.unlink()


def append_block_once(
    path: Path,
    marker: str,
    block: list[str],
) -> None:
    text = path.read_text(encoding="utf-8")

    if marker in text:
        return

    lines = text.rstrip().splitlines()
    lines.append("")
    lines.extend(block)
    atomic_write_lines(path, lines)


def replace_markdown_section(
    path: Path,
    heading: str,
    body: list[str],
) -> None:
    lines = read_lines(path)

    matches = [
        index
        for index, line in enumerate(lines)
        if line.strip() == heading
    ]

    if len(matches) != 1:
        die(
            "markdown section heading occurrence mismatch: "
            f"{path}:{heading} count={len(matches)}"
        )

    start = matches[0]
    end = len(lines)

    for index in range(start + 1, len(lines)):
        if lines[index].startswith("## "):
            end = index
            break

    replacement = [heading, ""] + body

    new_lines = (
        lines[:start]
        + replacement
        + [""]
        + lines[end:]
    )

    atomic_write_lines(path, new_lines)


def update_machine_mirror(
    path: Path,
    values: dict[str, str],
) -> None:
    lines = read_lines(path)

    try:
        start = next(
            index
            for index, line in enumerate(lines)
            if line.strip() == "## Machine-state mirror"
        )
    except StopIteration:
        die("docs machine-state mirror heading missing")

    end = len(lines)

    for index in range(start + 1, len(lines)):
        if lines[index].startswith("## "):
            end = index
            break

    for key, value in values.items():
        prefix = f"    {key}="
        matches = [
            index
            for index in range(start, end)
            if lines[index].startswith(prefix)
        ]

        if len(matches) != 1:
            die(
                "machine mirror field occurrence mismatch: "
                f"{key} count={len(matches)}"
            )

        lines[matches[0]] = prefix + value

    atomic_write_lines(path, lines)


def candidate_already_applied(
    authority: Path,
    migration: Path,
    docs: Path,
    status: Path,
    m: dict[str, str],
) -> bool:
    checks = [
        (
            authority,
            "CURRENT_NORMALIZATION_TRANCHE",
            m["TARGET_TRANCHE"],
        ),
        (
            authority,
            "CURRENT_SOURCE_COMMIT",
            m["SOURCE_COMMIT"],
        ),
        (
            authority,
            "CURRENT_SOURCE_AUTHORITY_COMMIT",
            m["SOURCE_AUTHORITY_COMMIT"],
        ),
        (
            authority,
            "CURRENT_WORKING_ELF_STATUS",
            "HARDWARE_PENDING",
        ),
        (
            authority,
            "CURRENT_WORKING_ELF_SHA256",
            m["CANDIDATE_ELF_SHA256"],
        ),
        (
            authority,
            "CURRENT_WORKING_PT_LOAD_SHA256",
            m["CANDIDATE_PT_LOAD_SHA256"],
        ),
        (
            authority,
            "CURRENT_HARDWARE_QUALIFICATION",
            "PENDING",
        ),
        (
            authority,
            "NEXT_ACTION",
            m["NEXT_ACTION"],
        ),
        (
            migration,
            "CURRENT_SOURCE_HEAD",
            m["SOURCE_COMMIT"],
        ),
        (
            migration,
            "CURRENT_DOC_HEAD",
            m["SOURCE_AUTHORITY_COMMIT"],
        ),
        (
            migration,
            "NEXT_ACTION",
            m["NEXT_ACTION"],
        ),
    ]

    for path, key, expected in checks:
        try:
            actual = get_last(path, key)
        except SystemExit:
            return False

        if actual != expected:
            return False

    marker = (
        f"CHECKPOINT_ID={m['CHECKPOINT_ID']}"
    )

    docs_text = docs.read_text(
        encoding="utf-8"
    )

    status_text = status.read_text(
        encoding="utf-8"
    )

    status_requirements = [
        marker,
        (
            "    LAST_HARDWARE_RESULT="
            f"{m['LAST_DIRECT_HARDWARE_RESULT']}"
        ),
        (
            "    LAST_VALIDATED_WORKING_ELF_SHA256="
            f"{m['LAST_VALIDATED_ELF_SHA256']}"
        ),
        (
            "    LAST_VALIDATED_PT_LOAD_SHA256="
            f"{m['LAST_VALIDATED_PT_LOAD_SHA256']}"
        ),
        (
            "    CURRENT_WORKING_ELF_SHA256="
            f"{m['CANDIDATE_ELF_SHA256']}"
        ),
        (
            "    CURRENT_WORKING_VALIDATION_BASIS="
            f"{m['CURRENT_WORKING_VALIDATION_BASIS']}"
        ),
        (
            "    NEXT_ACTION="
            f"{m['NEXT_ACTION']}"
        ),
        (
            "    BLOCKED_BY="
            f"{m['BLOCKED_BY']}"
        ),
    ]

    return (
        marker in docs_text
        and all(
            item in status_text
            for item in status_requirements
        )
    )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Activate an M4 source generation as "
            "hardware-pending without granting hardware authority."
        )
    )

    parser.add_argument("manifest")
    parser.add_argument(
        "--root",
        default=".",
        help="repository/state root; default current directory",
    )

    args = parser.parse_args()

    root = Path(args.root).resolve()
    manifest_path = Path(args.manifest)

    if not manifest_path.is_absolute():
        manifest_path = (
            Path.cwd() / manifest_path
        ).resolve()

    m = parse_manifest(manifest_path)

    required = [
        "ACTIVATION_MANIFEST_VERSION",
        "CHECKPOINT_ID",
        "NAMESPACE",
        "TARGET_TRANCHE",
        "SOURCE_COMMIT",
        "SOURCE_AUTHORITY_COMMIT",
        "SOURCE_EVIDENCE_MANIFEST_SHA256",
        "SOURCE_AUTHORITY_MANIFEST_SHA256",
        "PERMANENT_SOURCE",
        "PERMANENT_HEADER",
        "CANDIDATE_ELF_SHA256",
        "CANDIDATE_PT_LOAD_SHA256",
        "CANDIDATE_PT_LOAD_BYTES",
        "LAST_VALIDATED_ELF_SHA256",
        "LAST_VALIDATED_PT_LOAD_SHA256",
        "LAST_VALIDATED_PT_LOAD_BYTES",
        "CURRENT_HARDWARE_AUTHORITY_COMMIT",
        "LAST_DIRECT_HARDWARE_AUTHORITY",
        "LAST_DIRECT_HARDWARE_RESULT",
        "HARDWARE_TEST_ID",
        "HARDWARE_ELF_IDENTITY_SHA256",
        "HARDWARE_STAMPED_ELF_SHA256",
        "HARDWARE_STAMPED_PT_LOAD_SHA256",
        "HARDWARE_STAMPED_PT_LOAD_BYTES",
        "HARDWARE_PREP_MANIFEST_SHA256",
        "LAST_BUILD_RESULT",
        "BASELINE_STATUS",
        "NEXT_ACTION",
        "BLOCKED_BY",
        "CURRENT_WORKING_VALIDATION_BASIS",
        "DOC_HEADING",
        "EXPECT_CURRENT_SOURCE_COMMIT",
        "EXPECT_CURRENT_SOURCE_AUTHORITY_COMMIT",
        "EXPECT_CURRENT_WORKING_ELF_SHA256",
        "EXPECT_LAST_VALIDATED_ELF_SHA256",
        "EXPECT_NEXT_ACTION",
    ]

    require_fields(m, required)

    if m["ACTIVATION_MANIFEST_VERSION"] != "1":
        die(
            "unsupported activation manifest version: "
            + m["ACTIVATION_MANIFEST_VERSION"]
        )

    for name in [
        "SOURCE_COMMIT",
        "SOURCE_AUTHORITY_COMMIT",
        "CURRENT_HARDWARE_AUTHORITY_COMMIT",
        "EXPECT_CURRENT_SOURCE_COMMIT",
        "EXPECT_CURRENT_SOURCE_AUTHORITY_COMMIT",
    ]:
        require_git_oid(m, name)

    for name in [
        "SOURCE_EVIDENCE_MANIFEST_SHA256",
        "SOURCE_AUTHORITY_MANIFEST_SHA256",
        "CANDIDATE_ELF_SHA256",
        "CANDIDATE_PT_LOAD_SHA256",
        "LAST_VALIDATED_ELF_SHA256",
        "LAST_VALIDATED_PT_LOAD_SHA256",
        "HARDWARE_ELF_IDENTITY_SHA256",
        "HARDWARE_STAMPED_ELF_SHA256",
        "HARDWARE_STAMPED_PT_LOAD_SHA256",
        "HARDWARE_PREP_MANIFEST_SHA256",
        "EXPECT_CURRENT_WORKING_ELF_SHA256",
        "EXPECT_LAST_VALIDATED_ELF_SHA256",
    ]:
        require_sha256(m, name)

    for name in [
        "CANDIDATE_PT_LOAD_BYTES",
        "LAST_VALIDATED_PT_LOAD_BYTES",
        "HARDWARE_STAMPED_PT_LOAD_BYTES",
    ]:
        if not m[name].isdigit() or int(m[name]) <= 0:
            die(f"invalid byte count {name}: {m[name]}")

    architecture = (
        root / "runtime/M4_ARCHITECTURE_BASELINE.env"
    )
    authority = (
        root / "runtime/M4_SOURCE_AUTHORITY.env"
    )
    migration = (
        root / "runtime/MIGRATION_STATE.env"
    )
    docs = root / "docs/MIGRATION_STATE.md"
    status = root / "docs/status.md"

    for path in [
        architecture,
        authority,
        migration,
        docs,
        status,
    ]:
        if not path.is_file():
            die(f"required authority surface missing: {path}")

    if candidate_already_applied(
        authority,
        migration,
        docs,
        status,
        m,
    ):
        print(
            "TESTKIT_ACTIVATE_M4_HARDWARE_CHECKPOINT_VERSION=1"
        )
        print("ACTIVATION_RESULT=ALREADY_APPLIED")
        print(
            f"CHECKPOINT_ID={m['CHECKPOINT_ID']}"
        )
        print(
            "TESTKIT_ACTIVATE_M4_HARDWARE_CHECKPOINT=PASS"
        )
        return 0

    expected = {
        "CURRENT_SOURCE_COMMIT":
            m["EXPECT_CURRENT_SOURCE_COMMIT"],
        "CURRENT_SOURCE_AUTHORITY_COMMIT":
            m["EXPECT_CURRENT_SOURCE_AUTHORITY_COMMIT"],
        "CURRENT_WORKING_ELF_SHA256":
            m["EXPECT_CURRENT_WORKING_ELF_SHA256"],
        "LAST_VALIDATED_ELF_SHA256":
            m["EXPECT_LAST_VALIDATED_ELF_SHA256"],
        "NEXT_ACTION":
            m["EXPECT_NEXT_ACTION"],
    }

    for key, value in expected.items():
        actual = get_last(authority, key)

        if actual != value:
            die(
                "activation precondition mismatch: "
                f"{key} expected={value} actual={actual}"
            )

    if (
        m["LAST_VALIDATED_ELF_SHA256"]
        != m["EXPECT_LAST_VALIDATED_ELF_SHA256"]
    ):
        die(
            "manifest attempts to change last validated "
            "ELF during activation"
        )

    namespace = m["NAMESPACE"]

    set_last(
        authority,
        "CURRENT_NORMALIZATION_TRANCHE",
        m["TARGET_TRANCHE"],
    )
    set_last(
        authority,
        "CURRENT_SOURCE_COMMIT",
        m["SOURCE_COMMIT"],
    )
    set_last(
        authority,
        "CURRENT_SOURCE_AUTHORITY_COMMIT",
        m["SOURCE_AUTHORITY_COMMIT"],
    )
    set_last(
        authority,
        "CURRENT_HARDWARE_AUTHORITY_COMMIT",
        m["CURRENT_HARDWARE_AUTHORITY_COMMIT"],
    )
    set_last(
        authority,
        "CURRENT_WORKING_ELF_STATUS",
        "HARDWARE_PENDING",
    )
    set_last(
        authority,
        "CURRENT_WORKING_ELF_SHA256",
        m["CANDIDATE_ELF_SHA256"],
    )
    set_last(
        authority,
        "CURRENT_WORKING_PT_LOAD_SHA256",
        m["CANDIDATE_PT_LOAD_SHA256"],
    )
    set_last(
        authority,
        "CURRENT_WORKING_PT_LOAD_BYTES",
        m["CANDIDATE_PT_LOAD_BYTES"],
    )
    set_last(
        authority,
        "LAST_VALIDATED_ELF_SHA256",
        m["LAST_VALIDATED_ELF_SHA256"],
    )
    set_last(
        authority,
        "LAST_VALIDATED_PT_LOAD_SHA256",
        m["LAST_VALIDATED_PT_LOAD_SHA256"],
    )
    set_last(
        authority,
        "LAST_VALIDATED_PT_LOAD_BYTES",
        m["LAST_VALIDATED_PT_LOAD_BYTES"],
    )
    set_last(
        authority,
        "LAST_DIRECT_HARDWARE_AUTHORITY",
        m["LAST_DIRECT_HARDWARE_AUTHORITY"],
    )
    set_last(
        authority,
        "LAST_DIRECT_HARDWARE_RESULT",
        m["LAST_DIRECT_HARDWARE_RESULT"],
    )
    set_last(
        authority,
        "CURRENT_HARDWARE_QUALIFICATION",
        "PENDING",
    )
    set_last(
        authority,
        "CURRENT_HARDWARE_QUALIFICATION_MACHINE",
        "PENDING",
    )
    set_last(
        authority,
        "CURRENT_HARDWARE_QUALIFICATION_PHYSICAL",
        "PENDING",
    )
    set_last(
        authority,
        "NEXT_ACTION",
        m["NEXT_ACTION"],
    )
    set_last(
        authority,
        "BLOCKED_BY",
        m["BLOCKED_BY"],
    )

    authority_marker = (
        f"# {namespace} hardware-pending checkpoint"
    )

    authority_block = [
        authority_marker,
        f"{namespace}_SOURCE_COMMIT={m['SOURCE_COMMIT']}",
        (
            f"{namespace}_SOURCE_AUTHORITY_COMMIT="
            f"{m['SOURCE_AUTHORITY_COMMIT']}"
        ),
        (
            f"{namespace}_SOURCE_EVIDENCE_MANIFEST_SHA256="
            f"{m['SOURCE_EVIDENCE_MANIFEST_SHA256']}"
        ),
        (
            f"{namespace}_SOURCE_AUTHORITY_MANIFEST_SHA256="
            f"{m['SOURCE_AUTHORITY_MANIFEST_SHA256']}"
        ),
        (
            f"{namespace}_PERMANENT_SOURCE="
            f"{m['PERMANENT_SOURCE']}"
        ),
        (
            f"{namespace}_PERMANENT_HEADER="
            f"{m['PERMANENT_HEADER']}"
        ),
        (
            f"{namespace}_CANDIDATE_ELF_SHA256="
            f"{m['CANDIDATE_ELF_SHA256']}"
        ),
        (
            f"{namespace}_CANDIDATE_PT_LOAD_SHA256="
            f"{m['CANDIDATE_PT_LOAD_SHA256']}"
        ),
        (
            f"{namespace}_CANDIDATE_PT_LOAD_BYTES="
            f"{m['CANDIDATE_PT_LOAD_BYTES']}"
        ),
        f"{namespace}_HARDWARE_REQUIRED=YES",
        f"{namespace}_HARDWARE_QUALIFIED=NO",
        (
            f"{namespace}_HARDWARE_TEST_ID="
            f"{m['HARDWARE_TEST_ID']}"
        ),
        (
            f"{namespace}_HARDWARE_ELF_IDENTITY_SHA256="
            f"{m['HARDWARE_ELF_IDENTITY_SHA256']}"
        ),
        (
            f"{namespace}_HARDWARE_STAMPED_ELF_SHA256="
            f"{m['HARDWARE_STAMPED_ELF_SHA256']}"
        ),
        (
            f"{namespace}_HARDWARE_STAMPED_PT_LOAD_SHA256="
            f"{m['HARDWARE_STAMPED_PT_LOAD_SHA256']}"
        ),
        (
            f"{namespace}_HARDWARE_STAMPED_PT_LOAD_BYTES="
            f"{m['HARDWARE_STAMPED_PT_LOAD_BYTES']}"
        ),
        (
            f"{namespace}_HARDWARE_PREP_MANIFEST_SHA256="
            f"{m['HARDWARE_PREP_MANIFEST_SHA256']}"
        ),
    ]

    append_block_once(
        authority,
        authority_marker,
        authority_block,
    )

    set_last(
        migration,
        "CURRENT_SOURCE_HEAD",
        m["SOURCE_COMMIT"],
    )
    set_last(
        migration,
        "CURRENT_DOC_HEAD",
        m["SOURCE_AUTHORITY_COMMIT"],
    )
    set_last(
        migration,
        "LAST_BUILD_RESULT",
        m["LAST_BUILD_RESULT"],
    )
    set_last(
        migration,
        "LAST_HARDWARE_RESULT",
        m["LAST_DIRECT_HARDWARE_RESULT"],
    )
    set_last(
        migration,
        "LAST_VALIDATED_WORKING_ELF_SHA256",
        m["LAST_VALIDATED_ELF_SHA256"],
    )
    set_last(
        migration,
        "NEXT_ACTION",
        m["NEXT_ACTION"],
    )
    set_last(
        migration,
        "BLOCKED_BY",
        m["BLOCKED_BY"],
    )
    set_last(
        migration,
        "CURRENT_WORKING_ELF_SHA256",
        m["CANDIDATE_ELF_SHA256"],
    )
    set_last(
        migration,
        "CURRENT_WORKING_PT_LOAD_SHA256",
        m["CANDIDATE_PT_LOAD_SHA256"],
    )
    set_last(
        migration,
        "CURRENT_WORKING_PT_LOAD_BYTES",
        m["CANDIDATE_PT_LOAD_BYTES"],
    )
    set_last(
        migration,
        "CURRENT_WORKING_VALIDATION_BASIS",
        m["CURRENT_WORKING_VALIDATION_BASIS"],
    )

    set_last(
        architecture,
        "NORMALIZATION_TRANCHE_3_STATUS",
        m["BASELINE_STATUS"],
    )
    set_last(
        architecture,
        "NEXT_ACTION",
        m["NEXT_ACTION"],
    )

    baseline_marker = (
        f"# {namespace} hardware-pending checkpoint"
    )

    baseline_block = [
        baseline_marker,
        f"{namespace}_TARGET_TRANCHE={m['TARGET_TRANCHE']}",
        f"{namespace}_SOURCE_COMMIT={m['SOURCE_COMMIT']}",
        (
            f"{namespace}_SOURCE_AUTHORITY_COMMIT="
            f"{m['SOURCE_AUTHORITY_COMMIT']}"
        ),
        (
            f"{namespace}_SOURCE_EVIDENCE_MANIFEST_SHA256="
            f"{m['SOURCE_EVIDENCE_MANIFEST_SHA256']}"
        ),
        (
            f"{namespace}_SOURCE_AUTHORITY_MANIFEST_SHA256="
            f"{m['SOURCE_AUTHORITY_MANIFEST_SHA256']}"
        ),
        (
            f"{namespace}_PERMANENT_SOURCE="
            f"{m['PERMANENT_SOURCE']}"
        ),
        (
            f"{namespace}_PERMANENT_HEADER="
            f"{m['PERMANENT_HEADER']}"
        ),
        (
            f"{namespace}_ELF_SHA256="
            f"{m['CANDIDATE_ELF_SHA256']}"
        ),
        (
            f"{namespace}_PT_LOAD_SHA256="
            f"{m['CANDIDATE_PT_LOAD_SHA256']}"
        ),
        (
            f"{namespace}_PT_LOAD_BYTES="
            f"{m['CANDIDATE_PT_LOAD_BYTES']}"
        ),
        f"{namespace}_HARDWARE_REQUIRED=YES",
        f"{namespace}_HARDWARE_QUALIFIED=NO",
        (
            f"{namespace}_HARDWARE_TEST_ID="
            f"{m['HARDWARE_TEST_ID']}"
        ),
        (
            f"{namespace}_HARDWARE_STAMPED_ELF_SHA256="
            f"{m['HARDWARE_STAMPED_ELF_SHA256']}"
        ),
        (
            f"{namespace}_HARDWARE_STAMPED_PT_LOAD_SHA256="
            f"{m['HARDWARE_STAMPED_PT_LOAD_SHA256']}"
        ),
        (
            f"{namespace}_HARDWARE_PREP_MANIFEST_SHA256="
            f"{m['HARDWARE_PREP_MANIFEST_SHA256']}"
        ),
    ]

    append_block_once(
        architecture,
        baseline_marker,
        baseline_block,
    )

    mirror = {
        "CURRENT_SOURCE_HEAD":
            m["SOURCE_COMMIT"],
        "LAST_HARDWARE_RESULT":
            m["LAST_DIRECT_HARDWARE_RESULT"],
        "NEXT_ACTION":
            m["NEXT_ACTION"],
        "BLOCKED_BY":
            m["BLOCKED_BY"],
    }

    update_machine_mirror(
        docs,
        mirror,
    )

    doc_marker = (
        f"CHECKPOINT_ID={m['CHECKPOINT_ID']}"
    )

    doc_block = [
        f"## {m['DOC_HEADING']}",
        "",
        "Hardware checkpoint activation is pending direct PS2 qualification.",
        "",
        "Source authority:",
        "",
        f"    SOURCE_COMMIT={m['SOURCE_COMMIT']}",
        (
            "    SOURCE_AUTHORITY_COMMIT="
            f"{m['SOURCE_AUTHORITY_COMMIT']}"
        ),
        "",
        "Pristine reproducible candidate:",
        "",
        (
            "    ELF_SHA256="
            f"{m['CANDIDATE_ELF_SHA256']}"
        ),
        (
            "    PT_LOAD_SHA256="
            f"{m['CANDIDATE_PT_LOAD_SHA256']}"
        ),
        (
            "    PT_LOAD_BYTES="
            f"{m['CANDIDATE_PT_LOAD_BYTES']}"
        ),
        "",
        "Exact identity-stamped hardware DUT:",
        "",
        (
            "    TEST_ID="
            f"{m['HARDWARE_TEST_ID']}"
        ),
        (
            "    ELF_IDENTITY_SHA256="
            f"{m['HARDWARE_ELF_IDENTITY_SHA256']}"
        ),
        (
            "    STAMPED_ELF_SHA256="
            f"{m['HARDWARE_STAMPED_ELF_SHA256']}"
        ),
        (
            "    STAMPED_PT_LOAD_SHA256="
            f"{m['HARDWARE_STAMPED_PT_LOAD_SHA256']}"
        ),
        "",
        "The previous validated runtime remains authoritative until this",
        "checkpoint passes independent machine and physical qualification.",
        "",
        f"    LAST_VALIDATED_ELF_SHA256={m['LAST_VALIDATED_ELF_SHA256']}",
        f"    NEXT_ACTION={m['NEXT_ACTION']}",
        f"    BLOCKED_BY={m['BLOCKED_BY']}",
        f"    {doc_marker}",
    ]

    append_block_once(
        docs,
        doc_marker,
        doc_block,
    )

    status_focus = [
        (
            "A new M4 source generation is active as a "
            "hardware-pending checkpoint."
        ),
        "",
        (
            "Its source/build authority is established, but it "
            "does not replace the"
        ),
        (
            "previously validated runtime until independent "
            "machine and physical"
        ),
        "hardware qualification passes.",
        "",
        "Permanent ownership:",
        "",
        f"    {m['PERMANENT_SOURCE']}",
        f"    {m['PERMANENT_HEADER']}",
        "",
        "Pristine reproducible candidate:",
        "",
        (
            "    ELF_SHA256="
            f"{m['CANDIDATE_ELF_SHA256']}"
        ),
        (
            "    PT_LOAD_SHA256="
            f"{m['CANDIDATE_PT_LOAD_SHA256']}"
        ),
        (
            "    PT_LOAD_BYTES="
            f"{m['CANDIDATE_PT_LOAD_BYTES']}"
        ),
        "",
        "Exact identity-stamped hardware DUT:",
        "",
        (
            "    TEST_ID="
            f"{m['HARDWARE_TEST_ID']}"
        ),
        (
            "    ELF_IDENTITY_SHA256="
            f"{m['HARDWARE_ELF_IDENTITY_SHA256']}"
        ),
        (
            "    STAMPED_ELF_SHA256="
            f"{m['HARDWARE_STAMPED_ELF_SHA256']}"
        ),
        (
            "    STAMPED_PT_LOAD_SHA256="
            f"{m['HARDWARE_STAMPED_PT_LOAD_SHA256']}"
        ),
        "",
        (
            "Previous validated ELF remains:"
        ),
        "",
        (
            "    LAST_VALIDATED_WORKING_ELF_SHA256="
            f"{m['LAST_VALIDATED_ELF_SHA256']}"
        ),
        "",
        f"    NEXT_ACTION={m['NEXT_ACTION']}",
        f"    BLOCKED_BY={m['BLOCKED_BY']}",
        f"    CHECKPOINT_ID={m['CHECKPOINT_ID']}",
    ]

    replace_markdown_section(
        status,
        "## Current development focus",
        status_focus,
    )

    update_machine_mirror(
        status,
        {
            "LAST_HARDWARE_RESULT":
                m["LAST_DIRECT_HARDWARE_RESULT"],
            "LAST_VALIDATED_WORKING_ELF_SHA256":
                m["LAST_VALIDATED_ELF_SHA256"],
            "LAST_VALIDATED_PT_LOAD_SHA256":
                m["LAST_VALIDATED_PT_LOAD_SHA256"],
            "CURRENT_WORKING_ELF_SHA256":
                m["CANDIDATE_ELF_SHA256"],
            "CURRENT_WORKING_VALIDATION_BASIS":
                m["CURRENT_WORKING_VALIDATION_BASIS"],
            "NEXT_ACTION":
                m["NEXT_ACTION"],
            "BLOCKED_BY":
                m["BLOCKED_BY"],
        },
    )

    print(
        "TESTKIT_ACTIVATE_M4_HARDWARE_CHECKPOINT_VERSION=1"
    )
    print("ACTIVATION_RESULT=APPLIED")
    print(f"CHECKPOINT_ID={m['CHECKPOINT_ID']}")
    print(
        f"CURRENT_SOURCE_COMMIT={m['SOURCE_COMMIT']}"
    )
    print("CURRENT_WORKING_ELF_STATUS=HARDWARE_PENDING")
    print(
        "CURRENT_WORKING_ELF_SHA256="
        f"{m['CANDIDATE_ELF_SHA256']}"
    )
    print(
        "LAST_VALIDATED_ELF_SHA256="
        f"{m['LAST_VALIDATED_ELF_SHA256']}"
    )
    print("CURRENT_HARDWARE_QUALIFICATION=PENDING")
    print(f"NEXT_ACTION={m['NEXT_ACTION']}")
    print(
        "TESTKIT_ACTIVATE_M4_HARDWARE_CHECKPOINT=PASS"
    )

    return 0


if __name__ == "__main__":
    sys.exit(main())
