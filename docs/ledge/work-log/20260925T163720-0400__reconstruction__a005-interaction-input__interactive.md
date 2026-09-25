DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T16:37:20-04:00
COMPLETED_AT=2026-09-25T16:51:42-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a005-interaction-input
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=25dc15455be2ae68b535d74d5b1dc298ccd7fcac
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — B10 R30 human-readable product-action bindings

## Recovery provenance

This interactive Reconstruction shift began by independently refreshing the live
`ledge/h1-all-guns` branch and recovering repository authority rather than
trusting the commissioning transition marker.

The live branch at recovery was exactly:

`25dc15455be2ae68b535d74d5b1dc298ccd7fcac`

message:

`docs(work-log): accept R29 and hand off binding config R30`.

A direct branch comparison proved the commissioning head and live branch were
identical: zero commits ahead and zero behind.

Current Foreman State revision `0069` assigned exactly:

`B10-HUMAN-READABLE-PRODUCT-ACTION-BINDINGS-R30`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a005-interaction-input`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`;
- `PRODUCT_DEFAULT_BINDING_SELECTED=NO`;
- Application product-action routing deferred;
- management fetch/persistence deferred;
- local binding editor UI deferred.

The accepted lower authority consumed by this packet was R29 source
`56092a3a6d02df4a9feb89aec5c095e2542b178f` with immutable Reconstruction
record `6f6cffe32874b615bfdb71c03eef328d7c2a7188`.

The GitHub-connected Worker seat cannot observe an external Pi/local checkout's
untracked files, staging area, or dirtiness. No reset, clean, stash, or overwrite
of an external worktree is claimed.

## Governing authority consumed

This shift recovered and applied, at minimum:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/development/README.md`;
- `docs/development/source-naming-and-symbols.md`;
- `docs/development/source-topology.md`;
- `docs/development/module-lifecycle.md`;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md`;
- Work Log Contract revision `0007`;
- Foreman State revision `0069`;
- current Architecture Overlay;
- current Wire Runtime Decisions;
- Q1-Q12 reconciliation;
- `docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md`;
- `docs/ledge/LEDGE_AUDIT_A005_INTERACTION_INPUT.md`;
- the newest relevant Foreman/R29 immutable records.

Repository authority preserved the settled separation: Configuration owns
human-readable desired binding representation and strict validation; accepted
Input R28 owns gesture resolution; R29 owns live Input publication; later
Application composition owns semantic action routing/effects. R30 therefore
does not select a physical default, install live runtime state, persist a
document, or execute an MPEG/UI/Application effect.

## Objective executed

R30 establishes one small Configuration-owned model/parser/formatter for the
recognized human-readable surface:

```text
[bindings]
mpeg_calibration = <button[+button...]> , <trigger> , <context>
```

The parser produces the already-accepted R28
`pstvnc_product_action_binding_t` value rather than defining a second binding
type or gesture resolver.

Missing `[bindings]` or missing `mpeg_calibration` is an explicit valid
zero-binding model. No compiled product default was introduced.

## Exact source / test / documentation range

The completed pre-log R30 source authority is:

`6f42cf5ada266c46e2c6a6b84a91b823d939d1c8`.

The shift landed the following ordered commits:

1. `144c9055b53f9abdc8c03978761a1cc20e00f395` —
   `config: define human-readable product action binding model`;
2. `3a3410b16c09812aafdf78c748bf39b7ba891bf4` —
   `config: parse and format symbolic product action bindings`;
3. `33311eefcfc3e268f5018775bd0788e3b304d6db` —
   `test(config): prove R30 binding grammar and atomicity`;
4. `7baf6661f719d1fff1ae97777c032ff881d7e5e8` —
   `test(config): enforce R30 product binding ownership`;
5. `d26999464adce960a8ca871afa70edaac2923913` —
   `test(config): enroll R30 binding evidence`;
6. `091e59916638c92e2c8c10476f78aeb3803770a6` —
   `build: compile R30 configuration binding model`;
7. `7198f4e22681c12c12c248205f3a7c6c93c78175` —
   `docs(config): record R30 binding ownership`;
