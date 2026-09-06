# PS2VNC Dedicated-Link Transport Experiment

## Status

Experimental only.

This workstream is intentionally isolated from the qualified PS2VNC product
authority. Its purpose is to learn what a future PS2-specific transport layer
needs to provide before those requirements influence later product
architecture.

The experiment is not production integration and must not silently redefine
the behavior or authority of completed reconstruction work.

---

## Statement of Experimental Intent — v0

The PS2↔Pi Ethernet connection is a dedicated point-to-point device
interconnect, not a conventional shared network.

We own both endpoints and all meaningful traffic on the wire. Conventional
TCP/IP behavior is therefore an implementation option rather than an
architectural constraint.

This experiment will explore a PS2-specific transport layer that can exploit
exclusive ownership of the link to provide:

- deterministic scheduling;
- bounded flow control;
- application-level backpressure and credits;
- channel-aware priority and delivery semantics;
- explicit sequencing and observability;
- health and progress telemetry;
- controlled recovery behavior; and
- a stable abstraction above the underlying wire protocol.

Complex policy should live primarily on the Raspberry Pi. The PlayStation 2
endpoint should remain small, deterministic, bounded, and observable.

The transport must remain payload-agnostic. It should be capable of carrying
current RFB and audio traffic and, eventually, MPEG-2 audio/video,
input/control traffic, telemetry, and other project-defined logical channels
without forcing those subsystems to depend on a particular underlying
transport backend.

TCP is an acceptable first experimental backend because it allows the
multiplexing, scheduling, flow-control, and instrumentation ideas to be tested
with minimal unrelated change.

TCP is not assumed to be the final answer.

Future evidence may justify another backend, including UDP or a purpose-built
raw Ethernet protocol. The logical transport interface should be designed so
that such a change does not require application subsystems to be redesigned.

This statement is a hypothesis and design intention, not a finalized
protocol. It is expected to evolve when experimental evidence shows that an
assumption is wrong, unnecessary, or incomplete.

---

## Modular Integration Intent — v0

The transport experiment must connect to PS2VNC through explicit, narrow
interfaces rather than restructuring working subsystems around the
experiment.

Existing RFB parsing and rendering, input, local UI, controller, platform, and
future media components should remain independently understandable,
testable, and replaceable.

The transport is a service beneath those subsystems. It is not a new
monolithic application architecture.

Application consumers should not need to understand transport framing or the
underlying wire backend.

For example:

- the RFB subsystem should continue to consume an ordered RFB byte stream or a
  deliberately defined higher-level RFB adapter;
- the audio subsystem should consume audio payloads without parsing transport
  headers;
- future MPEG code should consume MPEG payloads without depending on TCP,
  UDP, or raw Ethernet details;
- input/control code should submit logical messages rather than writing
  directly to transport sockets.

The application-facing interface should describe logical intent, not backend
mechanics.

Conceptually:

                 existing PS2VNC modules
                          |
              narrow transport interface
                          |
                logical channel API
                          |
                 transport backend
                          |
                 dedicated Ethernet

A future backend change from TCP to UDP or raw Ethernet must not require RFB,
audio, MPEG, controller, or UI subsystems to be redesigned.

Scheduling and policy should remain primarily Pi-side. The PS2 transport
module should stay small and concentrate on framing, bounded queues,
demultiplexing, progress accounting, and reporting available capacity.

Instrumentation must preserve subsystem boundaries. Transport counters are
transport counters; RFB progress is RFB progress; audio progress is audio
progress. Debugging should make boundaries more visible rather than merging
them.

Experimental implementation should remain visibly isolated until deliberate
graduation into product source. Permanent integration, if justified later,
must follow the project's normal modular source organization, build rules,
documentation, symbol dictionaries, and qualification process.

Generalize the interface where evidence supports it. Do not prematurely
generalize the implementation.

---

## Why This Experiment Exists

The first live desktop-audio prototype used two independent TCP connections:

- RFB/video on TCP 5900;
- 48 kHz signed 16-bit stereo PCM audio on TCP 5901.

