# Foreman shift — accept A003 R6 and activate MPEG runtime-profile authority

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T07:35:37-04:00
COMPLETED_AT=2026-09-21T07:48:11-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=6b37d60690be15754d9f0bf1e47f4081f16b1bef
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed A003-MPEG-GENERATION-CONTROL-RELAY-R6 Reconstruction
baton, independently verify all packet criteria and evidence boundaries, then
publish the smallest owner-correct prerequisite before live Application MPEG
activation.

## Live authority and workspace boundary

Branch authority at pickup and immediately before write remained:

6b37d60690be15754d9f0bf1e47f4081f16b1bef

Current Foreman State was revision 0034 and had not consumed the newest
Reconstruction baton.

Newest immutable baton:

docs/ledge/work-log/20260921T071223-0400__reconstruction__a003-mpeg-generation__interactive.md

No newer Foreman, Validation or continuity work log followed it.

This Foreman seat has repository/GitHub authority but no attached Pi-local
worktree. Therefore staged, unstaged and untracked Pi-local state was not
asserted clean and was not modified, reset, cleaned, rebased or otherwise
destroyed. Repository refs/history/content and canonical CI were inspected
directly. Under Reconstruction Contract rev 0006 this did not justify turning
the user into a terminal proxy.

scripts/resume-state.sh was not executed in a local shell; equivalent resume
authority was recovered directly from current branch refs, current state,
governing documents and newest-first immutable logs.

scripts/check.sh was not executed locally. Its canonical repository equivalent,
the project-check job in the final coherent R6 GitHub Actions run, passed.

## Exact worker range reviewed

Previous Foreman base:

a646b49ed8200ee127dd96f37d4e3359145c6607

Final pre-log source/dictionary authority:

31db24ad56515d09a0218b1d9e20718c7bcf9662

Immutable worker-log head:

6b37d60690be15754d9f0bf1e47f4081f16b1bef

The pre-log compare is linear, ahead by eleven commits and behind by zero. It
touches only:

- docs/reference/SOURCE_SYMBOL_DICTIONARIES.md;
- src/transport/SYMBOLS.md;
- src/transport/bridge.c and bridge.h;
- src/transport/physical_stream.h;
- src/transport/runtime.c and runtime.h;
- tests/unit/transport_bridge_test.c;
- tests/unit/transport_mpeg_test.c;
- tests/unit/transport_protocol_test.c.

No Application, MPEG decoder/worker, Display/Presentation, RFB, audio,
configuration or Pi product source entered the packet.

## Independent R6 acceptance

Foreman independently confirmed:

- START/RETIRE/take-completion are public Transport bridge operations requiring
  current opaque Transport access;
- stale Session-A authority cannot send through or consume Session-B control
  state;
- exact START and RETIRE codecs/envelopes remain protocol-owned;
- outbound START and RETIRE enter
  pstvnc_transport_runtime_submit_frame();
- within current Transport product source only the private runtime I/O owner
  calls the physical framed-send primitive;
- inbound exact RETIRE completion is decoded into one synchronized bounded
  session-local slot;
- malformed RETIRE, wrong channel/flags/length/version and inbound START fail the
  Transport session;
- the pending slot is no-overwrite, nonblocking to take and one-shot;
- Transport performs no Application generation comparison;
- RETIRE completion does not synthesize producer_done/EOF and does not finalize
  residual bytes/credit;
- 44-byte START-shaped DATA/channel4 remains opaque media;
- the direct physical MPEG START convenience path is removed;
- existing rider behavior remains outside the new control semantics.

Disposition:

- A003-R6-C1 SOLE_IO_CONTROL_SEND — MET
- A003-R6-C2 EXACT_START_ENVELOPE — MET
- A003-R6-C3 EXACT_RETIRE_ENVELOPE — MET
- A003-R6-C4 INBOUND_RETIRE_COMPLETION — MET
- A003-R6-C5 ONE_SLOT_NONBLOCKING_TAKE — MET
- A003-R6-C6 SESSION_BOUND_ACCESS — MET
- A003-R6-C7 NO_GENERATION_BUSINESS_STATE — MET
- A003-R6-C8 NO_SYNTHETIC_PRODUCER_DONE — MET
- A003-R6-C9 PURE_MPEG_DATA — MET
- A003-R6-C10 NO_DIRECT_PHYSICAL_BYPASS — MET
- A003-R6-C11 EXISTING_RIDERS_PRESERVED — MET
- A003-R6-C12 CLEAN_EVIDENCE — MET

