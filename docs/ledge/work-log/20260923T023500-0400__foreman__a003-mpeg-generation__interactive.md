DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-23T02:35:00-04:00
COMPLETED_AT=2026-09-23T02:45:59-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=ce1d405bde73ee764408c2dddcc40d2f8cc6eccc
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R18 MPEG Transport run boundary; R19 RFB flow composition activated

## Objective and recovered authority

Receive the Reconstruction baton after `A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18`,
recover current repository authority independently, verify the exact worker
range including its late concurrency correction, classify exact machine/build
evidence, then publish only the smallest next dependency before MPEG
Application orchestration.

Live pickup authority was:

`ce1d405bde73ee764408c2dddcc40d2f8cc6eccc`

with immutable Reconstruction record:

`docs/ledge/work-log/20260922T234027-0400__reconstruction__a003-mpeg-generation__interactive.md`

The worker's final pre-log source authority was:

`2e0589fd9fe531c5dfb7130df301b6f0eadd09fd`

and exactly seventeen Reconstruction commits followed assigning Foreman
authority `95d7b77e431f0f0765491da79e75591aae6e0300` before the immutable R18 log.

Current authority read for this round included AGENTS/CONTRIBUTING/current
status, project intent, clean architecture, development naming/topology/module
lifecycle contracts, Reconstruction/work-log contracts, Foreman State 0049,
Wire Runtime Decisions revision 0011, Architecture Overlay revision 0007,
Q1-Q12 reconciliation, A001/A003/A004/A006 semantic audits, current R18
Transport source/tests, current Application RFB loop, A004 RFB flow policy,
Presentation/frame-consumer/calibration public seams, and the newest immutable
Reconstruction/Foreman records.

No Pi/local checkout was exposed to this Foreman seat. External staged,
unstaged and untracked worktree state remains unasserted; no local
`scripts/resume-state.sh` or `scripts/check.sh` execution is claimed.

## Exact R18 source review

R18 remained inside its authorized Transport/test/documentation/dictionary
surface. No Application, Pi product, RFB parser/session, AUDIO product, MPEG
decoder/worker/backend, Display/Presentation, calibration/Input/UI, or protocol-
framing product source changed.

Independent review accepts all twelve packet criteria:

A003-R18-C1 MPEG_RUN_BOUNDARY_OPENS_ONLY_FROM_CLEAN_IDLE_TRANSPORT — MET
A003-R18-C2 PRE_START_ABORT_RESTORES_PROVEN_CLEAN_BOUNDARY — MET
A003-R18-C3 MPEG_DATA_ADMISSION_REQUIRES_OPEN_NONRETIRING_RUN — MET
A003-R18-C4 RETIRE_COMPLETION_ATOMICALLY_CLOSES_DATA_ADMISSION — MET
A003-R18-C5 COMPLETION_TAKE_DOES_NOT_REOPEN_CHANNEL — MET
A003-R18-C6 RESIDUAL_QUEUE_DISCARD_IS_ATOMIC_AND_NOT_CONSUMPTION — MET
A003-R18-C7 RESIDUAL_PLUS_PENDING_CREDIT_RETURNED_EXACTLY_ONCE — MET
A003-R18-C8 FINALIZATION_RESETS_OLD_RUN_TRANSPORT_STATE — MET
A003-R18-C9 LIVE_WAITER_OR_UNPROVEN_CONSUMER_BLOCKS_FINALIZATION — MET
A003-R18-C10 FRESH_SUCCESSOR_RUN_CANNOT_OBSERVE_OLD_RUN_STATE — MET
A003-R18-C11 RFB_AUDIO_WIRE_R17_AND_MEDIA_OWNER_CONTRACTS_UNCHANGED — MET
A003-R18-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

Key independently verified behavior:

- `pstvnc_transport_mpeg_run_open()` requires one fully clean session-local
  MPEG channel: no prior run/control/retirement/finalization, no queued bytes,
  no producer-done fact, no pending credit/activity/waiter or completion state;
- pre-START abort succeeds only while that opened boundary remains pristine;
- channel-4 DATA is admitted only while an explicit run is open and neither
  retirement nor finalization has closed the admission gate;
- the Pi RETIRE completion must exactly match the submitted RETIRE payload;
- completion acceptance takes the MPEG queue lock, closes `mpeg_run_open`, and
  latches retirement before publishing the completion slot;
- consuming the completion slot does not reopen DATA admission;
- residual discard returns the exact old-run queue byte count and is explicitly
  distinct from decoder consumption;
- finalization adds residual discarded bytes to already-withheld consumed-byte
  credit exactly once and uses the existing Transport outbound path;
- successful finalization clears queue/producer/control/activity/credit and
  retirement facts before permitting a later clean open;
- a later N+1 run reuses the same bounded queue allocation without inheriting N
  bytes, producer-done, completion, pending credit or activity state.

## Late R18 concurrency correction independently reviewed

The worker found a real race after an earlier green checkpoint and corrected it
before closeout. Foreman review treats the final correction as required R18
authority, not optional follow-up.

