# Foreman shift — accept A003 R1 and activate A004 MPEG calibration core

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T16:43:19-04:00
COMPLETED_AT=2026-09-20T16:46:00-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=921dc29dd67baadc07c8f648aa4fdb46954b2167
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed A003 R1 Reconstruction baton, independently validate the
bounded source/evidence claims, decide whether A004 may advance, and publish the
next owner-correct Reconstruction packet.

## Authority refreshed

Live branch authority was re-read immediately before write and remained exactly:

`921dc29dd67baadc07c8f648aa4fdb46954b2167`

with parent source/dictionary authority:

`170e2198d41b3d703a5fe86bd5c329f2248d031a`.

The immutable worker handoff is:

`docs/ledge/work-log/20260920T160357-0400__reconstruction__a003-mpeg-generation__interactive.md`.

## R1 independent review

Inspected the three substantive commits, final source range, current
Transport/RFB/audio/MPEG boundaries, worker log, current architecture and
dependency authorities.

Accepted:

- single Transport-owned physical framed-I/O owner;
- cooperative idle scheduling invariant with 1000 us remaining implementation
  baseline rather than ABI;
- opaque session-bound Transport access across RFB/audio/MPEG;
- stale old-session access rejection without numeric session IDs in riders;
- module-owned retirement rather than generic Transport drain;
- final dictionary reconciliation;
- explicit exclusion of rejected Q12 drain/resume-branch work.

No R1 product defect was found.

R1-C1 through R1-C8 are accepted as MET within the packet's bounded scope.

## Independent CI evidence

GitHub Actions run `35536021547` at pickup HEAD completed SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS.

These are machine checks, not physical hardware qualification.

The Reconstruction worker was correct not to claim new HARDWARE_PROVEN status.

## Dependency finding

Current `src/app.c` still opens only
`pstvnc_transport_session_open()`, has no MPEG run activation/retirement
transaction, and default `pstvnc_app_run()` remains intentionally fail-closed
for absent validated concrete Transport configuration.

That is later product work, not an R1 defect.

Final A003 application orchestration should not be built now against placeholder
callbacks because Q6/Q7 and A004 assign real ownership to MPEG region,
calibration acceptance, RFB/presentation state, first physical presentation and
visible retirement handoff.

A004 can advance independently by reconstructing its own first owner seam: the
MPEG CALIBRATION value/state core. This provides a real accepted-region
authority for later presentation and Application work.

A004_ADVANCE_DECISION=YES
A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED_AFTER_REQUIRED_A004_SEAMS

## Current-authority correction retained

The historical A004 audit's old retirement sentence is not promoted as current
Q7 behavior. Current authority permits RFB restoration underneath after new MPEG
production closes while valid retiring MPEG presentation remains visible until
safe finalization.

The active P1 packet does not implement retirement, but later A004 work must use
the current Q7 ordering.

## Packet issued

Foreman state revision 0022 publishes:

`A004-MPEG-CALIBRATION-CORE-R1`

Target:

- ROLE_KEY=reconstruction
- WORK_ITEM_KEY=a004-presentation
- WORKER_KEY=interactive
- EXECUTION_SEAT=/home/ps2/src/PS-to-VNC-ledge-manual

The packet is intentionally limited to MPEG CALIBRATION geometry/state and
foreground accept/cancel semantics sourced from the frozen
`mpeg_presentation_calibration/` lineage.

It explicitly excludes RFB freeze/suppression, live MPEG activation, Pi
producer, Transport START/RETIRE wiring, first-frame ownership, compositor,
scheduler/drop, Application orchestration and hardware qualification.

## Checks / evidence for this Foreman shift

REMOTE_AUTHORITY_REFRESH=PASS
R1_SOURCE_REVIEW=PASS
R1_CI_WORKFLOW=PASS
R1_FOREMAN_ACCEPTANCE=PASS
Q12_REJECTED_DRAIN_REINTRODUCED=NO
A004_DEPENDENCY_DECISION=ADVANCE_P1
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## Next pickup

Interactive Reconstruction executes
`A004-MPEG-CALIBRATION-CORE-R1`, emits one immutable
`a004-presentation` worker log, and returns the baton to Foreman.

Do not resume final A003 Application orchestration and do not begin A004 RFB
freeze/compositor work in the same worker shift.
