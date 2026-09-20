# Ledge Wire Runtime Decisions

DOCUMENT=LEDGE_WIRE_RUNTIME_DECISIONS
DOCUMENT_REVISION=0011
BRANCH_SCOPE=ledge/h1-all-guns
DECISION_STATUS=GOVERNING_FOR_LEDGE_RECONSTRUCTION
BASED_ON_BRANCH_COMMIT=0e72d0ee26f8055c56d64b0a051239a6d87c8763
RECONCILED_TO_LEDGE_AT=2026-09-20T15:54:52-04:00
RECONCILIATION_RECORD=docs/ledge/LEDGE_Q1_Q12_RECONCILIATION.md

This document records architecture decisions made during the user-assisted A003
manual reconstruction so later engineering work does not depend on conversational
memory. Historical/current implementation evidence is preserved separately from
the mature design selected from that evidence.

These decisions refine the canonical Wire terminology in
`docs/ledge/LEDGE_WIRE_TERMINOLOGY.md`. They preserve accepted Wire Protocol
bytes and A003 START/RETIRE representations; they define ownership and lifecycle,
not a wire-format change.

## Decision-recording rule

For each question resolved during the manual reconstruction, record:

1. the historical/current implementation;
2. what the experiments taught us;
3. the mature design we would choose if those lessons had been known from the
   beginning;
4. the API/lifecycle obligations imposed on components that attach to the
   changed boundary;
5. downstream work required by the decision.

When implementation later changes code, the same contract must also be visible:

- directly in the affected source through synopsis/comments at the smallest
  meaningful ownership boundary;
- in the owning component/directory documentation or change record;
- in project/architecture notes that describe cross-component consequences.

Those records must agree and cross-reference the same architectural intent.
Working code alone does not complete an interface-affecting reconstruction
change.

## Q1 — RFB provider attachment and lifetime boundary

STATUS=ANSWERED

### Historical/current implementation

Before the shared Wire/mux architecture, the PS2-facing TCP endpoint was itself
an RFB endpoint. Different RFB providers were experimentally interchangeable at
the same endpoint, but provider replacement did not preserve the established
PS2 TCP/RFB connection.

The preserved provider-switch evidence shows an established PS2 connection to
WayVNC on `192.168.50.1:5900`, followed by stopping WayVNC, freeing port 5900,
starting TigerVNC `w0vncserver` on that endpoint, and then requiring the PS2VNC
ELF to be relaunched/reconnected to confirm the replacement provider.

Therefore the pre-mux system proved **provider interchangeability at the
endpoint**, not provider-independent connection lifetime.

The currently installed Pi service topology still contains this pre-mux
heritage: the direct RFB activation socket owns `192.168.50.1:5900`, while
audio remains a separate service/listener. This installed state is evidence of
history/current deployment, not authority for the mature reconstruction.

### Lesson

A replaceable RFB provider is not sufficient if the physical PS2↔Pi connection
is still owned by that provider. Provider failure, stop, or replacement then
also destroys the cross-machine connection.

The later mux experiments established the more useful abstraction: RFB should
be a logical rider over a product-owned connection whose lifetime does not
belong to RFB.

### Mature decision

RFB attaches to a **Pi-local provider-neutral RFB ingress/adapter**. That adapter
consumes ordinary RFB from whichever provider is selected and maps the bytes
onto the RFB Wire Channel of an already-established Wire Session.

Canonical relationship:

```text
RFB provider -> local RFB adapter -> Wire RFB channel -> PS2
```

The RFB provider does not own, terminate, or directly read/write the physical
PS2↔Pi Wire connection.

Starting, stopping, failing, or replacing the RFB provider must not by itself
terminate the Wire Session.

Historical H1 use of a loopback RFB endpoint such as `127.0.0.1:5903` is
mechanism evidence only; the local port/provider is configuration, not the
architecture boundary.

### Attachment contract

An RFB provider/adapter may rely on a Wire Session being available through the
Wire Transport public seam, but it may not:

- open a competing PS2-facing product socket;
- receive from the physical Wire socket;
- write raw RFB bytes directly to the physical Wire socket;
- make Wire Session lifetime contingent on RFB activity;
- assume that its own provider identity is the identity of the product
  connection.

## Q2 — Pi-side Wire Session owner and lifetime

STATUS=ANSWERED

### Historical/current implementation

The currently installed Pi runtime does not yet contain the mature production
Wire server. Its installed services still reflect the earlier direct-RFB/audio
topology.

Later H1 experiments introduced a PS2-facing multiplexed connection and a sole
session/receive owner. Those experiments proved the required mechanisms and
ownership invariants, but their coordinator/process shape remains experimental
scaffolding rather than an implementation that must be promoted wholesale.

### Lesson

The cross-machine connection must be a first-class PS-to-VNC product
abstraction. It cannot belong to RFB, PCM/audio, MPEG, or any other payload
provider.

The useful invariant from H1 is the one physical connection, one receive owner,
Wire framing/multiplexing, ordered send, and logical-channel dispatch. The
historical H1 class/process arrangement is evidence, not structural authority.

### Mature decision

A properly configured Pi installation contains a dedicated, continuously
running **Wire server** as part of the Pi-side PS-to-VNC installation bundle.

Systemd starts and supervises that service. The Wire server itself owns:

- the Wire listening endpoint;
- accepted physical connections;
- the active Wire Session;
- the sole physical receive path;
- Wire Protocol framing and validation;
- logical-channel multiplexing/demultiplexing;
- serialized/ordered physical transmission;
- Transport flow-control mechanism;
- Transport-level connection/session state and mechanism failure reporting.

The server normally remains running and listening whenever the Pi product
installation is active, unless a user deliberately stops/disables it or an
exceptional failure prevents operation.

The PS2 ELF attempts to establish the Wire Session at launch. Loss of an
individual Wire Session does not imply that the Wire server exits; the server
remains product infrastructure and returns to a state in which a connection can
be established again.

### Single-wire / mux-only invariant

The physical PS2↔Pi product connection carries **only Wire Protocol traffic for
its entire lifetime**.

Every application byte crossing that TCP connection is part of a Wire
Protocol/Wire Transport frame. No rider may temporarily take over the physical
connection or bypass the mux.

RFB, PCM/audio, MPEG, control, telemetry, and future facilities are Wire
Channels/riders. Even when only one rider is active, the physical connection is
still a Wire connection carrying framed logical-channel traffic.

TCP's own protocol packets/ACKs are below this application-layer rule and are
not bypass traffic.

### Idle-session invariant

An established Wire Session is valid while completely idle.

No RFB, PCM/audio, MPEG, control, telemetry, heartbeat, or other rider traffic
is required merely to keep a healthy Wire Session open.

