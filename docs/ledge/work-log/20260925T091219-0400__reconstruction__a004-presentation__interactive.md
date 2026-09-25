DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T09:12:19-04:00
COMPLETED_AT=2026-09-25T09:25:42-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=8bc28bd4cb023cbb0cf29625c5da165a37392fc2
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — A004 P10 protected calibration-to-run start handoff

## Recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`8bc28bd4cb023cbb0cf29625c5da165a37392fc2`.

Foreman State revision `0063` had accepted P9 and activated exactly:

`A004-APPLICATION-MPEG-PROTECTED-START-HANDOFF-P10`

with ordinary `src/app.c` MPEG activation, permanent controller trigger,
live MPEG service/retirement/reveal, Pi MPEG factory composition and
active-MPEG recalibration all explicitly deferred.

The worker consumed the current Reconstruction Contract, Work Log Contract,
Architecture Overlay, Wire Runtime Decisions, Foreman State 0063, the latest
Foreman handoff, Foreman-accepted P9 source, and accepted R21-R24 run
lifecycle/public seams.

The GitHub-connected Worker seat cannot observe external Pi/local checkout
staging, untracked files or local worktree dirtiness. No local reset, clean,
stash or checkout-over is claimed.

## Result

P10 is source-complete as a Reconstruction candidate for independent Foreman
review.

Final pre-log authority:

`2daf7194c0c92412464d10cee67990c3bb270f63`

P10 adds:

- `src/app_mpeg_activation.c`;
- `src/app_mpeg_activation.h`;

and extends P9 only with the two narrow owner seams required by State 0063:

- successful protected-handoff commit without P2 thaw;
- uncertain-handoff fault containment without P2 thaw.

No accepted R21-R24 run implementation or lower-owner mechanism changed.

## Exact ownership transfer

`pstvnc_app_mpeg_activation_start_protected()` is stateless with respect to
MPEG generation and performs one bounded P9 -> R21 ownership transfer.

Admission first reads the existing run status and requires exact run IDLE,
current_generation zero and no teardown requirement.

It then copies the exact P9 accepted presentation geometry through
`pstvnc_app_mpeg_calibration_copy_accepted_geometry()` before any run mutation.

After that side-effect-free copy, it requires:

- P9 still points to a non-null P2 owner;
- P2 is frozen and remote publication is denied;
- P9 still points to a non-null P3 owner;
- P3 is exact RFB_ONLY and exposes no live snapshot.

Only then is the existing `pstvnc_app_mpeg_run_start()` invoked exactly once,
with:

- the copied geometry;
- the same P2 pointer borrowed from P9;
- the same P3 pointer borrowed from P9;
- caller-supplied current Transport access;
- caller-supplied session media clock.

P10 does not duplicate profile lookup, generation allocation, Transport run
open, worker/runtime startup, P3 arm, P7 initialization or START serialization.

## Independent success proof

R21 returning OK is not sufficient for P10 success.

P10 independently re-reads run/P2/P3 authority and requires:

- run state `PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME`;
- nonzero current generation;
- no run teardown requirement;
- P2 still frozen;
- P2 still denies remote publication;
- P3 exact WAIT_FIRST_FRAME;
- P3 snapshot generation equals the run current generation;
- P3 snapshot geometry exactly equals the local copy obtained from P9.

Any contradiction after a returned R21 OK is treated as teardown-required, not
success and not clean rollback.

## P9 successful commit seam

P9 now exposes
`pstvnc_app_mpeg_calibration_commit_protected_handoff()`.

It is legal only from ACCEPTED_PROTECTED for one explicit nonzero run
generation, with:

- retained accepted geometry valid;
- P2 still frozen;
- mouse foreground already released;
- P8/manual source inactive;
- P3 exact WAIT_FIRST_FRAME;
- P3 snapshot generation equal to the supplied run generation;
- P3 snapshot geometry exactly equal to P9's retained accepted geometry.

Only then does P9 clear:

