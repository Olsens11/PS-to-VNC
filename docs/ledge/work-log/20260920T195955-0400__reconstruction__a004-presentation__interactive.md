# Reconstruction shift — A004 absolute video scheduler/drop R6

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T19:59:55-04:00
COMPLETED_AT=2026-09-20T20:11:33-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=1845f06c67b95c131f1824920781379fc6a69cae
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A004-ABSOLUTE-VIDEO-SCHEDULER-DROP-R6`

The bounded objective was to reconstruct a Display-owned deterministic MPEG
presentation scheduler for pictures after P4's already-physical first frame.

R6 establishes:

- one exact caller-generation scheduler instance;
- one narrow rational source-rate/drop profile;
- one immutable per-run anchor on the already-existing session video timeline;
- one absolute non-rebasing picture-deadline grid;
- pure WAIT / PRESENT_NOW / DROP_PRESENTATION_ONLY classification.

R6 deliberately does not wire the decoder worker, compositor, P5 retirement,
Transport, RFB, calibration or Application to the scheduler and performs no
physical wait/presentation itself.

## Authority consumed

Consumed current authority at wake:

- Reconstruction Contract revision 0006;
- Foreman State revision 0028;
- Foreman work log
  `20260920T194709-0400__foreman__a004-presentation__interactive.md`;
- A002 CONFIG/audio/common-clock audit revision 0001;
- A003 MPEG-generation audit revision 0001;
- A004 presentation/calibration audit revision 0001;
- Wire runtime decisions revision 0011;
- current P4/P5 Presentation source;
- current clean media-clock and CONFIG source;
- canonical host, project, strict-dictionary, PS2 compile and linked-build
  manifests.

Starting live branch authority was:

`1845f06c67b95c131f1824920781379fc6a69cae`

Execution classification remained:

- EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`
- USER_TERMINAL_POLICY=`EXCEPTION_ONLY`
- PI_LOCAL_USER_PROXY_REQUIRED=`NO`

No user terminal or physical-PS2 action was required or requested.

## Frozen timing evidence inspected

Frozen authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Inspected at minimum:

- `experiments/media-harness-h1/h1_video_runtime.c`;
- `experiments/media-harness-h1/h1_video_runtime.h`;
- `experiments/media-harness-h1/h1_media_clock.c`;
- `experiments/media-harness-h1/h1_media_clock.h`;
- `experiments/media-harness-h1/h1_profiles.py`;
- `experiments/media-harness-h1/qualified_p11_video_seed.c`;
- `experiments/media-stream-exp3/generate_network_live_streaming_p7_late_frame_drop.py`.

Recovered binding timing facts:

1. H1 absolute frame period was:

   `floor(timer_ticks_per_second * fps_denominator / fps_numerator)`.

2. The qualified reference profile used:

   - FPS numerator = 30000;
   - FPS denominator = 1001;
   - absolute scheduling.

3. Historical NONE / ONE_VSYNC / TWO_VSYNC modes were comparison machinery,
   not product modes to reconstruct.

4. Picture N used an absolute source-grid offset equivalent to:

   `(N - 1) * frame_period_ticks`

   from one established timing origin.

5. A late picture did not rebase the next picture deadline.

6. H1 represented stale threshold in milliframes:

   `floor(frame_period_ticks * threshold_milliframes / 1000)`.

7. The isolated P7 late-frame-drop experiment preserved MPEG decode/reference
   progress and omitted only the stale visible draw when lateness reached one
   complete source-frame period.

8. A002/A003 authority preserves one common session epoch. The qualified first
   physical video presentation owns initial clock arming.

Current P4/P5 source adds the modern multi-run requirement: later MPEG runs
reuse the already-armed session clock instead of minting a new per-run epoch.

## Clean owner and timing profile

Added:

- `src/display/mpeg_scheduler.h`;
- `src/display/mpeg_scheduler.c`.

The scheduler is a platform-neutral Display timing-policy owner.

Its narrow immutable profile is:

- `fps_numerator`;
- `fps_denominator`;
- `drop_enabled`;
- `drop_threshold_milliframes`.

No H1 scheduler-mode enum was restored.

No production CONFIG field, profile field, decoder contract, Transport wire
field or Application lifecycle surface was widened.

Tests use 30000/1001 and 1000 milliframe as historically qualified/reference
values only; R6 does not declare them new public user knobs.

## Per-run scheduler state

One initialized scheduler retains:

- pointer to the existing session media clock;
- copied timing profile;
- exact caller-owned nonzero run generation;
- calculated frame period;
- immutable run-base additional-tick offset;
- calculated stale threshold in ticks;
- initialized fact.

The scheduler does not contain:

- MPEG decoded buffers/surfaces;
- Presentation ownership state;
- P5 retirement state;
- Transport/RFB state;
- GS resources;
- EOF or decoder lifecycle state.

