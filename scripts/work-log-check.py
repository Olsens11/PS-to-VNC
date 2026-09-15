#!/usr/bin/env python3
"""Validate immutable ledge worker-log filenames and required metadata."""

from __future__ import annotations

import re
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LOG_DIR = ROOT / "docs" / "ledge" / "work-log"

SLUG = r"[a-z0-9]+(?:-[a-z0-9]+)*"
FILENAME_RE = re.compile(
    rf"^(?P<stamp>\d{{8}}T\d{{6}}[+-]\d{{4}})__"
    rf"(?P<role>{SLUG})__(?P<work>{SLUG})__(?P<worker>{SLUG})\.md$"
)
ISO_RE = re.compile(
    r"^(?P<year>\d{4})-(?P<month>\d{2})-(?P<day>\d{2})T"
    r"(?P<hour>\d{2}):(?P<minute>\d{2}):(?P<second>\d{2})"
    r"(?P<sign>[+-])(?P<tz_hour>\d{2}):(?P<tz_minute>\d{2})$"
)
REQUIRED_KEYS = {
    "DOCUMENT",
    "LOG_FORMAT_REVISION",
    "STARTED_AT",
    "COMPLETED_AT",
    "ROLE_KEY",
    "WORK_ITEM_KEY",
    "WORKER_KEY",
    "STATUS",
    "STARTING_BRANCH_COMMIT",
    "ENDING_BRANCH_COMMIT",
    "SELF_PAUSED",
}
ALLOWED_STATUS = {"COMPLETED", "PARTIAL", "BLOCKED", "SELF_PAUSED", "NOOP"}


def parse_metadata(text: str) -> dict[str, str]:
    metadata: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        if re.fullmatch(r"[A-Z][A-Z0-9_]*", key):
            metadata.setdefault(key, value.strip())
    return metadata


def safe_stamp(timestamp: str) -> str | None:
    match = ISO_RE.fullmatch(timestamp)
    if match is None:
        return None
    return (
        f"{match.group('year')}{match.group('month')}{match.group('day')}T"
        f"{match.group('hour')}{match.group('minute')}{match.group('second')}"
        f"{match.group('sign')}{match.group('tz_hour')}{match.group('tz_minute')}"
    )


def check_log(path: Path) -> list[str]:
    errors: list[str] = []
    match = FILENAME_RE.fullmatch(path.name)
    if match is None:
        return [f"{path}: filename does not match canonical work-log grammar"]

    metadata = parse_metadata(path.read_text(encoding="utf-8"))
    missing = sorted(REQUIRED_KEYS - metadata.keys())
    if missing:
        errors.append(f"{path}: missing required metadata: {', '.join(missing)}")
        return errors

    if metadata["DOCUMENT"] != "LEDGE_WORK_LOG_ENTRY":
        errors.append(f"{path}: DOCUMENT must be LEDGE_WORK_LOG_ENTRY")
    if metadata["LOG_FORMAT_REVISION"] != "0001":
        errors.append(f"{path}: unsupported LOG_FORMAT_REVISION={metadata['LOG_FORMAT_REVISION']}")
    if metadata["STATUS"] not in ALLOWED_STATUS:
        errors.append(f"{path}: invalid STATUS={metadata['STATUS']}")
    if metadata["SELF_PAUSED"] not in {"YES", "NO"}:
        errors.append(f"{path}: SELF_PAUSED must be YES or NO")

    started_stamp = safe_stamp(metadata["STARTED_AT"])
    if started_stamp is None:
        errors.append(f"{path}: STARTED_AT must use ISO-8601 local time with numeric offset")
    elif started_stamp != match.group("stamp"):
        errors.append(
            f"{path}: filename stamp {match.group('stamp')} != STARTED_AT-derived {started_stamp}"
        )

    if safe_stamp(metadata["COMPLETED_AT"]) is None:
        errors.append(f"{path}: COMPLETED_AT must use ISO-8601 local time with numeric offset")

    expected = {
        "ROLE_KEY": match.group("role"),
        "WORK_ITEM_KEY": match.group("work"),
        "WORKER_KEY": match.group("worker"),
    }
    for key, filename_value in expected.items():
        if metadata[key] != filename_value:
            errors.append(
                f"{path}: filename {key}={filename_value} != metadata {metadata[key]}"
            )

    return errors


def main() -> int:
    if not LOG_DIR.is_dir():
        print(f"WORK_LOG_CHECK=FAIL missing_directory={LOG_DIR}")
        return 1

    errors: list[str] = []
    records = 0
    for path in sorted(LOG_DIR.glob("*.md")):
        if path.name == "README.md":
            continue
        records += 1
        errors.extend(check_log(path))

    if errors:
        for error in errors:
            print(error)
        print(f"WORK_LOG_CHECK=FAIL records={records} errors={len(errors)}")
        return 1

    print(f"WORK_LOG_CHECK=PASS records={records}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
