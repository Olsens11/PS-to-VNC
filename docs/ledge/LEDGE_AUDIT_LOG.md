# Ledge Semantic Audit — Append-only Lane Log

DOCUMENT=LEDGE_AUDIT_LOG
DOCUMENT_REVISION=0004
RECORDED_AT=2026-09-15T13:11:00-04:00
SOURCE_COMMIT=SELF
TEMPORAL_CLASS=WORK_EVENT_LOG
TEMPORAL_SEMANTICS=EACH_ENTRY_TRUE_AS_KNOWN_AT_ITS_COMPLETION_TIME

## Audit Entry A001

STARTED_AT=2026-09-15T09:58:11-04:00
COMPLETED_AT=2026-09-15T10:11:00-04:00
STARTING_GLOBAL_STATE_REVISION=0002
ENDING_AUDIT_STATE_REVISION=0001
FORENSIC_SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
TEMPORAL_SEMANTICS=ENTRY_DESCRIBES_AUDIT_KNOWLEDGE_AT_COMPLETION_TIME

### Objective

Advance exact dependency closure and classify one coherent H1 behavioral process without modifying reconstructed product source.

### Evidence inspected

`AGENTS.md`, `CONTRIBUTING.md`, `docs/CLEAN_ARCHITECTURE.md`, ledge contract/state/audit/simplification/log documents, H1 integration intent, H1 base/cumulative/CP2O/CP2P build files, `h1_config.h`, transport runtime header, logical RFB channel/resource/credit/live/mux-I/O/session headers, and the latest forensic RFB shutdown-fence commit.

### Findings and decisions

The all-guns build is a recursive composition rather than a standalone H1-directory Makefile. The one-physical-PSTV-stream invariant is explicit in both build/source authority and H1 integration intent. Logical RFB queue/credit/fragmentation and exact-byte-stream semantics are required behavior. The build-time preprocessor I/O rename/global-bind technique is experiment scaffolding suitable for replacement by an explicit RFB byte-stream boundary. The latest counter+DelayThread shutdown fence protects a real dispatch-ownership invariant, but its diagnostic-counter/sleep implementation should be rewritten as explicit synchronization rather than copied.

A001 was marked reconstruction-ready at responsibility granularity with clean ownership, bridge process, defect treatment, and validation obligations. Diagnostic witness fields/counters remain under audit and are not implicitly promoted into product state.

### Repository changes

- semantic audit revision `0002`;
- simplification register revision `0002`;
- audit lane state revision `0001`;
- this audit lane log revision `0001`.

No product source, H1 forensic source, validation findings, or reconstruction-lane state was modified.

### Validation performed

Repository/API inspection verified the exact source/build declarations and forensic commit identities used by this classification. No executable build or hardware test was claimed in this audit-only shift.

### Exact next pickup

Trace CONFIG/profile + PCM/audio + media clock. Classify the 61-field laboratory CONFIG vocabulary and finite PCM producer/audio lifecycle before marking that process reconstruction-ready.

## Audit Entry A002

STARTED_AT=2026-09-15T10:59:42-04:00
COMPLETED_AT=2026-09-15T11:08:00-04:00
STARTING_GLOBAL_STATE_REVISION=0003
STARTING_AUDIT_STATE_REVISION=0001
ENDING_AUDIT_STATE_REVISION=0002
STARTING_BRANCH_COMMIT=aa9386fbc67974ab6d9a50ebea8d720efa0b15e1
FORENSIC_SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
TEMPORAL_SEMANTICS=ENTRY_DESCRIBES_AUDIT_KNOWLEDGE_AT_COMPLETION_TIME

### Objective

Classify CONFIG/profile + PCM/AUDSRV audio + shared media-clock behavior as one coherent process tranche, including all 61 H1 CONFIG field IDs, without changing reconstructed product source.

### Evidence inspected

Current branch/global/audit authority; `h1_config.{c,h}`; `h1_audio_runtime.{c,h}`; `h1_media_clock.{c,h}`; `H1_INTEGRATION_INTENT.md`; prior semantic audit and simplification register; forensic H1 source commit.

### Findings and decisions

A002 establishes that H1's complete dynamic 61-field CONFIG is laboratory exploration machinery around a smaller set of real product/profile semantics. All 61 IDs are now classified. Production should use narrow owner-specific immutable profile values and selected qualified defaults rather than expose the entire H1 sweep vocabulary as a mandatory per-session API.

PCM playback semantics are reconstruction-ready: transport-owned AUDIO bytes, preserved AUDSRV wait->play ordering, finite MEDIA_END+empty retirement, resident-ELF AUDSRV service lifetime, session-scoped worker/buffer lifecycle, and reservoir-before-epoch startup ordering. The common media epoch/deadline contract is also ready, while its exact video arm callsite remains deferred to MPEG/presentation audit. Publication ordering is essential; H1's volatile/EE_SYNCL representation is not sacred.

