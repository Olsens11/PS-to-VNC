DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-22T22:57:00-04:00
COMPLETED_AT=2026-09-22T23:08:09-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=8698890b4f839d5708e6cefc129c37608643e155
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R17 Pi MPEG mechanism; R18 Transport run boundary activated

## Objective and recovered authority

Receive the Reconstruction baton after `A003-PI-MPEG-CONTROL-PRODUCER-R17`,
recover current repository authority independently, verify the complete worker
range and exact machine evidence, then select and publish the smallest remaining
dependency before Application MPEG orchestration.

Live pickup authority was:

`8698890b4f839d5708e6cefc129c37608643e155`

with immutable Reconstruction record:

`docs/ledge/work-log/20260922T213555-0400__reconstruction__a003-mpeg-generation__interactive.md`

The worker's final pre-log source authority was:

`cc7dc1237957bfd288addc8379caae47e83bc5a6`

and exactly fifteen Reconstruction commits followed the assigning Foreman
closeout `826d19dd08efe48db9731ed4713f30b15be0e4b8` before the immutable R17 log.

Current authority read for this round included AGENTS/CONTRIBUTING/current status,
project intent, clean architecture, development naming/topology/lifecycle
contracts, Reconstruction/work-log contracts, Foreman State 0048, Wire Runtime
Decisions revision 0011, Architecture Overlay revision 0007, Q1-Q12
reconciliation, A003/A004/A006 semantic audits, current R17 source/tests, and
the current PS2 Transport/MPEG/Application public seams.

No Pi/local checkout was exposed to this Foreman seat. Staged, unstaged and
untracked external-worktree state remains unasserted; no local
`scripts/resume-state.sh` or `scripts/check.sh` execution is claimed.

## Exact R17 range reviewed

The fifteen pre-log commits were:

- `b72f58d00068ed63effd97bfaa38ee8035962867` — project MPEG runtime profile to Pi;
- `3d5f7d6d7ca6c90a1840ab71bd51536690880d67` — mirror exact MPEG generation control;
- `ba4d0893bbcc01efca1bd062ba2a6f3d41296850` — expose selected channel window to Pi;
- `ea5afa67ee9f9ce2df936338abe4b7747be0cf62` — add exact Pi MPEG generation owner;
- `1429416bbbc4bd7d5903da8ce2321bf8feecf749` — integrate MPEG rider with sole Wire owner;
- `d4723940d1e67931f22a3d61ee6869c497101f27` — prove exact MPEG generation lifecycle;
- `aabf33a06edf987369f3ba26e0acef367e30f073` — enroll R17 Pi MPEG lifecycle suite;
- `946a885557b3ab000b7785e2b7cf668ec5e9a8e7` — preserve R13 quiesce owner expression;
- `b1fe02cd84b37e3b8b011e137eda99ddbc740fa3` — record R17 MPEG generation boundary;
- `dcfd475cdf7da9625845679681e58349b474e857` — observe completion-state publication;
- `8cc27293fb37fe417c1c4d0e3f1573b9b0a01c8a` — strengthen freshness/buffer proof;
- `1f2a231986b290d0f0d8228f766b1aecb872b935` — deterministic dictionary reconciliation;
- `bd7fa8cd578e4a1d69f9d50b0bcd69a61489d9db` — reconcile clean definitions;
- `48e1118a9cfd67f1c9dff8caf93adb54607ee786` — describe R17 Pi ownership;
- `cc7dc1237957bfd288addc8379caae47e83bc5a6` — index R17 MPEG generation contract.

Material R17 source remained within the authorized Pi/configuration/test/
documentation/tooling surface. No PS2 Application, PS2 Transport, MPEG
decoder/worker/backend, Display/Presentation, Input/UI, RFB product source or
AUDIO product source changed.

## Independent R17 acceptance

All twelve R17 criteria are Foreman-accepted.

A003-R17-C1 PI_MPEG_CONTROL_CODEC_EXACTLY_MATCHES_ACCEPTED_PS2 — MET
A003-R17-C2 START_VALIDATES_ACTIVE_SESSION_GENERATION_AND_GEOMETRY — MET
A003-R17-C3 ONE_EXACT_GENERATION_OWNER_STALE_CONTROL_REJECTED — MET
A003-R17-C4 CAPTURE_AND_SUPPRESSION_PREPARED_FOR_EXACT_GENERATION — MET
A003-R17-C5 PRODUCER_LIFETIME_AND_BUFFERING_BOUNDED_FAIL_CLOSED — MET
A003-R17-C6 CHANNEL4_CREDIT_AND_SOLE_WIRE_SERIALIZATION_PRESERVED — MET
A003-R17-C7 RETIRE_CLOSES_ADMISSION_AND_DRAINS_INFLIGHT_SEND_LEASES — MET
A003-R17-C8 RETIRE_COMPLETION_ONLY_AFTER_REAL_PI_RETIREMENT — MET
A003-R17-C9 REPEATED_GENERATION_USES_FRESH_AUTHORITY_NO_STALE_REBIND — MET
A003-R17-C10 RFB_Q4_R16A_R13_R14_AND_PS2_MPEG_CONTRACTS_UNCHANGED — MET
A003-R17-C11 NO_APPLICATION_PRESENTATION_AUDIO_HEARTBEAT_OR_FINAL_SCOPE_CREEP — MET
A003-R17-C12 HOST_PROJECT_DICTIONARY_AND_BUILD_EVIDENCE_GREEN — MET

Independent source review established:

- START kind 11 / RETIRE kind 10 use control channel 0, flags zero and exact
  version-1 44/12-byte big-endian payloads symmetric with PS2 authority;
- one `MpegGenerationController` belongs to one ACTIVE Wire Session and
  fail-closes stale/session/geometry mismatch;
- the base rectangle is exact capture while suppression is a distinct exact
  outer footprint; inner matte remains PS2-local;
- `BufferedMpegProducer` owns one FFmpeg process/reader thread and a condition-
  bounded Configuration-selected buffer;
- unexpected producer EOF/error and unproven retirement remain failures;
- channel-4 CREDIT is session-scoped flow authority while each physical send
  holds an exact-generation local lease;
- `WireConnectionOwner` remains the only PS2-facing receiver, sender and global
  sequence allocator for MPEG and RFB alike;
- physical-frame readability is serviced before a simultaneous MPEG local wake,
  so a received RETIRE closes local emission before another producer flush;
- RETIRE completion is emitted only after producer/thread and suppression
  retirement, then local state returns to IDLE only after completion
  serialization succeeds;
- default `wire_runtime.py` still supplies no MPEG generation factory, so R17 is
  staged/dormant mechanism rather than hidden final product activation;
- the canonical MPEG JSON reproduces every accepted R7 value exactly and
  deterministically generates both C and Pi projections without adding
  H1 bitrate/GOP/VBV/laboratory tuning.

## R17 machine/build evidence

Exact final source run `35808953320` attempt 2 completed SUCCESS with:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link — PASS;
- current-source reproducibility — PASS.

Exact immutable-log-head run `35809286786` attempt 1 also completed SUCCESS
with the same canonical evidence classes.

Independent linked-identity inspection confirmed on both heads:

`ELF_PRISTINE_SHA256=615407bd6ce43722f6ead9ced75985d85bec18fa24e3e0f3a841c4af21894dfc`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=71846f590b0f46af905311d55a074e44ac1d514887dbdc7e87a5bd7472b18b3f`
`PT_LOAD_BYTES=487188`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

The whole ELF identity changed from R16B due to the Configuration projection
source/build inputs, while the loadable PT_LOAD fingerprint and size remained
exactly unchanged. No hardware qualification is inferred.

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R17
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

## Newly identified dependency before Application MPEG orchestration

Current PS2 Transport already owns the session-local MPEG queue, normal
decoder-consumption credit return, finite producer-done fact, START/RETIRE
outbound relay and one RETIRE-completion receive slot.

However, current source still lacks the A003-qualified old-run/new-run queue
boundary:

- no explicit MPEG DATA-admission interval open before START;
- `_accept_mpeg_frame()` accepts DATA whenever MPEG is enabled;
- accepting RETIRE completion stores one value but does not close DATA admission;
- taking completion clears only the completion slot and does not retain a
  retirement fence;
- `mpeg_channel` has no residual-discard/reset primitive;
- no operation combines residual discarded bytes with `mpeg_credit_pending` and
  returns the exact withheld credit once;
- `producer_done`, queue offsets/contents, retirement state and credit debt
  cannot yet be reset/reopened safely for a later run in the same Wire Session.

This is not Application policy. It is Transport channel/run fencing explicitly
required by A003 and Q6/Q7: one session-scoped queue epoch must completely close
and finalize before a later run can reuse that queue.

Historical `CP2P_LIVE_GENERATION_BOUNDARY.md` was used only to verify the proven
ordered-fence invariant: RETIRE completion closes channel-4 admission, local
consumer retirement precedes residual discard, residual + pending credit is
returned, then the same bounded queue may reopen. Its experiment-local class/
counter structure is not promoted.

## Foreman-owned publication

Published Foreman State revision 0049 at:

`3718615a9cf9d07c3598ecab6fbaeba09e5751a3`

with message:

`docs(foreman): accept R17 and activate MPEG run boundary`

State 0049 accepts R17 and activates:

`A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18`

R18 is restricted to the PS2 Transport MPEG queue/run fence: clean open/abort,
DATA admission gating, immediate closure on exact RETIRE completion, residual
discard/credit finalization, reset and safe same-session reopen.

`src/app.c`, Pi product source, MPEG decoder/worker/backend, Display/
Presentation, calibration/Input/UI, RFB product source and AUDIO product source
are outside R18.

Final Application MPEG orchestration remains dependency-queued after R18.

## State-0049 machine evidence

Exact State-0049 Actions run `35813070742` attempt 1 completed SUCCESS.

- host-unit — PASS;
- dedicated R16A PS2 RFB wait termination — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS.

The documentation-only State publication retained:

`ELF_PRISTINE_SHA256=615407bd6ce43722f6ead9ced75985d85bec18fa24e3e0f3a841c4af21894dfc`
`PT_LOAD_SHA256=71846f590b0f46af905311d55a074e44ac1d514887dbdc7e87a5bd7472b18b3f`
`PT_LOAD_BYTES=487188`

## Hardware/evidence boundary

R17 Pi MPEG generation/producer code has not been physically activated or
qualified. R16A/R16B recovery remains hardware-unqualified. R18 must preserve
new exact ELF/PT_LOAD identity if its linked PS2 source changes; repository
compile/link evidence never substitutes for physical qualification.

## Next pickup

Reconstruction should recover current branch authority and execute only
`A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18` from Foreman State 0049.

It must not begin `src/app.c` MPEG activation, Pi changes, decoder/worker/
presentation composition, AUDIO, heartbeat, CONFIG-on-Wire expansion, RFB
redesign or unrelated cleanup.

If safe Transport finalization requires Application-owned proof that is not
available through the current bridge, return a truthful `BLOCKED` record naming
that exact missing interface instead of substituting sleeps, timeouts,
diagnostics or implicit state guesses.

At shift end emit exactly one immutable Reconstruction work-log record and stop.
