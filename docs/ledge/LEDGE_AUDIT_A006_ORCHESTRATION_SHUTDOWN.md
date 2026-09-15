# Ledge Semantic Audit A006 — Top-Level Orchestration / Shutdown / Recovery

DOCUMENT=LEDGE_AUDIT_A006_ORCHESTRATION_SHUTDOWN
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T15:00:35-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_AUDIT_STATE_REVISION=0005
BASED_ON_GLOBAL_STATE_REVISION=0006
TEMPORAL_CLASS=AUDIT_DISPOSITION
TEMPORAL_SEMANTICS=TRUE_AS_CLASSIFIED_AT_RECORDED_TIME
PIPELINE_STATUS=RECONSTRUCTION_READY

This tranche classifies top-level resident/session orchestration proven by H1. It does not modify product source or promote historical hardware evidence into reconstructed qualification.

## Evidence inspected

Governance/current authority: `AGENTS.md`, `CONTRIBUTING.md`, `docs/CLEAN_ARCHITECTURE.md`, `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md`, global state revision 0006, audit state revision 0005 and audit log revision 0005. Forensic authority at `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`: `experiments/media-harness-h1/H1_INTEGRATION_INTENT.md`, `h1_main_rfb_visible_interaction_pcm.c`, `h1_rfb_session_runtime.h`, `.github/workflows/h1-cp2p-application-link.yml`, plus the already-classified A001-A005 tranche records that preserve later all-guns transport/media/presentation/interaction lifecycle evidence.

Repository-visible branch authority at audit start was a fast-forward descendant of global revision 0006. The connector cannot observe an external Pi worktree's uncommitted status; no claim is made that such a worktree is clean and no product source was touched.

## Required behavioral contract

1. **Boot prerequisites are resident-process prerequisites.** IOP preparation, network initialization/link establishment and graphics/presentation initialization occur before accepting a session. Failure before ownership is established halts/fails closed rather than entering a partly initialized session loop.
2. **One resident process may serve repeated finite sessions.** Session-scoped transport/RFB/media/input state is freshly initialized per accepted session; resident services that are explicitly load-once remain resident. A failed ordinary session converges through owned teardown and returns to admission only when dormancy/retirement is proven.
3. **Transport admission precedes CONFIG consumption.** The sole PSTV transport establishes the session and publishes a validated immutable CONFIG before application policy starts optional consumers. Missing/invalid/incompatible configuration rejects that session; it is not permission for components to guess defaults independently.
4. **Application policy admits a qualified capability/profile.** H1's checkpoint-specific profile predicates are experimental admission scaffolding; production consumes the narrow qualified profile semantics already classified in A002 rather than checkpoint names or the entire laboratory CONFIG surface.
5. **Startup follows dependency ownership.** Shared transport/config exist first; component-owned resources are acquired before their workers/consumers become active; media-clock publication follows the A003/A004 first-real-presentation contract rather than CP2O's earlier pre-MPEG `arm_now` placement; optional channels attach to the one transport owner.
6. **Steady state is coordinated through owned process boundaries.** RFB parser/safe-boundary service, input publication, presentation ownership, audio consumption and MPEG generation lifecycle remain separate owners. Top-level orchestration sequences them; it does not absorb their internal state machines.
7. **Finite producer retirement is explicit.** RFB quiescence, MPEG exact-generation END/retire and PCM MEDIA_END/empty completion are component/transport lifecycle facts. The application waits only on explicit owned completion/error facts needed to release the next dependency; diagnostic counters are not lifecycle authority.
8. **Failure converges monotonically toward shutdown.** Once a session-critical owner reports failure, application stops admitting new work for that session, asks active owners to quiesce/stop in dependency-safe order, preserves the first/most-specific failure evidence, and never restarts a failed subcomponent inside the same generation/session as an implicit recovery trick.
9. **Teardown is reverse-ownership, not a pile of delays.** Producers/interaction workers and media owners must be proven dormant/retired before their storage/resources are reclaimed; transport remains alive long enough for required terminal protocol/result exchange and channel retirement, then sole receiver/physical connection is stopped. A001's receiver-dispatch fence and A005's input dormancy rule are mandatory ordering constraints.
10. **RESULT/END are protocol/reporting facts, not proof by themselves.** H1's RESULT emission and END observation are useful terminal coordination/evidence. Product success is the conjunction of owned component outcomes and clean retirement; a successfully transmitted RESULT cannot erase an earlier local failure.
11. **Repeated-session reuse requires proven clean ownership.** Static H1 storage used to avoid reclaiming an unproven input worker is fail-closed scaffolding. Clean reconstruction may use normal session storage only when every worker/resource owner proves dormancy; otherwise resident admission halts rather than reusing possibly-live state.
12. **No generic timeout masks an unexplained silent wait.** Polling delays used by H1 to yield while waiting for explicit state may be replaced by events/condition ownership. Do not invent a generic timeout that converts unknown lack of progress into successful retirement/recovery. Any bounded timeout must belong to a separately justified protocol/platform requirement and fail closed.

## Historical mechanism, disposition, owner and validation

