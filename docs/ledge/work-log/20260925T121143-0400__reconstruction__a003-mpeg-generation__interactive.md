DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T12:11:43-04:00
COMPLETED_AT=2026-09-25T12:28:23-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=bc03d921ed08b0156db3de9ae4a4b60f7b751604
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — A003 R27 dormant PS2 MPEG session foundation

## Recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`bc03d921ed08b0156db3de9ae4a4b60f7b751604`.

Foreman State revision `0066` had independently accepted R26 and activated
exactly:

`A003-R27-APPLICATION-MPEG-SESSION-FOUNDATION`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `ARCHITECTURE_BLOCKER=NONE`;
- semantic MPEG calibration/entry still deferred;
- ordinary MPEG product activation still deferred;
- START+SELECT explicitly not a product binding;
- no P9/P10/R21 MPEG activation, media-clock arm, AUDIO activation, Pi change,
  Wire-byte change, or hardware qualification authorized.

The worker consumed Foreman State revision 0066, Reconstruction Contract
revision 0006, Work Log Contract revision 0007, Architecture Overlay revision
0007, Wire Runtime Decisions revision 0011, accepted R7 MPEG profile authority,
accepted R15/R16B/R19 ordinary RFB Application behavior, accepted R21-R24/P10
MPEG lifecycle seams, accepted R25 Pi product composition, accepted R26
Configuration/Platform media-clock binding, and current development topology/
lifecycle documentation.

The GitHub-connected Worker seat cannot observe external local/Pi checkout
staging, untracked files or worktree dirtiness. No local reset, clean, stash or
checkout-over is claimed.

## Result

R27 is source-complete as a Reconstruction candidate for independent Foreman
review.

Final pre-log authority:

`b126b749c2d3b3182c9b562973614106e98f0edb`

R27 changes ordinary PS2 session composition only. The existing RFB Application
loop now opens one MPEG-capable Transport session and owns one fresh unarmed
media-clock binding/clock for that exact physical attempt, while MPEG execution
remains completely dormant.

## Selected-profile admission before platform startup

`pstvnc_app_run()` now resolves:

1. the selected R14/R15 RFB Transport projection;
2. the selected R7 MPEG runtime profile;
3. the selected R26 media-clock profile;

before entering any IOP/network/platform/session startup.

The selected MPEG profile is required to be non-null. RFB projection failure or
missing MPEG profile returns immediately with zero platform/connect/Transport
ownership. The R26 media-clock selector is by-value immutable selected
authority and has no nullable runtime representation.

The configured Application entry is now:

`pstvnc_app_run_with_session_profiles()`

and rejects null RFB Transport, MPEG Transport, or media-clock profile pointers
before IOP startup.

Focused default-entry evidence records all successful profile acquisitions at
event-count zero, before the first `PSTVNC_APP` platform event.

## One physical Transport owner with dormant MPEG capacity

The ordinary Application path no longer calls:

`pstvnc_transport_session_open()`.

Each fresh attempt calls exactly:

`pstvnc_transport_session_open_with_mpeg()`

with:

- the unchanged selected RFB `pstvnc_transport_session_config_t`;
- the exact selected R7 `profile->transport`
  `pstvnc_transport_mpeg_channel_config_t`.

No second Transport runtime or channel-4 owner was introduced.

Focused Application evidence records both structures field-for-field and proves
the plain constructor call count is zero.

The deterministic source-boundary test also strips comments and requires
exactly one MPEG-capable Transport constructor call and zero plain
`pstvnc_transport_session_open(...)` calls in ordinary `src/app.c`.

## Fresh per-session R26 binding and A002 clock

After Transport admission and before RFB session startup, every attempt creates
one automatic:

`pstvnc_ps2_media_clock_binding_t`

initialized from `PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER`.

That binding's lifetime begins on that loop-body entry and ends with that
physical attempt; a retired binding object is not reset and reinitialized for
the successor session.

Application obtains from the exact binding:

- `pstvnc_media_clock_sync_t`;
- exact binding tick-rate authority.

It then initializes one attempt-local:

`pstvnc_media_clock_t`

using the selected R26 profile and the exact tick rate, and immediately requires
`pstvnc_media_clock_is_armed(...)` to report false.

R26 remains owner of the concrete EE semaphore/timer mechanism. A002
`src/media/clock.*` remains owner of epoch, armed state, offsets, deadline
arithmetic, and wait policy.

Focused host evidence assigns distinct synchronization identities across
successive attempts and proves every observed clock initialization receives
selected 0/0/0 profile values and the accepted 147456000-Hz `kBUSCLK` domain.

The unchanged R26 focused tests independently retain the direct
`GetTimerSystemTime()/kBUSCLK/DelayThread()` mechanism proof.

## Clock remains dormant

R27 never calls `pstvnc_media_clock_arm()`.

The source-boundary fixture explicitly rejects any R27 source call to that
operation. The accepted R26 ownership test was updated only where R27
legitimately supersedes its historical "ordinary app does not consume R26"
assumption; it still requires no Application media-clock arm.

Ordinary RFB startup, presentation, input/UI service, P2 request/publication and
idle polling therefore operate with the session clock unarmed.

P4's accepted first synchronized MPEG presentation remains the future arm
boundary.

## Provider-local replacement retirement order

R16B provider-local recovery remains the only fresh-session re-entry condition.

After provider CONNECT/READ/WRITE failure closes old-attempt admission,
replacement requires the following order:

1. prove input worker dormancy through the existing input shutdown seam;
2. release the exact active R26 media-clock binding;
3. abort/retire the old Transport session;
4. only then admit the next physical connect.

Focused Session-A/Session-B evidence records:

`INPUT_SHUTDOWN < MEDIA_BINDING_RELEASE < TRANSPORT_ABORT < CONNECT_B`.

The successor attempt performs a second fresh binding init and second clean
media-clock init with a distinct scripted semaphore/synchronization identity.

No old clock armed/epoch state is copied into the new `pstvnc_media_clock_t`.

## Binding-release failure blocks replacement

If R26 binding release returns failure, Application records that release proof
as failed, still continues monotonic Transport teardown, and does not admit a
replacement physical session.

The R26 release contract revokes local binding authority before reporting a
kernel `DeleteSema()` failure. R27 therefore never fabricates a second release
attempt or treats the old binding as reusable.

Focused evidence injects release failure after an R16B provider READ failure and
proves:

- one physical connect only;
- one Transport open only;
- one binding init only;
- one binding release attempt only;
- release precedes Transport abort;
- no Session-B connect occurs.

## Reverse-ownership fatal cleanup

Fatal convergence inside an admitted attempt preserves:

1. input shutdown if owned;
2. media-clock binding release if owned;
3. Transport abort/release;
4. resident graphics/diagnostics cleanup under existing policy.

A failure before binding initialization never fabricates release ownership.

Focused evidence proves:

- Transport pre-adoption failure performs zero binding init/release;
- binding-init failure aborts the admitted Transport without a fabricated
  binding release;
- a newly initialized clock that appears armed is rejected before RFB session
  startup and is cleaned up by binding release before Transport abort.

Existing R16B tests still prove unproven input or Transport retirement blocks
replacement and converges fatal.

## RFB behavior retained

R27 intentionally preserves the existing ordinary RFB loop after session
foundation creation.

The same established Application fixture continues to prove:

- R15 selected RFB projection;
- R16B provider CONNECT/READ/WRITE fresh-session recovery;
- generic RFB I/O remains fatal rather than provider-recovery eligible;
- input dormancy and Transport retirement fences;
- R19 P2 request accounting, FULL/incremental/HOLD policy, update completion,
  publication gating, and fresh-attempt P2 state;
- controller/keyboard/mouse/local-UI behavior from the frozen broad legacy
  fixture.

Final host output:

`app R15/R16B/R19/R27 tests: PASS`.

## Explicit MPEG dormancy / non-scope

The deterministic R27 source-boundary proof rejects all of the following from
ordinary `src/app.c`:

- `pstvnc_media_clock_arm()`;
- `pstvnc_transport_mpeg_run_open()`;
- generation START/RETIRE send;
- P9 calibration entry;
- P10 protected activation;
- R21 start/live-service/retirement;
- MPEG worker/decoder direct operations;
- P3 mutation;
- START+SELECT product binding.

No R27 source change was made to:

