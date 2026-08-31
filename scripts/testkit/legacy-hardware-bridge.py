#!/usr/bin/env python3

from __future__ import annotations

import argparse
from datetime import datetime
import hashlib
import os
from pathlib import Path, PurePosixPath
import shlex
import shutil
import stat
import subprocess
import sys


BRIDGE_VERSION = 1

DEFAULT_LEGACY_ROOT = Path(
    "/home/ps2/ps2vnc"
)

DEFAULT_LEGACY_HEAD = (
    "5781f4f7105ee290fa77819d7b9ec96e833bd2b8"
)

DEFAULT_WORKSPACE_ROOT = Path(
    "/home/ps2/.local/state/"
    "ps-to-vnc/hardware-bridge"
)


def die(message: str) -> "NoReturn":
    raise SystemExit(
        f"BRIDGE_ERROR={message}"
    )


def run(
    args: list[str],
    *,
    cwd: Path | None = None,
    env: dict[str, str] | None = None,
    capture: bool = False,
    check: bool = True,
) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        args,
        cwd=cwd,
        env=env,
        text=True,
        stdout=(
            subprocess.PIPE
            if capture
            else None
        ),
        stderr=(
            subprocess.PIPE
            if capture
            else None
        ),
        check=False,
    )

    if check and result.returncode != 0:
        if capture:
            if result.stdout:
                print(
                    result.stdout,
                    file=sys.stderr,
                    end="",
                )

            if result.stderr:
                print(
                    result.stderr,
                    file=sys.stderr,
                    end="",
                )

        die(
            "command failed rc="
            f"{result.returncode}: "
            + " ".join(
                shlex.quote(part)
                for part in args
            )
        )

    return result


def git_text(
    root: Path,
    *args: str,
) -> str:
    result = run(
        [
            "git",
            "-C",
            str(root),
            *args,
        ],
        capture=True,
    )

    return result.stdout.strip()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()

    with path.open("rb") as handle:
        while True:
            block = handle.read(
                1024 * 1024
            )

            if not block:
                break

            digest.update(block)

    return digest.hexdigest()


def require_repo(path: Path, label: str) -> None:
    if not path.is_dir():
        die(
            f"{label} root missing: {path}"
        )

    inside = git_text(
        path,
        "rev-parse",
        "--is-inside-work-tree",
    )

    if inside != "true":
        die(
            f"{label} is not a git worktree: {path}"
        )


def require_successor_clean(
    root: Path,
) -> str:
    require_repo(
        root,
        "successor",
    )

    status = git_text(
        root,
        "status",
        "--porcelain=v1",
    )

    if status:
        die(
            "successor worktree must be fully clean"
        )

    return git_text(
        root,
        "rev-parse",
        "HEAD",
    )


def require_legacy_immutable(
    root: Path,
    expected_head: str,
) -> tuple[str, int]:
    require_repo(
        root,
        "legacy",
    )

    head = git_text(
        root,
        "rev-parse",
        "HEAD",
    )

    if head != expected_head:
        die(
            "legacy HEAD mismatch "
            f"expected={expected_head} "
            f"actual={head}"
        )

    worktree = run(
        [
            "git",
            "-C",
            str(root),
            "diff",
            "--quiet",
            "--",
        ],
        check=False,
    )

    if worktree.returncode != 0:
        die(
            "legacy tracked working tree is modified"
        )

    index = run(
        [
            "git",
            "-C",
            str(root),
            "diff",
            "--cached",
            "--quiet",
            "--",
        ],
        check=False,
    )

    if index.returncode != 0:
        die(
            "legacy index is modified"
        )

    tracked_status = git_text(
        root,
        "status",
        "--porcelain=v1",
        "--untracked-files=no",
    )

    if tracked_status:
        die(
            "legacy tracked/index porcelain is not clean"
        )

    full_status = git_text(
        root,
        "status",
        "--porcelain=v1",
    )

    untracked = sum(
        1
        for line in full_status.splitlines()
        if line.startswith("??")
    )

    return head, untracked


