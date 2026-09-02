# Branch Lifecycle and Cold-Storage Policy

## Purpose

PS-to-VNC uses branches for active work, controlled candidates, preserved
experiments, reconstruction, audit work, and historical checkpoints. Branches
must remain understandable without requiring a developer to remember what each
name meant in a previous conversation.

The goal is to keep the live GitHub branch set small enough to understand while
never trading away provenance for tidiness.

## Non-negotiable deletion rule

> **No branch may be deleted from GitHub or a protected local repository without
> explicit user approval for that specific branch.**

A branch audit may classify a branch as `SAFE_TO_RETIRE`, but that classification
is a recommendation only. It is **not permission to delete the branch**.

Before asking for approval, present enough information to make the decision
meaningful:

- branch name;
- exact branch-head commit;
- what work the branch contains;
- whether its useful changes are merged, superseded, rejected, or archival;
- related issue/PR/case/evidence pointers;
- whether any unique commits or evidence remain reachable only through that
  branch;
- whether a cold-storage archive has been created and independently verified.

Deletion must never be bundled implicitly into cleanup, merge, PR closeout,
branch audit, or repository normalization.

## Branch status vocabulary

Use these classifications in the branch/workstream index:

- `MAIN_AUTHORITY` — repository default/current integrated authority.
- `ACTIVE` — current implementation, documentation, investigation, or test work.
- `CANDIDATE` — deliberate evaluating branch awaiting qualification or design
  disposition.
- `PROTECTED_ARCHIVE` — intentionally retained historical or experimental
  authority; not ordinary cleanup material.
- `SUPERSEDED` — no longer current, but still retained until its provenance and
  archival disposition are reviewed.
- `SAFE_TO_RETIRE` — audit found no remaining live GitHub purpose **and** the
  retirement prerequisites below have been satisfied or are ready to satisfy.

`SAFE_TO_RETIRE` does not mean "delete now."

## Cold storage is the preferred retirement path

In most cases, a branch approved for retirement should first be preserved in
cold storage so removing the live GitHub ref does not remove our practical
ability to reconstruct it later.

The preferred archive is a Git bundle or equivalent self-contained Git object
archive that preserves the exact branch ref and reachable history. A plain ZIP
of a checked-out working tree is not an adequate substitute because it loses
Git ancestry and object identity.

For each retirement archive, preserve a small manifest containing at least:

- repository name;
- archived branch name;
- exact branch-head commit SHA;
- archive creation date;
- related PR/issue/investigation identifiers where applicable;
- archive filename;
- archive byte size;
- SHA-256 of the archive;
- verification result;
- intended cold-storage location or media identifier when known.

If a branch owns important non-Git evidence outside the repository, record that
separately. A Git bundle cannot preserve untracked files, external hardware logs,
or artifacts that were never committed.

## Required retirement sequence

Retirement should normally follow this order:

1. **Read-only audit**
   - identify branch purpose and exact head;
   - compare against `main` and related active branches;
   - identify unique commits/content;
   - follow related issues, PRs, cases, evidence, and documentation;
   - inspect associated worktrees and dirty state.

2. **Classify**
   - keep as `ACTIVE`, `CANDIDATE`, or `PROTECTED_ARCHIVE`; or
   - mark `SUPERSEDED`; or
   - recommend `SAFE_TO_RETIRE`.

3. **Resolve dirty/external state**
   - never archive or delete over unexplained staged, unstaged, or untracked
     material;
   - preserve or deliberately disposition external evidence first.

4. **Create cold-storage archive**
   - produce a self-contained Git archive/bundle preserving the exact branch
     head and history needed to reconstruct it;
   - generate the archive manifest and SHA-256.

5. **Verify independently**
   - run the Git archive verification mechanism (for example `git bundle
     verify`);
   - restore/fetch the archived ref into a disposable repository or equivalent
     isolated environment;
   - prove the restored branch head equals the recorded original SHA;
   - record PASS/FAIL in the manifest.

6. **Discuss with the user**
   - show the audit conclusion and archive verification;
   - explain what, if anything, would cease to be conveniently accessible on
     GitHub;
   - request explicit approval for that named branch.

7. **Delete only after approval**
   - delete only the branch explicitly approved;
   - do not generalize one approval to other branches;
   - record the deletion date and archive pointer in the branch/workstream
     index or retirement record.

## Worktree rule

A branch checked out by a local worktree is not merely a remote ref. Before
retirement, identify every associated worktree and its status. Do not remove a
worktree, reset it, clean it, or detach it merely to make branch deletion easier.
Dirty state is potential project evidence until understood.

## Merged branches

A branch being fully merged does not automatically make it disposable. Its
name, PR discussion, exact historical head, or branch-local evidence may still
be useful. Merged status is one input to the retirement audit, not a deletion
trigger.

## Archive branches

Branches explicitly named or classified as archives are presumed
`PROTECTED_ARCHIVE`. They may still be moved to colder storage someday, but only
through the same audit, archive verification, discussion, and explicit approval
process.

## Relationship to fresh-session reconstruction

Fresh sessions should not attempt to memorize branch names. They should inspect
the current branch/workstream index and GitHub branch inventory, then classify
what is active, candidate, archival, superseded, or retirement-ready.

When branch count becomes confusing, the corrective action is to improve the
index and perform a read-only lifecycle audit—not to delete refs opportunistically.
