#!/usr/bin/env python3

from __future__ import annotations

import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]
TOOL = ROOT / "scripts/testkit/activate-m4-hardware-checkpoint.py"


def get_last(path: Path, key: str) -> str:
    prefix = key + "="
    found = None

    for line in path.read_text(
        encoding="utf-8"
    ).splitlines():
        if line.startswith(prefix):
            found = line[len(prefix):]

    if found is None:
        raise SystemExit(
            f"missing fixture field: {path}:{key}"
        )

    return found


def tree_digest(root: Path) -> str:
    digest = hashlib.sha256()

    for rel in [
        "runtime/M4_ARCHITECTURE_BASELINE.env",
        "runtime/M4_SOURCE_AUTHORITY.env",
        "runtime/MIGRATION_STATE.env",
        "docs/MIGRATION_STATE.md",
        "docs/status.md",
    ]:
        path = root / rel
        digest.update(rel.encode())
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")

    return digest.hexdigest()


def run_tool(args: list[str]) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        args,
        text=True,
        capture_output=True,
        check=False,
    )

    if result.returncode != 0:
        print(
            "===== ACTIVATION CHILD STDOUT =====",
            file=sys.stderr,
        )
        print(
            result.stdout,
            end="" if result.stdout.endswith("\n") else "\n",
            file=sys.stderr,
        )

        print(
            "===== ACTIVATION CHILD STDERR =====",
            file=sys.stderr,
        )
        print(
            result.stderr,
            end="" if result.stderr.endswith("\n") else "\n",
            file=sys.stderr,
        )

        raise SystemExit(
            f"activation child returned {result.returncode}"
        )

    return result


