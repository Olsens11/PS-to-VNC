DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T04:53:23-04:00
COMPLETED_AT=2026-09-26T05:05:27-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=5d4a5035fd97b5023c7acd15967a93823853879d
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R34C ordinary MPEG activation continuation

## Recovered authority

This shift independently recovered live `ledge/h1-all-guns` authority before
behavior-bearing writes.

Starting branch authority:

`5d4a5035fd97b5023c7acd15967a93823853879d`

message:

`docs(work-log): accept R34P and hand off R34 continuation`

Current Foreman State revision consumed:

`0075`

Exact active packet:

`PACKET_ID=A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34C`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a006-orchestration-shutdown`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`;
- `ORIGINAL_R34_CRITERIA=ALL_REOPENED_FOR_FINAL_ACCEPTANCE`;
- `NORMAL_MPEG_RETIREMENT_R23_R24=DEFERRED`;
- `ACTION_DRIVEN_RECALIBRATION=DEFERRED`;
- `AUDIO_ACTIVATION=DEFERRED`;
- `CONFIG_PERSISTENCE_EDITOR_RELOAD=DEFERRED`.

The packet was based on accepted R34P source
`c6e1dcf514adfc29296633374e75020b3480e5e0`, R34P log
`c5fdd7f1acdaa2dcb526a5113f2e538614f2d976`, blocked R34 partial source
`6f1acf56d0007a05c081c0217f099a5fcbc4a754`, and blocked R34 log
`488c6cc9c0e8c8d36d2bc44c94dc8b3c78cddbe4`.

The GitHub-connected Worker seat cannot observe an external Pi/local checkout's
untracked files, staging area or dirtiness. No local reset, clean, stash or
overwrite is claimed.

## Objective completed

R34C closes the exact Application-owned teardown-selection gap that remained
after R34P was independently accepted.

The ordinary product composition now distinguishes three meanings without
broadening any lower owner:

1. a genuinely started MPEG run eligible for current-tick R22 live service and
   requiring post-START R33 if the enclosing session dies;
2. a nonzero-generation teardown-required pre-START R21 owner requiring accepted
   R34P before retained Transport storage may be released;
3. a true no-MPEG/clean rollback attempt retaining accepted R16B one-shot
   Transport abort.

The already-landed R34 semantic activation/product composition remains the
implementation base. R34C adds only this missing distinction and its focused
proof.

No normal R23/R24 retirement, same-session reveal/reuse, recalibration, AUDIO,
persistence/editor/reload, Pi/Wire producer behavior or physical gesture/default
binding was added.

## Separate live-service and abnormal-teardown ownership

`pstvnc_app_mpeg_product_has_started_run()` remains unchanged in meaning.

It is still the exact live-service predicate used by ordinary `app.c` before
obtaining a session-clock tick and invoking
`pstvnc_app_mpeg_product_service_live()`. It requires the complete genuinely
started owner shape including:

- nonzero run generation;
- open Transport MPEG run;
- PS2 worker runtime ownership;
- started worker;
- initialized P7 frame consumer;
- armed presentation;
- `start_invoked=1`.

R34C does not treat a pre-START teardown-required generation as R22-live.

A separate product-level public seam now owns abnormal teardown selection:

`pstvnc_app_mpeg_product_requires_session_abort()`

It reads the accepted public `pstvnc_app_mpeg_run_status()` seam.

The predicate:

- rejects null/uninitialized product owners;
- rejects a zero current generation;
- returns true for a genuinely started run;
- otherwise returns true only while the nonzero-generation run reports
  `session_teardown_required`.

This means accepted R33/R34P teardown debt persists across partial local cleanup
retries, while clean R21 rollback remains excluded because the accepted clean
rollback path clears the current generation to zero.

The product abort-service entry now uses this abnormal-owner predicate rather
than reusing the live-service predicate.

## Ordinary Application teardown selection

`src/app.c::retire_attempt_owners()` now computes
`has_mpeg_abort_owner` through the new product seam.

Input retirement still occurs first.

If an abnormal MPEG teardown owner exists, an unproven Input shutdown blocks
dependent MPEG/Transport reclamation. No Transport begin-abort, local MPEG abort
service, final close or replacement admission is allowed until Input dormancy is
proven.

For either a post-START R33 owner or a pre-START R34P owner:

1. Input shutdown/dormancy is proven;
2. `pstvnc_transport_session_begin_abort()` terminalizes the exact old Wire
   Session while retaining its runtime storage;
