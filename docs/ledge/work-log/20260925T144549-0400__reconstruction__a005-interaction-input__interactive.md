DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T14:45:49-04:00
COMPLETED_AT=2026-09-25T15:05:29-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a005-interaction-input
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=dd771a86c0922f7c4f4b5a4ebc333e16d496f293
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — A005 R28 semantic product-action binding core

## Recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`dd771a86c0922f7c4f4b5a4ebc333e16d496f293`.

Foreman State revision `0067` had independently accepted R27 and activated
exactly:

`A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a005-interaction-input`;
- `WORKER_KEY=interactive`;
- `ARCHITECTURE_BLOCKER=NONE`;
- Input owns semantic action/binding recognition;
- physical MPEG CALIBRATION product binding selection remains deferred;
- live Input-runtime wiring remains deferred;
- Application routing into P9/P10 remains deferred;
- H1 held START+SELECT / 750 ms calibration entry is discarded as product
  binding authority;
- no UI/RFB/MPEG/media-clock/Transport/AUDIO/Pi side effects are authorized.

The worker consumed Foreman State revision 0067, Reconstruction Contract
revision 0006, Work Log Contract revision 0007, Architecture Overlay revision
0007, Wire Runtime Decisions revision 0011, A005 interaction/input audit
revision 0001, B07/B09 Input/keyboard/local-UI audit authority, B10/B11
configuration/recovery/management audit authority, and the frozen B4A runtime
hotkey/arbitration evidence identified by State 0067.

The GitHub-connected Worker seat cannot observe external local/Pi checkout
staging, untracked files, or worktree dirtiness. No local reset, clean, stash,
or checkout-over is claimed.

## Result

R28 is source-complete as a Reconstruction candidate for independent Foreman
review.

Final pre-log authority:

`c9df08288689d47eb85c889b8c95c7a4741a48a9`

R28 adds an Input-owned semantic product-action vocabulary, caller-supplied
typed binding model, pure arbitration resolver, and ordinary typed FIFO payload.
It selects no physical product binding and performs no action side effect.

## Semantic product-action vocabulary

New clean Input source:

- `src/input/product_action.c`;
- `src/input/product_action.h`.

The semantic vocabulary currently contains:

- `PSTVNC_PRODUCT_ACTION_NONE`;
- `PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION`.

`MPEG_CALIBRATION` is meaning only. It is not a controller chord, not P9
entry, and not MPEG execution.

`pstvnc_product_action_is_valid()` is the public value validator used by the
ordinary Input FIFO boundary.

## Caller-supplied typed binding model

One `pstvnc_product_action_binding_t` contains exactly:

- project-owned physical controller `button_mask`;
- semantic `action`;
- trigger kind `SETTLE`, `RELEASE`, or `HOLD`;
- context `DESKTOP` or `GLOBAL`.

`pstvnc_product_action_resolver_init()` accepts one immutable caller-supplied
binding array. A zero-count/null-array initialization is valid and represents
explicit absence of product bindings.

Non-empty binding validation rejects:

- zero button mask;
- any mask bit outside `PSTVNC_CONTROLLER_BUTTON_MASK`;
- NONE or unknown semantic action;
- NONE or unknown trigger;
- NONE or unknown context;
- duplicate exact physical masks, including cross-context duplicates that
  would become ambiguous while desktop is active;
- invalid resolver/array arguments.

R28 contains no static/default binding table.

## Reconstructed B4A arbitration behavior

The clean resolver preserves the reusable qualified B4A arbitration policy
without importing B4A's physical product mappings.

Frozen policy values retained exactly:

- ordinary settle: `8` stable polls;
- extendable settle: `18` stable polls;
- HOLD completion: `120` hold polls after ordinary settle admission.

The resolver samples only buttons that occur in the caller-supplied binding
universe.

Every exact relevant-mask change:

- records legitimate gesture growth/shrink;
- resets stable-settle progress;
- clears pending active SETTLE/HOLD state;
- resets HOLD progress.

### RELEASE safety

A RELEASE binding may arm only while arriving at its exact mask through pure
gesture growth.

If the physical gesture grows beyond the armed exact RELEASE mask, that release
arm is canceled.

