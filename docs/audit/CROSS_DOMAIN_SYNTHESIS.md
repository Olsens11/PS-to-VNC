# PS-to-VNC Cross-Domain Ownership and Architecture Synthesis

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_6
    INPUT_BEHAVIOR_FAMILIES=B01_B14
    INPUT_MATURITY=EVIDENCE_SUPPORTED
    SYNTHESIS_STATE=ACTIVE
    REBUILD_READY=NO

This document derives the clean reconstruction architecture from the completed
B01-B14 behavioral inventory.

It deliberately does **not** adopt the exploratory B4A layout or the successor
M4 normalization layout as the design. Those remain behavioral/reference and
historical architecture inputs.

The rule for this phase is:

> choose the smallest set of explicit owners and interfaces that makes the
> audited behavior easier to understand than the historical implementation.

The project intent remains controlling:

> do not trade a legible monolith for a well-organized pile of gibberish.

Therefore this document defines responsibilities, state owners, dependency
direction, concurrency boundaries, and lifecycle. It does not require one source
file per bullet point. File splits are earned by implementation coherence.

---

## 1. What survives from the old M4 architecture work

The earlier `ARCHITECTURE.md`, `TARGET_ARCHITECTURE.md`, `STATE_OWNERSHIP.md`, and
`DEPENDENCY_RULES.md` contain principles that remain strongly supported by the
semantic audit:

- one clear owner for important mutable state;
- application orchestration above domain mechanisms;
- RFB protocol and PS2 presentation are different responsibilities;
- policy and mechanism remain distinguishable;
- input emits intent rather than owning remote effects;
- UI does not own sockets or durable transactions;
- configuration does not execute product behavior while parsing;
- management transport does not own display policy;
- diagnostics observes but does not become a product-state owner;
- platform-specific PS2 mechanisms remain recognizable;
- host-testable pure logic should stay host-testable;
- new dependency cycles are not accepted as normal;
- broad writable global state is not a target architecture.

What does **not** automatically survive is the preselected fine-grained file
hierarchy. The old target proposed separate files for startup, recovery,
display-transition, RFB transport/session/resize/Hextile/input, numerous video
subdomains, several menu modules, and several management layers before the
current B01-B14 audit existed.

Those names are useful hypotheses, not obligations.

---

## 2. Clean PS2-side responsibility map

The clean runtime needs the following coherent owners. These are **domains**, not
promises that each domain immediately becomes a directory full of tiny files.

### 2.1 Application coordinator

Owns product-level orchestration and lifecycle policy.

It owns:

- application phase/lifecycle;
- startup ordering and shutdown routing;
- the main event/update/presentation loop;
- routing of semantic input/UI actions to their destination;
- the small cross-thread control mailbox used to publish urgent intent;
- manual Refresh policy and cooldown state;
- risky display-transition/confirmation/rollback policy;
- startup reconciliation of durable display-transaction authority;
- decisions about which subsystem failure is recoverable, transactional, or
  terminal.

It does **not** own:

- RFB parser/socket internals;
- framebuffer pixels;
- GS resources;
- controller hardware state;
- UI navigation state;
- configuration grammar;
- management HTTP framing;
- diagnostic transport.

The historical `app/startup`, `app/display_transition`, and `app/recovery`
responsibilities remain semantically real, but they should initially be viewed
as cohesive application-policy regions. They are split into separate source
files only when that makes control flow easier to follow.

### 2.2 RFB client/session

Owns one VNC/RFB protocol session and its exact protocol state.

It owns:

- TCP socket/session validity;
- RFB 3.8 handshake/security negotiation;
- ServerInit parsing and geometry acceptance;
- receive buffer/cursors and exact message-boundary state;
- negotiated pixel format/encodings/capabilities;
- FramebufferUpdate request state;
- server-message and rectangle framing;
- Raw/Hextile decoding mechanics;
- ExtendedDesktopSize protocol mechanics;
- serialization of remote pointer/key events;
- complete-session replacement mechanism;
- protocol-level failure classification.