A future heartbeat/liveness facility may be added to improve dead-peer
detection and recovery, but if added it is itself Wire Protocol control traffic
and must not become a prerequisite that defines whether an otherwise healthy
idle session is allowed to exist.

### Dependency direction

The Wire Session is a prerequisite for all PS2↔Pi cooperative functionality.

No PS2↔Pi cooperative rider is a prerequisite for the Wire Session.

Canonical dependency:

```text
Wire Session
    -> enables RFB
    -> enables PCM/audio
    -> enables MPEG
    -> enables control/telemetry
    -> enables future cooperative channels
```

Never the reverse.

The PS2 ELF remains an independently living local application. Local-only
capabilities such as menus, controller-driven local behavior, configuration,
diagnostics, and future PS2-local storage functionality must remain usable
without an active Wire Session where their own requirements permit it. Wire
loss disables or places into recovery only the functionality that genuinely
requires Pi cooperation; it is not a rule that the ELF or unrelated local
capabilities must terminate.

### Server/session lifetime distinction

**Wire server lifetime** is normally the lifetime of the installed Pi product
service.

**Wire Session lifetime** is the lifetime of one established PS2↔Pi Wire
connection/session.

A session may begin and end without terminating the server.

### Attachment contract

All cross-machine riders must attach through Wire Transport logical/public
interfaces. They may not:

- own the physical product socket;
- call `recv()` on the physical Wire stream independently;
- write raw payload bytes outside Wire framing;
- create a parallel PS2-facing product connection for ordinary rider traffic;
- couple Wire Session lifetime to their own activity or availability.

Transport owns mechanism only. RFB, MPEG, PCM/audio, Presentation, Input/UI,
generation coordination, and future domains retain their own semantics and
lifecycle policy.


## Q3 — Minimum contract between Wire Transport and riders

STATUS=ANSWERED

### Intent recovered from the user-assisted design discussion

A rider should be able to appear, use its logical Wire Channel, disappear, stall,
restart, or be replaced without becoming part of the Wire server's own lifetime.

The useful mental model is a persistent transport system with stable logical
destinations: the Wire Channel exists because Wire Protocol defines it, not
because a particular rider is presently running. The transport system must not
reach into a rider's private buffers or interpret the rider's domain payload in
order to perform delivery.

The design must also tolerate a rider that stops making progress without
allowing that rider to consume unbounded memory or force unrelated channels or
the Wire Session to fail merely because its own consumer is absent or stalled.

### Mature decision — Wire Channel Relay

Each logical Wire Channel connects to its owning domain through a
Transport-owned **Wire Channel Relay**.

A Wire Channel Relay is the last-mile courier between one logical Wire Channel
and the public boundary of the module that owns the channel's domain behavior.

Canonical relationship:

```text
owning module
     ↑↓
module public boundary
     ↑↓
Wire Channel Relay
     ↑↓
Wire Channel
     ↑↓
Wire Transport
     ↑↓
Wire Session
```

The Relay exists on the Transport side of the component boundary. Its ownership
name is deliberate: it is part of Wire Transport, not part of MPEG, RFB,
PCM/audio, or another rider domain.

### Payload-opacity invariant

The Wire Channel Relay transfers channel payload without interpreting,
translating, parsing, or transforming the payload's domain semantics.

For example, an MPEG Wire Channel Relay may move payload bytes between the
MPEG Wire Channel and MPEG's public boundary, but it does not understand MPEG
frame structure, decoding, generation semantics, MPEG-owned queue semantics, or
how MPEG will use those bytes.

Likewise, Wire Transport knows that payload belongs to a particular Wire
Channel and knows the Transport mechanics required to move it safely; it does
not need to know what the payload means to the receiving domain.

The receiving module owns all domain interpretation and processing after the
handoff. The sending module owns creation of domain payload before the handoff.

### Transport-bookkeeping invariant

Payload opacity does not make the Relay ignorant of Transport mechanics.

A Wire Channel Relay may participate in Transport-owned bookkeeping required
for safe handoff, including:

- bounded logical-channel capacity;
- channel consumption/release accounting;
- credits or equivalent Transport flow-control state;
- Transport-side readiness needed to determine whether more channel payload
  may safely be admitted;
- channel-local mechanism failure/reporting.

Those are Wire Transport semantics, not rider-domain semantics.

A domain module should not need to construct or interpret Wire credit protocol
messages merely to consume its payload. When a module drains accepted channel
data through its public boundary, the Transport side may use that consumption
to free capacity and update/send the corresponding Transport flow-control
state.

### Buffer and ownership boundary

Wire Transport and its Relay must not reach into another component's private
buffers, parser state, decoder queues, framebuffer state, or other domain
internals.

The domain owner decides how accepted payload is buffered, parsed, queued,
decoded, rendered, played, or otherwise processed after it crosses the public
handoff.

The architectural requirement is the explicit ownership boundary. It does not
require an artificial standalone process, class, or source file for every
Relay if the same responsibility can be expressed clearly inside the genuine
Wire Transport component without weakening the boundary.

### Stalled or absent rider behavior

A rider's absence or stall is not by itself a Wire Session failure.

If a receiving domain stops draining its channel, the corresponding bounded
Transport capacity may become exhausted. Further payload for that channel must
then be limited by the applicable Transport flow-control/backpressure
mechanism rather than accumulated without bound or pushed directly into the
domain's private memory.

The failure/stall of one rider must not inherently terminate unrelated Wire
Channels or the Wire Session.

This preserves distinct progress facts:

```text
Wire received payload
        ↓
payload is available through the logical channel / Relay
        ↓
domain accepted payload across its public boundary
        ↓
domain processed payload
```

Those are separate facts and must not be collapsed into one success state.

### Rider contract

A rider/domain may rely on the Wire Channel Relay as the Transport-owned
handoff for its logical channel.

The rider/domain owns:

- payload meaning;
- domain parsing/decoding/encoding;
- domain-private buffering and queues;
- domain lifecycle and semantic admission rules;
- what happens to payload after it accepts it;
- what domain payload it produces before handing it to Transport.

The rider/domain may not:

- read or write the physical Wire socket;
- bypass Wire framing/multiplexing;
- require Wire Transport to understand its private payload format;
- expose private domain buffers for Wire Transport to mutate directly;
- make its own continued activity a prerequisite for Wire Session health.

### Naming rationale

**Wire Channel Relay** is the canonical architecture term.

"Relay" is preferred over "adapter" because this boundary does not translate
domain representation; it relays payload across an ownership boundary.

"Relay" is preferred over "bridge" because the repository already uses
"bridge/public seam" for a component's own externally visible boundary. The
Wire Channel Relay connects Wire Transport to that owning module's public
boundary rather than replacing the module's bridge.

Plain-language explanation: the Wire Channel Relay is the channel's
**last-mile courier**.


