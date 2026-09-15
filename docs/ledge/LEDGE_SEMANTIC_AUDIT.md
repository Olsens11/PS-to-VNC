# Ledge Semantic Audit

DOCUMENT=LEDGE_SEMANTIC_AUDIT
DOCUMENT_REVISION=0005
RECORDED_AT=2026-09-15T17:27:10-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0004
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=INVENTORY_TRUE_AS_KNOWN_AT_RECORDED_TIME
SEMANTIC_AUDIT_COMPLETE=YES

This is the current semantic-audit tranche index. Detailed dispositions are held in the tranche documents named below. Earlier revisions and detailed tranche documents remain historical/classification authority and are not rewritten by this snapshot.

`RECONSTRUCTION_READY` means required behavior, historical mechanism, disposition, intended clean owner/process/bridge, known-defect treatment and validation obligations are explicit enough that reconstruction need not rediscover the experiment. `AUDIT_COMPLETE` means the residual completeness tranche found no unexplained project-defined source/object responsibility after explicit diagnostic/development/qualification classification.

## Dependency-closure authority

The final all-guns PS2 build is a recursive composition:

`CP2P application-link -> CP2O visible RFB/interaction/PCM -> cumulative through-Issue-39 -> base H1 thread-census build`.

A007 exhaustively classified the project-defined definition-bearing translation-unit families reachable through that route, including H1 sources, reused clean `src/` modules, common transport protocol/queue sources, calibration sources, the generated CP2P video translation unit, and Pi/host engineering surfaces. External/frozen SMS libmpeg, PS2IP, PS2SDK/IRX and ordinary libraries/toolchain are explicit dependencies rather than unexplained project ownership.

The four recursive makefile authorities have the same Git blob identities at active-runtime source checkpoint `d85e896093f9f22be5fb9ff8feb464c01b7cd0a9` and forensic source `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, so later shutdown/receive experiments remain inside the same traced object topology.

Exact linked-symbol/build/PT_LOAD identity of the future reconstructed DUT remains a reconstruction/validation gate rather than an H1 semantic-discovery prerequisite.

## Classified coherent tranches

| Tranche | Behavioral process | Detailed authority | Pipeline status | Key clean ownership | Known-defect/caution accounting |
| --- | --- | --- | --- | --- | --- |
| A001 | one physical PSTV transport + logical RFB + safe-boundary/quiescence | `LEDGE_AUDIT_A001_TRANSPORT_RFB.md` rev 0001 | `RECONSTRUCTION_READY`; currently consumed by reconstruction | transport physical PSTV/framing/dispatch/send/logical queues; RFB protocol; application sequencing | receiver-dispatch fence survives; counter/sleep scaffolding may be rewritten |
| A002 | qualified profile + PCM/AUDSRV lifecycle + shared media clock | `LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` rev 0001 | `RECONSTRUCTION_READY` | config/profile, audio, transport AUDIO, shared timing | 61/61 CONFIG fields classified; laboratory surface is not product API |
| A003 | MPEG ingest/decode + exact-generation START/retire + safe stop | `LEDGE_AUDIT_A003_MPEG_GENERATION.md` rev 0001 | `RECONSTRUCTION_READY` | video decoder/lifecycle + transport MPEG + application generation coordination | false-EOF stop defect forbidden; IPU/DMAC known-state acquisition survives |
| A004 | presentation/compositor/calibration + RFB suppression/restoration + scheduler | `LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` rev 0001 | `RECONSTRUCTION_READY` | one presentation/GS owner, calibration geometry, application ownership policy | acceptance != presentation ownership; first physical MPEG presentation arms epoch; exact retirement precedes RFB restoration |
| A005 | interaction/input composition + OSK/local foreground + calibration handoff | `LEDGE_AUDIT_A005_INTERACTION_INPUT.md` rev 0001 | `RECONSTRUCTION_READY` | existing input/UI/keyboard/RFB owners; application coordinates publication/foreground | preserve OSK unequal-row mapping during migration; fail-closed input shutdown; discard temporary test gestures only |
| A006 | resident startup + session admission + orchestration + finite shutdown/recovery | `LEDGE_AUDIT_A006_ORCHESTRATION_SHUTDOWN.md` rev 0001 | `RECONSTRUCTION_READY` | application lifecycle processes through component bridges | preserve A001 dispatch fence, A003 safe stop, A005 dormancy; no generic success-by-timeout; historical CP2O clock arm superseded by A003/A004 |
| A007 | residual diagnostics + recursive source/object/symbol-family completeness closure | `LEDGE_AUDIT_A007_DIAGNOSTIC_COMPLETENESS.md` rev 0001 | `AUDIT_COMPLETE`; no product behavior to reconstruct as an independent tranche | diagnostics/development/qualification surfaces remain observers/tools; product semantics map to A001-A006 | diagnostics never become synchronization authority; exact reconstructed binary qualification remains downstream |

## A007 closure summary

A007 proves definition-owner closure rather than pretending a future DUT binary already exists. Every project-defined symbol reachable from the forensic final H1 object topology is defined by an enumerated translation-unit family, and every such family is assigned to A001-A006 or an explicit A007 non-product disposition. No unexplained owner/process remains.

Residual witnesses are classified as observational: byte/frame/high-water/result counters, diagnostic words, thread census, stage markers/codes/colors/holds, RFB snapshots, checkpoint banners and exit codes, Pi stall/prefill/persistent witnesses, telemetry decoders and exact qualification manifests. Correctness must continue to work with these witnesses absent unless an A001-A006 contract separately owns the underlying state.

Generator/source-transformation machinery, monkeypatch/wrapper integration, checkpoint activation gates, historical mains, unused experiment variants, host tests/checkers/workflows/runbooks and evidence documents are development/qualification scaffolding. Their proven behavioral facts have been transferred into A001-A006; their implementation shapes are not production architecture.

## Completeness conclusion

The seeded H1 semantic audit is complete:

- coherent process tranches classified: 7/7 (A001-A007);
- product-behavior tranches ready for reconstruction: A001-A006;
- CONFIG field IDs classified: 61/61;
- recursive project-defined PS2 definition-bearing source/object families: closed with no unexplained owner/process;
- Pi/host engineering families: explicitly classified;
- residual diagnostics/stats/stage/qualification surfaces: explicitly classified;
- external/frozen dependencies: explicitly separated from project-defined ownership.

This completion does not mark any reconstructed tranche `VALIDATION_READY` and does not transfer historical hardware qualification. Reconstruction must continue in dependency order, and validation must independently prove the reconstructed source, host tests, canonical build, linked-symbol/source-dictionary completeness, reproducibility/PT_LOAD identity and eventual PS2 behavior.

## Reopen rule

There is no next seeded audit tranche. If reconstruction or validation finds a genuinely unexplained H1 responsibility, reopen the semantic audit narrowly against that new evidence rather than guessing or broadening an existing component silently.

Until such evidence appears, the semantic-audit lane has no further H1 discovery backlog and its scheduled worker capacity may be considered for deliberate reassignment after continuity/governance synthesizes this state.