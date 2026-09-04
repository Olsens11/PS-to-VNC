# Source Dictionary Validator

## Status

    VALIDATOR_STATUS=ACTIVE
    DEFINITION_DISCOVERY_STATUS=PENDING
    TRUSTED_LONG_BASELINE=UNSET
    IGNORE_REGISTRY_STATUS=DEFERRED_UNTIL_USEFUL

This document records the current design, significant evolution, and deliberate
limits of `scripts/source-dictionary.py`.

The validator exists to answer a narrow question:

> What project-defined names in current clean-generation source are not
> represented by the directory-owned source symbol dictionaries?

It is not an identifier linter, source-code indexer for third-party names, or a
requirement to document every token that appears in source.

The normative naming and coverage policy remains
`docs/development/source-naming-and-symbols.md`. This document records the
validator implementation contract and the reasoning behind the tooling choices.

## Why the validator changed

The first dictionary checker primarily verified manually maintained dictionary
structure. As the clean reconstruction grew, that was not sufficient to prove
that newly introduced project definitions had actually been indexed.

The validator therefore evolved from a dictionary-format checker into a
language-aware project-definition completeness checker.

That evolution deliberately avoided treating every identifier use as a symbol.
The desired boundary is project-owned definitions introduced by PS-to-VNC.

Examples include:

- functions and function declarations;
- parameters;
- local, file, and global variables;
- project types, structures, fields, enums, and enum values;
- project macros and constants;
- Python classes, functions, parameters, and maintained bindings;
- shell functions and maintained shell variables;
- project-owned Make variables and targets.

Names merely referenced from libc, PS2SDK, the compiler, the shell environment,
GNU Make, or another external interface are not dictionary candidates merely
because they appear in clean source.

## Structural failures versus maintenance drift

A major validator change separated failures that make the audit untrustworthy
from ordinary dictionary-maintenance work.

Structural failures remain blocking. Examples include:

- malformed dictionary metadata or table structure;
- duplicate dictionary entries;
- impossible or ambiguous ownership;
- unsupported source constructs whose ownership cannot be determined safely;
- parser/tool failures that prevent trustworthy discovery.

Ordinary maintenance drift is nonblocking during normal development and is
reported as `ATTENTION`. Examples include:

- discovered project definitions not yet represented in a dictionary;
- stale dictionary rows;
- incomplete dictionary coverage;
- inadequate descriptions;
- clean source files not yet covered by a directory dictionary.

Normal project checks therefore continue while displaying the outstanding
maintenance work.

`--strict` makes those maintenance findings blocking when a quality gate is
desired.

`--require-complete` separately requires all participating dictionaries to be
marked `COVERAGE=COMPLETE`.

This distinction was intentional: dictionary maintenance should remain visible
without preventing useful implementation work from reaching a coherent
checkpoint.

## Language-aware definition discovery

### C and C headers

C definition discovery uses Universal Ctags JSON output.

The adapter maps only definition kinds that correspond to project-owned
definitions, including functions, prototypes, parameters, locals, variables,
members, structures/unions, typedefs, enums, enumerators, macros, and external
variable declarations.

Ordinary references to functions, constants, types, or APIs supplied by libc,
PS2SDK, or other libraries are not definitions and therefore do not become
dictionary findings.

Universal Ctags is treated as a parser dependency, not as the final authority
for dictionary vocabulary. Parser kinds are normalized into the maintained
dictionary vocabulary before matching.

### Python

Python definition discovery uses the Python standard-library AST rather than
Ctags.

The visitor models lexical ownership for:

- modules;
- classes;
- functions and methods;
- nested functions;
- parameters;
- locals and fields;
- assignment targets;
- loops;
- context-manager bindings;
- exception bindings;
- match captures;
- the currently required `nonlocal` ownership case;
- lambdas;
- comprehensions.

Unsupported ownership semantics fail closed instead of being guessed.

#### Anonymous Python scopes

Lambda and comprehension bindings require an owner even though the source does
not provide a conventional lexical name.

Source coordinates were deliberately rejected as dictionary identity because a
harmless line insertion would rename the owner.

Anonymous owners therefore use:

1. the anonymous construct kind;
2. its binding names;
3. a location-independent SHA-256 fingerprint of the AST with source-position
   attributes excluded;
4. a deterministic occurrence number when syntactically identical anonymous
   scopes occur under the same lexical owner.

Conceptually:

    <setcomp[line]#a424d7827009>~1
    <setcomp[line]#a424d7827009>~2

This permits genuinely identical anonymous constructs while avoiding
line-number-derived dictionary churn.

Anonymous owner text also avoids literal `:` characters because missing-symbol
diagnostics themselves use colon-delimited fields.

### Shell

Shell discovery uses the `shfmt` Bash AST.

The adapter discovers project-maintained shell functions and bindings while
excluding ordinary external shell/environment vocabulary.

Supported binding forms include the shell constructs actually used by the
current clean-generation tooling.

Dynamic or unsupported binding grammar fails closed rather than being
misclassified.

This parser was adopted after simpler text/regular-expression approaches proved
unable to distinguish shell syntax reliably from heredocs, embedded programs,
command environment prefixes, and related constructs.

### Make

Make discovery is hybrid.

Universal Ctags is used for targets.

A deliberately small deterministic parser discovers project-owned variable
definitions because Ctags does not reliably cover every assignment form used by
the repository.

Known external/toolchain Make vocabulary is explicitly excluded.

