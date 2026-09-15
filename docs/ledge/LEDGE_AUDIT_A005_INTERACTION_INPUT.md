# Ledge Semantic Audit A005 — Interaction / Input Composition

DOCUMENT=LEDGE_AUDIT_A005_INTERACTION_INPUT
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T14:16:00-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_AUDIT_STATE_REVISION=0004
BASED_ON_GLOBAL_STATE_REVISION=0005
TEMPORAL_CLASS=AUDIT_DISPOSITION
TEMPORAL_SEMANTICS=TRUE_AS_CLASSIFIED_AT_RECORDED_TIME
PIPELINE_STATUS=RECONSTRUCTION_READY

This tranche classifies the interaction/input composition proven by H1. It does not redesign controller behavior, modify reconstructed product source, or claim reconstructed hardware qualification.

## Evidence inspected

Primary forensic source at `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`:

- `experiments/media-harness-h1/H1_INTEGRATION_INTENT.md`;
- `experiments/media-harness-h1/h1_main_rfb_visible_interaction_pcm.c`;
- `experiments/media-harness-h1/h1_interaction_coordinator.{c,h}`;
- `experiments/media-harness-h1/h1_rfb_session_runtime.h`;
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_calibration_interaction_binding.h`;
- `src/app.c`;
- `src/input/input_runtime.h`;
- `src/input/keyboard.h`;
- `src/ui/local_controller.h`;
- `src/ui/local_ui.h`;
- `src/ui/osk.h`;
- `.github/workflows/h1-cp2p-application-link.yml`.

The H1 integration intent explicitly says the experiment must exercise the real post-Issue-39 input/mouse/keyboard/OSK composition rather than create an H1 replacement. `h1_interaction_coordinator` identifies itself as experiment-local cross-domain wiring and reuses those clean owners directly.

## Required behavioral contract

1. **Physical observation and semantic ownership stay separate.** The input runtime owns libpad polling, immediate physical continuity/history, mouse interpretation, the ordinary semantic event FIFO, cooperative worker lifecycle, and explicit suspension/handoff synchronization. It does not own RFB serialization, UI meaning, presentation, or product actions.
2. **One application/main publication owner.** Pointer, wheel and keyboard RFB writes occur from application/main at a synchronized RFB service boundary. The controller worker never writes the RFB stream or touches GS.
3. **Published pointer state is explicit authority.** Cursor coordinates and ordinary click state advance only after the corresponding RFB publication succeeds. Wheel input is a momentary RFB button pulse at the current pointer while preserving ordinary held buttons; a partial pulse failure fails the session rather than fabricating a completed durable state.
4. **Keyboard tap serialization is balanced and deterministic.** An already-resolved semantic tap expands Shift -> Ctrl -> Alt, target down/up, then Alt -> Ctrl -> Shift. OSK printable Shift-layer selection remains distinct from a real remote Shift modifier; Ctrl/Alt and non-printable Shift use real modifiers.
5. **OSK/local foreground is semantic ownership, not pad ownership.** Local UI owns foreground, return relationship, transition quarantine and local dirty generation. The local-controller router maps normalized physical facts to local semantic actions and tracks transition-owned buttons until physical release.
6. **Opening local foreground establishes a hard mouse boundary.** Before OSK/calibration takes semantic foreground, application requests mouse-interpretation suspension, discards pre-boundary queued semantic work, neutralizes any remotely held ordinary click at the exact last-published coordinates, and rebases the suspended interpreter to that successfully-published neutral state.
7. **Physical polling continues during ordinary local foreground.** Mouse interpretation is suspended while OSK/calibration owns interaction, but libpad observation continues so physical-release quarantine can be proven. A physical continuity loss during suspension is remembered as a hard mouse reset boundary.
8. **Release, not time, ends transition quarantine.** Opening/closing gestures cannot leak into the new foreground. OSK close clears one-shot modifiers, returns foreground, and resumes desktop mouse interpretation only after physical release is proven and the no-overlay local generation has actually been presented.
9. **Calibration composes with the same ownership facts.** Once calibration owns foreground it gets first refusal over trustworthy controller observations and owns the shared mouse-suspension epoch until its release quarantine completes. Ordinary OSK and calibration foreground do not stack.
10. **RFB application servicing remains parser-safe.** Interaction work is serviced only at complete RFB server-message boundaries (including safe idle boundaries). It may emit ordered pointer/key writes but must not read the physical PSTV socket or consume server bytes.
11. **Input worker shutdown is fail-closed.** Cooperative shutdown must prove worker dormancy before its stack, queue/semaphore or libpad-owned state can be reclaimed/reused. An unproven shutdown is a session failure, not permission to force-delete live ownership.

## Historical mechanism, disposition, owner and validation

| Responsibility | H1/current mechanism | Disposition | Intended clean owner/process/bridge | Known-defect treatment | Validation obligation | Status |
| --- | --- | --- | --- | --- | --- | --- |
| libpad acquisition + physical continuity + semantic event production | clean `src/input/input_runtime.*`, reused directly by H1 | `KEEP_BEHAVIOR` | input component internals; input bridge exposes lifecycle/event/suspension processes | preserve zero-sample vs continuity-loss distinction and hard-reset behavior | host state tests where platform-neutral; PS2 disconnect/reconnect and lifecycle qualification | `RECONSTRUCTION_READY` |
| mouse pointer/button/wheel interpretation | clean mouse owner feeding input runtime | `KEEP_BEHAVIOR` | input component internals | do not move RFB button-bit vocabulary into mouse owner | host pointer/button/wheel/repeat tests; end-to-end RFB publication | `RECONSTRUCTION_READY` |
| application published-pointer authority + RFB pointer serialization | duplicated application orchestration in `src/app.c` and H1 coordinator | `SIMPLIFY` | application/RFB bridge process `remote interaction publication`; RFB owns wire encoding | preserve update-only-after-success and fail-closed partial wheel pulse | fake-RFB publication tests including failures; safe-boundary integration | `RECONSTRUCTION_READY` |
| keyboard tap expansion | clean `src/input/keyboard.*` | `KEEP_BEHAVIOR` | input keyboard owner; application/RFB bridge publishes sequence | no known defect in audited path | exhaustive modifier-order/balance host tests; RFB failure-prefix test | `RECONSTRUCTION_READY` |
| OSK state/key meaning | clean `src/ui/osk.*` | `KEEP_BEHAVIOR` with deferred known refinement | UI component internals | historical proportional vertical mapping is intentionally non-reversible for unequal rows; do not silently change during migration | existing OSK host tests; hardware interaction regression before any later navigation redesign | `RECONSTRUCTION_READY` |
| local foreground + transition quarantine | clean `src/ui/local_ui.*` + `local_controller.*` | `KEEP_BEHAVIOR` | UI internals; UI bridge exposes foreground/action process | release proof must remain physical, never timer-derived | deterministic open/close/quarantine tests; held-button and disconnect cases | `RECONSTRUCTION_READY` |
| suspend -> neutralize -> rebase -> release -> resume | clean input/UI/app composition, duplicated by H1 coordinator and calibration binding | `SIMPLIFY` | one application interaction-transition process coordinating input, UI and RFB bridges | do not collapse successfully-published remote state into sampled/local state | transition interleaving tests; held click; stale queued event; failed neutralization; presentation-before-resume | `RECONSTRUCTION_READY` |
| calibration foreground arbitration | H1 interaction coordinator + calibration binding | `SPLIT` | application process chooses foreground; calibration/UI owner consumes controller facts; input owner supplies facts | preserve no-stack and calibration-first-refusal once owned | OSK-vs-calibration arbitration, held entry, cancel/accept release quarantine | `RECONSTRUCTION_READY` |
| safe-boundary interaction servicing | H1 RFB session service callback | `KEEP_BEHAVIOR` | application/RFB bridge process section, consuming A001 logical stream boundary | never service benign work from a partial server message | parser boundary/idle-service tests and mux integration | `RECONSTRUCTION_READY` |
| cooperative input-worker teardown | input runtime + H1 static coordinator storage | `KEEP_BEHAVIOR` / `SIMPLIFY` storage consequence | input lifecycle owner; application shutdown sequencing | unproven dormancy must remain fail-closed; static H1 storage is scaffolding, not product requirement | forced worker-error/shutdown-order tests; repeated-session PS2 evidence | `RECONSTRUCTION_READY` |
| H1 interaction stats/counters | coordinator counters | `DIAGNOSTIC_ONLY` | diagnostics snapshots if still useful | never become synchronization authority | behavior with diagnostics absent | audit closure |
| CP2M L1+D-pad transient keyboard gesture | explicitly rejected by H1 integration intent | `DISCARD` | none | historical checkpoint remains evidence only | prove real keyboard/OSK composition instead | `RECONSTRUCTION_READY` |
| START+SELECT 750 ms calibration entry chord | explicitly labeled temporary hardware-test input adapter | `DISCARD_AS_PRODUCT_BINDING` | eventual product/UI binding policy; calibration lifecycle remains trigger-agnostic | do not discard the foreground/quarantine lifecycle it exercised | calibration entry via final semantic action plus regression of lifecycle | `RECONSTRUCTION_READY` |

## Clean ownership / bridge synthesis

No new product component is required by A005. The surviving responsibilities already have earned owners: input for physical acquisition/mouse/keyboard semantics, UI for OSK/foreground/local-controller semantics, RFB for wire serialization, and application for the cross-domain transition/publication sequence.

The experiment-local `h1_interaction_coordinator` and calibration interaction binding must not survive as parallel product owners. Their useful orchestration collapses into coherent process sections of the existing component bridges/application coordinator:

- `remote interaction publication` — consume semantic pointer/key work and publish through RFB at a safe boundary;
- `enter local foreground` — suspend interpretation, neutralize published remote click, rebase, establish foreground/quarantine;
- `leave local foreground` — prove release, publish disappearance, then resume interpretation;
- `calibration foreground handoff` — consume the same transition mechanism while preserving calibration's first-refusal ownership;
- `input lifecycle/shutdown` — stop producer and prove dormancy before resource reclamation.

Do not create destination-specific bridge files for input->RFB, UI->input, calibration->input, etc. The process is the unit of cross-component comprehension.

## Simplification rationale

H1 intentionally duplicated application-level composition because the experiment owned the physical PSTV connection and finite-session lifecycle. That duplication proved that the real clean interaction modules worked over muxed RFB; it did not prove a need for a second interaction architecture. The correct reconstruction is therefore unusually conservative: retain the existing domain owners and reconstruct only the changed cross-domain seams required by A001/A004 rather than rewriting input behavior.

The temporary CP2M keyboard gesture and START+SELECT hold are experimental trigger adapters. Their discard is semantically safe because the durable behavior is the semantic keyboard/calibration action and ownership lifecycle, not the physical test chord that invoked it.

## Known defects / cautions

- The OSK proportional vertical-navigation rule is a recorded historical imperfection: unequal-row transitions are not perfectly reversible. A005 preserves it during structural migration. Any later correction is a separate behavioral change with its own evidence.
- Input-runtime shutdown deliberately fails closed when dormancy cannot be proven. Reconstruction must not turn this into force termination merely to make repeated sessions easier.
- A physical controller continuity loss during mouse suspension is a hard state boundary and must not permit pre-loss persistent mouse mode/history to reappear after resume.
- Historical H1 interaction qualification applies to exact historical DUTs/checkpoints only. Reconstructed A005 source requires its own validation and, after PT_LOAD change, physical qualification.

## Reconstruction-ready acceptance

A005 is `RECONSTRUCTION_READY` because required behavior, current mechanisms, dispositions, clean owners/processes/bridges, known-defect treatment and validation obligations are explicit. Reconstruction may consume it only when dependency order permits; readiness does not override active A001 transport work or imply that A004 presentation is already reconstructed.

## Exact next audit pickup

A006 — top-level all-guns orchestration, finite shutdown and recovery. Trace boot/session loop ordering, CONFIG admission, transport/RFB/audio/MPEG startup dependencies, steady-state coordination, generation retirement, END/RESULT accounting, input/media shutdown ordering, failure convergence, repeated-session behavior, and which H1 resident-loop/test-control mechanics are scaffolding versus product lifecycle requirements. Preserve the current no-generic-timeout policy for unexplained silent waits unless contrary proven authority is found.
