DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T18:17:35-04:00
COMPLETED_AT=2026-09-25T18:27:50-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a005-interaction-input
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=410846c0826b988567c1f98987e980bd097a2e2b
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R32 resident desired product-binding snapshot

## Recovery provenance

This Reconstruction shift independently recovered live
`ledge/h1-all-guns` authority before making behavior-bearing writes.

Pickup authority was exactly:

`410846c0826b988567c1f98987e980bd097a2e2b`

Current Foreman State revision `0071` activated exactly:

`B10-B11-APPLICATION-PRODUCT-BINDING-SNAPSHOT-R32`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a005-interaction-input`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`;
- R30 parse model accepted but live Input installation deferred;
- R31 read-only retrieval accepted;
- Application PRODUCT_ACTION routing deferred;
- no default physical binding selected;
- POST/persistence deferred.

The accepted lower authority consumed by R32 includes:

- R29 live Input resolver/publication with ordinary zero bindings;
- R30 source `6f42cf5ada266c46e2c6a6b84a91b823d939d1c8`;
- R31 source `fb6eb12868a3045c098651bb97c2577a696bc686`;
- R31 immutable Reconstruction record
  `d80ab11662ef5b53b8ac792331ee69015deb37eb`.

The newest Foreman handoff consumed was:

`docs/ledge/work-log/20260925T181017-0400__foreman__a005-interaction-input__interactive.md`.

The GitHub-connected Worker seat cannot observe an external Pi/local checkout's
untracked files, staging area or dirtiness. No local reset, clean, stash or
overwrite is claimed.

## Governing authority consumed

This shift read and followed, at minimum:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- B10/B11 configuration/recovery/management audit authority;
- A005 interaction/input authority;
- A006 Application orchestration/shutdown authority;
- current source-topology and module-lifecycle policy;
- Foreman State revision `0071`;
- accepted R29-R31 source and test seams;
- ordinary `src/app.c/.h` lifecycle authority;
- R30 `product_action_bindings` parser contract;
- R31 `management/config_get` retrieval contract.

The architecture assigns startup sequencing and fallback policy to Application,
while Management owns retrieval mechanics and Configuration owns parsing.
R32 therefore composes those accepted owners without redistributing their
mechanisms.

## Objective executed

R32 reconstructs exactly one Application-owned resident desired-binding
snapshot.

At ordinary resident startup:

1. PS2 system and private network initialize;
2. Ethernet link readiness is proven;
3. Application starts from explicit zero binding authority;
4. R31 retrieves at most one raw human-config document;
5. successful raw bytes and the exact returned length are passed unchanged to
   R30;
6. successful R30 parsing publishes the exact typed desired model;
7. retrieval failure publishes explicit zero desired authority with a
   FETCH_FAILED status;
8. parse failure publishes explicit zero desired authority with an
   INVALID_CONFIG status;
9. the resulting snapshot remains resident across R16B provider replacement;
10. only then does Application begin its first PSTV connection attempt.

R32 deliberately stops before live Input installation or semantic action
routing.

## Exact R32 pre-log source authority

Final pre-log source authority:

`e18e0170d009093514768d9ea5e4c58b344582e5`.

From assigning authority
`410846c0826b988567c1f98987e980bd097a2e2b`
to final pre-log authority, the branch is sixteen commits ahead / zero behind:

1. `58037d7aacc10c9417e49638182776c4f43fed45` —
   `app: define resident desired binding snapshot`;
2. `697cc876e36b5e81d7591d64ba1351c7a24f20d8` —
   `app: compose config fetch and parse fallback`;
3. `fba6a0422834f635955230a145275c05b784b10b` —
   `app: acquire desired bindings once per resident run`;
4. `b18fd4f7608fac6a36233113fe74a7839674cd93` —
   `test(app): prove R32 binding snapshot composition`;
5. `2b9df32578de75628779ce40d5d3b276c4a44f32` —
   `test(app): enforce R32 binding snapshot boundaries`;
6. `1487ae1017001917b5c21dbb8e476ef062334073` —
   `test(app): prove R32 resident acquisition timing`;
7. `4a5c7eb388ab67d745464fb0f217f7ce2d086a23` —
   `test(app): enroll R32 snapshot evidence`;
8. `d393a8e5e24447b914bf0c7f67ed2376998eb36b` —
   `build: link R32 Application binding snapshot`;
