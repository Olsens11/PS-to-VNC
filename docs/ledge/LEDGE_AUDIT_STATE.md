# Ledge Semantic Audit — Lane State

DOCUMENT=LEDGE_AUDIT_STATE
STATE_REVISION=0004
RECORDED_AT=2026-09-15T13:10:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0003
BASED_ON_GLOBAL_STATE_REVISION=0005
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns semantic-audit continuity only. It does not supersede the global ledge state or another lane's state.

## Authority inspected

- branch `ledge/h1-all-guns` at audit start: repository-visible descendant of global revision 0005 authority; GitHub contents writes preserve branch fast-forward ancestry;
- forensic H1 source: `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- governing reconstruction contract: revision `0001`;
- global work state consumed: revision `0005`;
- prior audit state: revision `0003`;
- A003 detailed audit: revision `0001`;
- A004 detailed audit: `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision `0001`.

The repository connector exposes committed branch authority but not an external Pi worktree's uncommitted status. This audit therefore makes no claim that such a worktree is clean and did not modify reconstructed product source.

## Current audit phase

`DEPENDENCY_CLOSURE_AND_PROCESS_CLASSIFICATION`

## Completed this lane snapshot

- Preserved A001-A003 readiness unchanged.
- Completed coherent audit tranche A004: calibration foreground/acceptance, base/inner/outer geometry authority, RFB freeze/request policy, accept-to-first-frame protection, first physical MPEG ownership/media-clock arm, shared compositor layering, exact-generation RFB suppression, retirement/failure restoration, and scheduler/drop policy.
- Preserved the distinction between remote framebuffer truth and presentation suppression: MPEG ownership excludes RFB from presentation in the suppression footprint without creating a second framebuffer authority.
- Classified the first valid physically presented MPEG frame as both ownership-promotion and A003 common-clock-arm boundary; acceptance, START, decode and upload remain too early.
- Preserved one physical GS presentation owner/path and classified the historical cumulative39 source-inclusion/wrapper technique as reconstruction scaffolding rather than desired product topology.
- Completed disposition of deferred H1 fields 33, 38-41, 45-46 and 55-56: qualified scheduler/profile and accepted-generation geometry survive; independent laboratory knobs/competing geometry authorities do not.
- Added simplification entries S005-S008 for committed calibration authority, one presentation owner, explicit presentation ownership/RFB policy, and qualified absolute scheduler.
- No reconstructed product source, reconstruction state, validation findings, or global state was modified.

## Progress counts at this snapshot

- coherent process tranches fully classified: 4 (A001, A002, A003, A004)
- A001 responsibility/invariant rows reconstruction-ready: 8
- A002 responsibility rows reconstruction-ready: 6
- A003 reconstruction-ready responsibility groups: 7 plus 1 narrow media-clock arm contract
- A004 reconstruction-ready responsibility groups: 9 plus 1 diagnostic-only group
- CONFIG field IDs classified: 61/61; all presentation-deferred field dispositions resolved
- simplification register entries: 8
- remaining seeded major process families: 3 (interaction/input composition; top-level orchestration/shutdown/recovery; residual diagnostics/completeness closure)
- whole-build dependency closure: recursive route identified; exhaustive file/symbol proof still incomplete

## Unresolved questions

- General controller/mouse/keyboard/OSK interaction composition remains unaudited beyond calibration foreground/release-quarantine facts required by A004.
- Top-level all-guns startup, steady-state coordination, finite shutdown, failure/recovery and repeated-session sequencing remain unaudited.
- Residual diagnostic witnesses and exhaustive recursive build/source/symbol completeness proof remain open.
- Exact repeated-session qualification status across final H1 mechanisms remains evidence-specific; do not infer reconstructed qualification from historical H1 checkpoints.
- A004's qualified absolute scheduler/drop semantics are ready for reconstruction, but exact production constants remain qualified-profile evidence rather than new user-facing configuration.

## Exact next pickup

Audit interaction/input composition as A005. Trace physical controller acquisition versus semantic ownership, pointer/button/scroll routing, OSK/local-UI foreground transitions, keyboard tap/modifier serialization, calibration entry/exit handoff, release quarantine, and RFB safe-boundary interaction servicing. Do not reopen A004 geometry/presentation policy except where an interaction transition directly consumes its already-classified foreground/ownership contract.