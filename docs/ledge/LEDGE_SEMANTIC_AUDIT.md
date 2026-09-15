# Ledge Semantic Audit

DOCUMENT=LEDGE_SEMANTIC_AUDIT
DOCUMENT_REVISION=0004
RECORDED_AT=2026-09-15T15:00:35-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0003
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=INVENTORY_TRUE_AS_KNOWN_AT_RECORDED_TIME

This is the current semantic-audit tranche index. Detailed dispositions are held in the tranche documents named below. Earlier revisions and detailed tranche documents remain historical/classification authority and are not rewritten by this snapshot.

`UNCLASSIFIED` means work remains and is never permission to discard behavior. `RECONSTRUCTION_READY` means required behavior, historical mechanism, disposition, intended clean owner/process/bridge, known-defect treatment and validation obligations are explicit enough that reconstruction need not rediscover the experiment.

## Dependency-closure authority

The CP2P/all-guns target is a recursive build composition rather than the small H1-directory host-test Makefile. The closure route runs through the CP2P application target, inherited CP2O/cumulative through-Issue-39 targets and H1 base, and includes H1 sources, clean `src/` modules reused by H1, common transport protocol/queue sources, SMS libmpeg, frozen PS2IP, PS2SDK/IRX objects and calibration sources. This route is identified; exhaustive project-defined file/object/symbol enumeration remains the final A007 completeness obligation.

## Classified coherent tranches

| Tranche | Behavioral process | Detailed authority | Pipeline status | Key clean ownership | Known-defect/caution accounting |
| --- | --- | --- | --- | --- | --- |
| A001 | one physical PSTV transport + logical RFB + safe-boundary/quiescence | `LEDGE_AUDIT_A001_TRANSPORT_RFB.md` rev 0001 | `RECONSTRUCTION_READY`; currently consumed by reconstruction | transport physical PSTV/framing/dispatch/send/logical queues; RFB protocol; application sequencing | receiver-dispatch fence survives; counter/sleep scaffolding may be rewritten |
| A002 | qualified profile + PCM/AUDSRV lifecycle + shared media clock | `LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` rev 0001 | `RECONSTRUCTION_READY` | config/profile, audio, transport AUDIO, shared timing | 61/61 CONFIG fields classified; laboratory surface is not product API |
| A003 | MPEG ingest/decode + exact-generation START/retire + safe stop | `LEDGE_AUDIT_A003_MPEG_GENERATION.md` rev 0001 | `RECONSTRUCTION_READY` | video decoder/lifecycle + transport MPEG + application generation coordination | false-EOF stop defect forbidden; IPU/DMAC known-state acquisition survives |
| A004 | presentation/compositor/calibration + RFB suppression/restoration + scheduler | `LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` rev 0001 | `RECONSTRUCTION_READY` | one presentation/GS owner, calibration geometry, application ownership policy | acceptance != presentation ownership; first physical MPEG presentation arms epoch; exact retirement precedes RFB restoration |
| A005 | interaction/input composition + OSK/local foreground + calibration handoff | `LEDGE_AUDIT_A005_INTERACTION_INPUT.md` rev 0001 | `RECONSTRUCTION_READY` | existing input/UI/keyboard/RFB owners; application coordinates publication/foreground | preserve OSK unequal-row mapping during migration; fail-closed input shutdown; discard temporary test gestures only |
| A006 | resident startup + session admission + orchestration + finite shutdown/recovery | `LEDGE_AUDIT_A006_ORCHESTRATION_SHUTDOWN.md` rev 0001 | `RECONSTRUCTION_READY` | application lifecycle processes through component bridges | preserve A001 dispatch fence, A003 safe stop, A005 dormancy; no generic success-by-timeout; historical CP2O clock arm superseded by A003/A004 |

## A006 disposition summary

A006 confirms that H1's resident `main()` is experiment/test orchestration around durable product lifecycle semantics, not a new domain. The product application coordinator should own coherent processes for resident startup, session admission, session startup, steady-state coordination, failure convergence, terminal result and teardown/re-admission while each component retains its internal lifecycle.

Durable behavior includes boot prerequisites before admission; transport/config before optional consumers; qualified profile admission; one physical transport; explicit finite RFB/audio/MPEG retirement; reverse-dependency teardown; first-failure preservation; fail-closed ownership proof before reuse; and repeated finite sessions. Polling sleeps, checkpoint banners/counters, checkpoint-specific profile predicates and static storage used only to avoid unsafe reuse are scaffolding/diagnostics rather than correctness mechanisms.

A006 also reconciles temporal evidence: CP2O's immediate media-clock arm was correct for its pre-MPEG checkpoint but is not current all-guns authority. A003/A004's first valid physically presented MPEG frame remains the all-guns common-clock arm boundary.

## Remaining seeded inventory

| Evidence surface | Seed responsibility | Likely clean owner/process | Status |
| --- | --- | --- | --- |
| H1 diagnostics/stats/stage witnesses, recursive build/source/object/symbol closure | observability and completeness proof | diagnostics + development/qualification surfaces | A007 audit required |

## Completeness gate

Do not declare the whole semantic audit complete until A007 residual diagnostics plus recursive dependency/source/object/symbol closure is complete. Every project-defined responsibility in the all-guns build closure must have evidence, owner/process mapping, disposition and validation requirement.

Readiness is not implementation or qualification. Reconstruction must consume only ready tranches in dependency order, and physical PS2 qualification requires operator evidence tied to the exact reconstructed DUT/PT_LOAD.

## Exact next audit pickup

A007 — residual diagnostics and exhaustive completeness closure. Enumerate the recursive all-guns project-defined source/object/symbol closure, classify remaining diagnostics/stats/stage/qualification witnesses, reconcile every residual responsibility to A001-A006 or an explicit diagnostic/development/discard disposition, and stop rather than declare completion if any unexplained project-defined owner/symbol/process remains.