def require_relative_repo_path(
    raw: str,
    label: str,
) -> str:
    path = PurePosixPath(raw)

    if path.is_absolute():
        die(
            f"{label} must be repository-relative"
        )

    if ".." in path.parts:
        die(
            f"{label} may not contain .."
        )

    if raw in ("", "."):
        die(
            f"{label} must name a file"
        )

    return path.as_posix()


def parse_manifest(
    path: Path,
) -> dict[str, str]:
    values: dict[str, str] = {}

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
                f"manifest malformed "
                f"{path}:{number}"
            )

        key, value = raw.split(
            "=",
            1,
        )

        key = key.strip()

        if not key:
            die(
                f"manifest empty key "
                f"{path}:{number}"
            )

        try:
            tokens = shlex.split(
                value,
                posix=True,
            )
        except ValueError as exc:
            die(
                f"manifest parse error "
                f"{path}:{number}: {exc}"
            )

        if len(tokens) != 1:
            die(
                "manifest values must resolve "
                "to one shell token: "
                f"{path}:{number}"
            )

        if key in values:
            die(
                f"duplicate manifest key: {key}"
            )

        values[key] = tokens[0]

    return values


def tree_records(
    root: Path,
) -> list[tuple[str, str, int]]:
    if not root.is_dir():
        die(
            f"tool tree missing: {root}"
        )

    records: list[
        tuple[str, str, int]
    ] = []

    for path in sorted(
        root.rglob("*")
    ):
        if path.is_symlink():
            die(
                "TestKit tree contains unsupported "
                f"symlink: {path}"
            )

        if not path.is_file():
            continue

        rel = path.relative_to(
            root
        ).as_posix()

        mode = stat.S_IMODE(
            path.stat().st_mode
        )

        records.append(
            (
                rel,
                sha256_file(path),
                mode,
            )
        )

    return records


def tree_digest(
    records: list[
        tuple[str, str, int]
    ],
) -> str:
    digest = hashlib.sha256()

    for rel, sha, mode in records:
        digest.update(
            (
                f"{sha} {mode:04o} {rel}\n"
            ).encode("utf-8")
        )

    return digest.hexdigest()


def copy_testkit_exact(
    legacy_root: Path,
    workspace_repo: Path,
) -> str:
    source = (
        legacy_root
        / "scripts/testkit"
    )

    target = (
        workspace_repo
        / "scripts/testkit"
    )

    before = tree_records(
        source
    )

    if target.exists():
        shutil.rmtree(
            target
        )

    target.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    shutil.copytree(
        source,
        target,
        copy_function=shutil.copy2,
    )

    after = tree_records(
        target
    )

    if before != after:
        die(
            "copied legacy TestKit tree "
            "is not byte/mode exact"
        )

    return tree_digest(
        before
    )


def provenance_path(
    workspace: Path,
) -> Path:
    return (
        workspace
        / "BRIDGE-PROVENANCE.env"
    )


def write_provenance(
    *,
    workspace: Path,
    successor_root: Path,
    successor_head: str,
    legacy_root: Path,
    legacy_head: str,
    legacy_untracked: int,
    manifest_rel: str,
    manifest_sha: str,
    test_id: str,
    run_prefix: str,
    testkit_digest: str,
) -> None:
    content = "\n".join(
        [
            (
                "PS_TO_VNC_LEGACY_HARDWARE_"
                "BRIDGE_VERSION="
                f"{BRIDGE_VERSION}"
            ),
            f"TEST_ID={test_id}",
            f"RUN_PREFIX={run_prefix}",
            (
                "SUCCESSOR_SOURCE_ROOT="
                f"{successor_root}"
            ),
            (
                "SUCCESSOR_HEAD="
                f"{successor_head}"
            ),
            (
                "LEGACY_SOURCE_ROOT="
                f"{legacy_root}"
            ),
            (
                "LEGACY_HEAD="
                f"{legacy_head}"
            ),
            (
                "LEGACY_UNTRACKED_COUNT_AT_PREPARE="
                f"{legacy_untracked}"
            ),
            (
                "MANIFEST_REL="
                f"{manifest_rel}"
            ),
            (
                "MANIFEST_SHA256="
                f"{manifest_sha}"
            ),
            (
                "TESTKIT_TREE_SHA256="
                f"{testkit_digest}"
            ),
            (
                "WORKSPACE_REPO="
                f"{workspace / 'repo'}"
            ),
        ]
    )

    provenance_path(
        workspace
    ).write_text(
        content + "\n",
        encoding="utf-8",
    )


