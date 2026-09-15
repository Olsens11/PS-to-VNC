# Ledge Semantic Audit

DOCUMENT=LEDGE_SEMANTIC_AUDIT
DOCUMENT_REVISION=0002
RECORDED_AT=2026-09-15T10:08:00-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=INVENTORY_TRUE_AS_KNOWN_AT_RECORDED_TIME

This inventory is incomplete until a later revision explicitly proves dependency/symbol coverage. `UNCLASSIFIED` means work remains; it must never be interpreted as permission to discard behavior.

## Disposition vocabulary

- `KEEP_BEHAVIOR`: required semantics should survive.
- `SIMPLIFY`: semantics survive in a deliberately smaller mechanism.
- `SPLIT`: one exploratory responsibility becomes multiple clean owners/processes.
- `REWRITE`: behavior survives but historical implementation shape is unsuitable.
- `DIAGNOSTIC_ONLY`: preserve only if required by current observability/qualification contract.
- `DISCARD`: evidence supports omission from product behavior.
- `UNCLASSIFIED`: insufficient audit evidence yet.

Pipeline status `RECONSTRUCTION_READY` is stronger than a disposition: required behavior, mechanism, intended owner/process/bridge, known-defect treatment, and validation obligations must all be explicit.

## Dependency-closure evidence discovered this revision

The CP2P all-guns application target is layered rather than described by `experiments/media-harness-h1/Makefile`. `mk/media-harness-h1-cp2p-application-link.mk` inherits CP2O, which inherits the cumulative through-Issue-39 target, which inherits the thread-census H1 base. The closure therefore includes H1 sources, `experiments/audio-transport/common/{transport_queue,transport_protocol}.*`, clean `src/` modules reused by H1, SMS libmpeg sources/headers, the frozen MTU1458/wscale128 ps2ip archive, PS2SDK-generated IRX objects, and calibration sources. This is now the authoritative closure route to continue enumerating; filename-only H1-directory inventory is insufficient.

## Audited coherent process tranche A001 — one physical PSTV transport + logical RFB channel

Evidence: `mk/media-harness-h1-thread-census-diag.mk`, `mk/media-harness-h1-cumulative39-thread-census.mk`, `h1_transport_runtime.h`, `h1_rfb_channel.h`, `h1_rfb_runtime_resources.h`, `h1_rfb_credit_policy.h`, `h1_rfb_transport_live.h`, `h1_rfb_mux_io.h`, `h1_rfb_session_runtime.h`, and `H1_INTEGRATION_INTENT.md` at the forensic commit.

Required behavior:

1. One physical PSTV TCP connection has one sole receiver/`recv()` owner; logical AUDIO, MPEG2 and RFB traffic are dispatched from that owner rather than competing physical RFB/audio sockets.
2. Logical RFB channel 1 has session-CONFIG-sized queue storage, synchronization/activity rendezvous, independent credit accounting, inbound byte enqueue/read, and outbound fragmentation through the shared PSTV send path.
3. RFB protocol parsing remains the clean RFB responsibility. The H1 mux seam adapts the clean RFB I/O boundary; it does not implement RFB protocol semantics.
4. Credit is earned from parser-consumed RFB bytes. Ordered terminal quiesce may separately discard proven residual bytes without falsely counting them as parser consumption or returning post-commit credit.
5. The RFB owner can sleep/wake on producer activity without blind timer polling; the receiver publishes activity after committed logical-channel work.
6. RFB finite-session shutdown is ordered request -> boundary -> commit -> complete. Session/media teardown must not race a receiver still inside the just-counted frame's dispatch/signal path.
7. The RFB session owns synchronized protocol/session operation and authoritative CPU framebuffer state for this harness; application/presentation service is admitted only at complete RFB message boundaries, preserving parser synchronization.
8. Flow policy may hold/select framebuffer requests and suppress visual publication while protocol/framebuffer state continues to advance; policy does not own parsing, transport, or framebuffer storage.

Historical mechanism and disposition:

| Responsibility | Historical mechanism | Clean disposition | Intended clean owner/process/bridge | Known-defect treatment | Validation obligation | Pipeline status |
| --- | --- | --- | --- | --- | --- | --- |
| physical PSTV receive/send ownership and logical dispatch | `h1_transport_runtime.*` plus common transport protocol/queue | `SPLIT` | transport component: physical-session receive/dispatch and send serialization; bridge process sections for RFB/audio/MPEG channel delivery | preserve proven ordering/errors; do not preserve diagnostic counters as product state unless required | host framing/dispatch/sequence/error tests; integration concurrency; PS2 qualification after PT_LOAD change | `RECONSTRUCTION_READY` |
| logical RFB byte queue + consumed-byte accounting + outbound fragmentation | `h1_rfb_channel.*` | `KEEP_BEHAVIOR` | transport-owned logical RFB channel mechanism | residual quiesce bytes remain distinct from consumed bytes | host queue/fragment/credit accounting incl. full/empty/residual cases | `RECONSTRUCTION_READY` |
| RFB queue/event/semaphore allocation | `h1_rfb_runtime_resources.*` | `SIMPLIFY` | transport RFB-channel resource ownership; allocate only when RFB enabled | preserve RFB-OFF no-allocation invariant | lifecycle/resource-failure tests and repeated-session test | `RECONSTRUCTION_READY` |
| credit return decision | `h1_rfb_credit_policy.*` | `KEEP_BEHAVIOR` | transport logical-channel flow-control policy | none identified | exhaustive pure host policy table | `RECONSTRUCTION_READY` |
| live RFB channel transport operations/activity/quiesce markers | `h1_rfb_transport_live*`, snapshot helper | `SPLIT` | transport internal channel mechanism + transport bridge's RFB process section | preserve exact quiesce residual ownership checks | host state tests + integration sequence tests + PS2 | `RECONSTRUCTION_READY` |
| clean RFB I/O adaptation to mux | `h1_rfb_mux_io.*` plus build-time preprocessor renaming of `rfb_io` calls | `REWRITE` | RFB component bridge process `session transport I/O`; explicit transport interface rather than experiment-global bind/legacy socket-handle reinterpretation | no second socket/receiver may appear | clean architecture/include checker; RFB session tests against mux fake; one-physical-stream integration evidence | `RECONSTRUCTION_READY` |
| RFB protocol/session + authoritative CPU framebuffer + safe application service boundary | `h1_rfb_session_runtime.*` around clean `src/rfb` + framebuffer | `SPLIT` | existing clean RFB/session + framebuffer owners; application bridge owns presentation/service sequencing | do not regress complete-message safe-boundary rule | existing RFB host tests plus boundary-yield/service ordering tests | `RECONSTRUCTION_READY` |
| terminal receiver-dispatch fence | counter equality + bounded `DelayThread()` in latest `h1_rfb_session_runtime.c` | `SIMPLIFY` | transport lifecycle/quiescence state exposed explicitly to RFB/application shutdown process | do not silently remove race prevention; implementation form is not sacred | deterministic concurrency/state test where possible; stress/PS2 qualification | `RECONSTRUCTION_READY` |
| E206/E2xx progress witnesses and broad transport stats | volatile diagnostic stages/counters in transport runtime | `DIAGNOSTIC_ONLY` | diagnostics snapshots, only where qualification/debug contract still needs them | never use removable diagnostics as hidden semantic authority in final design | prove production synchronization without relying on witness counters; retain selected observability separately | audit continues |

### A001 bridge/process boundary

The clean transport component should own the physical PSTV socket, receiver, frame vocabulary/sequence validation, send serialization, and logical channel resources. Its single bridge body should group outward coordination by process: `RFB channel delivery`, `audio channel delivery`, `MPEG generation delivery`, and `session shutdown/telemetry`. RFB's bridge section consumes an explicit logical-byte-stream interface and adapts it to the existing RFB session owner. Mux framing must not leak into input/UI/RFB parsing.

### A001 known defect / caution register

No defect is intentionally fixed in this audit. The latest shutdown fence proves a real ordering requirement but also reveals that diagnostic counters were temporarily serving as synchronization evidence. Reconstruction must preserve the ordering invariant while replacing that accidental coupling with explicit ownership/quiescence state. A failure to do so is a behavioral regression, not a simplification.

## Remaining seeded inventory

| Evidence surface | Seed responsibility | Likely clean owner/process | Disposition | Status |
| --- | --- | --- | --- | --- |
| `h1_config*` + CONFIG gates | experimental session profile and activation policy | configuration + transport/media profile | UNCLASSIFIED | next audit |
| `h1_audio_runtime*` | PCM production/playback and AUDSRV lifecycle | audio | UNCLASSIFIED | audit required |
| `h1_media_clock.*` | shared media timing | media timing/value/service | UNCLASSIFIED | audit required |
| `h1_video_runtime*`, SMS libmpeg | MPEG ingest/decode/runtime | MPEG | UNCLASSIFIED | audit required |
| CP2P MPEG worker/session coordinator | generation/start/retire orchestration | application + MPEG/transport bridges | UNCLASSIFIED | audit required |
| MPEG presentation/calibration tree | presentation/composition/calibration behavior | display/media presentation + local calibration UI | UNCLASSIFIED | audit required |
| `h1_interaction_coordinator*` + reused clean input/UI modules | controller/mouse/keyboard/OSK interaction | existing input/UI/application | UNCLASSIFIED | audit required |
| H1 harness/coordinator sources | startup/all-guns orchestration/shutdown/telemetry | application coordinator | UNCLASSIFIED | audit required |

## Completeness gate

Do not declare the whole audit complete until the recursive CP2P build closure is enumerated and every project-defined responsibility in that closure is represented here with evidence, owner/process mapping, disposition, and validation requirement. A001 is reconstruction-ready independently; that status does not authorize unrelated media/product migration.

## Exact next audit pickup

Audit the CONFIG/profile + PCM/audio + shared media-clock process as one coherent tranche. Trace the inherited H1 base target and CP2O/CP2P CONFIG activation wrappers so experimental tuning vocabulary can be separated from production-required media/transport configuration.