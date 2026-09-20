# Reconstruction shift — A004 MPEG CALIBRATION core R1

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T17:18:48-04:00
COMPLETED_AT=2026-09-20T17:31:52-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=672f67eb818cb3bd08ebaf611ac9378784d6d6f6
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet `A004-MPEG-CALIBRATION-CORE-R1`: reconstruct the
clean MPEG CALIBRATION owner/model that defines one accepted MPEG region value
and its local foreground edit/review/accept/cancel semantics, without wiring
RFB freeze/suppression, live MPEG activation, Pi producer behavior, Transport
START/RETIRE, MPEG decoder ownership, GS composition, first physical
presentation, media-clock arm, scheduler/drop, final retirement handoff,
Application orchestration, A005 input reconstruction, or hardware qualification.

Foreman state revision 0022 remained current state authority throughout this
worker shift. No Foreman state document was modified.

## Startup / authority

The normal execution seat was:

`/home/ps2/src/PS-to-VNC-ledge-manual`

Initial execution-seat preflight observed local
`170e2198d41b3d703a5fe86bd5c329f2248d031a` behind live remote
`672f67eb818cb3bd08ebaf611ac9378784d6d6f6` by two commits, with a clean
worktree and no cherry-pick/merge/revert/rebase/sequencer markers.

A safe `git merge --ff-only origin/ledge/h1-all-guns` adopted the live
Foreman authority. Then:

- `scripts/resume-state.sh` — PASS
- `scripts/check.sh` — PASS

No unknown local work was discarded or rewritten.

## Frozen H1 forensic trace

Frozen authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Inspected at minimum:

- `experiments/media-harness-h1/mpeg_presentation_calibration/README.md`
  - established that this lineage owns MPEG calibration state/geometry only;
  - recorded the recovered interaction model;
  - kept RFB visual ownership/composition outside the calibration core.
- `mpeg_presentation_calibration.h`
  - separated base rectangle, inner matte, outer matte, screen state, candidate
    and committed region state;
  - exposed the review/accept/cancel and release-quarantine facts.
- `mpeg_presentation_calibration_geometry.c`
  - preserved 16-pixel MPEG base width/height alignment;
  - preserved pixel-precision X/Y movement;
  - preserved distinct inner-matte and outer-footprint edits;
  - preserved canvas-clipped suppression geometry.
- `mpeg_presentation_calibration_state.c`
  - recovered centered default candidate construction;
  - recovered committed-vs-current value separation;
  - recovered review acceptance only after X release then a fresh X press;
  - recovered cancel without acceptance and post-exit release quarantine.
- `mpeg_presentation_calibration_test.c`
  - confirmed the 704x462 historical default geometry
    (x=176, y=119, width=352, height=224);
  - confirmed center-preserving macroblock resize, pixel movement, separate matte
    edits, suppression footprint, acceptance, and release quarantine.
- `h1_mpeg_calibration_foreground.{c,h}` and
  `h1_mpeg_calibration_foreground_test.c`
  - showed that mouse suspension, pointer neutralization/rebase, RFB visual
    freeze, and resume ordering were later composition/foreground bridge work;
  - those cross-domain callbacks were intentionally not adopted into P1.
- `h1_mpeg_calibration_entry_hold.{c,h}` and
  `h1_mpeg_calibration_entry_hold_test.c`
  - showed the historical 750 ms START+SELECT timing policy and controller
    sample consumption;
  - this is entry/input orchestration, so P1 deliberately exposes explicit
    calibration begin rather than importing that timer.
- `h1_mpeg_calibration_accept_edge_test.c`
  - proved the historical accepted edge is emitted exactly once after review
    release/re-press while the committed region remains available afterward.
- `h1_mpeg_calibration_adapter.c`,
  `h1_mpeg_calibration_adapter_test.c`, and
  `h1_cp2p_session_coordinator_test.c`
  - used only to resolve how accepted calibration crosses into later
    orchestration;
  - no CP2P, RFB, worker, Transport, or MPEG-presentation ownership logic was
    imported into the P1 owner.

Historical adapters were treated as evidence, not mandatory production
structure.

## Placement / ownership decision

Placement selected:

- `src/ui/mpeg_calibration.c`
- `src/ui/mpeg_calibration.h`

Reason:

The current clean topology assigns PS2-local foreground/state semantics to
`src/ui/`. P1 contains only calibration-local value, edit/review/accept/cancel
state and geometry resolution. It performs no framebuffer conversion, GS
composition, first-frame presentation, or presentation mechanism, so
`src/display/` remained untouched.

No new top-level source directory was created.

