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

## Host-Driven Experimental Policy — v0

The first PS2 transport ELF should provide a stable, deliberately generic
experimental substrate.

Whenever practical, choices about scheduling, pacing, queue utilization,
channel priority, batching, flow-control policy, and test behavior should
remain controllable from the Raspberry Pi so they can be changed repeatedly
without rebuilding or redeploying the PS2 ELF.

The governing distinction is:

    PS2 = mechanism
    Pi  = experimental policy

The PS2 should implement the mechanisms that are fundamental to safely
receiving and consuming transport work:

- versioned transport framing;
- capability negotiation;
- logical-channel demultiplexing;
- bounded receive queues;
- the narrow adapter into the existing RFB consumer;
- the narrow adapter into the audio consumer;
- control-message reception;
- truthful capacity and credit reporting;
- sequence and progress accounting;
- health telemetry;
- validation of lengths, channels, versions, and safe operating bounds.

The Pi should own policy that we expect to tune repeatedly:

- transmission quantum;
- aggregate outstanding-work ceiling;
- per-channel policy ceilings;
- scheduling algorithm;
- channel weights;
- pacing interval;
- batching behavior;
- queue target levels;
- audio service interval;
- RFB service interval;
- telemetry interval;
- experimental policy profiles.

A change to those Pi-side policy choices should not normally require a new PS2
ELF.

### Capability Negotiation

The PS2 endpoint should advertise what it actually supports rather than
requiring the Pi to infer behavior from build history.

The initial capability exchange should be able to describe information such
as:

- transport protocol version;
- PS2 experimental build identity;
- supported logical channels;
- maximum accepted transport payload;
- physical RFB queue capacity;
- physical audio queue capacity;
- control/telemetry capabilities;
- supported audio formats;
- optional feature flags.

Future PS2 ELFs may advertise additional capabilities such as MPEG-2 without
changing the meaning of existing capabilities.

### Physical Capacity Versus Experimental Policy

Physical capacity and experimental policy must remain distinct.

For example, if the PS2 truthfully reports that 12288 bytes are free in an
RFB receive buffer, the Pi may still choose an experimental policy that allows
only 4096 bytes of RFB work to remain outstanding.

Conceptually:

    actual PS2 capacity
            |
       reported credit
            |
      Pi policy ceiling
            |
      permitted workload

The PS2 must not falsify capacity merely to implement an experiment.

The Pi may deliberately use less than the available capacity while exploring
the stability/performance frontier.

### Runtime Configuration

Where practical, experimental policy should be changeable through the
CONTROL channel after connection establishment.

Representative tunable policy may include:

    RFB quantum
    audio quantum
    aggregate window
    channel weights
    pacing interval
    scheduling mode
    telemetry interval

The exact command set should remain small and versioned.

The goal is not to create a scripting engine or transport virtual machine.

Use fixed mechanisms with broad, safe parameters.

### Telemetry First

The first experimental ELF should expose substantially more structured
telemetry than the final product is expected to need.

Useful initial observations include:

- total transport frames received;
- total transport bytes received;
- last valid transport sequence;
- RFB bytes enqueued;
- RFB bytes consumed;
- current RFB queue occupancy;
- RFB queue high-water mark;
- audio bytes enqueued;
- audio bytes consumed;
- current audio queue occupancy;
- audio queue high-water mark;
- audio underflow count;
- control messages received;
- malformed-frame count;
- invalid-length count;
- invalid-channel count;
- credit-policy violations;
- main/application heartbeat;
- RFB consumer heartbeat;
- audio consumer heartbeat.

Telemetry should be machine-readable on the Pi and should preserve subsystem
boundaries.

The objective is to turn failures into evidence about which boundary stopped
making progress.

### Experimental Distillation

The experimental framework is not automatically the future product
implementation.

Once repeated tests identify which mechanisms materially improve stability or
performance, preserve those mechanisms and discard unnecessary experimental
machinery.

A useful outcome may therefore be:

    experiment mechanism        eventual decision
    --------------------        -----------------
    receiver credits            preserve
    bounded queues              preserve
    channel framing             preserve
    scheduler policy knobs      reduce to defaults/config
    excessive telemetry         retain useful subset
    unused scheduler modes      discard
    unnecessary checks          discard if evidence supports it

