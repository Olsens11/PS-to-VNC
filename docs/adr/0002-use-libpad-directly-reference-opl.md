# ADR 0002: Use libpad directly and reference OPL for pad practice

## Status

Accepted — 2026-09-03

## Context

Issue #38 begins restoration of controller/input behavior around the qualified
minimal PS2/VNC core.

Early design discussion considered introducing a PS-to-VNC controller
translation layer over PS2SDK `libpad`. Inspection of libpad itself and mature
PS2 homebrew practice showed that such a layer would mostly rename concepts
that already have an established PS2 vocabulary.

libpad already supplies the fundamental controller representation and
mechanisms PS-to-VNC needs: port/slot addressing, pad state, `padRead()`,
`struct padButtonStatus`, `PAD_*` button masks, analog values, mode handling,
pressure support, and actuator support.

Open PS2 Loader (OPL) provides a mature real-world example of using these
mechanisms, while also containing OPL-specific application policy that
PS-to-VNC should not inherit automatically.

## Decision

### libpad is the primary controller API authority

PS-to-VNC uses PS2SDK `libpad` directly for controller facts and mechanisms.

Do not create PS-to-VNC constants, structures, or wrappers merely to rename an
equivalent libpad concept. Standard concepts such as `padRead()`,
`padGetState()`, `struct padButtonStatus`, `PAD_CROSS`, port/slot identity, and
raw analog fields remain recognizable as libpad vocabulary.

This is an intentional PS2-specific dependency, not a portability defect.

### OPL is the preferred mature usage reference

Future pad/controller work follows this discovery order:

1. inspect PS2SDK libpad and its examples;
2. inspect how Open PS2 Loader uses the relevant mechanism in practice;
3. add only the smallest PS-to-VNC-specific responsibility still required.

OPL is a usage reference, not a structural template. OPL-specific `KEY_*`
translation, analog-to-D-pad policy, repeat timing, merged application input,
pad emulation, menu behavior, and other unrelated policy do not enter the
minimal core merely because OPL implements them.

### PS-to-VNC owns application-specific use, not a translation API

The first pad-facing source responsibility is `src/pad.c` / `src/pad.h`.

A project-owned instance may use the conventional project type name
`pstvnc_pad_t`, with one instance corresponding to one libpad `(port, slot)`
endpoint. Exact fields and function signatures remain implementation decisions.

The pad owner may contain only state genuinely required around libpad, such as:

- endpoint identity and the required aligned pad buffer;
- open/poll/lifecycle bookkeeping;
- the latest valid `struct padButtonStatus`;
- the immediately previous valid button observation when needed to identify
  physical press/release transitions;
- stale-history invalidation across disconnect, reacquisition, or explicit
  libpad ownership boundaries.

Multiple controllers are represented by multiple endpoint instances rather
than by hard-wiring one global controller assumption.

### Temporal interpretation belongs to consumers

The basic pad owner does not retain seconds of raw input history for future
gesture recognition.

It remembers only enough previous state to describe physical changes such as
down now, newly pressed, and newly released.

A future chord recognizer owns its own timing, candidates, growth/shrink,
latching, hold, and release state.

Chord semantics remain a separate, opt-in consumer. A path that does not use
the chord recognizer continues to receive ordinary controller facts.

Pointer/mouse interpretation is also separate. Deadzones, acceleration,
response curves, mouse buttons, wheel behavior, and context-dependent actions
do not belong in basic pad acquisition.

Haptic policy is separate as well. libpad remains the physical actuator
mechanism; future PS-to-VNC code decides when and why to use it.

### Preserve raw analog representation

Analog values remain the values supplied by `struct padButtonStatus`.

Do not create signed or otherwise normalized duplicates merely for aesthetic
consistency. A consumer that genuinely needs a centered coordinate system,
deadzone, response curve, or other transformation owns that interpretation.

### Comments are the human translation layer

Explain unfamiliar PS2/libpad mechanics where they are used rather than hiding
them behind redundant abstractions.

Useful explanations include:

- active-low button bits;
- port/slot meaning;
- pad DMA-buffer alignment;
- relevant `PAD_STATE_*` transitions;
- analog byte ranges;
- controller-mode negotiation;
- actuator setup when introduced.

Comments should explain non-obvious PS2 behavior rather than obvious C syntax.

### OPL provenance remains explicit

If source is literally copied or closely adapted from OPL, preserve the
applicable copyright, license, attribution, and modification notices. OPL is
distributed under the Academic Free License 3.0.

If PS-to-VNC independently implements the small required mechanism against
libpad after consulting OPL, record OPL as the mature implementation reference
without claiming that established practice was invented here.

No OPL source is imported merely by accepting this ADR.

## Consequences

PS-to-VNC controller code remains readable in the same vocabulary used by
PS2SDK documentation and established homebrew projects.

Community libpad examples remain directly applicable without a translation
table.

The project avoids duplicate button constants, duplicate analog
representations, and a speculative portability layer.

Pointer, chord, UI, binding, gamepad-forwarding, and haptic behavior can evolve
independently of physical pad acquisition.

Future contributors have an explicit discovery rule: libpad first, OPL second,
then the smallest earned PS-to-VNC-specific addition.

## Alternatives considered

### Create a complete PS-to-VNC controller abstraction over libpad

Rejected because it mostly renamed established libpad concepts and created
maintenance cost without an actual portability requirement.

### Copy OPL's entire pad subsystem

Rejected because OPL includes application policy belonging to different
PS-to-VNC responsibilities.

### Reconstruct the historical monolithic PS2VNC controller path

Rejected because the exploratory implementation coupled pad acquisition,
pointer behavior, chords, UI/context policy, RFB behavior, display behavior,
and other concerns that the clean reconstruction deliberately separates.
