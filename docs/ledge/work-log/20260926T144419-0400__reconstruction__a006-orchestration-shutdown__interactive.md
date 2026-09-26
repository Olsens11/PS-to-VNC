# Ledge Reconstruction work log — R35 ordinary MPEG retire / restore / reveal

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T14:44:19-04:00
COMPLETED_AT=2026-09-26T14:58:02-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=ca9ebc585e61c0e2417bc98703096e3bddbf3f69
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Reconstruction round received the baton at live branch authority
`ca9ebc585e61c0e2417bc98703096e3bddbf3f69`, with Foreman State revision
0077 governing.

The exact active packet was:

`A006-ORDINARY-MPEG-ACTION-RETIRE-RESTORE-REVEAL-R35`.

The packet authorized only ordinary Application composition of already-accepted
R23/R23C/R24 normal same-session retirement, RFB restoration overlap and final
desktop reveal. Automatic recalibration, a new STOP/RETIRE action, binding
context reinterpretation, AUDIO, persistence/editor/reload, Pi/Wire producer
changes and hardware qualification remained out of scope.

No user terminal, Pi-local proxy, hardware action, Validation work or Foreman
state mutation was performed.

## Final pre-log authority

Final pre-log source/test/dictionary authority:

`e8e69c4f25ea832d2c6fb94139631eaafbc8a3e7`
— `test(app): verify reconciled R35 authority`.

The returned range is exactly 11 commits ahead of the assigning Foreman/log
head and zero behind:

1. `f8cf152265072b30b78b4208f7e90aac45f9c2ba`
   — `app(mpeg): expose R35 normal retirement seams`
2. `f315606439eb336899677fde8aa7bf7d3f628579`
   — `app(mpeg): compose R35 normal retirement policy`
3. `5ceab77ca5ae9f64ca9fd91d4ad9c320ee053d96`
   — `app: compose R35 retire restore reveal loop`
4. `e9531667c8e0817db88438d1f71affd1e4623cbe`
   — `test(app): prove R35 product retirement policy`
5. `fac72cdd08a3c032394225361b134d698a58fd48`
   — `test(app): prove R35 restoration presentation path`
6. `88287f217adcfe6c37c56f722fa3bca1d26d5434`
   — strict-host-fixture warning repair
7. `6342f27a43d7d1d929948b2b6bb2fbeda61727b4`
   — `test(app): enforce R35 product ownership boundaries`
8. `9c48df647bacc5501384a7129e8bd0b7f87a1c97`
   — source-boundary test syntax repair
9. `b705f8ce6d4c800b8e19e27e85fa1cdc8c77e43d`
   — exact empty deterministic dictionary trigger
10. `229862717ef61164bbff3462dea8a3804f9f1467`
    — automation-generated dictionary reconciliation
11. `e8e69c4f25ea832d2c6fb94139631eaafbc8a3e7`
    — no-content final canonical verification stamp

The exact changed file set is confined to the authorized R35 surface:

- `src/app.c`;
- `src/app_mpeg_product.c`;
- `src/app_mpeg_product.h`;
- `tests/unit/app_mpeg_product_test.c`;
- `tests/unit/app_mpeg_product_source_test.py`;
- `tests/unit/app_test.c`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No accepted `src/app_mpeg_run.*` R21-R24/R33/R34P/R35P implementation, MPEG
worker/runtime, Transport, RFB flow/session/framebuffer, Display/compositor,
Input/UI, media-clock, Configuration/Management, Pi, AUDIO or forensic source
changed.

## R35 implementation

The ordinary product coordinator now gives the existing semantic
`PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION` exact state-dependent Application
policy without changing Input/config authority:

- IDLE desktop retains the accepted R34 P9 calibration-begin meaning;
- exact MPEG_OWNED enters
  `pstvnc_app_mpeg_run_begin_retirement()` exactly once;
- WAIT_FIRST_FRAME, RETIRING, RESTORE_PENDING and REVEAL_PENDING consume the
  already-published semantic action without duplicate RETIRE, a new generation
  or automatic calibration.

No physical chord, mask, hold duration, trigger or binding-context logic entered
Application. A DESKTOP-context binding therefore remains ineligible while MPEG
owns presentation; only an action already legitimately published by Input can
reach this R35 state policy.

R22 and R23 service remain separate. The ordinary loop reads the exact current
session-clock tick and:

- calls accepted R22 live service for WAIT_FIRST_FRAME/MPEG_OWNED;
- calls accepted R23 retirement service for RETIRING;
- never calls R22 as the RETIRING owner.

After R23C thaws P2, Application re-enters only the existing
`service_rfb_flow_request()` path. HOLD/FULL/incremental serialization and
request accounting therefore remain R19/P2-owned. R35 introduces no direct RFB
request path and does not consume FULL-refresh debt itself.

For R24 restoration, Application identifies exact RESTORE_PENDING state after a
completed RFB response. It forces the existing
`present_current_application_frame()` framebuffer-to-GS presentation boundary
before recording the R24 marker. This forced boundary occurs even when the
completed authoritative FULL response reports unchanged pixels; the
authoritative framebuffer is rebuilt into GS presentation data rather than
treating protocol freshness as presentation proof.

A pre-thaw outstanding response may still complete and cross that presentation
boundary, but the accepted R24 marker seam returns NOT_FRESH while P2 FULL debt
remains. The subsequent required FULL is sent through ordinary R19/P2. Only
after the corresponding response has completed and crossed the presentation
boundary can the marker succeed.

Final reveal is serviced through
`pstvnc_app_mpeg_run_reveal_restored()` via the product seam at ordinary loop
cadence. PLATFORM_FAILED and SYNC_INVALID remain same-session retryable pending
outcomes. Any other R23/R24 failure becomes product session failure and retains
the existing abnormal-owner path for accepted R35P/R33 two-phase containment.