3. `pstvnc_app_mpeg_product_service_session_abort()` services the accepted
   R33/R34P local owner until `SESSION_ABORT_READY`;
4. the session media-clock binding is released only after local MPEG dormancy;
5. `pstvnc_transport_session_close()` performs final retained Transport
   reclamation;
6. replacement may proceed only after that complete reverse ownership proof.

No timeout is interpreted as local dormancy.

A true no-MPEG attempt does not enter this two-phase path and retains the
accepted R16B one-shot `pstvnc_transport_session_abort()` behavior.

## Focused deterministic evidence

Product-unit evidence proves:

- a fresh clean R21 owner is neither live-serviceable nor abnormal-abort-owned;
- a healthy post-START owner is both genuinely started and abnormal-abort-owned;
- a FAULTED nonzero-generation pre-START teardown-required owner is not
  live-serviceable but is abnormal-abort-owned;
- the same teardown owner remains classified as abnormal after partial local
  reclamation;
- clean IDLE/current-generation-zero rollback has no abnormal teardown debt;
- the same product abort-service seam services both accepted R33 and R34P
  owners;
- R34P progress to `SESSION_ABORT_READY` does not transform a pre-START owner
  into a live-service owner.

Full Application evidence proves:

- post-START live failure still performs Input shutdown -> Transport begin-abort
  -> local R33 service -> media-clock release -> final Transport close;
- a scripted pre-START P10 teardown-required semantic-action failure performs
  Input shutdown -> Transport begin-abort -> local R34P service -> media-clock
  release -> final Transport close;
- that pre-START case performs no current-tick lookup and no R22 live service;
- pre-START abnormal ownership with failed Input shutdown cannot begin Transport
  abort or local MPEG teardown;
- a clean/no-MPEG provider failure still performs one-shot R16B Transport abort
  and does not call begin-abort/local MPEG service/final-close;
- existing R16B replacement, R19 flow, R27 clock, R32 binding and R34 semantic
  routing behavior remains green.

The R34 source-boundary test additionally enforces that:

- the ordinary live-service branch contains
  `pstvnc_app_mpeg_product_has_started_run()`;
- the live-service branch does not use
  `pstvnc_app_mpeg_product_requires_session_abort()`;
- `retire_attempt_owners()` uses the abnormal teardown predicate rather than
  the live-service predicate;
- abnormal teardown contains begin-abort -> local MPEG abort service -> final
  close;
- deferred normal retirement/reveal APIs remain absent from ordinary R34
  composition.

## Commit range

From starting authority
`5d4a5035fd97b5023c7acd15967a93823853879d`
to final pre-log source/document authority
`ec99a1fa0276a9add773d81dfe0e82edcdd42748`,
GitHub compare reports twelve commits ahead and zero behind:

1. `00227fa0b076d280423b8e18b1fad72247bd7a5e` —
   `app(mpeg): distinguish live service from abort ownership`;
2. `ba817f8e93395467717bf7f101909e955d75f8e7` —
   `app(mpeg): expose abnormal teardown owner predicate`;
3. `d0c271dde1f31dc9305ff98662c09a63c73f25d9` —
   `app: route pre-start teardown through retained-session abort`;
4. `87b9f06c6f5b558d0db22926b07e5888ef8f9c3b` —
   `test(app): prove R34C teardown owner distinction`;
5. `6f664624d4d4e2a546eebc4d9883d6120d706050` —
   `test(app): prove R34C pre-start teardown selection`;
6. `da05c24c85fdde91fe02a670a09a72b937505699` —
   `test(app): enforce distinct R34C teardown predicate`;
7. `01e64eff9587b2145f5b170afbbf3101da30a45a` —
   `docs(app): clarify R34C teardown ownership`;
8. `057cfdbad02b2bba91aa1db87d1f41f1e1eb5470` —
   `docs(app): document R34C public predicates`;
9. `e3f5a5f0036b3e6eb8f57160a5557be826187414` —
   `test(app): model started run as abort owner`;
10. `d3356b3c403a6a67a40c008f1ce7964336a69f65` —
    exact empty trigger
    `tooling(symbols): run deterministic dictionary reconciliation`;
11. `41ceff29652e2f5b6f04785ad403f0fc31454244` —
    automation-generated
    `docs(symbols): reconcile current clean definitions`;
12. `ec99a1fa0276a9add773d81dfe0e82edcdd42748` —
    exact empty evidence trigger
    `test(app): verify final R34C authority`.

