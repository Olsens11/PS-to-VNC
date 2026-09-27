# Ledge Reconstruction work log — R43 retained-session single begin-abort correction

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T21:40:56-04:00
COMPLETED_AT=2026-09-26T21:55:40-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=1aab5632b67791b478ba18234057d717abec3684
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Authority and packet

This round recovered live branch authority at
`1aab5632b67791b478ba18234057d717abec3684` and consumed Foreman State
revision 0085.

The exact active packet was:

`A006-R42C-RETAINED-SESSION-SINGLE-BEGIN-ABORT-R43`

with:

- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a006-orchestration-shutdown`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`.

R43 was a narrow correction to R42 criterion 9. R42 criteria 1-8 and 10-12
were not reopened. No Foreman State, lower Transport/MPEG/AUDIO/R26 mechanism,
Pi product source, Wire version, R36 selected values, Input/UI/Display/RFB
mechanism, or H1 forensic source was modified.

Final pre-log source/test/dictionary authority:

`e2727cb31e214d11e094f1f45b0b9d8ab01d84e9`
— `test(app): bound R43 pending replacement fixture`.

This head is eight commits ahead and zero behind the assigning Foreman handoff.

## Application correction

`src/app.c` now owns one explicit Wire-attempt-local
`retained_session_abort_established` fact.

The fact:

- is initialized false on every fresh physical Wire attempt;
- is passed by pointer into `retire_attempt_owners()`;
- remains false when Transport begin-abort fails;
- is published true immediately after
  `pstvnc_transport_session_begin_abort()` returns OK and before either local
  MPEG or AUDIO abort service runs;
- survives later cleanup invocations for the same attempt;
- causes every later same-attempt cleanup invocation to skip Transport
  begin-abort and resume only still-owned local media dormancy work;
- cannot transfer to a replacement attempt because its storage lifetime is the
  attempt loop body.

Input shutdown/dormancy remains before the first begin-abort attempt when media
debt exists.

The existing internal OK-but-PENDING retry loop remains unchanged in principle:
MPEG and AUDIO local owners are serviced at the existing 1 ms cadence and delay
or retry count never constitutes readiness.

The no-media R16B path remains the historical one-shot
`pstvnc_transport_session_abort()` path and never publishes retained-media
authority.

R26 media-clock release and final Transport close remain after all owned local
MPEG/AUDIO paths prove abort-ready. Replacement admission remains after final
Transport release.

## Deterministic R43 evidence

The focused Application fixture now records, per local-abort call, the observed
media-clock release count and Transport-close count.

New deterministic cases prove:

1. MPEG post-begin error prefix:
   `test_r43_mpeg_abort_error_reentry_keeps_one_begin_abort`
   scripts an initial MPEG local-abort error followed by readiness. Fatal
   convergence re-enters cleanup with total begin-abort count exactly one.
   Both local-abort calls observe zero clock releases and zero Transport closes;
   final readiness produces exactly one release and one close.

2. AUDIO post-begin error prefixes:
   `test_r43_audio_abort_error_reentry_keeps_one_begin_abort`
   independently runs the same composition proof across:
   - session stop failure;
   - session poll failure;
   - session join failure;
   - session outcome-read failure;
   - terminal outcome failure;
   - session release failure;
   - runtime release failure.
   Every case keeps total begin-abort count one across later fatal cleanup and
   withholds clock release/Transport close until the later ready result.

3. Dual MPEG+AUDIO OK-but-PENDING:
   `test_r43_pending_media_abort_retries_after_one_begin_abort`
   scripts both owners not-ready then ready. The case proves one begin-abort,
   two service calls per local owner, one existing-cadence idle delay, no early
   clock release/close, then exactly one release and one close.

4. Begin-abort failure:
   `test_r43_begin_abort_failure_never_publishes_retained_authority`
   proves a failed initial begin-abort does not publish retained success.
   Existing recovery/fatal policy retries the unproven lower edge; no local
   abort service, clock release, or Transport close is allowed to rely on
   fictional retained authority.

