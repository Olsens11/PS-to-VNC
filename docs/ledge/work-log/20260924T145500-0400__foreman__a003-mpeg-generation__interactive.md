DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T14:55:00-04:00
COMPLETED_AT=2026-09-24T15:04:36-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=964d1cfa8db8c834d9ee32a1541c42ffb1aace70
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R21 MPEG run start accepted; R22 live service activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A003-APPLICATION-MPEG-RUN-START-R21`, independently recover live repository
authority, inspect the complete worker range and immutable closeout, review all
R21 criteria and evidence, then select the smallest safe next dependency.

Live pickup authority was the immutable R21 Reconstruction closeout:

`964d1cfa8db8c834d9ee32a1541c42ffb1aace70`

with final pre-log source authority:

`fc8bb2652d94d8163e1e5b375e33c9e37b73017b`

and assigning Foreman authority:

`05f8dd5cbcacbc46510f2d04f229e593def6601b`.

Exactly seven pre-log Reconstruction commits followed the assigning authority;
the immutable R21 log was the eighth commit and no later commit existed at
Foreman pickup.

Authority refreshed this round included AGENTS/CONTRIBUTING/current status,
project intent, clean architecture, development naming/topology/module
lifecycle rules, Reconstruction/work-log contracts, Foreman State 0056,
Architecture Overlay revision 0007, Wire Runtime Decisions revision 0011,
Q1-Q12 reconciliation, A001/A003/A004/A005/A006 audits, current R21 source and
tests, and the accepted P2/P3/P7/R18/R20/R3/R4/R5/R7 seams relevant to
Application run-start and the next dependency.

No external Pi/local checkout was exposed to this Foreman seat. External
staged/unstaged/untracked state remains unasserted; no local
`scripts/resume-state.sh` or `scripts/check.sh` execution is claimed.

## R21 independent source acceptance

R21 stayed within the packet-authorized Application coordinator/test/build/
topology/documentation/dictionary surface. Ordinary `src/app.c` and existing
lower-owner product implementation remained unchanged.

Independent Foreman criterion dispositions:

A003-R21-C1 APPLICATION_OWNS_MONOTONIC_SESSION_LOCAL_RUN_GENERATION — MET
A003-R21-C2 ACCEPTED_GEOMETRY_HAS_ONE_BASE_INNER_SUPPRESSION_AUTHORITY — MET
A003-R21-C3 START_BASE_AND_SUPPRESSION_MATCH_PRESENTATION_SNAPSHOT_EXACTLY — MET
A003-R21-C4 EXISTING_RFB_PROTECTION_IS_REQUIRED_NOT_STOLEN_OR_RELEASED — MET
A003-R21-C5 R18_RUN_OPEN_PRECEDES_MPEG_WORKER_CONSUMER_ACTIVITY — MET
A003-R21-C6 FRESH_R5_R3_R4_EXECUTION_OWNERS_BIND_EXACT_GENERATION — MET
A003-R21-C7 P3_WAIT_FIRST_FRAME_AND_P7_CONSUMER_READY_BEFORE_START — MET
A003-R21-C8 START_IS_FINAL_IRREVERSIBLE_STARTUP_ACTION — MET
A003-R21-C9 EVERY_PRE_START_FAILURE_PROVES_REVERSE_ORDER_UNWIND_OR_FAULTS — MET
A003-R21-C10 START_ATTEMPT_FAILURE_NEVER_FALSELY_USES_PRE_START_ABORT — MET
A003-R21-C11 NO_RETIREMENT_PI_TRIGGER_AUDIO_OR_ORDINARY_APP_SCOPE_CREEP — MET
A003-R21-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

All twelve R21 criteria are MET.

## Key verified R21 behavior

`src/app_mpeg_run.*` is a trigger-agnostic Application-owned cross-domain
transaction, not a new private feature mechanism.

Verified startup ordering:

1. select/copy the accepted R7 runtime profile;
2. validate the single resolved base/inner/suppression geometry value against
   P3 structure and decoder-profile maximum dimensions;
3. require P2 already to deny remote publication;
4. require P3 currently RFB_ONLY;
5. allocate one new nonzero session-local generation without reuse;
6. open one clean R18 Transport MPEG run;
7. initialize fresh R5 PS2 worker runtime and operation tables;
8. initialize fresh R3 PS2 decoder backend operations;
9. start fresh R4 worker for the exact generation/profile;
10. arm P3 WAIT_FIRST_FRAME and re-read the immutable exact snapshot;
11. initialize P7 against that worker, Presentation, media clock and scheduler;
12. build R20 START from the retained P3 base/suppression snapshot and invoke
    START as the final startup action.

The inner matte remains Presentation-only and is never serialized in START.

Generation allocation is monotonic for one coordinator/Wire-session lifetime.
An admitted failed attempt consumes its generation permanently; a later attempt
receives the next value. `UINT32_MAX` exhaustion fails closed without wrapping
to zero.

Geometry review confirmed R21 matches the accepted P3/calibration contract:
macroblock-sized base, nonempty/overflow-safe rectangles, symmetric inner inset,
suppression containment and selected decoder max-width/max-height bounds.

## Pre-START failure/unwind review

Every failure before START invokes reverse ownership retirement:

- discard only the not-yet-serviced P7 value;
- abort exact pending P3 WAIT_FIRST_FRAME ownership;
- request exact R4 worker stop;
- require exact worker join and outcome;
- release the exact worker;
- release R5 runtime resources;
- abort the pristine R18 pre-START run boundary last.

Returning to IDLE is allowed only after those acquired owners are proven
retired. Cleanup failure faults the coordinator and requires outer session
teardown instead of pretending generation/resources are reusable.

Foreman independently checked the accepted lower-owner contracts:

- P7 initialization owns no external resource before success;
- R5 partial initialization exposes `resources_owned` and can be explicitly
  released or left faulted if cleanup is unproven;
- R4 stack/thread-create failures self-clean; a thread-start failure either
  self-cleans after proven destroy or leaves detectable partial worker
  ownership. R21 detects retained `worker.initialized` after a failed start and
  faults rather than manufacturing cleanup.

## START irreversible boundary

R21 sets the START-invoked fact immediately before calling the R20 semantic
START operation. A non-OK result after that call is treated as potentially
having emitted bytes: the coordinator faults, requires session teardown, and
does not call R18 pre-START abort or local pre-START cleanup.

Successful START leaves:

`PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME`

with exact generation, live worker/runtime, armed P3 and initialized P7.

R21 deliberately does not service frames, promote first-frame ownership, retire
the run, thaw/reveal RFB, select a calibration/input trigger, activate Pi MPEG
production, or modify ordinary Application product flow.

## R21 exact machine/build evidence

Final source authority `fc8bb2652d94d8163e1e5b375e33c9e37b73017b`:

- GitHub Actions run `36035059916`, attempt 1 — SUCCESS.

Immutable closeout authority `964d1cfa8db8c834d9ee32a1541c42ffb1aace70`:

- GitHub Actions run `36035354181`, attempt 1 — SUCCESS.

Both exact heads passed:

- host-unit;
- project-check;
- complete strict source-dictionary audit;
- pinned PS2 compile;
- pinned PS2 link/current-source reproducibility.

Observed host evidence includes:

- `transport bridge tests passed`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- Pi R17 MPEG generation fixture — 12 tests, OK;
- `app R15/R16B/R19 tests: PASS`;
- `app_mpeg_run_test: PASS`.

Accepted R21 linked identity:

`ELF_PRISTINE_SHA256=82ea220b084d50a857957f68950c10e8e7eab6a23e4857ab923e61f713c03ac8`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=910597af49fe2121ab730a2db82aa4259177256156062853bd2a008f0d4d6045`
`PT_LOAD_BYTES=494996`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R21
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

