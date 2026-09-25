DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T16:20:06-04:00
COMPLETED_AT=2026-09-25T16:25:45-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a005-interaction-input
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=6f6cffe32874b615bfdb71c03eef328d7c2a7188
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R29 accepted; human-readable product binding config R30 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A005-INPUT-RUNTIME-PRODUCT-ACTION-PUBLICATION-R29`, independently recover
live repository authority, inspect the exact source/test/build range, disposition
R29, then choose one dependency-correct next Reconstruction packet without
performing product implementation from the Foreman seat.

Truthful round start:

`STARTED_AT=2026-09-25T16:20:06-04:00`

Live pickup authority:

`6f6cffe32874b615bfdb71c03eef328d7c2a7188`

The branch was eight commits ahead / zero behind prior Foreman closeout
`b51ea127e69717e4ea9ad3d6237377a8b9baa47f`.

Consumed governing/relevant authority included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/status.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Foreman State revision 0068;
- A005 interaction/input audit revision 0001;
- A006 top-level orchestration/shutdown audit revision 0001;
- B07/B09 input/UI authority;
- B10/B11 configuration/binding/recovery authority;
- accepted R28 semantic binding core;
- accepted R27 ordinary MPEG-capable session foundation;
- accepted P9/P10/R21-R26 and R25 Pi product composition;
- current clean config text helpers;
- project controller button vocabulary;
- frozen B4A typed hotkey evidence.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260925T160751-0400__reconstruction__a005-interaction-input__interactive.md`.

No newer Validation/Continuity/Diagnostics/support authority displaced R29
during pickup.

## Exact R29 source range

Assigning Foreman authority:

`b51ea127e69717e4ea9ad3d6237377a8b9baa47f`

Final pre-log source authority:

`56092a3a6d02df4a9feb89aec5c095e2542b178f`

Immutable Reconstruction closeout:

`6f6cffe32874b615bfdb71c03eef328d7c2a7188`

Independent compare proves seven source/test/docs commits ahead / zero behind:

1. `5d530ee22cdfc8ad8ceb54f326f53152ce9a5052` —
   `input: publish configured product actions from runtime`;
2. `6ca1a9e11f3e1616b0bb0311d23ead0421cf373b` —
   `test(input): prove R29 runtime action publication`;
3. `b0b180139a9cea4d5efa6b0eee1ec9d08dc2e72d` —
   `test(input): allow PS2 thread-entry cast in R29 host fixture`;
4. `34392a85285ee6486656fc4d272c08d676a84c8c` —
   `docs(input): record R29 runtime action publication`;
5. `b920500a50d08cf22b320723ea5db8a5ac987bce` —
   deterministic dictionary-reconciliation trigger;
6. `d0e30274d768bf83c005dacde4416bb4916e4dfd` —
   generated Input dictionary/portal reconciliation;
7. `56092a3a6d02df4a9feb89aec5c095e2542b178f` —
   `test: verify final R29 Input runtime action authority`.

Final source to closeout is exactly one additional immutable work-log commit.

The closeout worker's metadata truthfully records its own actual recovery start
at final source authority because the R29 source range had already landed before
that interactive closeout shift began. That is not treated as authoring
provenance; the assigning-Foreman-to-final-source ancestry above is the
independent source-range authority.

Changed paths are confined to:

- `src/input/input_runtime.c`;
- `src/input/input_runtime.h`;
- `src/input/SYMBOLS.md`;
- focused Input-runtime product-action tests/stubs;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- directly affected Input development/index documentation;
- generated source dictionary portal.

No ordinary Application, local UI/local-controller, Configuration parser, RFB,
Transport, media clock, MPEG, AUDIO, Pi or H1 forensic product source changed.

## Independent R29 source review

### Optional caller-owned bindings

Every runtime initializes one valid zero-binding R28 resolver.

`pstvnc_input_runtime_set_product_action_bindings()`:

- is rejected after worker start;
- normalizes zero count to a null binding pointer;
- delegates binding validation to the accepted R28 resolver;
- stores no compiled/default product binding.

Worker start reinitializes/revalidates the retained immutable caller binding
array before CreateThread. Caller mutation before start therefore cannot bypass
R28 validation.

### Runtime-local resolver ownership

The R28 resolver is a field of `pstvnc_input_runtime_t`, not a global.

Runtime start clears resolver gesture history while preserving only the selected
binding values. Separate runtime objects therefore carry separate physical-
gesture epochs and cannot inherit one another's settle/hold/release/latch state.

