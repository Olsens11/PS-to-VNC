# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0016
RECORDED_AT=2026-09-17T07:18:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0006
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0035
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A003_AUDIT_REVISION=0001
BASED_ON_A004_AUDIT_REVISION=0001
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0003
BASED_ON_A003_MANUAL_COMPLETION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This Foreman revision changes reconstruction execution strategy without declaring A003 complete. It preserves coherent accepted A003 source, supersedes the autonomous A003-P2A Pi/Transport completion packet from state revision 0015, moves the unresolved real Transport/Pi exact-generation mechanics into a durable manual completion item, freezes dependency-facing owner contracts, and releases downstream owner-local reconstruction that can be proved without a fake production Transport.

The live branch was re-read immediately before this state write at `099dec22c71544c0980dae1769d19b650bb71973`.

Independent Validation remains a separate authority. Cross-machine Pi/PS2 validation and physical qualification remain pending.

## Current Foreman phase

`A003_TRANSPORT_PI_MANUAL_DEFERRED__A004_MPEG_CALIBRATION_CORE_PACKET_ISSUED`

## Execution-strategy supersession

State revision 0015 correctly resolved the conceptual owner split between a Pi PSTV session/mechanism owner and a Pi exact-generation business-state owner. That ownership decision remains useful.

However, its active autonomous `A003-P2A-COMPLETION` implementation packet is now **SUPERSEDED**.

Reconstruction is no longer authorized by Foreman state to:

- create/adopt a production `pi/pstv/` runtime merely because revision 0015 named that namespace;
- solve real Pi service/socket/thread/session ownership autonomously;
- complete P2A-2/P2A-5/P2A-6 by inventing substitute production behavior;
- advance P2B/P2C/P2D to make the exact-generation lifecycle look complete;
- infer real endpoint success from host-side contract fakes.

The real implementation/proof sequence is now governed by:

`docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md`

revision `0001`, commit `8fc78235061e9e0d7a95b82531c0324bf05a140a`, status:

`A003_TRANSPORT_PI_MANUAL_COMPLETION_DEFERRED`