| Responsibility | H1/current mechanism | Disposition | Intended clean owner/process/bridge | Known-defect treatment | Validation obligation | Status |
| --- | --- | --- | --- | --- | --- | --- |
| resident boot prerequisites | `main()` performs IOP/network/link/graphics init once | `KEEP_BEHAVIOR` | application startup process via platform/network/presentation bridges | partial boot must not enter session admission | injected boot-failure ordering; PS2 cold/warm boot | `RECONSTRUCTION_READY` |
| session admission/retry | infinite resident loop + transport connect retry/yield | `SIMPLIFY` | application session-admission process + transport lifecycle | no second physical owner; no success-by-timeout | disconnect/reconnect, refused/late Pi, repeated admission | `RECONSTRUCTION_READY` |
| CONFIG/profile admission | transport CONFIG accessor + checkpoint predicate | `SPLIT` | transport validates/publishes; config/profile owner interprets; application admits | reject missing/incompatible config without guessed component defaults | malformed/missing/profile mismatch tests | `RECONSTRUCTION_READY` |
| component startup dependencies | checkpoint main starts clock/audio then RFB; later tranches add MPEG/presentation ownership | `REORDER_TO_CLASSIFIED_CONTRACT` | application startup process using component bridges | CP2O `arm_now` is pre-MPEG historical behavior and must not override A003/A004 first-presentation arm | startup-order fault injection; clock-arm assertion | `RECONSTRUCTION_READY` |
| steady-state cross-domain coordination | H1 main + callbacks/coordinators | `SIMPLIFY` | application coherent process sections; component internals remain owned | do not create monolithic H1 coordinator in product | integration/state-machine tests with individual owner failures | `RECONSTRUCTION_READY` |
| RFB finite quiescence | RFB boundary/commit/complete plus transport END wait | `KEEP_BEHAVIOR` with A001 rewrite | RFB + transport owners, application shutdown sequencing | preserve receiver-dispatch ownership fence; counters/sleeps not authority | deterministic quiescence interleavings + repeated-session PS2 | `RECONSTRUCTION_READY` |
| PCM/MPEG finite retirement | audio completion/shutdown; A003 exact-generation MPEG stop/retire | `KEEP_BEHAVIOR` | audio/video owners + transport logical-channel lifecycle | false-EOF MPEG stop must not return; empty-queue completion remains explicit | producer END/error/empty/restart cases | `RECONSTRUCTION_READY` |
| terminal RESULT/accounting | H1 sends RESULT after owned work, then tears down | `KEEP_PROTOCOL_SEMANTIC` / `SIMPLIFY_REPORTING` | application result aggregation; transport sends control message | result-send success cannot overwrite local failure; diagnostics not synchronization | result-send failure, prior-owner failure, END/result ordering | `RECONSTRUCTION_READY` |
| reverse-order teardown | interaction shutdown, media shutdown, RFB shutdown, transport shutdown; later all-guns adds generation retirement | `KEEP_BEHAVIOR` / `REORDER_TO_DEPENDENCIES` | application shutdown process through one bridge per owner | never reclaim live input/receiver/decoder resources | exhaustive startup-failure matrix and teardown-order assertions | `RECONSTRUCTION_READY` |
| resident repeated sessions | `for (;;)` plus between-session delay and completed counter | `KEEP_CAPABILITY`, `DISCARD_DELAY_AS_CORRECTNESS` | application resident session loop | unproven dormancy halts admission; no stale generation/config/pointer/credit state | >=2-session host integration and PS2 repeated-session qualification | `RECONSTRUCTION_READY` |
| checkpoint banners/counters/exit codes | `H1_*` printf/status words and checkpoint-specific return values | `DIAGNOSTIC_ONLY` | diagnostics/development qualification | cannot drive ownership decisions | behavior with diagnostics disabled | A007 closure |

## Clean ownership / process synthesis

A006 earns no new product domain. The application coordinator owns the cross-domain lifecycle processes: `resident startup`, `session admission`, `session startup`, `steady-state coordination`, `failure convergence`, `terminal result`, and `session teardown/re-admission`. Each process uses the single bridge body of transport, RFB, input/UI, audio, video and presentation as applicable. Component lifecycle internals remain inside those components.

The clean session model is a dependency graph, not a literal transcription of CP2O statement order. A001-A005 later evidence supersedes checkpoint-local sequencing where explicitly classified: especially the shared-media-clock arm boundary, exact-generation retirement, presentation ownership restoration and input dormancy proof.

## Simplification rationale

The resident H1 main accumulated checkpoint banners, profile predicates, polling sleeps, counters and deliberately static storage because it was both experiment runner and finite-session product surrogate. Those mechanisms are useful evidence but are not a reason to ship a checkpoint runner. Reconstruction should retain repeated finite sessions and fail-closed ownership ordering while expressing waits as explicit lifecycle state/events and keeping diagnostics observational.

## Known defects / cautions

- A001 receiver-dispatch quiescence remains a hard teardown invariant; never regress to `END received` alone as proof that receiver dispatch cannot touch session resources.
- A003 false-EOF MPEG stop remains forbidden.
- A005 unproven input-worker dormancy remains fatal to safe reuse; do not force-delete/reuse its storage.
- CP2O's immediate media-clock arm is historical pre-MPEG behavior, not current all-guns authority. A003/A004 first physical MPEG presentation is the classified all-guns arm boundary.
- Historical H1 repeated-session evidence is checkpoint/DUT-specific and does not qualify reconstructed code.

## Reconstruction-ready acceptance

A006 is `RECONSTRUCTION_READY`: required behavior, current mechanisms, dispositions, clean owners/processes/bridges, known-defect treatment and validation obligations are explicit. Reconstruction may consume it only when dependency order permits.

## Exact next audit pickup

A007 — residual diagnostics plus exhaustive dependency/source/symbol completeness closure. Enumerate the recursive CP2P/all-guns project-defined source/object/symbol closure, classify remaining H1 diagnostics/stats/stage witnesses and qualification-only surfaces, reconcile every residual responsibility to A001-A006 or a diagnostic/development disposition, and do not declare semantic-audit completion until no project-defined all-guns responsibility remains unclassified.