Permanent integration should be the distillation of experimental evidence,
not a wholesale import of the experiment.

### Governing Principle

Maximize experimental freedom on the Pi.

Minimize experimental machinery on the PS2.

Preserve only mechanisms that evidence shows are necessary.

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

### v0.2 — host-driven experimental policy

Established the rule that the first PS2 experimental transport implementation
should provide stable mechanisms while scheduling and tuning policy remain
primarily Pi-side.

Added capability negotiation, truthful receiver-capacity reporting,
runtime-configurable policy ceilings, structured subsystem telemetry, and the
requirement that later product integration preserve only experimentally
justified mechanisms.

The purpose is to allow many controlled Pi-side experiments against one known
PS2 ELF before additional experimental binaries are introduced.

---

## Live Experiment Record — through September 6, 2026

The preceding sections preserve the design intent that existed before the live
transport campaign. This appendix records what was subsequently implemented,
observed, and learned on real PlayStation 2 hardware. It is intentionally
chronological: later evidence should not be rewritten backward into earlier
hypotheses.

### Experimental authority and ports

The transport work remained isolated from qualified PS2VNC product authority.
The qualified Issue #39 application chassis was reused, but transport-specific
source stayed under `experiments/audio-transport/` and experimental deployments
were kept distinct from the normal rolling `mass:/0/PS2VNC.ELF`.

The principal live endpoints were:

- qualified/baseline RFB: PS2-facing TCP 5900;
- original independent PCM audio: PS2-facing TCP 5901;
- one-socket mux: PS2-facing TCP 5902;
- isolated RFB provider for the mux: localhost TCP 5903.

The first major mux work was developed on `experiment/audio-transport-mux`.
The later reusable configuration chassis lives on
`experiment/audio-config-exp2`.

---

## v0.3 — Implemented one-socket framed mux

The first prototype kept TCP as the backend but moved logical traffic onto one
PS2-facing connection on TCP 5902.

The version-1 transport uses:

- magic `PSTV` / `0x50535456`;
- fixed 16-byte headers;
- maximum frame payload of 8192 bytes;
- sequenced frames;
- `HELLO`, `CONFIG`, `DATA`, `CREDIT`, `TELEMETRY`, `HEARTBEAT`, and `ERROR`
  frame kinds;
- `CONTROL`, `RFB`, `AUDIO`, and `TELEMETRY` logical channels, with `MPEG2`
  reserved for later work.

RFB semantics were deliberately preserved. The existing RFB parser still
consumes an ordered byte stream, and once it begins a complete message its
exact-read semantics remain intact. The experiment changes transport framing,
queueing, scheduling, and flow control without making arbitrary RFB bytes
optional or discardable.

Receiver credit became the application-level backpressure mechanism. Credits
represent PS2 consumer capacity returned as queued bytes are retired; a TCP ACK
is not treated as proof that the PS2 application consumed the corresponding
payload.

Early experiments generally used 32768-byte RFB and audio queues, 4096-byte
audio chunks and credit batches, 8192/4096 RFB/audio quanta, and 48 kHz signed
16-bit stereo raw PCM.

---

## v0.4 — Mux survival improved, but small-reservoir audio shudder appeared

The original two-socket control had a strong terminal failure signature under
heavy moving-RFB plus audio load: the Pi remained healthy, the direct Ethernet
link remained 100 Mbps/full duplex, both PS2-facing TCP sockets remained
established with substantial queued data, but useful PS2 acknowledgements and
application progress stopped. Remote input died and the PS2 had to be reset or
the PS2-side application relaunched.

The one-socket mux materially changed that behavior. With a representative
scheduler using RFB/audio weights 12/4, minimized YouTube was stable and a
roughly 120-second maximized high-RFB run survived without the terminal freeze.
That run carried roughly 115 MB of RFB data and 24.7 MB of audio with transport
runtime error remaining zero.

However, the mux exposed intermittent recoverable audio shudder, and RFB/video
was less smooth than before audio was added. Shudder was also reproduced with
YouTube minimized, proving that heavy RFB competition was not required for the
brief audio problem.