Successful reveal returns the accepted R24 run to IDLE/P3 RFB_ONLY with P2 still
thawed. R35 stops there and does not call P9. A later fresh semantic action may
use the already-accepted R34 idle route.

## Focused deterministic evidence

The product fixture proves:

- MPEG_OWNED semantic action enters R23 exactly once;
- repeated action in RETIRING and actions in WAIT_FIRST_FRAME,
  RESTORE_PENDING or REVEAL_PENDING do not duplicate R23 or begin P9;
- idle semantic action retains R34 P9 behavior;
- RETIRING calls R23 service with the supplied exact tick and does not call R22;
- R24 NOT_FRESH marker outcome remains same-session pending;
- PLATFORM_FAILED and SYNC_INVALID reveal outcomes remain retryable;
- later successful reveal returns IDLE without automatic P9;
- nonretryable R23/R24 results become product session failure.

The broader Application fixture proves:

- RETIRING obtains the current session-clock tick and services R23, not R22;
- R23C restoration re-enters the ordinary R19/P2 request path;
- an already-outstanding pre-thaw incremental response can complete before the
  required FULL;
- the next R19/P2 request is the required nonincremental FULL;
- both the pre-thaw response and the subsequent pixel-identical FULL response
  cross the existing desktop prepare/present boundary while RESTORE_PENDING;
- the R24 marker call occurs only after that presentation boundary.

The source-boundary fixture proves:

- semantic routing remains free of physical binding vocabulary;
- only accepted R21-R24/R33 public owner seams are composed;
- no direct Transport RETIRE/take/producer-done/finalize, P3 mutation or
  compositor reveal appears in Application/product composition;
- R22/R23 service remains state-separated;
- the R24 marker is source-ordered after the existing presentation boundary;
- unchanged RESTORE_PENDING pixels still force that boundary;
- abnormal R35P teardown remains separate and contains no normal R35 replay;
- reveal does not automatically reopen calibration.

Existing R28-R30 Input/config context and publication tests remained unchanged
and green.

## Canonical machine evidence

Exact final-source GitHub Actions run:

`36264322641`

checked out exact head
`e8e69c4f25ea832d2c6fb94139631eaafbc8a3e7` on
`ledge/h1-all-guns` and completed SUCCESS.

Successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED on the final non-trigger commit.

Observed focused/regression evidence includes:

- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `WORK_LOG_CHECK=PASS records=238 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The deterministic dictionary automation changed only
`src/SYMBOLS.md` and
`docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` from the trigger tree.

## Exact linked identity and qualification boundary

Final R35 linked identity:

`ELF_PRISTINE_SHA256=a974cd2d463809ed9e5dea4c0538a163a378abee096b46713940e18e6e02850a`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=908f31b526fc51d2d819b9ac092e218b048c475576568ca26cfe13a3fc84bb66`

`PT_LOAD_BYTES=527380`

Newest Foreman-accepted R35P identity was:

`PT_LOAD_SHA256=39e08e49a5d6aaf32f12d134305a4f9925f7f419d75c4b15e6dde9ad17449fc9`

`PT_LOAD_BYTES=526612`.

Therefore:

- `R35_SOURCE_COMPLETE=YES_WITHIN_PACKET`
- `R35_HOST_TESTED=PASS`
- `R35_PROJECT_CHECK=PASS`
- `R35_STRICT_DICTIONARIES=PASS`
- `R35_PS2_COMPILE=PASS`
- `R35_PS2_LINK=PASS`
- `R35_CURRENT_SOURCE_REPRODUCIBILITY=PASS`
- `R35_MACHINE_EVIDENCE=GITHUB_ACTIONS`
- `R35_PS2_PT_LOAD_CHANGED=YES`
- `R35_INDEPENDENT_VALIDATION=NOT_RUN`
- `R35_OPERATOR_OBSERVED=NO`
- `R35_HARDWARE_QUALIFIED=NO`
- `R35_HARDWARE_PENDING=YES`.

No hardware qualification transfers from R35P or any earlier image.

## Reconstruction Worker requirement disposition

These are Reconstruction Worker dispositions only. They are not Foreman
acceptance.

1. Preserve Input/config authority exactly — MET.
2. One semantic action has state-dependent Application policy only — MET.
3. R23 begins once through its public owner seam — MET.
4. R22 and R23 service stay distinct — MET.
5. R23C restoration uses the existing P2/R19 request path — MET.
6. Graphics-fresh restoration proof crosses the existing presentation boundary
   — MET.
7. Completed FULL refresh is presented even when pixel truth is unchanged —
   MET.
8. R24 reveal is serviced without success-by-delay — MET.
9. Successful reveal restores ordinary desktop ownership — MET.
10. Failure convergence remains owner-correct through accepted R35P/R33 —
    MET.
11. Session media clock remains session authority — MET.
12. Focused plus canonical host/project/dictionary/PS2/reproducibility evidence
    closes the R35 fence without scope creep — MET.

## Scope and no-claim boundary

R35 does not automatically enter calibration after reveal, add a new product
action, reinterpret DESKTOP as GLOBAL, retire WAIT_FIRST_FRAME, start generation
N+1, activate AUDIO, add persistence/editor/reload, modify Pi/Wire producer
behavior, add timeout/watchdog success, or claim independent Validation,
operator observation or hardware qualification.

## Next pickup

The Interactive Reconstruction Worker stops here.

The Foreman must independently inspect the exact 11-commit returned range,
ordinary semantic state policy, R19/P2 restoration composition, unchanged-pixel
presentation proof, canonical run `36264322641`, changed linked identity and
this immutable closeout before accepting or rejecting R35 and choosing any next
packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R35_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION
