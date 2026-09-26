DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T19:44:06-04:00
COMPLETED_AT=2026-09-25T19:59:56-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=6410a5de2a6a17c94cd5d5c8011b7509dcdebdd7
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R33 Transport/MPEG enclosing-session abort fence

## Recovery provenance

This Reconstruction shift independently recovered live branch authority before
performing any R33 behavior-bearing write.

Pickup authority:

6410a5de2a6a17c94cd5d5c8011b7509dcdebdd7

Current Foreman State was revision 0072 and activated exactly:

A003-TRANSPORT-MPEG-SESSION-ABORT-FENCE-R33

with Reconstruction ownership under:

WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive

The newest Foreman handoff consumed was:

docs/ledge/work-log/20260925T193352-0400__foreman__a003-mpeg-generation__interactive.md

The packet explicitly deferred ordinary app.c wiring, R32 binding installation,
PRODUCT_ACTION routing, P9/P10 product activation, Pi work, persistence/editor
work, AUDIO changes and hardware qualification.

The GitHub-connected Worker seat cannot observe an external local/Pi checkout's
untracked files, staging state or dirtiness. No local reset, clean, stash or
overwrite is claimed.

## Governing authority consumed

This shift read and followed at minimum:

- AGENTS.md;
- CONTRIBUTING.md;
- docs/CLEAN_ARCHITECTURE.md;
- current Reconstruction Contract;
- Work Log Contract;
- docs/development/module-lifecycle.md;
- A003 MPEG-generation audit authority;
- Foreman State revision 0072;
- current Transport bridge/runtime contracts;
- accepted MPEG worker and PS2 worker-runtime contracts;
- accepted P7 app_mpeg_frame claim ownership;
- accepted R21-R24 app_mpeg_run start/live/retirement/reveal authority.

The lower Transport runtime already possessed the required mechanism:

- request_stop publishes terminal logical-rider wakeups and shuts down physical
  I/O;
- receiver completion proves receiver/outbound submitter dormancy;
- runtime release refuses reclamation while protected media waiters still own
  rendezvous state.

R33 therefore did not modify Transport runtime, MPEG worker, PS2 worker runtime,
Display presentation or media-clock implementations.

## Objective executed

R33 reconstructs the missing ownership fence for an active asynchronous MPEG
worker when the enclosing Wire Session dies.

The abnormal sequence is now:

1. Transport begin-abort closes/wakes the dying session and proves receiver
   completion without reclaiming runtime storage.
2. The exact old access ticket remains terminal and may only prove that its old
   terminal storage is retained.
3. Replacement-session admission remains blocked while that runtime is retained.
4. Application locally abandons any exact outstanding P7 frame claim without
   presenting it.
5. Application requests the accepted exact-generation worker safe stop.
6. Transport terminal wakeups allow an old MPEG feed/activity wait to return
   terminally; no EOF is synthesized.
7. Application waits for true worker finish and an empty slot.
8. Application joins the worker, reads and preserves its exact terminal outcome,
   then releases worker and PS2 worker-runtime resources.
9. Only after those local resources are proven dormant does the run reach
   SESSION_ABORT_READY.
10. A later outer owner may then perform final Transport close/release.

R33 deliberately does not yet wire that sequence into ordinary app.c.

## Exact R33 pre-log source authority

Final pre-log source authority:

c48d460aa3d2e095d079289d07321f96151113a9

From assigning authority
6410a5de2a6a17c94cd5d5c8011b7509dcdebdd7
to final pre-log authority, GitHub compare reports seventeen commits ahead /
zero behind.

Committed sequence:

1. 754cb19d9576f79c01691269f9574a7f85218c68
   transport: declare two-phase session abort fence
2. d96204f26484116f274b01b9d39c81597ab8580c
   transport: split abort terminality from storage release
3. 60b9337370c81e263d71b26fc0ddc37e521c37b1
   app(mpeg): declare exact claim abandonment seam
4. 21f6e799135c217d692b6f81cd32a112c77ab509
   app(mpeg): abandon claimed frame for session teardown
5. da1a386ab617c7a6a9aa88b79abd1633872d1e73
   app(mpeg): define terminal local session-abort lifecycle
6. fd0decdad3bc1bf796ea7f2ec897860b2e62dc6e
   app(mpeg): add local enclosing-session abort service
7. 151479652a25c53d2b348e8f3089440777cf33cf
   test(transport): prove retained-runtime abort fence
8. 4fd5f9a34214606656198207fdb6cee2741e7e4c
   test(transport): prove live MPEG waiter blocks release
9. 9290b15e71356d78377e8617f05201044df67306
   test(app): prove abort claim abandonment fence
10. 06cbd5c76d148a11bac1ca52505f3700430398e5
    test(app): script R33 local-abort owner seams
