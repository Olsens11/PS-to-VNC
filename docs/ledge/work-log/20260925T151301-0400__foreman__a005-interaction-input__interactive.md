DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T15:13:01-04:00
COMPLETED_AT=2026-09-25T15:21:18-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a005-interaction-input
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=22aea289ba06d744387913bc1809c60ba32800a8
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R28 accepted; Input-runtime product-action publication R29 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28`, independently recover live
repository authority, inspect the worker's exact source/test/build range,
disposition R28, reconcile the next dependency against A005/B07/B10 and clean
concurrency authority, and publish exactly one bounded Reconstruction packet
without implementing product behavior from the Foreman seat.

Truthful round start:

`STARTED_AT=2026-09-25T15:13:01-04:00`

Live pickup authority:

`22aea289ba06d744387913bc1809c60ba32800a8`

The branch was eight commits ahead / zero behind prior Foreman closeout
`dd771a86c0922f7c4f4b5a4ebc333e16d496f293`.

Consumed governing/relevant authority included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/status.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Foreman State revision 0067;
- A005 interaction/input audit revision 0001;
- A006 top-level orchestration/shutdown audit revision 0001;
- B07/B09 input/UI audit authority;
- B10/B11 configuration/binding/recovery audit authority;
- cross-domain semantic-input/concurrency authority;
- accepted R27 ordinary session foundation;
- accepted P9/P10/R21-R26 and R25 Pi composition authority;
- frozen B4A typed hotkey/arbitration authority.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260925T144549-0400__reconstruction__a005-interaction-input__interactive.md`.

No newer Validation/Continuity/Diagnostics/support authority displaced R28
during pickup.

## Exact R28 worker range

Reconstruction started from:

`dd771a86c0922f7c4f4b5a4ebc333e16d496f293`

and landed seven pre-log commits:

1. `93e11bee4a48c2b3740ae1714a995577ffed4415` —
   `input: add semantic product action resolver`;
2. `6f806cc63a8bc838fcf16ffe4320e03a43b724d9` —
   `test(input): prove R28 semantic binding core`;
3. `9d5a4c8580ee0f63d8751cdb795c7b03de6e8a64` —
   `test(input): fix R28 source scan input reference`;
4. `029379e131a8fc01e39c26689dbec7da6d5ee1d4` —
   `docs(input): record R28 semantic product action boundary`;
5. `667f69f8cd913324306e726cbaf83e557bd33719` —
   deterministic dictionary-reconciliation trigger;
6. `35bce051f532e7a0c59555650b1f77d68f65563c` —
   generated Input dictionary/portal reconciliation;
7. `c9df08288689d47eb85c889b8c95c7a4741a48a9` —
   `test: verify final R28 semantic product action authority`.

Final pre-log source authority:

`c9df08288689d47eb85c889b8c95c7a4741a48a9`

The next and only following commit was the required immutable Reconstruction log:

`22aea289ba06d744387913bc1809c60ba32800a8`.

Independent compare proved seven commits ahead / zero behind from assigning
Foreman authority to final source and exactly one log-only commit afterward.

Final-source changed paths are confined to:

- `src/input/product_action.c/.h`;
- `src/input/input.c/.h`;
- Input/root symbol dictionaries;
- focused product-action/FIFO tests;
- host/build enrollment;
- directly affected Input development/index documentation.

No Application, live Input-runtime, UI, RFB, MPEG, media-clock, Transport,
AUDIO, Pi, Configuration-parser or H1 forensic product source changed.

## Independent R28 source review

### Semantic value boundary

`pstvnc_product_action_t` currently admits exactly one routable meaning:

`PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION`.

The action is not a controller chord and carries no execution callback or
cross-domain owner.

The ordinary Input FIFO now has:

`PSTVNC_INPUT_EVENT_PRODUCT_ACTION`

whose payload is one validated semantic product-action value.

NONE and unknown product-action payloads are rejected at queue admission.

### Typed binding authority

`pstvnc_product_action_binding_t` contains exactly:

- project-owned physical button mask;
- semantic action;
- trigger kind;
- context.

Accepted named trigger values are:

- SETTLE;
- RELEASE;
- HOLD.

Accepted named contexts are:

