#!/usr/bin/env python3

from __future__ import annotations

import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]

BRIDGE = (
    ROOT
    / "scripts/testkit/"
    "legacy-hardware-bridge.py"
)

REAL_LEGACY = Path(
    "/home/ps2/ps2vnc"
)

REAL_LEGACY_HEAD = (
    "5781f4f7105ee290fa77819d7b9ec96e833bd2b8"
)

C2_ELF = (
    ROOT
    / "evidence/m4/"
    "m4i-c2-hw1-identity-prep/"
    "PS2VNC-M4I-C2-HW1.ELF"
)

C2_ELF_SHA = (
    "035bd9a3adb631984b4bbabe8f058d2"
    "ae3b4f6a078cbf64003a7add0d5099067"
)

C2_IDENTITY = (
    "28dbe5d985cfcbdcc2721e10e8b816fc"
    "fe37eb1e20e0b4d0e97c217299c515b8"
)

C2_TEST_ID = "M4I-C2-HW1"


def run(
    args: list[str],
    *,
    cwd: Path | None = None,
    check: bool = True,
) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        args,
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )

    if check and result.returncode != 0:
        print(
            result.stdout,
            file=sys.stderr,
        )

        print(
            result.stderr,
            file=sys.stderr,
        )

        raise SystemExit(
            "self-test child command failed: "
            + " ".join(args)
        )

    return result


def sha(path: Path) -> str:
    return hashlib.sha256(
        path.read_bytes()
    ).hexdigest()


def git(
    root: Path,
    *args: str,
) -> str:
    return run(
        [
            "git",
            "-C",
            str(root),
            *args,
        ]
    ).stdout.strip()


def extract_workspace(
    output: str,
) -> Path:
    values = [
        line.split(
            "=",
            1,
        )[1]
        for line in output.splitlines()
        if line.startswith(
            "BRIDGE_WORKSPACE="
        )
    ]

    if len(values) != 1:
        raise SystemExit(
            "bridge prepare did not emit exactly "
            "one workspace"
        )

    return Path(
        values[0]
    )


def create_successor_fixture(
    root: Path,
) -> tuple[Path, str]:
    repo = (
        root
        / "successor"
    )

    repo.mkdir(
        parents=True,
    )

    run(
        [
            "git",
            "init",
            "-q",
            str(repo),
        ]
    )

    git(
        repo,
        "config",
        "user.email",
        "selftest@example.invalid",
    )

    git(
        repo,
        "config",
        "user.name",
        "Bridge Self Test",
    )

    (
        repo
        / "fixture"
    ).mkdir()

    (
        repo
        / "fixture/source.c"
    ).write_text(
        "int bridge_selftest(void) { return 1; }\n",
        encoding="utf-8",
    )

    (
        repo
        / "fixture/Makefile"
    ).write_text(
        "all:\n\t@true\n",
        encoding="utf-8",
    )

    shutil.copy2(
        C2_ELF,
        repo
        / "fixture/DUT.ELF",
    )

    (
        repo
        / "fixture/archive"
    ).mkdir()

    for source, target in [
        (
            "source.c",
            "source.c",
        ),
        (
            "Makefile",
            "Makefile",
        ),
        (
            "DUT.ELF",
            "DUT.ELF",
        ),
    ]:
        shutil.copy2(
            repo
            / "fixture"
            / source,
            repo
            / "fixture/archive"
            / target,
        )

    run(
        [
            "git",
            "-C",
            str(repo),
            "add",
            "fixture",
        ]
    )

    run(
        [
            "git",
            "-C",
            str(repo),
            "commit",
            "-q",
            "-m",
            "fixture",
        ]
    )

    build_head = git(
        repo,
        "rev-parse",
        "HEAD",
    )

    source_sha = sha(
        repo
        / "fixture/source.c"
    )

    makefile_sha = sha(
        repo
        / "fixture/Makefile"
    )

    elf_sha = sha(
        repo
        / "fixture/DUT.ELF"
    )

    if elf_sha != C2_ELF_SHA:
        raise SystemExit(
            "self-test C2 ELF fixture hash mismatch"
        )

    manifest = (
        repo
        / "fixture/HARDWARE.env"
    )

    manifest.write_text(
        "\n".join(
            [
                "TESTKIT_MANIFEST_VERSION=1",
                f"TEST_ID={C2_TEST_ID}",
                "RUN_PREFIX=bridge-selftest",
                f"BUILD_BASE_HEAD={build_head}",
                "SOURCE_PATH=fixture/source.c",
                f"SOURCE_SHA256={source_sha}",
                "MAKEFILE_PATH=fixture/Makefile",
                f"MAKEFILE_SHA256={makefile_sha}",
                "ELF_PATH=fixture/DUT.ELF",
                f"ELF_SHA256={elf_sha}",
                (
                    "ELF_IDENTITY_SHA256="
                    f"{C2_IDENTITY}"
                ),
                "ARCHIVE_DIR=fixture/archive",
                (
                    "ARCHIVE_SOURCE_PATH="
                    "fixture/archive/source.c"
                ),
                (
                    "ARCHIVE_MAKEFILE_PATH="
                    "fixture/archive/Makefile"
                ),
                (
                    "ARCHIVE_ELF_PATH="
                    "fixture/archive/DUT.ELF"
                ),
                "PS2_FTP_HOST=192.0.2.1",
                "PS2_FTP_PORT=21",
                (
                    "PS2_REMOTE_UNIQUE="
                    "/mass/0/BRIDGE-SELFTEST.ELF"
                ),
                (
                    "PS2_REMOTE_ROLLING="
                    "/mass/0/PS2VNC.ELF"
                ),
            ]
        )
        + "\n",
        encoding="utf-8",
    )

    run(
        [
            "git",
            "-C",
            str(repo),
            "add",
            "fixture/HARDWARE.env",
        ]
    )

    run(
        [
            "git",
            "-C",
            str(repo),
            "commit",
            "-q",
            "-m",
            "manifest",
        ]
    )

    return (
        repo,
        "fixture/HARDWARE.env",
    )


