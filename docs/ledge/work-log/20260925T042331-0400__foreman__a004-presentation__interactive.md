DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T04:23:31-04:00
COMPLETED_AT=2026-09-25T04:29:50-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=5945eb7972ca1dd6c7b913275306ecc7b19f36ad
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — P8 accepted; Application calibration foreground P9 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A004-MPEG-CALIBRATION-MANUAL-REGION-SOURCE-P8`, independently recover live
repository authority, inspect the actual source/tests/dictionaries/evidence,
decide P8 acceptance, and publish exactly one dependency-correct next
Reconstruction packet.

Truthful round start:

`STARTED_AT=2026-09-25T04:23:31-04:00`

Live pickup authority:

`5945eb7972ca1dd6c7b913275306ecc7b19f36ad`

The branch was nine commits ahead / zero behind this Foreman's prior closeout
`7e7253a12626deb6f845a03cbb860318b3d39c73`.

Governing authorities consumed included:

- Foreman State revision 0061;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Wire Runtime Decisions revision 0011, especially Q6/Q7;
- Architecture Overlay revision 0007;
- A004 Calibration Separation Invariant revision 0001;
- accepted A004 P1 MPEG CALIBRATION core;
- accepted P2 RFB flow-policy authority;
- accepted P3 Presentation lifecycle;
- accepted R24 Application MPEG run lifecycle;
- A004/A006 audit authority;
- frozen H1 MPEG-calibration interaction/foreground/start-handoff evidence at
  `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260925T035017-0400__reconstruction__a004-presentation__interactive.md`.

No newer Validation/Continuity/Diagnostics/support authority appeared in the
recovered P8 range.

## Exact P8 worker range

Reconstruction started from:

`7e7253a12626deb6f845a03cbb860318b3d39c73`

and landed eight pre-log commits:

1. `f593c06a16f7901bdee40af4e5f22dc9325078cb` —
   `ui: add P8 manual MPEG calibration region source`;
2. `e0ea50a0e7a2f138e5eb58d409688eb10ad5c2b2` —
   `test(ui): prove P8 manual MPEG calibration source`;
3. `3558071de63fcf96447602934d243f3e4b952f16` —
   `docs: record P8 manual MPEG calibration region source`;
4. `58fc8aeb311f8341cbe2b1fafebeb583eec56cdf` —
   deterministic dictionary-reconciliation trigger;
5. `da4567278901f7649bd8ad7a72068407014d7101` —
   `docs(symbols): index P8 manual calibration source`;
6. `5530f27ae9371173b4e7a9b795fe6258027dd652` —
   second deterministic dictionary-reconciliation trigger;
7. `a39fd67e5a02ff99d25119b7c1554ba54426d299` —
   `docs(symbols): reconcile current clean definitions`;
8. `40841daec820c8be01ba2fbaa93d954dea43b506` —
   `test: verify final P8 manual MPEG calibration authority`.

Final pre-log source authority:

`40841daec820c8be01ba2fbaa93d954dea43b506`

The next and only following commit was the immutable Reconstruction record:

`5945eb7972ca1dd6c7b913275306ecc7b19f36ad`.

Independent compare proved eight commits ahead / zero behind to final source and
exactly one log-only commit afterward.

Final pre-log changed paths were limited to:

- `src/ui/mpeg_calibration_manual.c`;
- `src/ui/mpeg_calibration_manual.h`;
- `tests/unit/mpeg_calibration_manual_test.c`;
- `tests/Makefile`;
- `docs/development/mpeg-generation-control.md`;
- `src/ui/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No ordinary Application, app_mpeg_run, RFB, input-runtime, Display/P3/
compositor/Platform, MPEG execution, Transport, Configuration, Pi product,
AUDIO, DESKTOP CALIBRATION or Wire protocol implementation changed.

## Independent P8 source acceptance

P8 is one narrow UI-owned manual region source around the already-accepted P1
core.

Verified behavior:

1. `pstvnc_mpeg_calibration_manual_begin()` is explicit; P8 contains no
   START+SELECT detector, 750-ms hold timer or libpad polling;
2. normalized `pstvnc_controller_state_t` maps only to accepted P1 semantic
   actions: D-pad, R1, R2, L2, START, CROSS and CIRCLE;
3. P8 reports controller consumption only while P1 owns EDIT/REVIEW/release
   quarantine;
4. inactive controller samples are not claimed and cannot implicitly begin
   calibration;
5. P8 preserves P1's held-CROSS release/re-press acceptance boundary;
6. accepted geometry is copied by value from the exact P1 edge and cannot be
   retroactively mutated through committed storage;
7. cancel emits no accepted region and the release quarantine remains owned
   until complete release;
8. visual plan generation copies the P1 candidate and calls only
   `pstvnc_mpeg_calibration_resolve_geometry()` for base, inner-content and
   outer/suppression meanings;
9. the raster validates plan/capacity/non-overlap before any write, copies the
   immutable caller desktop to a distinct output, and draws only from resolved
   plan geometry;
10. the frozen desktop input remains unchanged;
11. no H1-only CONTROLS screen or adapter/runtime proliferation was introduced;
12. accepted output is neutral region data only, leaving future non-manual
    region sources possible;
13. no DESKTOP CALIBRATION source changed;
14. no lower-owner or product-activation behavior entered P8.

No P8 product defect was found.

### P8 criterion disposition

A004-P8-C1=MET
A004-P8-C2=MET
A004-P8-C3=MET
A004-P8-C4=MET
A004-P8-C5=MET
A004-P8-C6=MET
A004-P8-C7=MET
A004-P8-C8=MET
A004-P8-C9=MET
A004-P8-C10=MET
A004-P8-C11=MET
A004-P8-C12=MET