5. Fresh replacement:
   `test_r43_replacement_attempt_starts_with_retained_fact_false`
   proves a successfully retired provider-failure attempt and its successor
   each require their own begin-abort establishment. The per-attempt fact does
   not cross the physical Wire boundary.

Existing R42/R34 evidence still proves the no-media typed provider-failure path
uses one-shot Transport abort and does not enter retained-media begin/close
composition.

## Regression preservation

The complete host suite on the final source head is green, including the current
R15/R16B/R19/R27/R32/R34/R35 Application suites and focused MPEG/AUDIO owner
regressions.

Selected final-head evidence includes:

- `app_test: PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `APP_AUDIO_PRODUCT_TEST=PASS`;
- `APP_AUDIO_PRODUCT_SOURCE_TEST=PASS`;
- `APP_AUDIO_COMPOSITION_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `TRANSPORT_AUDIO_ACTIVATION_SOURCE_TEST=PASS`;
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`;
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`;
- `media_clock_test: PASS`;
- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`;
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`;
- `PI_AUDIO_PRODUCT_COMPOSITION_TEST=PASS`.

The R42 first-protected-MPEG AUDIO start rule, first-presentation reservoir gate,
finite AUDIO behavior, lazy exact-session Pi AUDIO factory, and 2.0-second
terminate-to-kill escalation policy therefore remain under their existing
unchanged tests and are green.

## Dictionary and topology reconciliation

Only one new clean product-source identifier was introduced, with local and
parameter occurrences recorded in `src/SYMBOLS.md`:

`retained_session_abort_established`.

The generated portal count is synchronized.

Final counts:

- `src:1132`;
- `pi:922`;
- total product symbols: `7349`.

Exact final project evidence includes:

- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

The dedicated long strict dictionary job also completed SUCCESS.

## Commits and changed paths

Substantive commits:

- `e3ba8f938790ac5af1991dec5baed3fedc8f9dee`
  — `fix(app): retain begin-abort authority across cleanup reentry`;
- `f56fe807565b1125b04f0f3397243b0b7135421a`
  — `test(app): script R43 audio abort error prefixes`;
- `dd58daf619ba0759cf76ed8cc479489838031da0`
  — `test(app): prove retained abort survives cleanup errors`;
- `88e315ac5ff02bd15dc65199787f6abb4d467681`
  — `docs(symbols): record R43 retained-abort authority`;
- `7457789b1e6f9fd8d5b6859f3d6ef2928d86a411`
  — `docs(symbols): sync R43 portal count`;
- `a31e9408613c8ac75d7e472e64f46903bcb68da1`
  — `test(app): cover all R43 abort prefixes and pending retries`;
- `8ed2638a62390c4d1fbac0213d2ad38e317aec84`
  — `test(app): make R43 dual pending debt explicit`;
- `e2727cb31e214d11e094f1f45b0b9d8ab01d84e9`
  — `test(app): bound R43 pending replacement fixture`.

Changed paths are confined to:

- `src/app.c`;
- `tests/unit/app_test.c`;
- `tests/unit/app_audio_composition_stubs.inc`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No public Application signature changed, so `src/app.h` did not require
modification.

## Canonical exact-head evidence

Final exact-source GitHub Actions run:

`36286921147`, attempt 1, exact head
`e2727cb31e214d11e094f1f45b0b9d8ab01d84e9`, completed SUCCESS.

Job disposition:

- host-unit = SUCCESS;
- project-check = SUCCESS;
- dictionary-long = SUCCESS;
- ps2-compile = SUCCESS;
- ps2-link = SUCCESS;
- dictionary-reconcile = correctly SKIPPED.

Additional exact-head results:

- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Two intermediate exact-head runs intentionally exposed defects in the newly
added dual-PENDING test fixture, not in product source:

- run `36286763450` failed because the first fixture did not establish explicit
  MPEG abort debt;