A long minimized run of roughly 716 seconds measured approximately 186886 B/s
audio enqueued and 186869 B/s played against a nominal raw-PCM rate of 192000
B/s. Repeated queue drains occurred.

The correct conclusion at this stage was not that one TCP socket is inherently
better than two. The bounded mux was surviving the high-load failure much
better, but its small audio reservoir exposed another timing/flow-control
problem.

---

## v0.5 — Pi audio-boundary instrumentation and W16/W32 gating tests

The Pi daemon was instrumented to separate source starvation, host-buffer
blocking, socket-send blocking, policy-window gating, receiver-credit gating,
and PS2 consumer progress.

Important instrumentation commits include:

- `379ae3590e76a8928b10ae529fdedb1ac14188be` — Pi audio-boundary
  instrumentation;
- `dc011dbd858dd4e629ee9369e951b6d67543051b` — audio credit-arrival gap
  tracing.

The instrumented path records PipeWire read gaps, audio host-buffer blocking,
DATA-send blocking, host-buffer depth, receiver credit/outstanding bytes,
policy room, send budget, age since the last audio credit, and PS2 telemetry.

With a 16 KiB audio policy window against the then-32 KiB PS2 audio queue, the
immediate gate was overwhelmingly `POLICY_WINDOW`: 761 of 767 sampled blocked
states were policy-window limited, with 6 ready states and no `NO_CREDIT`
samples. Brief shudders still occurred.

Raising the policy window to the full 32768-byte physical/effective queue
changed the immediate gate. In the W32 test, 758 of 766 sampled blocked states
were `NO_CREDIT`, with 8 ready states and no policy-window gating. The Pi audio
host buffer remained supplied/full, while the PS2 queue could still drain.

This moved the investigation away from PipeWire/source starvation and toward
the credit/delivery/consumer boundary.

---

## v0.6 — Clean W32 window proved normal credit flow is also possible

A clean approximately 180-second W32 observation produced a useful negative
result. In that timed window there were no current-window audio credit gaps >=
100 ms, no PipeWire read gaps, no audio host-buffer blocks, no meaningful
PS2-facing send blocks, and no PS2 audio queue-zero event.

The Pi received and sent audio at about 191964 B/s, and PS2 enqueued, consumed,
played, and credited deltas all matched at 36679680 bytes. Transport runtime
error remained zero and the final PS2 audio queue contained 24576 bytes.

A larger lifetime maximum credit gap had occurred before the timed window. It
was deliberately not attributed to the clean capture. The experiment therefore
showed that the W32 design could also operate normally for substantial periods;
the intermittent event still needed to be caught while audible.

---

## v0.7 — Persistent queue-full audio-consumer stall captured

A separate persistent failure was captured in which audio became stuck while
RFB, mouse/local UI, and the mux transport remained alive.

The decisive state was:

- audio enqueued: 120537112 bytes;
- audio consumed: 120504344 bytes;
- audio played: 120504344 bytes;
- audio queue occupancy: 32768 bytes, exactly full;
- audio read-call count stopped;
- audio credits stopped;
- Pi audio host buffer filled and the scheduler reported `NO_CREDIT`;
- RFB continued to make progress;
- transport runtime error remained zero.

Accounting was internally exact:

    enqueued - consumed = 32768

and

    initial credit + consumed bytes = total credit sent

There was no evidence of lost queue accounting.

This established an important causal direction: in the persistent failure,
`NO_CREDIT` was a consequence of the audio consumer no longer retiring bytes.
It was not evidence that the Pi independently stopped a healthy consumer.

The defining signature of this failure is therefore a **full** audio queue with
consumer progress stopped.

---

## v0.8 — Audio-worker breadcrumb instrumentation

To distinguish transport starvation from a blocked/dormant audio worker, the
PS2 audio thread received explicit stage breadcrumbs in commit
`14fa01b6987773564d3bf12edcae41d9c698e39b` (`experiment: trace PS2 audio
worker progress`).

The diagnostic word uses the form `0xA1SSEEGG`, where `A1` identifies the
format, `SS` is the worker stage, `EE` is the private audio error, and `GG` is a
rolling generation byte.

