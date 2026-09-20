# Reconstruction shift — A004 generic RFB freeze/refresh policy R2

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T17:52:45-04:00
COMPLETED_AT=2026-09-20T18:01:04-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=aaa452899780116e0bc82d3de5d2975799eb94aa
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet `A004-RFB-FREEZE-REFRESH-R2`: reconstruct the
generic RFB-owned live-flow policy needed by later A004 ownership transitions.

The bounded owner is responsible only for:

- whether new framebuffer requests may be issued;
- whether newly completed remote RFB visuals may be published;
- whether one framebuffer request is already outstanding; and
- whether a frozen->thawed interval owes one FULL/nonincremental recovery
  request.

This shift did not wire MPEG CALIBRATION, MPEG start/stop, first-frame
promotion, presentation suppression/composition, pointer neutralization,
Transport START/RETIRE, media-clock arm, scheduler/drop, final retirement
handoff, Application orchestration, A005 input reconstruction, or hardware
qualification.

No Foreman state document was modified and no next packet was published.

## Authority / execution classification

Consumed:

- Foreman state revision 0024;
- Reconstruction Contract revision 0006;
- work-log contract revision 0006;
- Wire runtime decisions revision 0011, especially current Q7;
- architecture overlay revision 0004;
- A004 audit revision 0001;
- current clean architecture and source-topology/naming policies.

Execution classification:

`AUTONOMOUS_RECONSTRUCTION`

The packet required no live-Pi or physical-PS2 observation. Repository/history
work, commits, dictionary reconciliation, CI execution and CI log inspection
were performed through GitHub/GitHub Actions. No user-proxy terminal action was
requested.

Live branch authority at wake was:

`aaa452899780116e0bc82d3de5d2975799eb94aa`

## Current-source authority inspected

Read current:

- `src/rfb/rfb_session.{c,h}`;
- `src/rfb/bridge.{c,h}`;
- RFB async-framing, initial-frame, initial-coverage and session host tests;
- current Application request cadence;
- `src/rfb/SYMBOLS.md`;
- `tests/Makefile`;
- canonical ledge reconstruction workflow and applicable repository build
  gates.

Recovered current clean facts:

- `pstvnc_rfb_session_request_update()` owns exact request serialization;
- initial RFB authority still requires a strict nonincremental complete initial
  desktop before READY;
- `pstvnc_rfb_session_try_receive_update()` may return IDLE only before
  consuming a byte of the next server message;
- once one server message begins, its exact parser ownership runs to the
  complete-message boundary;
- finite-session quiesce also uses that same proven boundary;
- current Application has no MPEG lifecycle and still drives ordinary simple
  request/response cadence.

The P2 work therefore did not create another parser or safe-boundary mechanism.

## Frozen H1 forensic trace

Frozen authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Inspected at minimum:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_rfb_gate.{c,h}`
  and its host test;
- `h1_mpeg_calibration_rfb_schedule.{c,h}` and its host test;
- `h1_mpeg_calibration_rfb_flow.{c,h}` and its host test;
- `h1_mpeg_presentation_owner.{c,h}` and its host test;
- generic `experiments/media-harness-h1/h1_rfb_session_runtime.{c,h}`
  far enough to identify callback timing and parser boundaries.

Recovered behavior:

- the old gate blocked new requests and remote visual presentation without
  destroying RFB/Transport state;
- frozen->thawed created one FULL-refresh debt;
- the old scheduler retained one outstanding request at a time;
- a response requested before freeze still completed protocol ownership and
  cleared the outstanding fact while visual publication stayed suppressed;
- post-thaw FULL recovery preceded ordinary incrementals;
- the generic H1 RFB runtime evaluated flow policy only at its existing
  complete-message/pre-message-IDLE scheduling boundary;
- WAIT_FIRST_FRAME was a global frozen-desktop gap, while later MPEG-owned
  presentation used a distinct composited mode.

Current Q7 supersedes the old implication that active/retiring MPEG globally
freezes RFB. P2 therefore reconstructs only a generic global ownership-gap
policy and contains no MPEG lifecycle state.

## Placement / design decision

Added one small pure policy owner in the existing RFB domain:

- `src/rfb/flow_policy.c`
- `src/rfb/flow_policy.h`

No new top-level directory was created.

The clean implementation collapses the H1 experiment's separate gate/schedule
layers into one RFB-owned value object because those facts form one coherent
live-request/publication policy in the clean architecture.

It deliberately does not embed the policy into the parser/session yet. The
existing session remains the sole protocol/framing owner; callers will later
consume the policy at that already-proven scheduling boundary.

### Intentional hindsight repair

The H1 scheduler consumed its pending FULL obligation while merely deciding the
next request. That allowed a failed/unperformed send to lose recovery debt.

The clean implementation makes
`pstvnc_rfb_flow_policy_next_request()` non-mutating. FULL debt is cleared
only when `pstvnc_rfb_flow_policy_record_request_sent()` successfully records
the currently owed FULL send.

This is a reconstruction-owned fail-closed correction required by the active
P2 packet; it does not alter RFB wire/parser behavior.

## Exact commits / files

### `7e779fdb113ee37dfda1e60f4887c7d7c608037a`

`feat(rfb): add generic freeze refresh flow policy`

Added/changed:

- `src/rfb/flow_policy.c`
- `src/rfb/flow_policy.h`
- `tests/unit/rfb_flow_policy_test.c`
- `tests/Makefile`

The policy contains only:

- generic frozen/thawed state;
- one outstanding-request bit;
- one pending FULL-refresh bit;
- HOLD / INCREMENTAL / FULL request decisions;
- non-mutating next-request inspection;
- fail-closed successful-send accounting;
- complete-response accounting;
- generic remote-publication permission.

The host test covers:

- ordinary incremental cadence;
- deterministic one-outstanding enforcement;
- repeated inspection without consuming FULL debt;
- wrong-kind/invalid accounting without clearing debt;
- same-state freeze/thaw idempotence;
- coalescing repeated restoration need before recovery advances;
- a distinct later freeze/thaw interval after recovery advances;
- freeze while one request is in flight;
- completion of that real response through the current
  `pstvnc_rfb_session_try_receive_update()` parser;
- authoritative framebuffer truth advancing while publication is blocked;
- thaw choosing a real nonincremental RFB request before incrementals resume.

### `7600d77f158892d1b850c686371cb3c9e72d67b1`

`tooling(symbols): run deterministic dictionary reconciliation`

Tree-identical trigger commit used only to invoke the repository's authorized
deterministic dictionary-reconciliation workflow.

### `a64b6427356ff1c2fcbc87f2e40cd126048b7589`

`docs(symbols): reconcile current clean definitions`

Created by the repository's dictionary automation.

It changed only:

- `src/rfb/SYMBOLS.md` — 45 new clean symbol rows;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` — generated RFB count update.

