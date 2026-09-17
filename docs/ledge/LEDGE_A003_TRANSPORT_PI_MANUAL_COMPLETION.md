# Ledge A003 — Deferred Manual Transport / Pi Completion

DOCUMENT=LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-17T07:09:43-04:00
SOURCE_COMMIT=SELF
BASED_ON_A003_AUDIT_REVISION=0001
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0002
BASED_ON_FOREMAN_STATE_REVISION=0015
TEMPORAL_CLASS=DEFERRED_WORK_ITEM
TEMPORAL_SEMANTICS=GOVERNING_UNTIL_SUPERSEDED
STATUS=A003_TRANSPORT_PI_MANUAL_COMPLETION_DEFERRED

This document makes the unresolved A003 Transport/Pi exact-generation implementation a deliberate final manual software item rather than an autonomous-Reconstruction blocker for otherwise independent A004-A006 work.

It does **not** declare A003 complete. It preserves already-accepted A003 behavior, freezes the dependency-facing facts that downstream owners may rely on, and records exactly what remains unproven until a later interactive engineering session with the user present at the real Pi, PS2, network, services, worktree, and runtime.

## Accepted A003 behavior preserved

The following accepted source behavior remains authoritative and must not be rolled back merely because the remaining lifecycle is deferred:

- A003-P1 Transport MPEG logical-channel and decoder-owner core previously accepted by Foreman;
- explicit START control identity: kind `11`, control channel `0`, flags `0`;
- exact 44-byte START v1 wire representation;
- explicit RETIRE control identity: kind `10`, control channel `0`, flags `0`;
- exact 12-byte big-endian RETIRE `(version, session_id, generation)` representation;
- MPEG media remains opaque `DATA/channel 4`;
- no magic-44-byte payload classification;
- no per-MPEG-packet generation tags;
- deterministic regression proving START-shaped, exactly 44-byte `DATA/channel 4` remains ordinary MPEG media;
- accepted PS2 outbound START framing through the existing ordered physical Transport send path;
- no generation business state moved into Transport or the MPEG decoder merely to complete P2A.

The Foreman P2A disposition remains:

- `P2A-1 START_CONTROL_IDENTITY = MET`;
- `P2A-2 START_SEMANTICS = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`;
- `P2A-3 RETIRE_WIRE_IDENTITY = MET`;
- `P2A-4 PURE_MPEG_DATA = MET`;
- `P2A-5 SOLE_TRANSPORT_OWNER = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`;
- `P2A-6 OWNERSHIP = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`;
- `P2A-7 NO_GENERATION_TAGGING = MET`;
- `P2A-8 BOUNDED_SCOPE = MET`.

No autonomous worker is authorized by this document to convert the three partial criteria to `MET` by inventing a substitute Pi runtime.

## Why this item is deferred

The historical H1 implementation proves the required transaction semantics, but the final production adoption of the Pi-side PSTV runtime/service owner must be reconciled against the actual Pi, networking, service topology, launch behavior, and cross-machine runtime. Those mechanics are a local unresolved implementation problem, not a reason to prevent other components from reconstructing behavior that depends only on stable owner facts.

The previously named `pi/pstv/` boundary remains useful architectural vocabulary for the Pi PSTV-session responsibility, but architecture revision 0003 supersedes any interpretation that its immediate creation is an autonomous Reconstruction assignment. The manual session must first inspect/adopt the actual production Pi runtime/service ownership and then place code according to genuine ownership rather than a preselected path.

## Binding component / directory / bridge invariant

The following rule governs all downstream reconstruction and this later manual completion:

> Within one genuine component/local-cooperation directory, internal implementation files may cooperate directly. Across a real component boundary, communication must pass through the owning component's defined bridge or public process seam.

Consequences:

- directory membership reflects real ownership and local cooperation, not caller convenience;
- unrelated responsibilities may not be moved into one directory merely to avoid a bridge;
- a component exposes one coherent process-organized bridge rather than destination-specific bridge sprawl;
- application may coordinate several owners, but it consumes each owner's public bridge and never reaches into private implementation;
- test doubles implement the same public owner boundary used by production callers;
- private socket, thread, queue, parser, decoder-worker, compositor, or input state may not be exported merely to make orchestration easier.

## Frozen dependency-facing owner contracts

These contracts define facts another component may rely on. They do **not** assert that every underlying real implementation currently exists or has been cross-machine qualified.

### Transport owner contract