Important stages include thread/audsrv/format/volume readiness, startup prefill
entry/return, loop/read/delay stages, `audsrv_wait_audio()` entry/return,
`audsrv_play_audio()` entry/return, played-byte recording, and explicit
error/stop/quit stages.

The rolling generation is essential. A single sample at `WAIT_ENTER` does not
prove a hang; healthy playback frequently samples that stage while generation
and counters continue changing. Repeated identical full breadcrumbs plus frozen
counters provide much stronger evidence of a blocked worker.

---

## v0.9 — Decisive recovering shudder localized to queue-empty starvation

The brief audible shudder was eventually captured directly and proved to be
qualitatively different from the persistent queue-full failure.

During the transient event the PS2 showed:

- audio queue occupancy: zero;
- enqueued == consumed == played;
- no in-flight audio chunk;
- breadcrumb at the empty-read/delay path;
- audio read calls and breadcrumb generation still progressing;
- no RFB workload was required to trigger it.

The worker was alive and repeatedly checking an empty queue.

At the Pi, the audio host buffer remained full/supplied, there was no matching
PipeWire source gap and no corresponding long PS2-facing send block, but
available audio credit remained zero. Credit/send totals stayed static while
the age since the last audio credit rose through roughly 262, 513, 763, 1014,
and 1264 ms. When credit arrived again, sending resumed, the PS2 queue refilled,
and playback recovered.

A particularly important boundary observation was that PS2-side consumed and
credit-sent accounting advanced while the Pi still did not see the returned
credit for about 1.26 seconds. That narrowed the unresolved pause to a boundary
after or below the EE application-level credit submission path: possible
regions include PS2IP/SMAP/TCP behavior, network delivery, TCP timing, or
Pi-side inbound processing. The evidence did not justify choosing one of those
as the root cause.

The defining signature of the recovering shudder is therefore an **empty**
audio queue with the worker alive and Pi audio already waiting to be sent.

---

## Two distinct audio failure signatures

The campaign established two opposite signatures that must not be conflated.

### Recovering transient

    PS2 audio queue = EMPTY
    audio worker = ALIVE / looping
    Pi audio source = BUFFERED
    Pi send budget = blocked by missing credit
    event = RECOVERS

The consumer is ready for work, but bytes are not reaching the queue in time.
A sufficiently large reservoir can absorb this pause.

### Persistent consumer stall

    PS2 audio queue = FULL
    audio worker progress = STOPPED
    Pi audio source = BUFFERED
    Pi send budget = NO_CREDIT
    event = PERSISTENT

The transport has delivered all work the consumer can hold, but the consumer
is no longer retiring it. Increasing the transport reservoir is not a root-cause
fix for this state.

This distinction is now part of the experiment's diagnostic model.

---

## v0.10 — Why W512 was tested

48 kHz signed 16-bit stereo PCM consumes exactly 192000 bytes/s. A 32768-byte
audio queue therefore contains only about 0.171 seconds of audio, far less than
the approximately 1.26-second pause captured above.

A 524288-byte queue contains about 2.73 seconds of PCM. Merely enlarging the
queue would not necessarily fill it because producer and consumer normally run
at nearly the same average rate, so the W512 diagnostic also introduced a
3-second startup delay to build a real reservoir before playback began.

This was an experimental mechanism, not a proposed final latency policy.

---

## v0.11 — W512 stability result

The W512 source state was committed as:

    15a879095cf6bc24df10268b6516bbb39c7a83ce
    experiment: test 512 KiB audio reservoir

The tested W512 ELF identity was:

    SHA256 f36b07af135a452062f8e8a554b9fc796111f7a457bdf8b131a5148b901dc9ce
    2728920 bytes

The PS2 audio queue and corresponding Pi audio policy window were both 524288
bytes. Other principal policy remained RFB/audio quanta 8192/4096, weights
12/6, RFB policy window 32768, RFB host buffer 524288, audio host buffer
262144, and sink-volume mirroring disabled.

### Minimized workload

Approximately ten minutes of YouTube playback with the browser minimized ran
with no audible shudder or recovering stall.

### Maximized/high-RFB workload

Approximately another ten minutes with maximized/windowed moving YouTube
content ran with no terminal freeze and no audible audio shudder.

Two performance costs were clearly observed:

- audio lagged video;
- video playback was less smooth than before audio was added.

The audio lag was expected because the diagnostic deliberately accumulated a
large reservoir. The RFB smoothness loss remained an optimization problem.

The strongest supported W512 conclusion is:

> A single bounded mux can carry RFB plus raw PCM audio through the tested
> workload that had terminally failed with the original independent-socket
> prototype when the audio path has enough reservoir to absorb the observed
> transient delivery/credit interruptions.

This is a stability result, not proof that the W512 values are desirable final
settings. It also does not prove that the original use of two sockets was by
itself the root cause, because the surviving mux simultaneously changes
aggregate scheduling, application credits, bounded queues, outstanding-work
policy, framing, and the number of PS2-facing TCP streams.

---

## Important negative findings

The captured recovering shudder was not explained by ordinary PipeWire source
starvation, Pi audio host-buffer starvation, synchronous sink-volume mirroring,
or a dead PS2 audio worker. Heavy RFB traffic was also not required to trigger
that transient.

Disabling volume mirroring removed an unnecessary synchronous policy action but
did not eliminate the brief shudder.

These negative findings matter because the terminal high-RFB failure, the
recovering queue-empty shudder, and the persistent queue-full consumer stall are
three different observations and should not be collapsed into one symptom.

---

## Experimental implementation ledger

Important experiment commits leading to the current state include:

- `863b40989e7d7f045172fbae70a314607ed89a86` — common framing/queue
  substrate;
- `569e8c17b4359e1e7d74c18e030497b13768d25a` — PS2 mux substrate;
- `3111489dc2ab614c1bb42ef06f5e9684b659de88` — Pi mux policy daemon;
- `85f74f5a95c6e829305be96327ce44b4bbd8b084` — generated Python-cache
  hygiene;
- `379ae3590e76a8928b10ae529fdedb1ac14188be` — Pi audio-boundary
  instrumentation;
- `dc011dbd858dd4e629ee9369e951b6d67543051b` — audio credit-gap tracing;
- `14fa01b6987773564d3bf12edcae41d9c698e39b` — PS2 audio-worker
  breadcrumbs;
- `15a879095cf6bc24df10268b6516bbb39c7a83ce` — 512 KiB audio reservoir
  experiment.

Git history remains the authority for exact commit subjects and details.

---

## v0.12 — Configurable EXP2 laboratory chassis

After the W512 pass, rebuilding a new ELF for every queue/latency combination
was recognized as an unnecessary experimental bottleneck.

The next governing rule became:

    PS2 ELF = mechanisms and interface constraints
    Pi      = operating point for this test

A new branch was created:

    experiment/audio-config-exp2

Its initial EXP2 implementation commit is:

    447143b31f026564e801da91f3422e83945db4fc
    experiment: add remotely configurable audio transport EXP2

The first EXP2 ELF identity is:

    PS-to-VNC-AudioTransport-CONFIG-EXP2.ELF
    SHA256 6ad5c3b54886c8c3de523c8dd879c0106707a62b15765029a72469441d2d95f2
    2755196 bytes

It was canonically deployed as:

    mass:/0/PS2VNC-mux-config-exp2-p0-6ad5c3b5.ELF

with the rolling experimental alias:

    mass:/0/PS2VNC-mux-config-exp2-current.ELF

### Connection-scoped CONFIG handshake

EXP2 makes the previously reserved `CONFIG` frame an active startup mechanism:

    PS2 connects
         |
    PS2 -> HELLO
         |
    Pi  -> complete CONFIG profile
         |
    PS2 validates profile
         |
    PS2 performs requested runtime allocations/socket options
         |
    PS2 -> exact CONFIG ACK
         |
    PS2 grants initial receiver credits
         |
    normal RFB/audio mux operation

Normal DATA scheduling waits for successful configuration acceptance. The ACK
echoes the complete CONFIG payload so the Pi can prove exactly what the PS2
accepted.

### Runtime allocation and no artificial queue ceiling

A provisional fixed-backing design was discarded before qualification. The
adopted EXP2 implementation allocates the actual requested RFB queue, audio
queue, audio consumer buffer, receiver-thread stack, and audio-thread stack at
runtime.

