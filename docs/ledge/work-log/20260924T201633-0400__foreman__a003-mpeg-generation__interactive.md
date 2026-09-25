DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T20:16:33-04:00
COMPLETED_AT=2026-09-24T20:23:07-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=f8d36483a5b8c3c231b79417bc9cf86a6faa3890
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman review — R23 requires Q7 overlap correction; R23C activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A003-APPLICATION-MPEG-RETIREMENT-DRAIN-R23`, independently recover live
repository authority, inspect what the worker actually landed, verify source,
tests, scope and exact-head machine evidence, reconcile the result against
governing architecture, disposition the packet, and publish exactly one next
bounded Reconstruction packet.

Truthful Foreman shift start:

`STARTED_AT=2026-09-24T20:16:33-04:00`

Live pickup authority was:

`f8d36483a5b8c3c231b79417bc9cf86a6faa3890`

The branch was seven commits ahead of the prior Foreman closeout
`ac905f8eefd0585f648ac7c03832d17f35534dc9` and zero behind.

The current authorities consumed included:

- Foreman State revision 0058;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Wire Runtime Decisions revision 0011;
- Architecture Overlay revision 0007;
- Q1-Q12 Reconciliation revision 0001;
- accepted R22 Application live-service source;
- accepted P2 RFB flow policy;
- accepted P3 Presentation state machine;
- accepted P7 frame-consumer drain behavior;
- accepted R18 Transport MPEG run boundary;
- accepted R20 exact private-session START/RETIRE projection;
- accepted R4 worker and R5 PS2 worker-runtime lifecycle contracts;
- A003/A004/A006 audit context where still consistent with later Q7 authority.

The worker handoff consumed was:

`docs/ledge/work-log/20260924T195801-0400__reconstruction__a003-mpeg-generation__interactive.md`

No newer Validation, Continuity, Diagnostics or support-lane commit existed in
the recovered worker range.

## Exact worker range recovered

The worker began from the previous Foreman closeout:

`ac905f8eefd0585f648ac7c03832d17f35534dc9`

and landed six pre-log commits:

1. `253d98ebabff8863f72c1df552e721ac5ba23136` —
   `app: add R23 MPEG retirement drain transaction`;
2. `9f6c1f0fe48e54b769c1bad61561a067761c8136` —
   `tooling(symbols): run deterministic dictionary reconciliation`;
3. `6e3c4138404df8b9a223a6b2de348beb96598bf5` —
   `docs(symbols): reconcile current clean definitions`;
4. `7413b1dedae2a32430f58969e4403e2d8b0350a7` —
   `test: verify final R23 MPEG retirement drain authority`;
5. `5e54a577d46b33490232d7340797f01eea1c11de` —
   `test(app): complete R23 retirement failure evidence`;
6. `3b4087d7b65992f0c3edda5fcf34a627ad6f1d28` —
   `test(app): correct R23 failed-outcome fixture`.

Final pre-log source authority:

`3b4087d7b65992f0c3edda5fcf34a627ad6f1d28`

The next and only following commit was the immutable Reconstruction log:

`f8d36483a5b8c3c231b79417bc9cf86a6faa3890`.

Independent compare confirmed six commits ahead / zero behind to final source,
then exactly one log-only commit to pickup. No concurrent write collision was
present.

Changed paths were limited to:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- `tests/unit/app_mpeg_run_test.c`;
- `docs/development/mpeg-generation-control.md`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- the immutable Reconstruction log.

No lower-owner implementation, ordinary `src/app.c`, Pi product source,
protocol bytes, AUDIO, Input/UI or calibration implementation changed.

## Independent R23 source review

The worker faithfully implemented State 0058.

Verified useful R23 behavior:

1. normal retirement admits only from an exact healthy Application
   MPEG_OWNED run;
2. P3 exact-generation MPEG_OWNED -> RETIRING precedes RETIRE;
3. one exact-generation RETIRE invocation is recorded before the irreversible
   Transport call;
4. existing P7 remains the sole frame-drain mechanism in RETIRING;
5. completion WOULD_BLOCK is pending and exact completion is generation-fenced;
6. producer-done is published only after exact RETIRE completion;
7. worker completion before producer-done is failure;
8. post-fence clean reclaim requires exact no-borrow worker status, join and
   COMPLETED outcome;
9. clean path does not request worker stop;
10. P7/R4/R5 reclaim precedes R18 finalization;
11. R18 finalization is the last execution reclaim step;
12. success records explicit RESTORE_PENDING without P3 seal/reveal.

The worker's final test hardening briefly introduced a nonexistent test-only
enum in commit `5e54a577...`; workflow `36076256414` rejected it. Commit
`3b4087d7...` corrected only the fixture to the real enum and became the final
candidate source. This did not alter product behavior.

## Architecture conflict discovered by Foreman

R23 cannot be accepted as a whole because State 0058's packet itself conflicted
with older-but-still-governing architecture that State 0058 explicitly named as
authority.

State 0058 required:

- P2 remain frozen throughout RETIRING drain;
- success end RESTORE_PENDING with P2 still frozen;
- no P2 thaw/FULL-refresh scheduling in R23.

But Wire Runtime Decisions revision 0011 Q7 requires normal MPEG retirement to:

1. close new MPEG production first;
2. let already accepted MPEG work drain;
3. release RFB suppression so RFB restoration begins underneath while valid
   retiring MPEG presentation remains visible;
4. continue MPEG retirement to its safe final boundary;
5. reveal the already-refreshing RFB state only when final MPEG removal is safe.

Architecture Overlay revision 0007 repeats that ordering and explicitly states
that it supersedes older complete-retirement-before-RFB-restoration wording.

State 0058 did not explicitly supersede Q7 and simultaneously declared Wire
Runtime Decisions 0011 / Overlay 0007 governing. Therefore its contrary P2
freeze requirement was a Foreman planning defect, not valid new architecture.

The Reconstruction worker did not violate its packet. Foreman records the
correction here rather than rewriting the worker's immutable history.

## R23 criterion disposition

The packet-written criteria are technically supported as follows:

A003-R23-C1=MET
A003-R23-C2=MET
A003-R23-C3=MET
A003-R23-C4=MET
A003-R23-C5=MET
A003-R23-C6=MET
A003-R23-C7=MET
A003-R23-C8=MET
A003-R23-C9=MET
A003-R23-C10=MET_AS_WRITTEN_BUT_ARCHITECTURALLY_INVALID
A003-R23-C11=MET_AS_WRITTEN_BUT_ARCHITECTURALLY_INVALID
A003-R23-C12=MET

Overall disposition:

`R23_DISPOSITION=CORRECTION_REQUIRED`

R23 is not Foreman-accepted until the Q7 overlap correction is landed and
reviewed.

The correction does not invalidate the useful safe-stop/drain evidence listed
above; it invalidates only the post-RETIRE P2-frozen lifecycle requirement.

## Exact R23 machine evidence

Final candidate source
`3b4087d7b65992f0c3edda5fcf34a627ad6f1d28` ran GitHub Actions workflow
`36076291875`, attempt 1.

Independently observed:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/current-source reproducibility — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Host evidence included:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Project/build evidence included:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact R23 candidate linked identity:

`ELF_PRISTINE_SHA256=7840ce4964e4285e30b68c291e9281ba9d0dfa063d2a11e639a78b430042b0b7`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=31b83c61377b5d647ed3692940aa1b9358d861e591d4b2113b3f1e50572aa0dd`
`PT_LOAD_BYTES=498708`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