- its accepted-geometry validity/value;
- borrowed foreground/owner references;
- ACCEPTED_PROTECTED state.

P9 returns reusable IDLE without calling
`pstvnc_rfb_flow_policy_set_frozen(..., 0)`.

P2 therefore stays continuously frozen across calibration acceptance, the one
R21 start transaction, WAIT_FIRST_FRAME proof and P9 ownership retirement.

P3 and run state are not mutated by the P9 commit operation.

## P9 uncertain-handoff fault seam

P9 also exposes
`pstvnc_app_mpeg_calibration_fault_protected_handoff()`.

It changes ACCEPTED_PROTECTED to FAULTED while deliberately preserving:

- current P2 state;
- retained accepted geometry;
- borrowed owner evidence.

It never thaws P2.

This seam prevents P9 from continuing to advertise a normally reusable
protected acceptance after downstream run authority may have been acquired.

## Clean pre-START rollback

When R21 returns non-OK, P10 does not classify rollback from that result code.

It first independently proves:

- run state exact IDLE;
- current_generation zero;
- no teardown requirement;
- P3 exact RFB_ONLY;
- no P3 snapshot;
- P2 still frozen.

Only that proof permits the existing P9
`pstvnc_app_mpeg_calibration_abort_accepted()` path.

That accepted P9 abort performs the single thaw and leaves P2's ordinary
FULL/HOLD debt semantics intact.

Focused P10 evidence begins this clean-failure case with an older framebuffer
request outstanding and proves:

- P9 abort is invoked exactly once;
- P2 thaws;
- full_refresh_pending is set;
- next request remains HOLD while the older request is outstanding;
- after that response completes, next request becomes FULL.

## Irreversible/uncertain start failure

If R21 returns non-OK and the independent clean pre-START proof fails, P10:

- does not invoke P9 abort;
- does not thaw P2;
- invokes P9 fault containment;
- returns explicit
  `PSTVNC_APP_MPEG_ACTIVATION_START_FAILURE_TEARDOWN_REQUIRED`.

Focused cases include:

- START/irreversible run failure with FAULTED run, nonzero generation and P3
  WAIT_FIRST_FRAME;
- a nominally pre-START error whose P3 state is nevertheless dirty/uncertain.

Both retain frozen P2 and fault-contained P9 authority.

## Success-proof and handoff-commit contradictions

Focused evidence also proves:

- returned R21 OK with wrong run state is teardown-required;
- returned R21 OK with wrong P3 geometry is teardown-required;
- no P9 commit is attempted before exact WAIT_FIRST_FRAME proof;
- P9 commit failure is teardown-required and never uses the clean-abort path;
- clean-run rollback whose P9 abort itself fails is teardown-required and
  remains protected/fault-contained.

After a successful P9 commit, P10 re-proves the downstream WAIT_FIRST_FRAME
run/P2/P3 authority once more before reporting success.

## No duplicate start and generation ownership

A successful P10 handoff leaves P9 IDLE with no accepted geometry. A second
activation call from that same calibration transaction is rejected before
another run-start call.

The activation coordinator has no persistent state object and no MPEG
generation field. It neither assigns nor modifies
`last_allocated_generation` or `current_generation`.

Focused evidence models accepted R24 terminal facts while preserving the same
run coordinator's `last_allocated_generation`, creates a new protected P9
acceptance, and proves the next R21 stub allocation advances from generation
41 to 42. The counter behavior is owned entirely by the run seam.

The real accepted R21-R24 host regression `app_mpeg_run_test: PASS` remains
green on the final exact tree.

## Direct non-scope audit

Direct final-source scan of `app_mpeg_activation.{c,h}` found:

- no START+SELECT or 750-ms permanent entry detector;
- no `pstvnc_app_mpeg_run_service()` call;
- no retirement begin/service call;
- no restored-RFB proof/reveal call;
- no P3 lifecycle-mutator call;
- no direct Transport operation;
- no Pi MPEG factory/controller reference;
- no assignment to run `last_allocated_generation` or
  `current_generation`.