The governing ledge architecture is now `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision `0003`, commit `099dec22c71544c0980dae1769d19b650bb71973`.

## A003 accepted work preserved

A003-P1 remains Foreman-accepted `MET` from the prior accepted reconstruction/integration evidence.

The coherent P2A partial source at `b609aec92f8f54d7d57ea81b38936bc45cd7d6d9` remains preserved. No rollback is assigned.

Accepted behavior includes:

- START explicit control identity: kind `11`, control channel `0`, flags `0`;
- exact 44-byte START v1 representation;
- RETIRE explicit control identity: kind `10`, control channel `0`, flags `0`;
- exact 12-byte big-endian RETIRE `(version, session_id, generation)` representation;
- explicit START / RETIRE / MPEG-DATA frame classification;
- MPEG media remains opaque `DATA/channel 4`;
- START-shaped exactly-44-byte MPEG DATA remains MPEG media;
- no magic-length payload sniffing;
- no per-MPEG-packet generation tags;
- accepted PS2 outbound START uses the existing ordered physical Transport send path;
- no prepared-generation or Pi producer business state was moved into PS2 Transport or MPEG decoder ownership.

### P2A criterion disposition under the deferral

- `P2A-1 START_CONTROL_IDENTITY = MET`
- `P2A-2 START_SEMANTICS = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`
- `P2A-3 RETIRE_WIRE_IDENTITY = MET`
- `P2A-4 PURE_MPEG_DATA = MET`
- `P2A-5 SOLE_TRANSPORT_OWNER = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`
- `P2A-6 OWNERSHIP = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`
- `P2A-7 NO_GENERATION_TAGGING = MET`
- `P2A-8 BOUNDED_SCOPE = MET`

The three partial criteria are not failed local source; they are exact real-owner implementation/proof gaps intentionally deferred. A003 as a whole is **not complete**.

## Binding source/component invariant

Architecture overlay revision 0003 makes the following rule binding for A004-A006 and the later A003 manual session:

> Inside one genuine component/local-cooperation directory, internal implementation files may cooperate directly. Across a real component boundary, communication must pass through the owning component's defined bridge or public process seam.

Therefore:

- directory membership follows coherent ownership, not call-site convenience;
- unrelated responsibilities may not be co-located merely to avoid a bridge;
- no bridge-per-caller or bridge-per-destination proliferation;
- Application coordinates owners only through public owner bridges;
- tests fake the same public owner boundary that production callers consume;
- private sockets, receiver threads, queues, parser state, decoder-worker state, compositor state, and input state remain private to their owner.

## Frozen dependency-facing contracts

The architecture overlay and deferred-manual item are the detailed authority. This state records the dependency facts downstream work may consume.

### Transport

Mechanism owner only: session lifecycle, one physical stream/sole receive path when real, framing/order, logical channels/control delivery, flow control, quiescence/dormancy, mechanism failure, and narrow public logical/control operations actually required by consumers.

Transport may eventually expose exact START send, RETIRE request/send, inbound exact-control publication, immutable mechanism-owned session/config facts, and terminal/quiescence/failure facts.

Transport does **not** own generation preparation/retirement proof, decode/presentation state, RFB restoration policy, or input/UI semantics.

### Exact generation

Owns exact active session/generation rules, immutable prepared generation state, START semantic validation, stale/repeated/conflicting rejection, and real `RETIREMENT_PROVEN` publication after the exact owner fence succeeds.

### MPEG

Owns decoder/worker behavior and decode/upload readiness. `FIRST_MPEG_FRAME_DECODED` is not physical presentation.

### RFB

Owns RFB protocol/framebuffer truth, safe complete-message/request freeze/quiescence, request mechanics, and fresh-full-update obligation.

### Presentation

Owns one physical GS/composition path, draw/matte/suppression geometry consumption, physical presentation, `FIRST_PHYSICAL_MPEG_FRAME`, visual ownership promotion, common-media-clock arm callsite, and presentation timing/drop policy.

### Input / UI

Owns physical observation, semantic foreground ownership, pointer/buttons/wheel, keyboard/modifier interpretation/publication, OSK/local foreground, release quarantine, and cooperative dormancy/teardown.

### Application

Owns cross-owner ordering/policy only. It may coordinate public facts such as `TRANSPORT_SESSION_READY`, `TRANSPORT_RECEIVER_DORMANT`, `GENERATION_PREPARED`, `RETIREMENT_PROVEN`, `RFB_QUIESCED`, `INPUT_DORMANT`, and `FIRST_PHYSICAL_MPEG_FRAME`, but obtains every fact from its real owner bridge.

## Contract-test evidence discipline

Downstream deterministic fakes are permitted only at public owner boundaries.

A downstream component may become:

- `SOURCE_COMPLETE`;
- `CONTRACT_COMPLETE`;
- `CONTRACT_TESTED`;
- `HOST_TESTED`;
- `PS2_COMPILE_PASS`;
- `PS2_LINK_PASS`;

while retaining exact dependency gaps.

Those labels never imply:

- `REAL_TRANSPORT_IMPLEMENTED`;
- `CROSS_MACHINE_VALIDATED`;
- `HARDWARE_QUALIFIED`.

Pending labels must state the precise unproven owner fact rather than hide local incompleteness behind a generic dependency label.

## A004-A006 replan

### A004 — Presentation / MPEG CALIBRATION

A004 is released for owner-local reconstruction against contracts. This includes MPEG CALIBRATION edit/commit semantics, base/inner/outer geometry, foreground semantics, RFB freeze/full-refresh policy, accept-to-first-frame protection, one Presentation/GS owner, compositor ordering, first physical frame promotion, common-clock arm callsite, absolute scheduler/drop policy, overlays, and restoration conditioned on abstract `RETIREMENT_PROVEN`.

The Pi mechanism producing `RETIREMENT_PROVEN` remains deferred.

**DESKTOP CALIBRATION** and **MPEG CALIBRATION** remain separate authorities under `LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` revision 0001.

### A005 — Interaction / Input

A005 may proceed with its owner-local physical polling/continuity, mouse/pointer/button/wheel interpretation, keyboard/modifier serialization, OSK/local foreground, release quarantine, suspend -> neutralize -> rebase -> release -> resume, calibration foreground handoff, parser-safe publication boundary, and cooperative teardown. Cross-owner interactions use the owning bridge.

### A006 — Application orchestration

A006 may reconstruct resident startup/session admission, CONFIG/profile consumption, component dependency ordering, steady-state coordination, failure convergence, terminal-result aggregation, reverse teardown, and repeated-session structure against owner bridges and deterministic contract fakes.

Application correctness is not evidence that a deferred producer of an owner fact is implemented.

## Exact A003 obligations remaining deferred

The manual completion item must eventually prove at minimum:

1. actual Pi PSTV runtime/service owner adoption;
2. one real Pi PSTV session owner and sole receive path;
3. explicit START receive through that owner;
4. real START semantic validation and immutable prepared generation;
5. producer preparation and MPEG emission;
6. retirement admission close with an intentionally exercised in-flight emission lease;
7. exact producer cleanup before RETIRE completion emission;
8. PS2 exact completion reception through the sole receiver/public seam;
9. decoder stop/join before reclaiming worker-visible resources;
10. residual MPEG queue drain with corresponding Transport credit restoration;
11. proof generation N cannot contaminate N+1;
12. repeated-generation finite EOF/cancel/error/retirement behavior;
13. cross-machine integration of reconstructed downstream owners;
14. physical PS2/Pi qualification and all-guns endurance.

A003 cannot be declared complete until these owner facts are proven.

## Fresh bounded Reconstruction packet — A004-MPEG-CALIBRATION-CORE

WORK_ITEM_KEY=`a004-presentation-calibration`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`099dec22c71544c0980dae1769d19b650bb71973`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md:0001`
ASSIGNING_CALIBRATION_INVARIANT=`LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md:0001`
ASSIGNING_ARCHITECTURE_OVERLAY=`LEDGE_ARCHITECTURE_OVERLAY.md:0003`
ASSIGNING_FOREMAN_STATE=`0016`
H1_FORENSIC_SOURCE=`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

### Objective

Reconstruct only the pure owner-local **MPEG CALIBRATION geometry and committed-state core** from frozen H1 evidence into clean production source with deterministic host tests and a narrow calibration-owner public seam. This packet must not depend on the deferred real Pi/Transport implementation.

### Required evidence to trace before editing

Read at minimum:

- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md`;
- `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md`;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision 0003;
- frozen H1 `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration.h`;
- frozen H1 `mpeg_presentation_calibration_geometry.c`;
- frozen H1 `mpeg_presentation_calibration_state.c` and the focused pure calibration tests;
- any later H1 evidence that proves a correction to those pure geometry/commit semantics.

