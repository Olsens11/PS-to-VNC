# Ledge Wire Runtime Decisions

DOCUMENT=LEDGE_WIRE_RUNTIME_DECISIONS
DOCUMENT_REVISION=0003
BRANCH_SCOPE=ledge/h1-all-guns
DECISION_STATUS=GOVERNING_FOR_ACTIVE_A003_MANUAL_RECONSTRUCTION
BASED_ON_BRANCH_COMMIT=d67b7c4de53f5eb6342fc075fdaf3f9cfbfb2f50

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

Q1, Q2, Q3, and Q4 are closed.

The next architecture question is intentionally not answered by this revision.
It should be decided from repository/runtime evidence and the mature design
principles before implementation, then appended here rather than left only in
conversation.