## Q4 — Wire Session establishment and provisional peer handling

STATUS=ANSWERED

### Intent recovered from the user-assisted design discussion

A successful TCP connection is not, by itself, sufficient evidence that the
peer is the intended PS-to-VNC PS2-side software. The Pi must have a small,
replaceable establishment step before ordinary Wire Channels become active.

The purpose of this first mechanism is practical accidental-peer protection and
compatibility checking, not strong security authentication. The design should
solve the present problem simply while preserving a clear seam where stronger
authentication, update, or recovery behavior can later be inserted without
reshaping Wire Transport, Wire Channel Relays, or rider domains.

### Provisional connection state

When the PS2 ELF opens the physical TCP connection and the Pi Wire server
accepts it, the connection initially enters a **provisional Wire connection**
state.

The connection speaks Wire Protocol from its first PS-to-VNC application byte.
There is no temporary raw RFB, raw media, or separate pre-Wire application
protocol.

Ordinary rider traffic is not enabled while the connection is provisional.

### Minimal PS2 establishment message

The PS2-side ELF initiates establishment by sending a small Wire-framed
handshake containing only the minimum current compatibility identity:

- the Wire Protocol version;
- the PS2 ELF/product version.

A recognizable valid pair of those version values is sufficient for the
current implementation to infer the intended software relationship and reject
accidental unrelated peers.

Separate product-name and endpoint-role strings are not required merely to
repeat information already implied by a valid establishment exchange on this
specific Wire server.

The concrete on-wire representation should be deterministic fields, not a
free-form concatenated text string whose field boundaries are ambiguous.

### Identification is not strong authentication

The initial version-pair mechanism is deliberately modest.

It answers the present question:

> Is this provisional peer intentionally behaving like a compatible PS-to-VNC
> PS2 client rather than an unrelated device or process that happened to connect?

It does not claim to prevent a knowledgeable party from deliberately imitating
the establishment exchange.

The rest of the system must depend on the **establishment result**, not on the
specific representation or proof mechanism used to obtain that result. This
keeps the provisional-to-active boundary replaceable so a future
challenge/response, shared-secret, signed-identity, certificate, or other
authentication design can occupy the same seam if a real requirement later
justifies it.

### Pi acceptance and Wire Session creation

If the Pi recognizes and accepts the establishment values, the Pi Wire server:

1. reports acceptance to the PS2;
2. establishes the authoritative Wire Session identity;
3. promotes the provisional connection to an **active Wire Session**.

Conceptually:

```text
PS2 ELF                           Pi Wire server
   |                                   |
   |------ TCP connect --------------->|
   |                                   |
   |                       provisional Wire connection
   |                                   |
   |------ version tuple ------------->|
   |       Wire version                |
   |       ELF version                 |
   |                                   |
   |                       validate / accept
   |                       establish session identity
   |                                   |
   |<----- ACCEPT + session_id --------|
   |                                   |
   |========= ACTIVE WIRE SESSION =====|
```

The Pi Wire server is authoritative for the session identity created by
successful establishment.

### Active-session independence from rider traffic

Successful establishment itself creates the active Wire Session.

RFB, PCM/audio, input/control, MPEG, telemetry, or any future rider traffic may
begin afterward, and in ordinary operation some of those riders will likely
become active immediately.

None of that rider traffic establishes or sustains the Wire Session.

An active Wire Session remains valid if completely idle, consistent with Q2.

### Establishment rejection

If the Pi cannot accept the presented establishment values, no ordinary Wire
Session is created and no normal rider traffic becomes active.

Wire Transport reports a clean establishment failure result upward rather than
embedding the application response inside Transport.

The current architecture requires at least a result equivalent to:

```text
ESTABLISHED
NOT_ACCEPTED
```

Exact enum/API spelling may be chosen during implementation.

### PS2 Wire Establishment Policy

The PS2 application owns a distinct **Wire Establishment Policy** boundary that
decides what the application does with the establishment result.

The initial implementation is intentionally minimal:

```text
ESTABLISHED  -> continue with the active Wire Session
NOT_ACCEPTED -> shutdown
```

The current `NOT_ACCEPTED -> shutdown` behavior is **initial application
policy**, not a Wire Transport invariant.

That distinction must be documented directly in the eventual source and local
component contract so the extension point is not forgotten merely because the
first implementation has only one rejection action.

The policy boundary is intentionally suitable for later evolution into choices
such as:

- retry establishment;
- request or perform an update;
- continue in local-only mode;
- close/shutdown.

Those future behaviors are not required for the present reconstruction.

### Future update path reserved, not designed

The version exchange creates a natural future opportunity for the Pi to
recognize that the PS2 ELF is outdated while still understanding enough Wire
Protocol to communicate safely.

A later implementation may use the provisional establishment/recovery space to
offer or deliver an updated PS2-side version, including eventual direct delivery
to suitable PS2 storage.

That update mechanism is **not designed or required by Q4**. The present
architecture only preserves a clean place for it.

If future software is so Wire-incompatible that safe in-band update traffic
cannot be exchanged, another recovery/bootstrap mechanism may be required. Q4
does not pretend that every future incompatibility can be repaired over the same
connection.

### Bounded implementation rule

The initial implementation should build only the smallest establishment
mechanism required now:

```text
TCP connect
    ->
provisional Wire connection
    ->
PS2 version tuple
    ->
Pi ACCEPT + session_id
        or
Pi NOT_ACCEPTED
```

Do not prematurely implement cryptographic authentication, capability
negotiation, update transfer, retry UI, bootstrap recovery, or a richer
connection-recovery state machine merely because the establishment seam now
exists.

The architectural obligation is to preserve that seam so those features can be
added later without coupling them into Wire Transport's ordinary session and
channel machinery.


## Q5 — Authoritative Pi desktop/session geometry

STATUS=ANSWERED

### Persistent configuration authority

The installed Pi product configuration is the persistent source of truth for
display modes and their calibrated desktop geometry.

The configuration is intended to support multiple PS2-compatible display modes.
For each mode it may retain facts such as:

- output/display resolution;
- calibrated safe-desktop width and height;
- calibrated safe-desktop X/Y offsets;
- whether a usable calibration exists for that mode.

The installation also provides a safe default display configuration. The
current design intent is a stable 480p default with a full-resolution/default
safe area suitable for first use before user-specific calibration.

The exact future config schema is not fixed by Q5; the ownership is.

### Active desktop/session authority

When the Pi desktop/session owner creates the active shared desktop, it resolves
the selected display mode and calibrated geometry from persistent
configuration and establishes an **immutable active-geometry snapshot** for that
desktop session.

Conceptually:

```text
Pi display configuration
        ↓
selected display-mode record
        ↓
Pi desktop/session owner
        ↓
immutable active desktop geometry
  - active resolution
  - safe X/Y offsets
  - safe width/height
        ↓
public session/config fact for consumers
```

