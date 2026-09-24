DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T13:10:21-04:00
COMPLETED_AT=2026-09-24T13:34:04-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=05f8dd5cbcacbc46510f2d04f229e593def6601b
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R21 Application MPEG run-start transaction

## Objective and authority

This interactive Reconstruction worker recovered live authority from
`ledge/h1-all-guns` and consumed Foreman State revision `0056`, whose active
packet was:

`A003-APPLICATION-MPEG-RUN-START-R21`

The assigning branch authority was:

`05f8dd5cbcacbc46510f2d04f229e593def6601b`

R21 required one trigger-agnostic Application-owned start transaction that
composes already-accepted P2/P3/P7/R18/R20/R3/R4/R5/R7 seams, allocates one
fresh session-local run generation, and invokes START only as the final startup
action. Ordinary product activation, retirement and `src/app.c` were expressly
outside this packet.

The worker also consumed accepted cumulative R20C/R20D/R20E reclaim authority
at `316ad217bef229c9ca0134b9b922a213cb5af247` and immutable R20E closeout
`5357e7ed422bb402f187a2887ad0e67bcc710e4a`.

External Pi worktree cleanliness is not observable through the GitHub
connector. No claim is made about external staged/unstaged/untracked state.

## Result

R21 is source-complete as a Reconstruction candidate for independent Foreman
review at final pre-log authority:

`fc8bb2652d94d8163e1e5b375e33c9e37b73017b`

New Application support source:

- `src/app_mpeg_run.c`
- `src/app_mpeg_run.h`

The coordinator is deliberately dormant from the ordinary product loop.
`src/app.c` is unchanged.

### Successful transaction

One successful start performs this exact ordering:

1. obtain/copy the selected R7 MPEG runtime profile;
2. validate already-resolved base/inner/suppression geometry and decoder profile
   bounds before activation;
3. require the supplied P2 flow policy already to deny remote publication;
4. require P3 Presentation to be RFB_ONLY;
5. allocate a new nonzero monotonically increasing session-local generation;
6. open one clean R18 Transport run;
7. initialize one fresh R5 PS2 worker runtime and obtain its operation tables;
8. initialize one fresh R3 PS2 decoder backend and obtain platform operations;
9. start one fresh R4 worker with the exact generation/profile;
10. arm P3 WAIT_FIRST_FRAME and read back the exact immutable
    geometry/generation snapshot;
11. initialize the P7 frame consumer against that worker, Presentation,
    generation, media clock and selected scheduler profile;
12. construct R20 START from the retained P3 snapshot and invoke START last.

Successful START leaves the coordinator in
`PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME`.

The START payload contains generation plus exact base and suppression geometry.
The inner-content matte is retained by Presentation and is never serialized in
START.

### Generation authority

`last_allocated_generation` is session-local coordinator authority. Allocation
occurs once per admitted start attempt and is never rolled back after pre-START
failure. A later attempt receives the next value. `UINT32_MAX` exhaustion
faults without wrapping to zero.

Reinitializing a coordinator during the same Wire Session is outside the R21
contract; one coordinator instance represents one session-local allocation
history.

### Pre-START unwind

Any failure before START attempts reverse-order cleanup:

- clear the not-yet-serviced P7 value;
- abort pending P3 WAIT_FIRST_FRAME ownership;
- request R4 worker stop;
- require exact worker join;
- require exact worker outcome;
- release R4 worker ownership;
- release R5 PS2 runtime resources;
- call R18 pre-START abort last.

The coordinator returns to IDLE only if all acquired ownership is proven
retired. If any cleanup proof fails, the coordinator enters FAULTED with
`session_teardown_required=1`; it does not pretend the generation or owners
are reusable.

### START boundary

START invocation is the irreversible R21 boundary. Once
`pstvnc_transport_mpeg_send_start()` has been called, a non-OK return cannot
prove that START bytes were absent. The coordinator therefore faults, requires
outer session teardown, and never calls R18 pre-START abort or local pre-START
cleanup after that invocation.

## Scope deliberately not entered

R21 does not:

- modify or invoke ordinary `src/app.c` product flow;
- choose a user gesture or calibration trigger;
- freeze or thaw P2;
- service any MPEG frame;
- perform first-frame presentation;
- retire a successful run;
- send RETIRE;
- publish producer completion;
- stop/join a successfully started live run;
- finalize R18;
- reveal RFB or schedule post-thaw FULL refresh;
- activate the Pi MPEG product path;
- modify AUDIO, RFB parser/session/flow-policy, Transport mechanism, decoder,
  worker, Presentation, Configuration or protocol-wire product source.