8. `96f694c039ed837ffd8a4fe969c738fe2439dded` —
   exact empty trigger commit
   `tooling(symbols): run deterministic dictionary reconciliation`;
9. `7f04a09cec9dd6ec73d1bef18a8eae5c5234c84d` —
   automation-generated
   `docs(symbols): reconcile current clean definitions`;
10. `6f42cf5ada266c46e2c6a6b84a91b823d939d1c8` —
    `docs(config): explain R30 dictionary ownership`.

From assigning authority to final pre-log source authority, the changed path
set is exactly:

- `src/config/product_action_bindings.c`;
- `src/config/product_action_bindings.h`;
- `tests/unit/config_product_action_bindings_test.c`;
- `tests/unit/config_product_action_bindings_source_test.py`;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `src/config/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `docs/development/source-topology.md`.

No `src/input/input_runtime.*`, ordinary Application, UI/local-controller,
RFB, Transport, media-clock, MPEG, AUDIO, Pi product source, or H1 forensic
source changed.

## Configuration-owned typed model

`src/config/product_action_bindings.h` defines one bounded Config model whose
storage contains only accepted R28 typed binding values plus a count.

Current explicit bounds are:

- recognized binding count maximum: `1`;
- input document maximum: `4096` bytes;
- physical line maximum: `256` bytes.

The parser accepts an explicit byte length, rejects an embedded NUL within that
length, and parses through temporary bounded state before publishing the result.

No production heap allocation, executable config, arbitrary callback framework,
file/network persistence, or runtime installation was added.

## Exact symbolic mapping

The recognized button vocabulary maps one-to-one onto project controller bits:

`select l3 r3 start up right down left l2 r2 l1 r1 triangle circle cross square`.

The recognized trigger vocabulary maps exactly to accepted R28:

- `settle` -> `PSTVNC_PRODUCT_ACTION_TRIGGER_SETTLE`;
- `release` -> `PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE`;
- `hold` -> `PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD`.

The recognized context vocabulary maps exactly to:

- `desktop` -> `PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP`;
- `global` -> `PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL`.

The one recognized action key maps only to
`PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION`.

A completed candidate typed binding is checked through the accepted R28
`pstvnc_product_action_resolver_init()` validation seam. Configuration does
not call the live resolver-observe path and does not become a second gesture
owner.

## Strict recognized grammar and tolerant unknown authority

Within `[bindings]`, the recognized `mpeg_calibration` value must contain
exactly:

1. one or more exact symbolic button tokens separated by `+`;
2. one exact trigger token;
3. one exact context token.

Empty chord tokens, unknown buttons, repeated buttons, malformed plus structure,
zero/out-of-project masks, unknown/empty triggers or contexts, missing fields,
extra fields, malformed recognized assignments, duplicate recognized keys, and
duplicate `[bindings]` sections reject.

Unknown well-formed future sections and unknown keys in `[bindings]` are
ignored/opaque. Blank lines, `#` comments, normalized surrounding whitespace,
and CRLF input are tolerated.

This keeps B10's forward compatibility without silently accepting ambiguity in
authority R30 actually understands.

## Atomic publication

The parser constructs one temporary candidate and copies it into caller-visible
storage only after the complete document succeeds.

Focused tests seed caller output with nonzero bytes and prove malformed
recognized authority leaves the complete output byte-for-byte unchanged.

The formatter similarly renders into a private fixed buffer first and modifies
neither caller output nor the caller's output-length value when capacity or
model validation fails.

## Canonical formatter

The formatter emits canonical lowercase recognized names and one stable button
ordering based on the project controller vocabulary.

For a nonempty model it emits:

```text
[bindings]
mpeg_calibration = <canonical-chord>, <trigger>, <context>
```

For a zero-binding model it emits only the `[bindings]` section.

Focused evidence proves parse -> format -> parse preserves the exact R28 typed
binding, including a deliberately noncanonical input button order.

Formatting is only data production. It does not open or write a config file.

## Scope-creep proof

The dedicated R30 source-boundary test proves:

- no compiled/static `pstvnc_product_action_binding_t` default instance;
- no discarded H1 `750000`, `PAD_START`, `PAD_SELECT`, or
  `START+SELECT` special adapter;
