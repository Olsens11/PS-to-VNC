# Current Project Status

Temporal role: `SNAPSHOT`

Recorded at: `2026-09-02T15:35:00-04:00`

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

Do **not** consume an available hardware session merely finishing documentation
or branch cleanup. The remaining continuity work is durably captured in
`docs/development/CONTINUITY_FOLLOWUPS.md`.

Resume technical work from the exact current Issue/PR/evidence records rather
than treating this prose snapshot as a substitute for live reconciliation.

### Issue #5 / PR #17 — clean Pi and RFB endpoint lifecycle

Recorded authorities at this snapshot:

- foundation branch: `pi/issue5-clean-baseline`;
- foundation head: `87baebce32e3c07ffc12298d6a168ac890231cf2`;
- evaluating branch: `pi/issue5-rfb-socket-activation-candidate`;
- candidate head observed during the branch audit:
  `66988ebcfb1eed57cb84f422fb84bdb6faf98952`;
- draft PR #17: `Evaluate PS2-facing RFB endpoint lifecycle`.

The next hands-on direction remains the documented Xtigervnc path:

1. inspect live Pi state before mutation, especially remaining exploratory
   w0/kanshi/helper state;
2. preserve the w0 experiment as reference only;
3. re-establish the documented dedicated packaged Xtigervnc path;
4. instrument untouched cold/no-carrier -> PS2 carrier -> NetworkManager address
   -> port-5900-listener sequencing;
5. qualify the systemd socket-activation + Xtigervnc `-inetd` candidate against
   the conventional persistent-provider control only after baseline timing is
   measured;
6. keep NetworkManager identity, provider choice, and service lifecycle as
   separate decisions;
7. reconcile live Pi mutations into Issue #5 before promotion.

Socket activation, a no-carrier override, w0/WayVNC, and historical traffic
shaping remain unpromoted unless their own evidence gates say otherwise.

### Issue #7 / PR #15 — clean minimal Raw/480p PS2 core

Recorded authority:

- `reconstruct/issue7-minimal-core`;
- branch head observed during the branch audit:
  `e7b4a35f681101fe39e59c4b04b28740b0f536de`;
- draft PR #15: `Reconstruct minimal Raw 480p PS2↔Pi core`.

The exact clean fixed-480p / Raw / incremental RFB / GS behavior has already
been physically demonstrated on the stamped DUT. Formal HW1 promotion still
failed because the diagnostic capture observed only:

`PSTVNC_STAGE DESKTOP_READY`

and did not observe the earlier runtime identity, `NET_READY`, or `GS_READY`
datagrams.

If Issue #7 is resumed, keep the next experiment narrow: isolate why early
startup/runtime-identity diagnostic `sendto` evidence is absent while the later
`DESKTOP_READY` send succeeds. Do **not** perturb the demonstrated RFB/Raw/GS
path merely to chase the evidence defect.

## Continuity work completed before this snapshot

The documentation/continuity branch is PR #16,
`docs/fresh-session-reconstruction`, still draft.

Completed during the current cleanup pass:

- full 11-branch GitHub inventory and provenance/lifecycle audit;
- review and harvest of all three unique `reconstruction/m1-minimal-core`
  commits;
- classification of four fully merged reboot/audit/state branches as historical
  cold-storage candidates, with no deletion authorization;
- protected w0vncserver archive classification retained;
- PS2VNC continuity-system harvest covering bootstrap, status, TestKit,
  provenance, historical-harvest, known-good, and context-routing lessons;
- root README now exposes the one-line fresh-chat handoff;
- `START_HERE.md` is now a compatibility redirect rather than a competing
  bootstrap contract;
- clean-current tool/context router added at
  `docs/reference/CURRENT_TOOL_AND_CONTEXT_MAP.md`;
- migration-era `FILE_AND_SERVICE_MAP.md` explicitly demoted to historical/
  compatibility routing;
- temporal snapshot rules wired into status documents, resume output, and
  continuity checks;
- PR #16 description refreshed to record its expanded scope and promotion gate.

Detailed continuation/backlog:

`docs/development/CONTINUITY_FOLLOWUPS.md`

## What remains before PR #16 should be promoted

- run `scripts/check.sh` on an actual checkout of the exact PR head;
- fix any migration-era checker assumptions that conflict with the new temporal
  semantics or routing model;
- verify every new Markdown file remains indexed/routed;
- review the complete PR diff for duplicate/conflicting authority;
- keep branch retirement as a separate cold-storage + explicit-user-approval
  workflow.

Those items are **not a blocker for the next available hardware session**.

## Major-investigation casebook on PR #16

- `MI-001` — `RESOLVED`: GS/EE HIRES HSync / `ExitHandler()` handoff;
- `MI-002` — `OPEN_NON_BLOCKING`: MTU1458 / SMAP RX corruption;
- `MI-003` — `OPEN_NON_BLOCKING`: PS2 receive burst tolerance / sender pacing;
- `MI-004` — `OPEN_NON_BLOCKING`: display-transition liveness / green-corruption
  family;
- `MI-005` — `OPEN_NON_BLOCKING`: persistent 576p low-level transition stall.

MI-001 includes the complete original Test14 mechanism report copied into the
successor repository so ordinary comprehension does not require the legacy
PS2VNC repository.

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

The intended instruction is:

> Open `Olsens11/PS-to-VNC`. Read `AGENTS.md` and follow the fresh-session
> reconstruction procedure completely before doing any project work.

A capable session must reconcile timestamps and current GitHub activity before
reusing this snapshot's present-tense claims.