`VIDEO_STAGE_MARKERS` and `VIDEO_STAGE_HOLD_VSYNCS` are diagnostic-only. `VIDEO_IPU_RESET_EACH_SESSION` remains deliberately unresolved pending decoder lifecycle audit. Reserved compressed-audio enum values are not current product behavior.

### Repository changes

- created `LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` revision `0001` with detailed evidence, field classification, responsibility dispositions and validation obligations;
- advanced audit lane state to revision `0002`;
- advanced this append-only lane log to revision `0002`.

No reconstructed product source, forensic H1 source, reconstruction state, validation finding, or global state was modified.

### Validation performed

Repository-source inspection verified the CONFIG field vocabulary, decoder/validator structure, PCM consumer ordering/lifecycle, and media-clock publication/deadline mechanism. No executable build, PT_LOAD comparison, or hardware test was performed or claimed.

### Progress

Two coherent tranches are now classified. A002 contributes six reconstruction-ready responsibility rows; 61/61 CONFIG IDs have a production-disposition classification. Five seeded major process families remain, and whole-build recursive file/symbol completeness proof remains incomplete.

### Exact next pickup

Audit MPEG ingest/decode plus CP2P generation/start/retire as A003. Resolve the IPU reset field, MPEG/video tuning fields, finite producer lifecycle, and the exact qualified video presentation boundary that arms the already-ready shared media clock before broadening into calibration/compositor behavior.

## Audit Entry A003

STARTED_AT=2026-09-15T11:58:11-04:00
COMPLETED_AT=2026-09-15T12:13:00-04:00
STARTING_GLOBAL_STATE_REVISION=0004
STARTING_AUDIT_STATE_REVISION=0002
ENDING_AUDIT_STATE_REVISION=0003
STARTING_BRANCH_COMMIT=1c317d429d8a6320c8e24ebf44e926565301d472
FORENSIC_SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
TEMPORAL_SEMANTICS=ENTRY_DESCRIBES_AUDIT_KNOWLEDGE_AT_COMPLETION_TIME

### Objective

Classify MPEG ingest/decode and CP2P exact-generation START/retire lifecycle as one coherent behavioral process without modifying reconstructed product source.

### Evidence inspected

Current branch/global/audit authority; `h1_video_runtime.c`; `h1_config.h`; CP2P START validation, live-generation boundary, all-guns activation, MPEG event-wake hardware result, safe-stop lifecycle, H1 integration intent, A002 clock/profile audit, semantic ledger and simplification register.

### Findings and decisions

A003 marks the transport-to-MPEG event-wake consumer, decoder acquisition/feed/release lifecycle, exact START/prepared generation, exact-generation emission fence, safe decoder stop boundary, retirement/final residual discard/credit return, and narrow first-presentation media-clock arm contract reconstruction-ready.

The known false-EOF stop defect is explicitly preserved as a defect to prevent: owner stop must be observed after `MPEG_Picture()` returns, never by making the active libmpeg data callback synthesize EOF. The 600-second event-wake proof and later safe-stop proof remain separate temporal evidence and are not flattened into a repeated-session hardware claim.

Field 44 is resolved: discard `VIDEO_IPU_RESET_EACH_SESSION` as a product toggle, but preserve the known-state IPU/DMAC preparation it guarded as decoder-acquisition lifecycle. MPEG timer-backed empty-queue polling is likewise not promoted to production; the qualified receiver-driven event wake is the semantic direction.

The common media epoch is now tied to the first real video presentation boundary. Presentation/calibration owns the concrete compositor callsite in A004; CONFIG, START, prefill and decode start do not arm the clock.

### Repository changes

