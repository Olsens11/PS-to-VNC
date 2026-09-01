# Contributing to PS-to-VNC

PS-to-VNC uses a repository-centered development workflow intended to preserve
continuity across long-running development sessions.

Start with `AGENTS.md`.

## Before changing an existing subsystem

1. establish current project state;
2. inspect dirty/uncommitted work;
3. read the relevant architecture and previous empirical evidence;
4. check for an existing implementation, decision, or canonical tool;
5. define the responsibility boundary being changed.

Prefer small, reviewable changes with explicit ownership.

## Build and test

Use canonical build and test tooling documented in:

    docs/development/tooling.md

Do not replace an established saved build, deployment, or empirical procedure
with an ad hoc equivalent.

If the apparatus changes, record the change.

Hardware claims must bind to the exact tested DUT identity.

## Documentation

Documentation is part of a change when the change affects architecture,
current state, decisions, operating procedures, testing assumptions, or
deferred work.

Follow:

    docs/development/documentation.md

Current truth and chronological history are separate responsibilities.

## Source readability

All maintained project-defined names and existing source are subject to:

    docs/development/source-naming-and-symbols.md

New or changed symbols must use clear names, preserve file-level orientation,
and remain synchronized with the canonical generated source-symbol dictionary.

## Architecture decisions

Use `docs/adr/` for durable decisions whose rationale should survive the code
that originally implemented them.

ADRs are not status logs or test histories.

## Checks

Run:

    scripts/check.sh

along with any subsystem-specific build/tests required by the change.

## Git hygiene

Do not destroy uncommitted work during catch-up or recovery.

Do not move immutable release/version tags.

Preserve significant source/build/hardware identities in repository evidence
rather than relying on terminal output or conversation memory.