- `src/config/mpeg_runtime_profile.*`;
- `src/config/media_clock_profile.*`;
- `src/platform/ps2_media_clock.*`;
- `src/media/clock.*`;
- Transport runtime/bridge implementation;
- RFB/P2/input/UI mechanisms;
- R21-R24/P9/P10 implementation;
- Pi product source;
- Wire protocol bytes;
- AUDIO.

Accepted R25 Pi composition therefore remains producer-inert until a future
authorized exact START. R27 merely makes logical channel 4 and session media
clock resources available beneath the still-deferred semantic action.

## Historical-test reconciliation

The first R27 host run exposed one expected superseded R26 assertion:
`media_clock_product_binding_source_test.py` still required ordinary
`app.c` to contain no R26 profile/binding references.

That was historically correct for R26 and is exactly what R27 later changes.
The test was updated to require R27's new ordinary composition while retaining
all R26 lower-owner constraints and requiring that Application still never call
`pstvnc_media_clock_arm()`.

No product source was changed to satisfy that historical-test correction.

## Documentation and dictionary reconciliation

Living documentation now records R27 in:

- `docs/development/mpeg-generation-control.md`;
- `docs/development/media-clock-product-binding.md`;
- `docs/development/source-topology.md`.

After source/tests/docs stabilized, the Worker emitted the exact authorized
no-tree-change trigger:

`664ffd10303dbaca39c8b179a68efdd2ccdc0bdd`

message:

`tooling(symbols): run deterministic dictionary reconciliation`.

The repository-authorized reconciler produced:

`8ea2cb83568a833b319767af1db6da1c492ddb5f`

message:

`docs(symbols): reconcile current clean definitions`.

That generated commit changed only:

- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

Final source-identical verification commit:

`b126b749c2d3b3182c9b562973614106e98f0edb`

message:

`test: verify final R27 dormant MPEG session authority`.

## Commits

The complete pre-log Worker range is seven commits ahead / zero behind starting
Foreman authority:

1. `595d969c49c8fc093f9e587f434b4560d759c2eb` —
   `app: compose dormant MPEG session foundation`;
2. `7897780f420739b13b255cb2c50b0cbb85f5573f` —
   `test(app): prove R27 dormant session foundation`;
3. `6e1bc6683d741760ec4afc8e33ec10e545531bb4` —
   `test(media): supersede R26 app deferral at R27`;
4. `68c024f6ccc704961e9d058e2e34e5e9a7abd334` —
   `docs(app): record R27 dormant MPEG session foundation`;
5. `664ffd10303dbaca39c8b179a68efdd2ccdc0bdd` —
   deterministic dictionary reconciliation trigger;
6. `8ea2cb83568a833b319767af1db6da1c492ddb5f` —
   generated root dictionary/portal reconciliation;
7. `b126b749c2d3b3182c9b562973614106e98f0edb` —
   exact final source verification.

No concurrent branch movement or write collision occurred during Worker writes.

## Changed paths

Exact compare from starting authority
`bc03d921ed08b0156db3de9ae4a4b60f7b751604` to final pre-log authority
`b126b749c2d3b3182c9b562973614106e98f0edb` is seven commits ahead / zero
behind.

Changed paths are only:

- `src/app.c`;
- `src/app.h`;
- `src/SYMBOLS.md`;
- `tests/unit/app_test.c`;
- `tests/unit/app_test_legacy.inc`;
- `tests/unit/app_mpeg_session_foundation_source_test.py`;
- `tests/unit/media_clock_product_binding_source_test.py`;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `docs/development/mpeg-generation-control.md`;
- `docs/development/media-clock-product-binding.md`;
- `docs/development/source-topology.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A003-R27-C1 REQUIRED_SELECTED_PROFILES_RESOLVE_BEFORE_PLATFORM_SESSION_START`
  — MET.
- `A003-R27-C2 ORDINARY_SESSION_OPENS_ONE_TRANSPORT_RUNTIME_WITH_SELECTED_MPEG_CHANNEL`
  — MET.
- `A003-R27-C3 CHANNEL4_CAPACITY_REMAINS_PRODUCER_AND_RUN_INERT_WITHOUT_START`
  — MET.
- `A003-R27-C4 EACH_ACCEPTED_WIRE_SESSION_OWNS_ONE_FRESH_UNARMED_MEDIA_CLOCK_BINDING`
  — MET.
