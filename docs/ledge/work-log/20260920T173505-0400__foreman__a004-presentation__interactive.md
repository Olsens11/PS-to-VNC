# Foreman shift — accept A004 P1 and activate RFB freeze/refresh P2

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T17:35:05-04:00
COMPLETED_AT=2026-09-20T17:38:46-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed `A004-MPEG-CALIBRATION-CORE-R1` Reconstruction baton,
independently verify the worker's source/evidence, reconcile the next A004
dependency against current Q7 authority, and publish the next bounded
Reconstruction packet without executing product behavior from the Foreman seat.

## Authority refreshed

At Foreman pickup, live branch authority had advanced from prior Foreman
authority `672f67eb818cb3bd08ebaf611ac9378784d6d6f6` to:

`b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe`

The branch was five commits ahead and zero behind the prior Foreman authority.

Newest relevant Reconstruction record:

`docs/ledge/work-log/20260920T171848-0400__reconstruction__a004-presentation__interactive.md`

The worker correctly left Foreman state revision 0022 untouched.

## Worker commit review

Reviewed the exact worker range:

- `ad00a7a07c15c8d56f89e5ac664e07507e50c312` —
  calibration core source/test integration;
- `51025be8553c3dad8b5a11002fb57f9bb7a5af62` —
  symbol dictionary/portal integration;
- `3d9f4a821f16d51a3d22597b4a539009aac7705f` —
  stable named calibration action enum;
- `094f07fff17801152bcd0d17ac69c570d8bfad27` —
  dictionary ownership repair only;
- `b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe` —
  immutable Reconstruction log.

Final packet paths are limited to:

- `src/ui/mpeg_calibration.c`
- `src/ui/mpeg_calibration.h`
- `src/ui/SYMBOLS.md`
- `tests/unit/mpeg_calibration_test.c`
- `tests/Makefile`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`
- the new immutable worker log.

No RFB, Transport, MPEG decoder, Display, Platform, Application or existing
DESKTOP CALIBRATION source changed.

## Independent P1 behavior review

Foreman independently inspected the final calibration source/header/tests and
the relevant frozen H1 calibration lineage.

Confirmed:

- base rectangle, inner content/matte and outer suppression footprint remain
  distinct meanings;
- the 704x462 recovered default remains 176,119 / 352x224;
- base dimensions remain >=16 and 16-pixel aligned;
- base placement remains one-pixel precise;
- resize/move/matte edit direction and center-preserving behavior agree with
  frozen H1 evidence;
- widened arithmetic is used for overflow-sensitive geometry;
- REVIEW accept cannot occur from the same held-X state that entered REVIEW;
- release then fresh accept press is required;
- acceptance copies one committed/caller-visible value and is exactly-once per
  successful transition;
- cancel does not create acceptance;
- release quarantine remains owner-local;
- no acceptance path claims MPEG activation, first physical presentation,
  media-clock arm or visual ownership.

No bounded-scope P1 product defect was found.

A004-P1-C1 through C9 are Foreman accepted as MET.

## Independent CI evidence

GitHub Actions run `35539055477`, run number 264, at worker HEAD completed
SUCCESS.

Observed jobs:

- host-unit — PASS
- project-check — PASS
- dictionary-long — PASS
- ps2-compile — PASS
- ps2-link/current-source reproducibility — PASS
- dictionary-reconcile — SKIPPED

No physical PS2 run was performed. Repository/build success is not promoted to
hardware qualification.

## Next-seam forensic review

Inspected current:

- `src/rfb/rfb_session.{c,h}`;
- `src/rfb/bridge.{c,h}`;
- `src/display/display.{c,h}`;
- `src/app.c`;
- current clean architecture/topology;
- A004 audit revision 0001;
- current Wire runtime Q7 revision 0011.

Inspected frozen H1:

- `h1_mpeg_calibration_rfb_gate.{c,h,test}`;
- `h1_mpeg_calibration_rfb_schedule.{c,h,test}`;
- `h1_mpeg_calibration_rfb_flow.{c,h,test}`;
- `h1_mpeg_presentation_owner.{c,h,test}`;
- start-handoff/foreground evidence where needed to resolve ownership.

Current RFB already has one authoritative complete-server-message safe boundary:
`pstvnc_rfb_session_try_receive_update()` may yield IDLE only before consuming
the first byte of the next message, and it completes an already-started message
before returning. P2 must reuse that authority rather than inventing another
parser boundary.

Frozen H1 demonstrates a separable generic behavior:

- freeze stops new requests and new remote visual publication but not protocol
  life;
- one pre-freeze outstanding response remains protocol-owned through completion;
- one outstanding request is tracked;
- thaw schedules one FULL refresh and then returns to incremental cadence.

That behavior can be reconstructed as generic RFB flow policy before the later
presentation owner exists.

## Q7 reconciliation

The historical A004 audit remains immutable temporal evidence.

Its older retirement ordering is not forward authority where it conflicts with
current Q7 revision 0011.

Current Q7 permits RFB to refresh the underlying desktop after new MPEG
production/admission closes while valid retiring MPEG/mattes remain visibly on
top until safe finalization.

Therefore P2 explicitly prohibits encoding active/retiring MPEG as a global RFB
freeze. Global freeze is only the mechanism needed for calibration and
WAIT_FIRST_FRAME-style ownership gaps. Later MPEG-owned presentation uses
region-local suppression/composition while RFB truth may advance.

## Packet issued

Foreman state revision 0023 publishes:

`A004-RFB-FREEZE-REFRESH-R2`

Target:

- ROLE_KEY=reconstruction
- WORK_ITEM_KEY=a004-presentation
- WORKER_KEY=interactive
- EXECUTION_SEAT=/home/ps2/src/PS-to-VNC-ledge-manual

The packet is bounded to generic RFB request/publish flow policy, one
outstanding request, in-flight freeze completion, one-shot FULL recovery,
safe-boundary reuse, and current-Q7 compatibility.

It explicitly excludes calibration wiring, controller-entry policy,
pointer/mouse handoff, MPEG START/RETIRE, presentation/first-frame ownership,
compositor/suppression drawing, media-clock arm, scheduler/drop, retirement
visible handoff, final Application orchestration, A005, and hardware
qualification.

## Checks / evidence for this Foreman shift

REMOTE_AUTHORITY_REFRESH=PASS
A004_P1_SOURCE_REVIEW=PASS
A004_P1_C1_C9_FOREMAN_ACCEPTANCE=PASS
A004_P1_CI_WORKFLOW=PASS
Q7_FORWARD_AUTHORITY_RECONCILED=PASS
NEXT_OWNER_SEAM=GENERIC_RFB_FREEZE_REFRESH
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## State / contract revisions

Consumed:

- Foreman state revision 0022
- Reconstruction contract revision 0005
- work-log contract revision 0006
- Wire runtime decisions revision 0011
- architecture overlay revision 0004
- A004 audit revision 0001
- A004 calibration-separation invariant revision 0001

Produced:

- Foreman state revision 0023
- packet `A004-RFB-FREEZE-REFRESH-R2`

## Evidence gaps

PENDING_LOCAL=NONE_FOR_FOREMAN_PACKET_PUBLICATION
HARDWARE_PENDING=YES_FOR_LATER_A004_RUNTIME_QUALIFICATION
FULL_A004_COMPLETION=NOT_CLAIMED

## Next pickup

Interactive Reconstruction executes
`A004-RFB-FREEZE-REFRESH-R2`, emits exactly one immutable
`a004-presentation` Reconstruction log, and returns the baton to Foreman.

Do not begin the presentation/first-frame tranche in the same worker shift.
