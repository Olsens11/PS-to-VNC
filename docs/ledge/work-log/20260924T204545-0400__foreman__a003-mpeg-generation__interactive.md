DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T20:45:45-04:00
COMPLETED_AT=2026-09-24T20:52:54-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=842625885cfda96c5e068cba9dff90415190c7ad
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — corrected R23/R23C retirement accepted; R24 final reveal activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A003-APPLICATION-MPEG-Q7-RESTORE-OVERLAP-R23C`, independently recover live
repository authority, inspect the actual worker range/source/tests/evidence,
decide whether R23 plus R23C now forms an architecture-correct retirement
transaction, and publish exactly one dependency-correct next Reconstruction
packet.

Truthful shift start:

`STARTED_AT=2026-09-24T20:45:45-04:00`

Live pickup authority:

`842625885cfda96c5e068cba9dff90415190c7ad`

The branch was six commits ahead and zero behind this Foreman's prior closeout
`5a7b26436ea8b72148945e44f65f94c99d5a92f5`.

Governing authorities consumed included:

- Foreman State revision 0059;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Wire Runtime Decisions revision 0011, especially Q7;
- Architecture Overlay revision 0007;
- Q1-Q12 Reconciliation revision 0001;
- accepted R22 live-service authority;
- correction-required R23 safe-stop/drain source;
- accepted P2 flow-policy semantics;
- accepted P3 RETIRING/REVEAL_PENDING/RFB_ONLY lifecycle;
- accepted P7 drain behavior;
- accepted compositor synchronized reveal seam;
- accepted R18/R20 Transport retirement seams;
- accepted R4/R5 worker/runtime lifecycle;
- current Platform retained-video reveal contract.

The Reconstruction handoff consumed was:

`docs/ledge/work-log/20260924T202745-0400__reconstruction__a003-mpeg-generation__interactive.md`

No newer Validation/Continuity/Diagnostics/support authority appeared in the
recovered R23C range.

## Exact Reconstruction range recovered

The worker started from:

`5a7b26436ea8b72148945e44f65f94c99d5a92f5`

and landed five pre-log commits:

1. `ae06725832ea0a77b3526d33464d404801225da8` —
   `app: restore Q7 overlap during MPEG retirement`;
2. `0b9946ceea9239175f8771820d39b8ea1d1e7d86` —
   `docs: record R23C Q7 restoration overlap`;
3. `4e5d6a29c4146e9201b60156883eead668d8fb9c` —
   `tooling(symbols): run deterministic dictionary reconciliation`;
4. `af1d70024633d40982451ce42dbf66ca46a3fb92` —
   `docs(symbols): reconcile current clean definitions`;
5. `da091d1e1fcb2398bd695cae1516c4730213cf55` —
   `test: verify final R23C Q7 retirement overlap authority`.

Final pre-log source authority:

`da091d1e1fcb2398bd695cae1516c4730213cf55`

The next and only following commit was the immutable Reconstruction record:

`842625885cfda96c5e068cba9dff90415190c7ad`.

Independent compare proved five commits ahead / zero behind to final source,
then exactly one log-only commit to pickup.

The complete source range changed only:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- `tests/unit/app_mpeg_run_test.c`;
- `tests/Makefile`;
- `docs/development/mpeg-generation-control.md`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

The `tests/Makefile` change is justified: the focused Application fixture now
links the real accepted `src/rfb/flow_policy.c` so Q7 FULL/HOLD accounting is
exercised rather than stubbed.

No lower-owner product implementation, ordinary `src/app.c`, Pi product
source, Wire/protocol bytes, AUDIO, Input/UI or calibration behavior changed.

## Independent R23C source acceptance

R23C fixes exactly the State-0059 correction target.

Before retirement, P2 must still be frozen and remote publication denied.
Application advances P3 to exact-generation RETIRING before the one
current-generation RETIRE call.

If RETIRE fails, the run faults irreversibly and P2 remains frozen. No
restoration is manufactured.

Only after RETIRE returns OK does Application invoke the real accepted P2
transition:

`pstvnc_rfb_flow_policy_set_frozen(run->rfb_flow_policy, 0)`.

The coordinator then validates public P2 consequences. With no outstanding
request, the real policy reports FULL. With a genuine prior request outstanding,
it reports HOLD. Product source does not send the request, clear debt, or record
update completion.

Retirement service thereafter requires P2 restoration to remain released while
P3 stays exact-generation RETIRING. P7 remains the sole MPEG drain coordinator.

The R23 execution-retirement fences remain intact:

1. exact RETIRE completion;
2. producer-done publication;
3. natural worker completion with no P7 borrow;
4. exact join;
5. exact current-generation COMPLETED outcome;
6. P7 retirement;
7. R4 worker release;
8. R5 runtime release;
9. R18 finalization.

The normal path still never requests worker stop.

Post-thaw failure does not re-freeze P2. Successful execution retirement ends
RESTORE_PENDING with current generation retained, P3 RETIRING/snapshot retained
and P2 thawed/publication-enabled.

### R23C criterion disposition

A003-R23C-C1=MET
A003-R23C-C2=MET
A003-R23C-C3=MET
A003-R23C-C4=MET
A003-R23C-C5=MET
A003-R23C-C6=MET
A003-R23C-C7=MET
A003-R23C-C8=MET
A003-R23C-C9=MET
A003-R23C-C10=MET
A003-R23C-C11=MET
A003-R23C-C12=MET

The corrected combined R23/R23C retirement transaction is therefore:

`FOREMAN_ACCEPTED`.

State 0058's obsolete P2-frozen/no-thaw R23 C10/C11 wording remains historical
evidence only and is superseded by the accepted R23C Q7 correction.

## Exact machine evidence

At exact source authority
`da091d1e1fcb2398bd695cae1516c4730213cf55`, GitHub Actions run
`36078318661`, attempt 1, completed SUCCESS.

Jobs independently observed:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Observed host evidence:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app R15/R16B/R19 tests: PASS`;
- `app_mpeg_run_test: PASS`.