The existing DESKTOP CALIBRATION authority remained untouched. The MPEG value
was recovered only from the frozen MPEG CALIBRATION lineage and current A004
authority.

The historical 750 ms START+SELECT hold timer was not imported because the
active packet explicitly excludes general input reconstruction. Entry timing
remains a later input/orchestration responsibility; the clean owner exposes a
direct begin seam.

## Exact product/test/docs work

### Commit `ad00a7a07c15c8d56f89e5ac664e07507e50c312`

`feat(ui): add MPEG calibration core`

Added/changed:

- `src/ui/mpeg_calibration.c`
- `src/ui/mpeg_calibration.h`
- `tests/unit/mpeg_calibration_test.c`
- `tests/Makefile`

Implemented:

- explicit base rectangle, inner matte, and outer/suppression geometry meanings;
- bounded 16-pixel-aligned width/height with one-pixel X/Y placement;
- overflow-safe widened arithmetic before storing bounded int32 geometry;
- deterministic centered defaults;
- edit, reset, review, accept, cancel, and release-quarantine state;
- held-X safety: entering REVIEW with accept held cannot publish acceptance;
  acceptance requires release followed by a fresh press;
- exactly one copied caller-visible accepted region;
- committed-value reuse on recalibration;
- no acceptance-to-MPEG-ownership implication.

Focused host tests cover default geometry, macroblock sizing, pixel placement,
inner/outer meanings, suppression clipping, invalid/overflow-like bounds,
held-X release/re-press, exactly-once acceptance, cancel, quarantine, and
recalibration from committed value.

### Commit `51025be8553c3dad8b5a11002fb57f9bb7a5af62`

`docs(symbols): index MPEG calibration core`

Updated:

- `src/ui/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

The comprehensive strict dictionary discovery identified only the two new file
coverage gaps plus their newly defined symbols. 119 unique canonical dictionary
rows were added; the generated `src/ui` portal count advanced from 351 to
470.

### Commit `3d9f4a821f16d51a3d22597b4a539009aac7705f`

`refactor(ui): name MPEG calibration action enum`

Strict dictionary validation exposed that the anonymous C action enum received
a synthetic Ctags identity that could not also satisfy the validator's
source-visible-name rule. The enum was therefore given the stable descriptive
source name:

`pstvnc_mpeg_calibration_action`

No enum value or runtime behavior changed.

The complete focused/full host suites were rerun after this product-source
naming change.

### Commit `094f07fff17801152bcd0d17ac69c570d8bfad27`

`docs(symbols): bind calibration actions to named enum`

Updated only `src/ui/SYMBOLS.md` so the ten action enum-value rows use the
new named enum as lexical owner.

GitHub comparison proved this final commit changed no product/test source.

## A004 P1 worker disposition

These are Reconstruction-worker evidence dispositions for Foreman review, not
Foreman acceptance.

### A004-P1-C1 CALIBRATION_SEPARATION — MET

No DESKTOP CALIBRATION source changed. MPEG behavior was recovered from the
frozen MPEG calibration lineage and current A004 authorities only.

### A004-P1-C2 MPEG_REGION_VALUE — MET

The clean owner represents base, inner-content matte, and outer/suppression
footprint as distinct meanings and exposes deterministic host-testable geometry.

### A004-P1-C3 GEOMETRY_RULES — MET

Base width/height are positive, at least 16, and macroblock aligned. Placement
remains pixel-precision. Bounds/clipping use widened arithmetic before int32
storage and are covered by host tests.

### A004-P1-C4 FOREGROUND_STATE — MET

Edit/review/accept/cancel plus release quarantine are owner-local. Final source
contains no RFB, Transport, MPEG-decoder, display, platform/PS2, Pi, or GS
dependency.

### A004-P1-C5 ACCEPT_EDGE — MET

Review acceptance is unarmed while the entering accept action remains held.
Only complete release arms acceptance; a later fresh accept press publishes
acceptance exactly once.

### A004-P1-C6 ACCEPTED_VALUE — MET

A successful accept copies the exact current region into committed owner state
and into caller-visible effects. Cancel publishes no accepted value. Later draft
edits cannot retroactively mutate the prior accepted copy.

### A004-P1-C7 ACCEPT_NOT_OWNERSHIP — MET

No code path sends START, admits a producer, starts the MPEG decoder, promotes
first physical presentation, arms the media clock, or claims MPEG visual
ownership. Acceptance means only that one region value was accepted.

### A004-P1-C8 CLEAN_INTEGRATION — MET

Clean source synopses, test registration, `src/ui/SYMBOLS.md`, and generated
portal are synchronized.

Final packet diff from Foreman base touches only:

- `src/ui/mpeg_calibration.c`
- `src/ui/mpeg_calibration.h`
- `src/ui/SYMBOLS.md`
- `tests/unit/mpeg_calibration_test.c`
- `tests/Makefile`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

No `src/display`, `src/rfb`, `src/transport`, `src/mpeg`,
`src/platform`, `src/app.c`, or `src/app.h` file changed.

### A004-P1-C9 EVIDENCE_BOUNDARY — MET

Evidence remains explicitly separated. No physical hardware run and no full
A004 completion claim are made.

## Verification / evidence

Execution-seat product-source authority
`3d9f4a821f16d51a3d22597b4a539009aac7705f`:

- packet `git diff --check` — PASS
- `make -C tests mpeg-calibration-unit` — PASS
- `make -C tests unit` — PASS
- `./scripts/check.sh` — PASS

The complete host suite passed with the final product-source bytes.

The final current head
`094f07fff17801152bcd0d17ac69c570d8bfad27` differs from that fully tested
product-source authority only in `src/ui/SYMBOLS.md`.

Final current-head gates:

- `./scripts/check.sh` — PASS
- `python3 scripts/source-dictionary.py check --long --require-complete --strict`
  — PASS
- generated portal vs
  `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` — exact/reproducible PASS
- final local/remote identity — PASS
- final execution-seat worktree — clean
- GitHub source dependency scan — no forbidden RFB/Transport/MPEG-decoder/
  display/platform/PS2 references in the calibration core
- GitHub compare from packet base — only the six authorized product/test/docs
  paths listed above changed

Evidence boundary:

- SOURCE=A004_P1_COMPLETE
- FOCUSED_HOST=PASS
- FULL_HOST=PASS
- PROJECT_CHECK=PASS
- STRICT_DICTIONARY=PASS
- PORTAL_REPRODUCIBILITY=PASS
- PS2_RUNTIME=NOT_WIRED_IN_THIS_PACKET
- HARDWARE=NOT_RUN_NOT_CLAIMED
- FULL_A004=NOT_CLAIMED

## Findings / known-defect accounting

The first post-source canonical project check failed only because the two newly
created clean UI files did not yet have local dictionary coverage. The source
and focused/full host tests had passed. Exact long dictionary discovery was used
to populate the local dictionary rather than guessing entries.

The first dictionary reconciliation then exposed a validator interaction with an
anonymous enum: Ctags generated `__anon4674034f0103`, but the dictionary
validator also requires documented names to appear literally in source. The
source enum was named `pstvnc_mpeg_calibration_action`; strict validation then
correctly required the ten enum-value rows to point at that named lexical owner.
Those rows were repaired in the final dictionary-only commit.

No behavior regression was found during these repairs.

## Ambiguities preserved rather than inferred

- The historical 750 ms START+SELECT entry hold is not treated as calibration
  core ownership; it remains later input/orchestration policy.
- Historical foreground bridge RFB freeze, pointer neutralization/rebase, and
  mouse suspension are not treated as calibration-core behavior.
- The calibration canvas is only a local geometry bound. It is not inferred to
  be Pi desktop geometry authority.
- Outer matte is represented as calibration-local suppression-footprint
  geometry; P1 does not implement RFB suppression ownership.
- Acceptance is not interpreted as MPEG visual ownership, START admission,
  first-frame presentation, clock arm, or producer/decoder activation.

## State / contract revisions

Consumed:

- Foreman state revision 0022
- Reconstruction contract revision 0005
- work-log contract revision 0006
- Wire runtime decisions revision 0011
- architecture overlay revision 0004
- A004 audit revision 0001
- A004 calibration-separation invariant revision 0001

Produced no Foreman state revision and no next-packet authority.

## Evidence gaps

PENDING_LOCAL=NONE_FOR_A004_P1_REQUIRED_HOST_REPOSITORY_GATES

PS2_RUNTIME_WIRING=NOT_IN_PACKET
HARDWARE_PENDING=YES_FOR_LATER_A004_VISUAL_RUNTIME_QUALIFICATION
FULL_A004_COMPLETION=NOT_CLAIMED

## Next pickup

Return baton to Foreman at pre-log source/dictionary authority
`094f07fff17801152bcd0d17ac69c570d8bfad27` plus this immutable
Reconstruction log commit.

Foreman should independently inspect the P1 source/tests/evidence and decide the
next bounded A004 owner seam. Reconstruction does not select or publish that
next packet.

Do not begin another A004 tranche from this worker shift.