There is deliberately no project-defined RFB/audio queue-capacity ceiling below
what the running PS2 can actually allocate. If a deliberately aggressive
profile cannot be allocated, that failure is useful evidence about the real
memory envelope rather than a value rejected by an arbitrary experimental
limit.

Structural validation remains for contradictory or unrepresentable profiles:
for example, queue capacity must be nonzero, complete legal DATA frames must
fit, audio sizes must preserve sample-frame alignment, startup target and credit
batch cannot exceed their queue, thread priorities must be representable, and
the DATA payload cannot exceed the actual version-1 wire maximum.

The initial complete PS2-side profile can tune:

- RFB and audio queue capacities;
- RFB and audio credit batches and flush-on-empty behavior;
- audio startup mode, target, and explicit delay;
- audio consumer chunk size;
- audio and RFB idle delays;
- audio and receiver thread priorities and stack sizes;
- audio rate, channels, bit depth, and volume;
- maximum DATA payload within protocol limits;
- requested socket receive/send buffers;
- queue allocation order.

Pi-side scheduler weights, quanta, policy windows, host buffers, telemetry, and
audio-capture policy remain independently configurable.

The result is a reusable transport test chassis rather than a collection of
hardcoded W32/W64/W128/W512 ELFs.

---

## EXP2 Profile 0 — W512 regression profile

The first EXP2 profile intentionally reproduces the known-good W512 operating
point before tuning anything.

Profile 0 uses:

- profile id: 0;
- RFB queue: 32768 bytes;
- audio queue: 524288 bytes;
- RFB/audio credit batch: 4096 bytes;
- flush pending credit when a queue empties: enabled;
- audio startup mode: fixed delay;
- startup delay: 3000000 us;
- audio startup-target field: 524288 bytes;
- audio chunk: 4096 bytes;
- audio idle delay: 1000 us;
- RFB empty delay: 1000 us;
- audio thread priority/stack: 65 / 16384 bytes;
- receiver thread priority/stack: 63 / 16384 bytes;
- audio: 48000 Hz, 16-bit, stereo, volume 100;
- maximum DATA payload: 8192 bytes;
- socket-buffer overrides: disabled;
- allocation order: RFB then audio.

Pi policy remains:

- RFB/audio quanta: 8192/4096;
- RFB/audio weights: 12/6;
- RFB policy window: 32768;
- audio policy window: 524288;
- RFB host buffer: 524288;
- audio host buffer: 262144;
- sink-volume mirroring: disabled.

On the first Profile-0 hardware launch, operator observation was that nothing
visibly changed from the W512 build. That is the desired regression result: the
control architecture moved from hardcoded values to a Pi-supplied profile
without changing the known-good operating behavior.

---

## Current experimental interpretation

As of September 6, 2026, the strongest supported conclusions are:

1. The original independent RFB/audio prototype can enter a terminal combined
   PS2-side failure under heavy RFB workload while the Pi and physical Ethernet
   remain healthy.
2. A bounded one-socket mux has survived the tested combined workload that
   repeatedly exposed that failure.
3. Small audio reservoirs expose intermittent queue-empty underruns.
4. At least one captured underrun corresponded to about 1.26 seconds without
   timely audio credit reaching the Pi, despite healthy source buffering and a
   live PS2 audio worker.
5. A separate persistent audio failure exists in which the PS2 audio queue is
   full and the consumer stops retiring bytes; missing credit is downstream of
   that consumer stall.
6. A 512 KiB audio reservoir plus deliberate startup prefill eliminated audible
   shudder during the tested minimized and maximized workloads.
7. That large reservoir also produced obvious audio latency and did not restore
   RFB/video smoothness to the pre-audio condition.
8. The root cause of the approximately 1.26-second credit/delivery pause remains
   unresolved.
9. The work has not established that one TCP connection is inherently superior
   to two, nor that TCP is necessarily the final backend.
10. The immediate engineering problem has shifted from basic survival to
    mapping the trade space among audio latency, underrun resilience, RFB
    smoothness, memory use, scheduling, and transport stability.

---

## Immediate next direction

Future tuning should use the same EXP2 ELF while changing Pi-supplied profiles.
The first intended latency experiment keeps the actual audio queue at 512 KiB
but replaces the diagnostic three-second fixed delay with a queue-occupancy
startup target.

