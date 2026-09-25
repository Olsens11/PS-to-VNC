DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T12:53:52-04:00
COMPLETED_AT=2026-09-25T13:01:57-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=41688117e7dd1f5283a687d08c26acf605afa3b1
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R27 accepted; semantic product-action binding core R28 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A003-APPLICATION-MPEG-SESSION-FOUNDATION-R27`, independently recover live
repository authority, inspect the worker's exact source/test/build range,
disposition R27, reconcile the next dependency against A005/B07/B10 authority,
and publish exactly one bounded Reconstruction packet without implementing
product behavior from the Foreman seat.

Truthful round start:

`STARTED_AT=2026-09-25T12:53:52-04:00`

Live pickup authority:

`41688117e7dd1f5283a687d08c26acf605afa3b1`

The branch was eight commits ahead / zero behind prior Foreman closeout
`bc03d921ed08b0156db3de9ae4a4b60f7b751604`.

Consumed governing/relevant authority included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/status.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Foreman State revision 0066;
- A005 interaction/input audit revision 0001;
- A006 top-level orchestration/shutdown audit revision 0001;
- B07/B09 input/UI audit authority;
- B10/B11 configuration/binding/recovery audit authority;
- cross-domain semantic-input interface authority;
- accepted R7/R15/R16B/R18/R19/R20/R21-R27 and P8/P9/P10 authority;
- accepted R25 Pi MPEG product composition;
- frozen B4A typed hotkey authority;
- frozen H1 temporary MPEG-calibration entry adapter.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260925T121143-0400__reconstruction__a003-mpeg-generation__interactive.md`.

No newer Validation/Continuity/Diagnostics/support authority displaced R27
during pickup.

## Exact R27 worker range

Reconstruction started from:

`bc03d921ed08b0156db3de9ae4a4b60f7b751604`

and landed seven pre-log commits:

1. `595d969c49c8fc093f9e587f434b4560d759c2eb` —
   `app: compose dormant MPEG session foundation`;
2. `7897780f420739b13b255cb2c50b0cbb85f5573f` —
   `test(app): prove R27 dormant session foundation`;
3. `6e1bc6683d741760ec4afc8e33ec10e545531bb4` —
   `test(media): supersede R26 app deferral at R27`;
4. `68c024f6ccc704961e9d058e2e34e5e9a7abd334` —
   `docs(app): record R27 dormant MPEG session foundation`;
5. `664ffd10303dbaca39c8b179a68efdd2ccdc0bdd` —
   deterministic dictionary-reconciliation trigger;
6. `8ea2cb83568a833b319767af1db6da1c492ddb5f` —
   generated root dictionary/portal reconciliation;
7. `b126b749c2d3b3182c9b562973614106e98f0edb` —
   `test: verify final R27 dormant MPEG session authority`.

Final pre-log source authority:

`b126b749c2d3b3182c9b562973614106e98f0edb`

The next and only following commit was the required immutable Reconstruction log:

`41688117e7dd1f5283a687d08c26acf605afa3b1`.

Independent compare proved seven commits ahead / zero behind from assigning
Foreman authority to final source and exactly one log-only commit afterward.

Final-source changed paths are confined to:

- `src/app.c`;
- `src/app.h`;
- `src/SYMBOLS.md`;
- Application host fixture/tests;
- one focused R27 source-boundary test;
- R26 source-test supersession;
- test/build dependency enrollment;
- directly affected media-clock/MPEG/source-topology documentation;
- generated source-symbol portal.

No lower MPEG mechanism, media-clock mechanism, Platform binding, Transport,
RFB, input-runtime, local-controller/UI, AUDIO, Pi product or protocol source
changed.

## Independent R27 source review

### Product-profile admission

`pstvnc_app_run()` now resolves all required dormant session authority before
platform startup:

1. selected RFB Transport profile;
2. selected R7 MPEG runtime profile;
3. selected R26 media-clock profile.

Missing RFB or MPEG profile authority returns failure before IOP/network/session
ownership.

The selected media-clock profile is obtained by value from Configuration's
immutable 0/0/0 authority.

### One MPEG-capable Transport session

The configured entry is now
`pstvnc_app_run_with_session_profiles()`, accepting exact RFB Transport, MPEG
Transport and media-clock profile values.

Every physical attempt calls only:

`pstvnc_transport_session_open_with_mpeg()`

using the supplied ordinary RFB Transport config and exact selected R7 MPEG
Transport subprofile.

The ordinary product path no longer calls the plain RFB-only Transport
constructor.

