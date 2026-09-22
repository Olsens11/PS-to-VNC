# Foreman shift — accept corrected A003 R13 and activate shared RFB profile

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T22:17:27-04:00
COMPLETED_AT=2026-09-21T22:23:32-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=30c18f7b50a397b3eb405bf88a574dbd14b4c310
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume A003-PI-RFB-ATTACHMENT-QUIESCE-R13-CORRECTIVE-A, independently verify
the Foreman-found wake defect is actually closed, accept or reject the full R13
boundary, and if accepted publish only the next configuration dependency.

## Live authority / workspace boundary

Branch authority at pickup and immediately before Foreman write remained:

30c18f7b50a397b3eb405bf88a574dbd14b4c310

Current Foreman State was revision 0042. The newest immutable Reconstruction
baton was:

docs/ledge/work-log/20260921T202758-0400__reconstruction__a003-mpeg-generation__interactive.md

No newer Foreman, Validation or Continuity baton followed it.

This Foreman seat has repository/GitHub authority but no attached Pi-local
worktree. Unknown external/Pi-local staged, unstaged or untracked state was not
asserted clean and was not modified.

scripts/resume-state.sh and scripts/check.sh were not run in a local shell.
Equivalent repository authority and canonical checks were recovered from refs,
governing documents, exact comparison and GitHub Actions.

## Corrective range reviewed

Corrective Foreman base:
e71c44c45904738b25b6c5596446db49d64b6f57

Final corrected pre-log authority:
534d2b88bf2e775e55dd865f616f6a7a5e538155

Immutable Reconstruction-log head:
30c18f7b50a397b3eb405bf88a574dbd14b4c310

The corrective range is linear, ahead by nine and behind by zero. No src/ or
mk/ path changed.

## Independent corrected-R13 acceptance

Foreman confirmed the private nonblocking socketpair is one attachment/session
wake only. request_quiesce() publishes REQUEST_PENDING and a local byte under a
narrow RLock. The requester has no physical Wire send/recv/frame/sequence
authority. WireConnectionOwner alone waits on/drains the wake and serializes
REQUEST through the existing sole Wire send path.

There is no select timeout or sleep polling workaround.

The focused test now proves idle REQUEST publication with no extra PS2 CREDIT,
provider bytes, endpoint close or timeout poll. Existing lifecycle tests no
longer inject the artificial wake CREDIT.

Exact clean ordering remains:

REQUEST -> BOUNDARY -> accepted provider-write drain -> provider close ->
COMMIT -> COMPLETE.

Provider reads stop at BOUNDARY, provider I/O is retired before COMMIT,
post-COMMIT ordinary attachment traffic is absent, COMPLETE stops only RFB,
provider failures remain RFB-local, and Session B receives fresh attachment and
wake state.

All original A003-R13-C1 through C12 are MET.

All corrective R13-CA1 through CA7 are MET.

FOREMAN_DISPOSITION=ACCEPT

## Machine evidence

Corrected pre-log workflow 35672554566 at 534d2b88... completed SUCCESS on
attempt 2. Attempt 1 hit the known unchanged transport_runtime_test scheduler
race; no src/ or mk/ source changed and only the failed host job was rerun on
the exact same commit.

Worker-log-head workflow 35672809990 at 30c18f7b... completed SUCCESS on
attempt 1 across every canonical job class.

Observed successful evidence includes the independent idle-wake test, corrected
ordered-quiesce test, fresh Session-B wake test, sole-sender/no-polling test,
Transport RFB/AUDIO/MPEG regressions, strict dictionaries/topology/project
checks, PS2 compile and current-source linked reproducibility.

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_CORRECTED_R13
HOST_TESTED=YES
PS2_SOURCE_CHANGED_BY_CORRECTIVE=NO
PS2_BUILD_INPUT_CHANGED_BY_CORRECTIVE=NO
INDEPENDENT_VALIDATION_R13=NOT_RUN
LIVE_PI_R13_ATTACHMENT=NOT_RUN_NOT_CLAIMED
R13_PRODUCT_HARDWARE_QUALIFICATION=NOT_RUN_NOT_CLAIMED
HARDWARE_QUALIFICATION_R13=PENDING

## Foreman architecture reconciliation

Architecture Overlay advances revision 0005 -> 0006 in this Foreman commit.
Revision 0006 records actual independent Foreman acceptance of corrected R13;
revision 0005 remains the truthful provisional pre-acceptance snapshot.

## Dependency decision

R13 is mechanism-complete but still intentionally lacks product values.

The next packet does not activate RFB. It creates one Configuration-owned,
single-source current RFB profile and deterministic projections into the already
accepted PS2 Transport config type and Pi R13 RfbFlowConfig.

Current selected profile values are taken from the functionally
hardware-qualified CP2J/CP2N RFB-only baseline:

window=32768
batch=8192
flush_on_empty=1
credit_return=1
receiver_priority=63
receiver_stack=16384
max_data_payload=8192
RFB semantic mode=ON

The historical H1 numeric RFB enum is not promoted. Q4 remains sole Wire Session
identity authority.

The Pi R13 provider-write capacity is projected from the same 32768-byte
canonical window to avoid a second independent tuning literal. This equality is
new product source policy, not a claim CP2N hardware-qualified the R13 Pi queue
direction.

## Packet publication

Foreman State advances 0042 -> 0043.

Active packet:
A003-RFB-SHARED-RUNTIME-PROFILE-R14

R14 owns configuration/profile authority and deterministic cross-language
projection only. Wire CONFIG delivery, default-service attachment, Application
RFB startup, retry/restart, MPEG and physical qualification remain deferred.

## Evidence boundary / pending work

PENDING_LOCAL=NONE_FOR_FOREMAN_PUBLICATION
A003_R13_FOREMAN_ACCEPTED=YES
A003_R13_INDEPENDENT_VALIDATION=NOT_RUN
A003_R14_SHARED_RFB_PROFILE=ACTIVE_RECONSTRUCTION_PACKET
DEFAULT_PI_WIRE_SERVICE_RFB_ATTACHMENT=DISABLED
PS2_APPLICATION_RFB_PROFILE_CONSUMPTION=NOT_IMPLEMENTED
WIRE_CONFIG_DELIVERY=NOT_IMPLEMENTED
APPLICATION_RFB_STARTUP_RESTART=NOT_IMPLEMENTED
PI_MPEG_CONTROL_OWNER=NOT_IMPLEMENTED
PI_MPEG_PRODUCER_OWNER=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PHYSICAL_HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes only
A003-RFB-SHARED-RUNTIME-PROFILE-R14 and emits exactly one immutable
Reconstruction log using reconstruction / a003-mpeg-generation / interactive.

Do not activate ordinary product RFB, add Wire CONFIG delivery, implement
retry/restart, Pi AUDIO/MPEG, Application MPEG activation or physical
qualification in the same shift.
