DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T12:21:00-04:00
COMPLETED_AT=2026-09-24T12:29:56-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=f15ff5bc4969cb867088b9567667865ef6849685
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman review — R20D not accepted; R20E fail-closed drain proof activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A001-TRANSPORT-OUTBOUND-SUBMITTER-DRAIN-R20D`, recover live repository
authority independently, inspect the exact returned source/test range and
machine evidence, decide all packet criteria, then either clear the A001
Transport blocker and reactivate queued R21 or publish the smallest remaining
corrective packet.

Live pickup authority was the immutable R20D Reconstruction record:

`f15ff5bc4969cb867088b9567667865ef6849685`

with final pre-log source:

`c719a46003e04ac625cb92f4b455c94fe7281020`

and assigning Foreman authority:

`3032424be8296b5329a6876e947ff87653ea2e20`.

Exactly eight pre-log Reconstruction commits followed the assigning authority;
the immutable log was the ninth commit and no later commit existed at pickup.

Current authority read included AGENTS/CONTRIBUTING/current status/project
intent/clean architecture, development naming/topology/module-lifecycle rules,
Reconstruction/work-log contracts, Foreman State 0054, Architecture Overlay,
Wire Runtime Decisions, Q1-Q12 reconciliation, A001/A003/A004/A006 audits,
R20D runtime/test source, the returned immutable log, and queued R21 authority.

No external Pi/local checkout was exposed to this Foreman seat. External staged,
unstaged and untracked state remains unasserted; no local
`scripts/resume-state.sh` or `scripts/check.sh` execution is claimed.

## What R20D correctly repaired

Foreman confirms R20D closes the normal outbound-submitter lifetime race that
caused R20C rejection.

Independent source review established:

- submitter registration occurs before any outbound slot/ready/done semaphore
  touch;
- registration and early receiver terminality share one short nonblocking
  critical section, so a caller is either in the pre-terminal drain set or
  rejected before outbound rendezvous ownership;
- a registered active submitter remains counted through outbound-done result
  consumption and its final outbound-slot release;
- a caller queued behind the occupied slot is counted while sleeping and, after
  terminality, acquires the slot only far enough to observe terminal state,
  release it and unregister;
- the last registered submitter signals one private drain semaphore only after
  its final outbound semaphore touch;
- receiver terminal ordering remains R20C pending-outbound resolution, RFB
  credit-owner wake, RFB terminal activity, AUDIO/MPEG terminal activity, then
  R20D submitter drain;
- normal final receiver completion is withheld while either the active or
  queued submitter remains live;
- `release()` therefore cannot delete outbound rendezvous resources during the
  deterministic normal active/queued race;
- callers starting after early terminality are rejected before registration or
  any outbound semaphore touch;
- fresh runtime initialization clears submitter count, drain-wait state and
  drain semaphore token.

The deterministic host fixture directly covers the exact R20C gap by holding
the active caller after outbound-done wake but before its final slot release,
with a second caller already queued behind that slot. It proves final receiver
completion and release/reclaim remain blocked until both callers leave.

## Remaining fail-closed defect

R20D's normal synchronization is correct, but its drain **failure path** still
violates the reclaim contract.

`pstvnc_transport_runtime_wait_outbound_submitters_drained()` returns failure
if either:

- `WaitSema(outbound_submitter_drain_semaphore_id)` returns an error; or
- after a drain wake the protected `outbound_submitter_count` is still nonzero.

Either condition means the runtime has not proven that pre-terminal submitters
can no longer touch outbound work/semaphores.

The receiver terminal path currently handles that return only by setting:

`runtime->failed = 1`

and then still proceeds to signal:

`receiver_done_semaphore_id`.

That token is the current reclaim-completion authority consumed by
`pstvnc_transport_runtime_wait_receiver_done()` and
`pstvnc_transport_runtime_release()`. Publishing it after the drain helper
explicitly failed can therefore convert unproven ownership into successful
reclaim authority.

This contradicts State-0054 governing invariant 2 and current
`docs/development/module-lifecycle.md`: receiver completion is published only
after every outbound submitter admitted before terminality has finished its
final outbound rendezvous touch. The general A006 shutdown authority likewise
requires unproven ownership to fail closed rather than become reusable session
state.

## R20D criterion disposition

A001-R20D-C1 PRETERMINAL_SUBMITTER_LIFETIME_IS_EXPLICITLY_DRAINED — MET_ON_NORMAL_PATH
A001-R20D-C2 TERMINALITY_PREVENTS_NEW_OUTBOUND_RENDEZVOUS_ADMISSION — MET
A001-R20D-C3 OUTBOUND_DONE_SIGNAL_IS_NOT_MISTAKEN_FOR_SUBMITTER_COMPLETION — MET
A001-R20D-C4 ACTIVE_SUBMITTER_EXITS_BEFORE_RECEIVER_COMPLETION — MET_ON_NORMAL_PATH
A001-R20D-C5 QUEUED_OUTBOUND_SLOT_WAITERS_CANNOT_OUTLIVE_COMPLETION — MET_ON_NORMAL_PATH
A001-R20D-C6 RELEASE_CANNOT_DELETE_OUTBOUND_SEMAPHORES_BEFORE_DRAIN — NOT_MET_ON_DRAIN_FAILURE
A001-R20D-C7 R20C_RECEIVER_NO_TOUCH_AND_LOGICAL_TERMINAL_ORDERING_PRESERVED — MET
A001-R20D-C8 OUTBOUND_SERIALIZATION_BACKPRESSURE_AND_SOLE_SENDER_UNCHANGED — MET
A001-R20D-C9 FAILED_SHUTDOWN_RETRY_AND_FRESH_RUNTIME_REUSE_REMAIN_SAFE — NOT_PROVEN_FOR_DRAIN_FAILURE
A001-R20D-C10 DETERMINISTIC_BARRIERS_PROVE_ACTIVE_AND_QUEUED_RACES_CLOSED — MET_FOR_NORMAL_RACE_NOT_DRAIN_FAILURE
A001-R20D-C11 NO_APPLICATION_PI_MEDIA_PRESENTATION_OR_PROTOCOL_SCOPE_CREEP — MET
A001-R20D-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

All twelve criteria were required. R20D is therefore not Foreman-accepted.

## R20D machine/build evidence retained without semantic promotion

Final R20D source authority `c719a46003e04ac625cb92f4b455c94fe7281020`
was exercised by workflow run `36025000101`. Attempt 1 was already completely
green; attempts 2 and 3 repeated the same source without source/delay/timeout
changes. Final observed attempt 3 completed SUCCESS.

Immutable R20D log head `f15ff5bc4969cb867088b9567667865ef6849685`
workflow run `36025494324` attempt 1 also completed SUCCESS.

Exact jobs on both final authorities include:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS.

R20D candidate linked identity:

`ELF_PRISTINE_SHA256=58d00a302171a8d1e37f1bb911a443912f00d3a5a04f68a3737f31f5124106b1`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=df81e351eb67b77560a124279e7cb664843462f89581bb57aa7fc9993125d8d9`
`PT_LOAD_BYTES=492052`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