Generation is copied verbatim. R6 does not allocate, increment or otherwise mint
run identity.

## First-frame / common-clock boundary

First frame remains exclusively P4 authority.

`pstvnc_mpeg_scheduler_init_after_first_sync()`:

- requires a nonzero caller generation;
- requires the existing session clock already to be armed;
- consumes the exact caller-supplied P4 first synchronized presentation tick;
- never calls `pstvnc_media_clock_arm()`;
- does not present or schedule ordinal 1.

`pstvnc_mpeg_scheduler_decide()` rejects every picture ordinal below 2.

The scheduler therefore cannot delay, drop, re-present or establish the first
MPEG frame.

## Session-axis run-anchor rule

R6 uses the Foreman-authorized multi-run placement rule.

At initialization it asks the common media clock for the video deadline at:

`additional_ticks = 0`

Then it retains exactly once:

`run_base_additional_ticks = max(first_sync_tick - video_deadline_zero, 0)`

This value is only a run-local placement offset on the existing session timeline.

It is not an epoch, is never published back into the media clock, and is never
recomputed from later pictures.

A later run whose first physical sync occurs far after the original session
epoch therefore starts its subsequent picture grid near that later physical
boundary without resetting the session clock.

Focused tests explicitly prove the session epoch is unchanged while separate
later-run scheduler instances acquire different immutable run-base offsets.

## Media-clock seam

Added only the smallest read-only common-clock query:

`pstvnc_media_clock_tick_rate()`

to:

- `src/media/clock.h`;
- `src/media/clock.c`.

The query exposes the immutable session tick rate already established at clock
initialization. It does not expose or mutate epoch/armed internals.

No existing clock arm/deadline/wait semantics changed.

The scheduler continues using the existing public
`pstvnc_media_clock_video_deadline()` operation for final absolute deadlines.

## Frame-period / deadline arithmetic

Frame period:

`floor(ticks_per_second * fps_denominator / fps_numerator)`

The operands are uint32 timing facts, so their product fits within uint64.
A result truncating to zero is rejected as an impossible scheduling profile.

For run-local picture ordinal N where N >= 2:

`ordinal_offset = saturating((N - 1) * frame_period_ticks)`

`additional_ticks = saturating(run_base_additional_ticks + ordinal_offset)`

The final deadline is then requested from the common media clock using that
absolute additional-tick value.

No prior decision result feeds the next deadline. Late presentation therefore
cannot rebase or drift the source grid.

Ordinal multiplication/addition are monotonic and saturating. A far-future
ordinal reaches UINT64_MAX rather than wrapping into an early deadline.

## Drop-threshold arithmetic

When drop policy is enabled:

`drop_threshold_ticks =
 floor(frame_period_ticks * drop_threshold_milliframes / 1000)`

The helper performs quotient/remainder multiply-divide with saturation so a
large threshold cannot wrap.

R6 rejects:

- zero FPS numerator;
- zero FPS denominator;
- frame period truncating to zero;
- non-boolean drop-enabled values;
- enabled drop with zero threshold;
- disabled drop with a nonzero threshold;
- enabled threshold that converts to zero ticks.

## Pure decision semantics

`pstvnc_mpeg_scheduler_decide()` accepts only:

- the initialized scheduler;
- the exact copied nonzero run generation;
- picture ordinal N >= 2;
- caller/injected current tick;
- output result storage.

It produces:

- absolute deadline tick;
- lateness when at/after deadline;
- one decision:
  - WAIT_UNTIL_DEADLINE;
  - PRESENT_NOW;
  - DROP_PRESENTATION_ONLY.

Decision rules:

- current < deadline -> WAIT_UNTIL_DEADLINE;
- current == deadline -> PRESENT_NOW;
- late but below enabled stale threshold -> PRESENT_NOW;
- drop enabled and lateness >= threshold -> DROP_PRESENTATION_ONLY;
- drop disabled -> PRESENT_NOW for every valid late frame.

The decision operation is const/pure with respect to scheduler and media-clock
state. Focused tests compare the scheduler byte-for-byte before/after decisions.

The module contains no sleep, VBlank wait, GS presentation or timer-reading API.
The current tick is injected data.

## Presentation-only stale drop

DROP_PRESENTATION_ONLY is deliberately a policy result, not an MPEG lifecycle
operation.

R6:

- does not mutate decoder/reference state;
- does not undo or suppress Transport consumption/credit;
- does not alter EOF semantics;
- does not change run generation;
- does not alter P3/P5 Presentation ownership or retirement;
- does not invoke Platform/GS.

A later runtime seam may use this result to omit the visible compositor call
while preserving already-completed decoder/Transport progress.

That runtime wiring is explicitly not part of R6.

