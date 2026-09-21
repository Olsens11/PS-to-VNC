# Foreman shift — accept A003 R7 and activate Pi Wire establishment foundation

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T08:06:00-04:00
COMPLETED_AT=2026-09-21T08:13:05-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=c2d7a5020797ac264c36f5f37d385401c8b49c1e
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7 Reconstruction
baton, independently verify the exact selected values, ownership, build/test and
claim boundaries, then publish the smallest mature next prerequisite before
Application MPEG activation.

## Live authority / workspace boundary

Branch authority at pickup and immediately before Foreman write remained:

c2d7a5020797ac264c36f5f37d385401c8b49c1e

Current Foreman State was revision 0035 and had not consumed the newest R7
Reconstruction baton.

Newest immutable baton:

docs/ledge/work-log/20260921T075451-0400__reconstruction__a003-mpeg-generation__interactive.md

No newer Foreman, Validation or continuity log followed it.

This Foreman seat has repository/GitHub authority but no attached Pi-local
worktree. Therefore local staged/unstaged/untracked Pi state was not asserted
clean and was not reset, cleaned, checked out over or otherwise modified.

scripts/resume-state.sh was not invoked in a local shell; equivalent resume
authority was recovered directly from branch refs, current state, governing
documents and newest-first immutable logs.

scripts/check.sh was not invoked locally. Canonical project-check and the
subsystem checks from exact GitHub Actions source authority were inspected
directly.

## Exact R7 range reviewed

Previous Foreman base:

d9f8e803048574bb5b02296b92f7571a37693675

Final pre-log source/dictionary/build authority:

5cdd5670a1924bde8e7348bd863c07b0cd28bc4c

Immutable worker-log head:

c2d7a5020797ac264c36f5f37d385401c8b49c1e

The pre-log compare is linear, ahead by six commits and behind by zero.

Final changed paths are limited to:

- docs/reference/SOURCE_SYMBOL_DICTIONARIES.md;
- mk/issue7-clean.mk;
- scripts/check-clean-ps2-compile.sh;
- src/config/SYMBOLS.md;
- src/config/mpeg_runtime_profile.c;
- src/config/mpeg_runtime_profile.h;
- tests/Makefile;
- tests/unit/config_mpeg_runtime_profile_test.c.

No Application, Transport runtime, MPEG mechanism, Display mechanism or Pi
product runtime source changed.

## Independent R7 acceptance

Foreman independently confirmed:

- one Config-owned static-const selected MPEG runtime profile;
- exact reuse of existing owner value types;
- exact Transport 524288/524288/8192/1/1 values;
- exact decoder 704x480/2/2048/16/64 values;
- exact worker 65536/67 values;
- exact PS2 worker-runtime 1000/3000 values;
- exact scheduler 30000/1001/drop-off/zero-threshold values;
- copy-safe immutable publication;
- unchanged production CONFIG wire version/header/entry/count/highest-ID;
- absence of H1 lab-only tuning fields;
- decoder-resource versus active geometry authority remains separated;
- existing media-clock presentation offset is not duplicated;
- no live activation call graph;
- final direct-compile script mode restored to 100755;
- strict Config dictionary coverage is coherent.

Disposition:

A003-R7-C1 CONFIG_OWNS_SELECTED_MPEG_PROFILE — MET
A003-R7-C2 EXACT_TRANSPORT_VALUES — MET
A003-R7-C3 EXACT_DECODER_VALUES — MET
A003-R7-C4 EXACT_WORKER_EXECUTION_VALUES — MET
A003-R7-C5 EXACT_SCHEDULER_VALUES — MET
A003-R7-C6 EXISTING_OWNER_TYPES_REUSED — MET
A003-R7-C7 CONFIG_WIRE_NOT_BROADENED — MET
A003-R7-C8 LAB_KNOBS_NOT_PRODUCTIZED — MET
A003-R7-C9 GEOMETRY_OWNERSHIP_PRESERVED — MET
A003-R7-C10 NO_LIVE_ACTIVATION — MET
A003-R7-C11 FOCUSED_TESTS_AND_STRICT_DICTIONARIES — MET
A003-R7-C12 CLEAN_BUILD_EVIDENCE — MET

FOREMAN_DISPOSITION=ACCEPT

## Machine evidence

