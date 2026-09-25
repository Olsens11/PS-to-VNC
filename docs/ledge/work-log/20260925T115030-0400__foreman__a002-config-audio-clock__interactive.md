DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T11:50:30-04:00
COMPLETED_AT=2026-09-25T11:56:27-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=dd8a116ff1473aaedd5483e671efb8bc8b767718
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R26 accepted; Application MPEG session foundation R27 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26`, independently recover live
repository authority, inspect the worker's exact source/test/build range,
disposition R26, and publish one bounded dependency-correct next Reconstruction
packet without implementing product behavior from the Foreman seat.

Truthful round start:

`STARTED_AT=2026-09-25T11:50:30-04:00`

Live pickup authority:

`dd8a116ff1473aaedd5483e671efb8bc8b767718`

The branch was eight commits ahead / zero behind prior Foreman closeout
`e3b55a804a3133c7bd1c495772039d122a403aff`.

Consumed governing/relevant authority included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/status.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Foreman State revision 0065;
- A002 CONFIG/audio/shared-media-clock audit revision 0001;
- A005 interaction/input audit revision 0001;
- A006 top-level orchestration/shutdown audit revision 0001;
- accepted R7 MPEG runtime profile;
- accepted R15/R16B/R19 ordinary RFB Application composition;
- accepted R18/R20 Transport MPEG run/control seams;
- accepted R21-R24/P9/P10 Application MPEG/calibration lifecycle;
- accepted R25 Pi ordinary MPEG composition;
- frozen H1 forensic authority at
  `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260925T105026-0400__reconstruction__a002-config-audio-clock__interactive.md`.

No newer Validation/Continuity/Diagnostics/support authority displaced R26
during pickup.

## Exact R26 worker range

Reconstruction started from:

`e3b55a804a3133c7bd1c495772039d122a403aff`

and landed seven pre-log commits:

1. `5afc2306d9caef768a570e7593eb1a51127e229d` —
   `media: bind selected PS2 session clock authority`;
2. `1aed69db46c1a975128b0633ca685b30c69f5045` —
   `docs(media): record R26 clock product binding`;
3. `e6301e484e40785ea16e7195f28231f9f3f2c65b` —
   deterministic dictionary-reconciliation trigger;
4. `bb425d993e4e486315edc0ae1a1fa7c6340c23b5` —
   generated dictionary/portal reconciliation;
5. `3ffcd17a6a41fd120037ae8b80bb17cc3bdabe22` —
   final R26 verification marker;
6. `23745eba862490b621c2cfd4ee35e6bb6144aa1e` —
   `docs: index R26 media clock binding note`;
7. `82a21a3a68a4288362003df2346783cb0c12ed0b` —
   `test: prove R26 media clock ownership boundaries`.

Final pre-log source authority:

`82a21a3a68a4288362003df2346783cb0c12ed0b`

The next and only following commit was the required immutable Reconstruction log:

`dd8a116ff1473aaedd5483e671efb8bc8b767718`.

Independent compare proved seven commits ahead / zero behind from assigning
Foreman authority to final source and exactly one log-only commit afterward.

The complete final-source changed-path range is limited to:

- `src/config/media_clock_profile.{c,h}`;
- `src/platform/ps2_media_clock.{c,h}`;
- local Configuration/Platform symbol dictionaries;
- focused host/source tests and exact PS2 host stubs;
- `tests/Makefile`;
- pinned PS2 compile/link enrollment;
- media-clock product-binding/source-topology/development documentation;
- deterministic dictionary portal;
- exhaustive documentation index.

No ordinary `src/app.c`, `src/app.h`, `src/app_mpeg_*`,
`src/media/clock.*`, local-controller/UI, AUDIO, Transport/Wire, Pi product or
forensic H1 behavior changed.

## Independent R26 source review

### Configuration authority

`src/config/media_clock_profile.*` publishes exactly:

- epoch lead 0 us;
- audio presentation offset 0 us;
- video presentation offset 0 us.

Selection returns a value copy. A caller may mutate its local copy without
mutating Configuration-owned selected authority. The source records A002 and
frozen H1 provenance and explicitly does not extend PSTV CONFIG or create a
runtime/user tuning surface.

### Sole media-clock mechanism preserved

No change was made to `src/media/clock.c` or `clock.h`.

The accepted media owner therefore remains the only implementation of:

- epoch storage;
- armed state;
- one-shot arm;
- signed offset conversion;
- saturation;
- deadline calculation;
- clock wait policy.

R26 does not call `pstvnc_media_clock_arm()`.

### PS2 binding ownership

`src/platform/ps2_media_clock.*` owns one small session-scoped adapter:

`NEW -> ACTIVE -> RETIRED`.

The only mutable authority is one EE semaphore ID plus binding state.

Successful initialization creates one semaphore with:

- init count 1;
- max count 1;
- option 0.

The accepted clock's injected lock/unlock callbacks call only
`WaitSema(binding->semaphore_id)` and
`SignalSema(binding->semaphore_id)`, and propagate failure.

Initialization pessimistically marks a candidate RETIRED before CreateSema. A
create failure therefore cannot expose or retry partially initialized authority.

Release snapshots the semaphore ID, revokes local authority first, and only then
calls DeleteSema. Consequently a DeleteSema failure is still a failure, but
previously copied observer structs reference a RETIRED binding and cannot
resurrect or reuse the old semaphore as product authority.

### Exact timer/yield domain

Current tick and the injected read-ticks operation use direct
`GetTimerSystemTime()`.

The selected tick rate is exactly `kBUSCLK`.

The injected delay operation delegates the exact requested microseconds to
`DelayThread()`, rejecting values not representable by the platform call and
propagating negative/failure results.

This matches the timer domain already used by accepted P4 synchronized graphics
timestamps. No conversion or alternate timer axis is invented.

### Repeated-session behavior

The focused tests prove:

- Session A gets one binding and observer context;
- release retires it;
- stale Session-A sync/time observers fail afterward;
- Session B uses a separate binding and fresh semaphore identity;
- Session-B observers work independently;
- no epoch/armed/deadline/media-generation state exists in the Platform adapter.

No R26 defect was found.

## R26 acceptance criteria

A002-R26-C1=MET
A002-R26-C2=MET
A002-R26-C3=MET
A002-R26-C4=MET
A002-R26-C5=MET
A002-R26-C6=MET
A002-R26-C7=MET
A002-R26-C8=MET
A002-R26-C9=MET
A002-R26-C10=MET
A002-R26-C11=MET
A002-R26-C12=MET

Overall disposition:

`R26_DISPOSITION=FOREMAN_ACCEPTED`

## Exact R26 machine/build evidence

Final pre-log source authority:

`82a21a3a68a4288362003df2346783cb0c12ed0b`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `36155777592`
attempt `1`
conclusion `success`

The exact run object reports:

- branch `ledge/h1-all-guns`;
- head SHA `82a21a3a68a4288362003df2346783cb0c12ed0b`;
- push event;
- title `test: prove R26 media clock ownership boundaries`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile skipped as expected.

Observed focused/cross-domain evidence:

- `media_clock_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_SOURCE_TEST=PASS`;
- R25 Pi MPEG product composition: 9 tests, OK;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=214 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned compile explicitly compiled both new R26 translation units;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=2dd5cd5a5083c659efcdc3f3cdbcafc9c3147f8ba964088b2d99d79b3e963613`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=e3b84ce42a3527c9518643edd808228637c980f5ea901fd0cf5256e28b6858aa`
`PT_LOAD_BYTES=513812`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This differs from the prior fully accepted P10/R25 PS2 PT_LOAD and becomes the
new current hardware-debt identity.

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

## Next dependency analysis

R26 makes the accepted A002 media clock constructible on the PS2, but ordinary
Application still does not instantiate it.

A second independent prerequisite is also still absent from ordinary
Application: current `src/app.c` opens Transport with
`pstvnc_transport_session_open()`, which creates an RFB-only runtime.
Transport's accepted MPEG run-open seam explicitly requires
`runtime->mpeg_enabled`. Therefore R21 cannot later start an MPEG run in the
ordinary product unless session admission first opts into the selected MPEG
channel.

Accepted Configuration R7 already owns the exact MPEG Transport subprofile.
Accepted Transport already exposes
`pstvnc_transport_session_open_with_mpeg()`.
Accepted R25 makes the corresponding Pi MPEG rider ordinary product
composition, and R25 independently proves controller construction/CREDIT remain
producer-inert until exact START.

The next packet can therefore safely compose these two dormant session
prerequisites without entering MPEG execution:

- ordinary Application resolves selected RFB, MPEG and media-clock profiles
  before startup;
- each physical attempt opens one Transport runtime with the selected MPEG
  channel;
- each accepted session builds one fresh R26 clock binding and one clean,
  unarmed A002 clock;
- provider-local R16B replacement retires the old clock binding before old
  Transport retirement and before the next connection;
- release failure blocks fresh-session reuse;
- all existing RFB/input/UI behavior remains intact.

This is a coherent session-ownership tranche. It avoids both a micro-packet that
only switches one constructor and an overbroad all-guns activation packet.

A005 remains governing: no physical product gesture is selected here.
START+SELECT remains discarded as product binding. P9/P10/R21-R24 and the
semantic MPEG product action remain downstream.

## Foreman-owned state publication

Published Foreman State revision 0066:

`92f745bb0fa9bb0b04091a843793db77673f0a6b`

message:

`docs(foreman): accept R26 and activate MPEG session foundation`

State 0066:

- accepts R26 and all twelve criteria;
- records exact R26 source, CI and new PS2 hardware-debt identity;
- activates exactly one next packet:
  `A003-APPLICATION-MPEG-SESSION-FOUNDATION-R27`;
- changes active Reconstruction work key to `a003-mpeg-generation`;
- requires ordinary Transport to opt into the existing selected MPEG channel;
- requires one fresh unarmed R26/A002 clock per physical session;
- preserves R16B replacement ordering and fail-closed reuse;
- keeps P9/P10/R21-R24 product activation and semantic trigger binding deferred;
- preserves A005's rejection of START+SELECT as product binding;
- claims no hardware qualification.

Immediately before this immutable log write, branch authority remained exactly
at State 0066.

## Current hardware debt

Current fully Foreman-accepted PS2 loadable authority is R26:

`PT_LOAD_SHA256=e3b84ce42a3527c9518643edd808228637c980f5ea901fd0cf5256e28b6858aa`
`PT_LOAD_BYTES=513812`

It is repository-reproducible and hardware-pending.

Accepted R25 maintained Pi product/runtime source remains at:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

with no Pi operator/hardware qualification claim.

R27 is expected to modify ordinary Application linked code and will likely
create a newer PS2 PT_LOAD identity. The worker must measure exact identity.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A003-APPLICATION-MPEG-SESSION-FOUNDATION-R27
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R27_FROM_FOREMAN_STATE_0066

The Reconstruction worker must independently recover live branch authority,
consume State 0066 and A005/A006 governing semantics, execute only R27, create
exactly one immutable Reconstruction record using WORK_ITEM_KEY
`a003-mpeg-generation`, stop, and return the baton.

Do not begin P9 calibration foreground, P10/R21 START, frame service,
retirement/reveal, semantic-action product binding, AUDIO activation, Pi/Wire
protocol changes or hardware qualification.