## No runtime wiring

Compared with Foreman base, no current runtime caller was modified to invoke the
scheduler.

Static source review confirms `mpeg_scheduler.{c,h}` has only the project
dependency:

`media/clock.h`

and contains no:

- gsKit/dmaKit or PS2 graphics dependency;
- MPEG decoder dependency;
- Transport dependency;
- RFB dependency;
- Application dependency;
- wait/sleep/VBlank operation;
- media-clock arm call.

No P4 compositor or P5 retirement source changed.

## Focused host contract

Added:

`tests/unit/mpeg_scheduler_test.c`

and registered it as `mpeg-scheduler-unit`.

Focused coverage proves:

- unarmed-clock initialization rejected;
- ordinal 1 rejected;
- wrong/stale generation rejected;
- 30000/1001 at 90000 ticks/sec -> 3003 ticks/frame;
- early picture -> WAIT;
- exact-deadline picture -> PRESENT_NOW;
- slightly late picture -> PRESENT_NOW;
- exactly one-frame late at 1000 milliframe threshold -> DROP_PRESENTATION_ONLY;
- decisions do not mutate scheduler state;
- a late picture does not rebase later ordinal deadlines;
- a later run receives an immutable nonzero session-axis offset;
- session media epoch remains unchanged across later-run scheduler initialization;
- drop-disabled valid frames never drop, even at extreme lateness;
- invalid rate/drop profiles fail closed;
- frame periods truncating to zero fail closed;
- extreme threshold/ordinal arithmetic saturates without wrap.

The focused CI log reports:

`MPEG_SCHEDULER_TEST=PASS`

## Build / integration

Direct build registration added the new product translation unit to:

- `scripts/check-clean-ps2-compile.sh`;
- `mk/issue7-clean.mk`.

The strict PS2 compile directly compiles:

`src/display/mpeg_scheduler.c`

and the linked clean build includes:

`mpeg_scheduler.o`.

The compile script's executable file mode was preserved.

## Dictionary reconciliation

The first tree-identical trigger used the noncanonical message:

`tooling(symbols): run deterministic scheduler reconciliation`

at commit:

`f356aee132e52ee3a6cc47e510a615c42c1eee98`

The workflow's exact message predicate correctly skipped reconciliation. This
changed no source/tree content.

After inspecting the workflow condition, the canonical tree-identical trigger:

`b5e16bb22281ffd750fcdc573d3ebc8fb36b76bf`

used exactly:

`tooling(symbols): run deterministic dictionary reconciliation`

Automation then committed:

`66e312997ae7495a0a83926d141dfd3ae952bffa`

message:

`docs(symbols): reconcile current clean definitions`

Reconciliation reported:

- Display: removed=0, added=84;
- Media: removed=0, added=6;
- all other maintained reconciliation domains: removed=0, added=0;
- total removed=0;
- total added=90;
- strict dictionary check PASS.

Maintained responsibility prose was then updated in:

`65fec4980fb1362716b5bc3ef6f1b73fdafacaf0`

to describe:

- Display ownership of post-first-frame absolute MPEG timing and
  presentation-only stale-drop policy;
- Media ownership of immutable timer-rate observation alongside the existing
  session epoch/deadline service.

## Worker commit range

R6 commits after Foreman base
`1845f06c67b95c131f1824920781379fc6a69cae`:

1. `02b71b0c1edd878faeafccc239592de8d27289d9`
   — `feat(display): add absolute MPEG presentation scheduler`
2. `f356aee132e52ee3a6cc47e510a615c42c1eee98`
   — tree-identical noncanonical reconciliation trigger; reconciliation skipped
3. `b5e16bb22281ffd750fcdc573d3ebc8fb36b76bf`
   — canonical tree-identical deterministic dictionary-reconciliation trigger
4. `66e312997ae7495a0a83926d141dfd3ae952bffa`
   — automation-created current clean dictionary reconciliation
5. `65fec4980fb1362716b5bc3ef6f1b73fdafacaf0`
   — maintained scheduler/media responsibility prose

The noncanonical trigger is preserved as immutable repository history; it
introduced no product or tree change.

## Exact final changed paths before this log

Relative to Foreman base, final pre-log authority changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `src/display/SYMBOLS.md`;
- `src/display/mpeg_scheduler.c`;
- `src/display/mpeg_scheduler.h`;
- `src/media/SYMBOLS.md`;
- `src/media/clock.c`;
- `src/media/clock.h`;
- `tests/Makefile`;
- `tests/unit/mpeg_scheduler_test.c`.

No CONFIG product source changed.

No compositor, presentation-ownership, Platform graphics, MPEG decoder,
Transport, RFB, Application, UI/calibration, input, audio, framebuffer or main
product source changed.

## Final exact-source evidence

Final pre-log source/dictionary authority:

`65fec4980fb1362716b5bc3ef6f1b73fdafacaf0`

GitHub Actions run:

`35546830313`

completed SUCCESS.

Observed final jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on the coherent ordinary commit.

Host logs explicitly report:

- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`.

Project check explicitly reports:

- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

Direct PS2 compile explicitly includes:

- `src/media/clock.c`;
- `src/display/mpeg_scheduler.c`;

and reports:

`CLEAN_PS2_COMPILE_CHECK=PASS`.

The linked clean build explicitly compiles/links both
`media_clock.o` and `mpeg_scheduler.o` in both reproducibility builds and
reports:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

These are repository/source/machine-build results only. They do not constitute
physical PS2 timing qualification.

## A004 P6 worker disposition

These dispositions are Reconstruction-worker results for independent Foreman
review.

### A004-P6-C1 OWNER_BOUNDARY — MET

The policy lives in Display and depends only on the common Media clock seam. It
owns no GS, decoder, Transport, RFB, calibration or Application lifecycle state.

### A004-P6-C2 NARROW_PROFILE — MET

The profile contains only rational FPS plus optional drop enable/threshold
facts. No scheduler-mode enum or CONFIG-wire expansion exists.

### A004-P6-C3 FIRST_FRAME_BOUNDARY — MET

Initialization requires an already-armed common clock plus the caller's exact
first-sync tick, and ordinal 1 is rejected from decision service. R6 never arms
the clock or presents first frame.

### A004-P6-C4 SESSION_AXIS — MET

Each run derives one immutable
`max(first_sync - session_video_deadline_zero, 0)` placement offset. Tests
prove later runs reuse the same session epoch without re-arm/replacement.

### A004-P6-C5 ABSOLUTE_GRID — MET

Ordinal N >= 2 uses immutable run base plus `(N-1)*period` and existing common
video-deadline authority. A late decision cannot alter later deadlines.

### A004-P6-C6 DECISION_SEMANTICS — MET

Focused tests cover WAIT, on-time PRESENT_NOW, slightly-late PRESENT_NOW,
deadline/lateness reporting and deterministic pure inspection.

### A004-P6-C7 PRESENTATION_ONLY_DROP — MET

Threshold-late classification produces only
DROP_PRESENTATION_ONLY. Scheduler source has no decoder, Transport, EOF,
generation-lifecycle, P5 retirement or physical presentation mutation path.

### A004-P6-C8 DROP_DISABLED — MET

With drop disabled, valid exact-run frames remain PRESENT_NOW regardless of
lateness.

### A004-P6-C9 EXACT_RUN_FENCE — MET

Scheduler copies a nonzero caller generation verbatim, never increments/mints
identity and rejects stale/wrong generation decisions.

### A004-P6-C10 ARITHMETIC_SAFETY — MET

Zero/impossible rate/threshold values are rejected. Ordinal, threshold and
deadline inputs use bounded/saturating arithmetic; extreme cases cannot wrap
into an early deadline.

### A004-P6-C11 NO_RUNTIME_WIRING — MET

No current compositor, P5 retirement, decoder, RFB, Transport or Application
product caller was wired to the scheduler. R6 remains a policy seam plus focused
tests/build registration.

### A004-P6-C12 CLEAN_EVIDENCE — MET

Focused/full host, project check, strict dictionary audit, direct PS2 compile
and linked current-source reproducibility all pass on the exact final pre-log
authority.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A004_P6_REQUIRED_REPOSITORY_MACHINE_GATES
PHYSICAL_PS2_VIDEO_TIMING=NOT_RUN_NOT_CLAIMED
PHYSICAL_AV_SYNC=NOT_RUN_NOT_CLAIMED
PHYSICAL_DROP_BEHAVIOR=NOT_RUN_NOT_CLAIMED
DECODER_WORKER_TO_SCHEDULER_HANDOFF=NOT_IMPLEMENTED
SCHEDULER_TO_COMPOSITOR_RUNTIME_WIRING=NOT_IMPLEMENTED
ACTUAL_WAIT_VBLANK_LOOP=NOT_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
FULL_A004_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Return the baton to Foreman at pre-log R6 authority:

`65fec4980fb1362716b5bc3ef6f1b73fdafacaf0`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- first-frame P4 ownership remains exclusive;
- one-session-axis run-anchor behavior;
- rational frame-period and absolute non-rebasing grid;
- presentation-only stale-drop semantics;
- exact-generation fencing;
- drop-disabled behavior;
- arithmetic saturation;
- common clock never re-armed/replaced;
- no runtime wiring;
- exact CI/build evidence.

The Foreman, not this worker, chooses the next decoded-frame runtime-handoff
packet from returned source.

This worker must not begin decoded-frame handoff or final Application
orchestration in this shift.
