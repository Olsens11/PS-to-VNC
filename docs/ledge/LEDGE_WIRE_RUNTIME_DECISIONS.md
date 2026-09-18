# Ledge Wire Runtime Decisions

DOCUMENT=LEDGE_WIRE_RUNTIME_DECISIONS
DOCUMENT_REVISION=0001
BRANCH_SCOPE=ledge/h1-all-guns
DECISION_STATUS=GOVERNING_FOR_ACTIVE_A003_MANUAL_RECONSTRUCTION
BASED_ON_BRANCH_COMMIT=6c416293435203ea77a7afb95db218f3b5a977fe

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

Q1 and Q2 are closed.

The next architecture question is intentionally not answered by this revision.
It should be decided from repository/runtime evidence and the mature design
principles before implementation, then appended here rather than left only in
conversation.