Transport owns mechanism:

- session open/abort/receiver-completion-or-dormancy/close;
- one physical PSTV stream and sole receive ownership per endpoint when the real endpoint is active;
- physical framing, ordered receive/send, logical-channel dispatch, flow control, quiescence, and mechanism failure;
- logical RFB reads/service/writes and quiesce/freeze boundary operations;
- AUDIO logical reads, producer status, activity snapshot/wait;
- MPEG logical reads, producer status, activity snapshot/wait, and real finite producer-completion publication;
- exact START send and RETIRE request/send when those operations are implemented behind the bridge;
- inbound exact control identity publication through the sole receiver when implemented;
- immutable validated Transport/session/config facts that are genuinely mechanism-owned;
- explicit terminal/quiescence/failure facts.

Existing result vocabulary (`OK`, `WOULD_BLOCK`, `STOPPED`, `EXHAUSTED`, `CLOSED`, `INVALID`, `FAILED`) remains the starting authority. Add only narrow operations required by an actual consumer.

Transport does **not** expose or delegate raw sockets, receiver threads, queue internals, physical sequence implementation, Pi service implementation, send-lease internals, parser internals, generic callback/event-bus machinery, or guessed future operations.

Transport also does **not** own generation, presentation, RFB, or input business facts merely because their messages pass over PSTV.

### Exact-generation owner contract

The exact-generation owner is the authority for:

- exact active session identity used for generation validation;
- nonzero/monotonic generation rules;
- one immutable prepared START geometry/state;
- stale/repeated/conflicting START rejection without mutation;
- exact-generation preparation result `GENERATION_PREPARED`;
- retirement request coordination;
- exact retirement proof `RETIREMENT_PROVEN` only after the real owner-specific fence has succeeded.

`GENERATION_PREPARED` and `RETIREMENT_PROVEN` are **not** Transport facts. A downstream contract fake may emit them for consumer tests, but that does not prove the real Pi implementation.

### MPEG owner contract

MPEG owns decoder/worker behavior and upload/decode readiness. It may expose owner facts such as decoder/worker readiness and first decoded/upload-ready state where required by a consumer. It does not own PSTV framing, exact-generation retirement proof, physical presentation, or common-clock arm.

`FIRST_MPEG_FRAME_DECODED` is MPEG-local evidence and is not equivalent to `FIRST_PHYSICAL_MPEG_FRAME`.

### RFB owner contract

RFB owns protocol/framebuffer truth, synchronized parser/session state, safe freeze/quiesce behavior, request mechanics, and the one fresh nonincremental/full-update obligation required after restoration.

RFB may publish facts such as `RFB_QUIESCED` and its fresh-full-update obligation through its bridge. It does not inspect MPEG, exact-generation, Presentation, or Transport private state to decide those facts.

### Presentation owner contract

Presentation owns the one physical composition/GS path, resolved draw/matte/suppression geometry, physical presentation state, and presentation timing policy.

Presentation alone publishes `FIRST_PHYSICAL_MPEG_FRAME` and `VISUAL_OWNERSHIP_PROMOTED` from the real physical-presentation boundary. The common media clock arm callsite belongs at that physical boundary; `COMMON_MEDIA_EPOCH_ARMED` is not a Transport or decode fact.

Presentation may consume abstract exact-generation retirement proof for restoration decisions, but it does not manufacture that proof.

### Input / UI owner contract

Input/UI owns physical observation, semantic foreground ownership, pointer/button/wheel and keyboard/modifier interpretation/publication, local UI/OSK foreground behavior, release quarantine, and cooperative dormancy/teardown.

It publishes only its own facts, such as `INPUT_DORMANT` or foreground handoff state, through its bridge. It does not perform RFB or Transport socket operations.

### Application owner contract

Application owns cross-owner ordering and policy only. It may coordinate abstract facts such as:

- `TRANSPORT_SESSION_READY`;
- `TRANSPORT_RECEIVER_DORMANT`;
- `GENERATION_PREPARED`;
- `RETIREMENT_PROVEN`;
- `RFB_QUIESCED`;
- `INPUT_DORMANT`;
- `FIRST_PHYSICAL_MPEG_FRAME`.

It must obtain each fact through the actual owner's bridge. Correct application behavior against a test double is not evidence that the deferred owner has implemented the fact correctly.

## Contract-test evidence discipline

Deterministic fakes are authorized only at public owner boundaries so downstream consumer behavior can be reconstructed now.