One intermediate host run exposed a test-stub modeling defect: the new test stub
for abnormal ownership initially returned only the explicit pre-START fixture
flag, so the existing healthy started-run R33 scenario was incorrectly modeled
as having no abort owner. Commit `e3f5a5f...` corrected only the test stub so
started runs imply abnormal ownership, matching production. No production
semantic change was made for that test correction.

## Final changed-path scope

The complete start-to-pre-log changed path set is:

- `src/app.c`;
- `src/app_mpeg_product.c`;
- `src/app_mpeg_product.h`;
- `tests/unit/app_mpeg_product_test.c`;
- `tests/unit/app_test.c`;
- `tests/unit/app_mpeg_product_source_test.py`;
- reconciled `src/SYMBOLS.md`;
- generated/reconciled
  `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No accepted R34P/R33 run/worker source, P9/P10 product source, Input, UI,
Configuration, Management, Transport, RFB, Display/compositor, Media, Pi, AUDIO
or H1/B4A forensic source changed in this R34C range.

Two attempted broader living-document writes were rejected by the repository
write endpoint before commit creation; no repository state changed from those
attempts. The maintained source/header synopses were updated within the
authorized product surface, and canonical documentation/project checks are
green.

## Dictionary reconciliation

Before reconciliation, strict dictionary-long reported exactly seven new
symbols requiring description:

- `src/app.c:has_mpeg_abort_owner`;
- `src/app_mpeg_product.c:pstvnc_app_mpeg_product_requires_session_abort`;
- its source parameter/local status symbols;
- its public header declaration/prototype parameter.

No broader dictionary debt was present.

After behavior evidence was green, the exact repository-owned deterministic
dictionary trigger was used. Automation commit
`41ceff29652e2f5b6f04785ad403f0fc31454244` reconciled the new definitions and
portal. Final strict dictionary evidence is green.

## Canonical final evidence

Exact final pre-log authority:

`ec99a1fa0276a9add773d81dfe0e82edcdd42748`

Canonical GitHub Actions run:

`36231587440`

Run conclusion:

`SUCCESS`

Jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected on the final non-trigger commit.

Observed host output includes:

- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_SOURCE_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_SOURCE_TEST=PASS`;
- `MANAGEMENT_CONFIG_GET_TEST=PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- all broader observed host regressions PASS.

Project evidence includes:

- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `WORK_LOG_CHECK=PASS records=234 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

Strict long dictionary job:

`SOURCE_DICTIONARIES=PASS`.

Pinned PS2 compile explicitly includes the modified `src/app.c` and
`src/app_mpeg_product.c`, and reports:

`CLEAN_PS2_COMPILE_CHECK=PASS`.

Linked/current-source reproducibility is deterministic twice:

`ELF_PRISTINE_SHA256=96137519a4cde6f984199fbaf893a660945cefa8cd85dbd729b8eddaaf54e7cc`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=e90c2f8de4729ca2c273a6cc81edbb4806e851cd3000e938f0c65efa8e8851a6`

`PT_LOAD_BYTES=525844`

`ISSUE7_LINKED_BUILD=PASS`

`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

These loadable bytes differ from the R34P branch identity:

`PT_LOAD_SHA256=76a9596819bead5147b0e8cc172c559d3fe59b22fe9d9f2550c4e6e1f6f1c0c3`

`PT_LOAD_BYTES=525716`.

The new R34C/R34-complete candidate image is therefore **HARDWARE_PENDING** and
is not hardware-qualified. No operator observation, independent Validation or
hardware qualification was performed or claimed.

## Original R34 criterion disposition

These are Reconstruction Worker dispositions only; they are not Foreman
acceptance.

- `A006-R34-C1 EXPLICIT_R32_BINDING_INSTALLS_INTO_EACH_FRESH_R29_RUNTIME`
  — MET.
- `A006-R34-C2 DESKTOP_CONTEXT_ELIGIBILITY_TRACKS_REAL_APPLICATION_OWNERSHIP`
  — MET.
- `A006-R34-C3 FRESH_ATTEMPT_OWNS_FRESH_P3_P9_R21_AND_TRANSPORT_ACCESS`
  — MET.
- `A006-R34-C4 SEMANTIC_MPEG_CALIBRATION_ACTION_ROUTES_WITHOUT_PHYSICAL_GESTURE_LOGIC`
  — MET.
- `A006-R34-C5 CALIBRATION_FOREGROUND_GETS_CONTROLLER_FIRST_REFUSAL`
  — MET.
