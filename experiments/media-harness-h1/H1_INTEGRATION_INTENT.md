# H1 Integration Intent

## Status and purpose

This document records the architectural intent that must guide the H1 resident
media / RFB-over-mux experiment as it matures.

H1 is a proving ground for an upgrade to the clean PS-to-VNC project. It is not
an alternate product architecture and it must not gradually become one.

The clean product architecture remains authoritative. In particular, read this
intent together with:

- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- the semantic audit / ownership documents referenced by the clean architecture;
- `experiments/media-harness-h1/H1_TOOL_GUIDE.md`; and
- the H1 hardware-result / transport-evidence documents.

This document does not begin Issue #40 and does not change any hardware
qualification claim by itself.

---

## 1. The product is being upgraded, not replaced

The intended end state is the existing clean PS-to-VNC program with proven new
capabilities integrated into it.

The central H1 transport change is an upgrade of the transport beneath existing
functionality: one physical PSTV TCP connection carries independent logical
channels, including RFB, audio, MPEG-2 and control/telemetry traffic. RFB already
exists as a product responsibility. H1's job is to prove that the RFB byte
stream and its existing product behavior can operate correctly over the mux
while audio and MPEG-2 coexist beside it.

The mux must therefore become a clean transport capability of PS-to-VNC, not a
reason to invent a second RFB application around it.

Where new capability genuinely requires production changes, those changes
should occur at the narrowest correct boundary. Examples include transport
adaptation, shared resource/scheduling policy, media presentation/composition,
and configuration needed for the new channels. Unrelated domains must not be
redesigned merely because H1 is the environment in which the new capability was
proven.

---

## 2. Existing clean module ownership is authoritative

The clean source structure and responsibility boundaries are the target shape
for surviving runtime code.

Mouse, controller/input, keyboard, OSK, local UI, RFB/session, graphics/display,
configuration, audio, MPEG-2, transport/networking and other responsibilities
must remain independently inspectable and maintainable according to the clean
architecture.

H1 must reuse the real modules and their public interfaces wherever possible.
It must not create H1-specific copies of behavior that a production module
already owns.

Examples of the intended rule:

- if the existing mouse/input path already converts semantic controller input
  into remote pointer behavior, H1 should instantiate and exercise that path;
- if the keyboard module already produces RFB key events, H1 should route those
  existing events through the upgraded RFB transport rather than inventing an
  H1 keyboard behavior;
- if OSK already invokes the keyboard machinery, H1 should eventually exercise
  that relationship rather than implementing a second OSK-to-RFB path;
- mux framing belongs to transport and must not leak into controller, mouse,
  keyboard or OSK behavior;
- MPEG-2 and audio policy must not leak into unrelated input/RFB modules merely
  because all channels share one physical connection.

A thin adapter is acceptable where the new transport or experimental harness
must meet an existing production interface. The adapter must adapt the boundary,
not duplicate the domain.

---

## 3. Surgical integration is the acceptance standard

Working on hardware is necessary but is not, by itself, sufficient for code to
be considered ready for production integration.

Every surviving H1 mechanism must ultimately pass two tests:

1. **Behavioral proof:** it works under controlled testing and, where hardware
   facing, on the real PS2.
2. **Architectural fit:** it has a clear owner and a narrow integration point in
   the clean project structure.

A hardware PASS with experiment-specific duplication is valuable evidence, but
it is not final production code.

The desired final integration should be reviewable feature by feature. It
should be possible to identify a small, coherent set of transport changes, a
small set of audio changes, a small set of MPEG/presentation changes, and so on.
The final upgrade must not arrive as one opaque H1 monolith.

Where new code interacts with original clean code, the integration should be
surgical: retain the existing responsibility, change only the interface or
mechanism genuinely affected by the new capability, and preserve independent
inspection/testing of the surrounding modules.

---

## 4. H1 should converge toward production-shaped code

During exploration H1 is allowed to contain temporary scaffolding, specialized
coordinators, instrumentation, qualification entry points and deliberately
exposed knobs. That freedom is useful for finding the correct mechanism.

It is not permission for temporary structure to become permanent by inertia.

As mechanisms become understood, H1 runtime code should be continuously
classified into three groups:

### A. Production functionality to integrate

Proven mechanisms that belong in the product should be teased apart and moved
or reshaped into the appropriate clean production ownership/domain.

Examples may include the mux transport, channel backpressure/credit machinery,
qualified audio mechanisms, MPEG decode/presentation mechanisms, and future
composition support.

### B. Development / qualification tooling to retain

