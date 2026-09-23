DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-23T04:26:21-04:00
COMPLETED_AT=2026-09-23T07:02:46-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=3ab9d8bbd57bac9cf078c92dae772474f358d198
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R19 Application RFB flow composition

## Objective and authority consumed

This interactive Reconstruction worker recovered live authority from
`ledge/h1-all-guns` after Foreman acceptance of R18 and consumed Foreman State
revision `0050`, whose active bounded packet was:

`A004-RFB-FLOW-APPLICATION-COMPOSITION-R19`

The starting branch authority was:

`3ab9d8bbd57bac9cf078c92dae772474f358d198`

R19 required the already-accepted A004 P2 generic RFB flow policy to become the
single authority for ordinary post-startup request cadence, successful-send
accounting, completed-update accounting, and remote visual publication
permission in the real Application live loop, while preserving current thawed
RFB-only behavior and stopping before MPEG/calibration activation.

Governing authority consumed included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision 0006;
- `docs/ledge/work-log/README.md` revision 0007;
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0050;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`;
- `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md`;
- `docs/ledge/LEDGE_Q1_Q12_RECONCILIATION.md`;
- `docs/development/module-lifecycle.md`;
- `docs/development/rfb-provider-failure.md`;
- current `src/app.c`, accepted `src/rfb/flow_policy.*`, RFB session,
  framebuffer, Application host fixtures and P2 policy tests.

The GitHub connector exposes repository/branch authority but not the external
developer Pi worktree. External staged/unstaged/untracked state and local
`scripts/resume-state.sh` execution remain
`PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`; no claim is made about
external worktree cleanliness.

## Result

R19 is source-complete as a Reconstruction candidate for independent Foreman
review at final pre-log source authority:

`cd09bc263e5835c75d5a8fea256cd71127e7821b`

The real ordinary Application loop now composes the accepted P2 RFB flow policy
without duplicating its mechanism:

- each fresh RFB attempt initializes one fresh thawed
  `pstvnc_rfb_flow_policy_t` only after the authoritative startup full frame;
- the existing startup full-frame acquisition/presentation remains outside P2;
- one small Application-local request service asks
  `pstvnc_rfb_flow_policy_next_request()` for HOLD/INCREMENTAL/FULL;
- HOLD sends nothing;
- INCREMENTAL maps to the existing RFB request API with incremental=1;
- FULL maps to the same API with incremental=0;
- `pstvnc_rfb_flow_policy_record_request_sent()` is called only after the RFB
  session reports successful serialization;
- if successful wire serialization cannot be reconciled with policy accounting,
  Application fails closed instead of repairing or inventing debt;
- IDLE receive neither records update completion nor schedules another request;
- each complete live UPDATE records exactly one
  `pstvnc_rfb_flow_policy_record_update_complete()` before validity,
  publication, or successor request scheduling;
- RFB parsing/framebuffer mutation remains independent from presentation;
- a dirty completed remote frame is published only if
  `pstvnc_rfb_flow_policy_allows_remote_publication()` permits it;
- R19 introduces no production freeze caller, so ordinary visible behavior
  remains thawed: one incremental request at a time and normal dirty-frame
  presentation;
- R16B provider replacement reconstructs P2 state with the rest of the
  attempt-local RFB authority and cannot inherit outstanding/freeze/FULL debt.

The accepted `src/rfb/flow_policy.c/.h` implementation itself was not changed.

Production build authority now links the accepted P2 object and the pinned PS2
compile manifest explicitly compiles `src/rfb/flow_policy.c`; the Application
host fixture mocks only the public P2 seam so composition ordering can be tested
independently while `rfb_flow_policy_test` continues to exercise the real P2
state machine.

No MPEG run-open/START/RETIRE/finalize, MPEG generation identity,
decoder/worker/backend, MPEG Presentation/compositor/frame-consumer,
calibration/UI trigger, Pi product source, AUDIO product source, RFB parser/
session mechanism, Transport product source, Wire protocol byte, heartbeat or
final all-guns composition was activated or modified.

## Commits

The complete pre-log R19 range is six commits ahead / zero behind the assigning
Foreman authority:

1. `559bfb9892c85e661fbbb3004033320a677a8d7e` —
   `app: compose RFB flow policy into live loop`
2. `d610219e2f06540859e489f4b824ff89cedb4c99` —
   `test(app): prove R19 RFB flow composition`
3. `ad0a9b3bc260333e437d8d77e25cd0162cad808b` —
   `docs(rfb): record R19 live-flow composition`
4. `8ad0bd1e462b30d33948da5695b08394337c9cc2` —
   `tooling(symbols): run deterministic dictionary reconciliation`
5. `f4099407e7585ca09a217bb2198c0018472c6461` —
   `docs(symbols): reconcile current clean definitions`
6. `cd09bc263e5835c75d5a8fea256cd71127e7821b` —
   `test: verify final R19 RFB flow authority`

## Changed paths

The complete Foreman-handoff-to-pre-log diff is confined to R19-authorized
Application/test/build/check/documentation/dictionary surfaces:

- `src/app.c`
- `tests/unit/app_test.c`
- `tests/unit/app_test_legacy.inc`
- `tests/Makefile`
- `mk/issue7-clean.mk`
- `scripts/check-clean-ps2-compile.sh`
- `src/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`
- `docs/development/module-lifecycle.md`
- `docs/development/rfb-provider-failure.md`

No Pi product, Transport product, RFB parser/session, framebuffer product,
AUDIO, MPEG, media clock, display/compositor, calibration/local-controller
product, protocol or Configuration product source changed.

## Deterministic R19 behavior evidence

The Application host fixture now proves:

1. the first ordinary post-startup request is selected through the P2 seam as
   incremental;
2. successful RFB serialization is followed by exactly one P2
   record-request-sent operation;
3. failed RFB request serialization never records a successful P2 send;
4. IDLE receive leaves the outstanding policy obligation intact and does not
   send another request;
5. a P2 HOLD decision sends nothing and records neither request nor completion;
6. a P2 FULL decision maps to the existing RFB request API with
   incremental=false;
7. one complete UPDATE records exactly one policy completion before publication
   permission is queried or a successor request is selected;
8. an ordinary thawed dirty UPDATE presents once and then schedules the next
   incremental request;
9. a dirty UPDATE remains parsed/framebuffer-authoritative when the mocked P2
   publication gate denies presentation;
10. a policy-accounting rejection after successful wire serialization fails
    before receive service rather than silently repairing counters;
11. provider-local failure closes the old attempt and the replacement
    independently initializes fresh thawed flow state before its first request;
12. existing R15/R16B lifecycle/input/provider-recovery tests remain green.

The real accepted `rfb_flow_policy_test` remains separately green and preserves
P2 authority for:

- one outstanding request;
- nonmutating HOLD inspection;
- frozen publication denial;
- in-flight completion while frozen;
- one-shot post-thaw FULL-refresh debt;
- FULL-debt coalescing;
- request-kind validation.

## Acceptance-criterion worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A004-R19-C1 FRESH_RFB_ATTEMPT_OWNS_FRESH_THAWED_FLOW_POLICY` — MET.
- `A004-R19-C2 LIVE_REQUESTS_ARE_SELECTED_ONLY_BY_P2_POLICY` — MET.
- `A004-R19-C3 REQUEST_ACCOUNTING_ADVANCES_ONLY_AFTER_SUCCESSFUL_SEND` — MET.
- `A004-R19-C4 COMPLETED_UPDATE_CLEARS_EXACT_OUTSTANDING_REQUEST_BEFORE_NEXT` — MET.
- `A004-R19-C5 REMOTE_VISUAL_PUBLICATION_IS_GATED_BY_P2_POLICY` — MET.
- `A004-R19-C6 ORDINARY_THAWED_INCREMENTAL_BEHAVIOR_IS_PRESERVED` — MET.
- `A004-R19-C7 IDLE_OR_HOLD_DOES_NOT_FABRICATE_REQUEST_OR_COMPLETION` — MET.
- `A004-R19-C8 PROVIDER_REPLACEMENT_CANNOT_INHERIT_FLOW_STATE` — MET.
- `A004-R19-C9 P2_FULL_REFRESH_DEBT_SEMANTICS_REMAIN_SINGLE_AUTHORITY` — MET.
- `A004-R19-C10 RFB_TRANSPORT_FRAMEBUFFER_PRESENTATION_OWNERSHIP_UNCHANGED` — MET.
- `A004-R19-C11 NO_MPEG_CALIBRATION_AUDIO_PI_OR_FINAL_COMPOSITION_SCOPE_CREEP` — MET.
- `A004-R19-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

## Exact final machine evidence

Final pre-log source authority:

`cd09bc263e5835c75d5a8fea256cd71127e7821b`

Exact-head workflow:

- workflow: `Ledge reconstruction checks`
- run: `35852093499`
- attempt: `1`
- conclusion: `success`
- head SHA: `cd09bc263e5835c75d5a8fea256cd71127e7821b`

Job results:

- `host-unit` — SUCCESS
  - `transport_runtime_test: PASS`
  - `transport_audio_test: PASS`
  - `transport_mpeg_test: PASS`
  - `RFB_FLOW_POLICY_TEST=PASS`
  - Pi MPEG generation fixture: 12 tests, OK
  - `app R15/R16B/R19 tests: PASS`
  - `transport_rfb_provider_failure_test: PASS`
- `project-check` — SUCCESS
  - `WORK_LOG_CHECK=PASS`
  - `SOURCE_DICTIONARIES=PASS`
  - `PS_TO_VNC_PROJECT_CHECK=PASS`
- `dictionary-long` — SUCCESS
  - `SOURCE_DICTIONARY_CHECK_MODE=LONG`
  - `SOURCE_DICTIONARIES=PASS`
- `ps2-compile` — SUCCESS, including the newly enrolled
  `src/rfb/flow_policy.c`
- `ps2-link` — SUCCESS
  - `ISSUE7_LINKED_BUILD=PASS`
  - `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`
- `dictionary-reconcile` — SKIPPED as expected on the non-trigger final
  evidence commit.

Final linked identity:

- `ELF_PRISTINE_SHA256=308688f933c72de19934eae3d8048e9476de91e688d2098db0e98d283ca0f4b0`
- `PT_LOAD_SEGMENTS=1`
- `PT_LOAD_SHA256=47111b7484e3dec8532d58d7964cf22434fc99391720902c851c15375b176867`
- `PT_LOAD_BYTES=491156`
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R19 changes the loadable image from accepted R18
(`PT_LOAD_SHA256=e244769e...`, `PT_LOAD_BYTES=490516`) and therefore creates
a new exact hardware-debt identity. No physical qualification is inferred.

## Intermediate CI observations

The first workflow attempt on `d610219...` was infrastructure-only failure:
all substantive jobs completed as failure with no recorded steps and no job-log
blob. Re-running the same workflow unchanged produced real runners; host-unit,
project-check and PS2 compile passed, while strict dictionaries correctly
reported the unreconciled new Application symbols.

The canonical dictionary trigger `8ad0bd1...` ran the deterministic reconciler
and produced only six new `src/SYMBOLS.md` rows plus the generated portal
count update. Its parallel host job reproduced the known timing-sensitive
`transport_runtime_test` asynchronous-publication observation at lines 862/864.
That head is content-identical in product source to the already host-green
`ad0a9b3...` behavior authority. No RFB Transport source was changed.

The final reconciled exact head `cd09bc2...` passed the same
`transport_runtime_test`, all R19 Application tests, the real P2 suite and all
canonical gates. The transient line-862 observation is therefore preserved as
evidence history, not treated as an R19 product defect or success condition.

## Evidence gaps / hardware debt

- `PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR` — external Pi worktree
  cleanliness/staging state was not observable.
- `R19_INDEPENDENT_VALIDATION=NOT_RUN`
- `R19_OPERATOR_OBSERVED=NO`
- `R19_HARDWARE_QUALIFIED=NO`
- `HARDWARE_PENDING=YES`

Machine/host evidence proves source behavior and reproducibility only. It does
not transfer physical qualification to the new PT_LOAD identity.

## State/contract accounting

Consumed:

- Foreman State revision `0050`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- accepted A004 P2 RFB flow-policy mechanism;
- accepted R16B provider-replacement policy;
- accepted R18 authority named by State 0050.

Produced:

- no Foreman state revision;
- no reconstruction-contract revision;
- no Wire/protocol version revision;
- no packet selection or Foreman acceptance.

## Findings / blockers

R19 has no remaining known source blocker within the authorized packet.

The current ordinary product remains deliberately thawed because R19 adds no
production freeze caller. This is intentional packet scope, not missing R19
behavior. The next MPEG/calibration transaction may consume the now-real P2 seam
only if separately authorized by later Foreman state.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R19_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
current repository authority, inspect this source/evidence/log, decide R19
acceptance, publish any Foreman-owned state/integration evidence, and select any
next bounded packet. Reconstruction must not self-accept R19 or begin MPEG
Application orchestration from this record.