- run `36286858548` failed because the successful provider-recovery fixture did
  not bound the replacement attempt.

Those focused fixture defects were corrected in later test-only commits.
No product-source change followed
`e3ba8f938790ac5af1991dec5baed3fedc8f9dee`.

## Linked identity and hardware boundary

Final R43 exact-head linked identity:

`ELF_PRISTINE_SHA256=993655ccffc430347281291c1fa917e3b22ce77f17407a99edbc920b71a65a2a`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=3bf4319f56d5678550fc0b05cc438d5a4281df133d544ce3ff269163185dba1b`

`PT_LOAD_BYTES=556180`.

The preceding R42 candidate had:

`ELF_PRISTINE_SHA256=bfc6a2fb9b22db2a170b6687b8625c7b6b85bb99fad6b7accc7002e9dea72021`

`PT_LOAD_SHA256=6375db546f08a60b2b8bf34aa6786e8f740f44213b64fedecfb4081f5fe9f065`

`PT_LOAD_BYTES=556180`.

Therefore R43 changes exact loadable identity and carries new hardware evidence
debt. This round makes no operator-observed, independent-Validation, or hardware
qualification claim.

R43_SOURCE_COMPLETE=YES_WITHIN_PACKET
R43_HOST_TESTED=PASS
R43_PROJECT_CHECK=PASS
R43_STRICT_DICTIONARIES=PASS
R43_PS2_COMPILE=PASS
R43_PS2_LINK=PASS
R43_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R43_MACHINE_EVIDENCE=GITHUB_ACTIONS
R43_PS2_PT_LOAD_CHANGED=YES
R43_INDEPENDENT_VALIDATION=NOT_RUN
R43_OPERATOR_OBSERVED=NO
R43_HARDWARE_QUALIFIED=NO
R43_HARDWARE_PENDING=YES

## Worker criterion disposition

Worker findings only; Foreman acceptance remains separate.

1. Explicit per-Wire-attempt retained-session begin-abort fact begins false and
   cannot transfer to replacement — MET.
2. `retire_attempt_owners()` consumes/updates the persistent attempt fact —
   MET.
3. Input dormancy remains before first media begin-abort attempt — MET.
4. Successful begin-abort publishes the retained fact before MPEG/AUDIO local
   abort service — MET.
5. Later same-session cleanup invocations skip begin-abort and resume local
   media cleanup — MET.
6. OK-but-PENDING MPEG+AUDIO retains the existing cadence and no
   delay/count-success rule — MET.
7. MPEG post-begin error/fatal-reentry prefix retains one begin edge and blocks
   clock/Transport release until later readiness — MET.
8. AUDIO stop/poll/join/outcome/session-release/runtime-release error prefixes
   independently retain one begin edge and block premature release — MET.
9. Begin-abort failure leaves retained authority false and does not authorize
   downstream reclamation — MET.
10. No-media R16B one-shot abort path remains unchanged and never sets the
    retained-media fact — MET.
11. Successful media teardown still waits for all local owners, releases R26
    once, closes Transport once, and only then admits replacement; normal R42
    AUDIO/Pi composition remains unchanged — MET.
12. Focused failure-prefix evidence and every required canonical deterministic
    gate are green; no hardware claim was made — MET.

## Collision, blockers and evidence limitations

Every repository-changing write was preceded by a live branch-authority refresh.
No concurrent branch movement or conflicting external write was observed.

BLOCKERS=NONE

Evidence is deterministic repository/Actions evidence only. Independent
Validation was not run and physical PS2 qualification was not authorized.

## Next pickup

The Interactive Reconstruction Worker stops here and does not self-accept R43.

The Foreman must independently inspect the R43 correction, deterministic
failure-prefix evidence, final exact-head machine evidence and changed PT_LOAD
identity, then re-disposition R42 criterion 9 and the complete R42 acceptance
state.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R43_REVIEW_AND_R42_C9_REDISPOSITION
