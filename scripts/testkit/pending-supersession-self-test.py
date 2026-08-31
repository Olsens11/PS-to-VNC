#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


FIVE_SURFACES = (
    Path("runtime/M4_ARCHITECTURE_BASELINE.env"),
    Path("runtime/M4_SOURCE_AUTHORITY.env"),
    Path("runtime/MIGRATION_STATE.env"),
    Path("docs/MIGRATION_STATE.md"),
    Path("docs/status.md"),
)


def die(message: str) -> None:
    raise SystemExit(message)


def parse_env(path: Path) -> dict[str, str]:
    values: dict[str, str] = {}

    for raw in path.read_text(
        encoding="utf-8",
        errors="strict",
    ).splitlines():
        line = raw.strip()

        if not line or line.startswith("#") or "=" not in line:
            continue

        key, value = line.split("=", 1)
        values[key.strip()] = value.strip()

    return values


def sha256(path: Path) -> str:
    digest = hashlib.sha256()

    with path.open("rb") as handle:
        for block in iter(
            lambda: handle.read(1024 * 1024),
            b"",
        ):
            digest.update(block)

    return digest.hexdigest()


def require_equal(
    label: str,
    actual: str | None,
    expected: str,
) -> None:
    if actual != expected:
        die(
            f"{label} mismatch: "
            f"expected={expected} actual={actual}"
        )