11. da175cccf2b6d567f6f014d1f38a290eb04d97a0
    test(app): prove R33 local-abort ordering and retry
12. a58e0ada7d60fa380a5d26559aeef75c2f27dace
    test(app): enforce R33 session-abort boundaries
13. f841b65f5370bc54b191f1cea1cec7a0f92a4da4
    test(app): enroll R33 session-abort source proof
14. c7d5dd1caa96c69a3ea76bc47d3cfa4a82f30d55
    docs(lifecycle): record R33 retained-runtime abort fence
15. c6c463e9c0f88bf0fbc28f553a649e9a7c8402a8
    tooling(symbols): run deterministic dictionary reconciliation
16. 76cc1b6bb355ffe0c0514d872b6ce948daf1f773
    docs(symbols): reconcile current clean definitions
17. c48d460aa3d2e095d079289d07321f96151113a9
    app(mpeg): require complete live owners before abort

The final changed-path set is confined to:

- docs/development/module-lifecycle.md;
- generated docs/reference/SOURCE_SYMBOL_DICTIONARIES.md;
- generated src/SYMBOLS.md;
- src/app_mpeg_frame.c/.h;
- src/app_mpeg_run.c/.h;
- generated src/transport/SYMBOLS.md;
- src/transport/bridge.c/.h;
- tests/Makefile;
- focused app_mpeg_frame, app_mpeg_run, Transport bridge/MPEG tests;
- new tests/unit/app_mpeg_session_abort_source_test.py.

No ordinary src/app.c/.h, app_product_bindings, Input, UI, Configuration,
Management, RFB, AUDIO, Pi product, Transport runtime, MPEG worker,
PS2 worker-runtime, Display presentation, media-clock, persistence/editor or
mailbox source changed.

## Two-phase Transport abort

R33 adds:

pstvnc_transport_session_begin_abort()

For an active Transport runtime this operation:

- invokes the accepted runtime request-stop mechanism;
- therefore publishes terminal logical-rider wakeups and shuts down physical
  I/O;
- waits for accepted receiver/outbound completion proof;
- deliberately does not call runtime release;
- leaves the runtime, queues, semaphores and active ticket storage retained.

R33 also adds:

pstvnc_transport_session_abort_storage_retained(access)

This is a narrow exact-ticket proof. It returns OK only when:

- a runtime is still active/owned;
- the supplied nonzero ticket is exactly the active old ticket;
- receiver completion is visible;
- receiver completion outcome is PROVEN.

A stale/released ticket is CLOSED, an incomplete fence is WOULD_BLOCK, and an
unproven terminal outcome fails closed.

Because runtime_active remains owned after begin-abort:

- Wire availability is INACTIVE;
- old ordinary access operations are terminal;
- new access acquisition cannot succeed;
- a replacement session cannot open.

The historical one-shot:

pstvnc_transport_session_abort()

retains compatibility. For rider runtimes it composes begin-abort followed by
session close; establishment-only Wire authority still has no rider storage to
retain and may retire directly.

## No early runtime reclamation

R33 does not weaken runtime release.

Focused real-runtime evidence creates the audited MPEG waiter protected state in
which a wake token has been published but the waiter still owns its rendezvous.
runtime_release() returns failure while that state is live and preserves:

- initialized runtime authority;
- MPEG queue semaphore;
- MPEG activity semaphore;
- waiter protected state.

After the protected waiter state is cleared, the same terminal runtime can be
released successfully.

The Transport bridge fixture separately proves a final-close release failure
keeps retained runtime authority and blocks replacement admission until a later
successful release.

## Exact P7 frame-claim abandonment

R33 adds:

pstvnc_app_mpeg_frame_consumer_abandon_claim()

The seam is generation-exact and terminalizes the P7 consumer.

If no claim exists, terminalization succeeds without fabricating a worker
release.

If a claim exists, it calls the accepted worker release-frame operation with the
exact held claim token. Successful release clears the held claim. A second call
cannot release the same claim twice.

The seam performs no compositor, scheduler or presentation operation.

If release fails, the exact claim remains represented and the caller cannot
advance teardown.

## Application local session-abort lifecycle

R33 extends app_mpeg_run with explicit terminal states:

PSTVNC_APP_MPEG_RUN_SESSION_ABORTING
PSTVNC_APP_MPEG_RUN_SESSION_ABORT_READY

and one trigger-agnostic service:

pstvnc_app_mpeg_run_session_abort_service(run)

Entry requires the exact old Transport ticket to prove terminal retained
storage and requires a complete live post-START owner set:

- current nonzero generation;
- open Transport MPEG run evidence;
- owned PS2 worker runtime;
- live worker;
- initialized exact-generation P7 consumer bound to that worker;
- armed presentation authority;
- successful START evidence.

