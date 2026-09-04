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

### Existing documentation consistency

    scripts/docs-check.sh

### Continuity consistency

    scripts/continuity-check.sh

### Combined project check

    scripts/check.sh

### Source-symbol dictionary validation

    scripts/source-dictionary.py
    scripts/source-dictionary-self-test.py

The source-dictionary validator separates dictionary structural integrity from
ordinary maintenance drift. Structural ambiguity or unsupported discovery
semantics fail closed. Normal maintenance drift reports `ATTENTION` while
`check --strict` makes that drift blocking.

Definition discovery is language-aware:

- Universal Ctags supplies C/C-header definition records and Make targets;
- Python's standard-library AST supplies Python lexical definitions;
- `shfmt` supplies Bash structure;
- a narrow deterministic Make parser supplements Ctags for project-owned
  variable assignments.

The live development-system records the tested/required discovery tool
identities in `runtime/DEVELOPMENT_SYSTEM.env`. Do not silently substitute
Exuberant Ctags for Universal Ctags or a different `shfmt` version when the
validator's checked contract requires the recorded tool.

Definition-discovery readiness and trusted comprehensive-audit authority are
separate concerns. `DEFINITION_DISCOVERY_STATUS` remains `PENDING` and
`LAST_LONG_PASS_COMMIT` remains `UNSET` until the complete clean-generation
dictionary retrofit has passed the genuine comprehensive gate described by
`docs/development/source-naming-and-symbols.md`.

## Hardware TestKit

Canonical TestKit operator guide:

    scripts/testkit/README.md

That README is the exhaustive front door for choosing and invoking TestKit
tools. Its complete file catalog is mechanically checked by
`scripts/testkit/self-test.sh`.

Implementation-level symbol ownership remains in:

    scripts/testkit/SYMBOLS.md

Generic PS2 ELF deployment is owned by:

    scripts/testkit/deploy-elf.py
    scripts/testkit/deploy-elf-self-test.sh

The deployer has one narrow job: take one exact caller-selected ELF, preserve
the same bytes under a deterministic unique `/mass/0` filename, replace the
stable `/mass/0/PS2VNC.ELF` launch target with those same bytes, read both
targets back, and require matching SHA256 and byte counts.

Normal deployment always writes both the unique archival target and the rolling
launch target. Live FTP requires `--operator-authorized`; `--dry-run` guarantees
no FTP contact. `--repo <DUT-worktree>` is the normal operator interface when
the canonical tool and the DUT are different worktrees. Relative ELF and
evidence operands resolve against that selected DUT repository.

The generic deployer does not build, stamp, launch, observe, qualify, inspect an
Issue-specific manifest, require an Issue-specific branch, or own hardware-test
policy.

Issue #7 hardware qualification remains directly owned by PS-to-VNC through:

    scripts/testkit/prepare-hardware-elf.sh
    scripts/testkit/issue7-dut-manifest.py
    scripts/testkit/issue7-arm-observers.sh
    scripts/testkit/issue7-stop-observers.sh
    scripts/testkit/issue7-result.py
    scripts/testkit/issue7-apparatus-self-test.sh

Issue #7 arms and validates its own observation apparatus, then invokes the
generic deployer with the exact stamped-ELF identity and writes the generic
deployment evidence as that run's `deployment.json`. Its evaluator consumes
the byte-exact deployment/readback evidence without making deployment mechanics
Issue-specific.

The Issue #7 apparatus deliberately keeps its qualification contract narrow:

- exact committed source/DUT identity;
- stamped ELF verification;
- ordered UDP runtime identity and startup-stage capture;
- PS2-facing packet capture;
- run-owned observer shutdown;
- raw-evidence SHA256 sealing;
- machine-result evaluation separate from physical/operator judgment.

It does not inherit the historical M4 automatic freeze/capture machinery and it
does not perform automatic silent-stall recovery.

The PS2VNC TestKit remains historical mechanism/evidence authority. It is not a
live runtime dependency of current qualification. The former
`legacy-hardware-bridge.py` compatibility path is retired.

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

The historical PS2VNC repository and TestKit remain reference/evidence
authority only. Current PS-to-VNC tooling must not depend on their checkout
location or executable presence.

## Cross-tool path boundaries

A reusable tool that passes file operands across a process or tool boundary
where the working directory may change must normalize those operands before
delegation. Relative paths belong to the caller's repository context and must
not silently acquire a different meaning downstream.

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


The activation tool itself owns those five authority surfaces. If the caller
also installs a different **tracked current working ELF** so that the on-disk
working artifact matches the newly activated `CURRENT_WORKING_ELF_SHA256`, that
ELF is an additional member of the caller transaction.

For the current repository layout this means
`working/b4a/PS2VNC.ELF` must be included in transaction review, staging, and
the activation commit whenever its bytes change. It is not a sixth authority
surface; it is the tracked artifact corresponding to the authority record.

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

The disposable M4 hardware-checkpoint activation self-test uses the
following fail-closed historical authority-state contract:

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

## Issue #7 apparatus provenance and boundary

The successor apparatus was reconstructed from the qualification requirements
and from mechanically recovered, hash-verified historical TestKit behavior.
Historical deployment and observer mechanisms remain evidence, not executable
dependencies.

The migration intentionally did not copy the full historical TestKit. Issue #7
does not require its display benchmarks, freeze watcher, automatic trigger,
sampler, or M4 authority machinery.

The active regression authority is:

    scripts/testkit/issue7-apparatus-self-test.sh
    scripts/testkit/self-test.sh

The apparatus self-test is non-hardware-facing: it uses loopback UDP only and
must not contact FTP, require sudo, or touch the PS2. Operational arming remains
an explicit operator action.

## Current project state versus migration history

Current clean-reconstruction machine state is:

    runtime/PROJECT_STATE.env

The completed exploratory migration/normalization state remains:

    runtime/MIGRATION_STATE.env

The latter is historical/reference authority and must not be rewritten merely
to reflect a later strategic change in project direction.

`scripts/resume-state.sh` reports current project state first while retaining
the migration state as preserved exploratory authority.

For compatibility:

    scripts/resume-state.sh --field <name>

continues to query migration-era state.

Current project fields are queried explicitly with:

    scripts/resume-state.sh --project-field <name>