The Pi desktop/session owner is authoritative for which configured geometry is
active at runtime.

The configuration file owns the persistent calibrated values; the runtime
desktop/session owner owns the resolved active snapshot.

### Exact-generation validation contract

The exact-generation owner consumes the already-established active geometry
through the owning public seam when validating START.

It must not derive the desktop bounds from:

- START itself;
- an RFB provider's current framebuffer report;
- a hardcoded qualified value such as `704x462`;
- PS2-side DESKTOP CALIBRATION state.

RFB may expose the desktop, but it does not define the project's configured
safe desktop geometry.

A003 START validation may rely on the active geometry as an established
session fact rather than becoming responsible for desktop configuration or
calibration policy.

### Runtime consistency

The desktop/session owner should establish that the actual desktop it created
is compatible with the selected/configured display geometry before publishing
that geometry as active.

A stale or contradictory persistent configuration must not silently become a
trusted active-session fact merely because it exists in the config file.

The exact verification mechanism belongs to the desktop/session implementation
and does not need to be designed as part of A003.

### Geometry-change rule

The active geometry used by consumers is immutable for the lifetime of that
desktop-session state.

A confirmed display-mode or calibration change establishes new active desktop
geometry/session state rather than silently mutating the bounds underneath an
already-prepared exact generation.

### Future display-mode and calibration policy — reserved, not required now

The intended future user experience includes several calibrated PS2-compatible
display modes.

The design space intentionally preserves these behaviors:

- the user can choose another PS2-compatible display mode;
- selecting the already-current display mode can enter safe-desktop calibration
  rather than redundantly reselecting the same mode;
- calibration adjusts the safe desktop rectangle for that display mode;
- confirming calibration replaces that mode's persistent calibrated
  X/Y/width/height values;
- abandoning or failing to confirm a display-mode/calibration change restores
  the previously accepted mode/state rather than leaving an unconfirmed mode
  active;
- the configuration may retain the state necessary to distinguish current,
  candidate, and previous/restorable display-mode state.

Startup calibration is also a future policy point. Prior project behavior used
an optional startup calibration entry after connection establishment when the
user held a controller chord such as `L1+R1`; a future implementation may
restore a similar explicit entry point or prompt for calibration on first boot
or first use of an uncalibrated mode.

Those menu, controller-chord, first-boot, rollback, and calibration UI mechanics
are intentionally **not required by Q5 or the active A003 packet**. Q5 requires
only that the persistent configuration and active-session authority be shaped so
those policies can be added later without changing who owns geometry.


## Q6 — MPEG activation, region definition, calibration, and suppression semantics

STATUS=ANSWERED

### Product behavior, not historical coordinator vocabulary

MPEG START is not a separate user-facing product operation. It is one internal
step in activating MPEG presentation.

The current/bootstrap interaction is conceptually:

```text
START+SELECT
    ↓
MPEG active?
    ├── yes -> end the current MPEG run
    └── no  -> enter MPEG-region calibration
                  ↓
               confirm
                  ↓
               suppress the corresponding RFB region
               start Pi MPEG capture
               start PS2 MPEG decode/presentation
               apply the configured matte geometry
```

`START+SELECT` is an initial trigger, not a permanent product requirement.

Likewise, the manual calibration sequence is an initial way to provide the
MPEG presentation region. It must not become the architectural definition of
how MPEG regions are selected.

### MPEG region definition

The durable concept is an **MPEG region definition** consumed by MPEG
capture/presentation and the cooperating RFB/presentation owners.

The proven manual calibration machinery is worth preserving because it encodes
real media/presentation constraints rather than temporary experiment
scaffolding.

The region definition includes the MPEG-compatible base/capture/presentation
area and the associated matte/suppression geometry.

The base MPEG dimensions remain aligned to the MPEG macroblock grid:

```text
width  = multiple of 16 pixels
height = multiple of 16 pixels
```

The grid itself is not globally locked to 16-pixel desktop coordinates. Its
X/Y placement may move at **one-pixel precision** so the MPEG region can be
visually aligned precisely while retaining 16x16-compatible dimensions.

Conceptually:

```text
x = pixel precision
y = pixel precision
width  = 16 * N
height = 16 * M
```

### Matte and RFB-suppression relationship

The existing internal/external matte relationship is preserved as product
behavior.

The **outer boundary of the external matte defines the RFB suppression area**.

Therefore RFB suppression geometry is not an unrelated Transport rule. It is a
consequence of the accepted MPEG region/presentation definition.

Conceptually:

```text
MPEG region definition
├── MPEG base/capture/presentation rectangle
├── internal matte geometry
├── external matte geometry
└── RFB suppression rectangle
    └── derived from the external matte's outer boundary
```

The exact matte implementation and rendering details remain owned by the
appropriate Presentation/calibration code, not by Wire Transport.

### Ownership of activation

Application owns the product-level MPEG activation/deactivation transaction.

Application coordinates the required owners through their public seams. It does
not absorb their internals.

The cooperating responsibilities are conceptually:

```text
Input/UI trigger or future region source
                 ↓
             Application
                 ↓
       activate / deactivate MPEG
          ├── region/calibration authority
          ├── RFB suppression request/state
          ├── Pi MPEG capture
          ├── PS2 MPEG receive/decode
          └── Presentation/matte state
```

Wire Transport carries the required control and MPEG media. It does not own the
meaning of the selected MPEG region, the matte geometry, or the user's
activation policy.

### Manual calibration is one region source

For the first implementation, manual calibration is an acceptable source of the
MPEG region definition.

The previously accepted manual MPEG calibration may be persisted in the Pi
configuration system described by Q5 so the user's last setting is not lost.

That persistent value is configuration state, not a rule that all future MPEG
activation must always enter the manual calibration UI.

A future menu item may allow the user to manually set or correct the MPEG region
without changing the underlying MPEG activation architecture.

### Future region sources

The architecture must deliberately leave room for additional MPEG-region
sources without redesigning MPEG, Wire Transport, or Presentation.

Expected future possibilities include:

- automatic detection of desktop regions that can reasonably be inferred to
  contain video;
- application-specific contracts for VNC or other application environments;
- region definitions pinned to an application/window so the MPEG presentation
  area can move with the corresponding video window;
- manual region selection as a fallback or correction path.

These mechanisms are future work. Q6 does not design their detection algorithms,
window-tracking protocol, application contracts, or UI.

They are listed here because the initial manual calibration flow must not be
implemented in a way that prevents them.

### Run/generation identity is fencing, not product semantics

A single MPEG activation may still carry a lightweight internal run/generation
identity where useful to prevent late bytes, stale completion, or retirement
from one MPEG run contaminating a later run.

