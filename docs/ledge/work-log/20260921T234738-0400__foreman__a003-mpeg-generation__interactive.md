# Foreman shift — accept A003 R14 and activate ordinary RFB product composition

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T23:47:38-04:00
COMPLETED_AT=2026-09-21T23:52:00-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=ac04f50f9a2ab5d53ca1242fb925c00c61a940db
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume A003-RFB-SHARED-RUNTIME-PROFILE-R14, independently verify the single
canonical profile and deterministic cross-language projections, reconcile a
Foreman-owned Architecture Overlay publication defect, and publish only the next
bounded dependency if R14 is acceptable.

## Authority recovered

Assigning Foreman base:
db36b13ff58840a64d844ef0d6c9da9635dbdb84

Final R14 pre-log authority:
df1ad8a1f3d43c767756c1b596b898a4f0fff406

Immutable Reconstruction-log head:
ac04f50f9a2ab5d53ca1242fb925c00c61a940db

Immutable Reconstruction log:
docs/ledge/work-log/20260921T225956-0400__reconstruction__a003-mpeg-generation__interactive.md

No newer Foreman, Validation or Continuity baton followed it.

This Foreman seat has repository/GitHub authority but no attached Pi-local
worktree. Unknown external/Pi-local staged, unstaged or untracked state was not
asserted clean and was not modified.

scripts/resume-state.sh and scripts/check.sh were not run in a local shell.
Equivalent repository authority and canonical CI/project evidence were inspected
from the exact branch commits and GitHub Actions.

## Independent R14 review

Foreman confirms one canonical source:
src/config/rfb_runtime_profile.json.

Selected semantic/current values are exactly:
RFB=ON; window=32768; batch=8192; flush-on-empty=1; credit-return=1;
receiver-priority=63; receiver-stack=16384; max-DATA=8192.

scripts/generate-rfb-runtime-profile.py is the deterministic generator/checker
for checked-in C and Python projections. scripts/check.sh invokes --check.
A temporary-tree host test mutates the canonical profile, observes stale
projection failure, regenerates, and returns to PASS.

The PS2 projection uses the existing pstvnc_transport_session_config_t. The Pi
projection uses the existing R13 RfbFlowConfig. Both Pi capacities derive from
the same 32768-byte canonical window. OFF returns no active projection on either
peer. No Configuration-owned Wire/session ID exists.

At CP2N exact hardware source head 7e047a9c..., H1_RFB_ONLY inherits priority
63, stack 16384 and max payload 8192 and supplies the 32768/8192 RFB window and
credit policy. CP2N hardware/result authority remains bounded and does not
qualify the new R13 Pi provider-write direction or R14 product bytes.

Exact blob comparison proves Wire Protocol, Pi Wire server, systemd Wire service
and PS2 Application are unchanged from the R14 assigning base.

All A003-R14-C1 through C12 are MET.

FOREMAN_DISPOSITION=ACCEPT

## Machine/build evidence

Pre-log workflow 35683895809 at df1ad8a1... is SUCCESS on attempt 2.

Immutable-log-head workflow 35684065544 at ac04f50f... is SUCCESS on attempt 3
without source changes. Earlier attempts exposed known scheduler-sensitive host
assertions: one R13 peer can receive REQUEST before the sending thread executes
its immediately-following WAIT_BOUNDARY state transition, and an unchanged
Transport fixture can observe fake receive-call visibility before queue/activity
publication.

The exact accepted R14 source passes profile generation, PS2/Pi profile tests,
R13 regressions, Wire/Q4 tests, Transport RFB/AUDIO/MPEG regressions, strict
dictionary/topology/project checks, PS2 compile and current-source linked
reproducibility.

R14 changes the accepted-R13-base PT_LOAD by +128 bytes. The final linked R14
PT_LOAD is one segment, 484756 bytes,
e13e4fa85635095a7ba8226620d37c4cd9956c2e3bd973959088476812e6e9ac.
This is reproducible build evidence only and remains hardware-pending.

## Foreman-owned Overlay correction

Overlay revision 0006 had correct revision metadata but retained revision
0005's provisional opening paragraph because the previous Foreman text
replacement missed its exact target. Reconstruction did not own or cause that
error. This shift publishes Overlay revision 0007 correcting the opening and
recording accepted R13/R14 authority.

## Packet publication

Foreman State advances 0043 -> 0044.

Active packet:
A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15

R15 activates only the ordinary RFB-only product composition through the normal
PS2 Application and Pi Wire-server entrypoints, enrolls the R14 Pi runtime
projection in inert staging, and stabilizes the known R13 test-only post-send
state assertion. It does not add retry/restart, Wire CONFIG, AUDIO/MPEG or
physical qualification.

## Evidence boundary

SOURCE_COMPLETE_R14=YES
HOST_TESTED_R14=YES
R14_FOREMAN_ACCEPTED=YES
INDEPENDENT_VALIDATION_R14=NOT_RUN
R14_PHYSICAL_QUALIFICATION=PENDING
R15_APPLICATION_RFB_ACTIVATION=ACTIVE_RECONSTRUCTION_PACKET
RFB_FAILURE_RESTART_POLICY=NOT_IMPLEMENTED
WIRE_CONFIG_DELIVERY=NOT_IMPLEMENTED
PI_MPEG_CONTROL_OWNER=NOT_IMPLEMENTED
PI_MPEG_PRODUCER_OWNER=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PHYSICAL_HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes only
A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15 and emits exactly one immutable
Reconstruction log using reconstruction / a003-mpeg-generation / interactive.

Do not combine RFB retry/restart, Wire CONFIG delivery, AUDIO/MPEG activation or
physical qualification into this shift.
