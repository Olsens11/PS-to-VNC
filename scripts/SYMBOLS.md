# Clean symbols — `scripts`

DIRECTORY=scripts
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the clean Issue #7 linked-build/compile verification tools,
the current-safe historical-stage reproducibility wrapper, and the hierarchical
source-dictionary tooling. Migration-era and historical scripts in this
directory remain deliberately outside the clean-generation scope. Successor
TestKit tooling is owned separately by `scripts/testkit`.

This dictionary is an inventory and orientation surface, not an execution
manifest. A tool's presence here documents what it is; it does not establish
that the tool is applicable to the current task or should be invoked.

**Issue #7 operational note:** `check-issue7-linked-reproducibility.sh` is
stage-local. Outside exact Issue #7 authority it intentionally reports
`SKIPPED` / `NOT_APPLICABLE`, performs no build, and returns success so an
unrelated workflow continues.

When a later-stage task actually requires Issue #7 reproducibility evidence,
use `check-historical-issue7-reproducibility.sh` and require its genuine PASS
result.

`SKIPPED` is not equivalent to `PASS`. Never broaden the historical Issue #7
object set merely to make current-stage source link through the old stage-local
build.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| ROOT | variable | scripts/build-issue7-clean.sh | linked build | file | Resolves the repository root used by every clean-build path. | Issue #7 linked build |
| BUILD_DIR | variable | scripts/build-issue7-clean.sh | linked build | file | Names the disposable canonical Issue #7 linked-build output directory. | Issue #7 linked build |
| DEP_DIR | variable | scripts/build-issue7-clean.sh | linked build | file | Names the build-local directory containing verified copied dependencies. | dependency authority |
| ELF | variable | scripts/build-issue7-clean.sh | linked build | file | Names the canonical pristine identity-ready Issue #7 ELF output. | DUT identity |
| FROZEN_DEP | variable | scripts/build-issue7-clean.sh | linked build | file | Points to the preserved qualified PS2IP archive used as build input. | dependency authority |
| BUILD_DEP | variable | scripts/build-issue7-clean.sh | linked build | file | Names the verified build-local copy of the qualified PS2IP archive. | dependency authority |
| EXPECTED_DEP_SHA | variable | scripts/build-issue7-clean.sh | linked build | file | Pins the SHA256 identity required for the qualified PS2IP archive. | dependency authority |
| IMAGE | variable | scripts/build-issue7-clean.sh | linked build | file | Pins the exact ps2dev container image digest used for linked builds. | reproducible build |
| TOOLCHAIN_PATH | variable | scripts/build-issue7-clean.sh | linked build | file | Defines the deterministic PS2 toolchain search path inside the container. | reproducible build |
| actual_dep_sha | variable | scripts/build-issue7-clean.sh | linked build | local | Holds the observed frozen dependency SHA256 before any build occurs. | fail-closed dependency check |
| HOST_UID | variable | scripts/build-issue7-clean.sh | linked build | file | Captures the invoking host UID for deterministic artifact ownership repair. | build mechanics |
| HOST_GID | variable | scripts/build-issue7-clean.sh | linked build | file | Captures the invoking host GID for deterministic artifact ownership repair. | build mechanics |
| ROOT | variable | scripts/check-clean-ps2-compile.sh | strict compile check | file | Resolves the repository root for clean translation-unit compilation. | strict R5900 compile |
| PS2DEV | variable | scripts/check-clean-ps2-compile.sh | strict compile check | environment | Names the PS2 development toolchain root with a deterministic default. | strict R5900 compile |
| PS2SDK | variable | scripts/check-clean-ps2-compile.sh | strict compile check | environment | Names the PS2SDK root derived from PS2DEV unless explicitly supplied. | strict R5900 compile |
| GSKIT | variable | scripts/check-clean-ps2-compile.sh | strict compile check | environment | Names the gsKit root derived from PS2DEV unless explicitly supplied. | strict R5900 compile |
| CC | variable | scripts/check-clean-ps2-compile.sh | strict compile check | file | Names the exact R5900 C compiler executable required by the check. | strict R5900 compile |
| BUILD_DIR | variable | scripts/check-clean-ps2-compile.sh | strict compile check | file | Owns the temporary directory used solely for strict compile objects. | host-side build isolation |
| COMMON_FLAGS | array | scripts/check-clean-ps2-compile.sh | strict compile check | file | Defines the warning, ABI, architecture, language, and include flags applied to every unit. | strict R5900 compile |
| SOURCES | array | scripts/check-clean-ps2-compile.sh | strict compile check | file | Enumerates every clean Issue #7 C translation unit required to compile. | clean build dependency surface |
| source | variable | scripts/check-clean-ps2-compile.sh | compile loop | local | Holds the current clean translation-unit path being compiled. | strict R5900 compile |
| object | variable | scripts/check-clean-ps2-compile.sh | compile loop | local | Names the temporary object output corresponding to the current source. | strict R5900 compile |
| MAGIC | constant | scripts/check-issue7-identity-blob.py | pristine identity check | file | Defines the exact runtime-identity blob magic expected in the linked ELF. | deterministic DUT identity |
| TEST_ID_SIZE | constant | scripts/check-issue7-identity-blob.py | pristine identity check | file | Defines the fixed byte capacity of the stampable test-identity field. | deterministic DUT identity |
| DIGEST_SIZE | constant | scripts/check-issue7-identity-blob.py | pristine identity check | file | Defines the fixed byte capacity of the textual digest field including terminator. | deterministic DUT identity |
| fail | function | scripts/check-issue7-identity-blob.py | pristine identity check | private | Emits a machine-readable failure reason and terminates the verifier. | fail-closed identity proof |
| message | parameter | scripts/check-issue7-identity-blob.py | fail | local | Supplies the exact diagnostic reason attached to an identity-check failure. | fail-closed identity proof |
| main | function | scripts/check-issue7-identity-blob.py | pristine identity check | private | Locates and verifies exactly one pristine identity blob in the supplied ELF. | deterministic DUT identity |
| path | variable | scripts/check-issue7-identity-blob.py | main | local | Holds the filesystem path of the candidate linked ELF. | deterministic DUT identity |
| data | variable | scripts/check-issue7-identity-blob.py | main | local | Holds the complete ELF bytes used for blob and whole-file identity checks. | deterministic DUT identity |
| offsets | variable | scripts/check-issue7-identity-blob.py | main | local | Collects every occurrence of the identity magic so multiplicity fails closed. | deterministic DUT identity |
| start | variable | scripts/check-issue7-identity-blob.py | main | local | Tracks the next byte position used while searching for identity magic. | deterministic DUT identity |
| offset | variable | scripts/check-issue7-identity-blob.py | main | local | Holds either a discovered magic location or the unique validated blob offset. | deterministic DUT identity |
| test_start | variable | scripts/check-issue7-identity-blob.py | main | local | Computes the first byte of the fixed test-identity field. | identity blob layout |
| digest_start | variable | scripts/check-issue7-identity-blob.py | main | local | Computes the first byte of the fixed textual digest field. | identity blob layout |
| end | variable | scripts/check-issue7-identity-blob.py | main | local | Computes the exclusive end of the complete fixed-size identity blob. | identity blob bounds |
| test_id | variable | scripts/check-issue7-identity-blob.py | main | local | Holds the observed pristine test-identity field bytes. | deterministic DUT identity |
| digest | variable | scripts/check-issue7-identity-blob.py | main | local | Holds the observed pristine digest field bytes. | deterministic DUT identity |
| expected_test | variable | scripts/check-issue7-identity-blob.py | main | local | Constructs the exact UNSTAMPED field representation required before preparation. | pristine identity contract |
| expected_digest | variable | scripts/check-issue7-identity-blob.py | main | local | Constructs the exact all-zero digest representation required before preparation. | pristine identity contract |
| ROOT | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Resolves the repository root for the two-build reproducibility proof. | reproducible linked build |
| BUILD | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Names the canonical successor clean linked-build script under test. | reproducible linked build |
| PT | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Names the preserved PT_LOAD fingerprint helper used for runtime-byte comparison. | PT_LOAD evidence |
| ELF | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Names the canonical clean linked ELF produced by each build. | reproducible linked build |
| tool | variable | scripts/check-issue7-linked-reproducibility.sh | prerequisite loop | local | Holds each executable prerequisite while fail-closed availability is checked. | build prerequisites |
| TMP | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Owns disposable copies, logs, and fingerprints for independent build comparison. | reproducible linked build |
| cleanup | function | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | private | Removes the isolated comparison workspace on every normal shell exit. | build hygiene |
| sha256 | function | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | private | Returns only the SHA256 digest of one supplied file. | reproducible linked build |
| value_from_file | function | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | private | Extracts exactly one machine-readable key value and rejects missing or duplicate records. | fail-closed evidence parsing |
| key | variable | scripts/check-issue7-linked-reproducibility.sh | value_from_file | local | Holds the machine-readable evidence key requested by the caller. | fail-closed evidence parsing |
| file | variable | scripts/check-issue7-linked-reproducibility.sh | value_from_file | local | Names the evidence file from which exactly one key value is required. | fail-closed evidence parsing |
| value | variable | scripts/check-issue7-linked-reproducibility.sh | value_from_file | local | Holds all matching values before uniqueness is enforced. | fail-closed evidence parsing |
| count | variable | scripts/check-issue7-linked-reproducibility.sh | value_from_file | local | Counts nonempty matches so ambiguous evidence cannot silently pass. | fail-closed evidence parsing |
| BUILD1_SHA | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Holds the whole-ELF SHA256 from the first independent clean build. | reproducible linked build |
| BUILD1_PT_SHA | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Holds the PT_LOAD digest from the first independent clean build. | PT_LOAD evidence |
| BUILD1_PT_BYTES | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Holds the PT_LOAD byte count from the first independent clean build. | PT_LOAD evidence |
| BUILD2_SHA | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Holds the whole-ELF SHA256 from the second independent clean build. | reproducible linked build |
| BUILD2_PT_SHA | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Holds the PT_LOAD digest from the second independent clean build. | PT_LOAD evidence |
| BUILD2_PT_BYTES | variable | scripts/check-issue7-linked-reproducibility.sh | linked reproducibility | file | Holds the PT_LOAD byte count from the second independent clean build. | PT_LOAD evidence |
| CLEAN_MARKER | constant | scripts/source-dictionary.py | dictionary validator | private | Defines the file-orientation marker that places source in clean-generation dictionary scope. | source-naming-and-symbols policy |
| DICT_NAME | constant | scripts/source-dictionary.py | dictionary validator | private | Defines the canonical filename used for each directory-owned symbol dictionary. | source-naming-and-symbols policy |
| EXCLUDED_PARTS | constant | scripts/source-dictionary.py | dictionary validator | private | Names historical, generated, and repository-control path components excluded from discovery. | clean-generation boundary |
| PLACEHOLDERS | constant | scripts/source-dictionary.py | dictionary validator | private | Defines inadequate description values that always fail validation. | dictionary description quality |
| HEADER | constant | scripts/source-dictionary.py | dictionary validator | private | Defines the exact ordered Markdown columns required for symbol entries. | dictionary format contract |
| COVERAGE_STATES | constant | scripts/source-dictionary.py | dictionary validator | private | Defines the only coverage claims accepted from a directory-owned dictionary. | dictionary completeness gate |
| STATE_PATH | constant | scripts/source-dictionary.py | dictionary baseline authority | private | Names the tracked machine-readable state file holding trusted comprehensive audit authority. | incremental dictionary validation |
| STATE_VERSION | constant | scripts/source-dictionary.py | dictionary baseline authority | private | Defines the supported schema version for source-dictionary baseline state. | baseline state integrity |
| BASELINE_UNSET | constant | scripts/source-dictionary.py | dictionary baseline authority | private | Represents the deliberate absence of any proven comprehensive definition-audit baseline. | baseline bootstrap |
| DEFINITION_DISCOVERY_STATUS | constant | scripts/source-dictionary.py | definition discovery gate | private | Prevents trusted baseline recording until project-definition discovery is genuinely implemented. | baseline safety |
| DictionaryError | type | scripts/source-dictionary.py | dictionary validator | private | Represents a structural dictionary failure that makes validation results untrustworthy. | dictionary integrity |
| Entry | type | scripts/source-dictionary.py | dictionary validator | private | Holds one parsed symbol description together with its source dictionary location. | dictionary validation |
| name | field | scripts/source-dictionary.py | Entry | private | Stores the exact project-defined symbol name described by an entry. | dictionary schema |
| kind | field | scripts/source-dictionary.py | Entry | private | Stores the symbol category used to disambiguate definitions. | dictionary schema |
| file | field | scripts/source-dictionary.py | Entry | private | Stores the repository-relative defining source path. | dictionary schema |
| owner | field | scripts/source-dictionary.py | Entry | private | Stores the owning module, function, or responsibility context. | dictionary schema |
| scope | field | scripts/source-dictionary.py | Entry | private | Stores the symbol visibility or lifetime classification. | dictionary schema |
| description | field | scripts/source-dictionary.py | Entry | private | Stores the required plain-language meaning of the symbol. | dictionary schema |
| context | field | scripts/source-dictionary.py | Entry | private | Stores an optional durable architecture or qualification context pointer. | dictionary schema |
| dictionary | field | scripts/source-dictionary.py | Entry | private | Stores the dictionary file from which the entry was parsed. | validation diagnostics |
| line | field | scripts/source-dictionary.py | Entry | private | Stores the source dictionary line number used in fail-closed diagnostics. | validation diagnostics |
| is_excluded | function | scripts/source-dictionary.py | dictionary validator | private | Reports whether a candidate path crosses an explicitly excluded tree boundary. | clean-generation boundary |
| path | parameter | scripts/source-dictionary.py | is_excluded | local | Supplies the candidate filesystem path being classified. | clean-generation boundary |
| root | parameter | scripts/source-dictionary.py | is_excluded | local | Supplies the repository root used to obtain relative path components. | clean-generation boundary |
| clean_files | function | scripts/source-dictionary.py | dictionary validator | private | Discovers clean-generation source files by their required file-synopsis marker. | clean-generation boundary |
| root | parameter | scripts/source-dictionary.py | clean_files | local | Supplies the repository root scanned for maintained source files. | clean-generation boundary |
| result | variable | scripts/source-dictionary.py | clean_files | local | Accumulates repository-relative clean-generation files discovered by marker. | clean-generation boundary |
| suffix | variable | scripts/source-dictionary.py | clean_files | local | Iterates the maintained source-language glob patterns accepted by discovery. | clean-generation boundary |
| path | variable | scripts/source-dictionary.py | clean_files | local | Holds each candidate source path examined for exclusion and synopsis marking. | clean-generation boundary |
| unescape | function | scripts/source-dictionary.py | dictionary parser | private | Normalizes one Markdown table cell into its stored dictionary value. | dictionary format contract |
| cell | parameter | scripts/source-dictionary.py | unescape | local | Supplies one raw Markdown table cell for normalization. | dictionary format contract |
| parse_dictionary | function | scripts/source-dictionary.py | dictionary parser | private | Parses directory identity, coverage state, and symbol rows from one dictionary. | dictionary format contract |
| path | parameter | scripts/source-dictionary.py | parse_dictionary | local | Supplies the dictionary file being parsed. | dictionary format contract |
| root | parameter | scripts/source-dictionary.py | parse_dictionary | local | Supplies repository authority for directory ownership validation. | dictionary ownership |
| lines | variable | scripts/source-dictionary.py | parse_dictionary | local | Holds all dictionary lines for deterministic single-pass parsing. | dictionary format contract |
| directory | variable | scripts/source-dictionary.py | parse_dictionary | local | Holds the declared directory owner parsed from metadata. | dictionary ownership |
| coverage | variable | scripts/source-dictionary.py | parse_dictionary | local | Holds the declared IN_PROGRESS or COMPLETE coverage claim. | dictionary completeness gate |
| entries | variable | scripts/source-dictionary.py | parse_dictionary | local | Accumulates validated-shape symbol rows parsed from the table. | dictionary format contract |
| in_table | variable | scripts/source-dictionary.py | parse_dictionary | local | Tracks whether the parser is currently consuming symbol table rows. | dictionary format contract |
| number | variable | scripts/source-dictionary.py | parse_dictionary | local | Tracks the one-based dictionary line number for diagnostics. | validation diagnostics |
| line | variable | scripts/source-dictionary.py | parse_dictionary | local | Holds the current dictionary line being interpreted. | dictionary format contract |
| cells | variable | scripts/source-dictionary.py | parse_dictionary | local | Holds normalized Markdown cells for a header or symbol row. | dictionary format contract |
| allowed | variable | scripts/source-dictionary.py | parse_dictionary | local | Formats accepted coverage states for an invalid-state diagnostic. | validation diagnostics |
| expected | variable | scripts/source-dictionary.py | parse_dictionary | local | Computes the directory metadata value required by dictionary location. | dictionary ownership |
| run_git | function | scripts/source-dictionary.py | Git state adapter | private | Executes one read-oriented Git query and converts unexpected query failure into dictionary integrity failure. | trusted baseline mechanics |
| root | parameter | scripts/source-dictionary.py | run_git | local | Supplies the repository in which the Git query executes. | trusted baseline mechanics |
| arguments | parameter | scripts/source-dictionary.py | run_git | local | Supplies the exact Git subcommand and arguments to execute. | trusted baseline mechanics |
| check | parameter | scripts/source-dictionary.py | run_git | local | Selects whether a nonzero Git result is an integrity failure or an expected probe result. | trusted baseline mechanics |
| completed | variable | scripts/source-dictionary.py | run_git | local | Holds the completed Git query with status and captured output. | trusted baseline mechanics |
| detail | variable | scripts/source-dictionary.py | run_git | local | Holds available Git failure output for an integrity diagnostic. | validation diagnostics |
| read_long_pass_baseline | function | scripts/source-dictionary.py | dictionary baseline authority | private | Reads and validates the tracked commit identifying the last trusted comprehensive dictionary audit. | incremental dictionary validation |
| root | parameter | scripts/source-dictionary.py | read_long_pass_baseline | local | Supplies repository authority for resolving baseline state. | baseline state integrity |
| state_path | variable | scripts/source-dictionary.py | read_long_pass_baseline | local | Resolves the repository-local source-dictionary baseline state file. | baseline state integrity |
| fields | variable | scripts/source-dictionary.py | read_long_pass_baseline | local | Collects unique machine-readable baseline state fields. | baseline state integrity |
| line | variable | scripts/source-dictionary.py | read_long_pass_baseline | local | Holds one baseline state line during strict parsing. | baseline state integrity |
| key | variable | scripts/source-dictionary.py | read_long_pass_baseline | local | Holds one parsed baseline state field name. | baseline state integrity |
| value | variable | scripts/source-dictionary.py | read_long_pass_baseline | local | Holds one parsed baseline state field value. | baseline state integrity |
| version | variable | scripts/source-dictionary.py | read_long_pass_baseline | local | Holds the declared baseline state schema version. | baseline state integrity |
| baseline | variable | scripts/source-dictionary.py | read_long_pass_baseline | local | Holds the stored trusted long-pass commit or explicit UNSET marker. | incremental dictionary validation |
| changed_paths_since_baseline | function | scripts/source-dictionary.py | Git state adapter | private | Combines committed, staged, unstaged, and untracked path changes since the trusted baseline. | incremental dictionary validation |
| root | parameter | scripts/source-dictionary.py | changed_paths_since_baseline | local | Supplies the repository whose complete change set is queried. | trusted baseline mechanics |
| baseline | parameter | scripts/source-dictionary.py | changed_paths_since_baseline | local | Supplies the trusted comprehensive commit used as the committed-diff origin. | trusted baseline mechanics |
| paths | variable | scripts/source-dictionary.py | changed_paths_since_baseline | local | Accumulates unique repository-relative paths changed since the baseline. | incremental dictionary validation |
| commands | variable | scripts/source-dictionary.py | changed_paths_since_baseline | local | Defines committed, index, worktree, and untracked Git queries required for complete delta coverage. | incremental dictionary validation |
| command | variable | scripts/source-dictionary.py | changed_paths_since_baseline | local | Holds one Git query while assembling the baseline delta. | incremental dictionary validation |
| completed | variable | scripts/source-dictionary.py | changed_paths_since_baseline | local | Holds one completed Git delta query and its NUL-delimited output. | incremental dictionary validation |
| value | variable | scripts/source-dictionary.py | changed_paths_since_baseline | local | Holds one repository-relative path decoded from Git output. | incremental dictionary validation |
| is_definition_source_path | function | scripts/source-dictionary.py | definition scope selector | private | Reports whether a changed path belongs to a source language admitted by current clean-source discovery. | incremental dictionary validation |
| path | parameter | scripts/source-dictionary.py | is_definition_source_path | local | Supplies one changed repository path for source-language classification. | incremental dictionary validation |
| resolve_definition_scope | function | scripts/source-dictionary.py | definition scope selector | private | Selects explicit long, safety-fallback long, or trusted-baseline incremental definition scope without weakening structural validation. | source-naming-and-symbols policy |
| root | parameter | scripts/source-dictionary.py | resolve_definition_scope | local | Supplies repository authority for baseline and Git-delta resolution. | incremental dictionary validation |
| force_long | parameter | scripts/source-dictionary.py | resolve_definition_scope | local | Requests comprehensive scope regardless of available trusted baseline. | comprehensive dictionary audit |
| baseline | variable | scripts/source-dictionary.py | resolve_definition_scope | local | Holds the tracked long-pass commit or UNSET marker while selecting audit mode. | incremental dictionary validation |
| commit_probe | variable | scripts/source-dictionary.py | resolve_definition_scope | local | Tests whether the stored baseline still resolves to a Git commit. | baseline safety fallback |
| ancestor_probe | variable | scripts/source-dictionary.py | resolve_definition_scope | local | Tests whether the stored baseline remains an ancestor of current HEAD. | baseline safety fallback |
| changed | variable | scripts/source-dictionary.py | resolve_definition_scope | local | Holds all repository paths changed since a usable trusted baseline. | incremental dictionary validation |
| candidates | variable | scripts/source-dictionary.py | resolve_definition_scope | local | Filters the complete Git delta to paths requiring definition discovery. | incremental dictionary validation |
| path | variable | scripts/source-dictionary.py | resolve_definition_scope | local | Holds one changed path while filtering incremental definition candidates. | incremental dictionary validation |
| write_long_pass_baseline | function | scripts/source-dictionary.py | dictionary baseline authority | private | Writes an explicitly proven comprehensive audit commit to tracked baseline state. | explicit baseline advancement |
| root | parameter | scripts/source-dictionary.py | write_long_pass_baseline | local | Supplies repository authority for the tracked baseline state update. | explicit baseline advancement |
| commit | parameter | scripts/source-dictionary.py | write_long_pass_baseline | local | Supplies the clean audited HEAD to record as trusted comprehensive authority. | explicit baseline advancement |
| state_path | variable | scripts/source-dictionary.py | write_long_pass_baseline | local | Resolves the tracked baseline state file being intentionally updated. | explicit baseline advancement |
| validate | function | scripts/source-dictionary.py | dictionary validator | private | Validates structural integrity while collecting ordinary dictionary-maintenance findings for deterministic reporting. | source-naming-and-symbols policy |
| root | parameter | scripts/source-dictionary.py | validate | local | Supplies repository authority for complete validation. | dictionary validation |
| require_complete | parameter | scripts/source-dictionary.py | validate | local | Requests attention findings for directories that still declare incomplete coverage. | dictionary completeness gate |
| dictionaries | variable | scripts/source-dictionary.py | validate | local | Accumulates parsed directory dictionaries for views and result reporting. | dictionary validation |
| covered | variable | scripts/source-dictionary.py | validate | local | Tracks clean files represented by at least one valid dictionary entry. | dictionary file coverage |
| keys | variable | scripts/source-dictionary.py | validate | local | Tracks compound entry identities used to reject ambiguous duplicates. | dictionary uniqueness |
| incomplete | variable | scripts/source-dictionary.py | validate | local | Collects directories whose coverage claim is not COMPLETE for optional attention reporting. | dictionary completeness gate |
| attention | variable | scripts/source-dictionary.py | validate | local | Collects deterministic nonfatal maintenance findings discovered during validation. | validation diagnostics |
| path | variable | scripts/source-dictionary.py | validate | local | Holds each discovered dictionary path during deterministic validation. | dictionary validation |
| directory | variable | scripts/source-dictionary.py | validate | local | Holds the parsed owner directory for the current dictionary. | dictionary ownership |
| coverage | variable | scripts/source-dictionary.py | validate | local | Holds the parsed completeness state for the current dictionary. | dictionary completeness gate |
| entries | variable | scripts/source-dictionary.py | validate | local | Holds the parsed symbol entries belonging to the current directory. | dictionary validation |
| entry | variable | scripts/source-dictionary.py | validate | local | Holds the current symbol description under validation. | dictionary validation |
| source | variable | scripts/source-dictionary.py | validate | local | Resolves the defining source file referenced by the current entry. | dictionary ownership |
| source_text | variable | scripts/source-dictionary.py | validate | local | Holds defining source text used to reject stale symbol names. | stale-entry detection |
| key | variable | scripts/source-dictionary.py | validate | local | Holds the compound identity used for duplicate-entry rejection. | dictionary uniqueness |
| missing | variable | scripts/source-dictionary.py | validate | local | Holds clean-generation files that receive maintenance attention because no dictionary entry represents them yet. | dictionary file coverage |
| render_portal | function | scripts/source-dictionary.py | dictionary generator | private | Produces lightweight directory navigation with coverage state and symbol counts. | generated dictionary portal |
| root | parameter | scripts/source-dictionary.py | render_portal | local | Supplies repository authority for relative dictionary links. | generated dictionary portal |
| dictionaries | parameter | scripts/source-dictionary.py | render_portal | local | Supplies validated dictionary records for deterministic rendering. | generated dictionary portal |
| lines | variable | scripts/source-dictionary.py | render_portal | local | Accumulates deterministic Markdown output lines. | generated dictionary portal |
| directory | variable | scripts/source-dictionary.py | render_portal | local | Holds each directory name while rendering one portal row. | generated dictionary portal |
| coverage | variable | scripts/source-dictionary.py | render_portal | local | Holds each directory coverage claim while rendering one portal row. | generated dictionary portal |
| path | variable | scripts/source-dictionary.py | render_portal | local | Holds each dictionary path while rendering its navigation link. | generated dictionary portal |
| entries | variable | scripts/source-dictionary.py | render_portal | local | Supplies each directory symbol count for portal rendering. | generated dictionary portal |
| rel | variable | scripts/source-dictionary.py | render_portal | local | Holds the repository-relative dictionary link target. | generated dictionary portal |
| render_aggregate | function | scripts/source-dictionary.py | dictionary generator | private | Produces the optional comprehensive Markdown symbol view in deterministic order. | generated dictionary aggregate |
| dictionaries | parameter | scripts/source-dictionary.py | render_aggregate | local | Supplies validated dictionaries to the comprehensive renderer. | generated dictionary aggregate |
| lines | variable | scripts/source-dictionary.py | render_aggregate | local | Accumulates deterministic aggregate Markdown lines. | generated dictionary aggregate |
| directory | variable | scripts/source-dictionary.py | render_aggregate | local | Holds each directory heading while rendering the aggregate. | generated dictionary aggregate |
| coverage | variable | scripts/source-dictionary.py | render_aggregate | local | Holds each directory coverage state displayed in the aggregate. | generated dictionary aggregate |
| entries | variable | scripts/source-dictionary.py | render_aggregate | local | Supplies the symbol rows rendered for one directory. | generated dictionary aggregate |
| entry | variable | scripts/source-dictionary.py | render_aggregate | local | Holds the current sorted symbol entry being rendered. | generated dictionary aggregate |
| values | variable | scripts/source-dictionary.py | render_aggregate | local | Holds escaped cell values for the current aggregate table row. | generated dictionary aggregate |
| main | function | scripts/source-dictionary.py | dictionary command line | private | Parses the requested operation and separates hard integrity failures from soft maintenance attention or strict-gate failure. | dictionary tooling entry point |
| parser | variable | scripts/source-dictionary.py | main | local | Owns the command-line parser for dictionary operations. | dictionary tooling entry point |
| args | variable | scripts/source-dictionary.py | main | local | Holds parsed command-line arguments supplied to the tool. | dictionary tooling entry point |
| dictionaries | variable | scripts/source-dictionary.py | main | local | Holds validated dictionaries returned for checking or rendering. | dictionary tooling entry point |
| attention | variable | scripts/source-dictionary.py | main | local | Holds maintenance findings returned by validation for normal or strict result handling. | dictionary tooling entry point |
| finding | variable | scripts/source-dictionary.py | main | local | Holds one maintenance finding while producing deterministic operator diagnostics. | validation diagnostics |
| root | variable | scripts/source-dictionary.py | main | local | Resolves repository authority once for validation, audit-scope selection, and explicit baseline recording. | dictionary tooling entry point |
| check_mode | variable | scripts/source-dictionary.py | main | local | Holds whether definition scope is incremental, explicit long, or safety-fallback long. | incremental dictionary validation |
| baseline | variable | scripts/source-dictionary.py | main | local | Holds the trusted long-pass commit reported for the current check. | incremental dictionary validation |
| baseline_reason | variable | scripts/source-dictionary.py | main | local | Explains why the current check uses incremental or comprehensive definition scope. | validation diagnostics |
| definition_scope | variable | scripts/source-dictionary.py | main | local | Holds source paths selected for current definition discovery. | incremental dictionary validation |
| path | variable | scripts/source-dictionary.py | main | local | Holds one selected incremental source path while reporting definition scope. | validation diagnostics |
| status | variable | scripts/source-dictionary.py | main | local | Holds the clean-worktree Git result required before explicit baseline recording. | explicit baseline advancement |
| head | variable | scripts/source-dictionary.py | main | local | Holds the clean current commit recorded after a passing comprehensive audit. | explicit baseline advancement |
| rendered | variable | scripts/source-dictionary.py | main | local | Holds generated portal or aggregate Markdown before output. | deterministic generation |
| exc | variable | scripts/source-dictionary.py | main | local | Holds a structural validation exception while formatting the hard-failure diagnostic. | dictionary integrity |
| TOOL | constant | scripts/source-dictionary-self-test.py | dictionary self-test | private | Resolves the validator executable exercised by disposable regression fixtures. | dictionary tooling self-test |
| write | function | scripts/source-dictionary-self-test.py | dictionary self-test | private | Creates one fixture file and any required parent directories. | dictionary tooling self-test |
| root | parameter | scripts/source-dictionary-self-test.py | write | local | Supplies the disposable fixture repository root. | dictionary tooling self-test |
| relative | parameter | scripts/source-dictionary-self-test.py | write | local | Supplies the repository-relative fixture file path. | dictionary tooling self-test |
| text | parameter | scripts/source-dictionary-self-test.py | write | local | Supplies exact UTF-8 fixture contents to persist. | dictionary tooling self-test |
| path | variable | scripts/source-dictionary-self-test.py | write | local | Resolves the concrete fixture path being written. | dictionary tooling self-test |
| run | function | scripts/source-dictionary-self-test.py | dictionary self-test | private | Executes the validator against a disposable fixture and checks its exit status. | dictionary tooling self-test |
| root | parameter | scripts/source-dictionary-self-test.py | run | local | Supplies the disposable fixture repository root passed to validation. | dictionary tooling self-test |
| expected | parameter | scripts/source-dictionary-self-test.py | run | local | Supplies the expected validator process exit status. | dictionary tooling self-test |
| command | parameter | scripts/source-dictionary-self-test.py | run | local | Selects check, portal, or aggregate behavior for the fixture run. | dictionary tooling self-test |
| require_complete | parameter | scripts/source-dictionary-self-test.py | run | local | Requests incomplete-coverage attention findings for the fixture run. | dictionary completeness self-test |
| strict | parameter | scripts/source-dictionary-self-test.py | run | local | Selects whether fixture maintenance findings must produce a nonzero strict-gate result. | dictionary strict-mode self-test |
| long | parameter | scripts/source-dictionary-self-test.py | run | local | Selects explicit comprehensive definition scope for one validator fixture run. | comprehensive audit self-test |
| record_baseline | parameter | scripts/source-dictionary-self-test.py | run | local | Selects explicit trusted-baseline recording for one validator fixture run. | baseline recording self-test |
| arguments | variable | scripts/source-dictionary-self-test.py | run | local | Builds the exact subprocess argument vector for the validator invocation. | dictionary tooling self-test |
| completed | variable | scripts/source-dictionary-self-test.py | run | local | Holds the completed validator subprocess result for assertion and output inspection. | dictionary tooling self-test |
| dictionary | function | scripts/source-dictionary-self-test.py | dictionary self-test | private | Builds fixture dictionary text with an explicit coverage state. | dictionary tooling self-test |
| rows | parameter | scripts/source-dictionary-self-test.py | dictionary | local | Supplies fixture symbol-table rows to embed in generated dictionary text. | dictionary tooling self-test |
| coverage | parameter | scripts/source-dictionary-self-test.py | dictionary | local | Supplies the fixture coverage state written into dictionary metadata. | dictionary completeness self-test |
| temporary | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Holds the temporary-directory path string supplied by the context manager. | dictionary tooling self-test |
| root | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Wraps the temporary directory as the fixture repository root Path. | dictionary tooling self-test |
| source | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Holds clean-marked fixture C source reused by positive and negative cases. | dictionary tooling self-test |
| row | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Holds the canonical valid fixture symbol row before targeted mutations. | dictionary tooling self-test |
| portal_a | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Holds the first generated portal used in deterministic-output comparison. | deterministic generation self-test |
| portal_b | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Holds the second generated portal used in deterministic-output comparison. | deterministic generation self-test |
| output | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Holds validator output used to assert audit-mode and baseline-fallback behavior. | baseline mode self-test |
| record_output | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Holds expected command-line refusal output while definition discovery remains pending. | baseline safety self-test |
| baseline_head | variable | scripts/source-dictionary-self-test.py | fixture runner | local | Holds a committed fixture head used as synthetic trusted comprehensive baseline authority. | incremental scope self-test |
| ROOT | variable | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Resolves the current repository root whose object database supplies the pinned historical source snapshot. | stage-specific reproducibility |
| ISSUE7_SOURCE_COMMIT | constant | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Pins the exact final Issue #7 source authority used for isolated historical reproduction. | historical source authority |
| EXPECTED_ELF_SHA | constant | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Pins the recorded pristine whole-ELF identity that the historical Issue #7 build must reproduce. | reproducible linked build |
| EXPECTED_PT_LOAD_SHA | constant | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Pins the recorded pristine Issue #7 PT_LOAD digest required for qualification continuity. | PT_LOAD evidence |
| EXPECTED_PT_LOAD_BYTES | constant | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Pins the recorded pristine Issue #7 PT_LOAD byte count. | PT_LOAD evidence |
| TMP | variable | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Owns the disposable historical source snapshot, build output, and verification evidence. | isolated historical build |
| SNAP | variable | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Names the disposable repository tree exported from the pinned Issue #7 source commit. | isolated historical build |
| cleanup | function | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | private | Removes the isolated historical snapshot and build evidence on shell exit. | build hygiene |
| ELF | variable | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Names the canonical Issue #7 ELF produced inside the pinned historical snapshot. | reproducible linked build |
| ACTUAL_ELF_SHA | variable | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Holds the reproduced historical Issue #7 whole-ELF SHA256 for exact authority comparison. | reproducible linked build |
| ACTUAL_PT_LOAD_SHA | variable | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Holds the reproduced historical Issue #7 PT_LOAD SHA256 for exact authority comparison. | PT_LOAD evidence |
| ACTUAL_PT_LOAD_BYTES | variable | scripts/check-historical-issue7-reproducibility.sh | historical Issue7 wrapper | file | Holds the reproduced historical Issue #7 PT_LOAD byte count for exact authority comparison. | PT_LOAD evidence |
| ISSUE7_STAGE_LOCAL_SOURCE_AUTHORITY | constant | scripts/check-issue7-linked-reproducibility.sh | Issue7 linked reproducibility | file | Pins the exact final Issue #7 source authority at which the stage-local two-build proof is applicable. | stage applicability |
| CURRENT_SOURCE_COMMIT | variable | scripts/check-issue7-linked-reproducibility.sh | Issue7 linked reproducibility | file | Holds the checkout's current commit so the stage-local tool can distinguish a real Issue #7 proof from a non-applicable later-stage invocation. | stage applicability |