def main() -> int:
    # Syntax-check without creating __pycache__ in the live repository.
    compile(
        TOOL.read_text(encoding="utf-8"),
        str(TOOL),
        "exec",
    )

    with tempfile.TemporaryDirectory(
        prefix="ps-to-vnc-activation-self-test."
    ) as temp_name:
        temp = Path(temp_name)

        for rel in [
            "runtime/M4_ARCHITECTURE_BASELINE.env",
            "runtime/M4_SOURCE_AUTHORITY.env",
            "runtime/MIGRATION_STATE.env",
            "docs/MIGRATION_STATE.md",
            "docs/status.md",
        ]:
            src = ROOT / rel
            dst = temp / rel
            dst.parent.mkdir(
                parents=True,
                exist_ok=True,
            )
            shutil.copy2(src, dst)

        authority = (
            temp / "runtime/M4_SOURCE_AUTHORITY.env"
        )

        current_source = get_last(
            authority,
            "CURRENT_SOURCE_COMMIT",
        )
        current_source_authority = get_last(
            authority,
            "CURRENT_SOURCE_AUTHORITY_COMMIT",
        )
        current_elf = get_last(
            authority,
            "CURRENT_WORKING_ELF_SHA256",
        )
        last_validated = get_last(
            authority,
            "LAST_VALIDATED_ELF_SHA256",
        )
        last_validated_load = get_last(
            authority,
            "LAST_VALIDATED_PT_LOAD_SHA256",
        )
        last_validated_bytes = get_last(
            authority,
            "LAST_VALIDATED_PT_LOAD_BYTES",
        )
        hardware_commit = get_last(
            authority,
            "CURRENT_HARDWARE_AUTHORITY_COMMIT",
        )
        last_direct = get_last(
            authority,
            "LAST_DIRECT_HARDWARE_AUTHORITY",
        )
        last_direct_result = get_last(
            authority,
            "LAST_DIRECT_HARDWARE_RESULT",
        )
        current_next = get_last(
            authority,
            "NEXT_ACTION",
        )

        manifest = temp / "activation.env"

        fake = {
            # Git object IDs intentionally use the repository's current
            # 40-hex SHA-1 form. Content/evidence identities below remain
            # SHA-256 and therefore use 64 hex characters.
            "source": "1" * 40,
            "source_authority": "2" * 40,
            "source_evidence": "3" * 64,
            "source_authority_manifest": "4" * 64,
            "candidate_elf": "5" * 64,
            "candidate_load": "6" * 64,
            "identity": "7" * 64,
            "stamped_elf": "8" * 64,
            "stamped_load": "9" * 64,
            "prep": "a" * 64,
        }

        manifest.write_text(
            "\n".join(
                [
                    "ACTIVATION_MANIFEST_VERSION=1",
                    "CHECKPOINT_ID=SELFTEST-HW",
                    "NAMESPACE=SELFTEST_HW",
                    "TARGET_TRANCHE=SELFTEST_hardware_pending",
                    f"SOURCE_COMMIT={fake['source']}",
                    (
                        "SOURCE_AUTHORITY_COMMIT="
                        f"{fake['source_authority']}"
                    ),
                    (
                        "SOURCE_EVIDENCE_MANIFEST_SHA256="
                        f"{fake['source_evidence']}"
                    ),
                    (
                        "SOURCE_AUTHORITY_MANIFEST_SHA256="
                        f"{fake['source_authority_manifest']}"
                    ),
                    "PERMANENT_SOURCE=src/selftest.c",
                    "PERMANENT_HEADER=src/selftest.h",
                    (
                        "CANDIDATE_ELF_SHA256="
                        f"{fake['candidate_elf']}"
                    ),
                    (
                        "CANDIDATE_PT_LOAD_SHA256="
                        f"{fake['candidate_load']}"
                    ),
                    "CANDIDATE_PT_LOAD_BYTES=123456",
                    (
                        "LAST_VALIDATED_ELF_SHA256="
                        f"{last_validated}"
                    ),
                    (
                        "LAST_VALIDATED_PT_LOAD_SHA256="
                        f"{last_validated_load}"
                    ),
                    (
                        "LAST_VALIDATED_PT_LOAD_BYTES="
                        f"{last_validated_bytes}"
                    ),
                    (
                        "CURRENT_HARDWARE_AUTHORITY_COMMIT="
                        f"{hardware_commit}"
                    ),
                    (
                        "LAST_DIRECT_HARDWARE_AUTHORITY="
                        f"{last_direct}"
                    ),
                    (
                        "LAST_DIRECT_HARDWARE_RESULT="
                        f"{last_direct_result}"
                    ),
                    "HARDWARE_TEST_ID=SELFTEST-HW",
                    (
                        "HARDWARE_ELF_IDENTITY_SHA256="
                        f"{fake['identity']}"
                    ),
                    (
                        "HARDWARE_STAMPED_ELF_SHA256="
                        f"{fake['stamped_elf']}"
                    ),
                    (
                        "HARDWARE_STAMPED_PT_LOAD_SHA256="
                        f"{fake['stamped_load']}"
                    ),
                    "HARDWARE_STAMPED_PT_LOAD_BYTES=123456",
                    (
                        "HARDWARE_PREP_MANIFEST_SHA256="
                        f"{fake['prep']}"
                    ),
                    (
                        "LAST_BUILD_RESULT="
                        "SELFTEST_REPRODUCIBLE_HARDWARE_REQUIRED"
                    ),
                    (
                        "BASELINE_STATUS="
                        "SELFTEST_HARDWARE_PENDING"
                    ),
                    (
                        "NEXT_ACTION="
                        "SELFTEST_direct_hardware_qualification"
                    ),
                    (
                        "BLOCKED_BY="
                        "SELFTEST_NONIDENTICAL_PT_LOAD"
                    ),
                    (
                        "CURRENT_WORKING_VALIDATION_BASIS="
                        "PENDING_DIRECT_HARDWARE_QUALIFICATION"
                    ),
                    (
                        "DOC_HEADING="
                        "Self-test hardware checkpoint — pending"
                    ),
                    (
                        "EXPECT_CURRENT_SOURCE_COMMIT="
                        f"{current_source}"
                    ),
                    (
                        "EXPECT_CURRENT_SOURCE_AUTHORITY_COMMIT="
                        f"{current_source_authority}"
                    ),
                    (
                        "EXPECT_CURRENT_WORKING_ELF_SHA256="
                        f"{current_elf}"
                    ),
                    (
                        "EXPECT_LAST_VALIDATED_ELF_SHA256="
                        f"{last_validated}"
                    ),
                    (
                        "EXPECT_NEXT_ACTION="
                        f"{current_next}"
                    ),
                ]
            )
            + "\n",
            encoding="utf-8",
        )

        first = run_tool(
            [
                sys.executable,
                str(TOOL),
                "--root",
                str(temp),
                str(manifest),
            ]
        )

        if (
            "ACTIVATION_RESULT=APPLIED"
            not in first.stdout
        ):
            raise SystemExit(
                "first disposable activation did not apply"
            )

        activated = (
            temp / "runtime/M4_SOURCE_AUTHORITY.env"
        )

        expectations = {
            "CURRENT_SOURCE_COMMIT":
                fake["source"],
            "CURRENT_SOURCE_AUTHORITY_COMMIT":
                fake["source_authority"],
            "CURRENT_WORKING_ELF_STATUS":
                "HARDWARE_PENDING",
            "CURRENT_WORKING_ELF_SHA256":
                fake["candidate_elf"],
            "CURRENT_WORKING_PT_LOAD_SHA256":
                fake["candidate_load"],
            "LAST_VALIDATED_ELF_SHA256":
                last_validated,
            "LAST_VALIDATED_PT_LOAD_SHA256":
                last_validated_load,
            "CURRENT_HARDWARE_QUALIFICATION":
                "PENDING",
            "CURRENT_HARDWARE_QUALIFICATION_MACHINE":
                "PENDING",
            "CURRENT_HARDWARE_QUALIFICATION_PHYSICAL":
                "PENDING",
            "NEXT_ACTION":
                "SELFTEST_direct_hardware_qualification",
        }

        for key, expected in expectations.items():
            actual = get_last(
                activated,
                key,
            )

            if actual != expected:
                raise SystemExit(
                    "activation self-test mismatch: "
                    f"{key} expected={expected} actual={actual}"
                )

        if (
            get_last(
                activated,
                "LAST_VALIDATED_ELF_SHA256",
            )
            != last_validated
        ):
            raise SystemExit(
                "activation changed last validated authority"
            )

        after_first = tree_digest(temp)

        second = run_tool(
            [
                sys.executable,
                str(TOOL),
                "--root",
                str(temp),
                str(manifest),
            ]
        )

        if (
            "ACTIVATION_RESULT=ALREADY_APPLIED"
            not in second.stdout
        ):
            raise SystemExit(
                "second disposable activation was not idempotent"
            )

        after_second = tree_digest(temp)

        if after_first != after_second:
            raise SystemExit(
                "idempotent activation changed authority bytes"
            )

        docs = (
            temp / "docs/MIGRATION_STATE.md"
        ).read_text(
            encoding="utf-8"
        )

        if docs.count(
            "CHECKPOINT_ID=SELFTEST-HW"
        ) != 1:
            raise SystemExit(
                "checkpoint documentation marker count != 1"
            )

        status = (
            temp / "docs/status.md"
        ).read_text(
            encoding="utf-8"
        )

        status_required = [
            "## Current development focus",
            "hardware-pending checkpoint",
            "CHECKPOINT_ID=SELFTEST-HW",
            (
                "    LAST_HARDWARE_RESULT="
                f"{last_direct_result}"
            ),
            (
                "    LAST_VALIDATED_WORKING_ELF_SHA256="
                f"{last_validated}"
            ),
            (
                "    LAST_VALIDATED_PT_LOAD_SHA256="
                f"{last_validated_load}"
            ),
            (
                "    CURRENT_WORKING_ELF_SHA256="
                f"{fake['candidate_elf']}"
            ),
            (
                "    CURRENT_WORKING_VALIDATION_BASIS="
                "PENDING_DIRECT_HARDWARE_QUALIFICATION"
            ),
            (
                "    NEXT_ACTION="
                "SELFTEST_direct_hardware_qualification"
            ),
            (
                "    BLOCKED_BY="
                "SELFTEST_NONIDENTICAL_PT_LOAD"
            ),
        ]

        for required in status_required:
            if required not in status:
                raise SystemExit(
                    "status activation surface missing: "
                    f"{required}"
                )

        if status.count(
            "CHECKPOINT_ID=SELFTEST-HW"
        ) != 1:
            raise SystemExit(
                "status checkpoint marker count != 1"
            )

        print("TESTKIT_M4_ACTIVATION_SELF_TEST_VERSION=2")
        print("DISPOSABLE_AUTHORITY_ROOT=PASS")
        print("FAIL_CLOSED_PRECONDITIONS=PASS")
        print("HARDWARE_PENDING_TRANSITION=PASS")
        print("LAST_VALIDATED_PRESERVED=PASS")
        print("PREVIOUS_HARDWARE_AUTHORITY_PRESERVED=PASS")
        print("MACHINE_MIRROR_UPDATE=PASS")
        print("HUMAN_CHECKPOINT_RECORD=PASS")
        print("STATUS_CURRENT_FOCUS_UPDATE=PASS")
        print("STATUS_MACHINE_MIRROR_UPDATE=PASS")
        print("CURRENT_CONTINUITY_FIVE_SURFACE_SET=PASS")
        print("IDEMPOTENT_SECOND_RUN=PASS")
        print("PS_TO_VNC_M4_ACTIVATION_SELF_TEST=PASS")

    return 0


if __name__ == "__main__":
    sys.exit(main())
