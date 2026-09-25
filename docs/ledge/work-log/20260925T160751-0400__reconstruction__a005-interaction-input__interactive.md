DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T16:07:51-04:00
COMPLETED_AT=2026-09-25T16:09:40-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a005-interaction-input
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=56092a3a6d02df4a9feb89aec5c095e2542b178f
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — A005 R29 Input-runtime product-action publication

## Recovery provenance

This Reconstruction closeout shift began after concurrent repository authority
had already advanced through the complete R29 source/test/documentation range.

At shift recovery the live branch was exactly:

`56092a3a6d02df4a9feb89aec5c095e2542b178f`

message:

`test: verify final R29 Input runtime action authority`.

Earlier in this interactive seat, while recovering the preceding R28 packet, an
attempted R28 source write was correctly rejected because branch authority had
moved. No stale write was forced. Independent refresh then proved that another
concurrent Reconstruction/Foreman sequence had already:

- completed R28;
- emitted its immutable Reconstruction record;
- obtained independent Foreman acceptance in State 0068;
- activated R29;
- landed and verified the complete R29 source range.

This shift therefore did not duplicate or rewrite already-landed R29 source.
It independently recovered State 0068, inspected the exact R29 implementation
and tests, verified the exact final source CI, and emits the one missing
immutable R29 Reconstruction closeout required by the active packet.

The GitHub-connected Worker seat cannot observe external local/Pi checkout
staging, untracked files or worktree dirtiness. No local reset, clean, stash or
checkout-over is claimed.

## Governing authority

Foreman State revision `0068` activates exactly:

`A005-INPUT-RUNTIME-PRODUCT-ACTION-PUBLICATION-R29`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a005-interaction-input`;
- `WORKER_KEY=interactive`;
- physical MPEG-calibration binding selection deferred;
- Application product-action routing deferred;
- Configuration binding persistence/editor deferred;
- urgent-control mailbox deferred;
- all product effects outside Input ownership prohibited.

The packet assignment/handoff authority is:

`b51ea127e69717e4ea9ad3d6237377a8b9baa47f`

The accepted R28 source authority consumed by R29 is:

`c9df08288689d47eb85c889b8c95c7a4741a48a9`.

The accepted R28 immutable Reconstruction record is:

`22aea289ba06d744387913bc1809c60ba32800a8`.

## Exact R29 source range

From assigning Foreman authority
`b51ea127e69717e4ea9ad3d6237377a8b9baa47f`
to final source authority
`56092a3a6d02df4a9feb89aec5c095e2542b178f`,
the branch is seven commits ahead / zero behind:

1. `5d530ee22cdfc8ad8ceb54f326f53152ce9a5052` —
   `input: publish configured product actions from runtime`;
2. `6ca1a9e11f3e1616b0bb0311d23ead0421cf373b` —
   `test(input): prove R29 runtime action publication`;
3. `b0b180139a9cea4d5efa6b0eee1ec9d08dc2e72d` —
   `test(input): allow PS2 thread-entry cast in R29 host fixture`;
4. `34392a85285ee6486656fc4d272c08d676a84c8c` —
   `docs(input): record R29 runtime action publication`;
5. `b920500a50d08cf22b320723ea5db8a5ac987bce` —
   exact deterministic dictionary-reconciliation trigger;
6. `d0e30274d768bf83c005dacde4416bb4916e4dfd` —
   generated Input dictionary/portal reconciliation;
7. `56092a3a6d02df4a9feb89aec5c095e2542b178f` —
   exact final source verification.

Changed paths in that R29 range are confined to:

- `src/input/input_runtime.c`;
- `src/input/input_runtime.h`;
- `src/input/SYMBOLS.md`;
- focused Input-runtime product-action tests and host stubs;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- directly affected Input development/index documentation;
- generated source-dictionary portal.

No ordinary Application, local UI/local-controller, Configuration parser,
RFB, Transport, media-clock, MPEG, AUDIO, Pi or H1 forensic product source
changed.

## Optional caller binding authority

Every `pstvnc_input_runtime_t` initializes with a valid zero-binding R28
resolver. Zero bindings therefore preserve the pre-R29 product behavior.

Before worker start a caller may invoke:

`pstvnc_input_runtime_set_product_action_bindings()`

with an explicit caller-owned immutable R28 binding set.

Invalid non-empty binding authority rejects before worker activation. Worker
start revalidates the retained immutable binding set and clears resolver history
before `CreateThread()`, so caller mutation between configuration and start
cannot become live Input authority.

No built-in/default physical MPEG CALIBRATION binding exists in Input runtime.

## Runtime-local resolver ownership

Each Input runtime instance owns exactly one
`pstvnc_product_action_resolver_t`.

No gesture state is process-global or shared between sessions.

Deterministic tests construct independent runtimes and prove advancing one
resolver's stable-poll history does not alter the other runtime.

Runtime start clears resolver history while preserving only the already-selected
immutable caller binding values.

## Every trustworthy sample drives R28

When a non-empty binding set is configured, every trustworthy physical
controller sample is passed exactly once to the accepted R28 resolver.

This occurs before sparse CONTROLLER_STATE publication and before mouse work.

Therefore stable held samples with no press/release edge still advance the
accepted R28 8/18/120 physical-poll timing. R29 does not reinterpret those
thresholds as queue-event counts.

Focused SETTLE and HOLD tests prove timing through stable no-edge samples.

## Explicit DESKTOP eligibility

R29 adds only one live caller-owned fact:

`pstvnc_input_runtime_set_product_action_desktop_eligible(runtime, 0|1)`.

Input runtime does not include or inspect Application/UI state and installs no
callback framework.

The current fact is supplied once per trustworthy physical sample to R28.
R28 remains sole owner of gesture-begin provenance, context loss and no
mid-gesture reacquisition semantics.

GLOBAL remains an R28 semantic context; Input runtime does not decide whether a
GLOBAL action is product-safe to execute.

## Product-action publication ordering

If R28 resolves one action, Input publishes exactly one:

`PSTVNC_INPUT_EVENT_PRODUCT_ACTION`

through the existing semaphore-protected ordinary
`pstvnc_input_queue_t`.

That action event is enqueued before any CONTROLLER_STATE or MOUSE_UPDATE from
the same physical sample.

Focused RELEASE evidence observes queue order:

1. PRODUCT_ACTION;
2. CONTROLLER_STATE;
3. MOUSE_UPDATE.

The existing activity-notify callback occurs only after successful per-sample
processing; focused evidence proves the callback sees the already-published
semantic queue work in that same order.

There is no second product-action queue and no urgent mailbox.

## Fail-closed publication

Product-action resolution contradiction records:

`PSTVNC_INPUT_RUNTIME_ERROR_PRODUCT_ACTION_RESOLVE`.

Product-action publication uses the existing queue semaphore and existing FIFO.
Wait, signal or full-queue failure records the corresponding established worker
error.

If an action resolves but cannot be published, the sample-processing path
returns failure immediately. Same-sample controller and mouse publication do not
continue after loss of the semantic action.

Focused injected queue-failure evidence proves that fail-closed ordering.

## Existing Input behavior retained

With zero bindings, deterministic sample sequences produce ordinary controller
and mouse events byte-for-byte equivalent to the pre-R29/nonfiring path.

With a configured binding that does not fire, the established controller-state
sparsity, mouse interpretation, physical continuity and ordinary event behavior
remain unchanged.

R29 adds recognition before those existing operations rather than replacing
their mechanisms.

## Hard ownership boundaries

A real physical continuity loss resets the runtime-local R28 resolver before the
existing mouse hard-boundary handling.

The explicit libpad handoff boundary discards queued semantic work, resets R28
gesture history before acknowledgement, then follows the existing pad
connection-epoch invalidation/rebase contract. Returned libpad ownership begins
from a fresh physical epoch and cannot continue pre-handoff
settle/hold/release/latch state.

Ordinary mouse-interpretation suspension does not transfer libpad ownership and
does not reset the product-action resolver merely because another local
foreground temporarily owns mouse interpretation.

Focused tests cover physical loss, hard handoff/return and mouse suspension
separately.

## Scope audit

The maintained R29 Input-runtime source contains no:

- compiled MPEG_CALIBRATION action constant or binding mask;
- SETTLE/RELEASE/HOLD physical product-selection constant;
- START+SELECT adapter;
- 750000-us H1 entry constant;
- Application/P9/P10 call;
- media-clock arm;
- Transport/RFB call or include;
- local UI/local-controller reach-through;
- MPEG worker/decoder/presentation call;
- AUDIO call;
- Pi product reference;
- Configuration parser/persistence;
- urgent/parallel mailbox.

The accepted R28 resolver remains the only recognition owner. R29 only feeds it
trustworthy physical samples and publishes its semantic result.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A005-R29-C1 OPTIONAL_CALLER_BINDINGS_VALIDATE_BEFORE_WORKER_START` — MET.
- `A005-R29-C2 RESOLVER_HISTORY_IS_FRESH_INPUT_RUNTIME_OWNERSHIP` — MET.
- `A005-R29-C3 EVERY_TRUSTWORTHY_PHYSICAL_SAMPLE_DRIVES_RESOLVER_TIMING` — MET.
- `A005-R29-C4 DESKTOP_ELIGIBILITY_IS_EXPLICIT_CALLER_FACT_NOT_UI_REACH_THROUGH` — MET.
- `A005-R29-C5 R28_CONTEXT_PROVENANCE_REMAINS_SINGLE_RECOGNITION_AUTHORITY` — MET.
- `A005-R29-C6 PRODUCT_ACTION_EVENT_PRECEDES_SAME_SAMPLE_CONTROLLER_MOUSE_WORK` — MET.
- `A005-R29-C7 RESOLVED_ACTION_PUBLICATION_FAILURE_IS_FAIL_CLOSED` — MET.
- `A005-R29-C8 ZERO_BINDING_AND_NONFIRING_PATHS_PRESERVE_EXISTING_INPUT_BEHAVIOR` — MET.
- `A005-R29-C9 HARD_HANDOFF_AND_CONNECTION_EPOCH_CANNOT_LEAK_GESTURE_HISTORY` — MET.
- `A005-R29-C10 EXISTING_ORDINARY_FIFO_AND_ACTIVITY_WAKE_ARE_THE_ONLY_PUBLICATION_PATH` — MET.
- `A005-R29-C11 NO_PHYSICAL_BINDING_APPLICATION_CONFIG_UI_RFB_MPEG_OR_MAILBOX_SCOPE_CREEP` — MET.
- `A005-R29-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

All twelve R29 criteria are therefore `MET` as Worker dispositions.

## Exact final machine evidence

Exact final pre-log source authority:

`56092a3a6d02df4a9feb89aec5c095e2542b178f`

Canonical GitHub Actions workflow:

`36182879910`

attempt:

`1`

conclusion:

`SUCCESS`.

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Observed focused/cross-domain evidence includes:

- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `INPUT_TEST=PASS`;
- `CONTROLLER_EVENT_TEST=PASS`;
- `LOCAL_CONTROLLER_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`;
- R25 Pi product composition: 9 tests, OK.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=220 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned compile explicitly compiled
  `src/input/product_action.c` and `src/input/input_runtime.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact R29 linked PS2 identity:

- `ELF_PRISTINE_SHA256=8b0c020f4e410d7a7cc5f7015cc34845f02d382b16055dc607d635a8bbdc7c51`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=f72a1a65b6a16abf792c803a8cfa3322ded51ccf6a038b6a27e94e3ee6eb43f7`;
- `PT_LOAD_BYTES=516116`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

The prior fully Foreman-accepted R28 identity was:

- `ELF_PRISTINE_SHA256=954e0cf8ff435e9e948afc5ace3251108d5736957bf45273f86bbdd274e5698f`;
- `PT_LOAD_SHA256=2281d0d06b62635dfb829512c7c51c10cf1bfd3c6f96169db677bb034b952876`;
- `PT_LOAD_BYTES=515732`.

R29 therefore changes linked PS2 bytes. The new identity is repository
reproducible but **hardware-pending**. No prior physical qualification is
inherited or claimed.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_R29`
`HOST_TESTED=PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`PS2_PT_LOAD_CHANGED=YES`
`MACHINE_EVIDENCE=GITHUB_ACTIONS`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO`
`HARDWARE_PENDING=YES`
`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State / contract accounting

Consumed:

- Foreman State revision `0068`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- accepted R28 semantic binding authority;
- A005/B07/B10 clean interaction/concurrency authority.

Produced by the recovered R29 source range:

- live optional Input-runtime semantic action publication;
- no physical MPEG CALIBRATION binding;
- no Application route;
- no Configuration persistence/editor;
- no urgent mailbox;
- no UI/RFB/MPEG/media-clock/Transport/AUDIO/Pi effect;
- no Foreman state;
- no Foreman acceptance;
- no independent Validation result;
- no hardware qualification.

## Findings / blockers

No R29 source blocker remains within the authorized packet.

The accepted R28 resolver and existing Input runtime/FIFO seams were sufficient.
No lower-owner implementation change was required outside Input ownership.

Physical MPEG-calibration binding selection and Application semantic-action
routing remain downstream/deferred authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R29_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect R29 source/evidence/log, decide acceptance,
and select any later bounded packet. Reconstruction must not self-accept R29,
select a physical MPEG-calibration chord, or begin Application routing.
