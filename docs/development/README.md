# Development Continuity

PS-to-VNC treats development continuity as an engineering concern rather than
a conversation-management convenience.

A fresh development session should be able to determine from the repository:

- what the project currently is;
- what state it is in;
- what has been proven;
- what remains uncertain;
- what rules govern development;
- which tools are canonical;
- how to continue the active experiment safely.

## Clean reconstruction development frame

Development continuity includes recovering the architectural reasoning that
governs new work, not merely the current branch, build, and experiment.

Before designing product structure, read:

- `../PROJECT_INTENT.md`;
- `../CLEAN_ARCHITECTURE.md`.

The essential working model is:

1. design the product from responsibilities, ownership, and stable interfaces;
2. reconstruct it from the smallest useful executable upward;
3. treat the Issue #7 minimal core as a proven chassis rather than a future
   monolith;
4. add later behavior as coherent feature families around that chassis;
5. keep each feature's actual mechanism with the owner that understands it;
6. keep the application coordinator focused on lifecycle, routing, and genuine
   cross-domain orchestration;
7. preserve historical lessons and qualified mechanisms without copying
   historical source organization;
8. prefer explicit composition and the least-powerful mechanism that clearly
   expresses the real requirement;
9. make boundaries sufficiently complete for known near-term growth without
   speculatively implementing later-stage behavior;
10. judge modularity by understandable ownership and local reworkability, not
    by file count.

"Smallest useful" therefore means the smallest coherent and growth-capable
boundary. A foundation is too narrow when the next already-known reconstruction
stage must immediately replace it merely to attach its own responsibility. A
foundation is too broad when it implements policy or abstraction for behavior
that has not yet earned a requirement.

The desired reconstruction pattern is skeleton first, then meat on the bones:
establish durable owners, lifecycle, value types, and seams; prove each with
real behavior; then deepen each family without moving its internal mechanisms
up into the application chassis or sideways into unrelated domains.

## Source-writing contract

Development continuity also includes understanding how new clean product code
is supposed to be written.

The normative source-readability and topology policies are:

    source-naming-and-symbols.md
    source-topology.md

A fresh collaborator designing or reviewing `src/` should know before changing
it that:

- project-defined names communicate their role and every
  correctness-relevant distinction;
- maintained C source/header files carry a synopsis describing responsibility,
  ownership, exclusions, and documentary context;
- comments explain non-obvious invariants, authority, ordering, failure
  consequences, hardware assumptions, and rationale rather than restating
  syntax;
- directory-owned symbol dictionaries provide canonical lookup and completeness
  without relieving source names and control flow of their readability burden;
- clean product files are placed with their owning responsibility according to
  `source-topology.md`; the application root is not a generic landing zone for
  new feature modules;
- a new clean source directory is an explicit architectural/topology change and
  carries its local dictionary, documentation, build/test paths, and continuity
  gate with it;
- source should be understandable end-to-end by reading the owning module and
  its narrow interfaces;
- names, comments, module boundaries, and dictionaries work together so future
  rework does not require reconstructing intent from conversation history.

These are architecture and continuity requirements rather than optional style.

## Cross-Wire module lifecycle

Before creating or materially changing a module that performs cross-Wire work,
read:

    module-lifecycle.md

The project-wide default is:

- Wire owns communication validity;
- each module owns whether and when it runs;
- an ordinary module has one running instance at a time;
- the old instance completely stops before restart;
- reconnect uses the module's ordinary startup path;
- Wire availability is a prerequisite, not a universal module-start trigger;
- old Transport work can never be redirected through a replacement Wire
  Session;
- Wire Session IDs and module generation IDs are introduced outside Transport
  only when an owning subsystem has an independent reason to need them.

This contract deliberately separates Transport correctness from module business
policy.

## Two-layer model

### Portable continuity layer

Potentially reusable concepts include:

- repository-centered session reconstruction;
- dirty-state protection;
- authority separation;
- experiment/apparatus provenance;
- exact DUT identity;
- machine versus operator evidence;
- documentation transactions;
- reusable-tool promotion;
- deferred-work capture;
- current-state/history separation.

### PS-to-VNC project adapter

Project-specific mechanics include:

- PS2 build authority;
- ELF/PT_LOAD identity;
- PS2/Pi services and ports;
- FTP deployment;
- display modes;
- hardware qualification procedures;
- inherited PS2VNC TestKit.

The portable layer must not hard-code these project-specific mechanics.

## Five kinds of truth

Keep these responsibilities distinct:

1. current truth — what is true now;
2. architectural truth — how the system is intended to work;
3. decision truth — why an important choice was made;
4. empirical truth — what controlled evidence demonstrated;
5. historical truth — what happened previously.

One document should not become all five.

## Real-project agitation

The continuity system is intentionally developed against real project pressure.

PS2VNC served as the initial agitator. PS-to-VNC continues that role.

Development friction is useful evidence when it exposes a missing invariant.

Generalization is not automatic. First decide whether the pressure is
project-specific or reusable.

## Promotion ladder

A development-infrastructure idea normally matures through:

1. observed project friction;
2. local rule or repair;
3. repeated need;
4. reusable mechanism with explicit inputs;
5. self-test/versioning;
6. possible extraction into a portable framework.

This prevents speculative framework design while still allowing real project
work to improve the development system.

## Failure classification

Distinguish among:

- source/product failure;
- DUT/build failure;
- experiment failure;
- apparatus failure;
- development-infrastructure failure.

An infrastructure failure may itself be useful evidence if it reveals a
recurring weakness.