No product/test behavior changed.

### `5aa84f1dc0e3017b88214e20637ab1a0605cc38a`

`docs(rfb): document live flow policy ownership`

Updated only the `src/rfb/SYMBOLS.md` responsibility prose so the domain
description explicitly includes generic live request/publication flow policy.

This also provided a normal user-authored coherent-head push for final CI after
the Actions-generated dictionary commit, because GitHub does not recursively
start another workflow from the workflow token's generated push.

## Packet source boundary

GitHub comparison from Foreman base
`aaa452899780116e0bc82d3de5d2975799eb94aa` through final pre-log authority
`5aa84f1dc0e3017b88214e20637ab1a0605cc38a` contains exactly six changed paths:

- `src/rfb/flow_policy.c`
- `src/rfb/flow_policy.h`
- `src/rfb/SYMBOLS.md`
- `tests/unit/rfb_flow_policy_test.c`
- `tests/Makefile`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

No existing `src/rfb/rfb_session.*`, `src/rfb/bridge.*`, Application,
Transport, MPEG, UI, Display, Platform, framebuffer, input, or media source was
changed.

## A004 P2 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review,
not Foreman acceptance.

### A004-P2-C1 GENERIC_RFB_POLICY — MET

The public API uses only generic RFB flow vocabulary. No MPEG, calibration,
decoder, producer, GS or presentation-owner type/header/dependency is required.

Boundary comments explicitly state excluded ownership but introduce no
cross-domain dependency.

### A004-P2-C2 SAFE_BOUNDARY — MET

No RFB parser/session source changed. Policy contracts require request decisions
at the existing RFB safe scheduling boundary. The integration host test clears
request accounting only after the real current parser returns a complete UPDATE.

No mid-message interruption or second parser/quiesce boundary was created.

### A004-P2-C3 ONE_OUTSTANDING — MET

Once one request send is recorded, next-request returns HOLD until exactly one
complete response clears the outstanding fact. Duplicate/wrong accounting fails
closed.

### A004-P2-C4 INFLIGHT_FREEZE — MET

The focused integration test sends a real incremental
FramebufferUpdateRequest, freezes while it is outstanding, consumes its real
complete update through current RFB session parsing, proves framebuffer truth
changed, clears outstanding once, and keeps remote publication disabled.

### A004-P2-C5 FULL_REFRESH — MET

A genuine frozen->thawed interval yields FULL. After that FULL is successfully
recorded and its response completes, ordinary next request returns INCREMENTAL.

The test additionally proves the actual RFB request incremental byte is zero for
the FULL decision.

### A004-P2-C6 COALESCED_OBLIGATION — MET

Repeated inspection is non-mutating. Frozen state blocks request issuance
without clearing debt. An older outstanding request blocks scheduling without
clearing debt. Repeated restoration before recovery advances coalesces into one
pending FULL, while a later genuine interval after a recovery send creates a
new FULL debt that survives until the prior response completes.