R21 changes PT_LOAD relative to accepted R20E. This is the current exact
repository-reproducible hardware debt and does not inherit physical
qualification.

## Next dependency review

Foreman did not jump directly from successful START to retirement.

R21 intentionally ends in STARTED_WAIT_FIRST_FRAME. The accepted P7 frame
consumer already owns the next mechanisms: exact worker-frame claims, first
synchronized physical presentation, shared-media-clock arm through the
compositor, P3 promotion to MPEG_OWNED, and later scheduler WAIT/PRESENT/DROP
service.

The R21 run owner currently has no service API and no MPEG_OWNED Application
state. A later caller would otherwise have to reach directly into the embedded
`frame_consumer` or infer Application lifecycle from P3 internals.

The smallest next dependency is therefore one Application-owned live-service
composition seam around existing P7, before retirement.

R22 must not duplicate P7 mechanics. It records only the cross-domain run state
that Application genuinely owns:

- STARTED_WAIT_FIRST_FRAME before first synchronized promotion;
- MPEG_OWNED after exact P7/P3 first-frame success;
- FAULTED when P7/lower-state facts contradict the live transaction.

Unexpected worker completion without an Application stop/retirement request is
A003 failure, not normal EOF/retirement proof.

## Foreman-owned publication

Published Foreman State revision 0057 at:

`6b0e06c99f950a3e1f145c38321b5fd0828b3802`

with message:

`docs(foreman): accept R21 and activate MPEG live service`

State 0057 activates only:

`A003-APPLICATION-MPEG-LIVE-SERVICE-R22`

R22 may modify only `src/app_mpeg_run.*`, focused Application service tests,
directly affected root dictionaries/documentation, and genuinely necessary
test/build enrollment.

R22 may not modify P7 itself, Display/Presentation/compositor/scheduler, MPEG
worker/backend/runtime, Transport, RFB policy, Configuration, Pi product, AUDIO,
Input/UI/calibration, or ordinary `src/app.c`. If composition exposes an
accepted lower-owner defect, Reconstruction must return BLOCKED rather than
broadening the packet.

Retirement and ordinary product activation remain deferred.

## State-0057 exact-head machine evidence

Exact State-0057 workflow run `36045426415`, attempt 1, completed SUCCESS.

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS.

The documentation-only State publication retained exact accepted R21 identity:

`ELF_PRISTINE_SHA256=82ea220b084d50a857957f68950c10e8e7eab6a23e4857ab923e61f713c03ac8`
`PT_LOAD_SHA256=910597af49fe2121ab730a2db82aa4259177256156062853bd2a008f0d4d6045`
`PT_LOAD_BYTES=494996`

## Next pickup

Reconstruction must recover current branch authority and execute only:

`A003-APPLICATION-MPEG-LIVE-SERVICE-R22`

from Foreman State revision 0057.

Compose the existing P7 frame service through the R21 run owner, record only
the exact first-frame Application state transition and live-service/failure
facts, and preserve all lower-owner evidence on failure.

Do not execute retirement, choose a product trigger, activate ordinary
`src/app.c`, enable the Pi MPEG product path, thaw RFB, or broaden any accepted
lower-owner mechanism.

At shift end emit exactly one immutable Reconstruction work-log record and stop.
