# Ledge Foreman work log — accept R34 and issue partial-retirement dormancy prerequisite

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T09:55:29-04:00
COMPLETED_AT=2026-09-26T10:03:32-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=12ef69048c80a7abed36e76217203eac3027b3db
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R34C ordinary MPEG activation composition.

Live pickup authority was:

`12ef69048c80a7abed36e76217203eac3027b3db`
— `docs(work-log): record completed R34C activation composition`.

The round independently recovered:

- current Foreman State revision 0075;
- the exact R34C immutable Reconstruction closeout;
- the exact 12-commit continuation range from assigning Foreman authority;
- the complete original R34 composition accumulated across the initial partial
  work, accepted prerequisite R34P and R34C;
- accepted R16B, R21-R24, R28-R33 and R34P owner contracts;
- exact source-head and log-head canonical GitHub Actions evidence;
- A005/A006 interaction/orchestration authority and module-lifecycle rules.

No user terminal or hardware action was required.

## R34C returned range

Assigning Foreman/log authority:

`5d4a5035fd97b5023c7acd15967a93823853879d`.

Final pre-log R34C source/document authority:

`ec99a1fa0276a9add773d81dfe0e82edcdd42748`.

Immutable Reconstruction closeout:

`12ef69048c80a7abed36e76217203eac3027b3db`.

Independent compare proves the continuation source range is exactly:

- 12 commits ahead;
- 0 behind.

Behavior-bearing source is confined to the authorized R34 continuation surface:

- `src/app.c`;
- `src/app_mpeg_product.c`;
- `src/app_mpeg_product.h`;
- focused Application/product tests.

The remaining changed files are exact symbol/dictionary reconciliation.

No accepted R34P/R33 run/worker source, P9/P10 source, Input, UI, Configuration,
Management, Transport, RFB, Display/compositor, media, Pi, AUDIO or H1/B4A
forensic implementation changed in R34C.

## Independent R34C source findings

The final production source now has two distinct Application MPEG predicates.

`pstvnc_app_mpeg_product_has_started_run()` remains a strict genuine-live owner
predicate requiring:

- nonzero current generation;
- open Transport MPEG run;
- PS2 worker-runtime ownership;
- started worker;
- initialized P7 frame consumer;
- armed presentation;
- START already invoked.

Only that predicate gates current-tick R22 live service.

A separate public seam,
`pstvnc_app_mpeg_product_requires_session_abort()`, reads accepted
`pstvnc_app_mpeg_run_status()` and means:

"this attempt has a nonzero MPEG lifecycle generation that must reach abnormal
local dormancy before enclosing Transport release."

It returns true for:

- a healthy genuine post-START run;
- accepted post-START teardown-required R33 state;
- accepted pre-START teardown-required R34P state, including partially reclaimed
  local ownership while the old run still owes final session close.

It returns false for clean IDLE/current-generation-zero rollback.

`retire_attempt_owners()` uses this abnormal-owner predicate rather than the
live-service predicate.

The resulting exact outer teardown sequence for a dependent MPEG owner is:

1. Input shutdown/dormancy;
2. Transport begin-abort;
3. exact retained old-session proof inside R33/R34P local service;
4. repeated proof-driven local service until SESSION_ABORT_READY;
5. media-clock binding release;
6. final Transport session close;
7. only then replacement admission.

A failed Input shutdown prevents begin-abort and replacement.

A true no-MPEG attempt continues to use accepted R16B one-shot
`pstvnc_transport_session_abort()`.

A pre-START teardown-required R21 attempt never enters current-tick R22 service
merely because it has a generation.

## Integrated original R34 review

The original R34 packet was re-reviewed as one complete product change rather
than accepting R34C in isolation.

The full reconstruction history was:

- assigning Foreman log:
  `39eef5de7061c43e76ad1a19d1d12f944b949106`;
- initial partial source:
  `6f1acf56d0007a05c081c0217f099a5fcbc4a754`;