## Focused deterministic evidence

`tests/unit/app_mpeg_run_test.c` proves:

1. initial allocation is generation 1 and exact generation is shared by worker,
   P3, P7 and START;
2. failed pre-START generations are never reused and a later attempt increments;
3. `UINT32_MAX` exhaustion cannot wrap to zero;
4. malformed/out-of-profile geometry and unprotected P2 state fail before R18
   run-open;
5. START base/suppression exactly match the P3 snapshot while inner matte stays
   Presentation-only;
6. ordering is R18 open < R5/R3/R4 worker < P3 snapshot < P7 init < START;
7. failures at run-open, runtime init, runtime ops, backend ops, worker start,
   Presentation arm, Presentation snapshot and P7 init never invoke START and
   exercise reverse-order cleanup;
8. cleanup failure faults and blocks retry on the same coordinator;
9. successful START is the final event and leaves WAIT_FIRST_FRAME-ready state;
10. START-call failure faults, requires teardown and never invokes pre-START
    abort.

Existing host suites remain independently green for P2, P3/P7, R18/R20,
Transport reclaim, R3/R4/R5 mechanisms and ordinary R15/R16B/R19 Application
behavior.

## Build/topology integration

The new coordinator is explicitly enrolled in:

- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `tests/Makefile`;
- root clean-source topology authority.

`docs/development/source-topology.md` and
`scripts/continuity-check.sh` now admit `app_mpeg_run.{c,h}` at the root
because it is an Application-owned cross-domain coordinator, analogous to the
already-admitted P7 frame coordinator rather than a new private mechanism
domain.

## Commits

The complete pre-log R21 range is seven commits ahead / zero behind the
assigning Foreman authority:

1. `67ba160257261ffa0fcc880448c7b60b71032648` —
   `app: add trigger-agnostic MPEG run start transaction`
2. `1f1172d8d7af4133c77e359892fb07dec08f4f48` —
   `test(app): prove R21 MPEG run start transaction`
3. `be5acd11fbf62b7224fdb382ceb6027968b1a0ca` —
   `app: correct R21 result and scheduler fixture`
4. `bb45f3f929ed060bf977a0dad47a24d5c5e69c1f` —
   `docs(app): record R21 MPEG run start authority`
5. `bc90457e73f5af627768843e322ebe0863700ebe` —
   `tooling(symbols): run deterministic dictionary reconciliation`
6. `64cee5fbca3d567a8392cbaafc55ff1050cc996b` —
   `docs(symbols): reconcile current clean definitions`
7. `fc8bb2652d94d8163e1e5b375e33c9e37b73017b` —
   `test: verify final R21 MPEG run start authority`

## Changed paths

The complete R21 diff is confined to:

- `src/app_mpeg_run.c`
- `src/app_mpeg_run.h`
- `tests/unit/app_mpeg_run_test.c`
- `tests/Makefile`
- `mk/issue7-clean.mk`
- `scripts/check-clean-ps2-compile.sh`
- `scripts/continuity-check.sh`
- `docs/development/source-topology.md`
- `docs/development/mpeg-generation-control.md`
- `src/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

No existing lower-layer product implementation changed.

## Acceptance-criterion worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A003-R21-C1 APPLICATION_OWNS_MONOTONIC_SESSION_LOCAL_RUN_GENERATION` — MET.
- `A003-R21-C2 ACCEPTED_GEOMETRY_HAS_ONE_BASE_INNER_SUPPRESSION_AUTHORITY` — MET.
- `A003-R21-C3 START_BASE_AND_SUPPRESSION_MATCH_PRESENTATION_SNAPSHOT_EXACTLY` — MET.
- `A003-R21-C4 EXISTING_RFB_PROTECTION_IS_REQUIRED_NOT_STOLEN_OR_RELEASED` — MET.
- `A003-R21-C5 R18_RUN_OPEN_PRECEDES_MPEG_WORKER_CONSUMER_ACTIVITY` — MET.
- `A003-R21-C6 FRESH_R5_R3_R4_EXECUTION_OWNERS_BIND_EXACT_GENERATION` — MET.
- `A003-R21-C7 P3_WAIT_FIRST_FRAME_AND_P7_CONSUMER_READY_BEFORE_START` — MET.
- `A003-R21-C8 START_IS_FINAL_IRREVERSIBLE_STARTUP_ACTION` — MET.
- `A003-R21-C9 EVERY_PRE_START_FAILURE_PROVES_REVERSE_ORDER_UNWIND_OR_FAULTS` — MET.
- `A003-R21-C10 START_ATTEMPT_FAILURE_NEVER_FALSELY_USES_PRE_START_ABORT` — MET.
- `A003-R21-C11 NO_RETIREMENT_PI_TRIGGER_AUDIO_OR_ORDINARY_APP_SCOPE_CREEP` — MET.
- `A003-R21-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

## Exact final machine evidence

Final pre-log source authority:

`fc8bb2652d94d8163e1e5b375e33c9e37b73017b`

Canonical workflow:

- workflow: `Ledge reconstruction checks`
- run: `36035059916`
- attempt: `1`
- conclusion: `success`
- head SHA: `fc8bb2652d94d8163e1e5b375e33c9e37b73017b`

Observed job evidence:

- `host-unit` — SUCCESS
  - `transport bridge tests passed`
  - `transport_runtime_test: PASS`
  - `transport_mpeg_test: PASS`
  - `RFB_FLOW_POLICY_TEST=PASS`
  - Pi MPEG generation fixture: 12 tests, OK
  - `app R15/R16B/R19 tests: PASS`
  - `app_mpeg_run_test: PASS`
- `project-check` — SUCCESS
  - `SOURCE_TOPOLOGY_CONTRACT=PASS`
  - `WORK_LOG_CHECK=PASS`
  - `SOURCE_DICTIONARIES=PASS`
  - `PS_TO_VNC_PROJECT_CHECK=PASS`
- `dictionary-long` — SUCCESS
  - `SOURCE_DICTIONARY_CHECK_MODE=LONG`
  - `SOURCE_DICTIONARIES=PASS`
- `ps2-compile` — SUCCESS
- `ps2-link` — SUCCESS
  - `ISSUE7_LINKED_BUILD=PASS`
  - `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`
- `dictionary-reconcile` — SKIPPED as expected on the final non-trigger
  evidence commit.

Final linked identity:

- `ELF_PRISTINE_SHA256=82ea220b084d50a857957f68950c10e8e7eab6a23e4857ab923e61f713c03ac8`
- `PT_LOAD_SEGMENTS=1`
- `PT_LOAD_SHA256=910597af49fe2121ab730a2db82aa4259177256156062853bd2a008f0d4d6045`
- `PT_LOAD_BYTES=494996`
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R21 changes loadable bytes from accepted cumulative R20E authority. The new
identity is repository-reproducible evidence only and is not physically
qualified.

## Intermediate evidence observations

The first R21 test/build head `1f1172d...` exposed only R21-local compile
defects: one duplicate enum constant name and two focused-test scheduler field
names that did not match the accepted profile type. They were corrected at
`be5acd1...`.

That corrected source passed host-unit, PS2 compile and PS2 link. Project-check
then correctly rejected the newly authorized root source until the canonical
root topology allowlist was updated.

After the topology/documentation update at `bb45f3f...`, project-check reached
only missing local dictionary coverage. The deterministic reconciliation trigger
then generated the complete root dictionary and portal. No behavior source was
changed by reconciliation.

## Evidence gaps / hardware debt

- `PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR` — external Pi worktree
  cleanliness/staging state was not observable.
- `R21_INDEPENDENT_VALIDATION=NOT_RUN`
- `R21_OPERATOR_OBSERVED=NO`
- `R21_HARDWARE_QUALIFIED=NO`
- `HARDWARE_PENDING=YES`

Machine evidence does not imply physical PS2/Pi qualification.

## State/contract accounting

Consumed:

- Foreman State revision `0056`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- accepted cumulative R20C/R20D/R20E Transport reclaim authority;
- accepted P2/P3/P7/R18/R20/R3/R4/R5/R7 seams.

Produced:

- no Foreman state revision;
- no contract revision;
- no Wire/protocol version revision;
- no ordinary product activation;
- no packet selection or Foreman acceptance.

## Findings / blockers

R21 has no remaining known source blocker within the authorized packet.

A successful R21 transaction intentionally stops at
STARTED_WAIT_FIRST_FRAME. Retirement, first-frame service, RFB reveal/thaw,
Transport RETIRE/finalize and product trigger integration remain later
authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R21_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect this source/evidence/log, decide R21
acceptance, publish Foreman-owned state/integration evidence, and select any
next bounded packet. Reconstruction must not self-accept R21 or begin R22 from
this record.