Sustained media playback with the browser minimized was observed to remain
stable for multiple music videos.

When the browser was maximized, causing heavy moving RFB framebuffer traffic
while the same audio workload continued, the PS2 entered a partially wedged
state.

A Pi-side freeze snapshot showed:

- the Raspberry Pi remained healthy;
- Ethernet physical carrier remained present at 100 Mbps;
- X0tigervnc remained alive;
- the desktop audio service remained alive;
- both PS2-facing TCP connections remained established;
- both connections accumulated significant queued data;
- neither connection was receiving useful acknowledgement/progress from the
  PS2; and
- the Pi received no Ethernet bytes during a five-second observation window.

The PS2 local UI retained limited signs of execution during the failure, but
remote input and both network streams had stopped making visible progress.

This does not prove that TCP or PS2IP is the root cause.

It does demonstrate that allowing independent producers to place unbounded or
poorly coordinated work onto the PS2-facing link is an architectural risk
worth investigating before future performance work.

---

## First Experimental Question

Can one bounded, multiplexed PS2-facing transport remain stable under a
combined workload that causes the current independent RFB and audio
connections to fail?

The first prototype should deliberately preserve the already-tested audio
payload:

- 48000 Hz;
- signed 16-bit;
- stereo;
- raw PCM.

Changing audio compression at the same time as changing transport would
confound the result.

Compressed audio and MPEG-2 media are later experiments.

---

## Initial Logical Channels

The first wire protocol should support at least:

1. `CONTROL`
2. `RFB`
3. `AUDIO`
4. `TELEMETRY`

The design should reserve clean extension space for a future `MPEG2` channel.

Logical channels must not imply separate TCP sockets.

---

## Initial Transport Properties

The prototype should investigate:

- one PS2-facing connection;
- explicit framing above the byte-stream backend;
- bounded per-channel queues;
- bounded aggregate outstanding work;
- small scheduled transmission quanta;
- audio deadline awareness;
- RFB deferral when the PS2 is behind;
- application-level consumption credit;
- sequence numbers;
- progress counters;
- queue high-water marks;
- heartbeat/health information; and
- instrumentation sufficient to identify the lowest layer still making
  progress during a failure.

TCP acknowledgement alone is not sufficient evidence of application
consumption.

Where practical, flow control should represent work actually accepted or
retired by the PS2 application.

---

## Device-Bus Model

The intended conceptual model is:

                 Raspberry Pi
                      |
          +-----------+-----------+
          |           |           |
         RFB        AUDIO       MPEG2
          |           |         future
          +-----------+-----------+
                      |
              logical channels
                      |
          PS2-specific transport
                      |
             backend boundary
                      |
              dedicated link
                      |
                 PlayStation 2

The Raspberry Pi acts conceptually as the more capable bus master and policy
owner.

The PS2 endpoint should primarily:

1. receive a bounded transport frame;
2. validate and demultiplex it;
3. place payloads into bounded consumer queues;
4. process those queues through the existing subsystems; and
5. report capacity and progress.

---

## Research Precedents / Borrowed Design Principles

This experiment is intentionally informed by existing remote-display,
thin-client, real-time transport, and point-to-point device-bus designs.

The goal is not to copy another protocol mechanically. The goal is to avoid
reinventing solutions to already-understood classes of problems and to borrow
mechanisms whose purposes match the constraints observed on the PS2.

### Sun Ray / Appliance Link Protocol

Sun Ray is the closest architectural precedent found so far.

Oracle documents ALP-RENDER as UDP traffic carrying on-screen drawing, user
input, and audio between the Sun Ray server and thin client.

Sun Ray also evolved accelerated multimedia paths in which supported
compressed video was redirected away from the ordinary desktop path and
decoded/rendered by the client. MPEG-2 was among the supported accelerated
formats. Compressed audio formats were also supported.

Borrowed principle:

- treat the client link as a purpose-built thin-client interconnect;
- keep ordinary desktop presentation separate from accelerated media;
- allow the capable host to redirect media into a more appropriate payload
  path when available.

This is especially relevant to the planned PS2VNC model of ordinary RFB
desktop rendering plus future MPEG-2 accelerated video.

