DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T19:33:52-04:00
COMPLETED_AT=2026-09-25T19:42:56-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=6011811a9f9778eca4a603b5679e2929e4649ebc
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R32 accepted; Transport/MPEG session-abort fence R33 activated

## Pickup and authority

This Foreman round began from live branch authority:

`6011811a9f9778eca4a603b5679e2929e4649ebc`

message:

`docs(work-log): record R32 resident binding snapshot`.

The live repository already contained Foreman State revision 0071 and the
completed R32 Reconstruction handoff. No stale/incomplete prior chat write was
used as authority.

Consumed governing/relevant authority included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/status.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Foreman State revision 0071;
- B10/B11 Configuration/Management audit authority;
- A003 MPEG lifecycle audit revision 0001;
- A004 Presentation/Calibration audit revision 0001;
- A005 Input authority and A006 Application orchestration authority;
- Wire Runtime Decisions revision 0011;
- Architecture Overlay revision 0007;
- current module-lifecycle contract;
- accepted R20E Transport receiver/outbound drain fences;
- accepted R21-R24 MPEG start/live/retire/reveal authority;
- accepted R29-R31 Input/Config/Management composition;
- accepted R32 worker source and immutable log.

Newest Reconstruction handoff consumed:

`docs/ledge/work-log/20260925T181735-0400__reconstruction__a005-interaction-input__interactive.md`.

## Exact R32 source range

Assigning Foreman log authority:

`410846c0826b988567c1f98987e980bd097a2e2b`

Final pre-log R32 source authority:

`e18e0170d009093514768d9ea5e4c58b344582e5`

Immutable Reconstruction closeout:

`6011811a9f9778eca4a603b5679e2929e4649ebc`

Independent GitHub compare proves exactly sixteen pre-log commits ahead / zero
behind, followed by exactly one log-only commit.

The source range is confined to:

- `src/app_product_bindings.c/.h`;
- one ordinary resident-start acquisition call in `src/app.c`;
- focused Application tests/build enrollment;
- root Application dictionary/topology documentation.

No R30 Configuration implementation, R31 Management implementation, Input
runtime/resolver, local UI/controller, RFB, Transport, MPEG, media clock, AUDIO,
Platform, Pi product/persistence or forensic source changed.

## Independent R32 review

R32 introduces one Application-owned desired snapshot containing the exact R30
typed model plus acquisition status.

The helper starts from a completely zeroed candidate before Management access.

R31 retrieval failure:

- invokes no parser;
- publishes exact zero desired authority;
- records FETCH_FAILED_ZERO;
- remains nonfatal to ordinary startup.

Successful retrieval:

- passes the exact returned byte pointer/count to R30;
- adds no terminator;
- performs no strlen/whitespace/syntax interpretation in Application.

R30 parse failure:

- preserves the pre-zeroed model;
- records INVALID_CONFIG_ZERO;
- remains nonfatal.

A valid parsed binding is retained exactly. Empty valid input remains VALID with
zero bindings.

Raw document storage is fixed automatic 4096-byte state with a compile-time bound
check against R30. No heap was added.

Ordinary Application acquires exactly once after network/link readiness and
before the first PSTV connection attempt. The snapshot lives outside the
provider replacement loop and therefore is not refetched on R16B replacement.

R32 still performs no Input binding install and has no PRODUCT_ACTION dispatch.
A configured nonzero desired binding therefore remains unreachable in ordinary
Input.

No R32 product defect was found.

## R32 acceptance criteria

B10-B11-R32-C1=MET
B10-B11-R32-C2=MET
B10-B11-R32-C3=MET
B10-B11-R32-C4=MET
B10-B11-R32-C5=MET
B10-B11-R32-C6=MET
B10-B11-R32-C7=MET
B10-B11-R32-C8=MET
B10-B11-R32-C9=MET
B10-B11-R32-C10=MET
B10-B11-R32-C11=MET
B10-B11-R32-C12=MET

`R32_DISPOSITION=FOREMAN_ACCEPTED`

## Exact R32 machine evidence

Exact final source:

`e18e0170d009093514768d9ea5e4c58b344582e5`

Canonical workflow:

`Ledge reconstruction checks`
run `36196725652`
attempt `1`
conclusion `success`.

The run object binds that evidence to exact R32 source SHA and branch.

Observed focused/regression evidence includes:

- `APP_PRODUCT_BINDINGS_SOURCE_TEST=PASS`;
- `APP_PRODUCT_BINDINGS_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_SOURCE_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27/R32 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `transport_mpeg_test: PASS`.

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=226 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned PS2 compile explicitly compiled `src/app.c` and
  `src/app_product_bindings.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=cf7db4dfd05c8aa0ae8a2b2df94bd279ec36addfbc8e9569e486532a06ca94ff`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=c9565c3c55cd758624967ee9cb4b5824ad73bc6f69543d5adb0f1c0b5be2429c`
