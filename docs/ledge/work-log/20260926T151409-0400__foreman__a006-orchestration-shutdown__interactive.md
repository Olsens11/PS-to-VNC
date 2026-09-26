# Ledge Foreman work log — accept R35 and issue audio runtime profile

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T15:14:09-04:00
COMPLETED_AT=2026-09-26T15:19:47-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=9bf3bd373e96a1a5928899880822f72879396fdf
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R35 ordinary MPEG retire/restore/reveal composition.

Live pickup authority was:

`9bf3bd373e96a1a5928899880822f72879396fdf`
— `docs(work-log): record R35 retire restore reveal composition`.

The round independently recovered current Foreman State revision 0077, the
exact R35 Reconstruction closeout, accepted R23/R23C/R24/R35P authority, the
11-commit returned range, focused Application/product tests, exact source-head
and log-head CI, A006 orchestration authority, A007 semantic completeness, and
the remaining unintegrated A002 AUDIO responsibility.

No user terminal, Pi-local proxy, hardware action, Validation work, or Foreman
product-behavior patch was required.

## Returned R35 range

Assigning Foreman/log authority:

`ca9ebc585e61c0e2417bc98703096e3bddbf3f69`.

Final pre-log R35 source authority:

`e8e69c4f25ea832d2c6fb94139631eaafbc8a3e7`.

Immutable Reconstruction closeout:

`9bf3bd373e96a1a5928899880822f72879396fdf`.

Independent compare proves:

- 11 commits ahead;
- 0 behind.

Behavior-bearing changes are confined to:

- `src/app.c`;
- `src/app_mpeg_product.c`;
- `src/app_mpeg_product.h`;
- focused Application/product tests.

Remaining changes are dictionary reconciliation.

No accepted run-owner, worker/runtime, Transport, RFB, Display/compositor,
Input/UI, media-clock, Configuration/Management, Pi, AUDIO or forensic product
implementation changed.

## Independent R35 findings

The existing semantic `PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION` has exact
state-dependent Application policy without any Input/config reinterpretation.

- IDLE retains R34 P9 calibration entry.
- MPEG_OWNED enters accepted R23 exactly once.
- WAIT_FIRST_FRAME, RETIRING, RESTORE_PENDING and REVEAL_PENDING consume the
  already-published semantic action without duplicate retirement or automatic
  calibration.

R22 and R23 service are distinct. RETIRING obtains the exact current session
media-clock tick and invokes only the accepted R23 service seam.

R23C restoration re-enters only the existing R19/P2 request path. Application
does not construct its own FULL request or consume P2 debt privately.

The R24 marker is ordered after a real existing desktop presentation/upload
boundary. RESTORE_PENDING forces that boundary even for a pixel-identical
completed FULL response, so protocol freshness cannot masquerade as physical
presentation proof.

A pre-thaw outstanding response may be presented, but accepted R24 freshness
still rejects the marker until the required P2 FULL request has completed and
crossed the presentation boundary.

R24 PLATFORM_FAILED and SYNC_INVALID remain retryable same-session
REVEAL_PENDING results. Other R23/R24 failures become product session failure
and preserve accepted R35P/R33 abnormal containment authority.

Successful reveal returns the run to IDLE and P3 RFB_ONLY with P2 thawed.
R35 does not automatically reopen P9.

## Independent R35 criterion disposition

All twelve requirements are independently accepted:

- A006-R35-C1=MET
- A006-R35-C2=MET
- A006-R35-C3=MET
- A006-R35-C4=MET
- A006-R35-C5=MET
- A006-R35-C6=MET
- A006-R35-C7=MET
- A006-R35-C8=MET
- A006-R35-C9=MET
- A006-R35-C10=MET
- A006-R35-C11=MET
- A006-R35-C12=MET

R35_SOURCE_COMPLETE=YES
R35_FOREMAN_ACCEPTED=YES

## Exact machine evidence

Final-source GitHub Actions run:

`36264322641`

checked out exact head
`e8e69c4f25ea832d2c6fb94139631eaafbc8a3e7` on
`ledge/h1-all-guns`, attempt 1, and completed SUCCESS.

Successful canonical jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

