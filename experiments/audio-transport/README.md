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