P10 does not modify ordinary `src/app.c`.

It does not modify P8/P1, RFB/input/local-UI lower owners, P3/compositor/
Platform, P7/MPEG worker/runtime/backend, Transport, Configuration, Pi product
source, AUDIO, DESKTOP CALIBRATION or Wire protocol.

## Build/topology enrollment

Because P10 is current maintained Application composition source,
current-source topology/build manifests add:

- `src/app_mpeg_activation.c`;
- `src/app_mpeg_activation.h`;
- `app_mpeg_activation.o` to the current linked source image;
- P10 source to the pinned current-source compile set;
- P10 root-file coverage to the topology allowlist.

This enrollment is not ordinary product activation. No caller in `src/app.c`
invokes the P10 operation.

## Dictionary reconciliation

The first complete P10 source/build authority
`07a1e734817722972abe94d667743b5724272728` passed host-unit, pinned PS2
compile and PS2 link/reproducibility. Its project/dictionary failures were only
the expected missing new root dictionary coverage.

The exact authorized no-tree-change trigger was then emitted:

`7c06b18f3a21a91c49882f091d864ac465ef77d8`

with message:

`tooling(symbols): run deterministic dictionary reconciliation`.

The repository-authorized reconciler produced:

`f2c0e23dc71727237b9e38d42ed576c825797a92`

with message:

`docs(symbols): reconcile current clean definitions`.

That generated commit changed only:

- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

Final source-identical verification authority is:

`2daf7194c0c92412464d10cee67990c3bb270f63`

with message:

`test: verify final P10 protected start handoff authority`.

## Commits

The complete pre-log Worker range is six commits ahead / zero behind the
starting Foreman authority:

1. `eb76d3303abea04bf87de38cd1142466b26b8092` —
   `app: add P10 protected MPEG start handoff`;
2. `a2a95f2ea04d736bb7639e7cea8d37f49babbf28` —
   `test(app): prove P10 protected start handoff`;
3. `07a1e734817722972abe94d667743b5724272728` —
   `build: enroll P10 protected start handoff`;
4. `7c06b18f3a21a91c49882f091d864ac465ef77d8` —
   deterministic dictionary reconciliation trigger;
5. `f2c0e23dc71727237b9e38d42ed576c825797a92` —
   generated clean-source dictionary/portal reconciliation;
6. `2daf7194c0c92412464d10cee67990c3bb270f63` —
   exact final source verification.

No concurrent branch movement or write collision occurred during Worker writes.

## Changed paths

Exact compare from starting authority
`8bc28bd4cb023cbb0cf29625c5da165a37392fc2` to final pre-log authority
`2daf7194c0c92412464d10cee67990c3bb270f63` is six commits ahead / zero
behind.

Changed paths are only:

- `src/app_mpeg_activation.c` — new;
- `src/app_mpeg_activation.h` — new;
- `src/app_mpeg_calibration.c`;
- `src/app_mpeg_calibration.h`;
- `tests/unit/app_mpeg_activation_test.c` — new;
- `tests/unit/app_mpeg_calibration_test.c`;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `scripts/continuity-check.sh`;
- `docs/development/source-topology.md`;
- `docs/development/mpeg-generation-control.md`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A004-P10-C1 HANDOFF_ADMISSION_REQUIRES_EXACT_P9_PROTECTION_AND_IDLE_RUN`
  — MET.
- `A004-P10-C2 ACCEPTED_GEOMETRY_COPY_IS_SIDE_EFFECT_FREE_BEFORE_RUN_START`
  — MET.
- `A004-P10-C3 EXACTLY_ONE_EXISTING_R21_START_TRANSACTION_IS_INVOKED` —
  MET.
- `A004-P10-C4 SUCCESS_REQUIRES_INDEPENDENT_WAIT_FIRST_FRAME_GENERATION_GEOMETRY_PROOF`
  — MET.
- `A004-P10-C5 P9_COMMIT_AFTER_SUCCESS_CLEARS_CALIBRATION_AUTHORITY_WITHOUT_P2_THAW`
  — MET.
- `A004-P10-C6 PROVEN_CLEAN_PRESTART_FAILURE_USES_EXISTING_P9_ABORT_AND_FULL_DEBT`
  — MET.
- `A004-P10-C7 UNCERTAIN_OR_IRREVERSIBLE_START_FAILURE_NEVER_THAWS_P2` —
  MET.
- `A004-P10-C8 P9_UNCERTAIN_HANDOFF_STATE_IS_FAULT_CONTAINED_NOT_NORMAL_ACCEPTED`
  — MET.
- `A004-P10-C9 SUCCESS_LEAVES_RUN_SOLE_LIFECYCLE_OWNER_WITH_P2_CONTINUOUSLY_FROZEN`
  — MET.
- `A004-P10-C10 GENERATION_ALLOCATION_AND_HISTORY_REMAIN_EXCLUSIVELY_APP_MPEG_RUN_OWNED`
  — MET.
- `A004-P10-C11 NO_TRIGGER_LIVE_SERVICE_RETIRE_REVEAL_PI_FACTORY_OR_RECALIBRATION_SCOPE`
  — MET.
- `A004-P10-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