The corrected source now keeps one `mpeg_consumer_active` transaction live from
queue dequeue through the existing consumed-byte credit-return operation. A
finalizer therefore cannot snapshot/reset `mpeg_credit_pending` after bytes
leave the queue but before those bytes enter credit accounting.

Finalization also publishes `mpeg_finalization_in_progress` before releasing the
MPEG queue lock for synchronous final-credit serialization. While that flag is
set, late MPEG read/status/activity-wait/producer-done operations cannot acquire
new run-private authority. After the send completes, finalization reacquires the
queue lock and fails the Wire runtime if any guarded old-run fact reappeared.

Deterministic tests cover the late-consumer fence, active-consumer blocking, live
activity-waiter blocking, exact RETIRE correlation, post-completion DATA
failure, exact residual+pending credit return, and N->N+1 cleanliness.

## R18 machine/build evidence

Exact final source run `35816881762` attempt 1 completed SUCCESS with:

- host-unit — PASS;
- dedicated R16A PS2 RFB wait-termination proof — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link — PASS;
- current-source reproducibility — PASS.

Exact immutable-log-head run `35817077825` attempt 1 completed SUCCESS with the
same canonical evidence classes.

Host logs explicitly include:

- `transport bridge tests passed`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- R17 Pi MPEG suite: 12 tests, OK.

Exact R18 linked identity on both final source and immutable-log heads:

`ELF_PRISTINE_SHA256=4d0bc02f6bc89f138acb53ddb7751fd9e91a300b9e7cbfd9bc7301920f08d1c4`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=e244769ef21dadd8a09e6fe65ec4b2201e0acd9beb52ae06180ab3d4f3a3a232`
`PT_LOAD_BYTES=490516`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

This PT_LOAD changed from accepted R17 and therefore creates a new exact
hardware-debt identity. No physical qualification is inferred.

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R18
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

## Next dependency review

Foreman did not jump directly into final MPEG `app.c` activation.

A004 already contains the accepted generic RFB flow-policy mechanism:
`pstvnc_rfb_flow_policy_t` owns global freeze/thaw, one outstanding live
request, one-shot post-thaw FULL-refresh debt, and remote-publication
permission. Its existing tests prove in-flight completion while frozen, FULL
debt coalescing, and nonmutating HOLD/inspection semantics.

The ordinary Application loop does not yet consume that authority. It still
directly issues an incremental request after startup and after every live
UPDATE, and presents dirty remote frames without consulting the P2 publication
gate.

That bypass is the smallest remaining prerequisite before cross-domain MPEG
activation. If MPEG start/stop/calibration were added now, one packet would have
to change RFB request/publication ownership and MPEG lifecycle simultaneously.

R19 therefore composes the already-accepted P2 policy into the real ordinary
Application loop while introducing no production freeze caller. In the thawed
path, behavior remains one incremental request at a time with ordinary dirty-
frame presentation. A later MPEG transaction can then freeze/thaw the existing
single authority rather than redesign request cadence.

## Foreman-owned publication

Published Foreman State revision 0050 at:

`1dd32c35e9ef528fe2d4b6561d18c6bc4b328e6d`

with message:

`docs(foreman): accept R18 and activate RFB flow composition`

State 0050 activates:

`A004-RFB-FLOW-APPLICATION-COMPOSITION-R19`

R19 is restricted to composing the accepted P2 policy into the ordinary
Application RFB loop: fresh attempt-local policy, policy-selected post-startup
requests, successful-send accounting, UPDATE completion accounting, publication
gating, and fresh state across R16B replacement.

R19 explicitly does not activate MPEG run open/START/RETIRE/finalize, run
identity, MPEG calibration, decoder/worker/backend, MPEG Presentation/compositor
or frame consumer, Pi product behavior, AUDIO, heartbeat, or final all-guns
composition.

## State-0050 machine evidence

Exact State-0050 Actions run `35828216349` attempt 1 completed SUCCESS.

- host-unit — PASS;
- dedicated R16A PS2 RFB wait termination — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS.

The documentation-only State publication retained exact R18 linked identity:

`ELF_PRISTINE_SHA256=4d0bc02f6bc89f138acb53ddb7751fd9e91a300b9e7cbfd9bc7301920f08d1c4`
`PT_LOAD_SHA256=e244769ef21dadd8a09e6fe65ec4b2201e0acd9beb52ae06180ab3d4f3a3a232`
`PT_LOAD_BYTES=490516`

## Next pickup

Reconstruction should recover current branch authority and execute only
`A004-RFB-FLOW-APPLICATION-COMPOSITION-R19` from Foreman State 0050.

Do not begin MPEG Application transaction wiring, calibration trigger/UI,
Transport MPEG lifecycle calls, worker/backend/compositor activation, Pi
changes, AUDIO, heartbeat, or unrelated cleanup.

If direct composition exposes a genuine defect in the accepted P2 public
contract, correct only the minimum authorized RFB flow-policy surface and prove
the defect deterministically. Do not fork freeze/FULL-refresh semantics into
`app.c`.

At shift end emit exactly one immutable Reconstruction work-log record and stop.