- no `pstvnc_input_runtime_set_product_action_bindings` call;
- no live `pstvnc_product_action_resolver_observe` call;
- no Application, local UI, RFB, Transport, MPEG, media-clock, AUDIO, or Pi
  product-effect invocation;
- no `fopen`, `fwrite`, socket/connect, `/ps2vnc.conf`, urgent-control
  or mailbox mechanism;
- exactly one accepted R28 resolver-initialization validation seam.

START and SELECT remain ordinary symbolic buttons a future explicit user
configuration may choose; no R30 source selects them as a product chord.

## Deterministic evidence disposition

The packet's required deterministic evidence is satisfied as Worker evidence:

1. empty, unrelated, and recognized-section-without-known-key documents return
   a valid zero-binding model — PASS;
2. all sixteen symbolic button names map to their exact project bits and a
   multi-button chord ORs only those bits — PASS;
3. duplicate, empty, unknown, malformed-plus, and zero chord cases reject —
   PASS;
4. all accepted trigger/context spellings map exactly; empty/unknown spellings
   reject — PASS;
5. normalized `l1+r1, settle, desktop` produces exactly one accepted R28
   semantic MPEG-calibration binding — PASS;
6. duplicate recognized section/key and malformed/missing/extra recognized
   value syntax reject with caller output unchanged — PASS;
7. unknown sections and unknown `[bindings]` keys remain forward-compatible
   and do not invent known authority — PASS;
8. comments, blanks, CRLF and surrounding whitespace follow B10 tolerance —
   PASS;
9. embedded NUL, document bound, and line bound reject safely — PASS;
10. canonical format/reparse preserves the exact typed value and buffer shortage
    leaves caller output/length unchanged — PASS;
11. the source-boundary scan proves no default, persistence/network/file effect,
    runtime install, cross-domain product effect, urgent mailbox, or H1 special
    adapter — PASS;
12. focused R28/R29/Config tests and canonical host/project/dictionary/PS2
    compile/link/current-source reproducibility are green — PASS.

## Packet acceptance-criterion dispositions

These are Reconstruction Worker dispositions only; they are not independent
Foreman acceptance.

- `B10-R30-C1 CONFIG_OWNS_HUMAN_READABLE_TYPED_BINDING_REPRESENTATION` —
  MET.
- `B10-R30-C2 BINDINGS_SECTION_AND_MPEG_CALIBRATION_VALUE_GRAMMAR_IS_EXPLICIT` —
  MET.
- `B10-R30-C3 ABSENT_RECOGNIZED_BINDING_PUBLISHES_ZERO_NOT_COMPILED_DEFAULT` —
  MET.
- `B10-R30-C4 SYMBOLIC_CHORD_VALIDATION_IS_STRICT_COMPLETE_AND_PROJECT_OWNED` —
  MET.
- `B10-R30-C5 ACTION_TRIGGER_CONTEXT_MAP_EXACTLY_TO_ACCEPTED_R28_TYPES` —
  MET.
- `B10-R30-C6 DUPLICATE_OR_MALFORMED_RECOGNIZED_AUTHORITY_FAILS_CLOSED` —
  MET.
- `B10-R30-C7 UNKNOWN_FUTURE_SECTIONS_AND_KEYS_REMAIN_FORWARD_COMPATIBLE` —
  MET.
- `B10-R30-C8 PARSE_PUBLICATION_IS_ATOMIC_ON_ALL_RECOGNIZED_FAILURES` —
  MET.
- `B10-R30-C9 CANONICAL_TEXT_ROUND_TRIP_PRESERVES_TYPED_BINDING` —
  MET.
- `B10-R30-C10 IMPLEMENTATION_IS_BOUNDED_NO_HEAP_AND_DATA_ONLY` —
  MET.
- `B10-R30-C11 NO_DEFAULT_RUNTIME_APPLICATION_UI_MANAGEMENT_OR_MEDIA_SCOPE_CREEP` —
  MET.
- `B10-R30-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` —
  MET.

All twelve R30 criteria are therefore `MET` as Worker dispositions.

## Final canonical machine evidence

Final pre-log source authority:

`6f42cf5ada266c46e2c6a6b84a91b823d939d1c8`