Do not copy the experimental directory wholesale.

### CALIBRATION SEPARATION INVARIANT

**DESKTOP CALIBRATION and MPEG CALIBRATION are separate historical systems with separate state, geometry authority, ownership, and evidence. Do not derive MPEG-calibration behavior from the older desktop-calibration implementation merely because both manipulate screen rectangles or use similar UI mechanics. For MPEG calibration, trace `experiments/media-harness-h1/mpeg_presentation_calibration/` and A004 audit authority. If any relationship is unclear, preserve the separation and flag the ambiguity rather than merging the systems.**

### Required deliverables

1. Establish the smallest genuine production **MPEG CALIBRATION owner/local-cooperation boundary** consistent with current clean source conventions. If a new directory is warranted, its membership must be justified by coherent calibration ownership, not convenience. Do not place Presentation, Input, RFB, Transport, or Application internals inside it.

2. Reconstruct the pure MPEG CALIBRATION geometry meanings:
   - **base rectangle** = exact MPEG capture/presentation region;
   - **inner matte** = PS2-local presentation-only inset state;
   - **outer/suppression footprint** = expanded MPEG visual-ownership/RFB-suppression perimeter.
   Keep the three meanings distinct.

3. Preserve the audited geometry invariants from frozen H1 evidence:
   - base width/height remain MPEG-compatible with a 16-pixel minimum and 16-pixel size alignment;
   - base position remains pixel-precise and clamped to the local canvas;
   - resizing preserves the recovered center behavior where H1 authority requires it;
   - inner matte remains bounded by the base region and does not alter capture geometry;
   - outer matte/suppression expansion remains local bounded state and produces a suppression rectangle clamped to the canvas;
   - no arithmetic path may create invalid negative width/height or out-of-canvas committed geometry.

4. Reconstruct a single owner-local draft/default/committed model sufficient for later UI/Application consumers to edit and obtain one accepted immutable MPEG CALIBRATION geometry. Do not create a second accepted geometry store merely to cross a component boundary.