Likewise, existing Wire-session identity may remain available to Transport or
protocol code where it provides useful stale-message fencing.

Those identities do not define whether the user has requested MPEG, do not own
the MPEG region, and do not turn MPEG activation into a separate
generation-management product subsystem.

The mature product meaning is simply one MPEG run operating on one accepted
MPEG region definition.

### Relationship to the active A003 exact-generation packet

This decision intentionally narrows the product semantics previously attached
to the phrase **exact generation**.

Existing A003 material that treats session identity, generation numbering, or
geometry validation as an independent product-level generation coordinator must
be reconciled against Q6 before behavior-bearing implementation continues.

Preserve the mechanisms that still serve a concrete purpose:

- exact control framing;
- old-run/new-run fencing where required;
- safe producer/decoder retirement;
- residual-byte isolation;
- representability and memory-safety validation;
- the proven MPEG-region calibration and matte/suppression relationships.

Do not preserve redundant business validation merely because historical H1 or
the current A003 packet described it in generation-centric language.

Q6 does **not by itself change the accepted START/RETIRE wire representation**.
Any later decision to remove, repurpose, or renumber fields is a separate
wire-format decision and must be made explicitly rather than inferred from this
semantic simplification.


## Q7 — MPEG retirement and overlapped RFB restoration

STATUS=ANSWERED

### User-facing meaning

Stopping MPEG means ending the current active MPEG run and returning the
affected desktop area to ordinary RFB presentation.

Stopping the run does **not** erase a confirmed manual MPEG calibration.
Confirmed calibration geometry may remain persisted in the Pi configuration
system described by Q5/Q6 and may be reused by a later manual MPEG activation.

Runtime MPEG state is disposable; accepted calibration configuration is
persistent state.

### Retirement begins by closing new MPEG production

When Application requests MPEG stop, the current MPEG run enters a retiring
state.

The first lifecycle obligation is to stop admitting/producing new MPEG content
for that run.

Already accepted/buffered MPEG data may continue to drain while the decoder and
presentation path remain alive long enough to consume it safely.

Conceptually:

```text
ACTIVE MPEG
    ↓ stop requested
RETIRING
    ├── no new MPEG production/admission
    └── already accepted MPEG data may drain
```

### RFB restoration overlaps MPEG retirement

RFB restoration does not need to wait for complete MPEG teardown.

As soon as retirement has closed the old run to new MPEG production, the RFB
suppression region may be released so RFB can resume requesting, receiving, and
updating the underlying desktop region while the visible MPEG presentation is
still winding down.

Therefore:

```text
RETIRING
    ├── MPEG:
    │   ├── drain accepted MPEG data
    │   ├── continue valid remaining presentation
    │   └── approach the safe decoder-stop boundary
    │
    └── RFB:
        ├── suppression released
        ├── fresh updates requested/accepted
        └── underlying desktop region rebuilt
```

Releasing RFB suppression does not itself require that RFB become immediately
visible. It permits the underlying RFB desktop state to become current again.

MPEG presentation and mattes may remain visually on top until their safe
retirement/finalization point.

### Safe finalization boundary

Teardown may begin while MPEG data drains, but resources that are still needed
by the decoder/worker must not be destroyed early.

Destructive finalization occurs only after the decoder/runtime reaches its safe
stop boundary.

The historical safe-stop lesson remains binding: an owner-requested stop must
not be implemented by making an active MPEG data callback synthesize EOF while
`MPEG_Picture()` is still waiting for input.

At finalization the implementation may:

- finish/close the decoder worker at its safe boundary;
- release decoder/session resources no longer visible to the worker;
- account for or discard old-run residual transport data as required;
- remove the active MPEG presentation;
- remove the internal/external mattes associated with that run.

### Visible handoff

The intended visual sequence is:

```text
MPEG visible
    ↓
MPEG retirement drains while RFB refreshes underneath
    ↓
MPEG presentation + mattes removed
    ↓
already-refreshing RFB desktop revealed
```

This deliberately avoids making the user wait for RFB recovery only after MPEG
has disappeared, and reduces the chance of exposing a stale frozen RFB region
during the transition.

### Old-run / new-run isolation

No MPEG bytes, decoder completion, retirement completion, or other run-private
state from the retiring run may contaminate a later MPEG activation.

A lightweight run/generation identity may be retained wherever needed to prove
that isolation, consistent with Q6.

This fencing is an implementation-safety responsibility. It does not make the
run/generation identity the product-level meaning of MPEG start/stop.

### Persistent calibration survives retirement

Confirmed manual MPEG calibration is not part of the disposable runtime
teardown set.

If persisted, it remains in Pi configuration after the MPEG run ends.

A later manual activation may therefore reuse the previous accepted region
definition rather than requiring the user to rediscover the same geometry.

Future automatic-detection or application-pinned region sources may supply a
different active MPEG region without destroying the stored manual fallback.

### Ownership summary

Application owns the product-level stop/retirement transaction and coordinates
owners through their public seams.

The owners retain their own responsibilities:

- Pi MPEG capture/producer owner closes new production and retires capture;
- Wire Transport enforces the channel/fencing mechanics needed to prevent
  old-run leakage;
- MPEG decoder owner drains and stops only at its safe lifecycle boundary;
- RFB owner resumes the formerly suppressed region;
- Presentation owner keeps MPEG/mattes visible until retirement reaches the
  safe visible handoff, then reveals the already-refreshing RFB state;
- configuration ownership remains independent and preserves accepted manual
  MPEG calibration when configured to do so.


## Q8 — Hierarchical fault containment and minimum-scope recovery

STATUS=ANSWERED

### General system rule

Unexpected failure is contained to the smallest ownership boundary that can
still be brought to a known-safe state.

Recovery begins at the component that failed.

If that component can prove that it has stopped its work, released or
reinitialized the resources it owns, restored its required invariants, and
reported a clean bounded failure upward, only that component is shut down or
reset.

If it cannot prove a safe local shutdown, recovery escalates to the next
containing ownership scope.

This repeats recursively until a scope is reached that can restore known-good
invariants.

Conceptually:

```text
unexpected failure
      ↓
can failing component restore its own invariants?
      ├── yes -> close/reset only that component
      │         recover locally
      │
      └── no  -> escalate to containing owner
                    ↓
              can that scope recover safely?
                    ├── yes -> close/reset that scope
                    └── no  -> escalate again
```

Whole-Wire, whole-application, or whole-system shutdown is therefore the final
escalation, not the default response.

### "Smallest possible" means smallest provably safe failure domain

The smallest recovery scope is not simply the smallest function, thread, or
source file that can be named.

It is the smallest **ownership/failure domain** whose safe state can actually be
proven.

