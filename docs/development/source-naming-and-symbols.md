# Source Naming and Symbol Dictionary Policy

## Status

    POLICY=ACTIVE
    APPLIES_TO_CLEAN_RECONSTRUCTION=YES
    PRE_REFRESH_RETROFIT_REQUIRED=NO
    SYMBOL_DICTIONARY=REQUIRED
    COMPLETENESS_GATE=REQUIRED
    CURRENT_RETROFIT_STATUS=IN_PROGRESS

Clean-reconstruction PS-to-VNC source must be understandable both where a name
appears and from one canonical lookup surface. The boundary begins with the
strategic refresh and applies to the new product generation being built from
`docs/CLEAN_ARCHITECTURE.md`, including Issue #7 and all later capabilities.

## Naming

Every project-defined name should communicate the thing or role represented.
Correctness-relevant distinctions such as desired versus active, authoritative
versus presentation-only, pristine versus stamped, and complete versus partial
must remain visible.

Short conventional names are acceptable only when scope and meaning are
immediate. An index entry never excuses an unclear source name. Rename unclear
symbols when doing so improves the code; any rename that changes PT_LOAD remains
a runtime change under the normal hardware gate.

## Orientation and comments

Every maintained C source/header begins with a synopsis stating what the file
accomplishes, owns, excludes, and where its plain-language context lives.
Internal comments explain invariants, authority, ordering, failure consequences,
hardware assumptions, and rationale rather than narrating syntax.

## Directory-owned dictionaries

Canonical descriptions live beside the clean source that owns them:

    <clean-source-directory>/SYMBOLS.md

Each dictionary covers symbols defined directly in its directory. Entries are
grouped by defining file and owner so local names remain near their context.
Moving a symbol between directories moves its canonical description as part of
the same change.

The lightweight generated portal is:

    docs/reference/SOURCE_SYMBOL_DICTIONARIES.md

It reports directory responsibilities, dictionary links, symbol counts, and
validation status. A deterministic comprehensive Markdown view is generated on
demand by `scripts/source-dictionary.py aggregate`; it is an output, not a
second manually maintained authority.

Every clean-generation project-defined symbol receives its exact name, kind,
language, defining file, owning function/module where applicable, scope, and a
concise plain-language description. Contextual documentation is included where
useful.

Coverage includes functions, parameters, local/file/global variables, types,
structures, fields, enums and values, macros/constants, Python definitions and
variables, shell functions and maintained variables, and project-owned Make
targets and variables. Repeated uses and third-party names are not separate
project symbols.

## Clean-generation scope and exclusions

Required scope:

- product source newly written for the clean reconstruction;
- tests newly written for that clean source;
- build, qualification, and development tools newly written to support it;
- every later capability added to the refreshed product generation.

Do not retrofit frozen PS2VNC/B4A, sealed evidence, migration-era normalization
source, retained pre-refresh modules, provenance snapshots, generated output, or
third-party interfaces merely because they remain present in this repository.
A retained module enters scope only when the clean reconstruction deliberately
adopts or rewrites it as current product source. The dictionary records this
generation boundary explicitly.

## Synchronization gate

A saved successor-owned checker must distinguish structural integrity failures
from ordinary dictionary-maintenance drift.

Malformed dictionary structure, impossible ownership, ambiguous duplicates, or
other conditions that make validation untrustworthy fail immediately.

Ordinary maintenance findings such as stale entries, inadequate descriptions,
clean files not yet represented by a dictionary, and requested incomplete
coverage are reported together as `ATTENTION` during normal development.
Normal `check` remains successful so active implementation can reach a sensible
checkpoint while the same findings continue to nag on every project check.

`check --strict` turns those maintenance findings into a nonzero quality gate.
`--require-complete` additionally reports directories still marked
`COVERAGE=IN_PROGRESS`; combine it with `--strict` when complete retrofit
coverage is intentionally required.

Directory-local checks may be used during editing. Deterministic fixtures must
prove positive, attention, strict-failure, stale, duplicate, ownership, and
excluded-tree behavior before the checker joins `scripts/check.sh`.

## Retrofit completion

The clean-generation retrofit is complete only when every in-scope refreshed
surface and symbol already written is inventoried and described, unclear names
are corrected or justified, the
synchronization gate and project checks pass, and any PT_LOAD-changing rename
remains hardware-pending until qualification.

Policy creation alone is not retrofit completion.
