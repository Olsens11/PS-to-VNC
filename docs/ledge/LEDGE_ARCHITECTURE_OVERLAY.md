# Ledge All-Guns Architecture Overlay

DOCUMENT=LEDGE_ARCHITECTURE_OVERLAY
DOCUMENT_REVISION=0003
RECORDED_AT=2026-09-17T07:10:44-04:00
BASED_ON_DOCUMENT_REVISION=0002
SUPERSEDES_DOCUMENT_REVISION=0002
TEMPORAL_CLASS=ARCHITECTURE_POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_LEDGE_ARCHITECTURE_AT_RECORDED_TIME
BASE_ARCHITECTURE=docs/CLEAN_ARCHITECTURE.md:ARCHITECTURE_VERSION_1
SCOPE=ledge/h1-all-guns

This overlay is the governing architecture amendment for the ledge all-guns reconstruction. `docs/CLEAN_ARCHITECTURE.md` version 1 remains the primary clean-generation architecture except where this overlay explicitly supersedes it for the ledge branch.

Revision 0003 preserves the one-physical-PSTV-stream architecture and the owner split recovered by revisions 0001-0002, but changes execution strategy for unresolved A003 Pi/Transport exact-generation mechanics. Immediate autonomous implementation of the production Pi endpoint is no longer an active Reconstruction assignment. That work is deliberately deferred to `docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md` for a later user-assisted session with direct runtime/hardware access.

This deferral changes **when** unresolved mechanism is implemented, not **who owns** the relevant responsibilities and not the accepted A003 wire/control behavior.

## A001 transport ownership reconciliation

For the ledge all-guns generation, supersede architecture-version-1 statements that make application/main the sole owner of VNC socket operations or make RFB directly own the PS2-facing socket.

Adopt one explicit **PSTV Transport owner** per endpoint with mechanism responsibilities:

- own one physical PS2-facing PSTV TCP stream/session when that real endpoint is active;
- own the sole physical receive path and receive sequencing;
- validate PSTV physical framing/version/length/sequence state;
- dispatch received payloads into transport-owned logical-channel/control state;
- serialize physical sends;
- own Transport/channel lifecycle, flow control, receiver quiescence/dormancy, and mechanism failure;
- expose narrow logical-channel/control operations without exposing raw socket ownership to consumers.

The application coordinator owns product-level startup/shutdown ordering, recovery policy, and cross-domain decisions. It requests Transport lifecycle operations; it does not become the socket mechanism owner.

## RFB ownership under shared Transport

RFB continues to own synchronized RFB protocol/session behavior: handshake/security/ServerInit, RFB receive buffering/framing, encoding negotiation, requests, server-message parsing, rectangle decoding, framebuffer truth, and pointer/key wire serialization.

Under shared PSTV, RFB consumes an explicit logical RFB byte-stream/public bridge supplied by Transport. RFB does not own or open another PS2-facing PSTV socket.

RFB also owns safe freeze/quiesce semantics and the fresh nonincremental/full framebuffer-update obligation required after restoration. Other components request/observe those facts through the RFB bridge rather than reaching into RFB parser/request internals.

## Concurrency and quiescence invariant

The all-guns concurrency rule remains:

> Exactly one Transport receive owner advances each physical PSTV stream; protocol/media/control consumers use owner-defined logical/public seams and may not independently receive from the physical socket.

A received logical frame must complete receiver dispatch before teardown may retire resources that dispatch could still touch. The historical H1 counter-equality plus bounded-delay fence is evidence of required ordering, not the required production synchronization mechanism.

## Binding component / directory / bridge invariant

The ledge branch now makes the following source-structure rule explicit and binding:

> **Inside one genuine component/local-cooperation directory:** internal implementation files may cooperate directly.
>
> **Across a real component boundary:** communication must pass through the owning component's defined bridge or public process seam.

This has several consequences:

- directory membership represents coherent responsibility/local cooperation, not call-site convenience;
- unrelated owners may not be moved into one directory merely to avoid a bridge;
- a component bridge is organized around that component's coherent owned processes, not around destinations or callers;
- destination-specific sprawl such as `input_to_rfb`, `input_to_ui`, `presentation_to_transport`, or `transport_to_everything` is prohibited unless evidence establishes a truly separate owned process rather than convenience routing;
- application may coordinate several owners, but still consumes every owner through that owner's public bridge;
- test doubles implement the same public owner boundary used by production callers;
- no component may export another owner's private socket, queue, parser, decoder-worker, compositor, or input state merely to bypass the owner bridge.

The converse is equally important: files that genuinely belong to one local cooperation boundary need not communicate through artificial per-file bridge layers. Do not create generic indirection merely to make adjacent internal modules look separated.

## Dependency-facing contracts versus implementations

A public contract means another owner may rely on an operation/fact. It does **not** mean the real underlying implementation is complete, cross-machine validated, or hardware qualified.

Downstream reconstruction may use deterministic fakes at public owner boundaries to prove consumer behavior. Such tests may support `CONTRACT_TESTED` or `HOST_TESTED`; they do not support `REAL_TRANSPORT_IMPLEMENTED`, `CROSS_MACHINE_VALIDATED`, or `HARDWARE_QUALIFIED`.

No fake production Transport, generic event bus, callback framework, or alternate receiver may be created merely to satisfy a consumer during the deferral.

## Transport public contract

Existing clean Transport bridge/session concepts remain the starting authority and should be extended only when an actual downstream consumer requires a missing owner-correct operation.

Transport owns:

### Session lifecycle

- session open;
- optional AUDIO/MPEG composition where already adopted;
- abort;
- receiver completion/dormancy;
- close;
- explicit mechanism-level terminal/failure state.

### RFB logical stream

- logical reads;
- polling/service;
- logical writes;
- quiesce/freeze-boundary operations.

### AUDIO

- logical reads;
- producer status;
- activity snapshot/wait.

### MPEG

- logical reads;
- producer status;
- activity snapshot/wait;
- publication of real finite producer completion when the real producer exists.

### Exact control mechanism

The contract may expose narrowly scoped operations required by consumers, including:

- send explicit START control;
- send RETIRE request/control;
- publish/observe inbound exact START/RETIRE control identity through the sole receiver;
- expose immutable validated Transport/session/config facts that are genuinely mechanism-owned;
- expose explicit Transport terminal/quiescence/failure facts.

The existing result vocabulary distinguishing states such as `OK`, `WOULD_BLOCK`, `STOPPED`, `EXHAUSTED`, `CLOSED`, `INVALID`, and `FAILED` remains the preferred narrow mechanism vocabulary.

Transport does **not** expose:

- raw sockets;
- receiver threads;
- queue internals;
- physical sequence implementation;
- Pi service implementation;
- send-lease internals;
- parser internals;
- generic callback/event-bus machinery;
- guessed future operations.

Transport remains a replaceable mechanism owner.

## Facts that are explicitly not Transport-owned

Messages may carry these facts over PSTV, but Transport does not own their semantics:

- `GENERATION_PREPARED`;
- `GENERATION_ACTIVE`;
- `CAPTURE_INSTALLED`;
- `SUPPRESSION_INSTALLED`;
- `PRODUCER_ADMITTED`;
- `RETIREMENT_PROVEN`;
- `FIRST_MPEG_FRAME_DECODED`;
- `FIRST_PHYSICAL_MPEG_FRAME`;
- `VISUAL_OWNERSHIP_PROMOTED`;
- `COMMON_MEDIA_EPOCH_ARMED`;
- `RFB_RESTORATION_ALLOWED`.

Those facts belong to the owner that can actually prove them.

## Exact-generation owner contract

The exact-generation/application-side owner is the authority for:

- active session identity used for exact-generation validation;
- highest/prepared generation rules;
- immutable prepared START geometry/state;
- stale/repeated/conflicting START rejection without mutation;
- exact preparation fact `GENERATION_PREPARED`;
- exact retirement request coordination;
- publication of `RETIREMENT_PROVEN` only after the real exact-generation retirement fence has succeeded.

Historical H1 `H1Session.reader()` and exact-generation application state remain forensic evidence. Revision 0002's **Pi PSTV Session** and **Pi Exact-Generation Coordinator** names remain the intended responsibility split, but revision 0003 supersedes the instruction to immediately create `pi/pstv/` autonomously. The later manual session must first inspect/adopt the actual Pi PSTV runtime/service ownership, then place the mechanism and coordinator according to genuine local cooperation boundaries.

Active Pi desktop bounds remain immutable Pi-local session metadata supplied by the Pi desktop/session authority; they are not derived from START and are not PS2 **DESKTOP CALIBRATION** state.

## MPEG owner contract

MPEG owns decoder/worker behavior and decode/upload readiness. It may expose decoded/upload-ready state through its own bridge when required by another owner.

MPEG does not own:

- PSTV socket/framing;
- exact-generation preparation or retirement proof;
- physical composition;
- visual ownership promotion;
- common-clock arm.

`FIRST_MPEG_FRAME_DECODED` is not `FIRST_PHYSICAL_MPEG_FRAME`.

## Presentation owner contract

Presentation owns:

- one physical GS/composition path;
- resolved draw geometry;
- inner matte and outer suppression rendering;
- presentation ownership state used for drawing;
- physical presentation boundary;
- common-media-epoch arm callsite;
- absolute common-clock scheduling and qualified lateness/drop policy.

Presentation alone can publish `FIRST_PHYSICAL_MPEG_FRAME` and `VISUAL_OWNERSHIP_PROMOTED` from the physical presentation boundary. CONFIG, calibration acceptance, START send, producer admission, queueing, decode, or upload do not establish those facts.

Within one real presentation component directory, compositor/render/timing files may cooperate directly. Calls to calibration, RFB, MPEG, Transport, Input/UI, or Application cross the owning bridge.

## MPEG CALIBRATION owner contract

**MPEG CALIBRATION** remains distinct from **DESKTOP CALIBRATION**.

MPEG CALIBRATION owns edit/review/commit semantics and one committed geometry containing:

- exact base rectangle used for capture/presentation;
- PS2-local inner matte;
- outer suppression footprint used to derive presentation/RFB ownership perimeter.

Acceptance emits one immutable committed geometry. Do not create a second competing geometry store or accepted-edge detector just to cross a component boundary. Presentation consumes resolved geometry through the calibration/public owner seam. Exact-generation preparation consumes only the exact generation inputs it owns; inner matte remains local Presentation data and is not Pi START geometry.

## RFB owner contract

RFB owns protocol/framebuffer truth, safe complete-message freeze/quiescence, request mechanics, and fresh-full-update obligation. `RFB_QUIESCED` and the full-refresh obligation are published through RFB's bridge.

RFB does not learn MPEG/Presentation/Transport internals. Restoration policy may be coordinated by Application using owner facts, but the RFB owner performs the actual protocol/request operation.

## Input / UI owner contract

Input/UI owns physical controller observation, semantic foreground ownership, mouse/pointer/button/wheel interpretation, keyboard/modifier serialization, OSK/local foreground, release quarantine, suspend/rebase/resume state, and cooperative dormancy/teardown.

Input/UI may publish facts such as `INPUT_DORMANT` or foreground handoff state. It does not perform RFB or Transport socket operations.

## Application owner contract

Application owns cross-owner ordering and product policy only. It may coordinate public owner facts such as:

- `TRANSPORT_SESSION_READY`;
- `TRANSPORT_RECEIVER_DORMANT`;
- `GENERATION_PREPARED`;
- `RETIREMENT_PROVEN`;
- `RFB_QUIESCED`;
- `INPUT_DORMANT`;
- `FIRST_PHYSICAL_MPEG_FRAME`.

