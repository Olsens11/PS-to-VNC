# Foreman shift — accept A004 P6 and activate A003 decoded-picture handoff

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T20:22:27-04:00
COMPLETED_AT=2026-09-20T20:31:50-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=352bf76f0535025e925b66a5e167bbf2e0e07d5c
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed A004 absolute-video-scheduler R6 baton, independently
verify its timing/owner/build claims, reconcile the duplicate-looking worker-log
chronology caused by the worker timeout/restart, and publish the smallest
owner-correct dependency needed before decoder-worker-to-Presentation runtime
wiring.

## Authority refreshed

Live pickup branch authority was:

`352bf76f0535025e925b66a5e167bbf2e0e07d5c`

Current Foreman State was revision 0028 with active packet:

`A004-ABSOLUTE-VIDEO-SCHEDULER-DROP-R6`

Final substantive R6 source/dictionary authority was:

`65fec4980fb1362716b5bc3ef6f1b73fdafacaf0`

The completed R6 Reconstruction log was committed at:

`64f74f12f2385e6595161c3f3f21cbd2e12a3525`

path:

`docs/ledge/work-log/20260920T195955-0400__reconstruction__a004-presentation__interactive.md`

A later immutable Reconstruction NOOP log was committed at:

`352bf76f0535025e925b66a5e167bbf2e0e07d5c`

path:

`docs/ledge/work-log/20260920T202020-0400__reconstruction__a004-presentation__interactive.md`

## Timeout/restart chronology

The user clarified during this Foreman round that the original Reconstruction
worker timed out and had to be restarted only to confirm that it had updated the
completed R6 log.

Independent repository review is consistent with that explanation.

The later NOOP record:

- starts from the already-completed R6 log commit;
- reports the active Foreman packet as already completed;
- changes no product source;
- changes no tests;
- changes no dictionaries or build manifests;
- changes no Foreman state;
- publishes no new packet;
- explicitly refuses to duplicate R6 or begin later runtime work.

It is therefore preserved as truthful timeout/restart confirmation chronology,
not treated as a second engineering execution or competing R6 result.

## R6 substantive commit range reviewed

After Foreman base
`1845f06c67b95c131f1824920781379fc6a69cae`:

1. `02b71b0c1edd878faeafccc239592de8d27289d9`
   — `feat(display): add absolute MPEG presentation scheduler`
2. `f356aee132e52ee3a6cc47e510a615c42c1eee98`
   — tree-identical noncanonical reconciliation trigger
3. `b5e16bb22281ffd750fcdc573d3ebc8fb36b76bf`
   — canonical deterministic dictionary-reconciliation trigger
4. `66e312997ae7495a0a83926d141dfd3ae952bffa`
   — automated current clean dictionary reconciliation
5. `65fec4980fb1362716b5bc3ef6f1b73fdafacaf0`
   — maintained Display/Media responsibility prose
6. `64f74f12f2385e6595161c3f3f21cbd2e12a3525`
   — completed R6 Reconstruction log
7. `352bf76f0535025e925b66a5e167bbf2e0e07d5c`
   — timeout/restart confirmation NOOP log only

## Independent R6 source review

Foreman independently inspected:

- `src/display/mpeg_scheduler.{c,h}`;
- `src/media/clock.{c,h}`;
- `tests/unit/mpeg_scheduler_test.c`;
- host registration;
- canonical strict PS2 compile registration;
- linked clean-build registration;
- current P3/P4/P5 presentation owners;
- frozen H1/P6/P7 timing evidence and current A002/A003 authority.

Confirmed source behavior:

- timing policy is owned by Display and imports only the common Media clock seam;
- the immutable scheduler profile contains rational FPS numerator/denominator
  plus explicit optional drop enable/threshold facts;
- H1 scheduler comparison modes were not restored;
- current production CONFIG was not widened;
- scheduler initialization requires a nonzero caller-owned generation and an
  already-armed session clock;
- scheduler initialization never calls `pstvnc_media_clock_arm()`;
- picture ordinal 1 is rejected, preserving P4 as the sole first-frame physical
  presentation, clock-arm and P3-promotion authority;
- one immutable per-run placement is derived as
  `max(first_sync_tick - video_deadline_zero, 0)` on the existing session
  video timeline;
- a later MPEG run therefore reuses the existing session epoch instead of
  minting a second clock epoch;