Evidence classification:

R23_SOURCE_MECHANICS_PROVEN=YES
R23_PACKET_ACCEPTANCE=CORRECTION_REQUIRED
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

## Next dependency decision

The smallest dependency-correct next tranche is not final reveal and not
ordinary product activation.

It is the narrow correction:

`A003-APPLICATION-MPEG-Q7-RESTORE-OVERLAP-R23C`.

R23C must preserve R23's proven safe-stop/drain ordering while changing only the
Q7 suppression/restoration edge:

- P2 remains frozen at retirement admission;
- P3 still enters RETIRING before RETIRE;
- RETIRE still crosses the irreversible exact-generation boundary;
- after RETIRE returns OK, Application thaws P2 exactly once through the existing
  flow-policy seam;
- that genuine transition creates P2's one-shot FULL-refresh debt;
- ordinary R19 RFB service can then request/receive/update the underlying desktop
  while P7 continues valid RETIRING MPEG drain;
- later R23 completion/producer-done/worker/reclaim/finalize fences stay intact;
- later failures do not re-freeze P2;
- execution retirement ends RESTORE_PENDING with P3 still RETIRING but P2
  thawed.

R23C may not fake freshness, clear FULL debt manually, send RFB requests from the
MPEG coordinator, seal/reveal P3, modify the compositor, activate ordinary
`src/app.c`, or change lower-owner mechanisms.

The later final-restoration packet remains dependency-queued. It can use the
already-accepted P2 accounting and compositor reveal seam to require actually
fresh RFB state before P3 seal/synchronized no-MPEG reveal.

## Foreman-owned state publication

Published Foreman State revision 0059 at:

`6969457f0f7b1e388b391c3adf2dbfb256704f61`

with message:

`docs(foreman): require Q7 retirement overlap correction`.

State 0059:

- records R23 as CORRECTION_REQUIRED;
- explicitly corrects State 0058's conflict with Q7;
- preserves the worker's useful R23 source/evidence rather than discarding it;
- activates exactly one packet, R23C;
- keeps final reveal and ordinary MPEG product activation deferred.

No Foreman product-source, dictionary, topology, build-manifest or tooling edit
was required.

Immediately after publication the branch remained exactly at the State 0059
commit. No commit-status or PR-triggered workflow record was exposed for that
documentation-only state publication at log close, so no state-head CI result
is claimed.

## Hardware debt

Current branch candidate identity is R23:

`PT_LOAD_SHA256=31b83c61377b5d647ed3692940aa1b9358d861e591d4b2113b3f1e50572aa0dd`
`PT_LOAD_BYTES=498708`

It is not hardware-qualified and is not yet fully Foreman-accepted because the
Q7 lifecycle correction remains outstanding.

The last fully accepted behavior-bearing source is R22. R23C is expected to
change loadable bytes again; any new identity remains hardware-pending unless
separately qualified.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A003-APPLICATION-MPEG-Q7-RESTORE-OVERLAP-R23C
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R23C_FROM_FOREMAN_STATE_0059

The Reconstruction worker must independently recover current branch authority,
consume Foreman State revision 0059, execute only R23C, create exactly one
immutable Reconstruction log, stop, and return the baton.

Final Presentation seal/synchronized reveal, ordinary MPEG product activation,
Pi product activation, AUDIO and Input/UI/calibration remain deferred.