### Physical-poll timing

R29 translates each trustworthy pad sample into the project-owned
`pstvnc_controller_state_t` value and runs
`pstvnc_product_action_resolver_observe()` before ordinary controller-state or
mouse publication.

This happens on every trustworthy pad sample, not only samples carrying
pressed/released edges. Therefore R28's qualified 8/18/120 thresholds remain
physical-poll timing rather than becoming sparse FIFO-event timing.

### Explicit context authority

R29 adds only:

`pstvnc_input_runtime_set_product_action_desktop_eligible(runtime, 0|1)`.

The Input runtime stores one live integer admission fact and supplies it to R28
for each trustworthy sample.

Input does not include local UI or Application state, and does not install a
callback owner. R28 remains sole owner of gesture-start context provenance,
context-loss invalidation and no mid-gesture reacquisition.

### Same-sample event ordering

If R28 resolves an action, R29 immediately attempts to enqueue exactly one
ordinary `PSTVNC_INPUT_EVENT_PRODUCT_ACTION`.

Only after successful action publication does the same sample proceed to the
existing CONTROLLER_STATE path and then mouse interpretation/publication.

This gives the later Application consumer the required semantic first-refusal
ordering without executing any product effect in the controller worker.

### Fail-closed publication

Resolver failure records:

`PSTVNC_INPUT_RUNTIME_ERROR_PRODUCT_ACTION_RESOLVE`.

Product-action publication uses the established event queue semaphore and
ordinary FIFO.

Queue wait, queue-full or queue-signal failure records the established worker
error and returns failure from sample processing. Same-sample controller/mouse
publication does not proceed after a resolved semantic action was lost.

### Hard ownership boundaries

Physical continuity loss resets R28 resolver history before existing hard mouse
boundary handling.

The explicit libpad handoff path resets product-action resolver history before
handoff acknowledgement. Returning pad ownership re-enters through the existing
new physical connection epoch, so held pre-handoff progress cannot continue as
the prior gesture.

Ordinary mouse-interpretation suspension does not transfer libpad ownership and
therefore deliberately does not create another resolver owner or automatically
reset the gesture.

### Existing behavior and wake path

Zero-binding and non-firing configured paths preserve current controller-state
sparsity and mouse behavior.

The existing activity notification runs after successful sample processing.
Tests prove it observes already-enqueued PRODUCT_ACTION, CONTROLLER_STATE and
MOUSE_UPDATE work in the expected order.

There is still exactly one ordinary FIFO and no urgent product-action mailbox.

### Scope review

Maintained R29 Input-runtime source contains no:

- compiled `MPEG_CALIBRATION` action selection;
- compiled SETTLE/RELEASE/HOLD product policy;
- START+SELECT adapter;
- 750000-us H1 entry value;
- P9/P10/Application route;
- media-clock arm;
- RFB/Transport/MPEG/AUDIO/Pi product effect;
- UI/local-controller reach-through;
- Configuration persistence;
- urgent mailbox.

No R29 product defect was found.

## R29 acceptance criteria

A005-R29-C1=MET
A005-R29-C2=MET
A005-R29-C3=MET
A005-R29-C4=MET
A005-R29-C5=MET
A005-R29-C6=MET
A005-R29-C7=MET
A005-R29-C8=MET
A005-R29-C9=MET
A005-R29-C10=MET
A005-R29-C11=MET
A005-R29-C12=MET

Overall disposition:

`R29_DISPOSITION=FOREMAN_ACCEPTED`

## Exact R29 machine/build evidence

Final source authority:

`56092a3a6d02df4a9feb89aec5c095e2542b178f`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `36182879910`
attempt `1`
conclusion `success`

The exact run object reports:

- branch `ledge/h1-all-guns`;
- head SHA `56092a3a6d02df4a9feb89aec5c095e2542b178f`;
- push event;
- title `test: verify final R29 Input runtime action authority`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile skipped as expected.