Application must obtain each through the owning bridge. Correct Application coordination against deterministic fakes proves the consumer policy, not the deferred producer of the fake fact.

## A003 accepted/deferred boundary

Accepted coherent A003 work must be preserved:

- A003-P1 Transport MPEG logical-channel/decoder core;
- START kind `11`, control channel `0`, flags `0`;
- exact 44-byte START v1 representation;
- RETIRE kind `10`, control channel `0`, flags `0`;
- exact 12-byte RETIRE representation;
- opaque `DATA/channel 4` MPEG media;
- no magic-44-byte MPEG discrimination;
- no per-packet generation tags;
- 44-byte MPEG-media regression;
- accepted ordered PS2 START send behavior.

The durable deferred item is:

`docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md`

Its status is `A003_TRANSPORT_PI_MANUAL_COMPLETION_DEFERRED`.

The active autonomous P2A-completion packet from Foreman state 0015 is superseded. Reconstruction is not authorized to create/adopt a production `pi/pstv/` runtime, implement real Pi exact-generation mechanics, or advance P2B/P2C/P2D merely to remove the pending labels.

A003 remains incomplete until the deferred manual item is completed.

## Downstream reconstruction release

The Transport/Pi deferral is not a project-wide stop. A downstream criterion may become source/contract/host/build complete when its own local behavior is complete, while retaining precise external dependency labels.

### A004 — Presentation / MPEG CALIBRATION

A004 may proceed with owner-local behavior against contracts, including:

- MPEG CALIBRATION edit/review/commit behavior;
- base rectangle, inner matte, outer suppression footprint;
- immutable accepted geometry;
- foreground calibration semantics;
- RFB safe freeze/full-refresh policy;
- accept-to-first-frame protection;
- one physical Presentation/GS owner;
- compositor ordering;
- first physical MPEG frame promotion;
- common-clock arm callsite;
- scheduler/deadline/drop policy;
- local overlays;
- restoration conditioned on abstract `RETIREMENT_PROVEN`.

A004 must not implement the Pi mechanism that produces `RETIREMENT_PROVEN`.

### A005 — Interaction / Input

A005 may proceed broadly with input polling/continuity, mouse interpretation, pointer/buttons/wheel, keyboard/modifier serialization, OSK/local foreground, release quarantine, suspend -> neutralize -> rebase -> release -> resume, calibration foreground handoff, parser-safe publication, and cooperative teardown. Every cross-owner call uses that owner's bridge.

### A006 — Application orchestration

A006 may proceed with resident startup, session admission, CONFIG/profile consumption, component startup dependencies, steady-state coordination, failure convergence, terminal result aggregation, reverse teardown, and repeated-session structure against public owner contracts.

A006 may not reach into Transport, MPEG, Presentation, RFB, Input/UI, or deferred Pi exact-generation internals. Its host tests may inject public owner facts but must label real-owner dependencies as pending.

## Acceptance and evidence discipline

Permitted downstream status vocabulary includes:

- `SOURCE_COMPLETE`;
- `CONTRACT_COMPLETE`;
- `CONTRACT_TESTED`;
- `HOST_TESTED`;
- `PS2_COMPILE_PASS`;
- `PS2_LINK_PASS`;
- precise external pending labels such as `PENDING_MANUAL_TRANSPORT`, `PENDING_CROSS_MACHINE`, and `HARDWARE_PENDING` followed by the exact unproven owner fact.

Do not use a generic pending label to hide incomplete local behavior. Conversely, do not fail a locally complete criterion solely because an unrelated external owner remains deliberately deferred.

## Deferred A003 completion and hardware gate

The required manual sequence and exact outstanding obligations are governed by `LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md`. That item must close before A003 can be declared complete, before real all-guns exact-generation integration can be claimed, and before final PS2/Pi hardware qualification can close.

Independent Validation remains a separate authority. No architecture state, host fake, compile/link pass, or Foreman disposition constitutes independent Validation PASS or physical qualification.