Valid evidence labels include:

- `SOURCE_COMPLETE`;
- `CONTRACT_COMPLETE`;
- `CONTRACT_TESTED`;
- `HOST_TESTED`;
- `PS2_COMPILE_PASS`;
- `PS2_LINK_PASS`.

They do not imply:

- `REAL_TRANSPORT_IMPLEMENTED`;
- `CROSS_MACHINE_VALIDATED`;
- `HARDWARE_QUALIFIED`.

Pending labels must name the precise unproven owner fact. Do not use a generic `Transport unfinished` label when, for example, the actual dependency is `PENDING_MANUAL_TRANSPORT: exact Pi RETIRE cleanup-before-completion proof`.

## Downstream work released from the Transport/Pi blocker

### A004 — Presentation / MPEG CALIBRATION

Owner-local A004 behavior may proceed against the frozen contracts, including MPEG CALIBRATION edit/commit behavior, base/inner/outer geometry, accepted immutable geometry, foreground semantics, RFB freeze/full-refresh policy, accept-to-first-frame protection, single physical presentation ownership, compositor ordering, first-physical-frame promotion, common-clock arm callsite, scheduler/drop policy, overlays, and restoration conditioned on abstract `RETIREMENT_PROVEN`.

The Pi mechanism that produces `RETIREMENT_PROVEN` remains deferred. **DESKTOP CALIBRATION** and **MPEG CALIBRATION** remain distinct systems.

### A005 — Interaction / Input

Owner-local A005 behavior may proceed broadly: physical input polling/continuity, mouse interpretation, pointer/buttons/wheel, keyboard/modifier serialization, OSK/local foreground, release quarantine, suspend -> neutralize -> rebase -> release -> resume, calibration foreground handoff, parser-safe publication, and cooperative teardown. Cross-component interactions use the owning component bridge.

### A006 — Application orchestration

A006 may reconstruct resident startup structure, session admission, CONFIG/profile consumption, component dependency ordering, steady-state coordination, failure convergence, terminal result aggregation, reverse teardown, and repeated-session structure against public owner contracts and deterministic fakes.

Application correctness does not erase any deferred real-owner gap.

## Deferred manual completion sequence

The later interactive user-assisted session must complete and prove, in order:

1. inspect and adopt the actual Pi PSTV runtime/service ownership;
2. make one Pi PSTV session owner real;
3. prove sole Pi receive ownership;
4. prove explicit START receive through that owner;
5. prove START semantic validation and immutable prepared generation;
6. prove exact-generation producer preparation;
7. prove MPEG emission;
8. prove emission admission closes at retirement;
9. deliberately exercise an in-flight emission lease during retirement;
10. prove exact producer cleanup completes before RETIRE completion is emitted;
11. prove PS2 exact RETIRE completion reception through the sole PS2 receiver/public seam;
12. prove PS2 decoder stop/join without reclaiming worker-visible resources early;
13. prove residual MPEG queue discard and corresponding Transport credit return;
14. prove generation N cannot contaminate generation N+1 across the ordered same-stream fence;
15. prove repeated generations and finite cancel/error/EOF/retirement behavior;
16. integrate the already-reconstructed A004-A006 owners through their public bridges;
17. proceed directly into real PS2/Pi cross-machine and hardware qualification.

## Exact obligations that remain deferred

At minimum, the following remain explicitly unproven until the manual session:

- `P2A-2`: real Pi exact START semantic validation/prepared-generation implementation;
- `P2A-5`: real production Pi sole PSTV session/receive ownership and corresponding PS2 inbound exact-control publication completion;
- `P2A-6`: real endpoint owner split wired through production bridges rather than contract fakes;
- P2B real Pi producer preparation/emission/admission behavior;
- P2C/P2D real retirement request/completion, decoder stop/join, residual drain/credit, successor fence, and repeated-generation lifecycle;
- real `RETIREMENT_PROVEN` production path;
- cross-machine behavior and failure convergence;
- physical PS2/Pi qualification and all-guns endurance.

These pending obligations do not block a downstream criterion whose own owner-local behavior can be fully reconstructed and tested against the frozen contract.

## Completion gate

This deferred item must be completed before A003 can be declared complete, before all-guns integration can claim the real exact-generation transaction, and before hardware qualification can close. Downstream source/contract/host/build completion may precede it, but must retain exact dependency labels until the real owner facts are proven.
