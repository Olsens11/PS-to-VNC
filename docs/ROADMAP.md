# PS-to-VNC Roadmap

Status: `DURABLE_DIRECTION_WITH_HISTORICAL_MIGRATION_SECTION`

## How to read this roadmap

This file records durable future direction and preserved earlier roadmap
thinking. It is **not** a live `NEXT_ACTION` surface.

Active work, exact branch/PR authority, and the next safe hands-on action must be
discovered from freshness-reconciled project/GitHub/live state under:

- `AGENTS.md`;
- `docs/development/SESSION_RECONSTRUCTION.md`;
- `docs/development/TEMPORAL_STATE_SEMANTICS.md`;
- current GitHub issues/PRs/branches;
- identity-valid live state when available.

Do not infer that an item is happening now merely because it appears earlier in
this file or is headed `Immediate` inside the preserved historical section.

## Current clean-reconstruction direction

The durable product direction is governed by `docs/PROJECT_INTENT.md` and
`docs/CLEAN_ARCHITECTURE.md`:

> Rebuild PS-to-VNC as the program we would have written if we had known at the
> beginning everything the exploratory implementation taught us.

Near-term implementation remains intentionally incremental: establish and
qualify the smallest clean PS2/Pi foundation before layering back proven product
behavior. Exact active milestones are tracked through their current GitHub
issues/PRs rather than duplicated here as timeless status.

### Deferred product directions worth preserving

These remain future directions, not commitments to implement them now:

- richer RFB/presentation optimization after the simple clean core is stable;
- portable Pi networking/configuration and reproducible companion installation;
- a PS-friendly dedicated media path, potentially using MPEG-2 where it earns
  complexity rather than forcing moving video through ordinary RFB updates;
- possible Pi gateway/source-normalization behavior for local/remote desktop or
  media sources;
- future audio support;
- broader display-mode restoration only after the clean display foundation and
  relevant major investigations are qualified;
- packaging/bootstrap work that can reproduce the Pi and PS2-side environment
  on fresh media.

Provider or mechanism names are not architecture merely because they appear in a
future idea. Preserve replaceable seams and qualify concrete present choices
before promotion.

## Continuing development requirements

- durable knowledge moves into repository authority rather than living only in
  chat;
- significant empirical work preserves exact DUT/apparatus/evidence identity;
- machine evidence and physical/operator observation remain separate;
- historical evidence remains recoverable;
- qualified workarounds stay isolated so they can later be removed cleanly;
- repeated useful procedures should graduate into tested repository tooling;
- mutable current-state claims are timestamped snapshots rather than eternal
  present-tense facts.

---

# Preserved migration-era roadmap

Temporal role: `HISTORICAL_ROADMAP_SNAPSHOT`

Recorded by the migration generation before the 2026-09-01 strategic clean
reconstruction reboot. The wording below is intentionally preserved as history;
its `Immediate`, `current`, and sequencing language must **not** be interpreted
as the present roadmap.

## Immediate — historical

### M0 — establish successor working baseline

Reproduce the frozen B4A implementation as current PS-to-VNC working build
authority without modifying the immutable baseline.

### Define exact extraction stages

Convert the frozen dependency graph into M1 through Mn operations with exact
function/state ownership and documentation deliverables.

### Mechanical modularization

Move code in small behavior-preserving stages.

Do not combine refactoring, optimization or new feature work with extraction.

## After modular baseline — historical

### Internal RFB-to-GS profiler

Add low-overhead subsystem timing/counter instrumentation as the first major
new feature in the modular tree.

Use it to localize the high-resolution RFB backpressure previously measured in
the historical project.

### Dedicated media path

Develop a PS-friendly media transport/render path, including MPEG-2 where
appropriate, rather than forcing moving video through ordinary RFB updates.

### Pi Cast Gateway

Expose the Pi as a discoverable local-network casting/media target and convert
received sessions into the dedicated PS media path.

## Continuing requirement — historical wording

Documentation moves in lockstep with implementation.

No completed source stage may leave the living source map or project state
behind.

## Post-migration project maturation — historical

After the behavior-preserving migration and modular regression are complete,
PS-to-VNC will undergo a dedicated project-maturation pass.

That pass may include:

- source formatting and whitespace normalization;
- consistent naming and file conventions;
- cleaner public and private APIs;
- removal of temporary migration scaffolding;
- consolidation of generated analysis and development artifacts;
- compiler-warning cleanup;
- build-system cleanup;
- coding and formatting policy;
- automated validation and CI where practical;
- test organization;
- `.gitignore` and artifact-retention policy;
- licensing and project metadata;
- contributor/developer documentation;
- release/versioning conventions;
- packaging and reproducible release builds;
- general repository organization suitable for a maintained project.

Historical provenance must remain recoverable.

Byte-exact frozen evidence should not be silently rewritten merely to make it
look cleaner. It may later be moved, packaged, or archived separately once its
identity and provenance remain durably preserved.

The working implementation and derived project artifacts are free to be
cleaned and normalized once migration no longer depends on byte-for-byte
comparison with the historical baseline.