Observed evidence includes:

- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `app_mpeg_run_test: PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_TEST=PASS`;
- `CONFIG_PRODUCT_ACTION_BINDINGS_TEST=PASS`;
- `PRODUCT_ACTION_TEST=PASS`;
- `PRODUCT_ACTION_EVENT_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact immutable-log-head run:

`36264443125`

at `9bf3bd373e96a1a5928899880822f72879396fdf` also completed SUCCESS with
host-unit, project-check, dictionary-long, ps2-compile and ps2-link all
successful.

Exact accepted linked identity:

`ELF_PRISTINE_SHA256=a974cd2d463809ed9e5dea4c0538a163a378abee096b46713940e18e6e02850a`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=908f31b526fc51d2d819b9ac092e218b048c475576568ca26cfe13a3fc84bb66`

`PT_LOAD_BYTES=527380`.

Evidence classification:

- HOST_TESTED=PASS
- PROJECT_CHECK=PASS
- STRICT_DICTIONARIES=PASS
- PS2_COMPILE=PASS
- PS2_LINK=PASS
- CURRENT_SOURCE_REPRODUCIBILITY=PASS
- MACHINE_EVIDENCE=GITHUB_ACTIONS
- INDEPENDENT_VALIDATION=NOT_RUN
- OPERATOR_OBSERVED=NO
- HARDWARE_QUALIFIED=NO
- HARDWARE_PENDING=YES

## Next dependency decision

A007 semantic completeness says reconstruction still owes every A001-A006
product responsibility, while diagnostics/scaffolding are already classified
outside product runtime.

With normal MPEG activation and retirement now composed, the remaining clearly
classified product gap is AUDIO.

The clean repository already contains:

- Transport logical AUDIO owner/seams;
- synchronous PCM playback;
- AUDSRV service binding;
- session-scoped AUDIO worker/lifecycle owner;
- common media-clock contract.

However, ordinary product code still has no selected AUDIO runtime profile
equivalent to the accepted RFB/MPEG profile authorities.

The exact qualified H1
`P11_COMPAT_PLUS_PCM` lineage provides the current selected values:

- AUDIO queue 524288;
- initial credit 524288;
- credit batch 4096;
- flush-on-empty true;
- credit return true;
- PCM 48000 Hz, stereo, 16-bit, volume 100;
- playback chunk/buffer 4096;
- startup reservoir 458752;
- worker priority 65;
- worker stack 16384;
- clean reservoir and clock poll fields each selected at 1000 us.

R26 remains sole common-clock offset authority and already selects audio offset
0, so R36 must not duplicate that field.

Automatic one-action MPEG recalibration remains optional policy and is not
selected ahead of an audited unimplemented AUDIO responsibility.

## Foreman state write

Foreman State advanced from revision 0077 to revision 0078 in:

`15019efa7b964f66b40c45abaf3d9a1105cc913d`
— `docs(ledge): accept R35 and issue audio runtime profile`.

Revision 0078:

- accepts R35 at exact source/log authority;
- records R35 as the newest accepted build identity;
- identifies AUDIO as the next audited product dependency;
- publishes exactly one selected-AUDIO-profile packet;
- keeps PS2 AUDIO execution binding and Application AUDIO composition deferred;
- keeps automatic recalibration as deferred optional policy.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A002-AUDIO-RUNTIME-PROFILE-AUTHORITY-R36`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a002-config-audio-clock`
`WORKER_KEY=interactive`.

R36 must create only Configuration's immutable selected AUDIO runtime profile,
using the exact qualified values above and narrow existing owner types.

It must not initialize AUDSRV, create AUDIO worker resources, open the Transport
AUDIO rider, modify ordinary Application, or enter Pi AUDIO/product composition.

After R36 acceptance, the expected dependency order is:

1. concrete PS2 AUDIO execution binding for the accepted session owner;
2. ordinary A006 AUDIO Application startup/steady-state/failure/shutdown
   composition;
3. then any remaining product-completeness or optional UX policy work.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A002-AUDIO-RUNTIME-PROFILE-AUTHORITY-R36`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A002-AUDIO-RUNTIME-PROFILE-AUTHORITY-R36
