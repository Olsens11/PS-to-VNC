# Current Project Status

Temporal role: `SNAPSHOT`

Recorded at: `2026-09-02T15:14:42-04:00`

This is the concise human-readable project-status snapshot **as recorded at the
time above**. It is not an eternal present-tense authority. Before repeating any
`current`, `active`, `next`, `blocked`, progress, branch-head, or workstream
claim later, reconcile it with newer GitHub/repository/live state under
`docs/development/TEMPORAL_STATE_SEMANTICS.md`.

## Project intent

PS-to-VNC is the clean successor to the historical PS2VNC research project.

Governing reconstruction principle:

> **Rebuild PS-to-VNC as the program we would have written if we had known at
> the beginning everything the exploratory implementation taught us.**

Integrated architecture authority at this snapshot remains
`docs/CLEAN_ARCHITECTURE.md` on `main`.

Recorded `main` head:

`34b53d724dede1f3875d136a8adf59ce888fa680`

## Hands-on priority when the operator returns

Do **not** consume an available hardware session merely finishing branch or
documentation cleanup. The continuity work has been durably captured in
`docs/development/CONTINUITY_FOLLOWUPS.md` and may be resumed later.

The active technical work should resume from the exact Issue/PR records rather
than from this prose snapshot.

### Issue #5 / PR #17 — clean Pi and RFB endpoint lifecycle

Recorded active authorities:

- foundation branch: `pi/issue5-clean-baseline`;
- foundation head observed during this snapshot cycle:
  `87baebce32e3c07ffc12298d6a168ac890231cf2`;
- evaluating lifecycle branch: `pi/issue5-rfb-socket-activation-candidate`;
- draft PR #17: `Evaluate PS2-facing RFB endpoint lifecycle`.

The next hands-on direction is the already-documented Xtigervnc path, not new
provider exploration:

1. inspect live Pi state before mutation, especially remaining exploratory
   w0/kanshi/helper state;
2. preserve the w0 experiment as reference only;
3. re-establish the documented dedicated packaged Xtigervnc path;
4. instrument untouched cold/no-carrier -> PS2 carrier -> NetworkManager
   address -> port-5900-listener sequencing;
5. qualify the systemd socket-activation + Xtigervnc `-inetd` candidate against
   the conventional persistent-provider control only after the baseline timing
   is measured;
6. keep NetworkManager identity, provider choice, and service lifecycle as
   separate decisions;
7. reconcile live Pi mutations into the Issue #5 dependency/provisioning record
   before promotion.

Socket activation, a no-carrier override, w0/WayVNC, and historical traffic
shaping remain unpromoted unless their own evidence gates say otherwise.

### Issue #7 / PR #15 — clean minimal Raw/480p PS2 core

Recorded branch authority:

- `reconstruct/issue7-minimal-core`;
- draft PR #15: `Reconstruct minimal Raw 480p PS2↔Pi core`.

The exact clean fixed-480p / Raw / incremental RFB / GS behavior has already
been physically demonstrated on the stamped DUT. Formal HW1 promotion still
failed because the diagnostic capture observed only:

`PSTVNC_STAGE DESKTOP_READY`

and did not observe the earlier runtime identity, `NET_READY`, or `GS_READY`
datagrams.

If Issue #7 is resumed, keep the next experiment narrow: isolate why the early
startup/runtime-identity diagnostic `sendto` evidence is absent while the later
`DESKTOP_READY` send succeeds. Do **not** perturb the demonstrated RFB/Raw/GS
path merely to chase the evidence defect.

## Development-continuity work captured but allowed to wait

The user-requested follow-up backlog is now explicit in:

`docs/development/CONTINUITY_FOLLOWUPS.md`

It includes:

- finish branch deobfuscation/lifecycle audit;
- specially harvest the three unique commits from
  `reconstruction/m1-minimal-core` before any retirement recommendation;
- review fully merged historical branches for possible verified cold-storage
  retirement;
- never delete any branch without explicit approval for that specific branch;
- finish/reconcile PR #16 and eventually promote the continuity/casebook package
  when appropriate;
- deeply mine the historical PS2VNC `START_HERE`, documentation, status,
  TestKit, file/service-map, resume-state, development-wrap, context-routing,
  provenance, and documentation-by-default systems for reusable lessons;
- make brand-new conversations require as little user handoff as practical;
- mechanically harden timestamp/snapshot semantics for mutable status records
  where worthwhile.

That backlog is deliberately **not a blocker for the next hardware session**.

## Major-investigation casebook recorded on PR #16

- `MI-001` — `RESOLVED`: GS/EE HIRES HSync / `ExitHandler()` handoff;
- `MI-002` — `OPEN_NON_BLOCKING`: MTU1458 / SMAP RX corruption;
- `MI-003` — `OPEN_NON_BLOCKING`: PS2 receive burst tolerance / sender pacing;
- `MI-004` — `OPEN_NON_BLOCKING`: display-transition liveness / green-corruption
  family;
- `MI-005` — `OPEN_NON_BLOCKING`: persistent 576p low-level transition stall.

MI-001 also has the complete original Test14 mechanism report copied into the
successor repository so ordinary future comprehension does not require opening
the legacy PS2VNC repository.

## Protected boundaries

- historical PS2VNC repository remains read-only historical/provenance authority;
- no branch deletion without branch-specific explicit user approval;
- preferred branch retirement path is verified cold storage before deletion;
- do not reset/clean/discard unexplained dirty worktree state;
- do not silently substitute PS2IP/network dependencies;
- machine evidence and physical/operator observation remain separate;
- exploratory provider or lifecycle success does not equal architecture
  promotion;
- old status snapshots state what was recorded then, not necessarily what is
  true now.

## Fresh-chat minimal handoff

The intended short instruction remains:

> Open `Olsens11/PS-to-VNC`. Read `AGENTS.md` and follow the fresh-session
> reconstruction procedure completely before doing any project work.

A capable session must reconcile timestamps and current GitHub activity before
reusing this snapshot's present-tense claims.