It does not own product recovery policy, display authority, local UI, controller
state, or GS presentation.

`Hextile` and desktop-resize implementation may remain private helpers inside
this domain unless their size/testing needs justify independent modules.

### 2.3 Remote desktop framebuffer

Owns the authoritative CPU-side image of the remote desktop used to connect RFB
decode to local presentation.

It owns:

- pixel storage/capacity;
- current logical framebuffer dimensions;
- representation validity/staleness;
- dirty-region/update bounds;
- any future representation-generation/version state needed to prove that an
  incremental update has a valid base.

RFB writes decoded pixels into this object. Presentation reads coherent pixels
from it. Neither side reaches through the other domain's private state.

This deliberately creates a neutral product concept rather than making RFB
depend on GS internals or forcing a generalized callback framework between RFB
and video.

The initial clean reconstruction should keep one authoritative conventional EE
framebuffer. Historical direct-to-presentation optimizations are performance
work and are deferred until the ordinary path is stable. If later restored,
representation validity must remain explicit.

### 2.4 Display model and presentation

Owns the product meaning of the local display and the mechanism that presents a
coherent remote desktop on PS2 hardware.

The **display model** owns stable value concepts:

- mode catalog and mode identity;
- backend identity/compatibility;
- physical raster/timing properties;
- safe-area calibration values;
- logical desktop geometry derived for a profile;
- presented output rectangle;
- complete `display_profile` value used by startup, transitions, management,
  rollback, and persistence validation.

The **active display/presentation state** owns:

- current active display profile;
- GS/presentation resources;
- Standard/HIRES backend resources;
- presentation-buffer validity;
- draw/finish/flip/memory-idle mechanism;
- PS2-specific HIRES mapping state where it is truly presentation-owned.

The display domain does not decide whether a candidate should be kept or rolled
back. The application coordinator owns that policy.

The old `mode`, `geometry`, `calibration`, `mapping`, and `display` file split is
not assumed. Pure mode/profile/geometry logic can remain together until size or
testability justifies a split; hardware presentation is a natural distinct
implementation boundary.

### 2.5 Input/controller

Owns physical controller interpretation and live binding recognition.

It owns:

- controller thread/polling lifecycle;
- current physical pad state;
- libpad pause/acquire/ack/release state;
- pointer position and fractional/acceleration state;
- D-pad/analog/button/scroll interpretation;
- chord/hold/release/settle recognition;
- runtime binding recognizer state;
- physical-release quarantine mechanism;
- a bounded queue/mailbox of semantic input events for the main owner.

It emits intent; it does not write an RFB socket or execute product-level
recovery/display policy.

A key clean adaptation is to stop generating pre-encoded RFB messages in the
controller thread. The controller produces semantic pointer/key/action events;
the main/RFB owner serializes them. This preserves the proven **main thread is
the sole RFB writer** invariant while reducing protocol coupling in input.

The human-readable representation of bindings is configuration-owned. Input
owns only the live compiled/recognized binding state.

### 2.6 Local UI

Owns PS2-local foreground interaction and its visual model.

It owns:

- current foreground/modal UI state;
- System/Display Settings navigation and selection;
- OSK page/selection/one-shot modifier state;
- confirmation/restored-ack presentation state supplied by application policy;
- underlay/return relationships;
- local UI bitmap/surface generation and dirty/generation state.

It produces semantic actions; it does not execute RFB, management, or display
transactions.

The ordinary local-UI "held button must not leak after close" behavior uses the
input subsystem's quarantine mechanism. UI decides that a physical control was
consumed; input owns the physical suppression-until-release state.

The clean main/presentation path must have an explicit local-UI dirty/wake path.
The historical one-pixel remote pointer jiggle used to wake a blocked render
loop is not adopted as architecture.