At 192000 bytes/s of PCM:

    64 KiB  ~= 0.341 s
    128 KiB ~= 0.683 s
    192 KiB ~= 1.024 s
    256 KiB ~= 1.365 s
    512 KiB ~= 2.731 s

A first proposed tuning point is a 512 KiB actual queue with a 256 KiB startup
target while keeping RFB queue, credits, quanta, weights, policy windows, host
buffers, thread parameters, audio format, and wire protocol unchanged.

Later profiles can independently explore RFB/audio queue capacity, policy
windows, scheduler weights, quanta, credit batches, audio chunk size, socket
buffers, worker priorities, and allocation order. One meaningful variable
should be changed at a time when the purpose is causal comparison.

EXP2 also intentionally permits aggressive memory-envelope experiments. If
future evidence suggests RFB buffering is limiting performance, the same ELF
can request progressively larger actual RFB queues until performance stops
improving, allocation fails, another subsystem becomes constrained, or the
application becomes unstable. That boundary is more informative than an
arbitrary compile-time ceiling.

---

## Experimental discipline carried forward

Future transport work should continue to:

- preserve qualified product authority and exact test-ELF identity;
- keep baseline and experimental deployment paths distinguishable;
- avoid overwriting the normal rolling `PS2VNC.ELF` during experiments;
- preserve exact RFB byte-stream semantics unless a later experiment
  deliberately changes that abstraction;
- distinguish transport progress, RFB progress, and audio-consumer progress;
- distinguish queue-empty starvation from queue-full consumer stalls;
- avoid treating TCP ACK as application consumption;
- avoid masking silent failures with automatic recovery while mechanism
  investigation is active;
- change one significant variable at a time when establishing causality;
- prefer Pi-side profiles over PS2 rebuilds for values EXP2 can already tune;
- preserve surprising failures as evidence instead of normalizing them away.

---

## Evolution Log — Continued

### v0.3 — framed mux and receiver-credit substrate

Implemented one PS2-facing TCP stream with logical framing, RFB/audio channels,
sequencing, bounded queues, receiver credits, Pi-side scheduling policy, and
structured telemetry while preserving existing RFB stream semantics.

### v0.4 — live mux survives high combined workload

Observed that the bounded mux could survive heavy moving-RFB plus PCM workloads
that had previously produced the terminal independent-socket failure. Also
observed intermittent audio shudder and reduced RFB smoothness.

### v0.5 — audio-boundary and credit instrumentation

Instrumented source reads, host buffering, PS2-facing send behavior,
policy-window gating, receiver-credit gating, and credit arrival. W16 and W32
experiments separated policy-window blocking from actual credit exhaustion.

### v0.6 — persistent queue-full consumer failure identified

Captured a persistent state with the PS2 audio queue exactly full while RFB and
the broader transport continued making progress. Accounting showed that the
lack of new audio credit was a consequence of stopped audio consumption.

### v0.7 — audio-worker breadcrumbs

Added explicit audio-thread lifecycle/blocking breadcrumbs and a rolling
generation counter so a sampled audsrv wait could be distinguished from an
actual persistent worker hang.

### v0.8 — recovering shudder localized to queue-empty starvation

Captured an audible transient with the PS2 audio queue empty, the worker alive,
the Pi source buffered, and no timely credit reaching the Pi for roughly 1.26
seconds. This separated the recovering shudder from the queue-full persistent
consumer stall.

### v0.9 — 512 KiB reservoir removes observed shudder

Expanded audio queue/policy window to 512 KiB and deliberately prefetched about
three seconds before playback. Roughly ten minutes minimized plus ten minutes
of maximized/windowed moving YouTube completed without audible shudder or
terminal freeze. Audio lag and reduced video smoothness remained.

### v0.10 — configurable EXP2

Replaced one-ELF-per-setting iteration with a reusable connection-configured
laboratory ELF. RFB/audio queues, worker buffers, and experimental stacks are
runtime allocated from the Pi-supplied profile with no artificial queue-size
ceiling. Profile 0 reproduces the known W512 operating point and appeared
behaviorally unchanged on its first hardware regression launch.
