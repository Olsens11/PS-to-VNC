#!/usr/bin/env python3
"""File synopsis:
Deploy one exact PS2 ELF to a deterministic unique archival filename and the
stable ``/mass/0/PS2VNC.ELF`` launch path, then read both files back and prove
their SHA-256 and byte counts match the local artifact.

This tool owns only deployment mechanics and deployment evidence. It does not
build, stamp, launch, observe, qualify, or impose experiment-specific Git,
manifest, branch, or apparatus policy.
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import sys
import tempfile


DEFAULT_FTP_HOST = "192.168.50.2"
DEFAULT_FTP_PORT = 21
DEFAULT_ROLLING_PATH = "/mass/0/PS2VNC.ELF"
UNIQUE_PREFIX = "PS2VNC"
TEST_ID_RE = re.compile(r"[A-Za-z0-9._-]{1,63}")
SHA256_RE = re.compile(r"[0-9a-f]{64}")
REMOTE_NAME_RE = re.compile(r"/mass/0/[A-Za-z0-9._-]+")


class DeploymentError(RuntimeError):
    """Represent one fail-closed repository, identity, FTP, or evidence error."""


def sha256_file(path: Path) -> str:
    """Stream one file and return its lowercase SHA-256 digest."""

    digest = hashlib.sha256()

    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)

    return digest.hexdigest()


def run_git(candidate: Path, *arguments: str) -> str:
    """Run one read-only Git query used to validate repository context."""

    result = subprocess.run(
        ["git", "-C", str(candidate), *arguments],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    if result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or "no detail"
        raise DeploymentError(
            f"Git repository query failed for {candidate}: {detail}"
        )

    return result.stdout.strip()


def resolve_repository(raw: Path | None) -> tuple[Path, str]:
    """Resolve explicit or current-working-directory Git repository context."""

    if raw is None:
        candidate = Path.cwd()
        source = "AUTO_DISCOVERED"
    else:
        candidate = raw.expanduser()
        source = "EXPLICIT"

    top = run_git(
        candidate.resolve(),
        "rev-parse",
        "--show-toplevel",
    )

    repository = Path(top).resolve()

    if not repository.is_dir():
        raise DeploymentError(
            f"resolved repository does not exist: {repository}"
        )

    return repository, source


def resolve_local_path(
    raw: Path,
    repository: Path,
    *,
    must_exist: bool,
) -> Path:
    """Resolve a relative operand against the selected DUT repository."""

    expanded = raw.expanduser()

    if expanded.is_absolute():
        path = expanded
    else:
        path = repository / expanded

    path = path.resolve(strict=must_exist)

    if must_exist and not path.is_file():
        raise DeploymentError(f"file not found: {path}")

    return path


def repository_metadata(repository: Path) -> tuple[str, str]:
    """Return informative Git identity without imposing branch policy."""

    head = run_git(repository, "rev-parse", "HEAD")
    branch = run_git(repository, "branch", "--show-current")

    if not branch:
        branch = "DETACHED"

    return head, branch


def validate_test_id(value: str) -> str:
    """Require a deterministic filename-safe test identifier."""

    if TEST_ID_RE.fullmatch(value) is None:
        raise DeploymentError(
            "test ID must contain 1-63 letters, numbers, dots, underscores, "
            "or hyphens"
        )

    return value


def validate_remote_path(value: str, label: str) -> str:
    """Confine deployment targets to simple filenames under PS2 mass:/0."""

    if REMOTE_NAME_RE.fullmatch(value) is None:
        raise DeploymentError(
            f"{label} must be a simple absolute /mass/0 filename"
        )

    path = PurePosixPath(value)

    if ".." in path.parts:
        raise DeploymentError(f"{label} may not contain '..'")

    return value


def validate_expected_identity(
    elf: Path,
    expected_sha256: str | None,
    expected_bytes: int | None,
) -> tuple[str, int]:
    """Compute local identity and enforce optional caller-supplied expectations."""

    actual_sha256 = sha256_file(elf)
    actual_bytes = elf.stat().st_size

    if expected_sha256 is not None:
        if SHA256_RE.fullmatch(expected_sha256) is None:
            raise DeploymentError(
                "--expected-sha256 is not a lowercase SHA-256 value"
            )

        if actual_sha256 != expected_sha256:
            raise DeploymentError(
                "local ELF SHA-256 does not match --expected-sha256"
            )

    if expected_bytes is not None:
        if expected_bytes <= 0:
            raise DeploymentError("--expected-bytes must be positive")

        if actual_bytes != expected_bytes:
            raise DeploymentError(
                "local ELF byte count does not match --expected-bytes"
            )

    if actual_bytes <= 0:
        raise DeploymentError("local ELF is empty")

    return actual_sha256, actual_bytes


def curl_executable() -> str:
    """Select system curl; permit a fake only under the explicit self-test."""

    if os.environ.get("TESTKIT_SELFTEST") == "1":
        override = os.environ.get("PS2VNC_DEPLOY_CURL", "")

        if override:
            return override

    executable = shutil.which("curl")

    if executable is None:
        raise DeploymentError("curl is required for FTP deployment")

    return executable


def ftp_url(host: str, port: int, remote_path: str) -> str:
    """Build one FTP URL from validated private-link deployment inputs."""

    return f"ftp://{host}:{port}{remote_path}"


def run_curl(
    host: str,
    port: int,
    arguments: list[str],
    *,
    accepted_returncodes: tuple[int, ...] = (0,),
) -> subprocess.CompletedProcess[str]:
    """Run one bounded noninteractive curl operation."""

    command = [
        curl_executable(),
        "--fail",
        "--silent",
        "--show-error",
        "--connect-timeout",
        "5",
        "--max-time",
        "120",
        *arguments,
    ]

    result = subprocess.run(
        command,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    if result.returncode not in accepted_returncodes:
        detail = (
            result.stderr.strip()
            or result.stdout.strip()
            or "no detail"
        )

        raise DeploymentError(
            f"curl failed rc={result.returncode}: {detail}"
        )

    return result


def ensure_unique_remote_absent(
    host: str,
    port: int,
    unique_remote: str,
    probe_output: Path,
) -> None:
    """Fail closed rather than silently overwrite an archival unique target."""

    result = run_curl(
        host,
        port,
        [
            ftp_url(host, port, unique_remote),
            "-o",
            str(probe_output),
        ],
        accepted_returncodes=(0, 78),
    )

    if result.returncode == 0:
        raise DeploymentError(
            f"unique archival target already exists: {unique_remote}"
        )

    if result.returncode != 78:
        raise DeploymentError(
            "could not prove unique archival target is absent"
        )


def default_evidence_path(
    repository: Path,
    test_id: str,
    sha256: str,
) -> Path:
    """Choose a durable generated evidence path inside the selected DUT repo."""

    return (
        repository
        / "build"
        / "testkit"
        / "deployments"
        / f"{test_id}-{sha256[:8]}.json"
    )


def validate_evidence_path(path: Path) -> None:
    """Refuse overwriting prior deployment evidence."""

    if path.exists():
        raise DeploymentError(
            f"deployment evidence already exists: {path}"
        )


def write_evidence(
    destination: Path,
    record: dict[str, object],
) -> str:
    """Atomically write generic deployment evidence and return its SHA-256."""

    destination.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    validate_evidence_path(destination)

    text = (
        json.dumps(
            record,
            sort_keys=True,
            indent=2,
        )
        + "\n"
    )

    handle = tempfile.NamedTemporaryFile(
        mode="w",
        encoding="utf-8",
        dir=destination.parent,
        prefix=destination.name + ".",
        suffix=".tmp",
        delete=False,
    )

    temporary = Path(handle.name)

    try:
        with handle:
            handle.write(text)

        os.replace(
            temporary,
            destination,
        )
    finally:
        temporary.unlink(missing_ok=True)

    return hashlib.sha256(
        text.encode("utf-8")
    ).hexdigest()


def build_parser() -> argparse.ArgumentParser:
    """Define the generic deployment command-line contract."""

    parser = argparse.ArgumentParser(
        description=(
            "Deploy one exact ELF to unique and rolling PS2 mass-storage "
            "targets and verify both by readback."
        )
    )

    parser.add_argument(
        "--repo",
        type=Path,
        help=(
            "DUT Git worktree. Relative ELF/evidence paths resolve here. "
            "Without this option, the current working Git repository is used."
        ),
    )

    parser.add_argument(
        "--elf",
        type=Path,
        required=True,
    )

    parser.add_argument(
        "--test-id",
        required=True,
    )

    parser.add_argument(
        "--expected-sha256",
    )

    parser.add_argument(
        "--expected-bytes",
        type=int,
    )

    parser.add_argument(
        "--evidence",
        type=Path,
    )

    parser.add_argument(
        "--host",
        default=os.environ.get(
            "PS2VNC_FTP_HOST",
            DEFAULT_FTP_HOST,
        ),
    )

    parser.add_argument(
        "--port",
        type=int,
        default=int(
            os.environ.get(
                "PS2VNC_FTP_PORT",
                str(DEFAULT_FTP_PORT),
            )
        ),
    )

    parser.add_argument(
        "--rolling-path",
        default=DEFAULT_ROLLING_PATH,
    )

    parser.add_argument(
        "--dry-run",
        action="store_true",
    )

    parser.add_argument(
        "--operator-authorized",
        action="store_true",
    )

    return parser


def main() -> int:
    """Validate, plan, and optionally execute one generic dual-target deploy."""

    args = build_parser().parse_args()

    try:
        repository, repository_source = resolve_repository(
            args.repo
        )

        head, branch = repository_metadata(
            repository
        )

        elf = resolve_local_path(
            args.elf,
            repository,
            must_exist=True,
        )

        test_id = validate_test_id(
            args.test_id
        )

        rolling_remote = validate_remote_path(
            args.rolling_path,
            "rolling path",
        )

        if args.port <= 0 or args.port > 65535:
            raise DeploymentError(
                "FTP port must be between 1 and 65535"
            )

        local_sha256, local_bytes = (
            validate_expected_identity(
                elf,
                args.expected_sha256,
                args.expected_bytes,
            )
        )

        unique_remote = validate_remote_path(
            (
                f"/mass/0/{UNIQUE_PREFIX}-"
                f"{test_id}-{local_sha256[:8]}.ELF"
            ),
            "unique path",
        )

        if unique_remote == rolling_remote:
            raise DeploymentError(
                "unique and rolling paths must differ"
            )

        if args.evidence is None:
            evidence = default_evidence_path(
                repository,
                test_id,
                local_sha256,
            )
        else:
            evidence = resolve_local_path(
                args.evidence,
                repository,
                must_exist=False,
            )

        validate_evidence_path(
            evidence
        )

        print(
            f"REPOSITORY_CONTEXT={repository}"
        )
        print(
            f"REPOSITORY_CONTEXT_SOURCE={repository_source}"
        )
        print(
            f"REPOSITORY_HEAD={head}"
        )
        print(
            f"REPOSITORY_BRANCH={branch}"
        )
        print(
            f"ELF={elf}"
        )
        print(
            f"ELF_SHA256={local_sha256}"
        )
        print(
            f"ELF_BYTES={local_bytes}"
        )
        print(
            f"TEST_ID={test_id}"
        )
        print(
            f"FTP_HOST={args.host}"
        )
        print(
            f"FTP_PORT={args.port}"
        )
        print(
            f"UNIQUE_REMOTE={unique_remote}"
        )
        print(
            f"ROLLING_REMOTE={rolling_remote}"
        )
        print(
            f"DEPLOYMENT_EVIDENCE={evidence}"
        )

        if args.dry_run:
            print("FTP_CONTACT=NO")
            print("DEPLOY_ELF_DRY_RUN=PASS")
            return 0

        if not args.operator_authorized:
            raise DeploymentError(
                "live deployment requires --operator-authorized"
            )

        with tempfile.TemporaryDirectory(
            prefix="ps2vnc-deploy."
        ) as raw_temp:
            temporary = Path(raw_temp)

            snapshot = temporary / "local.ELF"

            shutil.copyfile(
                elf,
                snapshot,
            )

            snapshot_sha256 = sha256_file(
                snapshot
            )
            snapshot_bytes = snapshot.stat().st_size

            if (
                snapshot_sha256 != local_sha256
                or snapshot_bytes != local_bytes
            ):
                raise DeploymentError(
                    "local ELF changed while creating deployment snapshot"
                )

            unique_probe = temporary / "unique-probe.ELF"

            ensure_unique_remote_absent(
                args.host,
                args.port,
                unique_remote,
                unique_probe,
            )

            run_curl(
                args.host,
                args.port,
                [
                    "--upload-file",
                    str(snapshot),
                    ftp_url(
                        args.host,
                        args.port,
                        unique_remote,
                    ),
                ],
            )

            run_curl(
                args.host,
                args.port,
                [
                    "--upload-file",
                    str(snapshot),
                    ftp_url(
                        args.host,
                        args.port,
                        rolling_remote,
                    ),
                ],
            )

            unique_readback = (
                temporary / "unique-readback.ELF"
            )

            rolling_readback = (
                temporary / "rolling-readback.ELF"
            )

            run_curl(
                args.host,
                args.port,
                [
                    ftp_url(
                        args.host,
                        args.port,
                        unique_remote,
                    ),
                    "-o",
                    str(unique_readback),
                ],
            )

            run_curl(
                args.host,
                args.port,
                [
                    ftp_url(
                        args.host,
                        args.port,
                        rolling_remote,
                    ),
                    "-o",
                    str(rolling_readback),
                ],
            )

            unique_sha256 = sha256_file(
                unique_readback
            )
            unique_bytes = unique_readback.stat().st_size

            rolling_sha256 = sha256_file(
                rolling_readback
            )
            rolling_bytes = rolling_readback.stat().st_size

            if (
                unique_sha256 != local_sha256
                or unique_bytes != local_bytes
            ):
                raise DeploymentError(
                    "unique FTP readback does not match local ELF"
                )

            if (
                rolling_sha256 != local_sha256
                or rolling_bytes != local_bytes
            ):
                raise DeploymentError(
                    "rolling FTP readback does not match local ELF"
                )

        record: dict[str, object] = {
            "version": 1,
            "deployment_result": "PASS",
            "deployed_at_utc": (
                datetime.now(timezone.utc)
                .isoformat()
            ),
            "repository_context": str(
                repository
            ),
            "repository_context_source": (
                repository_source
            ),
            "repository_head": head,
            "repository_branch": branch,
            "test_id": test_id,
            "elf_path": str(elf),
            "elf_sha256": local_sha256,
            "elf_bytes": local_bytes,
            "ftp_host": args.host,
            "ftp_port": args.port,
            "unique_remote": unique_remote,
            "rolling_remote": rolling_remote,
            "unique_readback_sha256": (
                unique_sha256
            ),
            "unique_readback_bytes": (
                unique_bytes
            ),
            "rolling_readback_sha256": (
                rolling_sha256
            ),
            "rolling_readback_bytes": (
                rolling_bytes
            ),
            "ftp_readback": "PASS",
            "hardware_run": False,
        }

        evidence_sha256 = write_evidence(
            evidence,
            record,
        )

        print(
            f"UNIQUE_READBACK_SHA256={unique_sha256}"
        )
        print(
            f"UNIQUE_READBACK_BYTES={unique_bytes}"
        )
        print(
            f"ROLLING_READBACK_SHA256={rolling_sha256}"
        )
        print(
            f"ROLLING_READBACK_BYTES={rolling_bytes}"
        )
        print(
            f"DEPLOYMENT_EVIDENCE_SHA256={evidence_sha256}"
        )
        print("FTP_CONTACT=YES")
        print("DEPLOY_ELF=PASS")

        return 0

    except DeploymentError as error:
        print(
            f"DEPLOY_ELF_ERROR={error}",
            file=sys.stderr,
        )
        return 1


if __name__ == "__main__":
    sys.exit(main())