System menu, display menu, overlays, and OSK may initially live in one cohesive
UI domain. OSK can remain a private substantial component or split later if its
independent size/testing warrants it.

### 2.7 Configuration

Owns the typed human-editable desired configuration model.

It owns:

- document grammar and text helpers;
- typed validation;
- defaults/fallback interpretation;
- per-mode safe-area settings;
- startup-mode desired setting;
- display lock/hide desired policy;
- human-readable controller/hotkey binding representation;
- serialization rules where a full config document/text representation is
  actually needed.

Parsing is side-effect free. Configuration values become live behavior only
when the appropriate owner/application accepts and applies them.

Configuration does not own management transport or crash-safe display
transaction state.

### 2.8 Management client

Owns the deliberately small PS2-side protocol used to communicate with the Pi
management service.

It owns:

- bounded HTTP/1.0 request/response framing;
- connection-local management transport state;
- semantic GET/POST operations for configuration, persistence, display policy,
  and durable display transaction state;
- strict parse/serialization of durable transaction wire data;
- best-effort development/test-control operations where those remain part of
  the test apparatus.

It does not decide product policy merely because it carries the message.

The clean implementation should start as one small client with private helpers.
Separate `http`, `client`, `display_transaction`, and `display_policy` modules
are created only if real implementation complexity makes that clearer.

### 2.9 Diagnostics and runtime identity

Owns optional observability, not product behavior.

It owns:

- diagnostic UDP transport/stage state;
- structured DBG/PRF/GEOM report serialization;
- profiling counters/timers that are genuinely diagnostic;
- exact runtime identity emission.

Identity may remain a distinct implementation unit because its linker-stamped
blob and `sendto()` wrapper are genuinely specialized and independently
qualified.

Other domains supply snapshots/values to diagnostics; diagnostics does not reach
back into them to make policy decisions.

### 2.10 PS2 platform mechanisms

Owns PS2SDK/hardware mechanisms whose lifecycle or hardware invariants deserve a
real boundary.

At minimum the clean runtime has recognizable platform responsibilities for:

- IOP/module/system bootstrap and OSDSYS exit mechanism;
- PS2 network-interface initialization/link status;
- libpad hardware access;
- GS/dmaKit/Standard/HIRES hardware operations and Test14 interrupt discipline;
- platform timing/cache primitives where an owner benefits from the seam.

Do not wrap every libc/BSD socket/PS2SDK function for appearance. A wrapper is
introduced when hardware ownership, lifecycle, host testing, or duplication
justifies it.

---

## 3. Core value types and passed objects

A small number of stable values should cross subsystem boundaries instead of
shared component globals.

### `display_profile`

A complete display profile is a first-class value containing enough information
to identify/verify the active or saved display configuration:

- mode identity;
- backend identity;
- logical desktop width/height;
- presented output x/y/width/height;
- safe-area/calibration state where needed for complete restoration or a stable
  reference to the validated calibration that derives those fields.

The exact C struct can be refined during implementation, but callers must not
reconstruct a "profile" by independently reading unrelated globals.

### `desktop_framebuffer`

Passed by reference to RFB decode and presentation. It contains the authoritative
CPU image plus explicit validity/dimensions/dirty state.

### semantic input events/actions

The controller/UI boundary should use typed values for:

- pointer movement/button state;
- scroll;
- key actions;
- UI navigation/activation;
- product commands such as Refresh, Display Settings, Exit, and other configured
  actions.

A configured gesture maps to a semantic action identifier; input does not own
what executing that action means.

### management transaction values

Management accepts stable domain values such as `display_profile` rather than
reading display globals. It serializes/deserializes wire data but does not own
whether a transition is currently allowed.

---

## 4. Concurrency and ownership model

The historical design has one essential concurrency fact worth preserving:
controller input must remain available while the main thread may be waiting on
RFB, but the controller thread must not become a second RFB/display owner.

### Main/application thread

The main thread is the owner of product side effects that cross major domains:

- RFB socket writes/requests;
- RFB receive/update orchestration;
- display transitions/reconstruction;
- management transaction sequencing;
- local UI foreground routing;
- application shutdown;
- recovery execution.

### Controller thread

The controller thread owns only controller/pointer/binding state and publishes
semantic events/urgent intent.

It never manipulates GS or performs RFB socket writes.

### Control mailbox / receive-interrupt seam

A small thread-safe mailbox bridges urgent controller intent to main-thread
policy. It must support the historical requirement that Refresh/rollback/exit
can become visible while the main thread is inside an RFB receive wait.

The mailbox contains **intent**, not subsystem internals. An urgent flag may be
passed/read by the RFB receive loop solely as a generic "return control to the
caller" condition. The application then reads the semantic reason and executes
the appropriate policy.

The exact implementation can be a small volatile/atomic state plus event queue;
it does not require a callback framework.

Ordinary pointer/key events do not automatically invalidate an in-progress RFB
receive. They can queue until the main writer drains them at a safe point.

### libpad ownership

Application-owned hazardous operations request controller/pad ownership through
an input API. Input acknowledges immediately before its next hardware access,
per the proven F8J2/B4A contract, and resumes only after release with stale
derived state invalidated.

The application can separately discard queued semantic remote-input events at a
hard display transition boundary before reconstructing the destination.

---

## 5. Dependency direction

The intended dependency graph is direct and mostly downward:

    application coordinator
        -> input
        -> ui
        -> config
        -> management
        -> rfb
        -> desktop framebuffer
        -> display
        -> diagnostics
        -> platform/ps2 startup/network as needed

    rfb
        -> desktop framebuffer
        -> diagnostics
        -> ordinary socket/libc/PS2IP facilities

    display presentation
        -> display model/value types
        -> desktop framebuffer
        -> platform/ps2 graphics
        -> diagnostics

    input
        -> stable action/event types
        -> platform/ps2 pad
        -> diagnostics

    ui
        -> stable action/event/display value types
        -> diagnostics where useful

    config
        -> stable display/input value types

    management
        -> stable display/config transaction value types
        -> ordinary socket/libc facilities
        -> diagnostics where useful

    diagnostics
        -> ordinary socket/platform timing as needed

    platform/ps2
        -> PS2SDK / gsKit / dmaKit / hardware

Important prohibitions:

- no subsystem depends on application policy internals;
- input/UI do not own RFB or management transport;
- RFB does not own GS/display policy;
- display does not parse RFB;
- management does not mutate display directly;
- config parsing does not execute behavior;
- diagnostics never calls back into product domains to make decisions;
- `common`/global headers are not used to hide cycles.

Stable value types may be placed with the domain that gives them meaning and
included by consumers. A new generic `common` domain is used only when there is
a truly ownerless low-level concept.

---

## 6. Startup/lifecycle model

The clean startup should preserve the evidence-backed ordering while making each
step visible in the coordinator:

1. allocate/initialize the authoritative desktop framebuffer capacity required
   by supported modes;
2. initialize PS2 system/IOP/module bootstrap;
3. initialize PS2 network hardware/NetMan foundation;
4. initialize early visible/debug facilities as appropriate;
5. initialize pad hardware and sample the explicit startup force-calibration
   chord;
6. configure PS2 IP and wait for private Ethernet readiness;
7. fetch human configuration and durable display transaction state from the Pi;
8. parse configuration side-effect-free and select one authoritative startup
   display profile through transaction reconciliation;
9. create/apply the local display/presentation state for that authority;
10. initialize optional diagnostics/runtime identity;
11. create a fresh reconciled RFB session and obtain an authoritative complete
    framebuffer;
12. publish the complete desktop;
13. start/enable ordinary controller event production;
14. enter the live coordinator loop.

