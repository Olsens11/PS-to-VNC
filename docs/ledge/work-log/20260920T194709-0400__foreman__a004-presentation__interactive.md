# Foreman shift — accept A004 P5 and activate absolute scheduler P6

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T19:47:09-04:00
COMPLETED_AT=2026-09-20T19:54:31-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=8ab034addb551899c4600c64063a1e1f7db7e557
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed
`A004-Q7-RETIREMENT-VISIBLE-HANDOFF-R5` Reconstruction baton, independently
verify exact-run retirement/drain/seal/final-reveal behavior and machine
evidence, then publish the next bounded absolute common-clock scheduler/drop
packet without executing Reconstruction product behavior from the Foreman seat.

## Authority refreshed

Live pickup branch authority was:

`8ab034addb551899c4600c64063a1e1f7db7e557`

Final pre-log P5 source/dictionary authority was:

`4367ed3b718c2740d3b3dcefb1ecd24ed93b9556`

The required Reconstruction log was:

`docs/ledge/work-log/20260920T192712-0400__reconstruction__a004-presentation__interactive.md`

Foreman state at pickup was revision 0027 with active packet
`A004-Q7-RETIREMENT-VISIBLE-HANDOFF-R5`.

## Reconstruction range reviewed

P5 worker commits after Foreman base
`3ce6b596a5fc0936c30ab7c31936e1a2c82c4675`:

1. `05e3930a8c82a7f0c4b1080de44972028bb0fd75`
   — `feat(display): add exact-run MPEG retirement reveal`
2. `6480038ae5ec69eae37a2af5e52ecd7972d107fa`
   — deterministic dictionary-reconciliation trigger
3. `3b8b9adc09a17fe373edf35b5416934e95e7cc8a`
   — automation-created current clean dictionary reconciliation
4. `4367ed3b718c2740d3b3dcefb1ecd24ed93b9556`
   — maintained Display/Platform retirement responsibility prose
5. `8ab034addb551899c4600c64063a1e1f7db7e557`
   — required immutable Reconstruction work log

No Application, RFB, Transport, MPEG-decoder, calibration/UI, input, audio,
configuration or framebuffer product source changed.

## Independent P5 source review

Foreman independently inspected:

- `src/display/mpeg_presentation.{c,h}`;
- `src/display/mpeg_compositor.{c,h}`;
- `src/platform/ps2_graphics.{c,h}`;
- focused presentation/compositor host fixtures;
- current Q7/A003/A004 authority;
- exact worker CI chronology.

Confirmed behavior:

- Presentation owns the exact visible sequence
  `RFB_ONLY -> WAIT_FIRST_FRAME -> MPEG_OWNED -> RETIRING ->
  REVEAL_PENDING -> RFB_ONLY`;
- begin-retirement is legal only for the exact nonzero current MPEG_OWNED
  generation;
- RETIRING keeps the immutable generation/geometry snapshot, remains COMPOSITED
  and visually MPEG-owned, and requires no generic P2 global RFB freeze;
- exact RETIRING frames remain presentable so already-accepted data may drain;
- stale/wrong generations fail closed;
- exact seal creates REVEAL_PENDING, retains the snapshot/visual owner and
  fences every further MPEG frame before Platform;
- ordinary desktop presentation can continue updating cached RFB truth while
  retained MPEG/suppression/mattes remain above it;
- Presentation/Display does not manufacture producer-close, decoder-safe-stop,
  Transport residual-finalization or RFB-current facts;
- Presentation/Display contains no RFB request/FULL-debt mechanism;
- Platform remains the sole gsKit/dmaKit owner;
- final reveal draws cached desktop plus current local overlay without retained
  MPEG through the same shared render path and synchronized flip;
- Platform retains `video_visible` and geometry while the no-video render is
  attempted and clears them only after synchronized success;
- reusable MPEG texture/VRAM resources survive successful visible retirement;
- Display validates exact REVEAL_PENDING generation/snapshot before invoking
  Platform and commits logical RFB_ONLY only after synchronized physical reveal;
- retirement reveal has no media-clock parameter and does not arm, clear or
  replace the one session epoch;
- later MPEG activity reuses that existing session epoch;
- no generic `clear_video()` or direct MPEG_OWNED -> RFB_ONLY stop shortcut
  exists.

A004-P5-C1 through A004-P5-C12 are independently accepted as MET within the
packet's bounded source/machine-evidence scope.

## P5 CI chronology

Behavior-head run `35544856315` and reconciliation-trigger run
`35544918534` failed only strict dictionary completeness for the newly
introduced Display/Platform symbols.

On those intermediate heads:

- host-unit was already PASS;
- project-check was already PASS;
- direct PS2 compile was already PASS;
- linked PS2 build was already PASS.

The second run's authorized dictionary-reconcile job succeeded.

After generated dictionary maintenance and responsibility prose landed,
pre-log run `35544987580` completed SUCCESS across all required gates.

Exact pickup-head run:

`35545125944` — run number 292

at
`8ab034addb551899c4600c64063a1e1f7db7e557`
completed SUCCESS.

Observed exact-head jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Exact-head host logs explicitly report:

- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`.

Exact-head strict PS2 compile directly includes current
`mpeg_presentation.c`, `mpeg_compositor.c` and `ps2_graphics.c`, and reports
`CLEAN_PS2_COMPILE_CHECK=PASS`.

Exact-head linked evidence reports:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

No physical PS2/television run occurred or is inferred from these machine gates.

## Next dependency chosen

With P1-P5 accepted, the remaining presentation-policy dependency before
decoded-frame runtime handoff is the qualified absolute common-clock video
scheduler/drop seam.

Recovered frozen timing evidence establishes:

- rational source cadence, historically 30000/1001;
- absolute frame deadlines derived from one common epoch rather than relative
  per-frame sleeps;
- late processing does not rebase later deadlines;
- H1 NONE/ONE_VSYNC/TWO_VSYNC scheduler modes were comparison machinery and are
  not production modes;
- stale presentation dropping must not suppress MPEG decode/reference-state
  advancement;
- P7's isolated presentation-drop experiment omitted only the stale visible draw
  once lateness reached one complete source-frame period, while frames less than
  one period late remained presentable.

Current clean CONFIG intentionally does not expose the H1 FPS/scheduler/drop
laboratory surface. Therefore the next packet defines a narrow owner-specific
Display timing profile but does not widen the production CONFIG wire contract.

P4/P5 also establish a modern multi-run constraint: one session media epoch
survives MPEG retirement and later runs. A later run cannot reset run-local
picture ordinal 1 against the original epoch without making its deadlines
artificially ancient.

The next packet therefore requires:

- first frame remains exclusively P4's physical sync/clock-arm/promotion event;
- scheduler initialization occurs only after that exact first sync and an
  already-armed session clock;
- one immutable per-run additional-tick anchor is derived on the existing
  session clock axis from the first-sync observation;
- the run anchor is not a second epoch and never mutates the common clock;
- subsequent picture deadlines use an absolute non-rebasing grid;
- timing decisions remain pure/inspectable and platform-neutral;
- optional stale drop remains presentation-only;
- decoder/Transport/RFB/lifecycle state is untouched;
- cross-owner runtime wiring remains deferred.

## Foreman state / packet publication

Published Foreman State revision 0028 in commit:

`a09f594035714f7d4e9a7eff04454d8a92d00360`

message:

`docs(foreman): accept A004 P5 and activate absolute scheduler`

State phase:

`A004_P5_INTEGRATED__A004_ABSOLUTE_VIDEO_SCHEDULER_DROP_RECONSTRUCTION_ACTIVE__A003_APPLICATION_ORCHESTRATION_DEPENDS_ON_A004_PRESENTATION_SEAMS`

State flags:

- ARCHITECTURE_BLOCKER=NONE
- A004_P1_FOREMAN_ACCEPTED=YES
- A004_P2_FOREMAN_ACCEPTED=YES
- A004_P3_FOREMAN_ACCEPTED=YES
- A004_P4_FOREMAN_ACCEPTED=YES
- A004_P5_FOREMAN_ACCEPTED=YES
- A004_P6_ACTIVE=YES
- A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
- HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

Active packet:

`A004-ABSOLUTE-VIDEO-SCHEDULER-DROP-R6`

Execution classification:

- EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`
- USER_TERMINAL_POLICY=`EXCEPTION_ONLY`
- PI_LOCAL_USER_PROXY_REQUIRED=`NO`

R6 is bounded to a Display-owned, deterministic, post-first-frame scheduling
policy. It explicitly excludes decoder-worker runtime handoff, actual wait/VBlank
loops, P4/P5 behavior changes, CONFIG protocol expansion, Transport/RFB
orchestration, final Application lifecycle and physical timing qualification.

## State-publication CI

Exact state commit
`a09f594035714f7d4e9a7eff04454d8a92d00360`
was verified by GitHub Actions run:

`35545963404` — run number 293

Conclusion: SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Thus the published P5 acceptance and R6 packet are repository-coherent on the
exact state authority.

## Checks / evidence for this Foreman shift

REMOTE_AUTHORITY_REFRESH=PASS
A004_P5_SOURCE_REVIEW=PASS
A004_P5_C1_C12_FOREMAN_ACCEPTANCE=PASS
P5_MACHINE_EVIDENCE=PASS
P5_INTERMEDIATE_DICTIONARY_FAILURE_CLASSIFICATION=EXPECTED_RECONCILIATION
CURRENT_Q7_COMPATIBILITY=PASS
NEXT_OWNER_SEAM=ABSOLUTE_VIDEO_SCHEDULER_DROP
FOREMAN_STATE_REVISION=0028
FOREMAN_STATE_COMMIT=a09f594035714f7d4e9a7eff04454d8a92d00360
FOREMAN_STATE_CI_RUN=35545963404
FOREMAN_STATE_CI=PASS
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## Evidence gaps

PENDING_LOCAL=NONE_FOR_FOREMAN_PACKET_PUBLICATION
P5_PHYSICAL_PS2_REVEAL=NOT_RUN_NOT_CLAIMED
P5_TELEVISION_VISIBLE_HANDOFF=NOT_PHYSICALLY_QUALIFIED
P5_RFB_UNDERLAY_RUNTIME_RESTORATION=NOT_WIRED
P6_VIDEO_SCHEDULER_DROP=ACTIVE_RECONSTRUCTION_PACKET
DECODER_WORKER_TO_PRESENTATION_HANDOFF=NOT_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
FULL_A004_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes
`A004-ABSOLUTE-VIDEO-SCHEDULER-DROP-R6`, emits exactly one immutable
`a004-presentation` Reconstruction log, and returns the baton to Foreman.

Do not begin decoder-worker runtime handoff or final Application orchestration in
the same worker shift.
