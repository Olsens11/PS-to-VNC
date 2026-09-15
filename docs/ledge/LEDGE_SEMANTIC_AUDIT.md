# Ledge Semantic Audit

DOCUMENT=LEDGE_SEMANTIC_AUDIT
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T09:43:34-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
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

## Seeded H1 responsibility inventory

| Evidence surface | Seed responsibility | Likely clean owner/process | Disposition | Status |
| --- | --- | --- | --- | --- |
| `h1_transport_runtime.*` | sole physical PSTV transport receive/runtime, frame dispatch, errors/stats | transport; receive/dispatch lifecycle | UNCLASSIFIED | audit required |
| `h1_rfb_mux_io.*` | RFB logical-channel I/O over PSTV mux and diagnostics | RFB bridge / transport-facing RFB process | UNCLASSIFIED | audit required |
| `h1_rfb_session_runtime.*` | synchronized RFB session operation and quiesce/shutdown coordination | RFB session; lifecycle/quiesce process | UNCLASSIFIED | latest fence requires semantic audit |
| `h1_pcm_runtime.*` | PCM runtime behavior | PCM/audio component | UNCLASSIFIED | audit required |
| `h1_mpeg_runtime.*` | MPEG runtime behavior | MPEG component | UNCLASSIFIED | audit required |
| `h1_mpeg_presentation.*` | MPEG presentation/composition behavior | MPEG presentation | UNCLASSIFIED | audit required |
| `h1_mpeg_calibration.*` | MPEG calibration UX/state | calibration/UI + MPEG presentation bridge process | UNCLASSIFIED | audit required |
| `h1_controller_runtime.*` | controller sampling/actions used by H1 | input/controller | UNCLASSIFIED | audit required |
| H1 harness/coordinator sources | startup, all-guns orchestration, shutdown ordering, diagnostics | application coordinator | UNCLASSIFIED | exact files/closure required |

## Known cross-process question from starting commit

The RFB owner waits for commit and then for proof that the sole transport receiver has returned to its receive-loop boundary before quiesce completion. The experimental implementation uses counters plus bounded `DelayThread()` polling. Required audit question: preserve the scheduling/ownership invariant while determining whether the counter/polling form is essential or merely H1 scaffolding.

## Completeness gate

Do not declare this audit complete until the H1 build closure is enumerated and every project-defined responsibility in that closure is represented here with evidence, owner/process mapping, disposition, and validation requirement.