A later shrink back into the subset does not manufacture a new release arm.

An armed RELEASE fires only when its exact old mask is broken by pure shrink and
its context remains eligible.

This preserves the B4A anti-subset-leakage property.

### SETTLE versus HOLD extension

A SETTLE binding ordinarily resolves after 8 stable polls.

If its exact mask is a strict subset of an eligible HOLD binding, it waits for
the 18-poll extendable settle threshold instead. This gives the larger HOLD
gesture the same qualified human-construction window recovered from B4A.

A HOLD binding becomes active only after the normal settle boundary and then
requires the full 120 hold-poll threshold before semantic resolution.

### Post-fire latch

After SETTLE or HOLD resolution, the resolver remains latched until all relevant
binding-universe buttons are physically released.

A RELEASE action that fires while a relevant subset remains down is likewise
latched until full relevant release.

If RELEASE fires on the sample that already reaches relevant mask zero, that
sample itself is full-release proof and the next nonzero sample may begin a
fresh gesture.

Shrinking a fired gesture cannot fire a subset action.

## Desktop-context provenance

For DESKTOP bindings, context is not inferred at fire time alone.

The resolver records whether the physical gesture began while desktop context
was eligible. Once a nonzero gesture loses desktop eligibility,
`desktop_context_preserved` becomes false monotonically for that gesture.

Therefore:

- a gesture that begins outside DESKTOP cannot become a desktop shortcut merely
  because UI context changes while buttons remain down;
- a gesture that begins in DESKTOP and later loses it cannot reacquire desktop
  authority before full physical release;
- GLOBAL bindings remain eligible independently of desktop context.

A physical `connection_epoch_started` sample clears all pending arbitration
state before interpreting that new epoch.

## Ordinary semantic Input FIFO

`src/input/input.h` now defines:

`PSTVNC_INPUT_EVENT_PRODUCT_ACTION`.

Its payload is one validated:

`pstvnc_product_action_t`.

Queue admission validates PRODUCT_ACTION payload meaning rather than accepting
NONE/unknown product actions merely because the event-type enum is valid.

Existing controller-state, mouse-update, and keyboard-tap admission behavior is
unchanged.

Focused FIFO evidence proves:

- MPEG_CALIBRATION semantic action round-trips by value;
- NONE and unknown actions are rejected;
- product action preserves FIFO order alongside controller/mouse/keyboard;
- product action survives ring-buffer wrap;
- discard removes it with the same ordinary queue semantics;
- existing event families remain usable after discard.

No urgent mailbox or second concurrency path was introduced.

## Historical START+SELECT disposition

R28 does **not** reproduce the H1
`h1_mpeg_calibration_entry_hold.*` product binding.

The source-boundary test strips comments and verifies the new Input action/
resolver source contains no:

- `PSTVNC_CONTROLLER_BUTTON_START`;
- `PSTVNC_CONTROLLER_BUTTON_SELECT`;
- native `PAD_START` / `PAD_SELECT`;
- `750000` hold constant;
- compiled default binding table.

The historical START+SELECT/750 ms adapter remains forensic evidence only, as
required by A005.

## No cross-domain effect or live routing

R28 deliberately does not wire the resolver into `src/input/input_runtime.c`.

No R28 product source performs:

- P9/P10/Application MPEG calls;
- RFB calls;
- Transport calls;
- media-clock arm;
- UI mutation;
- MPEG worker/decoder/presentation work;
- urgent-control publication;
- Configuration parsing or persistence;
- Pi/Wire/AUDIO behavior.

The source-boundary fixture enforces those exclusions mechanically.

A later packet may choose a physical binding and compose resolver output into
ordinary Input/Application routing. R28 itself does neither.

## Deterministic focused evidence

New host fixtures:

- `tests/unit/product_action_test.c`;
- `tests/unit/product_action_event_test.c`;
- `tests/unit/product_action_source_test.py`.

The resolver fixture covers:

1. invalid mask/action/trigger/context and ambiguous binding rejection;
2. caller-injected MPEG_CALIBRATION SETTLE resolution;
3. exact-mask change resetting settle progress;
4. legitimate RELEASE growth arm and shrink fire;
5. growth canceling release and shrink being unable to re-arm;
6. 18-poll SETTLE extension in the presence of a larger HOLD;
7. full 8+120 B4A HOLD timing and post-fire latch;
8. desktop-context start/provenance and no mid-gesture reacquisition;
9. connection-epoch reset.