- `A003-R27-C5 ORDINARY_RFB_PATH_NEVER_ARMS_MEDIA_CLOCK`
  — MET.
- `A003-R27-C6 PROVIDER_REPLACEMENT_RETIRES_CLOCK_BEFORE_OLD_TRANSPORT_AND_NEXT_CONNECT`
  — MET.
- `A003-R27-C7 CLOCK_RELEASE_FAILURE_BLOCKS_REPLACEMENT_REUSE`
  — MET.
- `A003-R27-C8 FATAL_CLEANUP_PRESERVES_REVERSE_OWNERSHIP_ORDER`
  — MET.
- `A003-R27-C9 REPEATED_SESSION_ATTEMPTS_REBUILD_CLOCK_AUTHORITY_FROM_ZERO`
  — MET.
- `A003-R27-C10 R15_R16B_R19_RFB_INPUT_UI_BEHAVIOR_REMAINS_GREEN`
  — MET.
- `A003-R27-C11 NO_CALIBRATION_TRIGGER_RUN_START_FRAME_SERVICE_OR_PROTOCOL_SCOPE_CREEP`
  — MET.
- `A003-R27-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN`
  — MET.

All twelve R27 criteria are therefore `MET` as Worker dispositions.

## Final machine evidence

Exact final pre-log authority:

`b126b749c2d3b3182c9b562973614106e98f0edb`

Canonical GitHub Actions workflow:

`36160798566`

attempt:

`1`

conclusion:

`SUCCESS`.

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Observed focused/cross-domain evidence includes:

- `APP_MPEG_SESSION_FOUNDATION_SOURCE_TEST=PASS`;
- `app R15/R16B/R19/R27 tests: PASS`;
- `media_clock_test: PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_TEST=PASS`;
- `MEDIA_CLOCK_PRODUCT_BINDING_SOURCE_TEST=PASS`;
- `transport_mpeg_test: PASS`;
- R25 `pi_mpeg_product_composition_test.py`: 9 tests, OK;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=216 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- pinned compile explicitly compiled current `src/app.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact R27 linked PS2 identity:

- `ELF_PRISTINE_SHA256=7f5decc9f09b32d1060f18eca1462bcb189cc3ce96297fd5b3435ba448288d2c`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=30c8d9d1db9106f51c8de56c88e68cf3d6181d98cfd8ff4d402ce24a6c94c13d`;
- `PT_LOAD_BYTES=513940`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

The prior fully Foreman-accepted R26 PS2 identity was:

- `ELF_PRISTINE_SHA256=2dd5cd5a5083c659efcdc3f3cdbcafc9c3147f8ba964088b2d99d79b3e963613`;
- `PT_LOAD_SHA256=e3b84ce42a3527c9518643edd808228637c980f5ea901fd0cf5256e28b6858aa`;
- `PT_LOAD_BYTES=513812`.

R27 therefore changes linked PS2 loadable bytes. The new identity is repository
reproducible but **hardware-pending**. No physical/hardware qualification is
inherited or claimed.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_R27`
`HOST_TESTED=PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`PS2_PT_LOAD_CHANGED=YES`
`MACHINE_EVIDENCE=GITHUB_ACTIONS`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO`
`HARDWARE_PENDING=YES`
`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State/contract accounting

Consumed:

- Foreman State revision `0066`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- accepted R7/R15/R16B/R19/R21-R26 authority.

Produced:

- no Foreman state revision;
- no architecture/contract revision;
- no semantic MPEG product action;
- no controller binding;
- no media-clock arm;
- no MPEG run start/service/retirement/reveal;
- no AUDIO activation;
- no Pi/Wire/protocol revision;
- no Foreman acceptance;
- no independent Validation result;
- no hardware qualification.

## Findings / blockers

No R27 source blocker remains within the authorized packet.

The accepted Transport MPEG constructor, R7 selected profile, R26 binding/clock
seams, and existing Application recovery policy were sufficient. No lower-owner
implementation had to be changed.

The next semantic MPEG action/controller binding remains downstream authority
and was not selected by this Worker.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R27_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect R27 source/evidence/log, decide acceptance,
and select any later bounded packet. Reconstruction must not self-accept R27 or
begin deferred semantic MPEG product activation.
