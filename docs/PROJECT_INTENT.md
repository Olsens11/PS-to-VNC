# PS-to-VNC Project Intent

This document records the seed of intent for the clean PS-to-VNC reconstruction.
It exists to keep the project's purpose, values, and reconstruction strategy
stable while implementation details remain deliberately malleable.

## What we are building

PS-to-VNC is a PlayStation 2 VNC client and Raspberry Pi companion system.

The product goal is a practical PS2-side interface to a Pi-hosted desktop and
utilities over Ethernet, with controller and keyboard interaction, deliberate
display-mode handling, configuration and persistence, recovery and management,
and a reproducible companion environment.

The final system should be understandable, rebuildable, testable, and usable on
ordinary supported hardware without requiring the exploratory development
machine that produced the original implementation.

## Platform scope principle

PS-to-VNC should provide excellent general access to the PS2 and build in the
broadly useful capabilities that make the PS2/Pi combination a practical
companion platform. Remote desktop and input, display handling, networking,
storage/file access, configuration, recovery, companion management, media
plumbing, and utility launching are examples of capabilities that can improve
the platform itself.

The project should not absorb every specialized PS2 workflow or utility merely
because that software can run on the Pi or benefit from PS-to-VNC facilities.
Specialized tools should be deliberately welcome as first-class compatible
guests without automatically becoming PS-to-VNC product features.

A useful scope test is:

> Does this capability improve the general PS2/Pi platform, or does it implement
> one particular use of that platform?

Capabilities in the first category are candidates for the core project when
they are broadly useful and fit the architecture. Capabilities in the second
category should normally remain separate optional utilities unless repeated
product experience demonstrates that they have become general platform needs.

PS-to-VNC should therefore make extension easy without making extension code
part of the core by default. Stable configuration, documented interfaces,
network/storage primitives, utility launching, and other general integration
points should let companion software participate cleanly without creating a
hidden dependency on that software or requiring it in the base installation.

This boundary is about product scope and architectural ownership, not a judgment
that adjacent utilities are undesirable. The intended result is a clean,
general companion platform on which useful PS2-community software is easy to
build, install, and use.

## Why the project is being restarted

The existing implementation proved that the product is viable and accumulated
a large amount of hard-won knowledge about PS2 hardware, networking, RFB/VNC,
GS/display behavior, input, recovery, build identity, testing, and the Pi-side
environment.

It also grew under exploratory pressure. Structural decisions were often made
while the project was still discovering what the hardware and protocol actually
required. Continuing to normalize that structure mechanically risks preserving
accidental organization rather than designing the product we now understand.

The clean reconstruction therefore keeps the knowledge and evidence while
reconsidering the structure.

## Core reconstruction principle

> Rebuild PS-to-VNC as the program we would have written if we had known at the
> beginning everything the exploratory implementation taught us.

The existing implementation is a behavioral reference, evidence source, and
library of proven mechanisms. It is not a structural blueprint for the new
implementation.

## Understand before rebuilding

Before replacing a subsystem, determine what it actually does and why.

The semantic audit should describe each meaningful component in plain language,
including where applicable:

- purpose and user-visible responsibility;
- inputs and triggers;
- owned state;
- normal behavior;
- outputs and side effects;
- dependencies;
- failure behavior;
- invariants;
- original B4A/current implementation references;
- relevant historical experiments and lessons;
- implications for the clean rebuild.

The audit should answer both "What is PS-to-VNC?" and "How does each part earn
its place in the product?"

## Preserve lessons, not accidents

Historical failures and awkward investigations often contain durable knowledge.
Those lessons should be mined explicitly rather than rediscovered.

Examples include hardware-specific exit behavior, display-mode bring-up,
deterministic runtime identity serialization, VNC/RFB failure modes, GS
presentation constraints, controller/input behavior, and test-apparatus lessons.

A successful historical mechanism may be reused when it is still the clearest
solution. A historical structure should not be retained merely because it
already exists.

## Design top-down, implement bottom-up

The product should be designed from responsibilities and interfaces downward,
then reconstructed from the smallest useful executable upward.

Expected reconstruction progression:

1. PS2/Pi Ethernet baseline;
2. minimal RFB/VNC connection and handshake;
3. fixed 480p desktop presentation;
4. controller input;
5. keyboard/on-screen keyboard interaction;
6. menus and local UI;
7. configuration and persistence;
8. display modes and per-mode calibration;
9. management and recovery;
10. performance work, including future hybrid-video paths where justified.

Each stage should live directly in the intended modular architecture. Do not
create a temporary second monolith simply to split it again later.

## Code-quality principle

> Do not trade a legible monolith for a well-organized pile of gibberish.

Prefer boring, readable C:

- structs that represent real concepts;
- enums for meaningful states;
- explicit ownership;
- direct calls;
- small coherent modules;
- readable control flow;
- abstractions justified by demonstrated need.

Avoid speculative frameworks, indirection for its own sake, or decomposition
that makes behavior harder to follow.

## Composition and complexity hierarchy

> Prefer explicit wiring and implicit behavior.

Complex product behavior should normally emerge from deliberately composing
simple components, each left to perform one defensible duty. A component should
not be taught about every product context merely so it can decide for itself
whether its capability is currently wanted. Where possible, the coordinator or
caller should make that choice explicitly by routing work through the component
when its capability applies.

The chord-arbitration example captures the intended style: when an input path
needs chord interpretation, it invokes the chord arbiter; when it does not,
there are no chord semantics in that path. The arbiter does not need a parallel
copy of the application's context model simply to know when to turn itself on or
off.