### A004-P2-C7 Q7_COMPATIBILITY — MET

The policy has no MPEG state. Global freeze is caller-supplied generic intent.
Nothing equates active or retiring MPEG with frozen RFB.

Current Q7 therefore remains possible: later presentation ownership may thaw
this global gate after first-frame promotion and allow RFB framebuffer truth to
refresh underneath composited MPEG, including the current-Q7 retirement
overlap.

### A004-P2-C8 CLEAN_INTEGRATION — MET

Both maintained source files have clean synopses. The new focused test is
registered in the canonical host unit suite. Deterministic dictionary
reconciliation added all 45 required RFB rows and regenerated the portal;
`src/rfb` now reports 409 symbols.

The final coherent-head workflow is fully green.

### A004-P2-C9 EVIDENCE_BOUNDARY — MET

Repository/source/host machine evidence is distinct from live product runtime
and physical hardware qualification.

The P2 policy is deliberately not yet wired into Application/product runtime.
No physical hardware claim and no full-A004 completion claim are made.

The broad repository PS2 compile/link jobs passed, but the stage-local clean
compile/link tooling does not directly establish that this currently unwired P2
module executes on PS2. Direct P2 compilation/execution evidence here is the
canonical host unit target. Later runtime wiring and hardware qualification
remain later A004 work.

## CI / verification evidence

### First behavior-head workflow

Run `35540127730` at
`7e779fdb113ee37dfda1e60f4887c7d7c608037a`:

- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- project-check — FAIL only because the two new clean RFB files had not yet
  received dictionary coverage;
- dictionary-long — FAIL with exactly those two file-coverage findings plus
  their 45 new definitions;
- host-unit — FAIL before the new RFB flow test was reached, at the pre-existing
  `transport_runtime_test`:
  - line 787 expected RFB-channel available bytes to equal payload size;
  - line 789 expected `activity_sequence` to advance.

No Transport source changed in this packet. The immediately preceding starting
authority workflow, run `35539918320` at `aaa452899...`, had completed
SUCCESS.

The first failure is retained as a nondeterministic pre-existing
`transport_runtime_test` occurrence and was not silently discarded or
"repaired" by changing unrelated Transport source.

### Reconciliation-trigger workflow

Run `35540163604` at
`7600d77f158892d1b850c686371cb3c9e72d67b1`:

- host-unit — PASS;
- `transport_runtime_test` — PASS;
- new `RFB_FLOW_POLICY_TEST` — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — PASS and published `a64b642...`;
- project-check/dictionary-long — expected FAIL in parallel against the
  pre-reconciliation trigger commit, before the generated dictionary commit
  existed.

### Final coherent-head workflow

Run `35540317214` at
`5aa84f1dc0e3017b88214e20637ab1a0605cc38a`:

- host-unit — PASS, including:
  - `transport_runtime_test: PASS`;
  - `RFB_FLOW_POLICY_TEST=PASS`;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile repository gate — PASS;
- ps2-link/current-source reproducibility repository gate — PASS;
- dictionary-reconcile — SKIPPED as expected on a normal coherent commit.

Final repository comparison and source inspection also confirmed no
unauthorized domain changes and no cross-domain type/header dependency in the
new policy.

## Ambiguities preserved rather than inferred

- "Global freeze" is a generic caller intent for an ownership gap, not a
  synonym for MPEG-active, MPEG-retiring, or calibration specifically.
- P2 does not decide which future owner asserts or clears that intent.
- P2 does not itself publish or suppress pixels; it exposes permission for the
  later presentation/application owner to honor at the established boundary.
- P2 does not infer Pi desktop geometry or suppression rectangles.
- P2 does not wire the policy into the current Application loop.
- Multiple freeze/thaw intervals that occur before any recovery send coalesce
  into the still-pending FULL debt. Once recovery work has advanced by
  successfully sending that FULL, a later genuine interval creates distinct
  debt even if the earlier FULL response is still in flight.
- Initial-frame authority remains unchanged and outside this READY/live policy.

## Evidence gaps

PENDING_LOCAL=NONE_FOR_A004_P2_REQUIRED_REPOSITORY_AND_HOST_GATES

P2_PRODUCT_RUNTIME_WIRING=NOT_IN_PACKET
P2_PHYSICAL_PS2_RUNTIME=NOT_RUN_NOT_CLAIMED
HARDWARE_PENDING=YES_FOR_LATER_A004_RUNTIME_AND_VISUAL_QUALIFICATION
FULL_A004_COMPLETION=NOT_CLAIMED

No user-assisted local evidence or hardware action was required.

## Next pickup

Return the baton to Foreman at pre-log authority
`5aa84f1dc0e3017b88214e20637ab1a0605cc38a` plus this immutable Reconstruction
log commit.

Foreman should independently inspect P2 source/tests/CI evidence and decide the
next bounded A004 owner seam under current Q7.

Reconstruction must not begin the presentation/first-frame tranche from this
shift.