- subsequent picture ordinal N uses an absolute non-rebasing grid based on
  `run_base + (N - 1) * frame_period`;
- decision logic is pure and receives current time as injected data;
- no scheduler wait, sleep, VBlank, Platform or GS operation exists;
- early frames classify WAIT_UNTIL_DEADLINE;
- on-time and below-threshold late frames classify PRESENT_NOW;
- enabled threshold-late frames classify DROP_PRESENTATION_ONLY;
- drop-disabled valid frames never classify DROP regardless of lateness;
- DROP_PRESENTATION_ONLY has no decoder, Transport, EOF, generation-lifecycle or
  visible-ownership side effect;
- stale/wrong generation is rejected and no generation is minted/incremented;
- arithmetic is monotonic/saturating for threshold, ordinal and deadline inputs;
- the only Media API addition is read-only
  `pstvnc_media_clock_tick_rate()`;
- no decoder/compositor/P5/RFB/Transport/Application runtime caller was wired to
  the scheduler.

A004-P6-C1 through A004-P6-C12 are independently accepted as MET within the
bounded source/machine-evidence scope.

## R6 machine-evidence chronology

Behavior run `35546545543` passed host-unit, PS2 compile and PS2 link while
project/dictionary checks correctly reported missing new scheduler
dictionary/topology coverage.

The first tree-identical trigger run `35546616557` remained red in the same
dictionary/topology gates because its noncanonical message did not satisfy the
authorized reconciliation predicate.

Canonical trigger run `35546710602` successfully executed dictionary
reconciliation; same-run ordinary project/dictionary jobs still observed the
pre-reconciliation trigger tree.