FOREMAN_DISPOSITION=ACCEPT

## Machine evidence

Canonical final pre-log workflow:

35593508080 at 31db24ad56515d09a0218b1d9e20718c7bcf9662 — SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R6
HOST_TESTED=YES
PS2_COMPILE=PASS
PS2_LINK_CURRENT_SOURCE_REPRODUCIBILITY=PASS
REPOSITORY_PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
INDEPENDENT_VALIDATION_R6=NOT_RUN
PHYSICAL_MACHINE_EVIDENCE_R6=NOT_RUN
OPERATOR_PHYSICAL_OBSERVATION_R6=NOT_RUN
HARDWARE_QUALIFICATION_R6=PENDING

The newest Validation log predates R6 and is not reused as R6 Validation.
No machine evidence was converted into hardware evidence.

## Cross-owner review

R6 remained inside Transport ownership. The relay transports and validates Wire
mechanics and session validity but does not own active MPEG meaning, decoder
lifecycle, Presentation policy, Pi producer lifecycle or Application
orchestration.

Current Q6 remains governing: MPEG generation identity is run fencing rather
than user-facing product semantics.

Current Q7 remains governing: new MPEG production closes first; already accepted
MPEG may drain; RFB restoration may proceed underneath visible retiring
MPEG/mattes; final retirement removes MPEG/mattes and reveals the already
refreshing desktop.

Current Q9/Q12 remain governing: Wire loss ends the session, reconnect never
resumes it, stale Transport authority cannot cross sessions, and each module
must complete its own local retirement before restart/resource reuse.

## Next dependency decision

The local mechanism chain is now intentionally parameteric. Transport MPEG
storage/credit, decoder feed/resource bounds, worker execution, PS2 dormancy
observation and Display scheduling all require explicit caller values.

The current production CONFIG wire deliberately does not carry the broad H1
laboratory tuning surface. A002/A003/A004 instead authorize narrow selected
qualified profiles.

The evidence-selected current values are recoverable without architectural
invention:

- MPEG Transport: 524288-byte queue and initial credit, 8192-byte credit batch,
  flush-on-empty enabled, credit return enabled;
- decoder: 704x480 bound, RGB16/two bytes per pixel, 2048-byte feed, 16-byte
  transfer alignment, 64-byte buffer alignment;
- worker: 65536-byte stack, priority 67;
- PS2 worker dormancy observation: 1000 us by 3000 observations, fail closed;
- Display scheduler: 30000/1001 absolute cadence with current initial product
  profile drop disabled and threshold zero.

These are selected profile provenance, not reconstructed-code hardware
qualification.

The mature Pi product Wire server/control topology is still absent from current
product source. Building it immediately would combine source/service topology,
Q4 establishment and MPEG remote lifecycle. The already-defined value-authority
seam is smaller and is required before Application activation regardless of Pi
implementation.

## State / next packet publication

Foreman State advances:

STATE_REVISION=0034 -> 0035

Current active packet:

A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7

R7 owns only Configuration selection/provenance for the narrow existing owner
value types. It explicitly forbids CONFIG-wire expansion, H1 lab-knob
productization, geometry-owner duplication and live MPEG activation.

Pi product Wire/control ownership remains dependency-queued after R7.

## Pending / evidence boundary

PENDING_LOCAL=NONE_FOR_FOREMAN_PUBLICATION
A003_R6_FOREMAN_ACCEPTED=YES
A003_R6_INDEPENDENT_VALIDATION=NOT_RUN
A003_R6_HARDWARE_QUALIFICATION=PENDING
A003_R7_MPEG_RUNTIME_PROFILE_AUTHORITY=ACTIVE_RECONSTRUCTION_PACKET
PI_PRODUCT_WIRE_CONTROL_OWNER=NOT_IMPLEMENTED
PI_PRODUCT_MPEG_PRODUCER_OWNER=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
Q7_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
WIRE_LOSS_MPEG_RECOVERY=NOT_IMPLEMENTED
FINAL_APPLICATION_MPEG_TRANSACTION=NOT_IMPLEMENTED
PHYSICAL_HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes:

A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7

and emits exactly one immutable Reconstruction log using:

- ROLE_KEY=reconstruction;
- WORK_ITEM_KEY=a003-mpeg-generation;
- WORKER_KEY=interactive.

Do not begin Pi product Wire ownership, Application activation, Q7 retirement
or physical qualification in the same shift.
