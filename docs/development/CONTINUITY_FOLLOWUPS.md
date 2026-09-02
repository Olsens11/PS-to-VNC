# Development Continuity Follow-ups

Temporal role: `SNAPSHOT_BACKLOG`

Recorded at: `2026-09-02T15:35:00-04:00`

This file records continuity/infrastructure work explicitly requested by the
user that may be set aside while hands-on PS-to-VNC engineering takes priority.
It is not product-roadmap authority and it is not proof that every item remains
current forever. Reconcile it with later work before resuming under
`docs/development/TEMPORAL_STATE_SEMANTICS.md`.

The purpose is simple: do not make the user re-explain these requests later.

## Priority when hands-on work resumes

Do **not** block an available hardware session merely to finish documentation
cleanup. Resume active technical qualification from its exact current issue/PR/
evidence authority.

At this snapshot the two important hands-on workstreams are:

### Issue #5 / PR #17 — clean Pi and PS2-facing RFB lifecycle

Resume from the existing Issue #5 and PR #17 records rather than improvising a
new Pi design.

The next hands-on sequence remains conservative:

1. inspect live Pi state before mutation, including exploratory w0/kanshi/helper
   state that still needs reconciliation;
2. preserve the exact w0vncserver experiment as reference only;
3. return to the documented dedicated packaged Xtigervnc path;
4. capture cold/no-carrier -> PS2 carrier -> NetworkManager address -> port 5900
   listener timing before changing the lifecycle mechanism;
5. stage/qualify the systemd socket-activation + Xtigervnc `-inetd` candidate
   only through PR #17's comparison against the conventional persistent-provider
   control;
6. keep NetworkManager ownership, RFB provider choice, and lifecycle policy as
   separate decisions;
7. reconcile every live package/config/service mutation into Issue #5 before
   promotion.

Do not silently promote socket activation, a no-carrier override, w0/WayVNC, or
historical traffic shaping merely because an experiment works.

### Issue #7 / PR #15 — clean minimal Raw/480p PS2 core

The physical Raw/480p/RFB/GS path has already been demonstrated on the stamped
DUT, but formal HW1 promotion failed because only the late
`PSTVNC_STAGE DESKTOP_READY` UDP diagnostic was observed. Early runtime identity,
`NET_READY`, and `GS_READY` evidence was absent.

If Issue #7 is resumed, keep the next formal experiment narrow: isolate why the
early startup/runtime-identity `sendto` evidence disappears while the later
`DESKTOP_READY` send succeeds. Do not disturb the demonstrated RFB/Raw/GS path
merely to chase the evidence defect.

## Continuity work completed since the previous snapshot

### Branch deobfuscation — first audit complete

The full live GitHub branch inventory was reviewed and recorded in:

`docs/development/branch-audits/2026-09-02-live-branch-audit.md`

Completed:

- all 11 live branches were classified;
- the four merged audit/reboot/state branches were confirmed as fully merged
  cold-storage candidates rather than active product directions;
- the w0vncserver archive remains explicitly protected;
- all three unique commits on `reconstruction/m1-minimal-core` were reviewed;
- M1's durable lessons were harvested and its temporary handshake-only design was
  compared against the stricter current Issue #7 implementation;
- M1 is now `SUPERSEDED / UNIQUE_HISTORY_HARVESTED / COLD_STORAGE_CANDIDATE`.

Nothing was deleted. No cold-storage candidate is deletion-authorized.

### PS2VNC continuity-system harvest — first deliberate pass complete

Reviewed the historical PS2VNC continuity authorities covering:

- complete-session bootstrap;
- documentation maintenance/transactions;
- development infrastructure;
- identity-bound status/resume behavior;
- TestKit/apparatus continuity;
- provenance grading;
- historical harvesting;
- role-scoped known-good milestones;
- file/service/context routing.

The successor dispositions are now recorded in:

`docs/development/PS2VNC_CONTINUITY_HARVEST.md`

The harvest distinguishes `ADOPT`, `ADAPT`, `DEFER`, and `REJECT` so useful
continuity lessons do not turn into blind copying of old product structure.

### New-chat entrance and context routing improved

Completed:

- root `README.md` now gives the one-line fresh-chat handoff prominently;
- `START_HERE.md` is now explicitly a migration-era compatibility redirect to
  `AGENTS.md`, not a second competing bootstrap checklist;
- `docs/reference/CURRENT_TOOL_AND_CONTEXT_MAP.md` is the clean-current router;
- the old `docs/reference/FILE_AND_SERVICE_MAP.md` is explicitly labeled
  migration-era compatibility/reference material;
- `docs/README.md` and `docs/INDEX.md` route the new continuity authorities.

### Temporal-state semantics partly mechanized

Completed:

- mutable status surfaces have explicit snapshot semantics;
- `scripts/resume-state.sh` no longer prints the old committed `NEXT_ACTION` as
  if it were automatically current;
- resume output identifies snapshot timestamp and newer local Git activity and
  refuses to prove present-tense next-action authority without reconciliation;
- `scripts/continuity-check.sh` now enforces snapshot roles/timestamps, the one
  canonical bootstrap, current context routing, and branch-deletion policy;
- the old machine state is explicitly `CURRENT_AT_RECORDED_TIME`, not eternal
  current state.

## Remaining continuity work

### 1. Validate PR #16 as a complete package

Before promoting PR #16 to `main`:

- run the repository checks on an actual checkout of the exact PR head;
- fix any remaining migration-era check assumptions exposed by the new temporal
  semantics or documentation routes;
- review the complete diff for duplicate/conflicting authority;
- confirm every new Markdown file remains indexed/routed;
- confirm the casebook, branch audit, temporal policy, bootstrap, and copied
  MI-001 source report are internally coherent.

Do not merge merely to reduce branch count. Promote only when the package is
qualified enough to become normal `main` authority.

### 2. Prepare cold storage only when useful

The first audit identified candidates, but actual retirement remains separate.
For any branch the user later wants removed from GitHub:

1. create a self-contained Git bundle/archive;
2. create manifest + size + SHA-256 + exact head;
3. independently restore/verify the exact branch head;
4. discuss that named branch with the user;
5. delete only after explicit approval.

Do not spend an available hardware session manufacturing branch bundles unless
branch cleanup is the task the user chooses at that time.

### 3. Adapt identity-bound live development state

The historical PS2VNC status system demonstrated a valuable rule: saved live
`NEXT_INTENDED` is usable only when its exact repository/content identity still
matches.

PS-to-VNC should eventually adapt that behavior to its **multiple active
workstreams/worktrees** rather than copy the old single-global-state schema.
Important identity inputs may include branch, HEAD, tracked diff, build/DUT
identity, apparatus/session identity, and a timestamp.

This is useful continuity infrastructure but should not be rushed into the next
hardware session unless needed to resume safely.

### 4. Improve canonical tool discovery as new work lands

The clean-current context map now exists. Keep it focused on canonical
responsibilities rather than volatile branch heads.

When PR #15, PR #17, or later product work promotes new build/test/deploy tools,
route those canonical mechanisms through the map so a fresh chat does not
regenerate routine procedures.

### 5. Consider a generated context bundle only after real pressure

The historical project-concordance/context-bundle idea remains attractive, but
is deliberately deferred. First use the improved manual routing in real fresh
sessions. If important context is still repeatedly missed or startup remains too
slow, then a generated `context`/bootstrap receipt can earn implementation.

## Already captured from the user's requests

These requirements should not need to be rediscovered from chat:

- major-investigation casebook with MI-001 through MI-005;
- full Test14/MI-001 ExitHandler mechanism report copied into PS-to-VNC and
  linked from Issue #19;
- 576p retained as its own unresolved major investigation;
- branch deletion always requires explicit approval for that specific branch;
- cold storage is the preferred retirement path;
- mutable present-state prose is a timestamped snapshot;
- branch/workstream status must be understandable without the user memorizing
  branch names;
- brand-new chats should require as little user handoff as practical;
- historical PS2VNC continuity infrastructure is a resource to harvest, not a
  structure to copy blindly.

## Resume rule

When hands-on engineering time is available, this file is a backlog, not a
blocker. Resume Issue #5/PR #17 or Issue #7/PR #15 from their exact current
records first. Return to the remaining continuity work when doing so will not
waste an available hardware session.