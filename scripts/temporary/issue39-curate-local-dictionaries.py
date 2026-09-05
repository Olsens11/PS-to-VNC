#!/usr/bin/env python3
"""
Topology-aware Issue #39 dictionary curation runner.

This helper lives only on an automation branch. It reuses the already reviewed
source-grounded Issue #39 semantic-description map, applies it to the new
per-domain dictionary topology introduced at 0b3e20c7c4840073618a1bb487d554bc3309d057,
runs repository-native validation, then commits and pushes only dictionary
prose to reconstruct/issue39-keyboard-osk-local-ui.
"""

from __future__ import annotations

from pathlib import Path
import importlib.util
import subprocess
import sys


EXPECTED_BRANCH = "reconstruct/issue39-keyboard-osk-local-ui"
EXPECTED_HEAD = "0b3e20c7c4840073618a1bb487d554bc3309d057"
BASE_SCRIPT = Path(__file__).with_name("issue39-curation-base.py")
PORTAL = Path("docs/reference/SOURCE_SYMBOL_DICTIONARIES.md")

DICTIONARIES = (
    Path("src/SYMBOLS.md"),
    Path("src/diagnostics/SYMBOLS.md"),
    Path("src/display/SYMBOLS.md"),
    Path("src/framebuffer/SYMBOLS.md"),
    Path("src/input/SYMBOLS.md"),
    Path("src/platform/SYMBOLS.md"),
    Path("src/rfb/SYMBOLS.md"),
    Path("src/ui/SYMBOLS.md"),
)

EXPECTED_CHANGED_DICTIONARIES = {
    "src/SYMBOLS.md",
    "src/input/SYMBOLS.md",
    "src/platform/SYMBOLS.md",
    "src/rfb/SYMBOLS.md",
    "src/ui/SYMBOLS.md",
}


def run(*args: str, capture: bool = False) -> str:
    completed = subprocess.run(
        args,
        check=True,
        text=True,
        stdout=subprocess.PIPE if capture else None,
    )
    return completed.stdout if capture else ""


def git(*args: str, capture: bool = False) -> str:
    return run("git", *args, capture=capture)


def load_base():
    spec = importlib.util.spec_from_file_location(
        "issue39_curation_base",
        BASE_SCRIPT,
    )
    if spec is None or spec.loader is None:
        raise RuntimeError("unable to load curation base module")

    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def find_rows(base, names: set[str]) -> None:
    for dictionary in DICTIONARIES:
        for line in dictionary.read_text(encoding="utf-8").splitlines():
            if not line.startswith("| ") or line.startswith("| Name |"):
                continue
            cells = base.parse_row(line)
            if cells[0] in names:
                print(f"{dictionary}:{line}")


