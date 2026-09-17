# Foreman shift — defer A003 Transport/Pi mechanics and release downstream reconstruction

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-17T07:09:43-04:00
COMPLETED_AT=2026-09-17T07:44:30-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=35ef6d7f63634c0ad267de17985a766a9bc91e09
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Replan the ledge so unresolved real A003 Transport/Pi exact-generation mechanics become an explicit later user-assisted manual completion item instead of blocking owner-local A004-A006 reconstruction. Preserve accepted A003 source, freeze dependency-facing owner contracts and component-boundary rules, supersede the autonomous P2A-completion packet, and issue exactly one downstream Reconstruction packet that does not require fake or unresolved production Transport behavior.

## Authority consumed

The round began from live branch authority `35ef6d7f63634c0ad267de17985a766a9bc91e09` and consumed:

- current `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0015;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision 0002 at round start;
- `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision 0001;
- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision 0001;
- `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` revision 0001;
- A005/A006 audit authority for downstream owner-contract planning;
- current clean source topology and existing Transport/RFB/MPEG/Input/Application ownership shape;
- work-log contract revision 0006.

Repository authority was re-read before each substantive write.

## A003 accepted work preserved

No product rollback was performed. A003-P1 remains accepted. The coherent P2A partial source remains preserved, including:

- START explicit control kind 11 / control channel 0 / flags 0;
- exact 44-byte START v1 wire representation;
- RETIRE explicit control kind 10 / control channel 0 / flags 0;
- exact 12-byte big-endian RETIRE `(version, session_id, generation)` representation;
- MPEG media remains opaque `DATA/channel 4`;
- no magic-44-byte payload discrimination;
- no per-MPEG-packet generation tags;
- deterministic 44-byte MPEG-media regression;
- accepted PS2 outbound START framing through the existing ordered Transport send path;
- no generation business state moved into PS2 Transport or MPEG decoder ownership.

The retained P2A disposition is:

- `P2A-1 START_CONTROL_IDENTITY = MET`;
- `P2A-2 START_SEMANTICS = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`;
- `P2A-3 RETIRE_WIRE_IDENTITY = MET`;
- `P2A-4 PURE_MPEG_DATA = MET`;
- `P2A-5 SOLE_TRANSPORT_OWNER = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`;
- `P2A-6 OWNERSHIP = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`;
- `P2A-7 NO_GENERATION_TAGGING = MET`;
- `P2A-8 BOUNDED_SCOPE = MET`.

A003 is not declared complete.

## Durable manual completion item

Created:

`docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md`

commit:

`8fc78235061e9e0d7a95b82531c0324bf05a140a` — `docs(a003): defer Transport Pi completion to manual session`

Status:

`A003_TRANSPORT_PI_MANUAL_COMPLETION_DEFERRED`

This item explicitly defers the real production Pi PSTV/session/service adoption and proof sequence to a later interactive engineering session with the user present at the Pi/PS2/network/runtime.

Exact deferred obligations include:

- real Pi exact START semantic validation and immutable prepared-generation implementation;
- real one-session/sole-receiver Pi PSTV ownership;
- owner-correct PS2 inbound exact-control publication completion;
- real Pi producer preparation/emission/admission lifecycle;
- in-flight lease retirement fencing;
- cleanup-before-RETIRE-completion proof;
- PS2 exact completion reception;
- decoder stop/join without early resource reclamation;
- residual MPEG queue discard and Transport credit return;
- generation-N to generation-N+1 same-stream contamination proof;
- repeated-generation and finite cancel/error/EOF/retirement behavior;
- real downstream all-guns integration;
- cross-machine and hardware qualification.

The document also records the later manual sequence from actual Pi runtime/service ownership inspection through direct physical qualification.

## Architecture / dependency contract revision

Updated `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` to revision 0003:

`099dec22c71544c0980dae1769d19b650bb71973` — `docs(architecture): defer Pi mechanics and freeze owner contracts`

Revision 0003 preserves the useful ownership decisions from revision 0002 while explicitly removing any autonomous requirement to create/adopt `pi/pstv/` merely because that namespace had been named.

Binding invariant:

> Within one genuine component/local-cooperation directory, internal implementation files may cooperate directly. Across a real component boundary, communication must pass through the owning component's defined bridge or public process seam.

Consequences recorded in architecture/state:

- directory membership reflects real coherent ownership, not call-site convenience;
- unrelated owners are not merged merely to avoid bridge crossings;
- no bridge-per-caller or bridge-per-destination sprawl;
- Application coordinates owners only through their public bridges;
- test doubles implement the same public owner boundary consumed by production callers;
- owner-private socket/thread/queue/parser/worker/compositor/input state stays private.

Frozen owner contracts distinguish:

- Transport mechanism/session/logical-channel/control/quiescence/failure facts;
- exact-generation prepared/retirement-proof semantics;
- MPEG decoder/worker facts;
- RFB protocol/framebuffer/freeze/full-refresh facts;
- Presentation physical-composition/first-physical-frame/visual-ownership/common-clock facts;
- Input/UI physical-observation/foreground/quarantine/publication facts;
- Application cross-owner ordering/policy only.

Contract fakes may establish `CONTRACT_TESTED` / `HOST_TESTED` consumer behavior but do not establish `REAL_TRANSPORT_IMPLEMENTED`, `CROSS_MACHINE_VALIDATED`, or `HARDWARE_QUALIFIED`.

## Foreman state and downstream release

Updated `docs/ledge/LEDGE_FOREMAN_STATE.md` to revision 0016:

`64a1bf103013c403323e0b17cc08dc68e9c4744d` — `docs(foreman): defer A003 Pi mechanics and issue A004 calibration core`

Current phase:

`A003_TRANSPORT_PI_MANUAL_DEFERRED__A004_MPEG_CALIBRATION_CORE_PACKET_ISSUED`

State 0016 explicitly supersedes the autonomous A003-P2A completion implementation packet from state 0015. No worker is currently authorized to solve the real Pi/Transport lifecycle autonomously.

A004-A006 are replanned around owner contracts so local criteria may become source/contract/host/build complete while carrying exact external dependency labels.

## One downstream packet issued

Exactly one downstream Reconstruction packet was issued: **A004 MPEG CALIBRATION geometry/commit core**.

It is deliberately limited to owner-local pure calibration state/geometry behavior that can be reconstructed from the A004 audit and frozen H1 `mpeg_presentation_calibration/` evidence without real Pi/Transport mechanics.

The packet requires:

- a coherent MPEG CALIBRATION owner/local-cooperation boundary;
- distinct base rectangle, inner matte, and outer/suppression-footprint meanings;
- audited minimum/alignment/clamping/center-preserving geometry rules;
- one draft/default/committed owner model;
- one immutable accepted MPEG CALIBRATION geometry authority;
- a narrow owner bridge for semantic geometry operations/query/commit/cancel/derived suppression;
- deterministic host tests;
- preservation of acceptance versus first-physical-presentation separation.

Explicitly out of scope are Input foreground/controller semantics, RFB freeze/full-refresh, real START/RETIRE or Pi mechanics, exact-generation lifecycle, MPEG worker changes, Presentation/GS composition, first physical frame, clock arm, scheduler/drop policy, DESKTOP CALIBRATION changes, and hardware qualification.

The packet carries the binding calibration separation invariant: DESKTOP CALIBRATION and MPEG CALIBRATION are separate historical systems with separate authority. MPEG behavior must be traced to `experiments/media-harness-h1/mpeg_presentation_calibration/` plus A004 audit authority, never inferred from the older DESKTOP CALIBRATION implementation merely because both manipulate rectangles.

Acceptance criteria issued are `A004-C1` through `A004-C6` as recorded in Foreman state revision 0016.

## Documentation discoverability

Added the new durable manual-completion authority to `docs/INDEX.md`:

`2e3c8b1849548096bed8d7b3e44e5dd419779937` — `docs(a003): index deferred Transport Pi completion authority`

That replacement accidentally changed one unrelated historical M1C index description. The drift was immediately detected and restored without altering the intended A003 entry:

`c82fc084abfb7e3cd2f6727c18dc51cb66e7bf1d` — `docs(index): restore M1C characterization wording`

The net index delta is therefore only the new A003 manual-completion discovery entry.

## Evidence/status discipline

No product source behavior was implemented by Foreman in this round. No fake production Transport was created. No A003 completion, independent Validation PASS, cross-machine success, or hardware qualification is claimed.

The last settled canonical product/checker evidence from the prior Foreman authority remains separate from these documentation/strategy changes. This round does not reinterpret contract-host evidence as real endpoint evidence.

Current explicit dependency labels are:

`PENDING_MANUAL_TRANSPORT` = real A003 Pi PSTV/session owner, exact START prepared-generation semantics, producer/emission/retirement fence, PS2 exact completion/decoder join/drain-credit, repeated-generation same-stream proof.

`PENDING_CROSS_MACHINE` = real PS2/Pi exact-generation transaction and later all-guns owner integration.

`HARDWARE_PENDING` = physical A001/A002/A003 qualification plus later A004-A006/all-guns qualification.

## Exact next pickup

Consume only the A004 MPEG CALIBRATION core Reconstruction handoff against live repository authority. Reinspect `A004-C1` through `A004-C6`, preserve `LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md` as deferred authority, and do not reopen autonomous Pi/Transport work. Select the next owner-local downstream slice only after the MPEG CALIBRATION core is accepted.
