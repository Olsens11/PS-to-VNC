# Source Naming and Symbol Dictionary Policy

## Status

    POLICY=ACTIVE
    APPLIES_TO_CLEAN_RECONSTRUCTION=YES
    PRE_REFRESH_RETROFIT_REQUIRED=NO
    SYMBOL_DICTIONARY=REQUIRED
    COMPLETENESS_GATE=REQUIRED
    CURRENT_RETROFIT_STATUS=COMPLETE

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

Clean product file placement is governed by
`docs/development/source-topology.md`. The domain-directory topology was adopted
on 2026-09-05; it is a forward/current policy, not a claim that the repository
always had that shape. New feature-family files do not accumulate directly in
`src/`. A new clean product directory must be an earned responsibility boundary
and must receive its own `SYMBOLS.md` in the same change.

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

### Definition-discovery adapters

Project-definition completeness is discovered from language structure rather
than by treating every identifier use as a project symbol.

- C and C headers use Universal Ctags JSON records and map only definition
  kinds owned by PS-to-VNC. Library, compiler, libc, and PS2SDK names that are
  merely referenced are not dictionary candidates.
- Python uses the standard-library AST. Named lexical owners are qualified;
  anonymous lambda and comprehension scopes use location-independent semantic
  fingerprints plus deterministic same-fingerprint occurrence numbers.
  Unsupported ownership forms fail closed rather than being guessed.
- Shell uses the `shfmt` Bash AST for functions and maintained bindings.
  Unsupported dynamic binding grammar fails closed.
- Make uses Universal Ctags for targets plus a deliberately small deterministic
  parser for project-owned variable definitions. Toolchain and conventional
  external Make variables are excluded explicitly.

Anonymous Python owner identities must remain safe inside the validator's
colon-delimited diagnostics. They therefore contain neither source-coordinate
identity nor literal colon delimiters. Repeated syntactically identical
anonymous scopes under one lexical owner are distinguished by deterministic
occurrence number.

The current development-system dependency authority for these adapters is
recorded in `runtime/DEVELOPMENT_SYSTEM.env`. The validator performs its own
fail-closed executable/version checks where a dependency contract is required.

The validator's implementation history, language adapters, maintenance-drift
contract, trusted-baseline behavior, and deliberately deferred exact-instance
ignore design are recorded in
`docs/development/source-dictionary-validator.md`.

## Clean-generation scope and exclusions

Required scope is maintained PS-to-VNC product source.

The current product-source root is:

    src/

A later source tree enters the completeness boundary only when the project
explicitly adopts that tree as maintained product source.

Development machinery is deliberately outside required symbol-dictionary
completeness. This includes:

- `scripts/`, including the dictionary validator itself;
- `scripts/testkit/` and other qualification/deployment tooling;
- validator and tooling self-tests;
- `tests/`;
- `mk/` and other build-system implementation;
- frozen PS2VNC/B4A and sealed evidence;
- migration-era normalization source;
- retained pre-refresh implementation examples;
- provenance snapshots and generated output;
- third-party interfaces.

Such areas may retain useful human-written symbol dictionaries, but their
presence does not make them part of the product completeness gate or generated
product-symbol portal.

A retained module enters scope only when the clean reconstruction deliberately
adopts or rewrites it as current product source. The dictionary records this
product-ownership boundary explicitly.

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

## Comprehensive and incremental audits

Definition-completeness checking uses an explicit trusted-baseline model.

Normal `check` is the fast development path. Once a trusted comprehensive
baseline exists, definition discovery examines only relevant product-source
paths changed since that baseline commit. The delta includes committed changes
after the baseline plus staged, unstaged, and untracked work. Cheap dictionary
structural validation remains comprehensive across participating product
dictionaries.

`check --long` explicitly ignores the incremental shortcut and selects the
entire current product-source definition scope. If the saved baseline is
missing, `UNSET`, unresolvable, or no longer an ancestor of current `HEAD`,
normal `check` safely falls back to comprehensive product scope rather than
claiming incremental coverage it cannot prove.

The machine-readable baseline authority is:

    runtime/SOURCE_DICTIONARY_STATE.env

`LAST_LONG_PASS_COMMIT` identifies the source commit most recently proven by a
genuine comprehensive project-definition audit. `UNSET` means no such trusted
baseline has been established.

A comprehensive check never advances that authority implicitly. A clean,
passing comprehensive product-source audit may explicitly request baseline
advancement with:

    python3 scripts/source-dictionary.py check --long --require-complete --record-baseline

The command records the audited clean `HEAD` in the state file; that state-file
change is then reviewed and committed normally.

Project-definition discovery is now implemented and
`DEFINITION_DISCOVERY_STATUS` is `READY`. The exact trusted comprehensive
baseline, including whether one has yet been established, is recorded only in
`runtime/SOURCE_DICTIONARY_STATE.env`. Baseline advancement remains explicit
and requires a clean passing comprehensive audit.

## Retrofit completion

The clean-generation retrofit is complete only when every in-scope refreshed
surface and symbol already written is inventoried and described, unclear names
are corrected or justified, the
synchronization gate and project checks pass, and any PT_LOAD-changing rename
remains hardware-pending until qualification.

Policy creation alone is not retrofit completion.

The current maintained product-source retrofit is complete: every in-scope
product definition is represented, all participating product dictionaries
declare complete coverage, and the comprehensive strict audit passes without
maintenance findings.
