# Foreman shift — accept A004 P2 and activate presentation ownership P3

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T18:05:54-04:00
COMPLETED_AT=2026-09-20T18:09:48-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=773cdd905cde5a0c38a1e732c10d4d5f2d123c46
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed `A004-RFB-FREEZE-REFRESH-R2` Reconstruction baton,
independently verify the worker's source/tests/CI and current-Q7 compatibility,
accept or reject P2 criteria, and publish the next bounded A004 Reconstruction
packet without performing product behavior from the Foreman seat.

## Authority refreshed

Live branch authority at Foreman pickup was:

`773cdd905cde5a0c38a1e732c10d4d5f2d123c46`

The branch was five commits ahead of prior Foreman authority
`aaa452899780116e0bc82d3de5d2975799eb94aa`.

Newest relevant Reconstruction record:

`docs/ledge/work-log/20260920T175245-0400__reconstruction__a004-presentation__interactive.md`

Current Foreman state remained revision 0024; the worker correctly did not
self-accept the packet or publish the next packet.

## Worker commit review

Reviewed the exact worker range:

- `7e779fdb113ee37dfda1e60f4887c7d7c608037a` —
  `feat(rfb): add generic freeze refresh flow policy`;
- `7600d77f158892d1b850c686371cb3c9e72d67b1` —
  tree-identical deterministic symbol-reconciliation trigger;
- `a64b6427356ff1c2fcbc87f2e40cd126048b7589` —
  automated dictionary reconciliation;
- `5aa84f1dc0e3017b88214e20637ab1a0605cc38a` —
  RFB domain responsibility prose;
- `773cdd905cde5a0c38a1e732c10d4d5f2d123c46` —
  immutable Reconstruction work log.

Final packet paths are limited to:

- `src/rfb/flow_policy.c`
- `src/rfb/flow_policy.h`
- `src/rfb/SYMBOLS.md`
- `tests/unit/rfb_flow_policy_test.c`
- `tests/Makefile`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`
- the immutable worker log.

No existing RFB session/parser/bridge, Application, Transport, MPEG, UI,
Display, Platform, framebuffer, input or media source changed.

## Independent P2 behavior review

Foreman independently inspected the final policy source/header, focused host
test, commit diffs, frozen H1 gate/schedule/flow evidence and final workflow
logs.

Confirmed:

- the clean owner is generic RFB flow policy rather than MPEG/calibration state;
- only frozen state, one outstanding request, one pending FULL debt, request
  choice and remote-publication permission are owned;
- no second parser/quiesce/safe-message boundary was created;
- an in-flight request remains outstanding across freeze and can complete at the
  existing RFB message boundary while publication remains blocked;
- request completion clears one outstanding fact exactly once;
- no new request is selected while frozen or while a request is outstanding;
- frozen->thawed creates one FULL debt;
- request inspection does not mutate or consume FULL debt;
- wrong-kind/invalid accounting cannot silently consume FULL debt;
- a successfully recorded FULL send advances recovery and permits a later
  genuine freeze/thaw interval to create distinct debt even if the earlier FULL
  response remains in flight;
- repeated restoration intent before recovery advances remains coalesced;
- initial authoritative RFB frame semantics remain untouched;
- nothing in the policy equates active/retiring MPEG with global RFB freeze.

The deliberate hindsight repair versus H1 is accepted: H1 consumed the pending
FULL obligation while deciding what request should be sent, whereas the clean
policy consumes it only after the currently owed FULL send is recorded as
successful. This is fail-closed and required by the active packet rather than an
unrelated behavioral rewrite.

A004-P2-C1 through A004-P2-C9 are Foreman accepted as MET.

## Independent CI evidence

Final pickup-head workflow:

- GitHub Actions run `35540415181`, run number 271;
- head `773cdd905cde5a0c38a1e732c10d4d5f2d123c46`;
- overall conclusion SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on coherent authority.

Host logs explicitly show:

- `RFB_FLOW_POLICY_TEST=PASS`;
- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- the remainder of the canonical host suite passing.

No physical PS2 run was performed. P2 is not yet wired into live Application
runtime, so broad repository PS2 build success is not promoted into runtime or
hardware proof for the new policy.

## Non-blocking host-test continuity finding

The worker preserved another behavior-head occurrence of the previously seen
timing-sensitive `transport_runtime_test.c` assertions around lines 787/789.

This packet did not touch Transport source. The starting authority was green,
the later reconciliation workflow passed unchanged Transport source, and the
final coherent head passed unchanged Transport source.

Foreman therefore does not classify that event as an A004 P2 product defect.
It remains test/infrastructure debt for the appropriate diagnostics/validation
lane rather than a reason to mutate unrelated Transport behavior in this shift.

## Next-seam forensic review

Inspected current:

- `src/display/display.{c,h}`;
- `src/display/SYMBOLS.md`;
- `src/platform/ps2_graphics.{c,h}`;
- `src/ui/mpeg_calibration.h`;
- `src/rfb/flow_policy.{c,h}`;
- `src/media/clock.{c,h}`;
- `src/mpeg/decoder.{c,h}`;
- current Application presentation/request cadence;
- A003 and A004 audit ownership rules;
- current architecture overlay and Q6/Q7 authority.

Inspected frozen H1:

- `h1_mpeg_presentation_owner.{c,h,test}`;
- `h1_mpeg_start_handoff.{c,h,test}`.

Recovered start-side facts:

- ordinary visible state begins RFB_ONLY;
- preparing one run enters WAIT_FIRST_FRAME;
- WAIT_FIRST_FRAME preserves the already-presented desktop and is not MPEG
  ownership;
- exact first-frame notification promotes to MPEG_OWNED;
- MPEG_OWNED requires composited remote presentation rather than a globally
  frozen RFB desktop;
- geometry/suppression belongs to the exact run and stale generation events
  cannot seize ownership;
- pending-start abort is distinct from active retirement.

Two frozen H1 mechanisms are intentionally **not** current authority:

1. H1 presentation code minted its own generation sequence. Current architecture
   keeps MPEG run/generation identity distinct and Presentation should consume a
   caller-owned exact generation rather than create a competing namespace.
2. H1 active `stop()` transitioned MPEG_OWNED directly back to RFB_ONLY and
   created refresh debt. Current Q7 supersedes that shortcut. Retirement must
   later close new production/admission, allow accepted old work to drain,
   permit RFB rebuilding underneath still-visible retiring MPEG, and only then
   finalize visible retirement.

The current platform graphics seam already identifies synchronized flip as the
application-visible physical completion boundary, but the current product has no
MPEG compositor path. A003/A004 require the common media epoch to be armed by
Presentation at the real first synchronized MPEG presentation boundary, not by
START/decode/upload/state alone.

Therefore the next packet reconstructs the **presentation ownership state
contract** first and deliberately leaves physical GS composition plus
media-clock arm for the next presentation-mechanism tranche. This prevents a
state-only host transition from masquerading as physical presentation evidence.

## Packet issued

Foreman state revision 0025 publishes:

`A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3`

Execution classification:

```text
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
```

The packet is bounded to:

- Presentation-owned RFB_ONLY / WAIT_FIRST_FRAME / MPEG_OWNED state;
- caller-supplied generation fencing;
- immutable resolved base/inner/suppression geometry snapshot;
- protected WAIT_FIRST_FRAME semantics;
- exact first-frame-promotion event contract;
- composited-mode semantics after promotion;
- exact pending-start abort;
- explicit prohibition on H1-style direct active stop under current Q7.

It explicitly excludes:

- physical MPEG GS drawing/composition;
- media-clock arming;
- scheduler/drop;
- active retirement/current-Q7 final handoff;
- calibration/Application/RFB runtime wiring;
- MPEG START/decoder/Pi producer orchestration;
- A005;
- hardware qualification.

## Checks / evidence for this Foreman shift

REMOTE_AUTHORITY_REFRESH=PASS
A004_P2_SOURCE_REVIEW=PASS
A004_P2_C1_C9_FOREMAN_ACCEPTANCE=PASS
A004_P2_FINAL_CI=PASS
CURRENT_Q7_COMPATIBILITY=PASS
NEXT_OWNER_SEAM=PRESENTATION_OWNERSHIP_START_SIDE
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## State / contract revisions

Consumed:

- Foreman State revision 0024
- Reconstruction Contract revision 0006
- work-log contract revision 0006
- Wire runtime decisions revision 0011
- architecture overlay revision 0004
- A003 audit revision 0001
- A004 audit revision 0001

Produced:

- Foreman State revision 0025
- packet `A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3`

## Evidence gaps

PENDING_LOCAL=NONE_FOR_FOREMAN_PACKET_PUBLICATION
HARDWARE_PENDING=YES_FOR_LATER_A004_RUNTIME_VISUAL_AND_TIMING_QUALIFICATION
FULL_A004_COMPLETION=NOT_CLAIMED

## Next pickup

Interactive Reconstruction executes
`A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3` autonomously, emits exactly one
immutable `a004-presentation` Reconstruction log, and returns the baton to
Foreman.

Do not begin the physical compositor/clock-arm tranche in the same worker shift.