- DESKTOP;
- GLOBAL.

Resolver initialization accepts an explicit zero-binding configuration and
rejects invalid non-empty authority, including:

- zero masks;
- bits outside the project controller mask;
- NONE/unknown actions;
- NONE/unknown triggers;
- NONE/unknown contexts;
- duplicate exact masks.

Duplicate exact masks are rejected across contexts as well because GLOBAL and
DESKTOP would otherwise become ambiguous while desktop is active.

The resolver retains the caller-supplied immutable binding array; it owns
recognition history, not binding-selection policy.

### No physical MPEG binding

Independent source review found no compiled/default product-action table.

Maintained clean product-action source contains no selected
MPEG_CALIBRATION button mask and no H1 750 ms START+SELECT adapter.

The H1 entry adapter remains forensic evidence only.

### Arbitration semantics

R28 preserves the qualified B4A mechanism values:

- SETTLE: 8 stable polls;
- extendable SETTLE: 18 stable polls;
- HOLD: 120 hold polls after ordinary settle admission.

Exact relevant-mask changes reset stable and HOLD progress.

RELEASE may arm only when the gesture reaches its exact mask by pure growth.
Growing beyond an armed RELEASE cancels the arm. Later shrink cannot manufacture
a subset release action.

A SETTLE binding whose exact mask is a strict subset of an eligible HOLD waits
through the 18-poll extension boundary instead of winning at ordinary settle.

After a SETTLE/HOLD fires, the resolver remains latched until all relevant
binding-universe buttons are released. A RELEASE that fires while relevant
buttons remain down receives the same latch protection.

This preserves the central B4A anti-subset-leakage rule.

### Context provenance

DESKTOP eligibility is not inferred solely at fire time.

A gesture must begin while DESKTOP is eligible and must preserve that authority
until resolution. Once DESKTOP is lost during a nonzero gesture,
`desktop_context_preserved` becomes false for the remainder of the gesture.

Therefore an already-held gesture cannot become or become again a desktop
shortcut merely because foreground later changes.

GLOBAL remains a distinct semantic context and R28 executes no GLOBAL effect.

A physical `connection_epoch_started` sample resets pending resolver history
before the new physical epoch is interpreted.

### FIFO behavior

PRODUCT_ACTION uses the same ordinary bounded FIFO as existing controller,
mouse and keyboard events.

Independent source/test review confirms:

- copy-by-value;
- preserved ordering with all event families;
- ring wrap;
- discard;
- existing event-family acceptance after discard;
- no second queue and no urgent mailbox.

### Scope review

R28 does not invoke or reach into:

- P9/P10/R21-R24 Application MPEG owners;
- media-clock arm;
- Transport;
- RFB;
- UI;
- MPEG worker/decoder/presentation;
- AUDIO;
- Pi;
- Configuration persistence;
- live Input-runtime production.

No R28 product defect was found.

## R28 acceptance criteria

A005-R28-C1=MET
A005-R28-C2=MET
A005-R28-C3=MET
A005-R28-C4=MET
A005-R28-C5=MET
A005-R28-C6=MET
A005-R28-C7=MET
A005-R28-C8=MET
A005-R28-C9=MET
A005-R28-C10=MET
A005-R28-C11=MET
A005-R28-C12=MET

Overall disposition:

`R28_DISPOSITION=FOREMAN_ACCEPTED`

## Exact R28 machine/build evidence

Final pre-log source authority:

`c9df08288689d47eb85c889b8c95c7a4741a48a9`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `36177342038`
attempt `1`
conclusion `success`

The exact run object reports:

- branch `ledge/h1-all-guns`;
- head SHA `c9df08288689d47eb85c889b8c95c7a4741a48a9`;
- push event;
- title `test: verify final R28 semantic product action authority`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile skipped as expected.