Overall disposition:

`P8_DISPOSITION=FOREMAN_ACCEPTED`

## Exact P8 machine evidence

Exact final source:

`40841daec820c8be01ba2fbaa93d954dea43b506`

GitHub Actions run:

`36110790554`, attempt 1 — SUCCESS.

The GitHub run object independently identifies:

- head branch `ledge/h1-all-guns`;
- head SHA `40841daec820c8be01ba2fbaa93d954dea43b506`;
- title `test: verify final P8 manual MPEG calibration authority`;
- event `push`;
- conclusion `success`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile skipped as expected.

Focused host evidence includes:

- `MPEG_CALIBRATION_UNIT=PASS`;
- `MPEG_CALIBRATION_MANUAL_TEST=PASS`.

Repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=206 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=d9fa5af6749f21f4a7e5ab6dc727bebb66c584c37bc487a4c909b96abcdfb549`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=0b4810977f794cb5c34e5ce4a90a54be628622a53ac74fe519df12f6e58efc4e`
`PT_LOAD_BYTES=500244`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This linked identity is exactly unchanged from accepted R24.

Evidence classification:

SOURCE_COMPLETE=YES
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
LOADABLE_BYTES_CHANGED=NO
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO_NEW_CLAIM
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Next dependency analysis

P8 closes the UI-owned manual region-source dependency, but it deliberately does
not establish the cross-domain foreground safety transaction.

The accepted ordinary Application already owns the necessary public facts/seams:

- P2 global freeze/thaw and one-shot FULL debt;
- input-runtime mouse suspension/rebase/resume while physical controller polling
  continues;
- exact successfully-published remote pointer state and remote pointer-release
  serialization;
- local UI DESKTOP/OSK ownership;
- Platform presentation of a complete desktop surface;
- P3 read-only RFB_ONLY / MPEG ownership state.

Frozen H1 evidence confirms the same ownership split: the manual calibration
adapter/rendering was separable from the higher-level foreground bridge, which
ordered RFB freeze, mouse suspension, pointer neutralization/rebase and later
resume. Current architecture must not revive the H1 adapter stack, but the
behavioral ordering remains relevant.

A crucial current-architecture distinction is that one P2 freeze bit now carries
the global protection used by the accepted MPEG run-start coordinator. Therefore
the post-calibration endpoint must differ by outcome:

- CANCEL: restore the frozen ordinary desktop, resume mouse interpretation, thaw
  P2 and preserve the resulting FULL-refresh debt;
- ACCEPT: restore the frozen ordinary desktop and resume mouse interpretation,
  but retain exact resolved MPEG presentation geometry and keep P2 frozen while
  P3 remains RFB_ONLY. That is the exact protected precondition for the already
  accepted `pstvnc_app_mpeg_run_start()`.

This makes the next packet independently useful without prematurely choosing a
product trigger or sending MPEG START.

## Next packet selected

`A004-APPLICATION-MPEG-CALIBRATION-FOREGROUND-P9`

P9 is trigger-agnostic and first-entry/RFB-only only. It reconstructs the
Application-owned safety transaction around P8:

- exact admission from ordinary desktop/RFB-only authority;
- P2 freeze first;
- input-runtime mouse suspension;
- exact pointer neutralization and rebase;
- immutable last-presented desktop snapshot;
- P8 visible service through a caller-owned work surface and existing Platform
  desktop presentation;
- release quarantine before foreground exit;
- exact physical frozen-desktop restoration before mouse resume;
- CANCEL -> P2 thaw + normal FULL debt + idle;
- ACCEPT -> exact P1-resolved P3 geometry retained + mouse resumed + P2 still
  frozen + P3 still RFB_ONLY + explicit ACCEPTED_PROTECTED;
- explicit safe abort from ACCEPTED_PROTECTED without MPEG start.

P9 does not wire ordinary `src/app.c`, choose START+SELECT, invoke
`app_mpeg_run`, implement active-MPEG recalibration, or alter any lower-owner
mechanism.

## Foreman-owned state publication

Published Foreman State revision 0062:

`197183e1ef078be69cd3e20ec9fd63e9b66e45f7`

message:

`docs(foreman): accept P8 and activate calibration foreground P9`

State 0062:

- accepts P8;
- records exact P8 source/evidence and unchanged loadable identity;
- marks the manual MPEG CALIBRATION region source Foreman-accepted;
- activates exactly one next packet, P9;
- keeps ordinary MPEG product activation deferred.

Immediately after publication, the branch remained exactly at State 0062.

## Hardware debt

Current fully Foreman-accepted behavior-bearing loadable authority remains R24:

`PT_LOAD_SHA256=0b4810977f794cb5c34e5ce4a90a54be628622a53ac74fe519df12f6e58efc4e`
`PT_LOAD_BYTES=500244`

P8 changes maintained source but not current linked bytes.

P9 may become enrolled Application product source and therefore may produce a
new linked identity. Any new exact identity remains hardware-pending unless
separately physically qualified.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A004-APPLICATION-MPEG-CALIBRATION-FOREGROUND-P9
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_P9_FROM_FOREMAN_STATE_0062

The Reconstruction worker must independently recover live branch authority,
consume State 0062, execute only P9, create exactly one immutable Reconstruction
record using WORK_ITEM_KEY `a004-presentation`, stop, and return the baton.

Do not begin ordinary `src/app.c` MPEG activation, choose a permanent product
trigger, call MPEG START/RETIRE, implement active-MPEG recalibration, persist
calibration to Pi configuration, or broaden lower-owner mechanisms in the same
shift.