5. Expose only a narrow calibration-owner public seam sufficient for later consumers to:
   - initialize/query owner state;
   - apply owner-local geometry edits through semantic operations supported by H1 evidence;
   - inspect the current reviewable geometry;
   - commit one immutable accepted geometry value;
   - cancel/revert draft edits to the proper prior/default value;
   - derive/query the outer suppression rectangle from the same authoritative value.

   The public seam must not expose Presentation/GS state, RFB parser/request internals, Transport, Pi exact-generation state, or Input hardware state.

6. Preserve **acceptance vs first physical presentation** separation. A committed MPEG CALIBRATION geometry is only accepted geometry. This packet must not produce `GENERATION_PREPARED`, `FIRST_PHYSICAL_MPEG_FRAME`, visual ownership promotion, common-clock arm, or `RETIREMENT_PROVEN`.

7. Add deterministic host tests covering at minimum:
   - default geometry is valid and aligned;
   - base resize/move clamping and center-preserving resize behavior from H1;
   - exact 16-pixel size stepping/minimum;
   - independent inner-matte behavior without changing base capture geometry;
   - independent outer-matte behavior and suppression-rectangle clipping;
   - commit produces exactly one immutable accepted value exposed through the owner seam;
   - later draft edits do not mutate the previously returned/accepted committed value;
   - cancel/revert restores the proper committed/default draft without fabricating acceptance;
   - base/inner/outer meanings remain distinct;
   - no DESKTOP CALIBRATION source/state is used as MPEG authority.

8. Follow source synopsis/naming/dictionary/topology conventions for new clean source. Reconstruction may make behavior-owned source/test changes and local source dictionary changes. Foreman retains canonical generated portal/topology/test/build integration chores after handoff when appropriate.

### Explicitly out of scope

Do **not** implement in this packet:

- controller acquisition, foreground ownership, held-X review acceptance, or release quarantine;
- A005 input routing;
- RFB safe freeze/full-refresh scheduling;
- accept-to-first-frame orchestration;
- START/RETIRE sends or any real Pi/Transport mechanics;
- exact-generation preparation/retirement;
- MPEG decode/worker changes;
- Presentation/GS compositor, raster/overlay rendering, first physical frame, common-clock arm, scheduler/drop policy;
- DESKTOP CALIBRATION changes;
- hardware qualification.

### Acceptance criteria

`A004-C1 MPEG_CALIBRATION_OWNER_BOUNDARY` — pure geometry/commit source is owned by one coherent MPEG CALIBRATION component/local-cooperation boundary with a narrow owner bridge; no cross-owner internals are imported for convenience.

`A004-C2 GEOMETRY_MEANINGS` — base, inner matte, and outer/suppression footprint retain distinct audited meanings and tested invariants.

`A004-C3 SINGLE_COMMITTED_AUTHORITY` — one accepted immutable MPEG CALIBRATION geometry is the only committed authority; draft edits/cancel cannot mutate an already accepted value.

`A004-C4 DESKTOP_SEPARATION` — DESKTOP CALIBRATION remains untouched and is not used as MPEG behavior authority.

`A004-C5 CONTRACT_HOST_TESTED` — deterministic host tests prove the local owner behavior. This may support `CONTRACT_TESTED` / `HOST_TESTED`; it does not imply real Transport, cross-machine, Presentation, or hardware completion.

`A004-C6 BOUNDED_SCOPE` — no RFB/Input/Transport/Pi/Presentation lifecycle behavior is reconstructed in this packet.

### Worker handoff

Return:

- exact starting and ending commit;
- substantive source commit(s);
- files changed and owner-boundary rationale;
- criterion disposition A004-C1 through A004-C6;
- host-test command/output;
- any precise dependency labels still pending;
- exactly one immutable Reconstruction work log under `docs/ledge/work-log/` following current contract.

Do not advance to another A004 slice. Stop at handoff.

## Foreman next pickup

Consume only the A004 MPEG CALIBRATION core handoff. Reinspect A004-C1 through A004-C6 against live authority, preserve the deferred A003 item, and do not reopen autonomous Pi/Transport work. Select the next downstream owner-local packet only after the core is accepted.

PENDING_MANUAL_TRANSPORT=A003 real Pi PSTV owner; exact START semantic/prepared-generation implementation; producer/emission/retirement fence; PS2 exact completion/decoder join/drain-credit; repeated-generation same-stream proof
PENDING_CROSS_MACHINE=A003 real PS2/Pi generation transaction; downstream all-guns owner integration
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification; A004-A006 later physical/all-guns qualification