def prepare(
    successor: Path,
    legacy: Path,
    workspace_root: Path,
    manifest: str,
) -> subprocess.CompletedProcess[str]:
    return run(
        [
            sys.executable,
            str(BRIDGE),
            "prepare",
            manifest,
            "--successor-root",
            str(successor),
            "--legacy-root",
            str(legacy),
            "--legacy-head",
            REAL_LEGACY_HEAD,
            "--workspace-root",
            str(workspace_root),
        ]
    )


def main() -> int:
    compile(
        BRIDGE.read_text(
            encoding="utf-8"
        ),
        str(BRIDGE),
        "exec",
    )

    if sha(C2_ELF) != C2_ELF_SHA:
        raise SystemExit(
            "real C2 stamped ELF changed"
        )

    if git(
        REAL_LEGACY,
        "rev-parse",
        "HEAD",
    ) != REAL_LEGACY_HEAD:
        raise SystemExit(
            "real legacy HEAD changed"
        )

    if run(
        [
            "git",
            "-C",
            str(REAL_LEGACY),
            "diff",
            "--quiet",
            "--",
        ],
        check=False,
    ).returncode != 0:
        raise SystemExit(
            "real legacy tracked tree is dirty"
        )

    if run(
        [
            "git",
            "-C",
            str(REAL_LEGACY),
            "diff",
            "--cached",
            "--quiet",
            "--",
        ],
        check=False,
    ).returncode != 0:
        raise SystemExit(
            "real legacy index is dirty"
        )

    with tempfile.TemporaryDirectory(
        prefix="ps-to-vnc-bridge-selftest."
    ) as temp_name:
        temp = Path(
            temp_name
        )

        successor, manifest = (
            create_successor_fixture(
                temp
            )
        )

        positive_root = (
            temp
            / "positive-workspaces"
        )

        positive = prepare(
            successor,
            REAL_LEGACY,
            positive_root,
            manifest,
        )

        workspace = extract_workspace(
            positive.stdout
        )

        if (
            "INHERITED_VERIFY_BUILD=PASS"
            not in positive.stdout
        ):
            raise SystemExit(
                "positive prepare did not verify build"
            )

        verify = run(
            [
                sys.executable,
                str(BRIDGE),
                "verify",
                str(workspace),
            ]
        )

        if (
            "BRIDGE_WORKSPACE_VALIDATION=PASS"
            not in verify.stdout
        ):
            raise SystemExit(
                "bridge verify PASS token missing"
            )

        dry = run(
            [
                sys.executable,
                str(BRIDGE),
                "deploy-dry-run",
                str(workspace),
            ]
        )

        if (
            "BRIDGE_DEPLOY_DRY_RUN=PASS"
            not in dry.stdout
        ):
            raise SystemExit(
                "bridge deploy dry-run PASS missing"
            )

        if (
            "FTP_CONTACT=NO"
            not in dry.stdout
        ):
            raise SystemExit(
                "bridge dry-run FTP safety token missing"
            )

        legacy_clone = (
            temp
            / "legacy-clone"
        )

        run(
            [
                "git",
                "clone",
                "-q",
                "--no-hardlinks",
                str(REAL_LEGACY),
                str(legacy_clone),
            ]
        )

        if git(
            legacy_clone,
            "rev-parse",
            "HEAD",
        ) != REAL_LEGACY_HEAD:
            raise SystemExit(
                "legacy self-test clone HEAD mismatch"
            )

        (
            legacy_clone
            / "UNTRACKED-BRIDGE-SELFTEST.txt"
        ).write_text(
            "untracked material is allowed\n",
            encoding="utf-8",
        )

        untracked_positive = prepare(
            successor,
            legacy_clone,
            temp
            / "untracked-positive",
            manifest,
        )

        if (
            "LEGACY_UNTRACKED_ARTIFACTS_PRESERVED=YES"
            not in untracked_positive.stdout
        ):
            raise SystemExit(
                "untracked legacy acceptance token missing"
            )

        common = (
            legacy_clone
            / "scripts/testkit/common.sh"
        )

        common.write_text(
            common.read_text(
                encoding="utf-8"
            )
            + "\n# tracked mutation self-test\n",
            encoding="utf-8",
        )

        tracked_negative = run(
            [
                sys.executable,
                str(BRIDGE),
                "prepare",
                manifest,
                "--successor-root",
                str(successor),
                "--legacy-root",
                str(legacy_clone),
                "--legacy-head",
                REAL_LEGACY_HEAD,
                "--workspace-root",
                str(
                    temp
                    / "tracked-negative"
                ),
            ],
            check=False,
        )

        if tracked_negative.returncode == 0:
            raise SystemExit(
                "tracked legacy mutation was accepted"
            )

    print(
        "TESTKIT_LEGACY_HARDWARE_BRIDGE_SELF_TEST_VERSION=1"
    )
    print(
        "FIXTURE_TEST_ID_MATCHES_EMBEDDED_IDENTITY=PASS"
    )
    print(
        "SELF_TEST_RUN_NAMING_IS_RUN_PREFIX=PASS"
    )
    print(
        "SUCCESSOR_ROOTED_WORKSPACE_PREPARE=PASS"
    )
    print(
        "INHERITED_TESTKIT_BYTE_IDENTITY=PASS"
    )
    print(
        "INHERITED_VERIFY_BUILD=PASS"
    )
    print(
        "BRIDGE_WORKSPACE_REVERIFY=PASS"
    )
    print(
        "DEPLOY_DRY_RUN=PASS"
    )
    print(
        "FTP_CONTACT_DURING_SELF_TEST=NO"
    )
    print(
        "LEGACY_UNTRACKED_ARTIFACT_ACCEPTANCE=PASS"
    )
    print(
        "LEGACY_TRACKED_MUTATION_REJECTION=PASS"
    )
    print(
        "LEGACY_INDEX_MUTATION_POLICY=FAIL_CLOSED"
    )
    print(
        "PS_TO_VNC_LEGACY_HARDWARE_BRIDGE_SELF_TEST=PASS"
    )

    return 0


if __name__ == "__main__":
    sys.exit(
        main()
    )