Missing/contradictory ownership cannot be interpreted as already-safe dormancy.

The service then orders:

1. exact retained Transport proof;
2. P7 claim abandonment;
3. safe worker stop request exactly once;
4. worker status/true-finish proof;
5. exact worker join;
6. exact terminal worker-outcome read and preservation;
7. worker release;
8. PS2 worker-runtime release;
9. SESSION_ABORT_READY.

A still-running worker is a successful pending service step in
SESSION_ABORTING; no local resource is reclaimed before synchronized finish.

Failure at claim release, safe-stop, status, join, outcome, worker release or
PS2 runtime release prevents SESSION_ABORT_READY. Retry preserves the exact
already-proven evidence and does not issue another generation or restart work.

A genuine FAILED worker outcome is retained exactly as failure evidence. It is
not converted into COMPLETED retirement.

## Abnormal abort is not normal R23/R24 retirement

The R33 abort service does not call:

- pstvnc_transport_mpeg_send_retire;
- pstvnc_transport_mpeg_take_retire_completion;
- pstvnc_transport_mpeg_mark_producer_done;
- pstvnc_transport_mpeg_run_finalize;
- P2 thaw/set-frozen;
- P3 begin-retirement;
- P3 retirement seal;
- compositor reveal;
- normal P7 live frame service.

Therefore enclosing-session loss cannot masquerade as successful same-session
MPEG retirement or final RFB reveal.

P2/P3 values remain old-session evidence while local asynchronous resources are
drained. Final Transport destruction is intentionally left to the later outer
Application composition packet.

SESSION_ABORT_READY is terminal for that old run object. Calling ordinary
run-start on it is rejected rather than allocating another generation.

## Focused deterministic evidence

Transport bridge tests prove:

- begin-abort orders STOP then receiver WAIT without RELEASE;
- old exact storage proof succeeds only after terminal completion;
- Wire becomes inactive;
- stale old operations are terminal;
- fresh access and replacement session admission are blocked;
- final close releases retained storage;
- failed final release preserves runtime storage and replacement exclusion;
- existing one-shot abort still orders STOP -> WAIT -> RELEASE.

Transport MPEG real-runtime tests prove:

- a live protected MPEG waiter prevents release from deleting runtime storage;
- the exact semaphore/storage identity remains present after rejected release;
- release succeeds only after waiter ownership clears.

P7 tests prove:

- a held frame is abandoned once without compositor presentation;
- repeated no-claim abandonment does not release twice;
- release failure retains the exact outstanding claim.

app_mpeg_run tests prove:

- local abort rejects before retained terminal Transport proof;
- WAIT_FIRST_FRAME abort may remain pending while worker is still live;
- claim abandonment precedes worker stop and join;
- STOPPED and FAILED terminal outcomes remain exact evidence;
- worker release and PS2 runtime release failure block abort-ready and can be
  retried without repeating stop/join/outcome;
- no RETIRE, producer-done, finalize, P2/P3 retirement/reveal or compositor
  reveal occurs;
- abort-ready cannot restart the same run/session.

The source-boundary test independently verifies the same owner ordering and
forbids normal-retirement calls in the R33 abort function. It also proves that
ordinary app.c contains no R33 invocation.

## R33 Worker criterion dispositions

These are Reconstruction Worker dispositions only and are not Foreman
acceptance.

A003-R33-C1 SPLIT_TERMINAL_QUIESCE_FROM_STORAGE_RELEASE = MET
A003-R33-C2 OLD_ACCESS_TERMINAL_AND_REPLACEMENT_BLOCKED_WHILE_RETAINED = MET
A003-R33-C3 LEGACY_ONE_SHOT_ABORT_COMPATIBILITY_PRESERVED = MET
A003-R33-C4 LIVE_MPEG_WAITER_PREVENTS_STORAGE_RECLAMATION = MET
A003-R33-C5 APPLICATION_LOCAL_ABORT_LIFECYCLE_PRESENT = MET
A003-R33-C6 P7_CLAIM_DISCARDED_EXACTLY_ONCE_BEFORE_JOIN = MET
A003-R33-C7 SAFE_STOP_AND_TRANSPORT_TERMINAL_WAKE_USED = MET
A003-R33-C8 JOIN_OUTCOME_WORKER_RUNTIME_RELEASE_PROVE_LOCAL_DORMANCY = MET
A003-R33-C9 ABNORMAL_ABORT_DOES_NOT_MASQUERADE_AS_NORMAL_RETIREMENT = MET
A003-R33-C10 ABORT_READY_IS_TERMINAL_AND_NON_RESTARTABLE = MET
A003-R33-C11 NO_DEFERRED_SCOPE_CREEP = MET
A003-R33-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN = MET

