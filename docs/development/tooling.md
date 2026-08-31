# Canonical Development Tooling

The development rule is:

> If a canonical saved tool exists, use it instead of reconstructing the same
> routine procedure in chat.

## PS-to-VNC tools

### Build

    scripts/build.sh

This is the canonical build authority.

### Resume

    scripts/resume-state.sh

### Migration consistency

    scripts/migration-check.sh

### Existing documentation consistency

    scripts/docs-check.sh

### Continuity consistency

    scripts/continuity-check.sh

### Combined project check

    scripts/check.sh

## Hardware TestKit

PS-to-VNC currently inherits proven hardware-test machinery from the frozen
PS2VNC development environment.

Canonical inherited TestKit location:

    /home/ps2/ps2vnc/scripts/testkit/

Before creating a deployment or hardware-test wrapper:

1. inspect the existing TestKit;
2. inspect the relevant manifest/procedure;
3. invoke the saved procedure when it already satisfies the test contract.

Do not casually copy the TestKit into PS-to-VNC. A future migration should be
deliberate, provenance-preserving, and self-tested.

## Tool promotion

Repeated manual procedures should become repository tooling when they:

- recur;
- encode non-obvious contracts;
- can affect correctness when reconstructed incorrectly;
- create or interpret evidence;
- materially reduce operator burden.

Promoted tools should have explicit inputs, fail closed where appropriate, and
offer validation/dry-run/self-test behavior where useful.

## Bespoke commands

Large one-off command packets remain appropriate for:

- new forensic investigation;
- one-time migration work;
- creation of a reusable tool;
- debugging a broken tool;
- examining a state with no established procedure.

They are not the normal interface for routine build/deploy/test work.

## Toolkit-first rule

Routine development mechanics are tooling responsibilities, not
conversation-specific shell-generation responsibilities.

Before writing bespoke commands for a repeated build, deploy, hardware,
identity, evidence, fingerprint, observer, sealing, or similar procedure:

1. inspect the existing project/TestKit tooling;
2. reuse an established fail-closed tool when available;
3. if a new one-off correction proves reliable and is generally reusable,
   promote it into successor-owned tooling and test it before using that
   procedure again.

Novel forensic audits and genuinely one-time structural investigations may
still use purpose-built commands when no reusable contract exists.

Reusable shell helpers must pass shell syntax validation and the relevant
TestKit self-test before they are trusted for an operational run. A tool failure
is development-infrastructure evidence, not a DUT failure.

The legacy `/home/ps2/ps2vnc` repository is immutable historical authority.
New reusable helpers belong to PS-to-VNC and may delegate to the legacy
TestKit without modifying it.

## Cross-tool path boundaries

A reusable tool that delegates file operands to another tool which may change
its working directory must normalize those operands to absolute paths before
delegation.

This is especially important when successor tooling calls immutable legacy
TestKit utilities from another repository. Relative paths are caller context;
they must not be allowed to acquire a different meaning after the delegated
tool changes directories.

Relevant self-tests must exercise relative caller paths as well as absolute
paths.

## M4 hardware-checkpoint activation

M4 hardware-pending authority transitions are a repeatable development
operation and must use:

    scripts/testkit/activate-m4-hardware-checkpoint.py

The tool is manifest driven and updates the five current M4 activation
surfaces together:

- `runtime/M4_ARCHITECTURE_BASELINE.env`
- `runtime/M4_SOURCE_AUTHORITY.env`
- `runtime/MIGRATION_STATE.env`
- the machine-state mirror and checkpoint record in `docs/MIGRATION_STATE.md`
- the current-development-focus section and machine-state mirror in
  `docs/status.md`

`docs/status.md` is part of the development-continuity authority and must move
atomically with the machine state. The disposable activation regression test
therefore includes all five surfaces.

The transition is deliberately not hardware authority. It makes a new
reproducible source generation `HARDWARE_PENDING`, preserves the previous
`LAST_VALIDATED` generation and previous direct hardware authority, and records
the exact identity-stamped hardware DUT that is awaiting qualification.

Before first live use after any tool change, the activation tool must pass:

    scripts/testkit/activation-self-test.py
    scripts/testkit/self-test.sh

The activation self-test operates only on disposable copies of the authority
files and requires idempotent second execution.

## M4 authority-state checker

`migration-check.sh` and the disposable M4 hardware-checkpoint activation
self-test share the same fail-closed state contract:

    scripts/testkit/m4-authority-state-check.py

Supported current-runtime states are:

- `HARDWARE_QUALIFIED`: no blocker is permitted and hardware qualification
  must be complete.
- `HARDWARE_PENDING`: a concrete non-`NONE` blocker is required and overall,
  machine, and physical hardware qualification must all remain `PENDING`.

The shared checker also requires migration state and M4 source authority to
agree on the current source commit, next action, and blocker.

The contract matrix is regression-tested by:

    scripts/testkit/m4-authority-state-self-test.py

## Pending-to-pending M4 checkpoint supersession

When an M4 hardware-pending checkpoint is invalidated and replaced by another
hardware-pending candidate, validate the replacement before changing live
authority:

    scripts/testkit/pending-supersession-self-test.py \
        evidence/m4/<checkpoint>-activation/ACTIVATION.env

The validator runs the canonical M4 hardware-checkpoint activator against
disposable copies of the five continuity surfaces. It requires the candidate
manifest to match the exact current pending checkpoint, proves the
pending-to-pending transition, preserves last-validated runtime and previous
direct-hardware authority, runs the shared M4 authority-state contract, proves
activator idempotence, and verifies that the live five continuity surfaces
remain byte-unchanged.

Only after that disposable regression passes should
`scripts/testkit/activate-m4-hardware-checkpoint.py` be used with the same
manifest against live authority.

## Side-effect-free Python static validation

When repository cleanliness is itself a checked invariant, do not use
`python -m py_compile` for a one-off static syntax check because it creates
`__pycache__` bytecode artifacts.

Use Python `compile()` on source text in memory instead. This validates syntax
without changing the filesystem and avoids contaminating exact dirty-state
preconditions.

## Successor / frozen-legacy Hardware TestKit bridge

PS-to-VNC successor hardware runs use:

    scripts/testkit/legacy-hardware-bridge.py

The bridge does not mutate or casually migrate the frozen PS2VNC repository.
Instead it:

1. requires a fully clean successor repository;
2. requires the exact frozen legacy HEAD;
3. requires the legacy tracked working tree and index to be unchanged;
4. deliberately permits and preserves pre-existing untracked legacy
   evidence/runtime artifacts;
5. creates a persistent successor-rooted isolated workspace;
6. copies the inherited legacy `scripts/testkit/` tree byte-for-byte and with
   executable modes preserved;
7. records bridge/TestKit provenance;
8. runs inherited `verify-build.sh` before hardware-facing use.

The bridge exposes:

    prepare
    verify
    deploy-dry-run
    start
    status
    watch
    result

A hardware manifest must be tracked in the successor repository before
`prepare`. The isolated clone therefore has the same committed source,
manifest, DUT evidence, and authority that the bridge is asked to validate.

`deploy-dry-run` is explicitly non-networking because inherited
`deploy-elf.sh` exits on `TESTKIT_DRY_RUN=1` before requiring `curl`.

The regression authority is:

    scripts/testkit/legacy-hardware-bridge-self-test.py

The self-test proves both sides of the legacy immutability rule:

- untracked legacy artifacts are accepted and preserved;
- any tracked legacy mutation fails closed.

Real hardware use still requires an explicit hardware manifest and a separate
operator-authorized `start` invocation. Preparing or self-testing the bridge
does not contact FTP or the PS2.