Exact early pad-thread timing may be refined to preserve the startup chord while
keeping remote actions disabled until the RFB/display session is ready.

Explicit exit/fatal startup outcomes still converge to a known OSDSYS/system
menu path.

---

## 7. Live loop model

At a conceptual level the main loop repeatedly:

1. process urgent application/controller intent;
2. drain semantic input events and route them to UI, product actions, or RFB
   input serialization;
3. update/present local UI if its local generation is dirty;
4. request/receive RFB damage when no request is outstanding;
5. decode into the authoritative desktop framebuffer;
6. present coherent changed desktop regions;
7. run confirmation/cooldown/wall-clock state machines;
8. poll best-effort management/test control where enabled;
9. classify explicit failures and invoke the appropriate application policy.

The implementation may continue using an exact receive helper internally, but a
high-priority control mailbox must be able to return control to the coordinator
without teaching the RFB layer what "Refresh" or "Exit" means.

---

## 8. Risky display transition orchestration

The application owns the transaction. It coordinates, in order, mechanisms from
input, management, display, RFB, desktop framebuffer, and UI.

The durable sequence remains conceptually:

1. validate/resolve complete previous and candidate display profiles;
2. begin the durable Pi transaction before destroying known-good authority;
3. acquire controller/libpad ownership and discard pre-boundary queued remote
   input intent;
4. show a local transition curtain;
5. reconstruct target presentation/profile;
6. converge TigerVNC/RFB logical geometry;
7. obtain a complete authoritative target framebuffer;
8. prepare/present one coherent destination desktop;
9. release controller ownership;
10. run local confirmation or remote test lease policy;
11. KEEP commits durable candidate authority; rollback reconstructs and verifies
    the complete previous profile;
12. durable restore/restored/ack state is reconciled explicitly.

Input, RFB, display, and management each implement their own mechanism; none of
them independently owns this cross-domain policy.

---

## 9. Recovery orchestration

Application recovery policy distinguishes:

- explicit carrier/socket/RFB failure -> evidence-supported automatic session
  replacement;
- explicit manual Refresh -> user-requested replacement plus visible lifecycle
  and cooldown;
- failure during an unconfirmed display candidate -> display transaction
  rollback rather than generic reconnect;
- unexplained silent wait -> remains observable/manual during current debugging
  policy.

The RFB domain exposes one complete replacement mechanism. Reusing that
mechanism for several policies does not merge those policies.

---

## 10. Pi-side responsibility boundary

The Pi is a separate product side, not a PS2 source module.

The clean companion remains:

- supported ordinary Raspberry Pi OS;
- standard package/network/service mechanisms;
- private PS2 Ethernet configuration;
- predictable dedicated VNC desktop/session;
- small management daemon and persistent state;
- service health/recovery based on endpoint usability;
- operator status/start/stop/restart controls;
- versioned installer/package definition and dependency ledger.

VNC desktop availability remains independent of management-service success.
Historical VNC pacing is explicit evaluated/adopted state, not folklore.
Samba/file services remain commodity/optional until a particular product
behavior gives them a required contract.

PS2 build toolchains, containers, caches, and unrelated development-machine
history are not Pi runtime dependencies unless separately adopted.

---

## 11. Development/qualification boundary

Build/TestKit/evidence infrastructure remains outside runtime domains but part of
product engineering authority.

The reconstruction must preserve:

- reproducible toolchain/dependency identity;
- exact whole-ELF/PT_LOAD/runtime identity;
- canonical deployment/readback where required;
- declarative hardware tests;
- apparatus provenance;
- machine versus physical/operator evidence separation;
- PT_LOAD hardware gate and explicit qualification transfer;
- byte-preserved/sealed raw evidence;
- safe continuity/dirty-state preservation.

Migration-specific M0-M4 extraction mechanics remain historical unless they
continue to serve one of these recurring contracts.

---

## 12. Historical mechanism disposition — first pass