Coherent final source run `35546830313`, completed-log run
`35546948240` (#299), and timeout/restart NOOP-head run `35547407928`
(#300) completed SUCCESS.

Run #299 explicitly reported:

- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- direct `PS2_COMPILE=src/media/clock.c`;
- direct `PS2_COMPILE=src/display/mpeg_scheduler.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

No physical PS2 timing/drop/A-V qualification was performed or inferred.

## Next dependency found from actual source

The initially expected next stage was decoded-frame runtime handoff from an MPEG
worker through R6 to the P4 compositor.

Current source reveals one smaller prerequisite.

`pstvnc_mpeg_decoder_run()` privately owns the repeated
`platform_ops.picture()` loop. After each successful picture it validates
decoder/feed state, increments `pictures_decoded`, observes the qualified safe
stop boundary, then immediately enters the next picture call.

No public MPEG-domain result currently exposes one successful decoded picture
boundary. In particular, there is no bounded caller-visible value containing:

- the just-decoded reusable picture buffer;
- validated sequence dimensions;
- pixel-width fact;
- a decoder-local picture ordinal;
- an explicit lifetime fence before that buffer is reused.

A direct worker-to-scheduler/compositor reconstruction at this point would
therefore require either:

- reaching into decoder-owned mutable internals while the decoder may run ahead;
  or
- duplicating the qualified feed/sequence/safe-stop loop outside the MPEG owner.

Neither is acceptable.

The next packet therefore reconstructs the MPEG-owned one-picture step /
borrowed decoded-picture publication seam first.

## Safe-stop authority preserved into the next packet

Frozen hardware-qualified safe-stop evidence remains binding:

- local stop is not stream EOF;
- an active `MPEG_Picture()` continues receiving real Transport data;
- stop is observed only after decoder-call ownership returns to project code;
- decoder release cannot occur while the picture call is active.

The next seam must expose decoded ordinal 1 neutrally without presenting it or
arming timing state and must preserve existing terminal classifications and
`pstvnc_mpeg_decoder_run()` compatibility.

Generation identity remains outside the decoder. Application/run authority will
later pair exact generation with the MPEG-owned picture value.

## Foreman state / packet publication

Published Foreman State revision 0029 in commit:

`c380790343d970ef8fe5237b82eaeea1922887e7`

message:

`docs(foreman): accept A004 P6 and activate decoder picture handoff`

State phase:

`A004_P6_INTEGRATED__A003_DECODED_PICTURE_STEP_RECONSTRUCTION_ACTIVE__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

State flags:

- ARCHITECTURE_BLOCKER=NONE
- A004_P1_FOREMAN_ACCEPTED=YES
- A004_P2_FOREMAN_ACCEPTED=YES
- A004_P3_FOREMAN_ACCEPTED=YES
- A004_P4_FOREMAN_ACCEPTED=YES
- A004_P5_FOREMAN_ACCEPTED=YES
- A004_P6_FOREMAN_ACCEPTED=YES
- A003_DECODED_PICTURE_STEP_ACTIVE=YES
- A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
- HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

Active packet:

`A003-DECODED-PICTURE-HANDOFF-R2`

Execution classification remains:

- EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`
- USER_TERMINAL_POLICY=`EXCEPTION_ONLY`
- PI_LOCAL_USER_PROXY_REQUIRED=`NO`

R2 is bounded to the decoder-owned one-picture step and borrowed-frame lifetime
contract. It explicitly excludes the MPEG worker/thread wrapper, worker/main
mailbox, scheduler/compositor calls, concrete platform adapter, START/RETIRE,
Pi producer lifecycle, RFB restoration and final Application orchestration.

## State-publication CI and known Transport test signal

State commit
`c380790343d970ef8fe5237b82eaeea1922887e7`
triggered run:

`35547829480` — run #301.

The state commit changed documentation/state only; product source was identical
to the already-green #300 authority.

Attempt 1:
- project-check PASS;
- dictionary-long PASS;
- ps2-compile PASS;
- ps2-link PASS;
- host-unit FAIL.

The host failure was entirely in the pre-existing
`transport_runtime_test`, beginning with the previously observed timing-shaped
assertions at lines 787/789 and cascading through quiesce assertions. No
Transport source changed.

Foreman re-ran the failed workflow jobs without changing source.

Attempt 2 again failed only host-unit and narrowed to exactly the familiar two
assertions:

- line 787: RFB channel available count;
- line 789: activity sequence advancement.

All other gates remained green.

Foreman then re-ran that host job alone, still without changing source.

Attempt 3 completed SUCCESS. GitHub therefore records run #301 overall
SUCCESS on the exact state commit.

This chronology reinforces the existing classification: the Transport host
fixture contains a nondeterministic/timing-sensitive test signal. It is not
evidence of an A004 R6 or revision-0029 product regression, but the failed
attempts remain preserved rather than hidden.

## Checks / evidence for this Foreman shift

REMOTE_AUTHORITY_REFRESH=PASS
R6_TIMEOUT_RESTART_NOOP_CLASSIFICATION=PASS
A004_P6_SOURCE_REVIEW=PASS
A004_P6_C1_C12_FOREMAN_ACCEPTANCE=PASS
R6_DIRECT_PS2_COMPILE_LINK_EVIDENCE=PASS
R6_NO_RUNTIME_WIRING=PASS
NEXT_OWNER_SEAM=A003_DECODED_PICTURE_STEP
FOREMAN_STATE_REVISION=0029
FOREMAN_STATE_COMMIT=c380790343d970ef8fe5237b82eaeea1922887e7
FOREMAN_STATE_CI_RUN=35547829480
FOREMAN_STATE_CI_ATTEMPTS=3
FOREMAN_STATE_CI_FINAL=PASS
TRANSPORT_RUNTIME_TEST_FLAKE=OBSERVED_UNCHANGED_SOURCE
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## Evidence gaps

PENDING_LOCAL=NONE_FOR_FOREMAN_PACKET_PUBLICATION
R6_PHYSICAL_PS2_VIDEO_TIMING=NOT_RUN_NOT_CLAIMED
R6_PHYSICAL_STALE_FRAME_DROP=NOT_RUN_NOT_CLAIMED
R6_PHYSICAL_AV_SYNC=NOT_RUN_NOT_CLAIMED
A003_DECODED_PICTURE_STEP=ACTIVE_RECONSTRUCTION_PACKET
MPEG_WORKER_MAIN_THREAD_HANDOFF=NOT_IMPLEMENTED
CONCRETE_PS2_LIBMPEG_ADAPTER=NOT_CONFIRMED_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
FULL_A004_PRODUCT_RUNTIME=NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes
`A003-DECODED-PICTURE-HANDOFF-R2`, emits exactly one immutable
`a003-mpeg-generation` Reconstruction log, and returns the baton to Foreman.

Do not begin the MPEG worker/mailbox, Presentation runtime wiring, concrete
platform adapter or final Application orchestration in the same shift.