Evidence classification:

R20D_SOURCE_COMPLETE_WORKER_CANDIDATE=YES
R20D_FOREMAN_ACCEPTED=NO
HOST_TESTED=PASS_BUT_DRAIN_FAILURE_PATH_NOT_PROVEN_FAIL_CLOSED
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

The last Foreman-accepted product remains R20. R20D is current branch candidate
hardware debt only.

## Corrective dependency decision

R21 remains queued. Its trigger-agnostic MPEG Application semantics are not
changed by this review, but R21 still depends on trustworthy Transport unwind.

Foreman preserves the valid R20C receiver fence and R20D normal active/queued
submitter drain. The next packet changes only the failure meaning at the final
drain proof:

`A001-TRANSPORT-DRAIN-FAIL-CLOSED-R20E`

R20E must ensure a failed drain wait or contradictory post-wake count can never
be surfaced as successful receiver completion or authorize release/reclaim.

A small Transport-private outcome fact is permitted if necessary to wake
completion waiters with failure rather than hanging forever, but that fact may
not become a public lifecycle owner or substitute for actual drain proof.

## Foreman-owned publication

State revision 0055 was first published at:

`a7357683b1ef6d1f02fe576681553f1c3b8d61d6`

with message:

`docs(foreman): require fail-closed drain proof before R21`

Foreman then detected that the initial `RECORDED_AT` value was ahead of actual
publication time and immediately corrected that metadata without changing the
engineering decision. Final State-0055 authority is:

`22698411407f594c6023e36de09f0a15a6aa637f`

with message:

`docs(foreman): correct State 0055 recorded time`

Final State 0055:

- does not accept R20D;
- retains the sound R20C/R20D normal synchronization as candidate source;
- narrows `ARCHITECTURE_BLOCKER` to
  `TRANSPORT_DRAIN_FAILURE_MUST_NOT_AUTHORIZE_RECLAIM`;
- leaves `A003-APPLICATION-MPEG-RUN-START-R21` queued;
- activates only `A001-TRANSPORT-DRAIN-FAIL-CLOSED-R20E`;
- authorizes only minimal Transport runtime/test/docs/dictionary correction.

## State-0055 exact-head machine evidence

Exact corrected State-0055 workflow run `36027567013` attempt 1 completed
SUCCESS across:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

The documentation-only State publication retained exact R20D candidate identity:

`ELF_PRISTINE_SHA256=58d00a302171a8d1e37f1bb911a443912f00d3a5a04f68a3737f31f5124106b1`
`PT_LOAD_SHA256=df81e351eb67b77560a124279e7cb664843462f89581bb57aa7fc9993125d8d9`
`PT_LOAD_BYTES=492052`

That green State run does not override the source-level fail-closed finding.

## Next pickup

Reconstruction must recover current branch authority and execute only:

`A001-TRANSPORT-DRAIN-FAIL-CLOSED-R20E`

from Foreman State revision 0055.

Preserve R20D's normal registration/count/drain design. Correct only the path
where drain proof fails or contradicts count state so successful receiver
completion/reclaim is impossible without actual drain proof.

Deterministically inject both drain-wait failure and a contradictory wake with
a nonzero submitter count. Prove completion wait/release cannot report success
or reclaim resources on either failure.

Do not execute queued R21 and do not broaden into Application, Pi, RFB policy,
AUDIO policy, MPEG worker/backend, Presentation/calibration, Configuration or
protocol changes.

At shift end emit exactly one immutable Reconstruction work-log record and stop.