Canonical GitHub Actions workflow:

`36187973098`

conclusion:

`SUCCESS`.

Final job results:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected for a non-trigger commit.

Observed focused regression output includes:

- `CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `config_text_test: PASS`;
- `PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `INPUT_TEST=PASS`;
- `CONTROLLER_EVENT_TEST=PASS`;
- `LOCAL_CONTROLLER_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`.

Observed repository/build output includes:

- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The deterministic dictionary reconciliation raised Config's complete generated
symbol count from `162` to `266`, and the aggregate portal is synchronized.

The project check immediately before this new immutable record reported
`WORK_LOG_CHECK=PASS records=222 grandfathered=9 format_compat=2 stamp_compat=1`.
This new record is intentionally the next immutable record and uses the
canonical revision-0001 schema.

## Exact linked PS2 identity and hardware classification

R30 changes linked PS2 bytes relative to the accepted R29 source.

Exact R30 linked identity:

- `ELF_PRISTINE_SHA256=5c78663f84cf6372e9887337095d5b5da2424c2f7e26b68017387ac188d5cca8`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=f6b0a92cb1667a6a5d41ec27d3836d95ddc56450e55562354894637076ceba12`;
- `PT_LOAD_BYTES=519700`.

The two canonical linked builds produced the exact same ELF and PT_LOAD
fingerprint.

Accepted R29 authority recorded:

- `ELF_PRISTINE_SHA256=8b0c020f4e410d7a7cc5f7015cc34845f02d382b16055dc607d635a8bbdc7c51`;
- `PT_LOAD_SHA256=f72a1a65b6a16abf792c803a8cfa3322ded51ccf6a038b6a27e94e3ee6eb43f7`;
- `PT_LOAD_BYTES=516116`.

Therefore:

`SOURCE_COMPLETE=YES_WITHIN_R30`
`HOST_TESTED=PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`PS2_PT_LOAD_CHANGED=YES`
`MACHINE_EVIDENCE=GITHUB_ACTIONS`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO`
`HARDWARE_PENDING=YES`
`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

No R30 hardware qualification was attempted or claimed; hardware qualification
is explicitly outside this packet.

## Findings / blockers / known-defect accounting

No R30 implementation blocker remains inside the authorized packet.

The accepted R28 typed seam was sufficient. Configuration did not need to
duplicate or modify Input's binding type or gesture-resolution mechanism.

No default physical MPEG-calibration binding has been selected. Application
semantic-action routing, management retrieval/persistence, and local binding
editor UI remain deferred exactly as assigned.

The first canonical CI execution after adding the new Config source correctly
reported missing generated source-dictionary/topology coverage while host-unit,
PS2 compile, and PS2 link already passed. The repository-authorized exact
dictionary-reconciliation trigger then generated commit
`7f04a09cec9dd6ec73d1bef18a8eae5c5234c84d`; the final reconciled source head
`6f42cf5ada266c46e2c6a6b84a91b823d939d1c8` passes all canonical gates.
Those intermediate completeness failures are resolved evidence, not an open
product defect.

## State / contract accounting

Consumed:

- Foreman State revision `0069`;
- Reconstruction Contract current authority;
- Work Log Contract revision `0007`;
- current Architecture Overlay;
- current Wire Runtime Decisions;
- Q1-Q12 reconciliation;
- accepted R28 typed product-action authority;
- accepted R29 live Input publication authority;
- A005 and B10/B11 audit authority.

Produced by this Reconstruction packet:

- bounded Config-owned human-readable R28 binding representation;
- strict recognized parser with tolerant unknown authority;
- canonical formatter;
- focused behavioral/source-boundary tests;
- canonical host/build enrollment;
- complete Config dictionary/topology documentation;
- no Foreman State update;
- no Foreman acceptance;
- no independent Validation result;
- no default physical binding;
- no runtime installation or Application product-action route;
- no management persistence/editor behavior;
- no hardware qualification.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R30_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction Worker stops here. The Foreman must independently recover
live repository authority, review R30 source/evidence/this immutable record,
decide packet acceptance, and select any later bounded packet. Reconstruction
must not self-accept R30, select a product default chord, or begin deferred
Application routing.
