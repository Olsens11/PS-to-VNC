# Foreman shift — accept A004 P4 and activate Q7 retirement P5

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T19:11:52-04:00
COMPLETED_AT=2026-09-20T19:18:02-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a88718833fb15a8c6fdba668daea9c9b78c960d9
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed
`A004-SHARED-COMPOSITOR-FIRST-SYNC-R4` Reconstruction baton, independently
verify the worker's single-GS-owner compositor, physical synchronization
boundary, media-clock/P3 promotion ordering, direct PS2 build coverage and
current-Q7 compatibility, then publish the next bounded owner-correct
retirement/final-visible-handoff packet without executing Reconstruction product
behavior from the Foreman seat.

## Authority refreshed

Live pickup branch authority was:

`a88718833fb15a8c6fdba668daea9c9b78c960d9`

Its parent, the final pre-log P4 source authority, was:

`37bbd71ca5afaf3dfb63568e85f9d31dbf043d08`

The required Reconstruction log was:

`docs/ledge/work-log/20260920T184357-0400__reconstruction__a004-presentation__interactive.md`

Foreman state was revision 0026 with active packet
`A004-SHARED-COMPOSITOR-FIRST-SYNC-R4`.

## Preserved role-error chronology

An immutable docs-only NOOP Foreman log exists at commit:

`2f234d140213ed79d37c6bc2c228f12dfae90fdf`

The Reconstruction conversation had briefly misinterpreted the baton as changing
its permanent role. The user corrected that relay rule before P4 product work
began.

Independent review confirms that erroneous NOOP record:

- changed no Foreman state;
- changed no active packet;
- changed no product source;
- changed no planning authority;
- did not supersede state revision 0026.

The immutable work-log contract requires preserving it as historical evidence
rather than deleting or rewriting it.

The actual P4 Reconstruction range begins after that record.

## Reconstruction range reviewed

P4 worker commits:

1. `75d5c87cd47a3d8b7108d790eed890de4bbaf7b9` —
   `feat(display): add synchronized MPEG compositor`
2. `bbf7e52f45ec577ccd9eeea688cd4023d87d075f` —
   `fix(tooling): restore PS2 compile check executable mode`
3. `25081e22864db4f09c67c10dd7c4662cfa975f50` —
   deterministic dictionary-reconciliation trigger
4. `8f147edc97219df4b59c5118566a377f0a0bd0ab` —
   generated current clean symbol reconciliation
5. `f0e1da106035063c63555f93d2b480c36306128c` —
   synchronized compositor responsibility prose
6. `37bbd71ca5afaf3dfb63568e85f9d31dbf043d08` —
   `fix(display): preserve full inner matte geometry`
7. `a88718833fb15a8c6fdba668daea9c9b78c960d9` —
   required immutable Reconstruction log

The initial P4 write accidentally removed executable mode from
`scripts/check-clean-ps2-compile.sh`; the subsequent tooling-only repair
restored that mode. No C defect was implicated.

The final source correction preserves P1/P3's legal full-inner-matte case:
resolved `inner_content` may be zero width and/or height while base and
suppression remain positive and bounded.

## Independent P4 source review

Foreman independently inspected:

- `src/display/mpeg_frame.{c,h}`;
- `src/display/mpeg_presentation.{c,h}`;
- `src/display/mpeg_compositor.{c,h}`;
- `src/platform/ps2_graphics.{c,h}`;
- `src/media/clock.{c,h}`;
- focused P4 host tests;
- strict PS2 compile manifest;
- linked clean-build manifest;
- current Q7/Q8/Q9 and A003/A004 audit authority.

Confirmed source behavior:

- one neutral decoder-independent RGB16 16x16-macroblock surface carries only
  pointer/capacity/width/height facts;
- the neutral detile path is bounded and deterministic;
- Display owns exact-run lifecycle/clock coordination;
- Platform remains the sole current gsKit/dmaKit owner;
- no second graphics presenter, graphics semaphore, or textual C-file wrapper
  exists in P4;
- Platform owns one bounded reusable MPEG VRAM allocation and one aligned
  maximum-size EE detile buffer;
- the single render path is:
  desktop -> black suppression -> MPEG -> black inner matte -> local overlay ->
  one GS queue -> one synchronized flip;
- ordinary desktop/local-overlay refresh while MPEG remains visible preserves
  the retained MPEG/suppression/matte layers, allowing RFB truth to advance
  underneath;
- Platform reports synchronized MPEG physical completion only after
  `gsKit_sync_flip()`;
- the Platform timer observation is sampled after that synchronized boundary in
  `kBUSCLK` ticks;
- Display rejects stale generation and source/base mismatch before physical
  presentation;
- WAIT_FIRST_FRAME first synchronizes physically, then arms the unarmed
  session-scoped media epoch from the exact post-sync tick, then performs exact
  P3 first-frame promotion;
- an already-armed session epoch survives later MPEG runs unchanged;
- MPEG_OWNED subsequent frames neither re-arm the epoch nor duplicate P3
  promotion;
- failures after physical synchronization preserve truthful effects while the
  coordinator still returns failure rather than falsely claiming a completed
  lifecycle transaction;
- no active direct clear/stop API was added, so current-Q7 retirement remains
  attachable;
- final P4 validation correctly permits zero-sized resolved inner content for a
  completely matted base.

A004-P4-C1 through A004-P4-C12 are independently accepted as MET within the
bounded source/machine-evidence scope.

## P4 pickup-head evidence

GitHub Actions run:

`35543110959` — run number 286

at pickup HEAD
`a88718833fb15a8c6fdba668daea9c9b78c960d9`
completed SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Host logs explicitly report:

- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `transport_runtime_test: PASS`.

