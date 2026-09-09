# H1 Resident Media / RFB Harness

H1 is the experiment-only resident audio/video/RFB harness descended from two
qualified media parents and the clean through-Issue-39 PS-to-VNC source:

- EXP3/P11 MPEG-2 video playback;
- configurable Audio Transport EXP2 PCM playback/mux transport; and
- the clean through-Issue-39 RFB, input, UI, display and platform modules.

The parent/historical sources remain preserved. H1-specific descendants and
qualification scaffolding live here.

## Transport architecture lock

H1 does **not** return to competing PS2-facing TCP sockets.

One physical PSTV TCP connection currently carries independent logical channels:

- channel 0 — CONTROL;
- channel 1 — RFB;
- channel 2 — AUDIO;
- channel 3 — TELEMETRY; and
- channel 4 — MPEG2.

RFB was intentionally absent from the first media-harness profile so the first
combined workload could isolate audio + MPEG video. It has since been
reintroduced on logical channel 1 without adding a second physical PS2-facing
connection. CP2J qualified the headless RFB path, CP2K qualified visible RFB,
CP2L qualified visible RFB plus the existing PS2 mouse semantics, and CP2N now
qualifies the real through-Issue-39 local-controller/local-UI/OSK/keyboard path
over that same mux-backed RFB session.

The current hardware authority is CP2N visible RFB plus the real Issue-39
interaction composition. AUDIO and MPEG remained OFF for that qualification;
combined RFB/media workloads remain later controlled experiments.

This preserves the transport-mux lesson already learned by the audio work. A
future A/B comparison may change audio *encoding* or place compressed audio in
an MPEG program stream, but it will not use separate PS2-facing TCP sockets as
the H1 reference architecture.

## Configuration philosophy

Every connection carries one complete CONFIG v4 profile from the Pi. The PS2
ACKs the exact accepted bytes only after structurally validating the profile
and successfully allocating requested runtime resources.

H1 deliberately does not clamp experimental queue sizes, prefill depths,
delays, feed sizes or presentation offsets to known-good values. Invalid means
contradictory/unrepresentable for the current mechanism, not merely unqualified.
A freeze, crash, underrun, missed deadline, allocation failure or decoder error
at an aggressive profile is legitimate experimental evidence.

## Latency and synchronization

Buffering and synchronization are separate variables.

Buffer/resilience controls include independent audio and MPEG queue capacities,
initial credits, credit batching, startup targets and consumer/feed sizes.
RFB additionally has an independently configurable queue/credit policy. These
are consumer-capacity controls, not merely wire-bandwidth knobs.

Presentation controls include independent signed audio/video presentation
offsets around one shared media epoch. H1 therefore does not use queue prefill
as a surrogate lip-sync knob.

## First compatibility profiles

The first video-only profile reproduces P11's effective settings:

- MPEG queue 524288 bytes
- startup target 458752 bytes
- 2048-byte IPU feed
- 30000/1001 absolute scheduler
- RGB16 / PSMCT16 presentation
- no frame dropping
- 704x480 maximum decoded geometry
- 640x512 draw geometry

The first combined profile adds EXP2-compatible 48000 Hz / 16-bit / stereo PCM
on the AUDIO logical channel while retaining those P11 video settings.

The original media-harness hardware sequence was:

1. old qualified P11 control;
2. H1 video-only P11-compatible session;
3. repeated identical H1 video-only sessions without ELF restart;
4. H1 PCM-audio-only compatibility session;
5. H1 combined PCM + MPEG session on the same physical mux;
6. only then begin automated parameter sweeps.

Those steps are historical qualification context, not a statement that RFB is
still absent from H1.

## Reusable operator / optimization tool

`h1_tool.py` is the canonical human-facing control surface for H1 testing. It
wraps the current cumulative runner rather than reimplementing transport or
media behavior.

It provides:

- `knobs` -- list every adjustable field, current profile default and meaning;
- `show` -- resolve modifiers without starting hardware;
- `run` -- execute one parameterized resident-H1 session; and
- `sweep` -- generate and execute repeatable multi-parameter batches with a
  recorded plan, per-case evidence/logs and machine-readable results.