For example, if a worker has failed while still owning DMA, decoder, socket, or
other shared resources, pretending to stop only one narrow function is not
sufficient. Recovery must escalate until an owner can definitively quiesce,
release, replace, or reinitialize the affected resources.

The architecture must therefore make component ownership and shutdown
invariants explicit enough that this escalation can be reasoned about rather
than guessed.

### Sibling independence

A failure in one component must not automatically terminate healthy siblings or
unrelated facilities.

Examples include:

- an MPEG decoder/run failure should normally retire MPEG and fall back to RFB
  without terminating the Wire Session;
- a PCM/audio failure should normally stop/recover PCM while RFB, MPEG, and Wire
  remain available;
- an RFB parser/session failure should normally replace/recover the RFB session
  without unnecessarily terminating unrelated riders;
- a Wire Channel Relay/channel-local failure should remain channel-local when
  Wire Transport invariants are still intact;
- a Wire Session failure may terminate/recover that session while PS2-local UI,
  configuration, diagnostics, input, and other genuinely local capabilities
  remain alive where their own invariants permit.

Escalation across those boundaries occurs only when the narrower owner cannot
restore a safe state or when the failure has demonstrably compromised the
parent's invariants.

### Component contract requirement

Every genuine runtime component should define:

- the resources and state it owns;
- the invariants that mean it is healthy;
- its bounded stop/reset/retire path;
- what evidence proves that shutdown completed safely;
- what failure result it reports upward;
- what conditions mean local recovery is no longer trustworthy and escalation
  is required.

This allows Application and other containing owners to make recovery decisions
without reaching into component-private implementation state.

### MPEG application of the rule

Q7 describes the normal MPEG retirement path.

On unexpected MPEG failure, the same minimum-scope rule applies:

1. stop or fence further MPEG production/admission where still possible;
2. release RFB suppression so RFB can begin restoration;
3. drain only MPEG state that can still be proven safe to drain;
4. retire the decoder/run at a valid safe boundary;
5. if the MPEG run cannot be safely contained, escalate to the MPEG subsystem
   owner;
6. if MPEG subsystem invariants cannot be restored without affecting shared
   state, escalate to the containing Application transaction;
7. escalate to Wire Session recovery only if Wire/Transport invariants are
   actually compromised;
8. terminate the whole ELF only when no smaller containing scope can restore a
   trustworthy state.

A failed MPEG run does not erase persisted manual MPEG calibration, consistent
with Q6/Q7.

### Relationship to diagnostics and debugging

Fault containment must not hide which layer actually failed.

The project should preserve enough diagnostic evidence to identify:

- the component where failure was first observed;
- the lowest ownership boundary that could still prove safe shutdown;
- each escalation step taken;
- the final recovery scope that restored known-good invariants.

This aligns with the project's layered freeze-debugging philosophy: identify
the lowest layer still making provable progress and avoid destroying healthy
layers merely because a higher-level symptom is visible.


## Q9 — Wire loss, non-resumable sessions, and service restoration

STATUS=ANSWERED

### Wire Session identity is disposable runtime state

Loss of the physical Wire connection permanently ends the Wire Session that
belonged to that connection.

A dead Wire Session is never resumed.

Every successful reconnect follows the normal establishment path again:

```text
physical connection lost
        ↓
old Wire Session ends permanently
        ↓
reconnect attempt
        ↓
new provisional Wire connection
        ↓
normal establishment
        ↓
new authoritative Wire Session identity
```

The new connection therefore receives a new session identity rather than
attempting to continue the old one.

### Runtime state dies with the old session

Distributed runtime state tied to the lost Wire Session is not silently carried
forward into the replacement session.

Examples include:

- old Wire Channel/Relay runtime state and queues;
- the old RFB stream/session;
- the currently active MPEG run;
- current PCM/audio stream runtime state;
- old session-scoped flow-control/bookkeeping state;
- any other rider-private runtime state that depended on the dead connection.

The old session identity remains useful only as stale-state evidence/fencing; it
is not a reconnect target.

### Local application lifetime remains independent

Wire loss does not by itself terminate the PS2 ELF.

Genuinely local facilities remain alive where their own invariants permit,
including local UI, input handling, configuration, diagnostics, and other
PS2-local behavior.

The Application may present a disconnected/reconnecting state while Wire
Transport attempts to establish a replacement connection according to current
recovery policy.

### Restoration is Application policy, not Wire behavior

Once a new Wire Session has been established, Wire Transport has restored
connectivity only.

It does not decide which higher-level services or user activities should be
recreated.

Application owns the restoration policy.

Ordinary baseline services such as RFB may be restored automatically.

Other interrupted activities may use different policy. In particular, an MPEG
run that was active when Wire was lost is dead with the old session and is not
silently resumed as though the interruption never occurred.

A future restored-connection UI may instead report that connectivity has
returned and offer to restore a previously active MPEG presentation using
available persistent configuration.

The exact UI and policy are future work.

### Confirmed MPEG calibration is persisted at confirmation time

Q6/Q7 established that accepted manual MPEG calibration may persist in Pi
configuration.

Q9 makes the persistence boundary explicit:

**A manual MPEG calibration becomes durable when the user confirms the
calibration, not when the MPEG run later retires.**

Conceptually:

```text
manual MPEG calibration
        ↓
user confirms
        ↓
accepted MPEG region definition
        ↓
persist to Pi configuration
        ↓
activate MPEG run
```

Retirement therefore has no authority over whether an already accepted
calibration survives.

An unexpected Wire loss shortly after MPEG activation cannot erase the accepted
manual calibration merely because normal MPEG retirement did not occur.

### Durable state versus session state

The architecture distinguishes durable configuration from session-scoped
runtime state.

Examples of state that may survive Wire Session loss:

- display configuration;
- confirmed desktop calibration;
- confirmed manual MPEG calibration;
- user settings and other persistent configuration.

Examples of state that dies with the old Wire Session:

- old session identity;
- current Wire queues/channel state;
- active RFB stream state;
- active MPEG run state;
- active PCM/audio stream state.

A replacement Wire Session reconstructs runtime behavior from current durable
configuration and Application policy rather than inheriting opaque live state
from the dead session.

### Future MPEG restoration

A future reconnect/restoration flow may use the persistent manual MPEG region to
offer a restart of interrupted MPEG activity.

For example, Application may eventually know:

- that MPEG had been active before connection loss;
- that a valid saved manual MPEG region exists;
- that a new Wire Session and baseline RFB service are healthy.

It may then offer the user a choice such as restoring MPEG or remaining with
ordinary RFB.

This is intentionally Application/UI policy. Wire Transport neither remembers
nor automatically recreates MPEG activity.


## Q10 — Authoritative Wire Session availability publication

STATUS=ANSWERED

### Minimal public lifecycle fact