def load_provenance(
    workspace: Path,
) -> dict[str, str]:
    path = provenance_path(
        workspace
    )

    if not path.is_file():
        die(
            f"bridge provenance missing: {path}"
        )

    return parse_manifest(
        path
    )


def validate_workspace(
    workspace: Path,
) -> tuple[
    Path,
    Path,
    dict[str, str],
]:
    workspace = workspace.resolve()

    data = load_provenance(
        workspace
    )

    repo = (
        workspace
        / "repo"
    )

    if not repo.is_dir():
        die(
            f"workspace repo missing: {repo}"
        )

    current_head = git_text(
        repo,
        "rev-parse",
        "HEAD",
    )

    if current_head != data.get(
        "SUCCESSOR_HEAD"
    ):
        die(
            "workspace successor HEAD changed"
        )

    manifest_rel = require_relative_repo_path(
        data.get(
            "MANIFEST_REL",
            "",
        ),
        "MANIFEST_REL",
    )

    manifest = (
        repo
        / manifest_rel
    )

    if not manifest.is_file():
        die(
            f"workspace manifest missing: {manifest}"
        )

    manifest_sha = sha256_file(
        manifest
    )

    if manifest_sha != data.get(
        "MANIFEST_SHA256"
    ):
        die(
            "workspace manifest hash changed"
        )

    legacy_root = Path(
        data.get(
            "LEGACY_SOURCE_ROOT",
            "",
        )
    )

    legacy_head = data.get(
        "LEGACY_HEAD",
        "",
    )

    require_legacy_immutable(
        legacy_root,
        legacy_head,
    )

    live_legacy = tree_records(
        legacy_root
        / "scripts/testkit"
    )

    copied = tree_records(
        repo
        / "scripts/testkit"
    )

    if live_legacy != copied:
        die(
            "workspace TestKit no longer matches "
            "frozen legacy TestKit"
        )

    digest = tree_digest(
        copied
    )

    if digest != data.get(
        "TESTKIT_TREE_SHA256"
    ):
        die(
            "workspace TestKit provenance digest "
            "mismatch"
        )

    return repo, manifest, data


def inherited(
    repo: Path,
    tool: str,
    args: list[str],
    *,
    env: dict[str, str] | None = None,
) -> int:
    allowed = {
        "verify-build.sh",
        "deploy-elf.sh",
        "start-hardware.sh",
        "status.sh",
        "watch.sh",
        "result.sh",
    }

    if tool not in allowed:
        die(
            f"unsupported inherited tool: {tool}"
        )

    command = [
        str(
            repo
            / "scripts/testkit"
            / tool
        ),
        *args,
    ]

    result = run(
        command,
        cwd=repo,
        env=env,
        check=False,
    )

    return result.returncode


