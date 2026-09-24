DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T00:13:00-04:00
COMPLETED_AT=2026-09-24T00:28:31-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=0a81114fb1c8691fb677fed343cd45bfbae24487
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R20 accepted; R21 queued behind A001 receiver-completion correction

## Objective and recovered authority

Receive the Reconstruction baton after `A003-MPEG-PRIVATE-SESSION-BINDING-R20`,
recover current branch authority independently, verify the exact worker range
and machine evidence, decide R20 acceptance, then select the smallest safe next
dependency.

Live pickup authority was:

`0a81114fb1c8691fb677fed343cd45bfbae24487`

with immutable Reconstruction record:

`docs/ledge/work-log/20260923T075029-0400__reconstruction__a003-mpeg-generation__interactive.md`

The final R20 pre-log source authority was:

`3e39753b1b3bce9fe187748deb7eeb4d6201151c`

Exactly eight Reconstruction commits followed assigning Foreman authority
`102010653642af43e7b1e32d585c4d16b280b12f` before the immutable R20 log.
No later commit existed at pickup.

Current repository authority read included AGENTS/CONTRIBUTING/current status,
project intent, clean architecture, development naming/topology/module lifecycle
contracts, Reconstruction/work-log contracts, Foreman State 0051, current Wire
Runtime Decisions/Architecture Overlay/Q1-Q12 authority, A001/A003/A004/A005/
A006 audits, R20 Transport source/tests, and the accepted Application/MPEG/
Presentation/RFB seams relevant to dependency selection.

No external Pi/local checkout was exposed to this Foreman seat. External staged,
unstaged and untracked state remains unasserted; no local resume/check execution
is claimed.

## R20 independent acceptance

R20 remained within its authorized Transport semantic-control/test/docs/
dictionary surface. It introduced owner-correct public semantic request values
while keeping the exact wire representation private below Transport.

Independent Foreman dispositions:

A003-R20-C1 PUBLIC_START_REQUEST_CONTAINS_NO_PRIVATE_WIRE_IDENTITY — MET
A003-R20-C2 TRANSPORT_STAMPS_CURRENT_PRIVATE_SESSION_AND_VERSION_ON_START — MET
A003-R20-C3 PUBLIC_RETIRE_REQUEST_CONTAINS_ONLY_EXACT_RUN_MEANING — MET
A003-R20-C4 COMPLETION_REMAINS_FULLY_CORRELATED_INSIDE_TRANSPORT — MET
A003-R20-C5 PUBLIC_COMPLETION_DOES_NOT_LEAK_PRIVATE_SESSION_OR_VERSION — MET
A003-R20-C6 NO_PUBLIC_Q4_SESSION_ID_ACCESSOR_OR_DUPLICATE_OWNER — MET
A003-R20-C7 START_RETIRE_WIRE_BYTES_AND_PRODUCT_COMPATIBILITY_UNCHANGED — MET
A003-R20-C8 STALE_ACCESS_OR_REPLACEMENT_SESSION_CANNOT_REUSE_OLD_IDENTITY — MET
A003-R20-C9 R18_RUN_BOUNDARY_AND_FINALIZATION_CONTRACTS_UNCHANGED — MET
A003-R20-C10 PI_R17_EXACT_SESSION_GENERATION_BEHAVIOR_UNCHANGED — MET
A003-R20-C11 NO_APPLICATION_CALIBRATION_MEDIA_OR_PI_SCOPE_CREEP — MET
A003-R20-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN — MET

Key verified source behavior:

- public START contains only exact generation plus base/suppression geometry;
- public RETIRE contains only exact generation;
- public completion exposes only exact completed generation;
- the bridge stamps `PSTVNC_MPEG_GENERATION_CONTROL_VERSION` and the current
  private Q4 session ID into the existing wire payloads;
- runtime still correlates the complete RETIRE wire payload exactly;
- bridge completion verifies private version/session authority before projecting
  generation upward;
- stale access from Session A cannot serialize onto replacement Session B;
- no public Wire-session-ID accessor was created;
- START remains 44-byte v1 and RETIRE remains 12-byte v1 with unchanged codec
  layout, frame kinds, channels, flags and product compatibility.

## R20 exact machine/build evidence

Final source run `35857721371` attempt 1 — SUCCESS.
Immutable-log-head run `35858084665` attempt 1 — SUCCESS.

Both exact heads passed:

- host-unit;
- project-check;
- complete strict dictionaries;
- pinned PS2 compile;
- pinned PS2 link/current-source reproducibility.

Host evidence included transport protocol/bridge/runtime/MPEG tests, P2 RFB flow
tests, Pi R17 MPEG generation tests and Application R15/R16B/R19 tests.

Accepted R20 linked identity:

`ELF_PRISTINE_SHA256=43f2c43f537a32f7205ab4b7711f0c53a6ac8bdb049818f5416d24a14fdb63d0`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=9bdb07b04ed9265ac430bc3816e5e9c29cdad3273ff9149c4e786b5605ba771e`
`PT_LOAD_BYTES=491540`
`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R20
HOST_TESTED=PASS_AT_R20_EXACT_HEADS
PROJECT_CHECK=PASS_AT_R20_EXACT_HEADS
STRICT_DICTIONARIES=PASS_AT_R20_EXACT_HEADS
PS2_COMPILE=PASS_AT_R20_EXACT_HEADS
PS2_LINK=PASS_AT_R20_EXACT_HEADS
CURRENT_SOURCE_REPRODUCIBILITY=PASS_AT_R20_EXACT_HEADS
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