Observed focused/cross-domain evidence:

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

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=220 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled
  `src/input/product_action.c` and `src/input/input_runtime.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=8b0c020f4e410d7a7cc5f7015cc34845f02d382b16055dc607d635a8bbdc7c51`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=f72a1a65b6a16abf792c803a8cfa3322ded51ccf6a038b6a27e94e3ee6eb43f7`
`PT_LOAD_BYTES=516116`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from accepted R28 and becomes the current PS2 hardware-debt
identity.

Evidence classification:

SOURCE_COMPLETE=YES
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
PS2_PT_LOAD_CHANGED=YES
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
HARDWARE_PENDING=YES
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Next dependency analysis: Configuration must own physical binding authority

R29 is capable of publishing a semantic action only when an explicit caller
supplies R28 bindings. Ordinary Application currently supplies none.

Choosing a chord inside Application would violate both clean architecture and
B10:

- Input owns recognition;
- Configuration owns human-readable controller/hotkey bindings;
- Application owns routing/execution;
- the old B4A compiled binding table is historical mechanism, not completed
  clean configuration authority.

B10 further requires configuration to remain inspectable, manually editable,
backup-friendly and eventually UI-editable, with recognized values strict,
unknown future content forward-compatible where ownership is unambiguous, and
parsing atomic with respect to live authority.

The existing clean `src/config/text.*` contains reusable whitespace, integer
and boolean helpers but no product-binding document/model.

Therefore Application routing is not yet the next dependency. It would have no
legitimate ordinary product binding set to install.

## R30 representation decision

The next packet is:

`B10-HUMAN-READABLE-PRODUCT-ACTION-BINDINGS-R30`.

The initial recognized grammar is deliberately small:

```
[bindings]
mpeg_calibration = <button[+button...]> , <trigger> , <context>
```

Current symbolic button names are exact lowercase project button names:

`select l3 r3 start up right down left l2 r2 l1 r1 triangle circle cross square`.

Current trigger names are:

`settle release hold`.

Current contexts are:

`desktop global`.

This grammar makes action identity the human-readable key and keeps the value
strictly physical binding plus trigger/context. Missing `[bindings]` or missing
`mpeg_calibration` means zero configured product bindings.

No chord is selected by State 0069. In particular, START and SELECT remain
ordinary legal symbolic buttons that a human may deliberately include in a
future config, but there is no START+SELECT special case and no inherited 750 ms
H1 behavior. Generic R28 trigger semantics remain authoritative.

Unknown future sections and keys remain opaque/ignored where ownership is
unambiguous so a newer config need not become unusable after rollback.

A recognized malformed or duplicate binding must fail atomically without
publishing partial typed authority.

A deterministic canonical formatter is included so the same typed binding model
can later serve manual editing and UI/persistence without creating hidden
parallel state. R30 itself performs no persistence.

## Foreman-owned state publication

Published Foreman State revision 0069:

`71c254b752dc6fac187622b3e4dd1c9dbef94642`

message:

`docs(foreman): accept R29 and activate binding config model`

State 0069:

- accepts R29 and all twelve criteria;
- records exact R29 source, CI and hardware-debt identity;
- activates exactly one next packet:
  `B10-HUMAN-READABLE-PRODUCT-ACTION-BINDINGS-R30`;
- keeps the Reconstruction work key `a005-interaction-input`;
- makes Configuration the physical binding representation owner;
- defines no default product chord;
- preserves user/manual configuration as the binding authority;
- keeps Application routing, config retrieval/persistence, local binding-editor
  UI and urgent mailbox deferred;
- preserves R28/R29 Input recognition ownership;
- claims no hardware qualification.

Immediately before this immutable log write, branch authority remained exactly
at State 0069.

## Current hardware debt

Current fully Foreman-accepted PS2 loadable authority is R29:

`PT_LOAD_SHA256=f72a1a65b6a16abf792c803a8cfa3322ded51ccf6a038b6a27e94e3ee6eb43f7`
`PT_LOAD_BYTES=516116`

It is repository-reproducible and hardware-pending.

Accepted R25 maintained Pi product/runtime source remains at:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

with no Pi operator/hardware qualification claim.

R30 may add linked Configuration binding code and may therefore produce a newer
PS2 PT_LOAD identity. Reconstruction must measure exact identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=B10-HUMAN-READABLE-PRODUCT-ACTION-BINDINGS-R30
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R30_FROM_FOREMAN_STATE_0069

The Reconstruction worker must independently recover live branch authority,
consume State 0069 plus B10/A005/R28/R29 authority, execute only R30, create
exactly one immutable Reconstruction record using WORK_ITEM_KEY
`a005-interaction-input`, stop, and return the baton.

Do not choose a default MPEG-calibration chord, do not install bindings into
R29, do not route P9/P10, do not add management fetch/persistence or local UI,
do not create an urgent mailbox, and do not claim hardware qualification.