9. `f00b0b6534657c5eff016ba4b6f8e70ef786ac7d` —
   `build: PS2-compile R32 Application snapshot`;
10. `01ce4bd2316e8aca4dbd6612f32da396c3ad55f2` —
    `docs(app): record R32 resident binding snapshot topology`;
11. `af6a712aa73191e150ecfa3ddfa2a895925d09c4` —
    `tooling(topology): enroll R32 Application helper`;
12. `8252cb6ab3917c16879833b3fa9ae3720de07570` —
    `docs(app): explain R32 snapshot ownership`;
13. `10dd60691ceb09bad848ee1e2f878ffb03ff4de3` —
    `test(app): isolate R32 acquisition failure fixture`;
14. `ccfd0ad1cfb1aa2e057df732b8384265ec0aa510` —
    deterministic dictionary-reconciliation trigger;
15. `98b266a0064240987b95a4d1e8f956a4deea8184` —
    automation-generated `docs(symbols): reconcile current clean definitions`;
16. `e18e0170d009093514768d9ea5e4c58b344582e5` —
    same-tree `test(app): verify reconciled R32 source authority` evidence
    trigger.

The final changed-path set is confined to:

- new `src/app_product_bindings.c/.h`;
- ordinary `src/app.c`;
- Application root dictionary;
- focused R32 tests and existing Application fixture;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- root source-topology allowlist;
- source-topology documentation;
- generated source-dictionary portal.

No R30 Configuration implementation, R31 Management implementation, Input
runtime/resolver, local UI/controller, RFB, Transport, MPEG, media-clock, AUDIO,
Platform, Pi product/persistence or H1/B4A forensic source changed.

## Application-owned typed snapshot

R32 introduces:

`pstvnc_app_product_bindings_snapshot_t`

containing:

- exact R30 desired typed bindings;
- one acquisition-status enum.

The status distinguishes:

- `PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID`;
- `PSTVNC_APP_PRODUCT_BINDINGS_STATUS_FETCH_FAILED_ZERO`;
- `PSTVNC_APP_PRODUCT_BINDINGS_STATUS_INVALID_CONFIG_ZERO`.

The snapshot is desired authority only. It is not live Input state.

The helper public operation is:

`pstvnc_app_product_bindings_acquire(snapshot)`.

It returns failure only for an invalid caller output pointer. Ordinary R31
retrieval failure and R30 parse rejection are represented as successful
Application composition with explicit zero desired authority.

## Zero-first and exact-byte composition

Before Management is called, the complete candidate snapshot is zeroed.

This is deliberate product policy: no physical chord is guessed and no prior
binding can survive a new acquisition failure.

Raw storage is:

`unsigned char document[PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES]`

with the compile-time requirement that the R31 maximum equals the R30 maximum
document size.

The storage is automatic/temporary stack state, not resident/static authority.
No heap allocation was introduced.

On successful retrieval, Application calls R30 using exactly:

- the raw returned byte buffer;
- the exact `document_length` reported by Management.

No NUL terminator, whitespace transform, string-length inference or
Application-side parsing is inserted between R31 and R30.

## Nonfatal fallback semantics

Management failure:

- does not call the R30 parser;
- publishes exact zero desired authority;
- records FETCH_FAILED_ZERO;
- continues ordinary Application startup.

Parse failure:

- relies on R30's atomic parser;
- leaves the pre-zeroed desired candidate intact;
- publishes exact zero desired authority;
- records INVALID_CONFIG_ZERO;
- continues ordinary startup.

An empty successful document is valid R30 input and therefore publishes
VALID status with zero bindings.

A valid explicit user binding publishes the exact typed R30 value. Focused
evidence uses deliberately non-default examples rather than a historical
hard-coded chord.

## Resident lifecycle placement

Ordinary `app.c` performs exactly one call to the R32 acquisition helper:

- after `pstvnc_ps2_network_wait_link()` succeeds;
- before the first `pstvnc_ps2_network_connect_pstv()`;
- outside the resident provider-replacement `for (;;)` loop.

The snapshot object is declared in the resident
`pstvnc_app_run_with_session_profiles()` scope and is not rewritten in the
provider-replacement loop.

Therefore an R16B provider failure may create fresh PSTV/Transport/RFB/Input/
media-clock attempt state without refetching or replacing the desired binding
snapshot.

## Explicit non-goals preserved

R32 adds no call to any Input binding-install seam.

Ordinary Application still has no
`PSTVNC_INPUT_EVENT_PRODUCT_ACTION` handling.