Frequently used settings have friendly switches such as `--audio-priority`,
`--capture`, `--encode`, `--draw`, and `--media-rect`. Every current/future H1
profile field remains reachable through `--set NAME=INTEGER`, while sweeps use
`--vary NAME=V1,V2,...`.

The current CP2N ELF is intentionally resident across sessions. After a clean
finite session it returns to its PSTV session-wait loop and automatically
reconnects when the next `h1_tool.py run` or sweep case opens the listener.
Ordinary repeated H1 tests therefore do not relaunch the ELF between cases.
This resident reconnect behavior is normal test-harness lifecycle behavior, not
a watchdog or silent-freeze recovery mechanism.

Use `h1_tool.py` as the default control path for subsequent profile exploration,
A/B testing, performance work, and stability sweeps. Bespoke runners should be
introduced only when the canonical tool/runner genuinely cannot express the
experiment.

The CLI is intentionally self-documenting:

```sh
python3 experiments/media-harness-h1/h1_tool.py --help
python3 experiments/media-harness-h1/h1_tool.py run --help
python3 experiments/media-harness-h1/h1_tool.py sweep --help
python3 experiments/media-harness-h1/h1_tool.py knobs \
    --profile P11_COMPAT_PLUS_PCM
```

See `H1_TOOL_GUIDE.md` for the compact operator guide.

## Current RFB checkpoint authority

The machine-readable experiment state is `RFB_MUX_PREP_STATUS.env`. The current
hardware-qualified boundary is CP2N visible RFB plus the real through-Issue-39
mouse, local-controller, local-UI, OSK, and keyboard interaction composition.
The exact qualification record is:

    RFB_MUX_CP2N_HARDWARE_RESULT.md

The corresponding candidate/source authority remains documented in:

    RFB_MUX_CP2N_VISIBLE_INTERACTION_CANDIDATE.md

CP2N intentionally replaced the wrong-abstraction CP2M transient L1+D-pad
keyboard experiment. It composes the existing Issue-39 input runtime,
local-controller router, local UI, OSK, keyboard builder, RFB session API,
display conversion and PS2 graphics interfaces with no new checkpoint-specific
controller gesture. The real-PS2 gate passed with exact ELF/PT_LOAD/deployment
identity, clean finite quiesce, clean machine validation, and full operator
interaction confirmation.

The operator noted somewhat sluggish/laggy cursor response. That is retained as
a later performance/profile-optimization observation rather than a qualification
failure. H1 profiles and reproducible sweeps are the intended mechanism for
characterizing and optimizing such behavior as workloads become more complex.

## Integration intent

H1 is a proving ground for upgrades to the clean PS-to-VNC program, not an
alternate product architecture. Proven runtime mechanisms should ultimately be
sorted into the existing clean ownership/domain structure and integrated at the
narrowest correct boundaries. Existing production modules should be reused
through their normal interfaces rather than reimplemented inside the harness.

H1 development tooling may remain useful in the repository for parameter
exploration, profiles, sweeps, evidence capture, qualification and later
re-optimization even if that machinery is not included in the production ELF.
Production may consume selected qualified profiles, including potentially
mode-specific profiles for future multiple-display-mode support.

Temporary experiment scaffolding must not become production structure by
inertia. Before extending user-facing runtime behavior beyond the current
qualified boundary, read `H1_INTEGRATION_INTENT.md`; it records the required
surgical-integration direction, cleanup/classification rules, profile philosophy,
shared-data-transport direction, future bulk-data compatibility constraint,
naming expectations, and the current disposition of CP2M.

## Future hybrid media-object composition note

The 2026-09-08 geometry experiments motivated a future model in which media is
represented as a semantic desktop object attached to a host window rather than
as a macroblock-aligned MPEG rectangle. The concept includes generic dynamic-
region discovery, exact host-relative media geometry, codec-only padding,
optional shape/visibility masks for rounded corners, and eventual compositor
layering of RFB -> MPEG media object -> local cursor/OSK.

This is a design note only; it is not current implemented behavior and does not
begin Issue #40.

See `MEDIA_OBJECT_COMPOSITION_NOTES.md` for the full rationale and proposed
separation between detection, host attachment, semantic geometry, codec
surfaces and presentation.
