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
