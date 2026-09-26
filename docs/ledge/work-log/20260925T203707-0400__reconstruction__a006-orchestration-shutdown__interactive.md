DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T20:37:07-04:00
COMPLETED_AT=2026-09-25T20:58:45-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=BLOCKED
STARTING_BRANCH_COMMIT=39eef5de7061c43e76ad1a19d1d12f944b949106
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R34 ordinary MPEG action activation BLOCKED on pre-START dormancy contract

## Recovery provenance

This Reconstruction shift independently recovered live
`ledge/h1-all-guns` authority before making behavior-bearing writes.

Pickup authority was exactly:

`39eef5de7061c43e76ad1a19d1d12f944b949106`

message:

`docs(work-log): accept R33 and hand off R34 activation`.

Current Foreman State revision `0073` activated exactly:

`A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a006-orchestration-shutdown`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`.

The packet explicitly requires ordinary Application composition of the accepted
R32 desired binding snapshot, R29 semantic Input runtime, P9 calibration, P10
protected activation, R21/R22 live MPEG service, and R33 enclosing-session
teardown. It also explicitly requires a BLOCKED return rather than bypassing an
ownership fence if accepted lower-owner contracts cannot safely support the
composition.

The GitHub-connected Worker seat cannot observe an external Pi/local checkout's
untracked files, staging area or dirtiness. No local reset, clean, stash or
overwrite is claimed.

## Governing authority consumed

This shift read and followed, at minimum:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- current Reconstruction Contract;
- Work Log Contract revision `0007`;
- Foreman State revision `0073`;
- current source-topology/module-lifecycle authority;
- accepted R29 semantic action publication;
- accepted R30 human-readable typed binding model;
- accepted R31 read-only configuration retrieval;
- accepted R32 resident desired-binding snapshot;
- accepted P9 calibration foreground owner;
- accepted P10 protected P9 -> R21 handoff;
- accepted R21/R22 MPEG start/live service;
- accepted R33 two-phase Transport abort and post-START local MPEG dormancy;
- accepted R16B provider replacement and R26 session media-clock lifecycle.

## Intended R34 composition implemented before blocker discovery

The partial R34 source range reconstructs the ordinary Application composition
that can be proven safe with existing accepted seams:

1. installs the exact R32 `bindings` pointer/count into each fresh R29 Input
   runtime after Input initialization and before worker start;
2. publishes live DESKTOP eligibility from actual Application ownership rather
   than from a second gesture detector;
3. creates fresh per-attempt Transport access, P3 presentation, P9 calibration
   and R21 run owners;
4. routes only semantic
   `PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION` from the Input event FIFO;
5. gives active P9 calibration first refusal over controller facts before local
   desktop/OSK routing;
6. hands accepted protected calibration to P10 once;
7. services WAIT_FIRST_FRAME / MPEG_OWNED through R22 using the exact current
   session media-clock tick at the existing nonblocking Application cadence;
8. suppresses overlapping semantic activation while calibration or a live run
   owns the lifecycle;
9. leaves P2 as the sole RFB freeze/thaw owner;
10. composes the accepted post-START abnormal teardown order as
    Input shutdown -> Transport begin-abort -> R33 local dormancy ->
    media-clock binding release -> final Transport close;
11. retains the accepted R16B one-shot Transport abort for attempts that never
    acquired a live MPEG asynchronous owner.

No physical chord/mask/settle/hold logic entered Application. No normal
R23/R24 retirement/reveal, AUDIO, Pi, persistence/editor/reload, default binding,
mailbox or timeout-as-success mechanism was introduced.

## New narrow Application coordinator

This shift added:

- `src/app_mpeg_product.c`;
- `src/app_mpeg_product.h`.

The coordinator owns only semantic cross-domain composition.

It contains fresh attempt-local:

- P3 presentation state;
- P9 calibration state;
- R21 run state;
- exact session Transport access;
- session media-clock reference.

Its public processes cover:

- initialization;
- truthful DESKTOP action eligibility;
- semantic MPEG-calibration action routing;
- P9 controller first refusal and P10 handoff;
- R22 live service;
- proof that a complete post-START run owner exists;
- R33 local-abort service forwarding.

The coordinator contains no physical product-action button tokens/masks or
gesture timing policy.

## Partial commit range

From assigning authority
`39eef5de7061c43e76ad1a19d1d12f944b949106`
to the final pre-log partial source authority
`6f1acf56d0007a05c081c0217f099a5fcbc4a754`,
GitHub compare reports twenty-one commits ahead / zero behind:

1. `9a17aa54274a9356b8ef24db6db0ec2cd7684592` —
   `app(mpeg): define R34 ordinary product coordinator`;
2. `4a87181d8ae32cb4d0cc03361d15a0e91c79c5da` —
   `app(mpeg): compose semantic calibration and live service`;
3. `131c1544972f75e42e33ed0fef6690ecdc216fe1` —
   initial `app: compose R34 ordinary MPEG activation lifecycle`;
4. `eb9ade6511f241afe57b70cd3acc9a5860507ded` —
   `app: correct R34 composition on clean coordinator authority`;
5. `45bd7c0af0197eb3d7137faa371b85095d8a7997` —
   `app: preserve non-MPEG input failure classification`;
6. `3b76ec441ee060f593476adf3a03d8947a6a53f2` —
   `test(app): prove R34 semantic MPEG product coordinator`;
7. `567f4aba460eaf2ae66dc6b6591f1451ca253cfc` —
   `test(app): enforce R34 activation boundaries`;
8. `a808e623dc00dce95589edac26a3475abc0dc8f8` —
   `build: link R34 ordinary MPEG product coordinator`;
9. `4f71d9cbb73da1f8fd6d6fd0b5e2a067e9349637` —
   `build: PS2-compile R34 product coordinator`;
10. `4509221f7f370dcc66990081289dc978791045e8` —
    `test(app): enroll R34 product evidence`;
11. `2e0c3ce6132892414c7646dcd6fc6448e5ce25f5` —
    `test(app): prove R34 binding and session teardown composition`;
12. `5d03cab9c6fa008f0b93d106e2c3dcd7d1346581` —
    `test(app): bound R34 live failure replacement fixture`;
13. `152375d9adef471a5a38ab078cca2ef61a43e861` —
    `test(app): reuse host libpad stub for R34 coordinator`;
14. `beebc00e0f104668f75fe818870609620f0b5804` —
    `build(app): declare R34 memory initialization dependency`;
15. `3ac273daccca9c907d1354a9e7bc8cf53be5f2ff` —
    `tooling(topology): enroll R34 Application coordinator`;
16. `43d162aa87168b36c865132c575cc62f64ae6292` —
    `docs(app): record R34 product composition topology`;
17. `d5a3ec5b2b94ccaf4f68705d9d2866ac30c9efc8` —
    `docs(app): explain R34 semantic product ownership`;
18. `cf74dfb1832c5bd57def23b2614e60927dc20923` —
    `test(app): scope R32 ownership proof to acquisition helper`;
19. `15ba4dcd3d66dc332f90e402164a3ef743ebbaa1` —
    `test(app): preserve R33 mechanism boundary under R34 wiring`;
20. `04a523f0e3f1305fc6d30666b88ddfae7450635c` —
    `test(app): isolate R34 live fixture from legacy scenarios`;
21. `6f1acf56d0007a05c081c0217f099a5fcbc4a754` —
    `app: preserve R16B abort on no-MPEG input stop failure`.

Commit `131c1544...` was an over-broad text-edit attempt that accidentally
modified neighboring guard expressions. It was detected before acceptance and
superseded by `eb9ade65...`, which reconstructed `src/app.c` from the exact
pre-R34 source authority and applied the composition structurally. The bad
intermediate commit is retained as immutable branch history and is not evidence
for any accepted behavior.

## Final partial changed-path set

The start-to-pre-log changed paths are confined to:

- `src/app.c`;
- new `src/app_mpeg_product.c/.h`;
- focused Application/R34 tests;
- two historical source-boundary tests narrowed only so R32/R33 mechanism
  ownership remains enforced while the later authorized R34 public composition
  is allowed;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- root source-topology allowlist;
- Application topology/dictionary explanatory text.

No Input, UI, Configuration, Management, RFB, Transport, MPEG worker/runtime,
Display implementation, media-clock implementation, AUDIO, Pi product,
persistence/editor or H1/B4A forensic source was modified.

## Exact blocker discovered

R34 cannot safely complete with the currently accepted lower-owner contract.

The missing contract is:

**a safe R33-style local dormancy path for a teardown-required pre-START R21
partial owner that may still contain a live MPEG worker/PS2 worker runtime and
open Transport MPEG run.**

The already-accepted R21 source proves such a state is possible.

In `pstvnc_app_mpeg_run_start()`, ordering is:

1. open the Transport MPEG run;
2. initialize PS2 worker runtime;
3. create/start the MPEG worker;
4. arm P3;
5. initialize P7;
6. only later set `start_invoked=1` and invoke Transport START.

Pre-START failure invokes `pstvnc_app_mpeg_run_unwind_pre_start()`. That
unwind correctly attempts worker safe-stop/join/release and runtime release.

But if the safe-stop or join proof itself fails, the accepted code fails closed:

- run state becomes `FAULTED`;
- `session_teardown_required=1`;
- exact generation remains allocated;
- worker may remain started/live;
- PS2 worker runtime may remain owned;
- Transport MPEG run may remain open;
- START was never invoked.

The accepted deterministic
`test_cleanup_failure_faults_and_blocks_retry()` proves exactly this class.
It forces a post-worker-start/pre-START failure plus
`PSTVNC_MPEG_WORKER_SYNC_FAILED` on worker stop and verifies:

- `PSTVNC_APP_MPEG_RUN_CLEANUP_FAILED`;
- state `FAULTED`;
- teardown required;
- worker STOP was attempted;
- worker JOIN did not occur;
- worker runtime release did not occur;
- Transport pre-start abort did not occur;
- START did not occur.

This is deliberately retained ownership, not a clean no-MPEG attempt.

## Why accepted R33 cannot retire that state

R33's accepted
`pstvnc_app_mpeg_run_session_abort_entry_valid()`
requires a complete post-START live-owner set, including all of:

- nonzero current generation;
- exact old Transport ticket;
- open Transport MPEG run;
- owned PS2 worker runtime;
- started worker;
- initialized exact-generation P7 consumer;
- armed P3 presentation;
- `start_invoked=1`.

A pre-START cleanup failure may have:

- `start_invoked=0`;
- no initialized P7 consumer;
- already-aborted P3 presentation;
- nevertheless a still-live worker/runtime/Transport run.

R33 therefore rejects exactly the partial owner that R34 would need to retire.

This is not safe to relabel as a no-MPEG attempt. The worker was really started
and may still be unwinding or blocked in a Transport-backed MPEG feed/activity
path.

## Why legacy one-shot Transport abort is insufficient

R34 must not call legacy
`pstvnc_transport_session_abort()`
and then destroy/reuse Application state for this partial owner.

R33 was introduced specifically to separate:

1. Transport terminal wakeup with runtime storage retained;
2. module-local asynchronous worker dormancy;
3. final Transport storage reclamation.

Transport runtime release has a media-waiter rendezvous fence, but that proves
only the Transport waiter's protected rendezvous is no longer owned. It does
not prove the Application MPEG worker has been safely stopped, joined, its exact
terminal outcome observed, its worker stack/thread released, and its PS2
worker-runtime resources reclaimed.

Using one-shot abort for the pre-START partial owner would therefore bypass the
module-local dormancy proof required by R33's ownership rationale.

The R34 packet explicitly forbids this kind of bypass and directs
Reconstruction to return BLOCKED when the lower-owner contract cannot support
safe composition.

## Required lower-owner prerequisite

A later lower-owner packet must earn one of the following equivalent safe
contracts before R34 can complete:

- extend R33's local-abort owner so it can accept and safely drain the exact
  retained **pre-START partial** R21 owner states that can survive failed
  pre-start unwind; or
- provide a narrower dedicated pre-START partial-owner teardown seam that,
  after Transport begin-abort/retained storage proof, can safely:
  - stop/join any started exact-generation worker,
  - preserve/read terminal outcome when applicable,
  - release worker/thread/stack,
  - release PS2 worker runtime,
  - retire any remaining local P7/P3 partial state according to exact ownership,
  - preserve the no-START fact,
  - then prove the old Application MPEG owner dormant before final Transport
    close.

The prerequisite must not synthesize START/RETIRE success and must not weaken
Transport's retained-runtime release fence.

After that prerequisite is Foreman-accepted, R34 may resume from current partial
Application composition and add deterministic coverage for the pre-START
teardown-required P10 failure path.

## Deterministic partial evidence

At exact partial source authority
`6f1acf56d0007a05c081c0217f099a5fcbc4a754`,
GitHub Actions run `36206592591` completed with an overall failure only because
dictionary reconciliation was deliberately not performed after this blocker was
found.

Successful final partial-source jobs:

- host-unit — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS with deterministic rebuild.

Observed host output includes:

- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `APP_PRODUCT_BINDINGS_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS`;
- `PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app_mpeg_run_test: PASS`;
- `transport_mpeg_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- existing Input/product-action/config/management/RFB/Transport/media-clock
  regressions PASS.

