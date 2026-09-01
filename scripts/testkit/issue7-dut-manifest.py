#!/usr/bin/env python3
"""Create a fail-closed Issue #7 DUT manifest from preparation evidence.

The manifest intentionally records only durable identity inputs. It does not
claim deployment, execution, machine evidence, or operator qualification.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
EXPECTED_BRANCH = "reconstruct/issue7-minimal-core"
BUILD_SCRIPT = ROOT / "scripts/build-issue7-clean.sh"
TRACKED_INPUTS = (
    "scripts/build-issue7-clean.sh",
    "mk/issue7-clean.mk",
    "scripts/testkit/prepare-hardware-elf.sh",
    "scripts/testkit/elf-identity.py",
    "scripts/testkit/pt-load-fingerprint.sh",
)
PREP_KEYS = (
    "PRISTINE_ELF",
    "PRISTINE_ELF_SHA256",
    "PRISTINE_ELF_BYTES",
    "HARDWARE_TEST_ID",
    "HARDWARE_ELF_IDENTITY_SHA256",
    "HARDWARE_STAMPED_ELF",
    "HARDWARE_STAMPED_ELF_SHA256",
    "HARDWARE_STAMPED_ELF_BYTES",
    "PRISTINE_PT_LOAD_SHA256",
    "PRISTINE_PT_LOAD_BYTES",
    "HARDWARE_STAMPED_PT_LOAD_SHA256",
    "HARDWARE_STAMPED_PT_LOAD_BYTES",
    "IDENTITY_STAMP_PT_LOAD_RELATION",
    "IDENTITY_STAMP_REPRODUCIBLE",
    "TESTKIT_PREPARE_HARDWARE_ELF",
)
SHA_KEYS = (
    "PRISTINE_ELF_SHA256",
    "HARDWARE_ELF_IDENTITY_SHA256",
    "HARDWARE_STAMPED_ELF_SHA256",
    "PRISTINE_PT_LOAD_SHA256",
    "HARDWARE_STAMPED_PT_LOAD_SHA256",
)


def fail(message: str) -> "NoReturn":
    raise SystemExit(f"ERROR: {message}")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def git(*args: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(ROOT), *args],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return result.stdout.strip()


def parse_prep(path: Path) -> dict[str, str]:
    values: dict[str, list[str]] = {key: [] for key in PREP_KEYS}
    for raw in path.read_text(encoding="utf-8").splitlines():
        for key in PREP_KEYS:
            prefix = key + "="
            if raw.startswith(prefix):
                values[key].append(raw[len(prefix) :])

    parsed: dict[str, str] = {}
    for key, found in values.items():
        if len(found) != 1 or found[0] == "":
            fail(f"expected exactly one non-empty {key} in {path}; found {len(found)}")
        parsed[key] = found[0]
    return parsed


def extract_build_authority() -> tuple[str, str]:
    text = BUILD_SCRIPT.read_text(encoding="utf-8")
    dep = re.search(r"^EXPECTED_DEP_SHA='([0-9a-f]{64})'$", text, re.MULTILINE)
    image = re.search(r"^IMAGE='([^']+@sha256:[0-9a-f]{64})'$", text, re.MULTILINE)
    if dep is None:
        fail("cannot extract EXPECTED_DEP_SHA from clean build script")
    if image is None:
        fail("cannot extract pinned PS2DEV IMAGE from clean build script")
    return dep.group(1), image.group(1)


def verify_hex_sha256(name: str, value: str) -> None:
    if re.fullmatch(r"[0-9a-f]{64}", value) is None:
        fail(f"{name} is not a lowercase SHA-256 value")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("preparation_log", type=Path)
    parser.add_argument("output_manifest", type=Path)
    args = parser.parse_args()

    prep_log = args.preparation_log.resolve()
    output = args.output_manifest.resolve()

    if not prep_log.is_file():
        fail(f"preparation log not found: {prep_log}")
    if output.exists():
        fail(f"output manifest already exists: {output}")
    if not output.parent.is_dir():
        fail(f"output directory not found: {output.parent}")

    # A DUT manifest must describe an exact committed source tree. Generated and
    # ignored build outputs are allowed; tracked working/staged changes are not.
    if git("diff", "--quiet") != "":
        fail("unexpected git diff output")
    if git("diff", "--cached", "--quiet") != "":
        fail("unexpected staged git diff output")

    head = git("rev-parse", "HEAD")
    if re.fullmatch(r"[0-9a-f]{40}", head) is None:
        fail("SOURCE_COMMIT is not a 40-character lowercase Git object ID")

    prep = parse_prep(prep_log)
    for key in SHA_KEYS:
        verify_hex_sha256(key, prep[key])

    if prep["IDENTITY_STAMP_REPRODUCIBLE"] != "YES":
        fail("preparation log does not prove reproducible stamping")
    if prep["TESTKIT_PREPARE_HARDWARE_ELF"] != "PASS":
        fail("preparation log is not a passing hardware-ELF preparation")

    pristine = Path(prep["PRISTINE_ELF"])
    stamped = Path(prep["HARDWARE_STAMPED_ELF"])
    if not pristine.is_file() or not stamped.is_file():
        fail("preparation log references a missing ELF")

    if sha256(pristine) != prep["PRISTINE_ELF_SHA256"]:
        fail("pristine ELF no longer matches preparation evidence")
    if sha256(stamped) != prep["HARDWARE_STAMPED_ELF_SHA256"]:
        fail("stamped ELF no longer matches preparation evidence")
    if pristine.stat().st_size != int(prep["PRISTINE_ELF_BYTES"]):
        fail("pristine ELF byte count no longer matches preparation evidence")
    if stamped.stat().st_size != int(prep["HARDWARE_STAMPED_ELF_BYTES"]):
        fail("stamped ELF byte count no longer matches preparation evidence")

    dep_sha, toolchain_image = extract_build_authority()

    fields: list[tuple[str, str]] = [
        ("ISSUE7_DUT_MANIFEST_VERSION", "1"),
        ("QUALIFICATION_SCOPE", "PRE_HARDWARE_IDENTITY_ONLY"),
        ("HARDWARE_QUALIFIED", "NO"),
        ("SOURCE_REPOSITORY", "Olsens11/PS-to-VNC"),
        ("SOURCE_BRANCH", EXPECTED_BRANCH),
        ("SOURCE_COMMIT", head),
        ("PS2DEV_IMAGE", toolchain_image),
        ("PS2IP_SHA256", dep_sha),
    ]

    for relative in TRACKED_INPUTS:
        key = relative.upper().replace("/", "_").replace("-", "_").replace(".", "_")
        fields.append((key + "_SHA256", sha256(ROOT / relative)))

    fields.extend(
        [
            ("HARDWARE_TEST_ID", prep["HARDWARE_TEST_ID"]),
            ("PRISTINE_ELF_SHA256", prep["PRISTINE_ELF_SHA256"]),
            ("PRISTINE_ELF_BYTES", prep["PRISTINE_ELF_BYTES"]),
            ("PRISTINE_PT_LOAD_SHA256", prep["PRISTINE_PT_LOAD_SHA256"]),
            ("PRISTINE_PT_LOAD_BYTES", prep["PRISTINE_PT_LOAD_BYTES"]),
            ("HARDWARE_ELF_IDENTITY_SHA256", prep["HARDWARE_ELF_IDENTITY_SHA256"]),
            ("HARDWARE_STAMPED_ELF_SHA256", prep["HARDWARE_STAMPED_ELF_SHA256"]),
            ("HARDWARE_STAMPED_ELF_BYTES", prep["HARDWARE_STAMPED_ELF_BYTES"]),
            ("HARDWARE_STAMPED_PT_LOAD_SHA256", prep["HARDWARE_STAMPED_PT_LOAD_SHA256"]),
            ("HARDWARE_STAMPED_PT_LOAD_BYTES", prep["HARDWARE_STAMPED_PT_LOAD_BYTES"]),
            ("IDENTITY_STAMP_PT_LOAD_RELATION", prep["IDENTITY_STAMP_PT_LOAD_RELATION"]),
            ("IDENTITY_STAMP_REPRODUCIBLE", prep["IDENTITY_STAMP_REPRODUCIBLE"]),
            ("DEPLOYED", "NO"),
            ("RUNTIME_IDENTITY_OBSERVED", "NO"),
            ("MACHINE_EVIDENCE_RESULT", "NOT_RUN"),
            ("OPERATOR_RESULT", "NOT_RUN"),
        ]
    )

    text = "".join(f"{key}={value}\n" for key, value in fields)
    output.write_text(text, encoding="utf-8")
    manifest_sha = hashlib.sha256(text.encode("utf-8")).hexdigest()

    print("ISSUE7_DUT_MANIFEST=PASS")
    print(f"ISSUE7_DUT_MANIFEST_PATH={output}")
    print(f"ISSUE7_DUT_MANIFEST_SHA256={manifest_sha}")
    print(f"SOURCE_COMMIT={head}")
    print(f"HARDWARE_TEST_ID={prep['HARDWARE_TEST_ID']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
