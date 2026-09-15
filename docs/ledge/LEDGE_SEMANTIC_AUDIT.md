# Ledge Semantic Audit

DOCUMENT=LEDGE_SEMANTIC_AUDIT
DOCUMENT_REVISION=0003
RECORDED_AT=2026-09-15T14:21:00-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0002
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=INVENTORY_TRUE_AS_KNOWN_AT_RECORDED_TIME

This is the current semantic-audit tranche index. Detailed dispositions are held in the tranche documents named below. Revision 0002's inline A001 detail has been preserved as `LEDGE_AUDIT_A001_TRANSPORT_RFB.md`; no historical classification was discarded.

`UNCLASSIFIED` means work remains and is never permission to discard behavior. `RECONSTRUCTION_READY` means required behavior, historical mechanism, disposition, intended clean owner/process/bridge, known-defect treatment and validation obligations are explicit enough that reconstruction need not rediscover the experiment.

## Dependency-closure authority

The CP2P all-guns target is a recursive build composition rather than the small H1-directory host-test Makefile. The closure route runs through the CP2P application target, inherited CP2O/cumulative through-Issue-39 targets and H1 base, and includes H1 sources, clean `src/` modules reused by H1, common transport protocol/queue sources, SMS libmpeg, frozen PS2IP, PS2SDK/IRX objects and calibration sources. This route is identified; exhaustive project-defined file/symbol enumeration remains an A007 completeness obligation.

## Classified coherent tranches

| Tranche | Behavioral process | Detailed authority | Pipeline status | Key clean ownership | Known-defect/caution accounting |
| --- | --- | --- | --- | --- | --- |
| A001 | one physical PSTV transport + logical RFB + safe-boundary/quiescence | `LEDGE_AUDIT_A001_TRANSPORT_RFB.md` rev 0001, preserving semantic-ledger rev 0002 | `RECONSTRUCTION_READY`; currently consumed by reconstruction | transport owns physical PSTV/framing/dispatch/send/logical queues; RFB owns protocol; application owns cross-domain sequencing | receiver-dispatch fence invariant survives while diagnostic-counter/sleep scaffolding may be rewritten |
| A002 | qualified profile + PCM/AUDSRV lifecycle + shared media clock | `LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` rev 0001 | `RECONSTRUCTION_READY` | config/profile, audio, transport logical AUDIO, shared media timing | 61/61 CONFIG fields classified; laboratory surface does not become product API |
| A003 | MPEG ingest/decode + exact-generation START/retire + safe stop | `LEDGE_AUDIT_A003_MPEG_GENERATION.md` rev 0001 | `RECONSTRUCTION_READY` | MPEG decoder/lifecycle + transport logical MPEG + application generation coordination | false-EOF stop defect must not be reintroduced; IPU/DMAC known-state prep survives without reset toggle |
| A004 | presentation/compositor/calibration + RFB suppression/restoration + scheduler | `LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` rev 0001 | `RECONSTRUCTION_READY` | one presentation/GS owner, calibration committed geometry, application ownership policy | acceptance != presentation ownership; first physical MPEG presentation arms shared epoch; exact retirement precedes RFB restoration |
| A005 | interaction/input composition + OSK/local foreground + calibration handoff | `LEDGE_AUDIT_A005_INTERACTION_INPUT.md` rev 0001 | `RECONSTRUCTION_READY` | existing input/UI/keyboard/RFB owners; application process sections coordinate publication and foreground transitions | preserve OSK unequal-row vertical mapping during migration; input shutdown remains fail-closed; discard CP2M transient gesture and temporary START+SELECT binding only |

## A005 disposition summary

A005 confirms that H1 deliberately reused the real through-Issue-39 input/mouse/keyboard/OSK/UI modules. `h1_interaction_coordinator` and the calibration interaction binding are experiment-local composition scaffolding, not new product domains. Surviving behavior includes main-thread-only RFB interaction writes, successfully-published pointer authority, wheel pulse semantics, balanced keyboard modifier sequencing, OSK modifier semantics, suspend/neutralize/rebase before local foreground, continued physical polling during ordinary local foreground, physical-release quarantine, presentation-before-desktop-mouse-resume, calibration first-refusal once owned, complete-message safe RFB servicing and cooperative fail-closed input shutdown.

No new product component is earned by A005. Cross-directory coordination should be organized by coherent processes (`remote interaction publication`, `enter local foreground`, `leave local foreground`, `calibration foreground handoff`, `input lifecycle/shutdown`) through the participating components' single bridge bodies rather than destination-specific adapter sprawl.

## Remaining seeded inventory

| Evidence surface | Seed responsibility | Likely clean owner/process | Status |
| --- | --- | --- | --- |
| H1 resident/all-guns coordinators, CONFIG activation, END/RESULT and teardown | startup, steady-state, finite shutdown, failure/recovery, repeated-session sequencing | application coordinator + component lifecycle bridges | A006 audit required |
| H1 diagnostics/stats/stage witnesses, recursive build/source/symbol closure | observability and completeness proof | diagnostics + development/qualification surfaces | A007 audit required |

## Completeness gate

Do not declare the whole semantic audit complete until A006 lifecycle/orchestration and A007 residual diagnostics plus recursive dependency/source/symbol closure are complete. Every project-defined responsibility in the all-guns build closure must have evidence, owner/process mapping, disposition and validation requirement.

Readiness is not implementation or qualification. Reconstruction must consume only ready tranches in dependency order, and physical PS2 qualification requires operator evidence tied to the exact reconstructed DUT/PT_LOAD.

## Exact next audit pickup

A006 — top-level all-guns orchestration, finite shutdown and recovery. Trace boot/session-loop ordering, CONFIG admission, transport/RFB/audio/MPEG startup dependencies, steady-state coordination, generation retirement, END/RESULT accounting, input/media shutdown ordering, failure convergence, repeated-session behavior, and resident-loop/test-control scaffolding. Preserve the current no-generic-timeout policy for unexplained silent waits unless contrary proven authority is found.