R32 does not invoke:

- P9 MPEG calibration;
- P10 activation;
- R21-R24 MPEG run startup;
- UI product-action behavior;
- RFB/Transport product effects;
- media-clock or AUDIO effects;
- POST/persistence;
- editor/reload behavior;
- retry/watchdog recovery policy;
- urgent mailbox behavior.

A fetched nonzero binding therefore remains desired state only and cannot yet
make PRODUCT_ACTION reachable in ordinary product.

## Focused deterministic evidence

The new `app_product_bindings_test` uses the real R30 parser plus a
deterministic R31 stub.

It proves:

- null output pointer performs no fetch;
- R31 retrieval failure publishes exact zero + FETCH_FAILED_ZERO;
- invalid config including embedded NUL publishes exact zero +
  INVALID_CONFIG_ZERO;
- explicit `l1+circle, hold, global` is retained exactly;
- explicit `square+r2, release, desktop` is retained exactly;
- poisoned bytes after the returned body prove the helper passes exact returned
  length rather than full capacity;
- empty successful body produces VALID zero bindings;
- a later failed acquisition cannot inherit an earlier valid binding.

The ordinary Application fixture proves:

- exactly one acquisition after link readiness;
- acquisition precedes first PSTV connect;
- two provider attempts still cause only one acquisition;
- an impossible helper-contract failure stops before PSTV admission without
  fabricating a desired snapshot.

The source-boundary test proves:

- one acquisition call in ordinary Application;
- placement outside the provider loop;
- zeroing before Management;
- exact returned length passed to R30;
- fixed non-static raw storage;
- no heap;
- no Input install;
- no PRODUCT_ACTION handling;
- no Application MPEG activation/calibration/run entry;
- no POST, watchdog or mailbox scope.

## Intermediate evidence findings

Two intermediate issues were observed and preserved rather than hidden.

First, the initial R32 Application test fixture injected a false return from its
acquisition stub and failed to restore that stub for later legacy scenarios.
That test-only state leak caused a cascade of downstream Application assertions.
Commit `10dd60691...` restores the fixture default at the end of that one
negative test. No product source changed for this repair.

Second, corrected-source workflow `36196462790` later encountered an
unrelated intermittent `transport_mpeg_test` assertion at:

`runtime.mpeg_retire_completion_pending == 1`.

R32 changes no Transport/MPEG source. The exact reconciled final-source
workflow `36196725652` reran the complete suite and reports
`transport_mpeg_test: PASS`. Only the exact final-source workflow is used as
R32 acceptance evidence.

The dictionary reconciliation trigger also necessarily ran project/dictionary
jobs before its generated dictionary commit existed. Those pre-generation
failures are superseded by the final reconciled-source workflow.

## R32 acceptance-criterion dispositions

These are Reconstruction Worker dispositions only; they are not Foreman
acceptance.

- `B10-B11-R32-C1 APPLICATION_OWNS_FETCH_PARSE_FALLBACK_COMPOSITION_ONLY` —
  MET.
- `B10-B11-R32-C2 ACQUISITION_OCCURS_ONCE_AFTER_LINK_BEFORE_FIRST_PSTV_ATTEMPT`
  — MET.
- `B10-B11-R32-C3 FALLBACK_AUTHORITY_STARTS_AS_EXPLICIT_ZERO_BINDING` — MET.
- `B10-B11-R32-C4 SUCCESSFUL_GET_BODY_IS_PASSED_BYTE_EXACT_TO_R30` — MET.
- `B10-B11-R32-C5 MANAGEMENT_FAILURE_CONTINUES_WITH_ZERO_BINDING` — MET.
- `B10-B11-R32-C6 PARSE_FAILURE_CONTINUES_WITH_ZERO_BINDING_ATOMICALLY` — MET.
- `B10-B11-R32-C7 VALID_TYPED_BINDING_IS_RETAINED_EXACTLY_WITHOUT_DEFAULT_POLICY`
  — MET.
- `B10-B11-R32-C8 RAW_DOCUMENT_STORAGE_IS_BOUNDED_TEMPORARY_AND_NO_HEAP` — MET.
- `B10-B11-R32-C9 RFB_PROVIDER_REPLACEMENT_DOES_NOT_REFETCH_OR_REWRITE_SNAPSHOT`
  — MET.