Wire Transport owns one authoritative module-facing fact describing whether a
usable Wire Session currently exists:

    WireSessionState
    - INACTIVE
    - ACTIVE

When no established Wire Session exists, the published state is INACTIVE.

When establishment succeeds, the published state becomes ACTIVE.

When that session is lost or closed, the published state returns to INACTIVE.

A later successful establishment publishes ACTIVE again for the new session.

Wire may retain an authoritative session identity internally for protocol,
Transport bookkeeping, evidence, diagnostics, or other Transport-owned needs.
Cross-Wire modules do not need that identity merely to decide whether Wire work
is currently possible.

### No rider or configuration prerequisite

A Wire Session requires no rider activity, configuration synchronization,
desktop session, RFB provider, MPEG subsystem, PCM/audio subsystem, telemetry
provider, or other product module in order to remain established.

The Wire server may have a valid active Wire Session while no ordinary rider is
doing useful work.

This preserves the dependency direction established in Q2:

    Wire Session exists
        |
        v
    modules may use it

Never:

    modules/configuration initialized
        |
        v
    therefore Wire Session may exist

### Common prerequisite for cross-Wire module activity

For a module that needs to perform work across Wire, ACTIVE is the common
Transport prerequisite.

ACTIVE means only that valid cross-Wire Transport is presently available.

It does not imply that a module's remote counterpart is ready, that the module
has completed its own establishment, or that its domain-specific prerequisites
have been satisfied.

Each module remains responsible for its own readiness and lifecycle.

Examples:

    Wire ACTIVE
        |
        +-- RFB may run its ordinary startup path
        +-- PCM/audio may run its ordinary startup path
        +-- remote input forwarding may become available
        +-- MPEG may start only when its own trigger is also satisfied
        +-- future modules apply their own start policy

### Publication, not orchestration

Wire owns connection/session state and publishes availability through its public
seam.

It does not start business modules, require those modules to be running,
maintain business-level startup policy, interpret domain-specific readiness, or
orchestrate rider lifecycle merely because Wire changed state.

Application and individual modules consume availability according to their own
responsibilities.

### Identity remains Transport-owned

The architecture does not require a module-facing session ID.

A concrete Wire Session identity may still exist where Transport itself needs
one. That identity does not need to be copied into every rider merely to enforce
reconnect safety.

The public lifecycle invariant is:

- no usable session -> INACTIVE;
- usable current session -> ACTIVE;
- an ended session never becomes current again.

## Q11 — Independent module readiness and failure domains

STATUS=ANSWERED

### Wire availability is not rider readiness

An active Wire Session means only that the common cross-machine Transport is
available.

It does not mean that RFB, MPEG, PCM/audio, telemetry, input/control, or any
future distributed module is already established or usable.

Each distributed module independently owns whatever additional establishment,
readiness, runtime, and shutdown state its own function genuinely requires.

Conceptually:

    WireSession = ACTIVE
        |
        v
    shared Transport prerequisite satisfied
        |
        v
    each module independently decides whether and how to establish

### No universal rider-readiness state machine

Wire Transport does not impose a single business-level state machine on all
riders.

A module may use states such as inactive, starting, usable, stopping, failed, or
something simpler if that better matches the domain.

The architecture requires clear ownership of readiness, not identical enum
shapes across unrelated domains.

A module-specific handshake or readiness protocol exists only when that module's
actual protocol or lifecycle requires it.

### Module failure does not imply Wire failure

A rider becoming unavailable or failing does not make the Wire Session
unavailable.

For example, this is valid:

    Wire = ACTIVE
    MPEG = FAILED
    RFB  = ACTIVE
    PCM  = ACTIVE

MPEG may contain and recover its own failure while Wire and healthy siblings
continue operating.

The same principle applies to RFB, PCM/audio, control/telemetry, and future
modules.

A module failure escalates beyond that module only when the module cannot
restore its own invariants or the failure has actually compromised a containing
owner's invariants, as defined by Q8.

### Readiness and failure ownership

Each distributed module owns:

- its own domain-specific establishment, if required;
- the evidence that its remote counterpart is usable;
- its own readiness state;
- its own runtime state;
- its own normal stop/retirement path;
- its own bounded failure handling;
- the conditions under which recovery must escalate under Q8.

Wire Transport owns none of those business-level meanings.

Wire provides the common active Transport through which the module may
communicate.

### Fault-containment consequence

Keeping module readiness below the Wire ownership boundary is deliberate fault
containment.

A module failure is normally contained and recovered by that module while Wire
remains ACTIVE and healthy siblings continue.

Only evidence that shared Transport or another containing ownership scope is no
longer trustworthy justifies upward escalation.

This directly reinforces Q8 minimum-scope recovery and Q10's minimal Wire
availability contract.

## Q12 — Transport-owned session validity and module lifecycle containment

STATUS=ANSWERED

### Wire contains the cross-Wire communication lifetime

A running module may perform cross-Wire work only while an active Wire Session
exists.

The module does not need to know the Wire Session identity.

Wire loss makes the current running instance terminal for future cross-Wire
work. A dead instance is never resumed, migrated, rebound, or adopted by a
replacement Wire Session.

### Module generation is conceptual by default

A module generation normally means one uninterrupted running instance.

The architecture does not require a numeric generation ID.

For the ordinary module:

    STOPPED -> RUNNING -> STOPPED

with STARTING or STOPPING phases added where the module needs them.

A later start is conceptually a new generation because the old running instance
ended, not because a counter was incremented.

### Stop completely before restart

The normal module lifecycle contract is:

> **A module completely stops its current running instance before starting its
> next running instance.**

Complete stop means previous-instance work can no longer mutate, publish into,
or otherwise interfere with the next instance.

Once complete retirement is established, ordinary module runtime storage may be
reused without a project-wide generation counter.

If a future module cannot establish complete retirement because some
asynchronous facility genuinely overlaps old and new work, that module may earn
additional module-specific fencing.

### Wire availability and module policy remain separate

An active Wire Session makes cross-Wire communication possible.

It does not universally start every module.

Each module owns its own start condition.

RFB, PCM/audio, and remote input forwarding may include new-session availability
in their normal startup conditions.

MPEG is deliberately different. MPEG requires Wire before cross-Wire MPEG work
can occur, but Wire Session establishment is not itself the MPEG start trigger.
Current MPEG startup is user/calibration driven. A future autonomous
implementation may use an MPEG-owned detection condition.

The rule is:

    Wire availability
        +
    module-owned start condition
        =
    module may start

Wire Transport does not own the second term.

### Transport owns the validity of every Wire transaction

Modules do not compare Wire Session IDs to decide whether old traffic belongs
to a replacement session.

Transport owns that correctness boundary.

When Transport grants a running module access to cross-Wire work, that access is
valid only for the Wire Session in which Transport issued it. Every operation
admitted through that access belongs permanently to the same Transport validity
domain.