- initial blocked closeout:
  `488c6cc9c0e8c8d36d2bc44c94dc8b3c78cddbe4`;
- accepted lower prerequisite R34P:
  `c6e1dcf514adfc29296633374e75020b3480e5e0`;
- final R34C source:
  `ec99a1fa0276a9add773d81dfe0e82edcdd42748`;
- final R34 closeout:
  `12ef69048c80a7abed36e76217203eac3027b3db`.

Independent final criterion disposition:

- A006-R34-C1=MET
- A006-R34-C2=MET
- A006-R34-C3=MET
- A006-R34-C4=MET
- A006-R34-C5=MET
- A006-R34-C6=MET
- A006-R34-C7=MET
- A006-R34-C8=MET
- A006-R34-C9=MET
- A006-R34-C10=MET
- A006-R34-C11=MET
- A006-R34-C12=MET

Independent findings supporting those dispositions:

1. The immutable resident R32 configured binding snapshot is installed into each
   fresh R29 Input runtime before worker start. Zero binding remains valid and
   no compiled/default physical chord exists.
2. DESKTOP eligibility is recomputed from real UI foreground/quarantine and
   calibration/run ownership; held gestures cannot gain DESKTOP authority merely
   because context changes.
3. Each physical attempt owns fresh Transport access, P3 presentation, P9
   calibration and R21 run state. Wire establishment alone does not start MPEG.
4. Application routes only semantic
   `PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION`; it contains no physical mask,
   hold-duration or chord logic.
5. P9 receives controller state first while ACTIVE. Consumed calibration samples
   do not fall through to ordinary local-controller routing. Cancel does not
   invoke P10.
6. Protected accept invokes P10 once. Clean rollback may return to ordinary
   idle; uncertain/pre-START ownership fails closed and is handled by accepted
   R34P without false thaw.
7. R22 current-tick service remains limited to a genuinely started run.
   First physical MPEG presentation remains the sole P3/common-clock arm
   boundary.
8. Active calibration or live MPEG prevents overlapping generation start.
9. P2 remains the sole RFB request/publication protection owner throughout
   calibration/protected/live MPEG.
10. Session failure now composes exact post-START R33, pre-START R34P or true
    no-MPEG R16B semantics before replacement.
11. No normal R23/R24 same-session retirement/reveal, live recalibration, AUDIO,
    config persistence/editor/reload, default binding, Pi/Wire producer change,
    generic timeout/watchdog or hardware-success claim entered R34.
12. Exact final source and log heads are green under all canonical gates.

R34 is therefore Foreman-accepted.

## Exact machine evidence

### Final source head

GitHub Actions run:

`36231587440`

independently confirms:

- event: push;
- branch: `ledge/h1-all-guns`;
- exact head:
  `ec99a1fa0276a9add773d81dfe0e82edcdd42748`;
- attempt: 1;
- conclusion: SUCCESS.

Successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

Observed host/project/build evidence includes:

- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_TEST=PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact accepted linked identity:

`ELF_PRISTINE_SHA256=96137519a4cde6f984199fbaf893a660945cefa8cd85dbd729b8eddaaf54e7cc`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=e90c2f8de4729ca2c273a6cc81edbb4806e851cd3000e938f0c65efa8e8851a6`

`PT_LOAD_BYTES=525844`.

### Immutable log head

GitHub Actions run:

`36231794981`

at exact closeout head
`12ef69048c80a7abed36e76217203eac3027b3db`
also completed SUCCESS with host-unit, project-check, dictionary-long,
ps2-compile and ps2-link all successful.

Evidence classification:

- R34_SOURCE_COMPLETE=YES
- R34_FOREMAN_ACCEPTED=YES
- R34_HOST_TESTED=YES
- R34_PROJECT_CHECK=PASS
- R34_STRICT_DICTIONARIES=PASS
- R34_PS2_COMPILE=PASS
- R34_PS2_LINK=PASS
- R34_CURRENT_SOURCE_REPRODUCIBILITY=PASS
- R34_MACHINE_EVIDENCE=GITHUB_ACTIONS
- R34_INDEPENDENT_VALIDATION=NOT_RUN
- R34_OPERATOR_OBSERVED=NO
- R34_HARDWARE_QUALIFIED=NO
- R34_HARDWARE_PENDING=YES

The R34 identity is now the newest fully Foreman-accepted product source/build
identity, but physical qualification remains outstanding.

## Newly discovered dependency before ordinary retirement composition

The previously queued next product behavior was normal same-session MPEG
retirement/restoration/reveal using accepted R23/R23C/R24.

Before assigning that product composition, this round independently inspected
the accepted R23/R24 failure fixtures and current R33 abnormal-session admission.

A real lower-owner gap exists.

Accepted R23 already proves reachable FAULTED prefixes such as:

- joined worker with outcome failure;
- worker release failure after P7 has been cleared;
- PS2 worker-runtime release failure after worker release;
- Transport MPEG finalization failure after worker and runtime release.

Accepted R24 also proves nonretryable restore/reveal contradictions after normal
execution owners have already been retired.

R33's current post-START abnormal-session validator still requires the original
full live shape:

- Transport MPEG run open;
- worker runtime owned;
- worker started;
- P7 initialized;
- P3 armed;
- START invoked.

Therefore it cannot consume every legitimate monotonic R23/R24 partial-retirement
prefix.

Directly wiring ordinary product retirement before closing this gap would allow
a normal retirement failure to request enclosing-session replacement without a
complete local-dormancy proof path.

This is a lower Application-MPEG-run owner deficiency, not a reason to broaden
Transport or to patch ordinary `src/app.c` around it.

## Foreman state write

Foreman State advanced from revision 0075 to revision 0076 in:

`2967b37e9df02f24a260dd68c1b97a456372ea7e`
— `docs(ledge): accept R34 and issue retirement dormancy prerequisite`.

Revision 0076:

- accepts the complete original R34 packet;
- records its exact source/log authorities and accepted linked identity;
- makes R34 the newest fully Foreman-accepted product image;
- records the newly exposed R23/R24 partial-retirement abnormal-dormancy gap;
- publishes exactly one lower-owner prerequisite;
- keeps ordinary action-driven retirement and auto-recalibration deferred.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A003-MPEG-PARTIAL-RETIREMENT-SESSION-DORMANCY-R35P`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a003-mpeg-generation`
`WORKER_KEY=interactive`.

R35P must extend abnormal local session dormancy to exact reachable R23/R24
partial-retirement failure prefixes without replaying normal retirement.

The packet must preserve:

- retained Transport proof before local reclaim;
- exact P7 abandonment only when a borrow is represented;
- truthful already-joined/finished worker ownership;
- runtime-only cleanup when the worker is already gone;
- transport-run-only old-session state without retrying normal finalize;
- post-finalize nonretryable R24 fault cleanup with no fabricated execution
  owner;
- retryable R24 PLATFORM_FAILED/SYNC_INVALID as same-session retryable states;
- existing R33 and R34P semantics.

R35P explicitly may not wire ordinary user/product retirement.

## Pending dependency order

Current dependency order is:

1. R35P — partial normal-retirement abnormal-session dormancy;
2. ordinary Application product retirement/restoration/reveal composition using
   accepted R23/R23C/R24;
3. only after that, decide whether one-action automatic recalibration is needed
   or whether a fresh semantic calibration action from restored desktop is the
   desired final policy.

AUDIO activation and config persistence/editor/live reload remain deferred.

Independent Validation, operator observation and hardware qualification remain
not run/not claimed.

## Next pickup

The permanent Interactive Reconstruction Worker must recover current live
repository authority and execute only:

`A003-MPEG-PARTIAL-RETIREMENT-SESSION-DORMANCY-R35P`.

It must not enter ordinary product retirement. At completion or block it must
emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A003-MPEG-PARTIAL-RETIREMENT-SESSION-DORMANCY-R35P