### Adopt as durable mechanisms/invariants

- private PS2/Pi Ethernet foundation and qualified PS2IP dependency discipline;
- RFB 3.8 exact framing/fail-closed stream handling;
- complete Raw authoritative frame before entering/re-entering normal live
  incremental operation;
- explicit ExtendedDesktopSize negotiation/confirmation;
- conventional authoritative framebuffer validity tracking;
- Test14 HIRES interrupt-delivery discipline and `ExitHandler()` return rule;
- complete display-profile transaction/rollback semantics;
- libpad acquire/ack/release ownership and stale-state invalidation;
- one-shot OSK modifiers;
- local held-button quarantine until release;
- manual Refresh admission/cooldown and suspect-stream replacement;
- human config strict recognized-value validation plus forward compatibility;
- durable Pi display transaction state/reconciliation;
- deterministic runtime identity;
- Pi systemd/service-health model;
- exact DUT/PT_LOAD/apparatus/evidence qualification contracts.

### Adapt into cleaner ownership

- encoded controller-thread RFB output queue -> semantic input event queue;
- scattered action/request globals -> small application control mailbox;
- display component globals -> complete display profile values + owner state;
- RFB/GS shared framebuffer globals -> explicit desktop framebuffer object;
- copied management HTTP helpers -> one bounded management client;
- multiple menu globals -> one explicit local UI/foreground model;
- Pi development checkout/runtime entanglement -> installed versioned companion
  package/state;
- legacy TestKit direct usage -> immutable successor bridge until replacement is
  intentionally qualified.

### Defer from the initial clean reconstruction

- historical direct-to-presentation/high-resolution fast paths that can make the
  conventional linear framebuffer stale;
- further Hextile/dirty-region/presentation micro-optimization beyond the
  simplest proven stable path;
- MPEG/hybrid-video paths;
- optional/experimental display catalog entries outside the first reconstruction
  milestone;
- replacement of mature TestKit purely for aesthetic consistency.

These are reconsidered after the smallest clean desktop path is working.

### Do not adopt as clean-product architecture

- B4A/successor `.inc` cut boundaries;
- old core/tail/directional-header structure;
- one source file per historical migration leaf merely because it already
  exists;
- controller/input code writing RFB socket data directly;
- local-UI pointer-jiggle hack as the primary repaint mechanism;
- automatic silent-stall reconnect during the current debugging policy;
- best-effort hardware-test remote control as a hard runtime dependency;
- broad unrestricted application globals or a giant public app-state struct;
- a general component/callback/DI framework.

---

## 13. Open synthesis questions

These questions must be resolved before `REBUILD_READY` promotion. They are
implementation-boundary questions, not missing behavioral knowledge.

1. What is the smallest practical C representation of the cross-thread control
   mailbox on EE that remains obvious and safe without introducing a framework?
2. Should the display model/pure geometry and hardware presentation begin in one
   source module or two? The ownership boundary is clear; file boundary remains
   intentionally malleable.
3. How small can the initial UI implementation remain while retaining explicit
   foreground/OSK state and a local dirty/wake path?
4. Which pieces of the historical direct-write/high-resolution path are needed
   for the first useful 480p reconstruction, versus deferred performance work?
5. What exact Pi OS/package baseline becomes the clean companion authority, and
   which historical runtime dependencies/pacing settings enter the dependency
   ledger as ADOPTED versus EVALUATING?
6. Which successor-owned TestKit pieces must exist before the first clean PS2
   reconstruction hardware milestone, versus continuing through the proven
   legacy bridge?

## Next synthesis action

Build the explicit mutable-state ownership table and interface matrix from this
responsibility map, then check the complete startup/live/display-transition/
recovery flows for forbidden dependencies or hidden shared ownership. After
those checks, revise the old normative architecture documents or supersede them
with this evidence-derived model and promote the behavioral families to
`REBUILD_READY` where justified.
