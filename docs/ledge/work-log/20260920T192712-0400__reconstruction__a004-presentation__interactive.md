# Reconstruction shift — A004 current-Q7 retirement / visible handoff R5

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T19:27:12-04:00
COMPLETED_AT=2026-09-20T19:35:01-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=3ce6b596a5fc0936c30ab7c31936e1a2c82c4675
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A004-Q7-RETIREMENT-VISIBLE-HANDOFF-R5`

The bounded objective was to reconstruct the Presentation/Platform side of
current-Q7 exact-run MPEG retirement:

`MPEG_OWNED -> RETIRING -> REVEAL_PENDING -> RFB_ONLY`

while preserving the exact run snapshot and retained visible MPEG/mattes until a
single synchronized no-MPEG physical reveal actually succeeds.

This packet deliberately does not prove or orchestrate producer closure,
Transport retirement/residual discard, decoder safe-stop, RFB-underlay
restoration, scheduler/drop, decoder-worker handoff, final Application
orchestration, Wire-loss recovery, or physical qualification.

## Authority consumed

Consumed current authority at wake:

- Foreman State revision 0027;
- Reconstruction Contract revision 0006;
- immutable work-log contract revision 0006 / log format revision 0001;
- Wire runtime decisions revision 0011, especially current Q7/Q8/Q9;
- A003 MPEG-generation audit revision 0001;
- A004 presentation/calibration audit revision 0001 as temporal evidence, with
  current Q7 governing conflicting retirement wording;
- current clean architecture and architecture overlay;
- current P3/P4 Presentation/Platform source;
- current RFB flow-policy and MPEG-decoder ownership seams;
- canonical host, strict PS2 compile, and linked-build manifests.

Starting branch authority was:

`3ce6b596a5fc0936c30ab7c31936e1a2c82c4675`

Foreman packet execution classification remained:

`AUTONOMOUS_RECONSTRUCTION`

No user terminal proxy or physical PS2 action was required or requested.

## Current source inspected

Inspected current:

- `src/display/mpeg_presentation.{c,h}`;
- `src/display/mpeg_compositor.{c,h}`;
- `src/platform/ps2_graphics.{c,h}`;
- `src/rfb/flow_policy.{c,h}`;
- `src/mpeg/decoder.{c,h}`;
- `tests/unit/mpeg_presentation_test.c`;
- `tests/unit/mpeg_compositor_test.c`;
- `tests/Makefile`;
- `scripts/check-clean-ps2-compile.sh`;
- `mk/issue7-clean.mk`.

Recovered current ownership facts:

- P3 owns one exact caller-generation visible run snapshot.
- P4 owns one exact-run synchronized compositor coordinator.
- Platform remains the sole GS/dmaKit owner and retains desktop, MPEG and local
  overlay presentation resources.
- Ordinary desktop presentation already redraws retained MPEG above refreshed
  desktop truth, so RFB underlay may advance without revealing the suppression
  footprint.
- Session media-clock lifetime is independent from a single MPEG visible run.
- RFB flow policy owns request/FULL-debt mechanics independently.
- MPEG decoder owns decoder-call/resource safe-stop but not visible retirement.

## Frozen retirement evidence traced

Frozen H1 authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

The immediately preceding P4 forensic trace already established the qualified
single-owner compositor mechanism in:

- `experiments/media-harness-h1/h1_cumulative39_graphics.{c,h}`.

For this P5 retirement shift, retirement-specific evidence was freshly traced
through:

- `experiments/media-harness-h1/CP2P_MPEG_SAFE_STOP_LIFECYCLE.md`;
- `experiments/media-harness-h1/h1_cp2p_session_coordinator.c`;
- `experiments/media-harness-h1/h1_cp2p_session_coordinator_stop_only.c`;
- `experiments/media-harness-h1/h1_cp2p_mpeg_worker.c`;
- `experiments/media-harness-h1/h1_cp2p_retirement_control.py`;
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_cp2p_rfb_restoration_test.c`.

Recovered binding lessons:

- asynchronous lifecycle stop must not synthesize decoder EOF from inside an
  active `MPEG_Picture()` data callback;
- decoder stop is observed at a safe completed-picture/project-code boundary;
- old H1/CP2P then fenced Pi retirement/ACK, joined the finished worker, cleared
  MPEG pixels and finalized old residual queue state;
- historical `clear_video()` was a useful physical mechanism but combined too
  much lifecycle meaning for the current architecture;
- current Q7 supersedes older post-teardown-only RFB restoration: RFB underlay
  may rebuild while retiring MPEG remains visually on top;