- created `LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision `0001` with detailed required behavior, field dispositions, ownership, defect treatment and validation obligations;
- advanced audit lane state to revision `0003`;
- advanced this append-only lane log to revision `0003`.

No reconstructed product source, forensic H1 source, reconstruction state, validation finding, or global state was modified.

### Validation performed

Repository-source and durable hardware-result inspection verified the source-level decoder/feed/presentation boundary, exact-generation ordered-fence contract, event-wake qualification boundary and safe-stop lifecycle evidence. No reconstructed build, PT_LOAD comparison, or new hardware test was performed or claimed.

### Progress

Three coherent tranches are now classified. A003 contributes seven reconstruction-ready responsibility groups plus one narrow ready media-clock arm contract. Four seeded major process families remain; whole-build recursive file/symbol completeness proof remains incomplete.

### Unresolved questions

Final scheduler/drop policy, draw/capture/suppression geometry ownership, calibration acceptance, RFB matte/suppression alignment, presentation ownership transitions, cursor/OSK layering, interaction and top-level shutdown remain unaudited.

### Exact next pickup

Audit presentation/compositor/calibration as A004. Trace calibration acceptance, draw/capture/suppression geometry, RFB suppression/matte alignment, `RFB_ONLY`/MPEG ownership transitions, cursor/OSK/local-UI layering and scheduler/drop policy. Preserve the A003 first-presentation epoch-arm contract and do not broaden into general interaction except where calibration UI input is necessary.

## Audit Entry A004

STARTED_AT=2026-09-15T13:01:52-04:00
COMPLETED_AT=2026-09-15T13:11:00-04:00
STARTING_GLOBAL_STATE_REVISION=0005
STARTING_AUDIT_STATE_REVISION=0003
ENDING_AUDIT_STATE_REVISION=0004
FORENSIC_SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
TEMPORAL_SEMANTICS=ENTRY_DESCRIBES_AUDIT_KNOWLEDGE_AT_COMPLETION_TIME

### Objective

Classify presentation/compositor/calibration as one coherent behavioral process, including the concrete A003 first-presentation clock-arm boundary and final disposition of the deferred presentation geometry/scheduler/drop fields, without modifying reconstructed product source.

### Evidence inspected

Current branch/global/audit authority; `AGENTS.md`; `CONTRIBUTING.md`; `docs/CLEAN_ARCHITECTURE.md`; ledge reconstruction contract; `h1_video_runtime.c`; the durable `CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md`; A003 audit; and the simplification register. Historical entries were interpreted with point-in-time semantics and later qualified/current-path entries were not projected backward into earlier checkpoints.

### Findings and decisions

A004 marks calibration foreground/acceptance, base/inner/outer geometry authority, RFB freeze/request scheduling, accept-to-first-frame protection, first physical MPEG ownership/common-clock arm, one shared compositor/GS owner, exact-generation RFB suppression, ordered restoration, and qualified absolute scheduler/drop policy reconstruction-ready.

The geometry authorities are explicit: base is capture plus MPEG presentation; inner matte is PS2-local presentation only; outer matte determines the RFB suppression footprint without changing capture. Accepted generation geometry replaces independent live draw/encode CONFIG authorities.

Acceptance is not presentation ownership. MPEG ownership and the shared media epoch begin only at the first valid physically presented MPEG frame. RFB remains protected in the gap. Exact Pi retirement must prove generation suppression/capture is gone before local retirement allows the one fresh FULL RFB restoration request.

The recovered cumulative39 compositor is evidence for one graphics owner, not two modules. Its source-inclusion/private-renaming technique is reconstruction scaffolding. The clean presentation component should directly own the combined remote-desktop/matte/MPEG/local-overlay composition through one physical synchronization/flip path.

Scheduler comparison modes, stage holds and dynamic draw/encode knobs are experimental surface. The qualified absolute common-clock scheduler, presentation offset, narrow lateness/drop semantics and exact generation geometry survive as profile/owned state.

### Repository changes

- created `LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision `0001`;
- advanced simplification register to revision `0003` with S005-S008;
- advanced audit lane state to revision `0004`;
- advanced this append-only lane log to revision `0004`.

No reconstructed product source, forensic H1 source, reconstruction state, validation finding, or global state was modified.

### Validation performed

Repository/source/history inspection established the ownership and process contracts above. No executable build, PT_LOAD comparison, reconstructed host test, or new hardware test was performed or claimed. The repository connector cannot inspect an external Pi worktree's uncommitted status, so no external-worktree-clean claim is made.

### Progress

Four coherent tranches are now classified. A004 contributes nine reconstruction-ready responsibility groups plus one diagnostic-only group. All 61 CONFIG fields remain classified, including the presentation-deferred fields resolved here. Three seeded major process families remain, and exhaustive recursive file/symbol completeness proof remains incomplete.

### Unresolved questions

General interaction composition, top-level all-guns orchestration/shutdown/recovery, residual diagnostics, and exhaustive recursive dependency/source/symbol closure remain unaudited. Historical H1 hardware evidence remains forensic evidence only and does not qualify reconstructed presentation code.

### Exact next pickup

Audit A005 interaction/input composition: physical controller acquisition versus semantic ownership, pointer/button/scroll routing, OSK/local-UI foreground transitions, keyboard tap/modifier serialization, calibration entry/exit handoff, release quarantine, and RFB safe-boundary interaction servicing. Do not reopen A004 geometry/presentation policy except where an interaction transition consumes its already-classified foreground/ownership contract.