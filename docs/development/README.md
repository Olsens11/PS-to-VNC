# Development Continuity

PS-to-VNC treats development continuity as an engineering concern rather than
a conversation-management convenience.

A fresh development session should be able to determine from the repository:

- what the project currently is;
- what state it is in;
- what has been proven;
- what remains uncertain;
- what rules govern development;
- which tools are canonical;
- how to continue the active experiment safely.

## Canonical fresh-session procedure

The detailed reconstruction procedure is:

    docs/development/SESSION_RECONSTRUCTION.md

`AGENTS.md` is the canonical entry point and requires that procedure before
substantive project work begins.

Fresh-session reconstruction is intentionally not tied to today's issue or PR
numbers. A capable session must discover the active work dynamically by
reconciling current repository authority with the complete GitHub work record,
including open and closed issues, issue comments, open/merged/closed PRs, PR
reasoning, relevant branch heads, and recent Git history.

If a new chat requires a long conversation-specific handoff to know what the
project is doing, treat that as evidence that repository continuity needs to be
improved.

## Historical continuity harvest

PS2VNC's mature documentation/status/TestKit system is a deliberate source of
lessons, not a structural template for the clean successor.

The reviewed harvest and `ADOPT` / `ADAPT` / `DEFER` / `REJECT` dispositions are
recorded in:

    docs/development/PS2VNC_CONTINUITY_HARVEST.md

This keeps the useful historical continuity reasoning inside PS-to-VNC so a
fresh session does not need the legacy repository merely to understand why the
successor bootstrap, temporal-state, provenance, apparatus, and tool-routing
rules exist.

## Branch lifecycle, workstream index, and cold storage

Branch count is a continuity concern too. Developers should not be expected to
remember which branch contains which work or infer safety from a branch name.

Current branch/workstream routing:

    docs/development/BRANCH_WORKSTREAM_INDEX.md

Branch audit / retirement policy:

    docs/development/BRANCH_LIFECYCLE.md

The workstream index identifies what is current, candidate, archival, or
superseded. Fresh sessions must still verify it against the actual GitHub branch
inventory and related PRs; the index exists to make that verification fast and
comprehensible rather than to become another stale authority.

Important lifecycle rules include:

- branch audits may recommend `SAFE_TO_RETIRE`, but never delete automatically;
- **every branch deletion requires explicit user approval for that specific
  branch**;
- the preferred retirement path is a verified self-contained Git archive/bundle
  plus a manifest and SHA-256 in cold storage before the live branch ref is
  removed;
- merged status alone is not permission to retire a branch;
- protected archive branches and dirty worktrees must be treated as evidence,
  not cleanup debris.

When the branch list becomes difficult to understand, improve the branch/workstream
index and perform a read-only audit before considering any deletion.

## Reconstruction philosophy must affect decisions

Continuity is incomplete if a session can recite `docs/PROJECT_INTENT.md` but
then makes implementation choices without applying it.

After reconstructing state and before promoting a meaningful solution, use the
decision discipline defined in `AGENTS.md` and
`docs/development/SESSION_RECONSTRUCTION.md`:

- identify the durable requirement or historical lesson;
- separate it from accidental historical implementation structure;
- start with the conventional supported mechanism for the target platform;
- justify any PS-to-VNC-specific adaptation by a concrete product benefit;
- keep ownership/provider seams replaceable where evidence says future change is
  plausible;
- defer speculative future infrastructure rather than pre-building it;
- state the empirical qualification gate that can promote or reject the choice.

A development handoff should therefore communicate not only **what we are doing**
but enough authority and procedure for the next session to understand **why this
is the clean-reconstruction choice**.

## Two-layer model

### Portable continuity layer

Potentially reusable concepts include:

- repository-centered session reconstruction;
- dirty-state protection;
- authority separation;
- experiment/apparatus provenance;
- exact DUT identity;
- machine versus operator evidence;
- documentation transactions;
- reusable-tool promotion;
- deferred-work capture;
- current-state/history separation.

### PS-to-VNC project adapter

Project-specific mechanics include:

- PS2 build authority;
- ELF/PT_LOAD identity;
- PS2/Pi services and ports;
- FTP deployment;
- display modes;
- hardware qualification procedures;
- inherited PS2VNC TestKit.

The portable layer must not hard-code these project-specific mechanics.

## Five kinds of truth

Keep these responsibilities distinct:

1. current truth — what is true now;
2. architectural truth — how the system is intended to work;
3. decision truth — why an important choice was made;
4. empirical truth — what controlled evidence demonstrated;
5. historical truth — what happened previously.

One document should not become all five.

## Real-project agitation

The continuity system is intentionally developed against real project pressure.

PS2VNC served as the initial agitator. PS-to-VNC continues that role.

Development friction is useful evidence when it exposes a missing invariant.

Generalization is not automatic. First decide whether the pressure is
project-specific or reusable.

## Promotion ladder

A development-infrastructure idea normally matures through:

1. observed project friction;
2. local rule or repair;
3. repeated need;
4. reusable mechanism with explicit inputs;
5. self-test/versioning;
6. possible extraction into a portable framework.

This prevents speculative framework design while still allowing real project
work to improve the development system.

## Failure classification

Distinguish among:

- source/product failure;
- DUT/build failure;
- experiment failure;
- apparatus failure;
- development-infrastructure failure.

An infrastructure failure may itself be useful evidence if it reveals a
recurring weakness.