This is a preference, not a prohibition on state. Some responsibilities really
do persist across calls or have meaningful phases. Pointer acceleration, parser
position, UI foreground ownership, Refresh cooldown, and risky display
transactions are examples where local state, typed state, or a state machine can
be the clearest representation.

When choosing a mechanism, prefer the lowest level of complexity that expresses
the requirement clearly and completely:

1. direct composition or a direct call;
2. local state owned by one component;
3. explicit ownership or routing between components;
4. typed persistent state or a small state machine;
5. deliberate cross-domain coordination when an operation genuinely spans
   multiple owners.

Moving upward in that hierarchy should have a clear reason. A mode flag, shared
state, or broader coordinator is acceptable when it represents real behavior
that cannot be expressed more clearly through ownership, routing, or the current
operation itself. It should not be introduced merely to duplicate information
already obvious from the call graph.

A useful design question is:

> What is the least-powerful mechanism that expresses this behavior clearly and
> completely?

The goal is not statelessness or maximum decomposition. It is to give each
component a duty that can be understood end-to-end, make the wiring between
components explicit, and let sophisticated behavior arise from their
composition. This keeps future rework local: changing one responsibility should
not require unrelated modules to understand or duplicate that responsibility's
internal state machine.

## Architecture remains malleable

The clean architecture is a working design, not a doctrine.

Semantic audit findings, early reconstruction experience, hardware constraints,
and better understanding may change module boundaries or ownership. The system
should become more deliberate as evidence accumulates, not more rigid.

## Reference material

The project distinguishes three roles:

- `reference/` or preserved baseline material: what taught us how the system
  works;
- `docs/`: what we currently understand about how it works and why;
- `src/`: intentionally written product code for the new implementation.

Existing B4A slices and preserved exploratory artifacts remain useful for
archaeology and comparison even when their structure is not carried forward.

The frozen legacy repository remains immutable historical authority.

## Raspberry Pi philosophy

The Pi companion should begin from a normal, supported Raspberry Pi OS baseline.
Commodity responsibilities should remain commodity wherever practical:
networking, service management, package management, SSH, Samba, and similar
platform functions should use standard mechanisms unless PS-to-VNC has a real
reason to replace them.

Custom Pi-side software should exist only where the product itself needs custom
behavior.

## Pi reproducibility

A fresh supported Raspberry Pi OS installation should be transformable into a
PS-to-VNC companion through a documented, versioned installation process.

The canonical artifact is the install/package definition, not a hand-tuned SD
card. A ready-to-flash image may later be produced as a convenience artifact.

Every meaningful manual Pi deviation should either become declared installer
state or be removed.

## Dependency ledger

Dependencies should be tracked from the moment they are tried, including
rejected experiments.

Useful states include:

- `EVALUATING`;
- `ADOPTED_RUNTIME`;
- `ADOPTED_BUILD`;
- `ADOPTED_OPTIONAL`;
- `OS_BASE`;
- `REJECTED`;
- `REPLACED`.

The reason for adoption, rejection, or replacement matters as much as the final
package list.

Before a packaging milestone, unexplained environmental changes should reach
zero.

## Installation packages are development artifacts

Pi installation/reproduction should be exercised during development rather than
postponed until release.

At meaningful milestones, prove that a clean supported OS can be brought to the
required companion state using the tracked installer/package process.

## GitHub is part of development

GitHub is the collaboration and traceability surface, not the sole knowledge
store.

Use:

- Discussions for open-ended design questions;
- Issues for defined work, audits, experiments, bugs, and qualification tasks;
- Projects for cross-cutting status and roadmap views;
- Pull requests for deliberate change units;
- Actions for automated build/test/validation work;
- Releases for durable milestone artifacts.

Durable technical knowledge must still live in the version-controlled
repository.

The desired traceability chain is:

historical experiment -> lesson -> behavior specification -> architecture
choice -> issue -> implementation/PR -> automated test -> hardware
qualification -> release.

## Development values

The reconstruction should favor:

- evidence over intuition;
- explicit ownership over hidden coupling;
- reproducibility over machine folklore;
- preserved history over destructive cleanup;
- small verified increments over sweeping rewrites;
- clear product behavior over architectural aesthetics;
- standard mechanisms over custom infrastructure when both serve equally well;
- repository-centered continuity over conversation-dependent knowledge.

## Immediate phase

The first phase is:

> Preserve -> Clean -> Understand -> Reconstruct

Concretely:

1. seal and verify the exploratory repositories, runtime state, evidence, and Pi
   environment;
2. establish the clean Raspberry Pi baseline and reproducibility ledger;
3. perform the semantic audit of the existing product and its historical test
   record;
4. derive a deliberately simple target architecture from the audited behavior;
5. reconstruct the smallest useful PS2/Pi path and grow it through controlled
   milestones.

The previous extraction-oriented normalization plan is retained as historical
work. Its completed evidence remains valid evidence; its planned next steps are
not automatically the new reconstruction roadmap.

## Definition of success

The reconstruction succeeds when PS-to-VNC is not merely working, but is:

- understandable by a fresh development session;
- behaviorally grounded in evidence;
- modular without obscuring control flow;
- reproducible on a fresh Pi;
- buildable and testable through tracked tooling;
- qualified on real PS2 hardware;
- recoverable without relying on undocumented machine state;
- ready to evolve without repeating the exploratory project's avoidable
  structural debt.

## Guiding statement

> PS-to-VNC should be rebuilt as the program we would have written if we had
> known at the beginning everything the exploratory implementation taught us.
> The old implementation provides proven knowledge. The new implementation
> provides the future.