- restoration evidence belongs to RFB/Application policy and must be genuinely
  new/current, not manufactured by Presentation.

P5 therefore reconstructs only the visible retirement gate and synchronized
reveal primitive. External lifecycle proofs remain caller/Application facts.

## Presentation retirement state

Extended:

- `src/display/mpeg_presentation.h`
- `src/display/mpeg_presentation.c`

with explicit states:

- `PSTVNC_MPEG_PRESENTATION_RETIRING`;
- `PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING`.

The resulting visible lifecycle is:

`RFB_ONLY -> WAIT_FIRST_FRAME -> MPEG_OWNED -> RETIRING -> REVEAL_PENDING -> RFB_ONLY`

New exact-generation transitions:

- `pstvnc_mpeg_presentation_begin_retirement()`
  - legal only for the exact nonzero current MPEG_OWNED generation;
- `pstvnc_mpeg_presentation_seal_retirement()`
  - legal only for the exact RETIRING generation;
- `pstvnc_mpeg_presentation_commit_reveal()`
  - legal only for the exact REVEAL_PENDING generation after the caller's
    physical reveal coordinator has succeeded.

RETIRING and REVEAL_PENDING both:

- retain the immutable geometry/generation snapshot;
- report COMPOSITED mode;
- report MPEG visual ownership;
- do not require the generic P2 global RFB freeze.

No direct MPEG_OWNED -> RFB_ONLY transition exists.

## Drain and seal behavior

Updated:

- `src/display/mpeg_compositor.{c,h}`.

`pstvnc_mpeg_compositor_present()` now accepts exact frames in:

- WAIT_FIRST_FRAME;
- MPEG_OWNED;
- RETIRING.

Therefore already accepted exact-generation data can continue to drain after
visible retirement begins.

It rejects:

- wrong/stale run generations;
- new run identity against the retained snapshot;
- any frame once Presentation reaches REVEAL_PENDING.

The seal transition is therefore an explicit frame-admission fence in
Presentation. It does not claim that producer, decoder or Transport work has
actually completed; the caller must prove those prerequisites before choosing to
seal.

## Underlay overlap

No RFB product source changed.

During RETIRING, ordinary
`pstvnc_ps2_graphics_present()` continues updating the cached desktop/local
overlay while Platform retains and redraws the MPEG/suppression/matte layers on
top.

Thus the physical source shape supports current-Q7 overlap:

- remote desktop truth can advance underneath;
- retiring MPEG remains visible;
- suppression is not removed merely because retirement started.

P5 creates or consumes no P2 FULL-refresh debt and calls no RFB API.

## REVEAL_PENDING and synchronized final handoff

Added the Platform physical primitive:

`pstvnc_ps2_graphics_reveal_retained_video()`

to the existing sole GS/dmaKit owner.

It:

1. requires initialized graphics, cached desktop and retained MPEG visibility;
2. submits cached desktop plus current local overlay with the retained
   MPEG/suppression/inner-matte layers omitted;
3. uses the existing single shared render path;
4. reaches the existing `gsKit_sync_flip()` completion boundary;
5. only after synchronized success commits:
   - `video_visible = 0`;
   - retained base/inner/suppression geometry cleared;
6. preserves the reusable MPEG texture/VRAM allocation for a later run.

The function receives no run generation and knows nothing about producer,
decoder, Transport or RFB lifecycle facts.

If the no-video render fails before synchronized completion, retained-video facts
remain intact and the reveal remains retryable/containable.

## Exact-run reveal coordinator

Added:

`pstvnc_mpeg_compositor_reveal_retired()`

The coordinator:

1. requires exact REVEAL_PENDING state;
2. requires the exact nonzero retained generation;
3. requires the immutable run snapshot still to exist;
4. invokes only the Platform no-video synchronized reveal;
5. requires Platform synchronized success;
6. only then calls exact
   `pstvnc_mpeg_presentation_commit_reveal()`;
7. reports `retirement_revealed` only after logical commit.

It has no media-clock parameter and calls no media-clock API.

It imports/calls no RFB, Transport, MPEG decoder or calibration mechanism.

## Session-clock stability

Retirement does not arm, clear, replace or reset the session media clock.

The focused compositor fixture proves:

- first MPEG presentation establishes one session epoch;
- a complete P5 retirement/reveal leaves that epoch unchanged;
- a later distinct MPEG run reuses the already-armed session epoch rather than
  replacing it.

## Focused tests

Extended:

- `tests/unit/mpeg_presentation_test.c`;
- `tests/unit/mpeg_compositor_test.c`.

