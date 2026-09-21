# Foreman shift — accept A003 R12 and activate RFB attachment/quiesce

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T18:38:24-04:00
COMPLETED_AT=2026-09-21T18:44:28-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=1d5b297acac059f8906a67f28d141972a357dfc6
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed A003-PI-RFB-INTERNAL-PROVIDER-ENDPOINT-R12 baton,
independently verify the Pi-local provider endpoint/mutual-exclusion boundary,
then select the smallest mature dependency before ordinary RFB product
activation.

## Live authority / workspace boundary

Branch authority at pickup and immediately before Foreman write remained:

1d5b297acac059f8906a67f28d141972a357dfc6

Current Foreman State was revision 0040. The newest immutable baton was:

docs/ledge/work-log/20260921T181526-0400__reconstruction__a003-mpeg-generation__interactive.md

No newer Foreman, Validation or Continuity baton followed it.

This Foreman seat has repository/GitHub authority but no attached Pi-local
worktree. Unknown external/Pi-local staged, unstaged or untracked state was not
asserted clean and was not modified.

scripts/resume-state.sh and scripts/check.sh were not run in a local shell.
Equivalent authority and canonical checks were recovered from refs, governing
documents, exact changed-path comparison and GitHub Actions.

## Exact R12 range reviewed

Previous Foreman base:
956ffd1f521f6e6654d6161d2cd34f01225b5b9e

Final pre-log authority:
4bab683674f29a790cc1d8a6d80f788aab2db744

Immutable worker-log head:
1d5b297acac059f8906a67f28d141972a357dfc6

The pre-log compare is linear, ahead by 14 commits and behind by zero. No src/
or mk/ path changed. The worker-log commit adds only the required immutable
Reconstruction record.

## Independent R12 acceptance

Foreman independently confirmed exact loopback-only endpoint ownership, selected
native :0 X0 provider reuse, inherited-listener behavior, no second provider
listener, direct/internal systemd mutual exclusion, exact preservation of R11
direct/historical units, development-5903 exclusion, separate manager-inert
internal staging and unchanged/unattached R10 Wire/Relay authority.

Disposition:

A003-R12-C1 INTERNAL_PROVIDER_ENDPOINT_TRACKED — MET
A003-R12-C2 LOOPBACK_ONLY_127_0_0_1_5900 — MET
A003-R12-C3 NATIVE_DISPLAY_0_X0_PROVIDER_REUSED — MET
A003-R12-C4 SYSTEMD_DEMAND_ACTIVATION — MET
A003-R12-C5 NO_COMPETING_PROVIDER_LISTENER — MET
A003-R12-C6 DIRECT_AND_INTERNAL_PATHS_MUTUALLY_EXCLUSIVE — MET
A003-R12-C7 R11_DIRECT_AND_HISTORICAL_AUTHORITY_PRESERVED — MET
A003-R12-C8 WINDOWS_5903_EXCLUDED — MET
A003-R12-C9 FAIL_CLOSED_INACTIVE_STAGING — MET
A003-R12-C10 R10_RELAY_REMAINS_UNATTACHED — MET
A003-R12-C11 STATIC_TEST_AND_DOC_RECONCILIATION — MET
A003-R12-C12 CLAIM_BOUNDARY_PRESERVED — MET

FOREMAN_DISPOSITION=ACCEPT

## Machine evidence

Final pre-log workflow 35662431648 at 4bab6836... finished SUCCESS on attempt 5.
The retry history is retained because exact-head attempts exposed known
scheduler-sensitive unchanged Transport host fixtures plus one non-reproducible
project-check failure. No product source was changed between attempts.

The final successful run passed Transport RFB/AUDIO/MPEG host fixtures, all
R10/R11/R12 Pi tests, project/topology checks, strict dictionaries, PS2 compile
and current-source linked reproducibility.

Worker-log-head workflow 35663279849 at 1d5b297a... completed SUCCESS on attempt
1 across every canonical job class.

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R12
HOST_STATIC_TESTED=YES
PS2_SOURCE_CHANGED=NO
PS2_BUILD_INPUT_CHANGED=NO
REPOSITORY_PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
INDEPENDENT_VALIDATION_R12=NOT_RUN
LIVE_PI_R12_STAGING=NOT_RUN_NOT_CLAIMED
INTERNAL_X0_PROVIDER_FRESH_PHYSICAL_QUALIFICATION=NOT_RUN_NOT_CLAIMED
HARDWARE_QUALIFICATION_R12=PENDING

## Dependency decision

R10/R11/R12 now provide the Relay, provider identity and internal provider
endpoint. The next missing seam is not Application activation yet: the Pi still
has no lifecycle owner that lazily connects that provider to one Wire Session
and executes the existing finite RFB quiesce protocol.

R13 therefore owns provider attachment + ordered quiesce only.

Two constraints prevent shortcutting this step:

1. Wire ACTIVE must remain valid while idle, so Q4 success alone cannot
   demand-start X0tigervnc.
2. Pi does not yet own the shared RFB flow-profile values, so R13 cannot hardcode
   lab queue/window/payload numbers into the installed daemon. The attachment
   core accepts explicit finite configuration and the default service remains
   establishment-only until later session-composition/config authority supplies
   it.

The first PS2 channel-1 CREDIT is the bounded lazy rider-activity edge. Provider
connect must remain nonblocking. Provider failure is local to RFB. The existing
PS2 parser-safe REQUEST/BOUNDARY/COMMIT/COMPLETE implementation remains peer
authority and is not rewritten in R13.

## Packet publication

Foreman State advances 0040 -> 0041.

Active packet:
A003-PI-RFB-ATTACHMENT-QUIESCE-R13

R13 owns only Pi provider attachment lifecycle, R10 composition mechanics and
the ordered four-marker quiesce. Shared session configuration, Application RFB
startup/restart policy, Pi MPEG work and physical qualification remain deferred.

## Evidence boundary / pending work

PENDING_LOCAL=NONE_FOR_FOREMAN_PUBLICATION
A003_R12_FOREMAN_ACCEPTED=YES
A003_R12_INDEPENDENT_VALIDATION=NOT_RUN
A003_R13_RFB_ATTACHMENT_QUIESCE=ACTIVE_RECONSTRUCTION_PACKET
PI_RFB_RELAY_CORE=FOREMAN_ACCEPTED_SOURCE_MACHINE_SCOPE
PI_INTERNAL_RFB_PROVIDER_ENDPOINT=FOREMAN_ACCEPTED_SOURCE_STATIC_SCOPE
RFB_PROVIDER_ATTACHMENT_QUIESCE=NOT_YET_RECONSTRUCTED
SHARED_RFB_SESSION_COMPOSITION_CONFIG=NOT_IMPLEMENTED
APPLICATION_RFB_STARTUP_RESTART=NOT_IMPLEMENTED
PI_MPEG_CONTROL_OWNER=NOT_IMPLEMENTED
PI_MPEG_PRODUCER_OWNER=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PHYSICAL_HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes only
A003-PI-RFB-ATTACHMENT-QUIESCE-R13 and emits exactly one immutable
Reconstruction log using reconstruction / a003-mpeg-generation / interactive.

Do not implement shared CONFIG/session composition, Application RFB startup,
automatic retry/restart, Pi AUDIO/MPEG, Application MPEG activation or physical
qualification in the same shift.