def run_checked(
    command: list[str],
) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        command,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )

    if result.stdout:
        print(result.stdout, end="")

    if result.stderr:
        print(
            result.stderr,
            end="",
            file=sys.stderr,
        )

    if result.returncode != 0:
        die(
            "child command failed "
            f"rc={result.returncode}: "
            + " ".join(command)
        )

    return result


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Prove an M4 hardware-pending checkpoint can be "
            "superseded by another hardware-pending checkpoint "
            "using the canonical activator without touching the "
            "live five continuity surfaces."
        )
    )

    parser.add_argument(
        "manifest",
        help="candidate activation manifest",
    )

    parser.add_argument(
        "--root",
        default=".",
        help="live repository root; default current directory",
    )

    args = parser.parse_args()

    root = Path(args.root).resolve()
    manifest = Path(args.manifest).resolve()

    activator = (
        root
        / "scripts/testkit/activate-m4-hardware-checkpoint.py"
    )

    state_checker = (
        root
        / "scripts/testkit/m4-authority-state-check.py"
    )

    if not manifest.is_file():
        die(f"manifest not found: {manifest}")

    if not activator.is_file():
        die(f"activator not found: {activator}")

    if not state_checker.is_file():
        die(f"state checker not found: {state_checker}")

    live_authority = parse_env(
        root / "runtime/M4_SOURCE_AUTHORITY.env"
    )

    live_migration = parse_env(
        root / "runtime/MIGRATION_STATE.env"
    )

    candidate = parse_env(manifest)

    require_equal(
        "live CURRENT_WORKING_ELF_STATUS",
        live_authority.get(
            "CURRENT_WORKING_ELF_STATUS"
        ),
        "HARDWARE_PENDING",
    )

    require_equal(
        "live CURRENT_HARDWARE_QUALIFICATION",
        live_authority.get(
            "CURRENT_HARDWARE_QUALIFICATION"
        ),
        "PENDING",
    )

    preconditions = (
        (
            "EXPECT_CURRENT_SOURCE_COMMIT",
            live_authority.get(
                "CURRENT_SOURCE_COMMIT"
            ),
        ),
        (
            "EXPECT_CURRENT_SOURCE_AUTHORITY_COMMIT",
            live_authority.get(
                "CURRENT_SOURCE_AUTHORITY_COMMIT"
            ),
        ),
        (
            "EXPECT_CURRENT_WORKING_ELF_SHA256",
            live_authority.get(
                "CURRENT_WORKING_ELF_SHA256"
            ),
        ),
        (
            "EXPECT_LAST_VALIDATED_ELF_SHA256",
            live_authority.get(
                "LAST_VALIDATED_ELF_SHA256"
            ),
        ),
        (
            "EXPECT_NEXT_ACTION",
            live_migration.get(
                "NEXT_ACTION"
            ),
        ),
    )

    for manifest_key, live_value in preconditions:
        if live_value is None:
            die(
                f"live state missing value for {manifest_key}"
            )

        require_equal(
            manifest_key,
            candidate.get(manifest_key),
            live_value,
        )

    if (
        candidate.get("CANDIDATE_ELF_SHA256")
        ==
        live_authority.get(
            "CURRENT_WORKING_ELF_SHA256"
        )
    ):
        die(
            "candidate ELF does not supersede current "
            "pending candidate"
        )

    live_hashes = {
        relative: sha256(root / relative)
        for relative in FIVE_SURFACES
    }

    preserved = {
        "LAST_VALIDATED_ELF_SHA256":
            live_authority.get(
                "LAST_VALIDATED_ELF_SHA256"
            ),
        "LAST_VALIDATED_PT_LOAD_SHA256":
            live_authority.get(
                "LAST_VALIDATED_PT_LOAD_SHA256"
            ),
        "LAST_VALIDATED_PT_LOAD_BYTES":
            live_authority.get(
                "LAST_VALIDATED_PT_LOAD_BYTES"
            ),
        "CURRENT_HARDWARE_AUTHORITY_COMMIT":
            live_authority.get(
                "CURRENT_HARDWARE_AUTHORITY_COMMIT"
            ),
        "LAST_DIRECT_HARDWARE_AUTHORITY":
            live_authority.get(
                "LAST_DIRECT_HARDWARE_AUTHORITY"
            ),
        "LAST_DIRECT_HARDWARE_RESULT":
            live_authority.get(
                "LAST_DIRECT_HARDWARE_RESULT"
            ),
    }

    with tempfile.TemporaryDirectory(
        prefix="ps-to-vnc-pending-supersession-"
    ) as directory:
        disposable = Path(directory)

        for relative in FIVE_SURFACES:
            destination = disposable / relative
            destination.parent.mkdir(
                parents=True,
                exist_ok=True,
            )

            shutil.copy2(
                root / relative,
                destination,
            )

        first = run_checked(
            [
                sys.executable,
                str(activator),
                "--root",
                str(disposable),
                str(manifest),
            ]
        )

        if (
            "ACTIVATION_RESULT=APPLIED"
            not in first.stdout
        ):
            die(
                "disposable supersession did not report "
                "ACTIVATION_RESULT=APPLIED"
            )

        after_authority = parse_env(
            disposable
            / "runtime/M4_SOURCE_AUTHORITY.env"
        )

        after_migration = parse_env(
            disposable
            / "runtime/MIGRATION_STATE.env"
        )

        expected_after = {
            "CURRENT_SOURCE_COMMIT":
                candidate["SOURCE_COMMIT"],
            "CURRENT_SOURCE_AUTHORITY_COMMIT":
                candidate[
                    "SOURCE_AUTHORITY_COMMIT"
                ],
            "CURRENT_WORKING_ELF_STATUS":
                "HARDWARE_PENDING",
            "CURRENT_WORKING_ELF_SHA256":
                candidate[
                    "CANDIDATE_ELF_SHA256"
                ],
            "CURRENT_WORKING_PT_LOAD_SHA256":
                candidate[
                    "CANDIDATE_PT_LOAD_SHA256"
                ],
            "CURRENT_HARDWARE_QUALIFICATION":
                "PENDING",
        }

        for key, expected in expected_after.items():
            require_equal(
                f"disposable {key}",
                after_authority.get(key),
                expected,
            )

        require_equal(
            "disposable NEXT_ACTION",
            after_migration.get("NEXT_ACTION"),
            candidate["NEXT_ACTION"],
        )

        require_equal(
            "disposable BLOCKED_BY",
            after_migration.get("BLOCKED_BY"),
            candidate["BLOCKED_BY"],
        )

        for key, expected in preserved.items():
            if expected is None:
                die(
                    f"live authority missing preserved key {key}"
                )

            require_equal(
                f"preserved {key}",
                after_authority.get(key),
                expected,
            )

        state = run_checked(
            [
                sys.executable,
                str(state_checker),
                str(
                    disposable
                    / "runtime/MIGRATION_STATE.env"
                ),
                str(
                    disposable
                    / "runtime/M4_SOURCE_AUTHORITY.env"
                ),
            ]
        )

        if (
            "M4_AUTHORITY_STATE_CONTRACT=PASS"
            not in state.stdout
        ):
            die(
                "shared M4 state checker did not pass "
                "disposable supersession"
            )

        second = run_checked(
            [
                sys.executable,
                str(activator),
                "--root",
                str(disposable),
                str(manifest),
            ]
        )

        if (
            "ACTIVATION_RESULT=ALREADY_APPLIED"
            not in second.stdout
        ):
            die(
                "second disposable activation did not "
                "prove idempotence"
            )

    for relative, expected_hash in live_hashes.items():
        actual_hash = sha256(root / relative)

        if actual_hash != expected_hash:
            die(
                "live continuity surface mutated during "
                f"disposable self-test: {relative}"
            )

    print(
        "PENDING_TO_PENDING_PRECONDITION_MATCH=PASS"
    )

    print(
        "PENDING_TO_PENDING_TRANSITION=PASS"
    )

    print(
        "PENDING_TO_PENDING_LAST_VALIDATED_PRESERVED=PASS"
    )

    print(
        "PENDING_TO_PENDING_HARDWARE_AUTHORITY_PRESERVED=PASS"
    )

    print(
        "PENDING_TO_PENDING_SHARED_STATE_CONTRACT=PASS"
    )

    print(
        "PENDING_TO_PENDING_IDEMPOTENCE=PASS"
    )

    print(
        "LIVE_FIVE_SURFACES_IMMUTABLE_DURING_SELF_TEST=PASS"
    )

    print(
        "PS_TO_VNC_PENDING_SUPERSESSION_SELF_TEST=PASS"
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