- `A006-R34-C6 ACCEPTED_CALIBRATION_HANDS_TO_P10_ONCE_OR_FAILS_CLOSED`
  — MET; accepted R34P now gives teardown-required pre-START P10 failure the
  required owner-correct dormancy path.
- `A006-R34-C7 WAIT_FIRST_FRAME_AND_MPEG_OWNED_ARE_SERVICED_AT_SAFE_CADENCE`
  — MET; live cadence still uses the genuinely-started predicate only.
- `A006-R34-C8 OWNED_CALIBRATION_OR_LIVE_RUN_BLOCKS_OVERLAP`
  — MET.
- `A006-R34-C9 RFB_FLOW_REMAINS_P2_GOVERNED_THROUGH_PROTECTED_AND_LIVE`
  — MET.
- `A006-R34-C10 SESSION_FAILURE_USES_R33_BEFORE_TRANSPORT_RELEASE_OR_REPLACEMENT`
  — MET with the accepted generalization required by R34C: post-START owners
  use R33 and pre-START teardown-required owners use accepted R34P before final
  Transport release/replacement; true no-MPEG attempts retain R16B.
- `A006-R34-C11 NO_RETIRE_RECALIBRATION_AUDIO_PERSISTENCE_OR_DEFAULT_SCOPE_CREEP`
  — MET.
- `A006-R34-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN`
  — MET.

All original R34 criteria are MET at the Reconstruction Worker level.

## R34C packet disposition

The twelve R34C required behaviors are also MET at the Reconstruction Worker
level:

1. exact R32 binding installation preserved — MET;
2. truthful product-action DESKTOP context preserved — MET;
3. live-service and teardown predicates remain distinct — MET;
4. one exact Application abnormal-teardown owner predicate added — MET;
5. teardown-required pre-START P10 owner selects accepted R34P — MET;
6. post-START owner retains accepted R33 ordering — MET;
7. true no-MPEG owner retains accepted R16B one-shot abort — MET;
8. Input dormancy precedes dependent MPEG teardown — MET;
9. abort service reaches SESSION_ABORT_READY before clock release/final close —
   MET;
10. ordinary semantic activation/live behavior remains unchanged in scope — MET;
11. retirement/recalibration/AUDIO/persistence/Pi/default-binding scope fences
    preserved — MET;
12. focused and complete deterministic evidence green — MET.

## Evidence classification

`SOURCE_COMPLETE=YES_WITHIN_R34C`

`ORIGINAL_R34_CRITERIA_MET=YES_AS_RECONSTRUCTION_DISPOSITION`

`HOST_TESTED=PASS`

`PROJECT_CHECK=PASS`

`STRICT_DICTIONARIES=PASS`

`PS2_COMPILE=PASS`

`PS2_LINK=PASS`

`CURRENT_SOURCE_REPRODUCIBILITY=PASS`

`MACHINE_EVIDENCE=GITHUB_ACTIONS`

`INDEPENDENT_VALIDATION=NOT_RUN`

`OPERATOR_OBSERVED=NO`

`HARDWARE_QUALIFIED=NO`

`HARDWARE_PENDING=YES`

`R34_FOREMAN_ACCEPTED=NO__PENDING_FOREMAN_REVIEW`

`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State and scope accounting

Consumed:

- Foreman State revision `0075`;
- accepted R34P pre-START dormancy;
- accepted R33 post-START abnormal retirement;
- accepted R16B no-MPEG replacement retirement;
- accepted R26 session-clock binding;
- accepted R29/R32 product-action binding authority;
- existing partial R34 Application composition;
- Work Log Contract revision `0007`.

Produced:

- distinct product live-service and abnormal-teardown owner seams;
- pre-START R34P selection in ordinary attempt teardown;
- preserved post-START R33 and no-MPEG R16B selection;
- focused product/Application/source-boundary evidence;
- reconciled exact new symbols;
- green canonical host/project/dictionary/PS2 build evidence.

Not produced:

- no Foreman State update;
- no Foreman acceptance;
- no normal R23/R24 retirement/reveal;
- no action-driven stop/recalibration;
- no AUDIO activation;
- no configuration persistence/editor/reload;
- no Pi/Wire production expansion;
- no physical default gesture;
- no hardware qualification.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R34C_FULL_R34_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction Worker stops here. The Foreman must independently recover
live repository authority, verify the complete R34 source/evidence rather than
relying on this report, decide all original A006-R34-C1 through C12 as one
packet, and only then publish any next bounded work.