No second Transport or channel-4 owner was created.

### Fresh dormant session media clock

After exact Transport admission, each loop attempt owns fresh automatic:

- `pstvnc_ps2_media_clock_binding_t`;
- `pstvnc_media_clock_sync_t`;
- `pstvnc_media_clock_t`;
- exact tick-rate value.

The Application initializes the R26 binding, obtains its synchronization seam
and exact `kBUSCLK` tick rate, initializes the accepted A002 clock from the
selected profile, and immediately proves the clock is not armed.

R27 does not obtain time-ops/current-tick service because no media deadline is
active yet.

No R27 code calls `pstvnc_media_clock_arm()`.

### Provider replacement ordering

The existing R16B provider-local failure classifier remains limited to
CONNECT/READ/WRITE typed causes.

After a recoverable provider failure, actual R27 source orders ownership
retirement as:

1. prove input worker shutdown/dormancy;
2. release the exact current R26 media-clock binding;
3. abort/release the current Transport session;
4. admit the next loop iteration and new physical connect only when all prior
   proof succeeded.

A binding release failure is remembered even though R26 has already revoked
local observer authority. The object is not retried or reused. Transport
teardown continues monotonically, but successor admission is blocked.

An input or Transport teardown failure likewise blocks replacement.

This preserves A006's repeated-session rule: no fresh session is admitted while
session-owned dormancy/retirement remains unproven.

### Fatal cleanup

Fatal convergence preserves reverse ownership:

- input shutdown if owned;
- active clock binding release if owned;
- adopted Transport abort, otherwise caller-owned socket close;
- resident graphics shutdown;
- optional diagnostics shutdown.

A failure before clock-binding ownership causes no fabricated binding release.

### Dormant MPEG boundary

R27 creates the channel-4 Transport capacity needed by accepted R21 but does not
cross the execution boundary.

Independent source review found no R27 invocation of:

- `pstvnc_transport_mpeg_run_open()`;
- MPEG generation START/RETIRE;
- P9 calibration begin;
- P10 protected activation;
- R21-R24 run start/service/retirement/reveal;
- MPEG worker/decoder creation/service;
- P3 presentation mutation;
- AUDIO activation;
- media-clock arm;
- any controller-product gesture.

This is consistent with accepted R25: Pi controller construction and CREDIT are
producer-inert; exact START remains the first producer edge.

No R27 defect was found.

## R27 acceptance criteria

A003-R27-C1=MET
A003-R27-C2=MET
A003-R27-C3=MET
A003-R27-C4=MET
A003-R27-C5=MET
A003-R27-C6=MET
A003-R27-C7=MET
A003-R27-C8=MET
A003-R27-C9=MET
A003-R27-C10=MET
A003-R27-C11=MET
A003-R27-C12=MET

Overall disposition:

`R27_DISPOSITION=FOREMAN_ACCEPTED`

## Exact R27 machine/build evidence

Final pre-log source authority:

`b126b749c2d3b3182c9b562973614106e98f0edb`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `36160798566`
attempt `1`
conclusion `success`

The exact run object reports:

- branch `ledge/h1-all-guns`;
- head SHA `b126b749c2d3b3182c9b562973614106e98f0edb`;
- push event;
- title `test: verify final R27 dormant MPEG session authority`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile skipped as expected.

Observed focused/cross-domain evidence:

- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `transport_mpeg_test: PASS`;
- `media_clock_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_SOURCE_TEST=PASS`;
- R25 Pi MPEG product composition: 9 tests, OK;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=216 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned compile explicitly compiled `src/app.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=7f5decc9f09b32d1060f18eca1462bcb189cc3ce96297fd5b3435ba448288d2c`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=30c8d9d1db9106f51c8de56c88e68cf3d6181d98cfd8ff4d402ce24a6c94c13d`
`PT_LOAD_BYTES=513940`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from accepted R26 and becomes the new current PS2 hardware-debt
identity.

Evidence classification:

SOURCE_COMPLETE=YES
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
PS2_PT_LOAD_CHANGED=YES
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
HARDWARE_PENDING=YES
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Next dependency analysis: semantic action before physical binding

R27 closes the dormant ordinary-session prerequisites beneath MPEG execution.

The accepted P8/P9/P10/R21-R24 stack remains intentionally trigger-agnostic:

- P8 manual calibration requires explicit begin;
- P9 owns the calibration foreground/suspend/rebase/release transaction but
  explicitly does not choose entry gesture;
- P10 transfers an accepted protected P9 result into R21 but explicitly does not
  choose a controller trigger;
- R21-R24 own run execution/retirement/restoration, not product input policy.

A005 explicitly classifies the H1 held START+SELECT 750 ms entry chord as:

`DISCARD_AS_PRODUCT_BINDING`.

The frozen H1 adapter itself calls the gesture experiment-local and temporary.
Its useful result was proof of the foreground ownership lifecycle, which is
already reconstructed separately.

The clean product therefore needs a semantic product-action boundary before it
can lawfully connect ordinary controller input to P9.

### B07/B10 authority

B07 places physical acquisition, gesture/hotkey recognition and logical-action
publication in Input. Application consumes those actions and routes product
effects.

B10 records the final historical typed binding shape:

- button mask;
- logical action;
- trigger kind: SETTLE / RELEASE / HOLD;
- context: DESKTOP / GLOBAL.

B10 also explicitly says the final B4A table was compiled-in and that the audit
found no completed human-readable binding configuration contract. The clean
reconstruction must therefore preserve typed binding semantics without falsely
claiming that user-configurable persistence already existed. The eventual
human-readable config/UI must edit the same semantic binding values.

Frozen B4A additionally proves the important hotkey arbiter rules:

- exact held mask must settle;
- candidate-mask change resets pending progress;
- RELEASE is armed only by legitimate growth into the exact mask;
- growing past an armed release cancels it;
- shrink from a superset cannot manufacture a subset release action;
- a SETTLE chord that is a strict subset of an eligible HOLD chord receives an
  extended settle interval;
- fired actions latch until all hotkey buttons release;
- a desktop-only session records whether it began in desktop so foreground
  changes cannot convert an already-held gesture into a desktop shortcut.

Those are ownership/safety semantics independent of the historical action list.

### Packet boundary

The next bounded packet is therefore:

`A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28`.

R28 reconstructs:

- a small semantic product-action value including MPEG_CALIBRATION;
- typed binding values;
- the pure platform-neutral binding resolver/arbitration state;
- ordinary semantic input-event representation for a product action.

It does **not** choose a physical MPEG binding.

Tests may inject caller-supplied binding values to prove the resolver, but clean
product source must contain no default MPEG_CALIBRATION button mask and no
START+SELECT/750 ms special case.

R28 also stops before live input-runtime integration, human-readable binding
persistence and Application routing. Those are separate dependencies and must be
selected by a later Foreman from then-current repository truth.

This split avoids both prohibited shortcuts:

1. hardwiring the H1 test chord; and
2. making Application interpret raw controller masks directly.

## Foreman-owned state publication

Published Foreman State revision 0067:

`bf26fc0b10435b355a6e97445d04a8a8368028ed`

message:

`docs(foreman): accept R27 and activate semantic binding core`

State 0067:

- accepts R27 and all twelve criteria;
- records exact R27 source, CI and hardware-debt identity;
- activates exactly one next packet:
  `A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28`;
- changes active Reconstruction work key to `a005-interaction-input`;
- preserves B4A typed action/trigger/context and chord arbitration as the
  evidence-supported mechanism shape;
- adds MPEG_CALIBRATION only as semantic meaning;
- selects no physical product binding;
- keeps START+SELECT discarded;
- keeps config persistence, live input-runtime production, Application routing
  and MPEG execution deferred;
- claims no hardware qualification.

Immediately before this immutable log write, branch authority remained exactly
at State 0067.

## Current hardware debt

Current fully Foreman-accepted PS2 loadable authority is R27:

`PT_LOAD_SHA256=30c8d9d1db9106f51c8de56c88e68cf3d6181d98cfd8ff4d402ce24a6c94c13d`
`PT_LOAD_BYTES=513940`

It is repository-reproducible and hardware-pending.

Accepted R25 maintained Pi product/runtime source remains at:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

with no Pi operator/hardware qualification claim.

R28 may add linked Input code and may therefore create a newer PS2 PT_LOAD
identity. Its worker must measure exact identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A005-SEMANTIC-PRODUCT-ACTION-BINDING-CORE-R28
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R28_FROM_FOREMAN_STATE_0067

The Reconstruction worker must independently recover live branch authority,
consume State 0067 plus A005/B07/B10 governing semantics, execute only R28,
create exactly one immutable Reconstruction record using WORK_ITEM_KEY
`a005-interaction-input`, stop, and return the baton.

Do not choose an MPEG-calibration button chord, do not revive START+SELECT, do
not route P9/P10/R21, do not add binding persistence/config UI, do not create an
urgent mailbox, and do not claim hardware qualification.