def command_prepare(
    args: argparse.Namespace,
) -> int:
    successor = Path(
        args.successor_root
    ).resolve()

    legacy = Path(
        args.legacy_root
    ).resolve()

    successor_head = (
        require_successor_clean(
            successor
        )
    )

    legacy_head, legacy_untracked = (
        require_legacy_immutable(
            legacy,
            args.legacy_head,
        )
    )

    manifest_rel = require_relative_repo_path(
        args.manifest,
        "manifest",
    )

    manifest_source = (
        successor
        / manifest_rel
    )

    if not manifest_source.is_file():
        die(
            f"manifest missing: {manifest_source}"
        )

    tracked = run(
        [
            "git",
            "-C",
            str(successor),
            "ls-files",
            "--error-unmatch",
            manifest_rel,
        ],
        capture=True,
        check=False,
    )

    if tracked.returncode != 0:
        die(
            "hardware manifest must be tracked "
            "before bridge preparation"
        )

    values = parse_manifest(
        manifest_source
    )

    for required in [
        "TEST_ID",
        "RUN_PREFIX",
    ]:
        if not values.get(
            required
        ):
            die(
                f"manifest missing {required}"
            )

    workspace_root = Path(
        args.workspace_root
    ).resolve()

    workspace_root.mkdir(
        parents=True,
        exist_ok=True,
    )

    stamp = datetime.now().strftime(
        "%Y%m%d-%H%M%S"
    )

    safe_test = "".join(
        character
        if (
            character.isalnum()
            or character in "-_."
        )
        else "_"
        for character in values[
            "TEST_ID"
        ]
    )

    workspace = (
        workspace_root
        / (
            f"{safe_test}-{stamp}-"
            f"{os.getpid()}"
        )
    )

    if workspace.exists():
        die(
            f"workspace already exists: {workspace}"
        )

    repo = (
        workspace
        / "repo"
    )

    workspace.mkdir(
        parents=True,
    )

    run(
        [
            "git",
            "clone",
            "--quiet",
            "--no-hardlinks",
            str(successor),
            str(repo),
        ]
    )

    cloned_head = git_text(
        repo,
        "rev-parse",
        "HEAD",
    )

    if cloned_head != successor_head:
        die(
            "workspace clone HEAD differs "
            "from successor source HEAD"
        )

    testkit_digest = copy_testkit_exact(
        legacy,
        repo,
    )

    manifest = (
        repo
        / manifest_rel
    )

    if not manifest.is_file():
        die(
            "tracked hardware manifest missing "
            "from workspace clone"
        )

    manifest_sha = sha256_file(
        manifest
    )

    write_provenance(
        workspace=workspace,
        successor_root=successor,
        successor_head=successor_head,
        legacy_root=legacy,
        legacy_head=legacy_head,
        legacy_untracked=legacy_untracked,
        manifest_rel=manifest_rel,
        manifest_sha=manifest_sha,
        test_id=values["TEST_ID"],
        run_prefix=values["RUN_PREFIX"],
        testkit_digest=testkit_digest,
    )

    rc = inherited(
        repo,
        "verify-build.sh",
        [
            manifest_rel,
        ],
    )

    if rc != 0:
        die(
            "inherited verify-build failed "
            f"for prepared workspace: {workspace}"
        )

    print(
        "PS_TO_VNC_LEGACY_HARDWARE_BRIDGE_VERSION=1"
    )
    print(
        "BRIDGE_ACTION=PREPARE"
    )
    print(
        f"BRIDGE_WORKSPACE={workspace}"
    )
    print(
        f"BRIDGE_REPO={repo}"
    )
    print(
        f"BRIDGE_MANIFEST={manifest_rel}"
    )
    print(
        f"TEST_ID={values['TEST_ID']}"
    )
    print(
        f"RUN_PREFIX={values['RUN_PREFIX']}"
    )
    print(
        f"SUCCESSOR_HEAD={successor_head}"
    )
    print(
        f"LEGACY_HEAD={legacy_head}"
    )
    print(
        "LEGACY_TRACKED_MUTATION=NO"
    )
    print(
        "LEGACY_INDEX_MUTATION=NO"
    )
    print(
        "LEGACY_UNTRACKED_ARTIFACTS="
        f"{legacy_untracked}"
    )
    print(
        "LEGACY_UNTRACKED_ARTIFACTS_PRESERVED=YES"
    )
    print(
        f"TESTKIT_TREE_SHA256={testkit_digest}"
    )
    print(
        "INHERITED_VERIFY_BUILD=PASS"
    )
    print(
        "PS_TO_VNC_LEGACY_HARDWARE_BRIDGE=PASS"
    )

    return 0


def command_verify(
    args: argparse.Namespace,
) -> int:
    workspace = Path(
        args.workspace
    )

    repo, manifest, data = (
        validate_workspace(
            workspace
        )
    )

    rc = inherited(
        repo,
        "verify-build.sh",
        [
            data["MANIFEST_REL"],
        ],
    )

    if rc != 0:
        return rc

    print(
        "BRIDGE_WORKSPACE_VALIDATION=PASS"
    )
    print(
        "PS_TO_VNC_LEGACY_HARDWARE_BRIDGE=PASS"
    )

    return 0