Observed project/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=202 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=c5fc16d2b958dd9689697ea6c15556da59d2c440539cedbcc3bfab33be4e3704`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=c7443e28a6eb4a6580a153768e816d4011b2b2784dabb8ef274954cb36c081fa`
`PT_LOAD_BYTES=498964`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

Evidence classification:

SOURCE_COMPLETE=YES
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
HARDWARE_PENDING=YES
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

No workflow or combined-status record was exposed for the later immutable
Reconstruction-log head at review time, so no log-head CI claim is made.

## Next dependency analysis

The next dependency is the final visible RFB handoff, but P2 protocol state is
not by itself enough to authorize that handoff.

The accepted Platform contract shows why: ordinary
`pstvnc_ps2_graphics_present()` refreshes the cached desktop texture while an
installed MPEG surface remains composited above it. Final
`pstvnc_ps2_graphics_reveal_retained_video()` then presents that cached desktop
without the retained MPEG/suppression/matte layers.

Therefore a completed P2 FULL response proves protocol/framebuffer freshness but
does not by itself prove the refreshed desktop actually crossed the graphics
presentation/upload boundary. Revealing in the narrow gap after P2 update
completion but before successful desktop presentation could expose the stale
cached desktop that Q7 is specifically intended to avoid.

The next packet must therefore add a narrow run-scoped Application proof that
future ordinary orchestration records only after the post-thaw FULL response has
completed and its authoritative framebuffer has successfully passed through the
existing desktop graphics presentation path.

After that explicit proof, the already-accepted lower-owner seams are sufficient:

- P3 seal RETIRING -> REVEAL_PENDING;
- compositor exact synchronized no-MPEG reveal;
- P3 commit to RFB_ONLY only after physical synchronized success;
- Platform pre-sync reveal failure leaves retained video retryable.

The accepted compositor test already proves PLATFORM_FAILED and SYNC_INVALID
attempts preserve P3 REVEAL_PENDING and allow retry, while exact success commits
RFB_ONLY without re-arming or changing the media clock.

## Next packet selected

`A003-APPLICATION-MPEG-FINAL-RFB-REVEAL-R24`

R24 remains trigger-agnostic. It does not wire ordinary `src/app.c`.

Required shape:

1. admit only exact RESTORE_PENDING with all R23C execution owners retired,
   exact P3 RETIRING and thawed P2;
2. add explicit run-scoped proof that the post-thaw FULL-refreshed authoritative
   desktop was actually successfully presented/uploaded through the existing
   graphics path;
3. require real P2 FULL completion and no outstanding request before that proof
   may be accepted;
4. block seal/reveal while FULL debt remains, a request is outstanding, P2 is
   frozen, or presentation proof is absent;
5. seal exact P3 only after proof and record explicit Application
   REVEAL_PENDING;
6. use only `pstvnc_mpeg_compositor_reveal_retired()` for physical handoff;
7. treat compositor PLATFORM_FAILED/SYNC_INVALID as retryable pre-sync failures:
   retain exact REVEAL_PENDING, generation and proof, no re-seal and no automatic
   session teardown;
8. treat invalid/commit/contradictory post-sync outcomes as fail-closed;
9. require synchronized + retirement_revealed + exact P3 RFB_ONLY for success;
10. only then clear current-run/transient state to reusable IDLE, set
    current_generation to zero, preserve last_allocated_generation, leave P2
    thawed and do not reset the external media-clock object;
11. prove same-coordinator next-generation allocation after an external caller
    re-establishes the start precondition by freezing P2;
12. keep ordinary product activation and all lower-owner implementation out of
    scope.

This keeps final visual safety separate from the later product trigger/service
wiring.

## Foreman-owned state publication

Published Foreman State revision 0060:

`3ceb16b59d73535244d6afb2b42a9485ba04580b`

Message:

`docs(foreman): accept R23C and activate final RFB reveal`

State 0060:

- accepts corrected R23/R23C retirement;
- records exact source/evidence/hardware-debt identity;
- activates exactly one next packet, R24;
- keeps ordinary MPEG product activation deferred.

Immediately after state publication the live branch remained exactly at the
State 0060 commit. No workflow/status record was exposed for that
documentation-only state commit at log close, so no state-head CI result is
claimed.

## Hardware debt

Current fully Foreman-accepted behavior-bearing identity:

`PT_LOAD_SHA256=c7443e28a6eb4a6580a153768e816d4011b2b2784dabb8ef274954cb36c081fa`
`PT_LOAD_BYTES=498964`

It is repository-reproducible and not physically hardware-qualified.

R24 is expected to change Application loadable bytes. Any new exact identity
remains hardware-pending unless separately qualified.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A003-APPLICATION-MPEG-FINAL-RFB-REVEAL-R24
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R24_FROM_FOREMAN_STATE_0060

The Reconstruction worker must independently recover live branch authority,
consume State 0060, execute only R24, emit exactly one immutable Reconstruction
record, stop, and return the baton.

Ordinary MPEG product activation, real trigger/service wiring, Pi product MPEG
activation, AUDIO and Input/UI/calibration remain deferred.