### SPICE

SPICE provides several directly relevant flow-control mechanisms.

Its protocol defines distinct logical channels including display, input,
cursor, playback audio, and recording audio.

SPICE can request acknowledgements after a configured window of messages.
Those acknowledgements are based on messages consumed by the client, which is
more useful than merely knowing that TCP accepted bytes.

SPICE also defines ping/pong progress measurement and token-based flow control
for some bidirectional data.

Borrowed principle:

- expose logical channels explicitly;
- measure application consumption, not merely socket delivery;
- use bounded receiver-advertised permission to send more work;
- include protocol-level health/progress telemetry.

### Microsoft RDP Dynamic Virtual Channels

RDP Dynamic Virtual Channels multiplex multiple logical functions through a
common remote-desktop transport.

Version 2 of the Dynamic Virtual Channel extension introduced priority
classes specifically so different virtual channels can receive different
bandwidth allocations.

Borrowed principle:

- multiplexing does not imply equal scheduling;
- channel priority and bandwidth allocation are first-class transport policy;
- control/input and time-sensitive media should not compete identically with
  bulk display traffic.

### PCoIP

PCoIP uses UDP for session data and applies reliability according to the type
of information being transported.

HP documentation describes guaranteed handling for information that must be
delivered, such as USB data, while compressed audio and pixel information may
use selective retransmission so stale real-time information is not needlessly
delivered after its useful time has passed.

PCoIP also performs traffic adaptation based on available resources.

Borrowed principle:

- reliability is a property of the logical payload, not necessarily of the
  entire transport;
- stale real-time media may be less valuable than newer media;
- transport policy should consider timeliness as well as correctness.

For the first experiment, the RFB byte stream remains reliable and ordered.
No arbitrary RFB bytes may be discarded because doing so would destroy RFB
stream synchronization.

Future semantic RFB adapters may investigate whether complete obsolete
rectangle/update work can be coalesced or superseded safely.

### PCI Express

PCI Express provides a useful device-bus analogy because its flow control is
point-to-point and credit based.

The receiver advertises available buffering as flow-control credits. The
transmitter is constrained by the capacity the receiver has made available.

Borrowed principle:

- PS2 capacity should be explicit;
- the Pi should not infer unlimited capacity merely because its local socket
  accepted another write;
- the PS2 should advertise bounded work capacity and replenish credit as work
  is accepted or retired.

The experiment does not attempt to reproduce PCI Express. It borrows the
receiver-credit concept.

### USB

USB provides a useful scheduling and real-time-transfer analogy.

Periodic isochronous traffic is scheduled for timely service, while bulk
traffic uses remaining opportunity. Isochronous transfers deliberately avoid
retransmission because timeliness is more important than replaying stale data.

Modern USB specifications also preserve bandwidth for non-periodic traffic
rather than allowing periodic traffic to consume the entire service interval.

Borrowed principle:

- reserve regular service opportunities for deadline-sensitive payloads;
- do not allow bulk RFB traffic to starve audio/control;
- do not allow real-time traffic to starve all other work;
- timeliness and reliability may legitimately differ by channel.

### Initial Synthesis for PS2VNC

The first transport prototype should therefore investigate a deliberately
small combination of established ideas:

                 host-side producers
                       |
          +------------+------------+
          |            |            |
        CONTROL       AUDIO         RFB
          |            |            |
          +------------+------------+
                       |
              logical channel mux
                       |
             priority / scheduler
                       |
              receiver credit
                       |
             framed wire backend
                       |
                      PS2

Initial channel semantics:

- CONTROL / INPUT:
  reliable, ordered, tiny, latency-sensitive, never starved.

- AUDIO:
  bounded and deadline-sensitive. The first experiment keeps the existing
  48 kHz signed 16-bit stereo PCM payload unchanged so transport behavior can
  be isolated from codec behavior.

- RFB:
  reliable and ordered in the first experiment. It is bulk traffic and should
  yield scheduling opportunity to urgent control/audio work.