R20 changed PT_LOAD from R19 and therefore remains current exact hardware debt.

## Initial next-dependency analysis

After R20 acceptance, Foreman traced the remaining MPEG activation dependencies.
Ordinary activation was not authorized because:

- ordinary `app.c` still opens only the RFB Transport runtime;
- maintained `pi/wire_runtime.py` composes RFB only and does not inject the
  dormant R17 `mpeg_generation_factory`;
- current ordinary Pi authority does not yet select all MPEG producer-composition
  values such as active desktop/display and retirement policy;
- A005 explicitly discards the historical START+SELECT calibration chord as a
  product binding;
- no Application owner yet sequences accepted geometry through exact run
  generation, worker/backend/runtime, Presentation, P7 and START.

Foreman therefore initially published State 0052 at:

`f90d69c62612400cf2c72d8a922e6cef21c13359`

with message:

`docs(foreman): accept R20 and activate MPEG run start`

State 0052 described a trigger-agnostic
`A003-APPLICATION-MPEG-RUN-START-R21` packet and deliberately did not authorize
ordinary `app.c` or Pi product activation.

## State-0052 exact-head validation defect

Exact State-0052 workflow run `35955243662` attempt 1 passed project-check,
strict dictionaries, PS2 compile and PS2 link/reproducibility but failed
`host-unit` only.

The exact failures were:

`unit/transport_runtime_test.c:1273`:
`EVENT_TERMINATE_THREAD` was not after `EVENT_RECEIVER_DONE_SIGNAL`;

`unit/transport_runtime_test.c:1275`:
`EVENT_DELETE_THREAD` was not after `EVENT_RECEIVER_DONE_SIGNAL`.

Foreman reran the failed host job on the same SHA. Attempt 2 reproduced exactly
the same two failures.

Because State 0052 is documentation-only, the executable source was identical
to accepted R20. The repeated failure therefore triggered independent
lifecycle/source/history investigation rather than an assertion that R21 docs
caused a product regression.

## Root cause

The failure is a genuine latent A001 lifecycle race.

The original A001 receiver-release design assumed that `receiver_done` was the
receiver's quiescent point. Under that historical layout a post-done
`TerminateThread()` was intended only to make an already-quiescent owner
DORMANT before dynamic stack reclaim.

The later sole-physical-I/O-owner refactor retained that release assumption but
added substantial mandatory terminal work after `receiver_done = 1`:

- resolve/fail a racing pending outbound submission;
- wake a writer blocked on RFB outbound credit;
- publish terminal RFB activity;
- publish enabled AUDIO terminal activity;
- publish enabled MPEG terminal activity;
- signal the receiver-done semaphore;
- finally call `ExitThread()`.

`pstvnc_transport_runtime_wait_receiver_done()` currently short-circuits as soon
as it observes the early `receiver_done` flag. `release()` likewise accepts that
flag as reclaim authority and may force a still-RUNNING thread dormant. It can
therefore terminate/delete the sole I/O owner while the terminal publication
sequence still has session-owned state/semaphores to touch.

The host fixture uses mutex-protected lifecycle events. Its ordering failure is
not explained by an unsynchronized diagnostic recorder.

This violates A001 authority: teardown may not race the sole receiver/I/O owner
inside its terminal dispatch/signal path.

## Foreman correction of active authority

R21 cannot safely depend on Transport pre-START unwind while this race exists.
Foreman therefore superseded State 0052 with State 0053:

`27be6d8f5cf753617912a8e4c4284d720534d919`

message:

`docs(foreman): block R21 on receiver completion fence`

State 0053:

- preserves R20 Foreman acceptance;
- sets `ARCHITECTURE_BLOCKER=TRANSPORT_RECEIVER_COMPLETION_FENCE`;
- queues `A003-APPLICATION-MPEG-RUN-START-R21` without authorizing execution;
- activates only `A001-TRANSPORT-RECEIVER-COMPLETION-FENCE-R20C`.

R20C requires a truthful completion rendezvous after pending-outbound and
logical-owner terminal publication; release must not terminate/delete/reclaim
before that fence; failed shutdown convergence and retryable ownership must
remain intact; and deterministic barrier tests must prove the race closed.

Foreman made no Transport product-source correction itself.

## State-0053 evidence

Exact State-0053 workflow run `35955721354` attempt 1 completed SUCCESS across:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility.

This green scheduling instance does **not** erase the State-0052 attempts or the
source-level race. R20C acceptance requires deterministic synchronization proof
that makes both orderings impossible rather than reliance on a favorable run.

State 0053 retained the exact R20 linked identity:

`ELF_PRISTINE_SHA256=43f2c43f537a32f7205ab4b7711f0c53a6ac8bdb049818f5416d24a14fdb63d0`
`PT_LOAD_SHA256=9bdb07b04ed9265ac430bc3816e5e9c29cdad3273ff9149c4e786b5605ba771e`
`PT_LOAD_BYTES=491540`

## Next pickup

Reconstruction must recover current authority and execute only:

`A001-TRANSPORT-RECEIVER-COMPLETION-FENCE-R20C`

from Foreman State 0053.

Do not execute queued R21, do not add Application MPEG run-start source, do not
activate Pi MPEG runtime, and do not broaden into RFB/MPEG/Presentation/AUDIO/
calibration/protocol behavior.

The corrective source must distinguish early terminal/admission-closing state
from reclaim-authoritative completion, preserve the sole I/O owner through all
terminal wake/publication work, and prove the boundary with deterministic host
synchronization rather than sleeps or rerun luck.

At shift end emit exactly one immutable Reconstruction work-log record and stop.
