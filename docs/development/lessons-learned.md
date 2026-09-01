# Development-System Lessons Learned

This file records real project pressure that changed or reinforced the
development-continuity rules.

It is not a product bug log.

## Canonical TestKit was missed

**Pressure:** an established five-mode hardware regression was reconstructed in
chat even though reusable PS2VNC TestKit machinery already existed.

**Lesson:** tool existence is not enough. The bootstrap must route future
sessions to canonical tooling.

**Rule:** discover and reuse saved procedures before constructing routine test
machinery.

## Test runner owned its run directory

**Pressure:** a bespoke wrapper pre-created the display-stress run directory.

**Classification:** apparatus failure, not DUT failure.

**Lesson:** procedure lifecycle contracts belong in saved tooling rather than
conversation recollection.

## Deployment manifest schema failed closed

**Pressure:** a generated deployment manifest omitted the required TestKit
manifest version.

**Classification:** apparatus/schema failure before FTP mutation.

**Lesson:** schemas should fail closed and expose validation or dry-run paths
when useful.

## API audit counted a private helper as public

**Pressure:** a prefix-based audit treated a private static geometry helper as
a fourth public API function.

**Classification:** audit-apparatus failure.

**Lesson:** checks should model structural intent rather than brittle textual
counts when ownership matters.

## Raw empirical evidence failed a source-style whitespace gate

**Pressure:** sealing M4H hardware evidence stopped because `git diff --check`
was applied indiscriminately to machine-generated TSV/socket/config evidence.

**Classification:** development-infrastructure hygiene-scope error.

**Lesson:** authored source/document hygiene and raw evidence integrity are
different concerns. Raw evidence should be preserved byte-for-byte and
validated by its evidence contract rather than rewritten to satisfy source
formatting rules.

## Documentation drift

**Pressure:** migration-era current-state and architecture documents lagged
behind M4 normalization.

**Lesson:** concise current state, historical chronology, architecture, and
machine state must be separate authorities with coherence checks.

## Agitation principle

These incidents are inputs to development-system evolution.

Generalize only the invariant demonstrated by real pressure. Keep
PS-to-VNC-specific mechanics in the project adapter.

## Migration checker stopped at its historical stage ceiling

**Pressure:** the continuity umbrella invoked the existing migration validator
after the project had legitimately advanced to M4. The validator still accepted
only M1 through M3.

**Classification:** development-infrastructure evolution gap.

**Additional finding:** the compatibility machine-state mirror still described
M3, and generic current fields in the M4 source-authority file had not advanced
after M4H hardware qualification.

**Lesson:** coherence tooling must validate current authority relationships
rather than freeze the development system at the lifecycle topology that
existed when the checker was first written.

**Rule:** when the project advances to a new lifecycle model, evolve the
validator and reconcile generic current authorities rather than bypassing the
check.

## Untracked-directory presentation broke an exact preflight

**Pressure:** an exact DOCS1 recovery preflight compared file-level expected
untracked paths against default `git status --porcelain` output. Git collapsed
untracked directory contents to `docs/adr/` and `docs/development/`.

**Classification:** development-infrastructure preflight error.

**Lesson:** human-oriented Git status presentation and file-level repository
identity are different interfaces.

**Rule:** use `git ls-files --others --exclude-standard` or porcelain with an
explicit all-files mode when an automated invariant depends on exact untracked
file identity.

## Compatibility index drifted behind repository documentation

**Pressure:** after the M4 migration and continuity checks were repaired, the
existing documentation validator rejected current architecture documents
because the migration-era `docs/INDEX.md` had not kept pace with documents
created during M4.

**Classification:** documentation-infrastructure routing drift.

**Lesson:** a curated current documentation router and a complete mechanical
document inventory serve different purposes.

**Rule:** `docs/README.md` is the curated canonical router. While legacy tooling
requires complete indexing, `docs/INDEX.md` may additionally provide a complete
compatibility inventory so every document is discoverable without implying
that every document is a current authority.

## Canonical checks must work on review branches

During formalization of the clean-reconstruction reboot, the canonical project
and documentation checks failed after all repository, evidence, migration, and
human/machine coherence checks had passed.

The failure was development-infrastructure-specific: Git authority was
hard-coded to require the checked-out branch to be `main`. As a result, no
pull-request branch could ever pass the canonical checks before merge.

The durable rule is that repository authority and branch identity are separate
concerns. The default branch remains exact published authority, while a
published development branch may be validated when its ancestry, upstream, and
relationship to the default branch are explicit and fail closed.

A review workflow should never require merging unvalidated work merely so the
validation machinery will run.

## Strategic transitions need a new current-state authority

A strategic project restart must not rewrite a completed historical machine
authority merely to make it look current.

The final M4 migration and hardware state remains valid evidence of the
exploratory implementation. The clean reconstruction therefore introduced a
separate current project-state authority rather than changing M4's recorded
next action after the fact.

The durable rule is to separate:

- what was current at the end of a completed development generation;
- what the project is doing now;
- the evidence connecting the two.

Historical authority stays historical. Current direction gets an explicit new
authority surface.