Pinned PS2 compile explicitly compiles:

- `src/app.c`;
- `src/app_product_bindings.c`;
- `src/app_mpeg_product.c`;
- `src/app_mpeg_activation.c`;
- `src/app_mpeg_calibration.c`;
- `src/app_mpeg_frame.c`;
- `src/app_mpeg_run.c`.

`CLEAN_PS2_COMPILE_CHECK=PASS`.

Linked/reproducibility evidence at this partial head:

- `ELF_PRISTINE_SHA256=8cc45464d0e8d71111677a865ddb5bca7f09c58013f1ad88e70f683b86533adc`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=185f1de7fd1bd658f350e3927bae3e7d69fb0733fb6194baeee37e7bd35ff8cd`;
- `PT_LOAD_BYTES=524820`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

These partial R34 bytes differ from accepted R33 and are **not accepted product
authority**. They are build evidence only and are hardware-pending. They must
not be treated as hardware-qualified or deployment-ready.

## Intentionally unreconciled dictionary/project gates

Dictionary reconciliation was intentionally not triggered after the ownership
blocker was proven.

Therefore the same run reports expected bookkeeping failures:

- project-check:
  - `SOURCE_TOPOLOGY_MISSING_LOCAL_FILE_COVERAGE=src/app_mpeg_product.c`;
  - `SOURCE_TOPOLOGY_MISSING_LOCAL_FILE_COVERAGE=src/app_mpeg_product.h`;
- dictionary-long:
  - strict missing generated R34 symbols;
  - `ATTENTION_COUNT=124`;
  - `SOURCE_DICTIONARIES=FAIL_STRICT`.

This is not represented as R34 acceptance evidence. No generated dictionary
authority was minted for a packet that cannot yet be accepted.

## R34 criterion disposition at block

These are Reconstruction Worker dispositions only.

- `A006-R34-C1 EXPLICIT_R32_BINDING_INSTALLS_INTO_EACH_FRESH_R29_RUNTIME`
  — MET in partial source/host evidence.
- `A006-R34-C2 DESKTOP_CONTEXT_ELIGIBILITY_TRACKS_REAL_APPLICATION_OWNERSHIP`
  — MET in partial source/host evidence.
- `A006-R34-C3 FRESH_ATTEMPT_OWNS_FRESH_P3_P9_R21_AND_TRANSPORT_ACCESS`
  — MET in partial source/host evidence.
- `A006-R34-C4 SEMANTIC_MPEG_CALIBRATION_ACTION_ROUTES_WITHOUT_PHYSICAL_GESTURE_LOGIC`
  — MET in partial source/host evidence.
- `A006-R34-C5 CALIBRATION_FOREGROUND_GETS_CONTROLLER_FIRST_REFUSAL`
  — MET in partial source/host evidence.
- `A006-R34-C6 ACCEPTED_CALIBRATION_HANDS_TO_P10_ONCE_OR_FAILS_CLOSED`
  — PARTIAL/BLOCKED: ordinary accept/rollback paths are composed, but a
  teardown-required pre-START P10 failure lacks a safe lower-owner dormancy
  route.
- `A006-R34-C7 WAIT_FIRST_FRAME_AND_MPEG_OWNED_ARE_SERVICED_AT_SAFE_CADENCE`
  — MET in partial source/host evidence.
- `A006-R34-C8 OWNED_CALIBRATION_OR_LIVE_RUN_BLOCKS_OVERLAP`
  — MET in partial source/host evidence.
- `A006-R34-C9 RFB_FLOW_REMAINS_P2_GOVERNED_THROUGH_PROTECTED_AND_LIVE`
  — MET in partial source/host evidence.
- `A006-R34-C10 SESSION_FAILURE_USES_R33_BEFORE_TRANSPORT_RELEASE_OR_REPLACEMENT`
  — BLOCKED by the exact pre-START partial-owner gap above.
- `A006-R34-C11 NO_RETIRE_RECALIBRATION_AUDIO_PERSISTENCE_OR_DEFAULT_SCOPE_CREEP`
  — MET in partial source/host evidence.
- `A006-R34-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN`
  — NOT MET because the packet is blocked before dictionary reconciliation and
  final canonical project/dictionary acceptance run.

R34 cannot be source-accepted because all twelve criteria are required.

## State / contract accounting

Consumed:

- Foreman State revision `0073`;
- accepted R29-R33 semantic/config/management/lifecycle authority;
- R16B replacement lifecycle;
- R26 media-clock session binding;
- current Work Log Contract revision `0007`.

Produced before the block:

- partial ordinary R34 semantic activation composition;
- narrow `app_mpeg_product.{c,h}` coordinator;
- exact R32 binding installation into fresh R29 runtimes;
- live DESKTOP eligibility publication;
- P9 controller first refusal and P10 handoff composition;
- R22 current-tick service composition;
- post-START R33 teardown composition;
- focused host/source-boundary tests;
- PS2 build enrollment;
- exact lower-owner blocker evidence.

Not produced:

- no Foreman State update;
- no Foreman acceptance;
- no dictionary reconciliation/final project green claim;
- no safe pre-START teardown-required P10 handling;
- no normal R23/R24 retirement/reveal;
- no recalibration-after-run policy;
- no AUDIO/Pi/persistence/editor/mailbox work;
- no hardware qualification.

## Next pickup

`NEXT_PICKUP=FOREMAN_REVIEW_R34_BLOCKER_AND_ISSUE_LOWER_OWNER_PRESTART_PARTIAL_MPEG_DORMANCY_PACKET_BEFORE_R34_RESUME`

The Reconstruction Worker stops here. The Foreman must independently recover
live repository authority, review this BLOCKED R34 source/evidence record and
the accepted R21/R33 contracts, then either issue the required lower-owner
prerequisite packet or otherwise resolve the ownership gap. Reconstruction must
not bypass R33 with legacy one-shot Transport release for a retained pre-START
MPEG worker.