Presentation tests cover:

- wrong-generation begin-retirement rejection;
- exact MPEG_OWNED -> RETIRING;
- retained COMPOSITED/visual ownership and immutable snapshot;
- duplicate begin/new arm/wrong seal rejection;
- exact RETIRING -> REVEAL_PENDING;
- retained snapshot/visual ownership while sealed;
- first-frame/abort/rebegin rejection while sealed;
- wrong exact reveal commit rejection;
- exact logical reveal commit -> RFB_ONLY;
- snapshot removal only at final commit;
- duplicate final commit rejection.

Compositor tests cover:

- exact-generation drain frame presentation while RETIRING;
- stale generation rejection during drain;
- complete frame fence after REVEAL_PENDING;
- wrong-generation reveal rejected before Platform;
- Platform reveal failure leaves REVEAL_PENDING and snapshot intact;
- unsynchronized reveal leaves REVEAL_PENDING;
- exact synchronized reveal commits RFB_ONLY;
- duplicate reveal rejection;
- session clock unchanged by retirement;
- later run preserves the same session epoch.

## Commits

P5 worker source/integration range after Foreman base:

1. `05e3930a8c82a7f0c4b1080de44972028bb0fd75`
   — `feat(display): add exact-run MPEG retirement reveal`
2. `6480038ae5ec69eae37a2af5e52ecd7972d107fa`
   — tree-identical deterministic dictionary-reconciliation trigger
3. `3b8b9adc09a17fe373edf35b5416934e95e7cc8a`
   — automation-created `docs(symbols): reconcile current clean definitions`
4. `4367ed3b718c2740d3b3dcefb1ecd24ed93b9556`
   — `docs(symbols): describe MPEG retirement handoff ownership`

No new build-manifest commit was needed because P4 had already placed every
modified P5 translation unit directly in canonical strict PS2 compile and
current linked-build coverage.

## Exact changed paths

Compared with Foreman base
`3ce6b596a5fc0936c30ab7c31936e1a2c82c4675`, final pre-log authority
`4367ed3b718c2740d3b3dcefb1ecd24ed93b9556` changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `src/display/SYMBOLS.md`;
- `src/display/mpeg_compositor.c`;
- `src/display/mpeg_compositor.h`;
- `src/display/mpeg_presentation.c`;
- `src/display/mpeg_presentation.h`;
- `src/platform/SYMBOLS.md`;
- `src/platform/ps2_graphics.c`;
- `src/platform/ps2_graphics.h`;
- `tests/unit/mpeg_compositor_test.c`;
- `tests/unit/mpeg_presentation_test.c`.

No Application, RFB, Transport, MPEG-decoder, calibration/UI, input, audio,
configuration or framebuffer product source changed.

## Dictionary integration

Deterministic reconciliation reported:

- Display before=186, removed=0, added=33;
- Platform before=147, removed=0, added=5;
- every other maintained reconciliation domain unchanged;
- total removed=0;
- total added=38;
- complete strict dictionary check PASS.

Maintained responsibility prose now states:

- Display owns exact-run visible ownership, two-phase retirement and final
  visible-handoff sequencing;
- Platform owns fail-closed retained-video reveal through the sole synchronized
  GS/dmaKit presentation mechanism.

## Final static boundary review

At final pre-log authority:

`4367ed3b718c2740d3b3dcefb1ecd24ed93b9556`

Presentation contains:

- no RFB dependency;
- no Transport dependency;
- no MPEG-decoder dependency;
- no media-clock call;
- no direct MPEG_OWNED -> RFB_ONLY retirement shortcut.

Display compositor contains:

- no RFB dependency;
- no Transport dependency;
- no MPEG-decoder dependency;
- no calibration/UI dependency;
- no gsKit/dmaKit call;
- no generic clear-video path;
- no media-clock parameter on retirement reveal.

Platform contains:

- no run generation;
- no RFB/decoder lifecycle dependency;
- no generic `clear_video()` API;
- the sole retained-video reveal primitive;
- retained-video visibility commit only after synchronized no-video render
  success.

## Final machine evidence

Final exact source/dictionary authority:

`4367ed3b718c2740d3b3dcefb1ecd24ed93b9556`

GitHub Actions run:

`35544987580`

completed SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on the ordinary coherent commit.

Host logs explicitly report:

- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`.

Direct PS2 compile explicitly compiled:

- `src/display/mpeg_presentation.c`;
- `src/display/mpeg_compositor.c`;
- `src/platform/ps2_graphics.c`;

and reported:

`CLEAN_PS2_COMPILE_CHECK=PASS`

The linked current-source build compiled/linked
`mpeg_presentation.o`, `mpeg_compositor.o` and `ps2_graphics.o` in both clean
reproducibility builds and reported:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

This is machine/source/build evidence only, not physical display qualification.

## A004 P5 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A004-P5-C1 RETIRING_STATE — MET

Only the exact active nonzero MPEG_OWNED generation may enter RETIRING.
RETIRING retains snapshot, COMPOSITED mode and visual MPEG ownership.

### A004-P5-C2 DRAIN_FENCE — MET

Exact retiring frames remain presentable before seal. Wrong/stale generations
fail closed. REVEAL_PENDING rejects further MPEG frames before Platform is
called.

### A004-P5-C3 UNDERLAY_OVERLAP — MET

Ordinary desktop updates continue through the existing Platform presenter while
retained MPEG/suppression/matte layers remain drawn above the refreshed cached
desktop.

### A004-P5-C4 REVEAL_PENDING — MET

Exact seal creates a distinct REVEAL_PENDING state that retains run snapshot and
visible MPEG ownership while fencing further MPEG frame presentation.

### A004-P5-C5 EXTERNAL_PROOF_BOUNDARY — MET

P5 contains no producer-close, decoder-safe-stop, Transport-residual or
RFB-current proof mechanism. Those remain external caller/Application facts.

### A004-P5-C6 SINGLE_GS_REVEAL — MET

Final no-video reveal executes only through existing
`src/platform/ps2_graphics.c`, the sole GS/dmaKit owner, and its synchronized
render boundary.

### A004-P5-C7 FAIL_CLOSED_REVEAL — MET

Platform does not commit `video_visible=0` or clear retained geometry until the
no-video synchronized render succeeds. Failed/unsynchronized host reveal leaves
logical REVEAL_PENDING and run snapshot intact.

### A004-P5-C8 FINAL_COMMIT — MET

Presentation reaches logical RFB_ONLY and clears the snapshot only after exact
REVEAL_PENDING generation validation plus successful synchronized Platform
reveal.

### A004-P5-C9 CLOCK_STABILITY — MET

Retirement coordinator has no media-clock parameter. Focused evidence proves
the session epoch is unchanged through retirement and reused by a later run.

### A004-P5-C10 RFB_POLICY_BOUNDARY — MET

No RFB API, request selection, FULL-refresh debt or restoration proof is
duplicated inside Presentation/Platform P5 source.

### A004-P5-C11 Q7_COMPATIBILITY — MET

The source shape supports the current-Q7 sequence:

external close/fence -> exact RETIRING -> accepted-frame drain while RFB underlay
may advance -> exact seal -> synchronized no-MPEG reveal -> logical RFB_ONLY.

No H1 direct-stop shortcut exists.

### A004-P5-C12 CLEAN_EVIDENCE — MET

Focused/full host tests, project check, complete strict dictionary audit, direct
PS2 compile and current linked reproducibility all pass on the exact final
source authority.

Physical qualification remains separate.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A004_P5_REQUIRED_REPOSITORY_MACHINE_GATES
PHYSICAL_PS2_REVEAL=NOT_RUN_NOT_CLAIMED
TELEVISION_VISIBLE_HANDOFF=NOT_PHYSICALLY_QUALIFIED
RFB_UNDERLAY_RESTORATION=NOT_RUNTIME_WIRED
PRODUCER_RETIREMENT=NOT_IMPLEMENTED_IN_P5
DECODER_WORKER_SAFE_STOP_ORCHESTRATION=NOT_IMPLEMENTED_IN_P5
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED_IN_P5
VIDEO_SCHEDULER_DROP=NOT_IMPLEMENTED
DECODER_WORKER_TO_COMPOSITOR_HANDOFF=NOT_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
FULL_A004_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Return the baton to Foreman at pre-log P5 authority:

`4367ed3b718c2740d3b3dcefb1ecd24ed93b9556`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- exact RETIRING / REVEAL_PENDING generation fencing;
- drain-before-seal behavior;
- RFB-underlay overlap source shape;
- fail-closed Platform reveal ordering;
- logical commit only after synchronized physical success;
- session-clock stability;
- absence of RFB/Transport/decoder proof fabrication;
- absence of an MPEG_OWNED -> RFB_ONLY direct-stop shortcut;
- final machine evidence.

The Foreman, not this worker, chooses any next packet from returned source.

This Reconstruction worker must not begin scheduler/drop, decoded-frame runtime
handoff, or final Application orchestration in this shift.