Final pre-log workflow 35597128137 at 5cdd5670... completed SUCCESS on attempt
1. Direct log inspection confirms:

- transport_runtime_test: PASS;
- config_profile_test: PASS;
- config_mpeg_runtime_profile_test: PASS;
- MPEG_WORKER_TEST=PASS;
- APP_MPEG_FRAME_TEST=PASS;
- SOURCE_DICTIONARIES=PASS;
- SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS;
- SOURCE_TOPOLOGY_CONTRACT=PASS;
- SOURCE_DICTIONARY_PORTAL_SYNC=PASS;
- PS_TO_VNC_PROJECT_CHECK=PASS;
- PS2_COMPILE=src/config/mpeg_runtime_profile.c;
- CLEAN_PS2_COMPILE_CHECK=PASS;
- ISSUE7_LINKED_BUILD=PASS;
- LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS.

Worker-log head workflow 35597326282 also completed SUCCESS.

Evidence classification:

SOURCE_COMPLETE=YES_WITHIN_R7
HOST_TESTED=YES
PS2_COMPILE=PASS
PS2_LINK_CURRENT_SOURCE_REPRODUCIBILITY=PASS
REPOSITORY_PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
INDEPENDENT_VALIDATION_R7=NOT_RUN
PHYSICAL_PS2_PI_R7=NOT_RUN_NOT_CLAIMED
HARDWARE_QUALIFICATION_R7=NOT_TRANSFERRED

Historical H1 hardware evidence remains provenance only.

## Architecture / next dependency reasoning

Current Q1-Q12 authority is sufficiently settled to build the Pi physical Wire
owner without designing Application MPEG lifecycle.

Current installed Pi RFB socket/provider remains pre-mux qualified deployment
evidence, not target Wire architecture. It must not be destructively repurposed
inside this packet.

The proof branch already demonstrates the minimal Q4 establishment mechanism,
but its source is explicitly experiment apparatus. The mature product now needs:

- a real maintained Pi runtime source home;
- a persistent systemd-supervised Wire server;
- current 192.168.50.1:5902 endpoint ownership;
- exact product HELLO/ACCEPT/NOT_ACCEPTED representation;
- provisional -> ACTIVE session transition;
- authoritative Pi session identity;
- one physical recv/send owner;
- idle-session validity and repeated finite sessions;
- fail-closed reproducible staging.

The smallest coherent packet stops there. Adding RFB ingress, MPEG control or
producer lifecycle simultaneously would force the worker to design multiple
rider boundaries while establishing the containing physical owner.

## Packet publication

Foreman State advances:

STATE_REVISION=0035 -> 0036

Active packet:

A003-PI-WIRE-SERVER-ESTABLISHMENT-R8

The packet deliberately productizes ACCEPT kind 12 and NOT_ACCEPTED kind 13,
the exact proof-backed Q4 payload forms, and top-level pi/ as the maintained
installed companion runtime source root.

It explicitly forbids live Pi activation, ordinary riders, MPEG control,
producer lifecycle, PS2 establishment wiring and Application activation.

## Evidence boundary / pending work

PENDING_LOCAL=NONE_FOR_FOREMAN_PUBLICATION
A003_R7_FOREMAN_ACCEPTED=YES
A003_R7_INDEPENDENT_VALIDATION=NOT_RUN
A003_R8_PI_WIRE_ESTABLISHMENT_FOUNDATION=ACTIVE_RECONSTRUCTION_PACKET
PI_PRODUCT_WIRE_SERVER=NOT_YET_IMPLEMENTED_AT_START_OF_R8
PI_MPEG_CONTROL_OWNER=NOT_IMPLEMENTED
PI_MPEG_PRODUCER_OWNER=NOT_IMPLEMENTED
PS2_PRODUCT_Q4_ESTABLISHMENT_CALL_GRAPH=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
Q7_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
WIRE_LOSS_MPEG_RECOVERY=NOT_IMPLEMENTED
PHYSICAL_HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes:

A003-PI-WIRE-SERVER-ESTABLISHMENT-R8

and emits exactly one immutable Reconstruction log using:

- ROLE_KEY=reconstruction;
- WORK_ITEM_KEY=a003-mpeg-generation;
- WORKER_KEY=interactive.

Do not begin Pi MPEG control/producer work, RFB-to-Wire migration,
PS2/Application Q4 wiring or physical qualification in the same shift.
