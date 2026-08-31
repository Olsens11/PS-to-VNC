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