- `B10-B11-R32-C10 ORDINARY_INPUT_RUNTIME_REMAINS_ZERO_BOUND` — MET.
- `B10-B11-R32-C11 NO_PRODUCT_ACTION_UI_MEDIA_PERSISTENCE_OR_MAILBOX_SCOPE_CREEP`
  — MET.
- `B10-B11-R32-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

All twelve R32 criteria are therefore `MET` as Worker dispositions.

## Final canonical machine evidence

Final pre-log source authority:

`e18e0170d009093514768d9ea5e4c58b344582e5`

Canonical workflow:

`Ledge reconstruction checks`
run `36196725652`
attempt `1`
conclusion `success`.

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected for the final non-trigger
  verification commit.

Observed focused/cross-domain output includes:

- `APP_PRODUCT_BINDINGS_SOURCE_TEST=PASS`;
- `APP_PRODUCT_BINDINGS_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_SOURCE_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_TEST=PASS`;
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
- `app R15/R16B/R19/R27/R32 tests: PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`.

Observed repository/build output includes:

- exact root clean-file topology including
  `src/app_product_bindings.c/.h`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `WORK_LOG_CHECK=PASS records=226 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled `src/app.c`,
  `src/app_product_bindings.c` and `src/management/config_get.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Generated dictionary authority now records:

- `src:768` complete symbols;
- `SOURCE_DICTIONARY_TOTAL=6657`;
- synchronized aggregate portal.

## Exact linked PS2 identity / hardware classification

R32 changes linked PS2 bytes relative to accepted R31.

Exact R32 identity:

- `ELF_PRISTINE_SHA256=cf7db4dfd05c8aa0ae8a2b2df94bd279ec36addfbc8e9569e486532a06ca94ff`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=c9565c3c55cd758624967ee9cb4b5824ad73bc6f69543d5adb0f1c0b5be2429c`;
- `PT_LOAD_BYTES=520852`.

Both canonical linked builds produced the same ELF and PT_LOAD fingerprints.

Accepted R31 authority was:

- `ELF_PRISTINE_SHA256=c834f488d568b4a67e9b4b0eb7622ba2ddf5d820dd95203b907350f3576fae20`;
- `PT_LOAD_SHA256=03e6511045ed8f46ee82e91f8de7d264275b54bf840371a2dfd190367352db3e`;
- `PT_LOAD_BYTES=520724`.

Therefore:

`R32_SOURCE_COMPLETE=YES`
`R32_HOST_TESTED=PASS`
`R32_PROJECT_CHECK=PASS`
`R32_STRICT_DICTIONARIES=PASS`
`R32_PS2_COMPILE=PASS`
`R32_PS2_LINK=PASS`
`R32_CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`R32_PS2_PT_LOAD_CHANGED=YES`
`R32_MACHINE_EVIDENCE=GITHUB_ACTIONS`
`R32_INDEPENDENT_VALIDATION=NOT_RUN`
`R32_OPERATOR_OBSERVED=NO`
`R32_HARDWARE_QUALIFIED=NO`
`R32_HARDWARE_PENDING=YES`
`R32_LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

No hardware qualification was attempted or claimed.

## State / contract accounting

Consumed:

- Foreman State revision `0071`;
- current Reconstruction Contract;
- Work Log Contract revision `0007`;
- accepted R29 Input semantic-publication seam;
- accepted R30 typed human-readable binding model;
- accepted R31 bounded read-only config retrieval;
- current Application resident/provider-replacement lifecycle.

Produced:

- one Application-owned resident desired-binding snapshot type;
- one Application-owned R31-to-R30 acquisition/fallback helper;
- once-per-resident acquisition after link and before PSTV;
- nonfatal explicit-zero fallback for retrieval and parse failure;
- exact valid typed desired-binding retention;
- focused host/source-boundary evidence;
- Application topology/dictionary/build enrollment;
- no Foreman State update;
- no Foreman acceptance;
- no live Input binding installation;
- no PRODUCT_ACTION route;
- no MPEG activation;
- no persistence/editor/reload;
- no recovery watchdog or urgent mailbox;
- no hardware qualification.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R32_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction Worker stops here. The Foreman must independently recover
live repository authority, review R32 source/evidence/this immutable record,
decide acceptance, and select any later bounded packet. Reconstruction must not
self-accept R32 or begin deferred live binding installation, PRODUCT_ACTION
routing, MPEG activation, persistence/editor/reload, recovery/watchdog or
hardware-qualification work.
