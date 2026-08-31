#!/usr/bin/env python3

from __future__ import annotations

import argparse
from pathlib import Path
import sys


def die(message: str) -> "NoReturn":
    raise SystemExit(
        f"TESTKIT_ERROR={message}"
    )


def read_env(path: Path) -> dict[str, str]:
    if not path.is_file():
        die(f"state file missing: {path}")

    result: dict[str, str] = {}

    for number, raw in enumerate(
        path.read_text(
            encoding="utf-8"
        ).splitlines(),
        start=1,
    ):
        line = raw.strip()

        if not line or line.startswith("#"):
            continue

        if "=" not in raw:
            die(
                f"malformed env line "
                f"{path}:{number}"
            )

        key, value = raw.split("=", 1)

        if key in result:
            die(
                f"duplicate env field "
                f"{path}:{key}"
            )

        result[key] = value

    return result


def require(
    state: dict[str, str],
    name: str,
    source: Path,
) -> str:
    value = state.get(name)

    if value is None or value == "":
        die(
            f"required field missing "
            f"{source}:{name}"
        )

    return value


def require_equal(
    left_name: str,
    left_value: str,
    right_name: str,
    right_value: str,
) -> None:
    if left_value != right_value:
        die(
            "authority mismatch "
            f"{left_name}={left_value} "
            f"{right_name}={right_value}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Validate coherent M4 migration/source authority "
            "state, including hardware-qualified and "
            "hardware-pending generations."
        )
    )

    parser.add_argument("migration_state")
    parser.add_argument("m4_authority")

    args = parser.parse_args()

    migration_path = Path(
        args.migration_state
    ).resolve()

    authority_path = Path(
        args.m4_authority
    ).resolve()

    migration = read_env(
        migration_path
    )

    authority = read_env(
        authority_path
    )

    stage = require(
        migration,
        "CURRENT_STAGE",
        migration_path,
    )

    if stage != "M4":
        die(
            f"unsupported stage for M4 state contract: {stage}"
        )

    migration_source = require(
        migration,
        "CURRENT_SOURCE_HEAD",
        migration_path,
    )

    authority_source = require(
        authority,
        "CURRENT_SOURCE_COMMIT",
        authority_path,
    )

    require_equal(
        "MIGRATION_CURRENT_SOURCE_HEAD",
        migration_source,
        "M4_CURRENT_SOURCE_COMMIT",
        authority_source,
    )

    migration_next = require(
        migration,
        "NEXT_ACTION",
        migration_path,
    )

    authority_next = require(
        authority,
        "NEXT_ACTION",
        authority_path,
    )

    require_equal(
        "MIGRATION_NEXT_ACTION",
        migration_next,
        "M4_NEXT_ACTION",
        authority_next,
    )

    migration_blocked = require(
        migration,
        "BLOCKED_BY",
        migration_path,
    )

    authority_blocked = require(
        authority,
        "BLOCKED_BY",
        authority_path,
    )

    require_equal(
        "MIGRATION_BLOCKED_BY",
        migration_blocked,
        "M4_BLOCKED_BY",
        authority_blocked,
    )

    status = require(
        authority,
        "CURRENT_WORKING_ELF_STATUS",
        authority_path,
    )

    qualification = require(
        authority,
        "CURRENT_HARDWARE_QUALIFICATION",
        authority_path,
    )

    machine = require(
        authority,
        "CURRENT_HARDWARE_QUALIFICATION_MACHINE",
        authority_path,
    )

    physical = require(
        authority,
        "CURRENT_HARDWARE_QUALIFICATION_PHYSICAL",
        authority_path,
    )

    if status == "HARDWARE_QUALIFIED":
        if migration_blocked != "NONE":
            die(
                "hardware-qualified M4 generation "
                "must have BLOCKED_BY=NONE"
            )

        if qualification != "PASS_MACHINE_AND_PHYSICAL":
            die(
                "hardware-qualified M4 generation "
                "must have "
                "CURRENT_HARDWARE_QUALIFICATION="
                "PASS_MACHINE_AND_PHYSICAL"
            )

        if machine == "PENDING":
            die(
                "hardware-qualified M4 generation "
                "cannot have machine qualification PENDING"
            )

        if physical == "PENDING":
            die(
                "hardware-qualified M4 generation "
                "cannot have physical qualification PENDING"
            )

        state_class = "HARDWARE_QUALIFIED"

    elif status == "HARDWARE_PENDING":
        if migration_blocked == "NONE":
            die(
                "hardware-pending M4 generation "
                "must record a concrete blocker"
            )

        if qualification != "PENDING":
            die(
                "hardware-pending M4 generation "
                "must have current qualification PENDING"
            )

        if machine != "PENDING":
            die(
                "hardware-pending M4 generation "
                "must have machine qualification PENDING"
            )

        if physical != "PENDING":
            die(
                "hardware-pending M4 generation "
                "must have physical qualification PENDING"
            )

        state_class = "HARDWARE_PENDING"

    else:
        die(
            "unsupported M4 CURRENT_WORKING_ELF_STATUS="
            f"{status}"
        )

    print(
        "TESTKIT_M4_AUTHORITY_STATE_CHECK_VERSION=1"
    )
    print(
        f"M4_AUTHORITY_STATE_CLASS={state_class}"
    )
    print(
        f"CURRENT_SOURCE_COMMIT={authority_source}"
    )
    print(
        f"NEXT_ACTION={authority_next}"
    )
    print(
        f"BLOCKED_BY={authority_blocked}"
    )
    print(
        f"CURRENT_HARDWARE_QUALIFICATION="
        f"{qualification}"
    )
    print(
        "M4_AUTHORITY_STATE_CONTRACT=PASS"
    )

    return 0


if __name__ == "__main__":
    sys.exit(main())
