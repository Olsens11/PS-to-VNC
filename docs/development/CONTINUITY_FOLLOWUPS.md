# Development Continuity Follow-ups

Temporal role: `SNAPSHOT_BACKLOG`

Recorded at: `2026-09-02T15:14:42-04:00`

This file records continuity/infrastructure work explicitly requested by the
user that may be set aside while hands-on PS-to-VNC engineering takes priority.
It is not product roadmap authority and it is not proof that every item remains
current forever. Reconcile it with later work before resuming under
`docs/development/TEMPORAL_STATE_SEMANTICS.md`.

The purpose is simple: do not make the user re-explain these requests later.

## Priority when hands-on work resumes

Do **not** block an available hardware session merely to finish documentation
cleanup. Preserve this backlog and resume the active technical qualification
work from its own exact issue/PR/evidence authority.

At this snapshot the two important technical workstreams are:

### Issue #5 / PR #17 — clean Pi and PS2-facing RFB lifecycle

Resume from the existing Issue #5 and PR #17 records rather than improvising a
new Pi design.

The next hands-on sequence should remain conservative:

1. inspect the live Pi before mutation, including the exploratory w0/kanshi/helper
   state that still needs reconciliation;
2. preserve the exact w0vncserver experiment as reference only;
3. return to the documented dedicated packaged Xtigervnc path;
4. capture the cold/no-carrier -> PS2 carrier -> NetworkManager address -> port
   5900 listener timing before changing the lifecycle mechanism;
5. stage/qualify the systemd socket-activation + Xtigervnc `-inetd` candidate
   only through PR #17's documented comparison against the conventional
   persistent-provider control;
6. keep NetworkManager ownership, RFB provider choice, and lifecycle policy as
   separate decisions;
7. reconcile every live package/config/service mutation into Issue #5 before
   promotion.

Do not silently promote socket activation, the no-carrier override, w0/WayVNC,
or historical traffic shaping merely because an experiment works.

The clean Pi's repository-scoped read-only deploy-key path already exists.
Do not let credential cleanup or a new write-path experiment derail a hardware
session unless it is actually required for the test being run.

### Issue #7 / PR #15 — clean minimal Raw/480p PS2 core

The physical Raw/480p/RFB/GS path has already been demonstrated on the stamped
DUT, but the formal HW1 promotion failed because only the late
`PSTVNC_STAGE DESKTOP_READY` UDP diagnostic was observed. The early runtime
identity, `NET_READY`, and `GS_READY` evidence was absent.

If Issue #7 is resumed, the next formal experiment should stay narrow: isolate
why the early startup/runtime-identity `sendto` evidence disappears while the
later `DESKTOP_READY` send succeeds. Do not disturb the demonstrated RFB/Raw/GS
path merely to chase the evidence defect.

## User-requested continuity work to resume later

### 1. Finish branch deobfuscation / lifecycle audit

- keep `docs/development/BRANCH_WORKSTREAM_INDEX.md` understandable and
  freshness-checked;
- finish the substantive harvest of `reconstruction/m1-minimal-core`, which had
  three unique commits at the recorded audit point and must not be treated like
  a fully merged branch;
- finish reviewing the already-merged audit/reboot/state branches for eventual
  retirement disposition;
- never delete a branch automatically;
- before any branch deletion, create and independently verify cold-storage Git
  preservation when appropriate, then discuss that **specific branch** with the
  user and obtain explicit approval.

### 2. Finish PR #16 as a coherent package

Complete review/reconciliation of the documentation-continuity package before
promotion to `main`, including:

- fresh-session reconstruction;
- major-investigation casebook and its navigation;
- copied full MI-001/Test14 ExitHandler mechanism report so ordinary future
  comprehension does not require the legacy repository;
- branch lifecycle/cold-storage policy;
- branch/workstream index;
- temporal-state semantics;
- cross-links and documentation routing.

Do not leave foundational continuity material stranded indefinitely on a draft
branch, but do not merge merely for tidiness before the package is internally
coherent.

### 3. Deeply mine the PS2VNC documentation/development system

The historical PS2VNC repository is a valuable read-only source of lessons.
Deliberately review its successful continuity infrastructure rather than copying
its product architecture blindly. Important surfaces already identified include:

- `docs/START_HERE.md` and its bootstrap-completeness contract;
- `docs/DOCUMENTATION.md`;
- `docs/DEVELOPMENT_INFRASTRUCTURE.md`;
- `docs/status/STATUS_MODEL.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`;
- `docs/testing/TEST_TOOLKIT.md`;
- documentation-by-default event/commit/check mechanics;
- resume-state and development-wrap concepts;
- durable machine state and stale-state reconciliation;
- context routing / project-concordance ideas;
- historical harvesting, evidence/provenance separation, and known-good-build
  routing.

Classify each lesson as adopt, adapt, defer, or reject for PS-to-VNC. Preserve
useful principles without recreating obsolete PS2VNC structure by inertia.

### 4. Make brand-new conversations faster to bootstrap

The target user experience is that a new chat needs only a very short handoff,
ideally equivalent to:

> Open `Olsens11/PS-to-VNC`, read `AGENTS.md`, and complete the repository's
> fresh-session reconstruction procedure before doing project work.

Improve the system until that instruction reliably yields an accurate working
context with minimal user intervention.

Specific goals:

- quickly discover what is actually active now;
- know exact branch/commit authority for each active workstream;
- distinguish `main` authority from branch-local candidates;
- load relevant open **and resolved** major investigations;
- find last proven result and next safe action without asking the user to retell
  project history;
- find canonical tools instead of regenerating procedures;
- route deeper context progressively rather than blindly rereading everything;
- consider a one-command/context-bundle mechanism only if real use demonstrates
  that it improves orientation.

### 5. Harden temporal/current-state semantics mechanically

The semantic rule now exists: mutable status records are timestamped snapshots,
not eternal present-tense facts.

Later infrastructure work should determine where mechanical checks are worth
adding, especially for designated status-like files. Possible checks include:

- required ISO-8601 recorded-at metadata;
- explicit snapshot/current-at-recorded-time role;
- rejection of a supposedly current state file that predates clearly newer
  authoritative activity without reconciliation;
- checks that branch/workstream inventories expose their observation time;
- checks that fresh-session output says `recorded as of ...` when currentness has
  not been proved.

Do **not** use an arbitrary age threshold as the sole definition of staleness.
Git/PR/issue/live chronology determines whether freshness-sensitive claims were
superseded.

## Already captured from this discussion

The following requests are already represented elsewhere on PR #16 and should
not need to be rediscovered from chat:

- major-investigation casebook with MI-001 through MI-005;
- MI-001 full Test14 mechanism report copied into PS-to-VNC and linked from
  Issue #19;
- 576p preserved as its own unresolved major investigation rather than folded
  into the broader display-liveness case without evidence;
- branch deletion requires branch-specific explicit user approval;
- cold storage is the preferred path before retiring a live branch;
- mutable status/current-state prose must carry temporal snapshot semantics.

## Resume rule

When hands-on engineering time is available, this file is a backlog, not a
blocker. Resume Issue #5/PR #17 or Issue #7/PR #15 from their exact current
records first. Return to these continuity follow-ups when doing so will not waste
an available hardware session.