def command_deploy_dry_run(
    args: argparse.Namespace,
) -> int:
    workspace = Path(
        args.workspace
    )

    repo, manifest, data = (
        validate_workspace(
            workspace
        )
    )

    rc = inherited(
        repo,
        "verify-build.sh",
        [
            data["MANIFEST_REL"],
        ],
    )

    if rc != 0:
        return rc

    env = os.environ.copy()
    env["TESTKIT_DRY_RUN"] = "1"

    rc = inherited(
        repo,
        "deploy-elf.sh",
        [
            data["MANIFEST_REL"],
        ],
        env=env,
    )

    if rc != 0:
        return rc

    print(
        "BRIDGE_DEPLOY_DRY_RUN=PASS"
    )
    print(
        "FTP_CONTACT=NO"
    )
    print(
        "PS_TO_VNC_LEGACY_HARDWARE_BRIDGE=PASS"
    )

    return 0


def command_start(
    args: argparse.Namespace,
) -> int:
    workspace = Path(
        args.workspace
    )

    repo, manifest, data = (
        validate_workspace(
            workspace
        )
    )

    rc = inherited(
        repo,
        "verify-build.sh",
        [
            data["MANIFEST_REL"],
        ],
    )

    if rc != 0:
        return rc

    return inherited(
        repo,
        "start-hardware.sh",
        [
            data["MANIFEST_REL"],
        ],
    )


def command_job_tool(
    args: argparse.Namespace,
    tool: str,
) -> int:
    workspace = Path(
        args.workspace
    )

    repo, manifest, data = (
        validate_workspace(
            workspace
        )
    )

    return inherited(
        repo,
        tool,
        [
            args.run,
        ],
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Run the frozen legacy PS2VNC Hardware "
            "TestKit from an isolated successor-rooted "
            "workspace without mutating the frozen "
            "legacy repository."
        )
    )

    sub = parser.add_subparsers(
        dest="command",
        required=True,
    )

    prepare = sub.add_parser(
        "prepare",
    )

    prepare.add_argument(
        "manifest",
        help=(
            "tracked successor-repository-relative "
            "hardware manifest"
        ),
    )

    prepare.add_argument(
        "--successor-root",
        default=str(
            Path(__file__).resolve().parents[2]
        ),
    )

    prepare.add_argument(
        "--legacy-root",
        default=str(
            DEFAULT_LEGACY_ROOT
        ),
    )

    prepare.add_argument(
        "--legacy-head",
        default=DEFAULT_LEGACY_HEAD,
    )

    prepare.add_argument(
        "--workspace-root",
        default=str(
            DEFAULT_WORKSPACE_ROOT
        ),
    )

    prepare.set_defaults(
        function=command_prepare,
    )

    verify = sub.add_parser(
        "verify",
    )

    verify.add_argument(
        "workspace",
    )

    verify.set_defaults(
        function=command_verify,
    )

    dry = sub.add_parser(
        "deploy-dry-run",
    )

    dry.add_argument(
        "workspace",
    )

    dry.set_defaults(
        function=command_deploy_dry_run,
    )

    start = sub.add_parser(
        "start",
    )

    start.add_argument(
        "workspace",
    )

    start.set_defaults(
        function=command_start,
    )

    for name, tool in [
        (
            "status",
            "status.sh",
        ),
        (
            "watch",
            "watch.sh",
        ),
        (
            "result",
            "result.sh",
        ),
    ]:
        command = sub.add_parser(
            name,
        )

        command.add_argument(
            "workspace",
        )

        command.add_argument(
            "run",
        )

        command.set_defaults(
            function=(
                lambda parsed, t=tool:
                command_job_tool(
                    parsed,
                    t,
                )
            ),
        )

    return parser


def main() -> int:
    parser = build_parser()

    args = parser.parse_args()

    return int(
        args.function(
            args
        )
    )


if __name__ == "__main__":
    sys.exit(
        main()
    )