A useful mental model is a round-trip ticket:

    Wire Session A
        |
    Transport admits operation
        |
    operation receives A-only validity

The exact implementation may use an opaque access object, capability, runtime
identity, epoch, pointer identity, or another Transport-private mechanism.

The module need not know the concrete identity.

### Session termination invalidates old Transport authority

If Wire Session A ends, Transport access issued under A becomes terminal.

That remains true even after Wire Session B is active.

No new cross-Wire leg may be started through A after that point, and work still
waiting on A's Wire lifetime may not complete by silently using B. Old A work may
not be sent through B, retried automatically through B, rebound to B, interpreted
as B work, or allowed to consume B-owned inbound state.

Work whose Wire exchange already completed while A was valid is simply final
valid A work. If a local continuation still exists after A ends, it remains part
of the old module instance and is retired by that module's stop path; it is not
reclassified as B work.

The concrete opaque ticket is module-facing authority, not necessarily a literal
tag attached to every byte or callback. The invariant is ownership: a Wire
exchange valid under A never becomes valid under B.

### Transport is the final stale-work fence

Normal module behavior stops promptly when Wire becomes unavailable.

Transport nevertheless remains the final cross-Wire backstop if an old module
worker has not yet observed shutdown and tries to initiate more remote work.

For example:

    old module instance is still unwinding
        |
    old Wire Session is gone
        |
    replacement Wire Session is current
        |
    old worker attempts new cross-Wire work

The old worker still holds only its old Transport authority. That authority is
terminal, so the attempted operation is rejected before it can become B work.

This is the final communication-validity fence if something from the old module
instance "bolts out the gate" before normal shutdown catches it.

Transport does not thereby become the owner of that module's local workers,
callbacks, queues, or retirement process. Those remain module-owned.

### Transport documentation depends on module-owned complete retirement

The complete cross-session safety argument has two owners and both are required:

    Transport
        -> stale A authority cannot communicate through B

    module lifecycle
        -> old A local work completely stops before replacement module
           startup or reuse of module-owned live resources

The second rule is a module invariant, not a Transport implementation job.

Transport documentation states it explicitly because it is part of the system
logic at this boundary. It must not remain only implicit "between the lines."

Accordingly, Transport does not add a generic project-wide admitted-call counter,
module generation counter, or module-retirement manager merely to enforce this
rule. If a particular module cannot establish complete retirement, that module
must add the smallest module-specific fence required by its own asynchronous
behavior.

The reusable minimum module requirements are maintained in:

    docs/development/module-lifecycle.md

### Reconnect uses ordinary module startup

A replacement Wire Session does not cause an old module instance to continue.

Instead:

    Wire A active
        |
    module ordinary startup if its own condition is true
        |
    module running
        |
    Wire A ends
        |
    module ordinary stop
        |
    module fully stopped
        |
    Wire B active
        |
    module ordinary startup if its own condition is true

The first connection and the hundredth reconnect use the same module lifecycle
mechanisms.

Application/module policy chooses which modules should start; it does not
migrate old runtime objects.

### Mixed local and cross-Wire modules

A module family may contain both local and cross-Wire responsibilities.

Wire loss ends only behavior whose validity depends on Wire.

For example, local controller/input acquisition may remain alive while remote
mouse/key forwarding stops.

The module boundary makes that distinction explicit instead of treating all
local behavior as disposable merely because remote forwarding is unavailable.

### Durable state may survive; live runtime does not

Configuration, confirmed calibration, user preferences, and other explicitly
durable state may survive Wire loss.

A later module instance may reconstruct itself from durable state.

Live runtime state belonging to the dead instance does not silently survive into
the replacement instance.

### IDs are earned, not assumed

Wire Session IDs remain Transport-owned authority where Transport itself needs
them.

Module generation IDs, epochs, or other identifiers may be introduced when a
module has an independent demonstrated requirement for them.

Reconnect by itself is not sufficient justification.

The default is lifecycle ownership rather than copied identity.

### Developer contract

The reusable implementation and review rules for cross-Wire modules are
documented in:

    docs/development/module-lifecycle.md

Before implementing or materially changing a distributed module, its start
condition, stop condition, Wire-loss behavior, complete-retirement condition,
durable state, and Transport-validity boundary must be understood.

### Implementation status

The product Transport bridge issues opaque module-facing access bound to the
Wire lifetime in which it was acquired. RFB, PCM/audio, and MPEG carry only that
opaque access rather than copied Wire Session IDs.

Host regression coverage proves that stale access A is rejected after Session B
is established and cannot invoke B's logical runtime operations.

Real-PS2 run `Q12-STALE-HW1-20260920T011710Z` proves the same Transport boundary
in both directions: stale A outbound traffic does not enter B, a stale A read
does not consume known B inbound data, and fresh B access remains usable.

The earlier interpretation that Q12 additionally required a generic
Transport-owned admitted-call drain was rejected after ownership review. That
would move the module's complete-stop-before-restart responsibility into
Transport. Complete local retirement remains a mandatory module invariant and
must be proven by each module where its lifecycle is implemented or changed.

Current classification:

    Q12_TRANSPORT_SESSION_VALIDITY_HOST_PROVEN=YES
    Q12_TRANSPORT_SESSION_VALIDITY_HARDWARE_PROVEN=YES
    STALE_A_OUTBOUND_FENCING_HARDWARE_PROVEN=YES
    STALE_A_INBOUND_NONCONSUMPTION_HARDWARE_PROVEN=YES
    FRESH_B_ACCESS_HARDWARE_PROVEN=YES
    MODULE_COMPLETE_STOP_BEFORE_RESTART=REQUIRED
    MODULE_LIFECYCLE_PROOF_SCOPE=MODULE_OWNER

This does not change the separate Q4 limitation: Wire establishment in the Q12
DUT was proof-local, so the generic exact product ELF is not thereby hardware
proven.

Result authority:

    experiments/wire-q1-q12-proof/PROOF5_Q12_STALE_ACCESS_HW1_RESULT.md


## Historical/current state versus target state

Do not silently rewrite history to make the old implementation appear to have
had the mature ownership model.

When old service files or documents say that the direct RFB endpoint is the
PS2-facing product boundary, preserve that statement as historical/current
evidence where appropriate and explicitly record that the mature Wire server
supersedes that architectural role.

Likewise, standalone historical audio/RFB listeners may remain useful forensic
or migration evidence, but they are not justification for parallel rider-owned
product sockets in the mature design.

## Next unresolved question

Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, Q10, Q11, and Q12 are closed.

The next architecture question is intentionally not answered by this revision.
It should be decided from repository/runtime evidence and the mature design
principles before implementation, then appended here rather than left only in
conversation.