`PT_LOAD_BYTES=520852`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R32 remains hardware-pending and is not independently hardware-qualified.

## Dependency review: session abort must become two-phase before MPEG activation

Current ordinary provider recovery is safe only because no MPEG worker is
started.

The current Transport one-shot abort performs:

1. runtime request-stop;
2. terminal media wake publication + physical I/O shutdown;
3. receiver completion wait;
4. immediate runtime release, including MPEG queue/semaphore/storage.

Once an MPEG worker exists, steps 2 and 4 cannot remain inseparable.

The accepted decoder/worker design deliberately refuses synthetic EOF while an
MPEG_Picture call owns libmpeg. The correct wake for a blocked Transport-backed
feed path is therefore Transport terminality, not fabricated decoder data.

The runtime request-stop mechanism already publishes MPEG terminal activity and
shutdown state. Transport access becomes terminal when runtime stop is requested,
while the runtime memory still physically exists until release. That existing
mechanism is the correct foundation for a two-phase bridge API.

The lifecycle contract requires:

`Wire terminal -> old module stop -> complete local retirement -> Transport
storage release -> replacement startup`.

A replacement must never be permitted merely because the receiver stopped.

## R33 architecture decision

The next packet is:

`A003-TRANSPORT-MPEG-SESSION-ABORT-FENCE-R33`.

R33 is deliberately abnormal-session cleanup, not normal user MPEG stop.

It must earn:

- a public Transport begin-abort/quiesce boundary that terminalizes old Wire
  access and proves receiver completion while retaining runtime storage;
- preservation of existing one-shot `session_abort()` semantics for current
  callers;
- an `app_mpeg_run` local session-abort lifecycle that releases any exact P7
  borrow, safely stops/joins the exact worker, preserves terminal outcome
  evidence, releases worker and PS2 worker runtime, and then reports local
  dormancy;
- final Transport release only after the old module no longer owns queue/waiter
  state.

The abnormal path must not send RETIRE, publish producer-done, finalize a normal
MPEG run, thaw P2, seal/reveal P3 or claim a Q7 restoration. The whole enclosing
session is being destroyed.

Successful local abort must remain terminal for that run object/session; it must
not return to reusable IDLE. Replacement uses fresh ordinary ownership.

Ordinary app.c wiring, desired binding installation and semantic action routing
remain deferred.

## Foreman State publication

Published Foreman State revision 0072:

`75109b71226afd7e3798e6d2f70f8b846228d61c`

message:

`docs(foreman): accept R32 and activate session abort fence`

State 0072:

- accepts all twelve R32 criteria;
- records exact R32 source, CI and new hardware-debt identity;
- activates exactly one bounded next packet:
  `A003-TRANSPORT-MPEG-SESSION-ABORT-FENCE-R33`;
- switches the next Reconstruction work item to `a003-mpeg-generation`;
- requires Transport terminalization before old MPEG local cleanup while
  retaining old runtime storage;
- requires module-local dormancy before final Transport release;
- preserves normal R21-R24 semantics unchanged;
- keeps ordinary Application wiring, Input binding installation and
  PRODUCT_ACTION routing deferred;
- claims no hardware qualification.

Immediately before this immutable Foreman log write, live branch authority
remained exactly at State 0072.

## Current hardware debt

Current fully Foreman-accepted PS2 loadable authority is R32:

`PT_LOAD_SHA256=c9565c3c55cd758624967ee9cb4b5824ad73bc6f69543d5adb0f1c0b5be2429c`
`PT_LOAD_BYTES=520852`

It is repository-reproducible and hardware-pending.

Accepted R25 maintained Pi product/runtime source remains:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

with no Pi hardware qualification claim.

R33 may alter linked Transport/Application MPEG lifecycle code and must record
any new exact identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A003-TRANSPORT-MPEG-SESSION-ABORT-FENCE-R33
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R33_FROM_FOREMAN_STATE_0072

The Reconstruction worker must independently recover live branch authority,
consume State 0072 plus Transport/module-lifecycle/R21-R24 authority, execute
only R33, create exactly one immutable Reconstruction record with
WORK_ITEM_KEY `a003-mpeg-generation`, stop, and return the baton.

Do not modify ordinary app.c, install desired bindings, route PRODUCT_ACTION,
start ordinary MPEG, add persistence/UI/AUDIO, weaken Transport release fences,
invent a timeout, or claim hardware qualification.