Current maintained `Makefile` and `.mk` build/test infrastructure enters the
clean-generation dictionary boundary even though those files predate the
C/Python/shell `File synopsis:` convention.

## Duplicate dictionary integrity

Duplicate dictionary keys are structural errors.

The key is the logical definition identity:

    file + owner + kind + name

Duplicate detection occurs before source-existence handling so two identical
rows cannot evade the structural check merely because they point at a source
path that is currently missing.

A missing source does not count as covered source.

## Clean-generation scope

The validator is intended only for current clean-generation ownership.

Explicit excluded path components currently include retained areas such as:

- `baseline`;
- `evidence`;
- `working`;
- generated `build` output;
- `.git`.

C, Python, and shell files normally enter discovery through the clean-source
`File synopsis:` marker.

Maintained Make sources use the Make-specific clean-scope rule described above.

The policy is broader than any one hard-coded path list: frozen evidence,
pre-refresh implementation examples, provenance material, generated output, and
third-party source do not become dictionary obligations merely because they are
retained in the repository.

A retained source file enters dictionary scope only when the clean
reconstruction deliberately adopts or rewrites it as maintained current
source.

## Comprehensive and incremental discovery

Definition discovery supports a trusted long-pass baseline.

`check --long` always performs comprehensive clean-generation discovery.

Normal `check` may use an incremental source scope only after a trusted
comprehensive baseline has been explicitly established.

The saved authority is:

    runtime/SOURCE_DICTIONARY_STATE.env

with:

    LAST_LONG_PASS_COMMIT=<commit>

If the baseline is:

- `UNSET`;
- missing;
- unresolvable;
- or no longer an ancestor of current `HEAD`;

normal checking safely falls back to comprehensive discovery.

The incremental delta includes:

- committed source changes since the baseline;
- staged changes;
- unstaged changes;
- untracked relevant source paths.

A comprehensive check never advances the baseline implicitly.

Baseline recording is explicit and requires the appropriate comprehensive,
complete, clean-worktree conditions.

At the time of this document:

    DEFINITION_DISCOVERY_STATUS=PENDING
    LAST_LONG_PASS_COMMIT=UNSET

Those values must remain unchanged until the complete retrofit and genuine
comprehensive gate are intentionally finished.

## Discovery dependencies

The current development-system authority records the tested discovery
dependencies in:

    runtime/DEVELOPMENT_SYSTEM.env

Current adapter dependencies are:

- Python 3.13.5 for the standard-library AST implementation;
- Universal Ctags 5.9.0;
- `shfmt` 3.8.0.

The validator fails closed when a parser dependency required by its implemented
contract is unavailable or incompatible.

## Generated views

Directory-owned `SYMBOLS.md` files remain the human-maintained authority.

The generated lightweight portal is:

    docs/reference/SOURCE_SYMBOL_DICTIONARIES.md

A comprehensive deterministic aggregate view can be generated with:

    python3 scripts/source-dictionary.py aggregate

Generated views do not replace directory-owned descriptions.

## Planned exact-instance ignore registry

### Status

    IMPLEMENTED=NO
    ADD_WHEN=FIRST_REAL_REVIEWED_FALSE_POSITIVE_OR_EQUIVALENT_NEED

The validator intentionally does not yet contain a general ignore mechanism.

A future exact-instance ignore registry is planned for the case where
definition discovery reports a specific item that has been manually reviewed
and is known not to require a dictionary entry.

A motivating example is a parser edge case in which an obscure C construct is
mechanically classified as a project function or variable even though the
maintainer has verified that the finding is not a useful dictionary obligation.

The intended operation is:

> I reviewed this exact validator finding. Its absence from the symbol
> dictionary is intentional. Remember that decision and stop reporting this
> exact finding.

The registry should identify the same stable logical definition identity used
by discovery:

    file + name + kind + owner

A human-readable reason must also be required.

### Intended safety rules

When this facility becomes useful, its implementation should remain narrow:

- exact instances only;
- no wildcard names;
- no blanket file suppression through the symbol-ignore mechanism;
- no line number as stable identity;
- a required human explanation;
- exact kind and owner matching;
- duplicate or malformed ignore records are structural failures;
- an ignore entry suppresses only the corresponding
  `MISSING_DICTIONARY_SYMBOL`;
- parser failures, unsupported syntax, dictionary corruption, and unrelated
  structural errors must never be suppressible by a symbol ignore;
- an ignore whose matching source definition disappears or changes identity
  should become a stale-ignore maintenance finding rather than silently
  persisting forever.

Conceptually, discovery should eventually classify every discovered definition
into exactly one of three states:

    documented
    intentionally ignored
    unexplained missing

Only the third state should produce `MISSING_DICTIONARY_SYMBOL`.

### Why it is deferred

The ignore registry is deliberately deferred until a real reviewed exception
exists.

There is no value in spending additional development time designing a broad
suppression framework speculatively.

The first genuine false-positive or equivalent justified exception should be
used as the concrete fixture for the smallest implementation that satisfies
the rules above.

## Current development rule

The validator is mature enough to support current development and should not
become an open-ended tooling project.

Until evidence demonstrates a real defect:

- do not redesign working language adapters;
- do not broaden parser semantics speculatively;
- allow ordinary retrofit work to remain `ATTENTION`;
- preserve structural fail-closed behavior;
- add the exact-instance ignore facility only when a real reviewed finding
  proves it useful;
- prioritize product development over further validator refinement.

The remaining dictionary retrofit can proceed incrementally and does not need
to block unrelated clean-reconstruction implementation.