Observed focused/cross-domain evidence:

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
- `transport_mpeg_test: PASS`;
- R25 Pi MPEG product composition: 9 tests, OK.

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=218 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned compile explicitly compiled `src/input/input.c`;
- pinned compile explicitly compiled `src/input/product_action.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=954e0cf8ff435e9e948afc5ace3251108d5736957bf45273f86bbdd274e5698f`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=2281d0d06b62635dfb829512c7c51c10cf1bfd3c6f96169db677bb034b952876`
`PT_LOAD_BYTES=515732`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from accepted R27 and becomes the new current PS2 hardware-debt
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

## Next dependency analysis: live recognition remains Input-owned

R28 deliberately stopped before `input_runtime`.

The clean architecture is explicit:

- Input owns live binding recognition and semantic event production;
- the controller thread publishes only semantic events or urgent intent;
- Application owns routing and all cross-domain product side effects;
- configuration owns human-readable binding selection.

Current `input_runtime` publishes trustworthy CONTROLLER_STATE events only for
transition-bearing samples outside mouse suspension. That is sufficient for
local UI routing but is not sufficient for R28 SETTLE/HOLD timing, because a
stable held gesture must advance on every trustworthy physical poll.

Moving the resolver into Application over the existing sparse queue would
therefore silently redefine the qualified 8/18/120 physical-poll contract and
would move live hotkey recognition out of its clean Input owner.

The next dependency is consequently the Input-runtime publication boundary, not
Application execution and not a guessed physical product binding.

### Packet boundary

The next bounded packet is:

`A005-INPUT-RUNTIME-PRODUCT-ACTION-PUBLICATION-R29`.

R29 composes the accepted R28 resolver with trustworthy live pad polling.

It must preserve these boundaries:

1. bindings remain explicit caller-supplied immutable values;
2. zero bindings preserves current product behavior;
3. resolver timing sees every trustworthy physical sample;
4. DESKTOP eligibility is one narrow caller-owned fact rather than Input
   reaching into UI/Application internals;
5. if an action resolves, its ordinary semantic event is queued before
   same-sample controller/mouse work;
6. the Input worker executes no action effect;
7. existing hard libpad handoff/connection epochs cannot leak resolver history;
8. queue/resolver failures are fail-closed;
9. no urgent mailbox is introduced;
10. no physical MPEG binding is selected.

Physical binding selection, human-readable persistence and Application routing
remain separate downstream dependencies.

## Foreman-owned state publication

Published Foreman State revision 0068:

`395127dd548ab2fe0eecfebfffd4f1969e8f32b3`

message:

`docs(foreman): accept R28 and activate input action publication`

State 0068:

- accepts R28 and all twelve criteria;
- records exact R28 source, CI and hardware-debt identity;
- activates exactly one next packet:
  `A005-INPUT-RUNTIME-PRODUCT-ACTION-PUBLICATION-R29`;
- keeps Reconstruction work key `a005-interaction-input`;
- requires live R28 timing from every trustworthy physical poll;
- keeps semantic publication on the ordinary FIFO;
- requires action event ordering before same-sample controller/mouse work;
- preserves Application as sole product-effect owner;
- selects no physical MPEG-calibration mapping;
- keeps Application routing, Configuration persistence, UI binding editor and
  urgent mailbox deferred;
- claims no hardware qualification.

Immediately before this immutable log write, branch authority remained exactly
at State 0068.

## Current hardware debt

Current fully Foreman-accepted PS2 loadable authority is R28:

`PT_LOAD_SHA256=2281d0d06b62635dfb829512c7c51c10cf1bfd3c6f96169db677bb034b952876`
`PT_LOAD_BYTES=515732`

It is repository-reproducible and hardware-pending.

Accepted R25 maintained Pi product/runtime source remains at:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

with no Pi operator/hardware qualification claim.

R29 may change linked Input-runtime code and may therefore create a newer PS2
PT_LOAD identity. Its worker must measure exact identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A005-INPUT-RUNTIME-PRODUCT-ACTION-PUBLICATION-R29
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R29_FROM_FOREMAN_STATE_0068

The Reconstruction worker must independently recover live branch authority,
consume State 0068 plus A005/B07/B10 and clean concurrency authority, execute
only R29, create exactly one immutable Reconstruction record using
WORK_ITEM_KEY `a005-interaction-input`, stop, and return the baton.

Do not choose a physical MPEG-calibration chord, do not revive START+SELECT, do
not route P9/P10, do not add binding persistence/config UI, do not create an
urgent mailbox, and do not claim hardware qualification.