All twelve P10 criteria are therefore `MET` for independent Foreman review.

## Machine evidence

Initial complete source/build authority:

`07a1e734817722972abe94d667743b5724272728`

Canonical workflow:

`36140478655`

Observed:

- host-unit — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/current-source reproducibility — SUCCESS;
- project-check/dictionary-long — expected failure only for unreconciled new
  root source symbols/file coverage.

Focused host output already included:

- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`.

After deterministic dictionary reconciliation, exact final pre-log authority:

`2daf7194c0c92412464d10cee67990c3bb270f63`

ran canonical workflow:

`36140699072`

attempt:

`1`

with conclusion:

`SUCCESS`.

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Observed final regression evidence includes:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `MPEG_CALIBRATION_MANUAL_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_test: PASS`;
- `app R15/R16B/R19 tests: PASS`;
- `app_mpeg_run_test: PASS`.

Observed final repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=210 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact final linked identity:

- `ELF_PRISTINE_SHA256=7a25c34ccbd1b0047ee346acce4b273c85cc4dae5a38be6ed8e153089817d64d`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=38eb2ed29857687d232d604f21a8e9b2bb267a24fa8605a57149fd1f204033fb`;
- `PT_LOAD_BYTES=512788`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

P10 therefore changes linked current-source identity from Foreman-accepted P9:

- P9 `PT_LOAD_SHA256=b57964cb50464bcbe8239865f7d54d890e0fffe9d5bf4479b79d178c0293270a`;
- P9 `PT_LOAD_BYTES=510996`.

The P10 identity is repository-reproducible build evidence only and does not
inherit physical qualification.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_P10`
`HOST_TESTED=PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`LOADABLE_BYTES_CHANGED=YES`
`MACHINE_EVIDENCE=GITHUB_ACTIONS`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO`
`HARDWARE_PENDING=YES`
`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State/contract accounting

Consumed:

- Foreman State revision `0063`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- Foreman-accepted P9 source;
- accepted R21-R24 run lifecycle.

Produced:

- no Foreman state revision;
- no architecture/contract revision;
- no Wire/protocol revision;
- no permanent product trigger;
- no ordinary `src/app.c` activation;
- no live service/retirement/reveal composition;
- no Pi MPEG factory composition;
- no active-MPEG recalibration;
- no Foreman acceptance;
- no independent Validation result.

## Findings / blockers

No P10 source blocker remains within the authorized packet.

Existing accepted P9 and R21-R24 public seams were sufficient. No lower-owner
change was required.

Ordinary product trigger/main-loop composition and Pi MPEG factory activation
remain downstream/deferred authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_P10_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect the P10 source/evidence/log, decide
acceptance, and select any later bounded packet. Reconstruction must not
self-accept P10 or begin ordinary MPEG product activation.