- TELEMETRY:
  small, observable, and sufficient to expose queue depth, sequence progress,
  credits, consumer progress, and health.

- MPEG2:
  reserved for later experimentation. Future MPEG-2 A/V should fit the same
  logical transport model without redesigning the transport boundary.

### Backend Implication

A single multiplexed TCP connection remains a useful first experiment because
it changes scheduling and aggregate queueing while preserving the familiar
PS2IP/TCP substrate.

It is not an architectural commitment.

The framing layer should be message-oriented enough that the same logical
transport frames could later be evaluated over UDP or a purpose-built raw
Ethernet backend without changing application modules.

### Reference Material Consulted

Primary/vendor documentation consulted for this stage:

- Oracle Sun Ray Software Administration Guide / Product Requirements:
  ALP ports and protocols.
- Oracle Sun Ray Software Administration Guide:
  video acceleration and multimedia redirection.
- SPICE Protocol documentation:
  channels, acknowledgement windows, ping/pong, and token flow control.
- Microsoft Remote Desktop Protocol Dynamic Channel Virtual Channel
  Extension:
  capability negotiation and priority classes.
- HP Anyware PCoIP Session Planning documentation:
  UDP transport, selective retransmission, and traffic adaptation.
- PCI-SIG PCI Express Basics and Background:
  point-to-point credit-based flow control.
- USB-IF Embedded USB2 supplement and USB architecture documentation:
  periodic/isochronous scheduling and transfer reliability semantics.

These sources are precedents, not normative specifications for PS2VNC.

---

## Experimental Discipline

This workstream must preserve causality.

Change one major variable at a time.

In particular:

- do not combine transport replacement with audio-codec replacement in the
  first test;
- do not interpret a successful prototype as production qualification;
- do not mask silent freezes with automatic recovery while root-cause work is
  active;
- record failures as evidence;
- preserve the current qualified product authority;
- keep experimental deployment paths distinguishable from normal runtime;
- instrument before attempting aggressive optimization.

---

## Planned First Comparison

Use the same media workload that exposed the current failure.

### Existing control

- 48 kHz PCM on independent TCP 5901;
- RFB on independent TCP 5900;
- several music videos with browser minimized;
- then maximize the browser to introduce sustained heavy RFB activity.

### Experimental candidate

- same desktop;
- same RFB semantics;
- same 48 kHz PCM payload;
- one bounded multiplexed PS2-facing transport;
- transport instrumentation enabled.

The central result is whether the multiplexed/bounded candidate survives the
combined workload materially better than the independent-socket control.

---

## Questions This Workstream Should Answer

1. Is the observed instability principally caused by independent concurrent
   PS2 network streams?
2. Does bounding aggregate outstanding work prevent the failure?
3. Does application-level credit provide useful stability beyond TCP
   backpressure?
4. What transmission quantum gives good audio continuity without allowing RFB
   bursts to monopolize the PS2 receive path?
5. Which PS2 subsystem becomes the limiting consumer under heavy combined
   traffic?
6. Does a single TCP backend remain sufficient once scheduling is controlled?
7. If not, does a different backend improve behavior enough to justify its
   complexity?
8. What requirements must Issue #40 and later architecture avoid violating so
   future transport integration remains clean?

---

## Evolution Log

### v0

Established the dedicated-link/device-bus premise and the initial goal of
testing a bounded, multiplexed transport using unchanged 48 kHz PCM plus RFB.

No conclusion has yet been reached about the final wire protocol or backend.

### v0.1 — modularity and precedent research

Added the requirement that the transport integrate through narrow,
replaceable module boundaries rather than reorganizing PS2VNC around the
experiment.

Surveyed established thin-client, remote-display, real-time transport, and
device-bus designs. Sun Ray/ALP, SPICE, RDP Dynamic Virtual Channels, PCoIP,
PCI Express flow control, and USB scheduling provide the principal precedents
for the first prototype.

The initial synthesis is intentionally conservative: preserve existing RFB
and PCM payload semantics, add bounded multiplexing and observability, and
treat the first TCP backend as an experimental substrate rather than a final
wire-protocol decision.