Some H1 infrastructure is valuable precisely because it is *not* normal product
runtime. `h1_tool.py`, profile exploration, sweeps, evidence capture,
qualification runners, instrumentation and controlled experimental overrides may
remain first-class repository development tools even if none of that machinery
ships in the production ELF.

The important rule is that retained development tooling should exercise the
same production modules and interfaces whenever practical rather than retaining
parallel implementations of them.

### C. Vestigial experimental scaffolding to discard

Temporary duplicated behavior, obsolete coordinators, superseded runners,
one-off diagnostic implementations and abandoned experiment paths must be
removed once their evidence has been preserved and their useful lesson has been
integrated elsewhere.

Nothing earns a permanent place merely because development effort was spent on
it.

---

## 5. H1 tooling and production are related but distinct

H1 may remain a durable engineering environment after the mux/media work is
integrated.

Its long-term value can include:

- parameter discovery and optimization;
- repeatable A/B tests and sweeps;
- queue/credit and thread-priority tuning;
- media geometry/timing experimentation;
- regression qualification after source changes;
- evidence capture and comparison against known-good profiles; and
- re-optimization when future features or display modes change the workload.

This does not imply that H1 tooling belongs in the production executable.
Development may expose a large parameter space while production consumes only a
small set of selected, qualified settings.

---

## 6. Profiles may bridge development and production

Profiles are a likely clean way to carry qualified H1 findings into production.

H1 can explore many temporary combinations. Once a configuration is selected
and qualified, production may consume a deliberately limited profile rather
than scattering the resulting values throughout unrelated source files.

This is especially relevant to the planned multiple-display-mode feature. A
production display mode may naturally correspond to a validated bundle of
settings such as framebuffer/display geometry, RFB geometry, media capture/draw
geometry, memory/queue sizing and any timing values that prove genuinely
mode-dependent.

Not every H1 knob must become a production profile field. Some findings may be
universal production defaults, while others may vary by display mode or other
real product configuration. H1 exists partly to discover that distinction.

The preferred relationship is:

> H1 explores profiles; qualification promotes selected profiles; production
> consumes the selected profiles through normal clean module interfaces.

Development profiles should remain useful engineering/history records even if
the production build ships only a small qualified subset.

---

## 7. Hardware qualification and production readiness are separate records

The project must continue to preserve exact source/build/ELF/PT_LOAD identities
for hardware claims.

At the same time, hardware qualification must not be used to bless an
experiment-specific architecture accidentally. A checkpoint may prove a
mechanism and later be re-expressed using cleaner production modules. The old
hardware evidence remains evidence for the mechanism; the reshaped production
implementation earns its own appropriate verification/qualification.

CP2J, CP2K and CP2L remain valuable qualified evidence for RFB-over-mux,
visible presentation and PS2-originated interaction. Their existence does not
require their temporary experiment coordinators to become production
architecture.

---

## 8. Current CP2M disposition

Commit `9362ea9897113de82cdc41be578f499497dd0723` introduced an
experiment-specific transient keyboard checkpoint using an H1-specific
L1+D-pad keyboard gesture.

That checkpoint is **not** the architectural direction to advance or
hardware-qualify merely because its CI build is green. It was created before
this integration intent was clarified.

Before any further CP2M runtime work, trace the existing clean controller/input,
mouse, keyboard, OSK, application-coordinator and RFB relationships and identify
how H1 can instantiate those production modules through their normal interfaces.
The next keyboard proof should demonstrate that the real module composition can
operate over the qualified muxed RFB transport, not that H1 can independently
reproduce keyboard behavior.

The CP2M commit remains useful repository history/evidence of the design turn;
it is not production authority.

---

## 9. Required next architectural step

Before adding another user-facing H1 runtime feature, perform a read-only
integration inventory.

For each relevant production module, record:

- responsibility and owner;
- public interface;
- normal lifecycle;
- dependencies;
- current RFB/transport/presentation assumptions;
- whether H1 can use it unchanged;
- any genuinely required thin adapter or interface change;
- H1 code that becomes redundant when the production module is used; and
- expected final source-directory ownership for any new surviving code.

Then compare the H1 runtime tree against that inventory and classify each H1
piece as production functionality to integrate, development tooling to retain,
or vestigial scaffolding to discard.

Do not resume feature implementation until the intended wiring is understood.

---

## 10. Guiding statement

> H1 is the proving ground, not the replacement product. Prove the new mux,
> media and scheduling capabilities aggressively; then tease the successful
> mechanisms apart and integrate them surgically into the clean PS-to-VNC
> architecture. Reuse the existing production modules instead of imitating
> them. Keep H1 as development tooling where it remains useful, discard
> vestigial scaffolding, and let production consume only the qualified
> mechanisms and profiles it actually needs.