def main() -> int:
    print("===== ISSUE39 TOPOLOGY-AWARE GITHUB DICTIONARY CURATION =====")

    print()
    print("===== 1. EXACT TARGET AUTHORITY =====")

    branch = git("branch", "--show-current", capture=True).strip()
    head = git("rev-parse", "HEAD", capture=True).strip()
    remote = git(
        "ls-remote",
        "origin",
        f"refs/heads/{EXPECTED_BRANCH}",
        capture=True,
    ).strip().split()
    remote_head = remote[0] if remote else ""

    print(f"BRANCH={branch}")
    print(f"HEAD={head}")
    print(f"REMOTE_HEAD={remote_head}")

    if branch != EXPECTED_BRANCH:
        raise RuntimeError(f"wrong target branch: {branch}")
    if head != EXPECTED_HEAD:
        raise RuntimeError(f"unexpected target HEAD: {head}")
    if remote_head != EXPECTED_HEAD:
        raise RuntimeError(f"remote target drifted: {remote_head}")
    if git("status", "--porcelain=v1", "--untracked-files=all", capture=True):
        raise RuntimeError("target worktree must be completely clean")

    print("CURATION_AUTHORITY=PASS")

    print()
    print("===== 2. LOAD REVIEWED SOURCE-GROUNDED SEMANTIC MAP =====")

    base = load_base()
    base.EXPECTED_BRANCH = EXPECTED_BRANCH
    base.EXPECTED_HEAD = EXPECTED_HEAD
    base.EXPECTED_BLOBS = {}
    base.DICTIONARIES = DICTIONARIES
    base.PORTAL = PORTAL

    print("CURATION_BASE=LOADED")
    print("CURATION_BASE_EXPECTED_ROWS=263")

    print()
    print("===== 3. APPLY SEMANTIC CURATION TO LOCAL DICTIONARIES =====")

    changed, kind_counts = base.curate()

    print(f"CURATED_DESCRIPTION_ROWS={changed}")
    for kind in sorted(kind_counts):
        print(f"CURATED_KIND_COUNT={kind}:{kind_counts[kind]}")

    if changed != 263:
        raise RuntimeError(f"unexpected curated row count: {changed}")

    print()
    print("===== 4. REGENERATE PORTAL / RUN FULL DICTIONARY GATE =====")

    run(
        "python3",
        "scripts/source-dictionary.py",
        "portal",
        "--output",
        str(PORTAL),
    )

    run("python3", "scripts/source-dictionary-self-test.py")
    run(
        "python3",
        "scripts/source-dictionary.py",
        "check",
        "--long",
        "--require-complete",
        "--strict",
    )

    print("SOURCE_DICTIONARY_SYSTEM=PASS")

    print()
    print("===== 5. CANONICAL PROJECT CHECK =====")

    run("./scripts/check.sh")
    print("CANONICAL_PROJECT_CHECK=PASS")

    print()
    print("===== 6. CURATION DIFF SCOPE =====")

    git("diff", "--check")

    changed_paths = set(
        git("diff", "--name-only", capture=True).splitlines()
    )

    allowed_paths = {
        str(path)
        for path in DICTIONARIES
    } | {str(PORTAL)}

    unexpected = changed_paths - allowed_paths
    if unexpected:
        raise RuntimeError(
            "curation changed unexpected paths: "
            + ", ".join(sorted(unexpected))
        )

    changed_dictionaries = {
        path
        for path in changed_paths
        if path.endswith("SYMBOLS.md") and path.startswith("src/")
    }

    if changed_dictionaries != EXPECTED_CHANGED_DICTIONARIES:
        raise RuntimeError(
            "unexpected changed dictionary set: "
            + repr(sorted(changed_dictionaries))
        )

    print("CURATION_PATH_SCOPE=PASS")
    print("CHANGED_DICTIONARIES=" + ",".join(sorted(changed_dictionaries)))
    git("diff", "--stat")

    print()
    print("===== 7. REPRESENTATIVE SOURCE-GROUNDED CONTRACTS =====")

    find_rows(
        base,
        {
            "publish_semantic_keyboard_tap",
            "pstvnc_keyboard_build_tap_sequence",
            "pstvnc_input_runtime_rebase_suspended_mouse_state",
            "pstvnc_mouse_reset_transient_history",
            "pstvnc_local_controller_route",
            "pstvnc_local_ui_open_osk",
            "pstvnc_local_ui_mark_presented",
            "pstvnc_osk_move_vertical",
            "pstvnc_osk_activate_selected",
            "pstvnc_osk_render_surface",
            "pstvnc_rfb_build_key_event",
            "pstvnc_rfb_session_send_key_event",
            "pstvnc_ps2_graphics_present",
        },
    )

    print()
    print("===== 8. STAGE DICTIONARY-ONLY CURATION =====")

    git(
        "add",
        "--",
        *(str(path) for path in DICTIONARIES),
        str(PORTAL),
    )

    git("diff", "--cached", "--check")

    if git("diff", "--name-only", capture=True).strip():
        raise RuntimeError("unstaged tracked changes remain after dictionary staging")

    staged_paths = set(
        git("diff", "--cached", "--name-only", capture=True).splitlines()
    )

    if staged_paths != changed_paths:
        raise RuntimeError(
            "staged curation paths do not equal validated changed paths"
        )

    print("STAGED_CURATED_PATHS=" + ",".join(sorted(staged_paths)))

    print()
    print("===== 9. COMMIT / PUSH CURATION =====")

    git("commit", "-m", "docs: curate Issue 39 source dictionaries")
    commit = git("rev-parse", "HEAD", capture=True).strip()
    print(f"CURATION_COMMIT={commit}")

    git("push", "origin", f"HEAD:{EXPECTED_BRANCH}")

    remote_after = git(
        "ls-remote",
        "origin",
        f"refs/heads/{EXPECTED_BRANCH}",
        capture=True,
    ).strip().split()
    remote_after_head = remote_after[0] if remote_after else ""

    print(f"REMOTE_HEAD_AFTER={remote_after_head}")
    if remote_after_head != commit:
        raise RuntimeError("remote target did not advance to curation commit")

    if git("status", "--porcelain=v1", "--untracked-files=all", capture=True):
        raise RuntimeError("curation clone is dirty after commit")

    print()
    print("===== FINAL =====")
    print(f"CURATION_COMMIT={commit}")
    print("CURATED_DESCRIPTION_ROWS=263")
    print("CURATION_PATH_SCOPE=PASS")
    print("SOURCE_DICTIONARY_SYSTEM=PASS")
    print("CANONICAL_PROJECT_CHECK=PASS")
    print("RUNTIME_SOURCE_MUTATION=NO")
    print("ISSUE39_CLOSED=NO")
    print("NEXT=FAST_FORWARD_REAL_WORKTREE_AND_FINAL_ISSUE39_CLOSEOUT_REVIEW")

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print()
        print("===== CURATION FAILED =====")
        print(f"ERROR={exc}")
        print("COMMIT_CREATED=NO_OR_REQUIRES_INSPECTION")
        print("PUSH_PERFORMED=NO_OR_REQUIRES_INSPECTION")
        raise
