#!/usr/bin/env python3

from __future__ import annotations

from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]

TOOL = (
    ROOT
    / "scripts/testkit/m4-authority-state-check.py"
)


def write_env(
    path: Path,
    values: dict[str, str],
) -> None:
    path.write_text(
        "\n".join(
            f"{key}={value}"
            for key, value in values.items()
        )
        + "\n",
        encoding="utf-8",
    )


def run_case(
    name: str,
    migration: dict[str, str],
    authority: dict[str, str],
    should_pass: bool,
) -> None:
    with tempfile.TemporaryDirectory(
        prefix="m4-state-contract."
    ) as temp_name:
        temp = Path(temp_name)

        migration_path = (
            temp / "MIGRATION_STATE.env"
        )

        authority_path = (
            temp / "M4_SOURCE_AUTHORITY.env"
        )

        write_env(
            migration_path,
            migration,
        )

        write_env(
            authority_path,
            authority,
        )

        result = subprocess.run(
            [
                sys.executable,
                str(TOOL),
                str(migration_path),
                str(authority_path),
            ],
            text=True,
            capture_output=True,
            check=False,
        )

        actual_pass = (
            result.returncode == 0
        )

        if actual_pass != should_pass:
            print(
                f"CASE={name}",
                file=sys.stderr,
            )
            print(
                "----- STDOUT -----",
                file=sys.stderr,
            )
            print(
                result.stdout,
                file=sys.stderr,
            )
            print(
                "----- STDERR -----",
                file=sys.stderr,
            )
            print(
                result.stderr,
                file=sys.stderr,
            )

            raise SystemExit(
                f"case {name} expected "
                f"pass={should_pass} "
                f"actual={actual_pass}"
            )

        print(
            f"M4_STATE_CASE_{name}=PASS"
        )


def main() -> int:
    compile(
        TOOL.read_text(
            encoding="utf-8"
        ),
        str(TOOL),
        "exec",
    )

    source = "1" * 40

    qualified_migration = {
        "CURRENT_STAGE": "M4",
        "CURRENT_SOURCE_HEAD": source,
        "NEXT_ACTION": "M4_NEXT",
        "BLOCKED_BY": "NONE",
    }

    qualified_authority = {
        "CURRENT_SOURCE_COMMIT": source,
        "NEXT_ACTION": "M4_NEXT",
        "BLOCKED_BY": "NONE",
        "CURRENT_WORKING_ELF_STATUS":
            "HARDWARE_QUALIFIED",
        "CURRENT_HARDWARE_QUALIFICATION":
            "PASS_MACHINE_AND_PHYSICAL",
        "CURRENT_HARDWARE_QUALIFICATION_MACHINE":
            "PASS_5_OF_5",
        "CURRENT_HARDWARE_QUALIFICATION_PHYSICAL":
            "FULL_PASS",
    }

    pending_migration = {
        **qualified_migration,
        "NEXT_ACTION": "RUN_HARDWARE",
        "BLOCKED_BY": "NONIDENTICAL_PT_LOAD",
    }

    pending_authority = {
        **qualified_authority,
        "NEXT_ACTION": "RUN_HARDWARE",
        "BLOCKED_BY": "NONIDENTICAL_PT_LOAD",
        "CURRENT_WORKING_ELF_STATUS":
            "HARDWARE_PENDING",
        "CURRENT_HARDWARE_QUALIFICATION":
            "PENDING",
        "CURRENT_HARDWARE_QUALIFICATION_MACHINE":
            "PENDING",
        "CURRENT_HARDWARE_QUALIFICATION_PHYSICAL":
            "PENDING",
    }

    run_case(
        "QUALIFIED_ACCEPTED",
        qualified_migration,
        qualified_authority,
        True,
    )

    run_case(
        "PENDING_ACCEPTED",
        pending_migration,
        pending_authority,
        True,
    )

    run_case(
        "PENDING_WITH_NONE_BLOCKER_REJECTED",
        {
            **pending_migration,
            "BLOCKED_BY": "NONE",
        },
        {
            **pending_authority,
            "BLOCKED_BY": "NONE",
        },
        False,
    )

    run_case(
        "QUALIFIED_WITH_BLOCKER_REJECTED",
        {
            **qualified_migration,
            "BLOCKED_BY": "SHOULD_NOT_EXIST",
        },
        {
            **qualified_authority,
            "BLOCKED_BY": "SHOULD_NOT_EXIST",
        },
        False,
    )

    run_case(
        "PENDING_WITH_PASS_QUALIFICATION_REJECTED",
        pending_migration,
        {
            **pending_authority,
            "CURRENT_HARDWARE_QUALIFICATION":
                "PASS_MACHINE_AND_PHYSICAL",
        },
        False,
    )

    run_case(
        "NEXT_ACTION_MISMATCH_REJECTED",
        pending_migration,
        {
            **pending_authority,
            "NEXT_ACTION": "DIFFERENT",
        },
        False,
    )

    run_case(
        "UNSUPPORTED_STATUS_REJECTED",
        pending_migration,
        {
            **pending_authority,
            "CURRENT_WORKING_ELF_STATUS":
                "UNKNOWN_STATE",
        },
        False,
    )

    print(
        "TESTKIT_M4_AUTHORITY_STATE_SELF_TEST_VERSION=1"
    )
    print(
        "QUALIFIED_STATE_CONTRACT=PASS"
    )
    print(
        "HARDWARE_PENDING_STATE_CONTRACT=PASS"
    )
    print(
        "FAIL_CLOSED_NEGATIVE_CASES=PASS"
    )
    print(
        "PS_TO_VNC_M4_AUTHORITY_STATE_SELF_TEST=PASS"
    )

    return 0


if __name__ == "__main__":
    sys.exit(main())