The strict R5900 compile log explicitly contains:

- `PS2_COMPILE=src/display/mpeg_frame.c`;
- `PS2_COMPILE=src/display/mpeg_presentation.c`;
- `PS2_COMPILE=src/display/mpeg_compositor.c`;
- `PS2_COMPILE=src/platform/ps2_graphics.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

The canonical linked build explicitly compiles and links:

- `mpeg_frame.o`;
- `mpeg_presentation.o`;
- `mpeg_compositor.o`;
- `ps2_graphics.o`

into both reproducibility builds and reports:

`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`

This closes the previous P3 evidence gap: current P3/P4 presentation mechanism
source is now directly compiled and linked by canonical PS2 jobs.

No physical PS2 or television run occurred. Repository/toolchain evidence does
not qualify actual visible layer order, first-frame display timing, A/V sync or
endurance.

## Next dependency chosen

Current P4 can establish and retain exact-run MPEG visual ownership and can
refresh the RFB desktop underneath that retained visual state.

Current source still intentionally lacks an owner-correct exact-run visible
retirement path.

Current Q7 requires:

1. close new MPEG production/admission;
2. allow already accepted MPEG data to drain;
3. permit RFB restoration underneath still-visible MPEG/mattes;
4. reach the local decoder/runtime safe stop boundary;
5. only then remove MPEG/mattes and reveal the already-refreshing RFB desktop.

Presentation must not manufacture the producer-close, decoder-safe-stop,
Transport residual-finalization, or RFB-current proofs. Those remain later
Application orchestration facts.

The next bounded source seam is therefore Presentation/Platform retirement and
final visible handoff rather than decoder-worker runtime wiring.

That ordering prevents creation of a runtime that can start MPEG safely but has
no bounded owner-correct path to stop it.

Absolute scheduler/drop remains required A004 work but is independent timing
policy and is deliberately deferred to the next later tranche rather than being
bundled into Q7 retirement.

## Foreman state / packet publication

Published Foreman State revision 0027 in commit:

`9f913b52166bbb963554f796c26049902b5bebee`

message:

`docs(foreman): accept A004 P4 and activate Q7 retirement`

State phase:

`A004_P4_INTEGRATED__A004_Q7_RETIREMENT_VISIBLE_HANDOFF_RECONSTRUCTION_ACTIVE__A003_APPLICATION_ORCHESTRATION_DEPENDS_ON_A004_PRESENTATION_SEAMS`

State flags:

- ARCHITECTURE_BLOCKER=NONE
- A004_P1_FOREMAN_ACCEPTED=YES
- A004_P2_FOREMAN_ACCEPTED=YES
- A004_P3_FOREMAN_ACCEPTED=YES
- A004_P4_FOREMAN_ACCEPTED=YES
- A004_P5_ACTIVE=YES
- A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
- HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

Active packet:

`A004-Q7-RETIREMENT-VISIBLE-HANDOFF-R5`

Execution classification remains:

- EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`
- USER_TERMINAL_POLICY=`EXCEPTION_ONLY`
- PI_LOCAL_USER_PROXY_REQUIRED=`NO`

R5 is bounded to:

`MPEG_OWNED -> RETIRING -> REVEAL_PENDING -> RFB_ONLY`

with exact generation fencing, drain-before-seal behavior, continued RFB
underlay refresh, retained MPEG/mattes until final synchronized reveal,
fail-closed Platform removal, and logical snapshot clear only after successful
physical reveal.

R5 explicitly excludes scheduler/drop, decoder-worker runtime handoff, Pi
producer/Transport retirement execution, P2 request orchestration, calibration
runtime wiring, final Application orchestration, Wire-loss restoration policy
and hardware qualification.

## State-publication CI

Exact state commit
`9f913b52166bbb963554f796c26049902b5bebee`
was verified by GitHub Actions run:

`35544148730` — run number 287

Conclusion: SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Thus the published P4 acceptance and R5 packet are repository-coherent on the
exact state authority.

## Checks / evidence for this Foreman shift

REMOTE_AUTHORITY_REFRESH=PASS
P4_ROLE_ERROR_CHRONOLOGY_REVIEW=PASS
A004_P4_SOURCE_REVIEW=PASS
A004_P4_C1_C12_FOREMAN_ACCEPTANCE=PASS
P4_DIRECT_PS2_COMPILE_LINK_EVIDENCE=PASS
CURRENT_Q7_COMPATIBILITY=PASS
NEXT_OWNER_SEAM=Q7_RETIREMENT_VISIBLE_HANDOFF
FOREMAN_STATE_REVISION=0027
FOREMAN_STATE_COMMIT=9f913b52166bbb963554f796c26049902b5bebee
FOREMAN_STATE_CI_RUN=35544148730
FOREMAN_STATE_CI=PASS
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## Evidence gaps

PENDING_LOCAL=NONE_FOR_FOREMAN_PACKET_PUBLICATION
P4_PHYSICAL_PS2_PRESENTATION=NOT_RUN_NOT_CLAIMED
P4_TELEVISION_LAYER_ORDER=NOT_PHYSICALLY_QUALIFIED
P4_FIRST_REAL_MPEG_FRAME_CLOCK_ARM=NOT_PHYSICALLY_QUALIFIED
P4_AUDIO_VIDEO_SYNC=NOT_PHYSICALLY_QUALIFIED
P5_Q7_RETIREMENT=ACTIVE_RECONSTRUCTION_PACKET
FULL_A004_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes
`A004-Q7-RETIREMENT-VISIBLE-HANDOFF-R5`, emits exactly one immutable
`a004-presentation` Reconstruction log, and returns the baton to Foreman.

Do not begin scheduler/drop, decoder-worker handoff or final Application
orchestration in the same worker shift.