The event fixture covers PRODUCT_ACTION validation, FIFO ordering, copy,
wraparound, and discard.

Final host output includes:

- `PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `INPUT_TEST=PASS`;
- `CONTROLLER_EVENT_TEST=PASS`;
- `LOCAL_CONTROLLER_TEST=PASS`.

Cross-domain regression evidence also remains green for R27 Application, P9/P10,
R21-R24, R26 media-clock binding, and Transport MPEG seams.

## Documentation and topology

Living authority now records R28 in:

- `docs/development/semantic-product-actions.md`;
- `docs/development/source-topology.md`;
- `docs/development/README.md`;
- `docs/INDEX.md`.

The Input dictionary introduction now explicitly records caller-supplied
semantic product-action binding/arbitration ownership and the absence of
compiled physical product mappings/effects.

## Dictionary reconciliation

After source/tests/docs stabilized, the Worker emitted the exact authorized
no-tree-change trigger:

`667f69f8cd913324306e726cbaf83e557bd33719`

message:

`tooling(symbols): run deterministic dictionary reconciliation`.

The repository-authorized reconciler produced:

`35bce051f532e7a0c59555650b1f77d68f65563c`

message:

`docs(symbols): reconcile current clean definitions`.

That generated commit changed only:

- `src/input/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

The final source-identical verification commit is:

`c9df08288689d47eb85c889b8c95c7a4741a48a9`

message:

`test: verify final R28 semantic product action authority`.

## Intermediate deterministic corrections

The first host run reached the new source-only test before executing the compiled
unit binaries and exposed one test-script typo:
`input_code` was referenced instead of the already-loaded `input_source`
text.

Commit:

`9d5a4c8580ee0f63d8751cdb795c7b03de6e8a64`

corrected only that test variable reference.

The following run proved all compiled R28 and existing host regressions green.
The only remaining failures before reconciliation were expected strict
dictionary/topology coverage for the two new Input files and changed Input
symbols.

No product behavior was loosened to satisfy either issue.

## Commits

The complete pre-log Worker range is seven commits ahead / zero behind starting
Foreman authority:

1. `93e11bee4a48c2b3740ae1714a995577ffed4415` —
   `input: add semantic product action resolver`;
2. `6f806cc63a8bc838fcf16ffe4320e03a43b724d9` —
   `test(input): prove R28 semantic binding core`;
3. `9d5a4c8580ee0f63d8751cdb795c7b03de6e8a64` —
   `test(input): fix R28 source scan input reference`;
4. `029379e131a8fc01e39c26689dbec7da6d5ee1d4` —
   `docs(input): record R28 semantic product action boundary`;
5. `667f69f8cd913324306e726cbaf83e557bd33719` —
   deterministic dictionary reconciliation trigger;
6. `35bce051f532e7a0c59555650b1f77d68f65563c` —
   generated Input dictionary/portal reconciliation;
7. `c9df08288689d47eb85c889b8c95c7a4741a48a9` —
   exact final source verification.

No concurrent branch movement or write collision occurred during Worker writes.

## Changed paths

Exact compare from starting authority
`dd771a86c0922f7c4f4b5a4ebc333e16d496f293` to final pre-log authority
`c9df08288689d47eb85c889b8c95c7a4741a48a9` is seven commits ahead / zero
behind.

Changed paths are only:

- `src/input/product_action.c`;
- `src/input/product_action.h`;
- `src/input/input.c`;
- `src/input/input.h`;
- `src/input/SYMBOLS.md`;
- `tests/unit/product_action_test.c`;
- `tests/unit/product_action_event_test.c`;
- `tests/unit/product_action_source_test.py`;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `docs/development/semantic-product-actions.md`;
- `docs/development/source-topology.md`;
- `docs/development/README.md`;
- `docs/INDEX.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No Application, Input-runtime, UI, RFB, MPEG, media-clock, Transport, AUDIO, Pi,
Configuration parser, or H1 forensic product source was changed.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A005-R28-C1 TYPED_SEMANTIC_PRODUCT_ACTION_VOCABULARY` — MET.
- `A005-R28-C2 TYPED_BINDING_MASK_ACTION_TRIGGER_CONTEXT` — MET.
- `A005-R28-C3 NO_COMPILED_PHYSICAL_MAPPING_FOR_MPEG_CALIBRATION` — MET.
- `A005-R28-C4 EXACT_MASK_CHANGE_RESETS_PENDING_PROGRESS` — MET.
- `A005-R28-C5 RELEASE_ARMING_CANNOT_BE_MANUFACTURED_BY_SHRINK` — MET.
- `A005-R28-C6 SETTLE_HOLD_EXTENSION_PREVENTS_SUBSET_PREEMPTION` — MET.
- `A005-R28-C7 FIRED_ACTION_LATCHES_UNTIL_FULL_RELEVANT_RELEASE` — MET.
- `A005-R28-C8 DESKTOP_CONTEXT_CANNOT_BE_ACQUIRED_MID_GESTURE` — MET.
- `A005-R28-C9 ORDINARY_INPUT_FIFO_CARRIES_TYPED_PRODUCT_ACTION_WITH_NO_REGRESSIONS` — MET.
- `A005-R28-C10 NO_APPLICATION_CONFIG_UI_RFB_MPEG_OR_MAILBOX_SCOPE_CREEP` — MET.
- `A005-R28-C11 H1_START_SELECT_750MS_PRODUCT_BINDING_REMAINS_DISCARDED` — MET.
- `A005-R28-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

All twelve R28 criteria are therefore `MET` as Worker dispositions.

## Final machine evidence

Exact final pre-log authority:

`c9df08288689d47eb85c889b8c95c7a4741a48a9`

Canonical GitHub Actions workflow:

`36177342038`

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

- `PRODUCT_ACTION_SOURCE_TEST=PASS`;
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
- `transport_mpeg_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=218 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned compile explicitly compiled `src/input/input.c` and
  `src/input/product_action.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact R28 linked PS2 identity:

- `ELF_PRISTINE_SHA256=954e0cf8ff435e9e948afc5ace3251108d5736957bf45273f86bbdd274e5698f`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=2281d0d06b62635dfb829512c7c51c10cf1bfd3c6f96169db677bb034b952876`;
- `PT_LOAD_BYTES=515732`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

The prior fully Foreman-accepted R27 PS2 identity was:

- `ELF_PRISTINE_SHA256=7f5decc9f09b32d1060f18eca1462bcb189cc3ce96297fd5b3435ba448288d2c`;
- `PT_LOAD_SHA256=30c8d9d1db9106f51c8de56c88e68cf3d6181d98cfd8ff4d402ce24a6c94c13d`;
- `PT_LOAD_BYTES=513940`.

R28 therefore changes linked PS2 loadable bytes. The new identity is repository
reproducible but **hardware-pending**. No physical/hardware qualification is
inherited or claimed.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_R28`
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

## State/contract accounting

Consumed:

- Foreman State revision `0067`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- A005/B07-B09/B10-B11 audit authority;
- frozen B4A arbitration evidence named by State 0067.

Produced:

- no Foreman state revision;
- no architecture/contract revision;
- no physical MPEG CALIBRATION binding;
- no live Input-runtime resolver wiring;
- no Application P9/P10 routing;
- no UI mapping/help surface;
- no config parser/persistence;
- no RFB/MPEG/media-clock/Transport/AUDIO/Pi behavior revision;
- no Foreman acceptance;
- no independent Validation result;
- no hardware qualification.

## Findings / blockers

No R28 source blocker remains within the authorized packet.

The existing physical controller facts and ordinary semantic Input FIFO were
sufficient. The qualified B4A arbitration policy could be reconstructed as a
pure Input owner without adopting B4A's binding table or H1's MPEG entry chord.

Physical MPEG CALIBRATION binding selection and Application routing remain
downstream authority and were not selected by this Worker.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R28_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect R28 source/evidence/log, decide acceptance,
and select any later bounded packet. Reconstruction must not self-accept R28 or
begin deferred physical binding selection or Application routing.