All twelve R33 criteria are MET as Worker dispositions.

## Final canonical machine evidence

Final source authority:

c48d460aa3d2e095d079289d07321f96151113a9

Canonical workflow:

Ledge reconstruction checks
run 36203058779
attempt 1
conclusion success

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected on the final non-trigger commit.

Observed focused/cross-domain host output includes:

- APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS;
- APP_MPEG_FRAME_TEST=PASS;
- app_mpeg_run_test: PASS;
- transport bridge tests passed;
- transport_mpeg_test: PASS;
- MPEG_WORKER_TEST=PASS;
- APP_MPEG_CALIBRATION_TEST=PASS;
- APP_MPEG_ACTIVATION_TEST=PASS;
- app R15/R16B/R19/R27/R32 tests: PASS;
- existing product-action/config/management/input tests PASS.

Observed repository/build output includes:

- SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS;
- SOURCE_TOPOLOGY_CONTRACT=PASS;
- SOURCE_DICTIONARY_PORTAL_SYNC=PASS;
- CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS;
- DEVELOPMENT_CONTINUITY_CHECK=PASS;
- WORK_LOG_CHECK=PASS records=228 grandfathered=9 format_compat=2 stamp_compat=1;
- SOURCE_DICTIONARIES=PASS;
- PS_TO_VNC_PROJECT_CHECK=PASS;
- pinned PS2 compile explicitly compiled app_mpeg_frame.c,
  app_mpeg_run.c, mpeg/worker.c and transport/bridge.c;
- CLEAN_PS2_COMPILE_CHECK=PASS;
- ISSUE7_LINKED_BUILD=PASS;
- LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS.

Generated dictionaries now report:

src:806 complete symbols
src/transport:1416 complete symbols
SOURCE_DICTIONARY_TOTAL=6702

## Exact linked PS2 identity / hardware classification

R33 exact linked identity:

ELF_PRISTINE_SHA256=95a3828c42b7ec6c77aab87f6048500d9e77896874fea351714b053fd9b523ea
PT_LOAD_SEGMENTS=1
PT_LOAD_SHA256=c07ef5062c8d467d2075dfaa3162681924bfd1a344bd2a4d56c45516f3493812
PT_LOAD_BYTES=522388

Both canonical builds produced the same ELF and PT_LOAD fingerprints.

R33 changes linked PS2 bytes relative to accepted R32, so the new exact identity
is hardware-pending.

R33_SOURCE_COMPLETE=YES
R33_HOST_TESTED=PASS
R33_PROJECT_CHECK=PASS
R33_STRICT_DICTIONARIES=PASS
R33_PS2_COMPILE=PASS
R33_PS2_LINK=PASS
R33_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R33_PS2_PT_LOAD_CHANGED=YES
R33_MACHINE_EVIDENCE=GITHUB_ACTIONS
R33_INDEPENDENT_VALIDATION=NOT_RUN
R33_OPERATOR_OBSERVED=NO
R33_HARDWARE_QUALIFIED=NO
R33_HARDWARE_PENDING=YES
R33_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

No hardware qualification was attempted or claimed.

## State / contract accounting

Consumed:

- Foreman State revision 0072;
- current Reconstruction and Work Log contracts;
- accepted R20C/R20D/R20E Transport terminal/reclaim fences;
- accepted R18 Transport MPEG run boundary;
- accepted MPEG worker/PS2 worker-runtime safe stop/join/release;
- accepted P7 frame-claim ownership;
- accepted R21-R24 Application MPEG run lifecycle.

Produced:

- two-phase Transport begin-abort/final-close fence;
- exact retained-old-ticket proof;
- one-shot abort compatibility;
- explicit P7 abnormal claim-abandonment seam;
- Application SESSION_ABORTING -> SESSION_ABORT_READY local lifecycle;
- exact worker terminal-outcome preservation;
- deterministic proof that live media ownership blocks reclaim;
- lifecycle documentation and dictionary reconciliation;
- no Foreman State update;
- no Foreman acceptance;
- no ordinary app.c composition;
- no Input binding installation or PRODUCT_ACTION routing;
- no normal MPEG retirement/reveal substitution;
- no Pi/persistence/editor/mailbox work;
- no hardware qualification.

## Next pickup

NEXT_PICKUP=FOREMAN_INDEPENDENT_R33_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION

The Reconstruction Worker stops here. The Foreman must independently recover
live repository authority, inspect R33 source/evidence/this immutable record,
decide acceptance, and select any later bounded packet. Reconstruction must not
self-accept R33 or begin ordinary Application wiring, binding installation,
PRODUCT_ACTION routing, Pi/persistence work or hardware qualification.
