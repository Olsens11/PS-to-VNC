# Ledge Semantic Audit — Lane State

DOCUMENT=LEDGE_AUDIT_STATE
STATE_REVISION=0005
RECORDED_AT=2026-09-15T14:19:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0004
BASED_ON_GLOBAL_STATE_REVISION=0005
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns semantic-audit continuity only. It does not supersede the global ledge state or another lane's state.

## Authority inspected

- branch `ledge/h1-all-guns` at audit start: `55dec26d05a46feb1ffe570cd8955c0cd749fc8f`, rechecked identical immediately before audit writes;
- forensic H1 source: `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- governing reconstruction contract: revision `0001`;
- global work state consumed: revision `0005`;
- prior audit state: revision `0004`;
- A004 detailed audit: revision `0001`;
- A005 detailed audit: `docs/ledge/LEDGE_AUDIT_A005_INTERACTION_INPUT.md` revision `0001`.

The repository connector exposes committed branch authority but not an external Pi worktree's uncommitted status. This audit therefore makes no claim that such a worktree is clean and did not modify reconstructed product source.

## Current audit phase

`DEPENDENCY_CLOSURE_AND_PROCESS_CLASSIFICATION`

## Completed this lane snapshot

- Preserved A001-A004 readiness unchanged.
- Completed coherent audit tranche A005: physical controller observation versus semantic ownership; mouse pointer/button/wheel routing; successfully-published pointer authority; deterministic keyboard tap/modifier sequencing; OSK/local-UI foreground transitions; suspend/neutralize/rebase/release/resume ordering; physical-release quarantine; calibration foreground arbitration; safe-boundary RFB interaction servicing; and fail-closed input-worker shutdown.
- Confirmed that H1 intentionally reused the real through-Issue-39 input/mouse/keyboard/OSK/UI owners. `h1_interaction_coordinator` is experiment-local composition scaffolding, not a second product interaction owner.
- Classified the CP2M L1+D-pad keyboard gesture as discarded historical experiment behavior and the START+SELECT 750 ms calibration entry chord as a temporary hardware-test trigger rather than product binding. The trigger-agnostic calibration lifecycle survives.
- Preserved the recorded OSK proportional vertical-navigation imperfection during structural migration; correction remains a separate future behavioral change.
- Preserved main/application-thread RFB publication and complete-server-message safe-boundary servicing. The controller worker remains transport-agnostic and GS-agnostic.
- No reconstructed product source, reconstruction state, validation findings, global state, or prior audit history entry was modified.

## Progress counts at this snapshot

- coherent process tranches fully classified: 5 (A001, A002, A003, A004, A005)
- A001 responsibility/invariant rows reconstruction-ready: 8
- A002 responsibility rows reconstruction-ready: 6
- A003 reconstruction-ready responsibility groups: 7 plus 1 narrow media-clock arm contract
- A004 reconstruction-ready responsibility groups: 9 plus 1 diagnostic-only group
- A005 reconstruction-ready behavioral responsibility groups: 10, plus diagnostic-only counters and 2 discarded experiment-only trigger bindings
- CONFIG field IDs classified: 61/61
- simplification register entries: 8; A005 simplification rationale is recorded in its detailed audit and does not alter prior S001-S008
- remaining seeded major process families: 2 (top-level orchestration/shutdown/recovery; residual diagnostics/completeness closure)
- whole-build dependency closure: recursive route identified; exhaustive file/symbol proof still incomplete

## Known-defect / caution accounting

- A003 false-EOF decoder-stop defect remains explicitly prohibited from reintroduction.
- A005 preserves the historical OSK unequal-row proportional vertical-navigation imperfection during migration; no silent behavior fix is authorized.
- A005 input-runtime shutdown remains fail-closed when worker dormancy cannot be proven; force termination is not an accepted simplification.
- A005 physical continuity loss during a mouse-suspension epoch remains a hard reset boundary.
- Historical H1 hardware/checkpoint evidence remains forensic evidence only and does not qualify reconstructed A005 code.

## Unresolved questions

- Top-level all-guns startup, CONFIG-to-domain activation order, steady-state coordination, finite shutdown, failure/recovery and repeated-session sequencing remain unaudited.
- Residual diagnostic witnesses and exhaustive recursive build/source/symbol completeness proof remain open.
- Exact repeated-session qualification status across final H1 mechanisms remains evidence-specific; do not infer reconstructed qualification from historical checkpoints.
- Dependency ordering for reconstruction remains global/reconstruction-lane authority: A005 readiness does not authorize bypassing active A001 transport reconstruction.

## Exact next pickup

Audit A006 top-level all-guns orchestration, finite shutdown and recovery. Trace boot/session-loop ordering, CONFIG admission, transport/RFB/audio/MPEG startup dependencies, steady-state coordination, generation retirement, END/RESULT accounting, input/media shutdown ordering, failure convergence, repeated-session behavior, and resident-loop/test-control scaffolding. Preserve the current no-generic-timeout policy for unexplained silent waits unless contrary proven authority is found. After A006, perform A007 residual diagnostics plus exhaustive recursive dependency/source/symbol completeness closure.