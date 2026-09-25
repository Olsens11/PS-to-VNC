DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T10:39:18-04:00
COMPLETED_AT=2026-09-25T10:47:37-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=cb78de304554f1ee61be39a24fd04d256a8bdea9
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R25 accepted; PS2 media-clock product binding R26 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A003-PI-MPEG-ORDINARY-PRODUCT-COMPOSITION-R25`, independently recover live
repository authority, inspect the worker's exact source/test/build range,
disposition R25, reconcile the next dependency against governing audits, and
publish one bounded Reconstruction packet without performing product behavior
implementation from the Foreman seat.

Truthful round start:

`STARTED_AT=2026-09-25T10:39:18-04:00`

Live pickup authority:

`cb78de304554f1ee61be39a24fd04d256a8bdea9`

The branch was eleven commits ahead / zero behind previous Foreman closeout
`501d6ade655e0ef68692ac42b9b9959c340f3e17`.

Governing/relevant authority consumed included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/status.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Foreman State revision 0064;
- source naming/topology authority;
- A002 CONFIG/audio/shared-media-clock audit revision 0001;
- A004 presentation/calibration authority;
- A005 interaction/input audit revision 0001;
- A006 top-level orchestration/shutdown audit revision 0001;
- accepted R15 Pi ordinary RFB composition;
- accepted R17 Pi MPEG generation owner;
- accepted P9/P10 and R21-R24 PS2 MPEG/calibration lifecycle;
- frozen H1 forensic source at
  `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260925T101750-0400__reconstruction__a003-mpeg-generation__interactive.md`.

No newer Validation/Continuity/Diagnostics/support authority displaced R25
during pickup.

## Exact R25 worker range

Reconstruction started from:

`501d6ade655e0ef68692ac42b9b9959c340f3e17`

and landed ten pre-log commits:

1. `aafc2fb583b388f7c1cd05e42b5dcbe28ad934a2` —
   `pi: compose ordinary MPEG generation factory`;
2. `7bc38563034e4e737e907b202d87607ae15ac05f` —
   `test(pi): prove R25 ordinary MPEG composition`;
3. `a9312a7d9653c5c290747d31133513a74401582c` —
   `test(pi): supersede dormant R17 composition assertion`;
4. `651e0ca50490d30d3106bb6d7b32cc44f1b17050` —
   `test(pi): align R25 with lazy RFB credit edge`;
5. `839ae894d4a66288806f0ed6c8d2ba5d95f251d3` —
   `test(pi): wait for exact R25 controller publication`;
6. `8f3e8e7137232f633b878dec2b3b187b8d018ddb` —
   `docs(pi): record R25 MPEG product composition`;
7. `7f6b154df4dc8a4ec646051ae49b02b5ccbfa070` —
   deterministic dictionary-reconciliation trigger;
8. `b395bbc0f727447dcb40a6c5c59bddecc8ef89f7` —
   generated Pi dictionary/portal reconciliation;
9. `db86b263bbf326509a3d1c0ac441454c39989e15` —
   final R25 verification marker;
10. `60b7759fb78d5f555a589b9ce8cb58ce96096945` —
    `test(pi): prove fresh R25 controller per session`.

Final pre-log source authority:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

The next and only following commit was the required immutable Reconstruction
record:

`cb78de304554f1ee61be39a24fd04d256a8bdea9`.

Independent compare proved ten commits ahead / zero behind from the assigning
Foreman log to final source and exactly one log-only commit afterward.

The complete final-source changed-path range is:

- `pi/mpeg_product_profile.py` — new;
- `pi/wire_runtime.py`;
- `pi/README.md`;
- `pi/SYMBOLS.md`;
- `scripts/pi/install-wire-runtime.sh`;
- `tests/unit/pi_mpeg_product_composition_test.py` — new;
- `tests/unit/pi_mpeg_generation_test.py`;
- `tests/Makefile`;
- `docs/development/mpeg-generation-control.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No `pi/wire_server.py`, `pi/mpeg_generation.py`, Wire protocol, PS2
`src/` product source, systemd runtime state, AUDIO, calibration or
active-MPEG recalibration behavior changed.

## Independent R25 source review

### Selected product composition authority

`pi/mpeg_product_profile.py` owns exactly four composition facts outside R17
producer tuning:

- display `:0`;
- desktop width 704;
- desktop height 462;
- retirement timeout 8.0 seconds.

The profile is a frozen dataclass returned through one selected-profile function.
It validates non-empty display, positive/bounded geometry and positive timeout.

The source documents exact frozen H1 deadline provenance and explicitly excludes
Wire identity, producer launch, generation identity, RFB provider state and R17
channel/buffer/frame-rate tuning.

The existing R17 `mpeg_runtime_profile.py` remains unchanged and continues to
own:

- channel window 524288;
- producer buffer 524288;
- max dimensions 704x480;
- rate 30000/1001.

### Ordinary Wire composition

`pi/wire_runtime.py` now imports R17 generation ownership plus both selected
profiles and exposes a narrow exact-session factory.

`selected_mpeg_generation_factory()` captures immutable selected composition
and producer profiles and returns:

`session_id -> MpegGenerationController`.

The controller receives the exact session ID supplied by WireServer plus display,
desktop bounds, retirement timeout and R17 producer profile.

`build_product_wire_server()` now supplies:

- the already-accepted fresh RFB attachment factory; and
- the new fresh MPEG generation factory.

The file contains no physical send/recv owner, no sequence allocation, no START
interpretation, no generation counter and no session-retirement implementation.

### Q4 and producer-admission boundaries

No WireServer mechanism changed.

Focused tests against the real accepted WireServer prove:

- incompatible product-version Q4 invokes the MPEG factory zero times;
- malformed establishment sequencing invokes it zero times;
- exact Q4 ACCEPT establishes the real allocated session identity first;
- only then is the MPEG factory invoked exactly once with that exact identity;
- the returned exact-session controller is attached through the accepted seam.

Fresh R17 controller construction proves:

- producer is absent;
- capture plan is absent;
- generation high-water is zero;
- MPEG credit is zero.

Valid channel-4 CREDIT changes only accepted capacity and does not create a
producer.

Exact START is the first producer-factory invocation.

No real FFmpeg process is launched by the tests.

### Rider independence and physical ownership

The product server carries both RFB and MPEG rider factories.

The focused MPEG test deliberately sends no RFB CREDIT. R13 therefore correctly
keeps its provider lazy; provider connection-attempt count remains zero while
MPEG CREDIT, START, DATA, RETIRE and successor-generation behavior execute.

No new physical serializer exists. The test observes channel-4 DATA and exact
RETIRE completion only after the real WireConnectionOwner serializes them.

Final R25 composition/profile source contains no direct `sendall`, `.recv`,
`start_exact`, real `subprocess.Popen`, PS2 application-MPEG reference or
controller trigger.

### Generation/session freshness

Within one Wire Session, the real R17 owner performs:

generation 1 START -> exact RETIRE/completion -> generation 2 START -> exact
RETIRE/completion.

The same controller retains `highest_generation == 2`; stale generation 1
START is rejected fail-closed.

The final worker hardening proves sequential clean physical Sessions A and B on
one persistent WireServer receive:

- different allocated session IDs;
- distinct controller objects;
- Session B producer/plan absent;
- Session B generation high-water zero;
- exact close of each session-owned controller.

No dead-generation object or mutable generation authority is reused.

### Session retirement and factory failure

WireServer's existing finish path still calls attached controller `close()`.
A false close result marks an otherwise accepted session protocol-failed.

An exception from the exact post-Q4 factory terminates that physical session
without retry/rebind/RFB-only fallback. The persistent server can then accept a
fresh later session and construct a fresh controller.

### Staging

Because R25 adds one maintained Pi product module, the installer now
syntax-checks, safe-target checks, stages, verifies and removes it.

The existing explicit no-live-mutation guard remains. No daemon reload,
enable/disable, start/stop/restart or provider/display mutation was added.

No R25 source defect was found.

## R25 acceptance criteria

A003-R25-C1=MET
A003-R25-C2=MET
A003-R25-C3=MET
A003-R25-C4=MET
A003-R25-C5=MET
A003-R25-C6=MET
A003-R25-C7=MET
A003-R25-C8=MET
A003-R25-C9=MET
A003-R25-C10=MET
A003-R25-C11=MET
A003-R25-C12=MET

Overall disposition:

`R25_DISPOSITION=FOREMAN_ACCEPTED`

## Exact R25 machine/build evidence

Final pre-log source authority:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `36148209311`
attempt `1`
conclusion `success`

The run object independently reports exact head SHA
`60b7759fb78d5f555a589b9ce8cb58ce96096945`, branch
`ledge/h1-all-guns`, push event, and display title
`test(pi): prove fresh R25 controller per session`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile skipped as expected.

Observed Pi evidence includes:

- `pi_wire_server_test.py`: 20 tests, OK;
- `pi_internal_rfb_provider_test.py`: 9 tests, OK;
- `pi_mpeg_generation_test.py`: 12 tests, OK;
- `pi_mpeg_product_composition_test.py`: 9 tests, OK.

Observed cross-domain evidence includes:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app_mpeg_run_test: PASS`.

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=212 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked PS2 identity:

`ELF_PRISTINE_SHA256=7a25c34ccbd1b0047ee346acce4b273c85cc4dae5a38be6ed8e153089817d64d`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=38eb2ed29857687d232d604f21a8e9b2bb267a24fa8605a57149fd1f204033fb`
`PT_LOAD_BYTES=512788`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

This is exactly the accepted P10 PS2 loadable identity.

Evidence classification:

SOURCE_COMPLETE=YES
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
PS2_PT_LOAD_CHANGED=NO
PI_PRODUCT_BYTES_CHANGED=YES
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO_NEW_CLAIM
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Repository-authority correction: START+SELECT is not product binding

While tracing the next dependency, the Foreman re-read A005 interaction/input
audit revision 0001.

A005 explicitly classifies:

`START+SELECT 750 ms calibration entry chord`

as:

`DISCARD_AS_PRODUCT_BINDING`.

The audit identifies it as a temporary hardware-test input adapter. The durable
behavior to preserve is the semantic calibration action and the
foreground/quarantine lifecycle, not that physical chord.

Therefore earlier Foreman prose describing the recovered H1 chord as
"permanent" is not carried forward. Historical immutable logs remain untouched;
State 0065 records the correction.

Any later product activation packet must consume a semantic action from an
earned product/UI binding policy. It must not simply hardwire the H1 test chord
into `local_controller.c` or `app.c`.

## Next dependency analysis: media-clock production binding

R25 closes the Pi-side MPEG product-composition prerequisite.

The PS2 ordinary application still cannot correctly compose P9/P10/R21-R24
because it has no production media-clock binding.

Current `src/media/clock.*` already owns the accepted clean A002 mechanics:

- synchronized epoch publication;
- one-shot arm;
- immutable ticks-per-second;
- signed audio/video presentation offsets;
- saturating deadline math;
- wait behavior through injected time operations.

But ordinary `src/app.c` currently creates no `pstvnc_media_clock_t`, supplies
no `pstvnc_media_clock_sync_t`, and has no current-tick platform observer for
R22 scheduler service.

The existing PS2 system seam exposes bounded delay only.

P10/R21 require a valid clock pointer before exact START. R22 frame service
requires current ticks in the same timer domain as P4's first synchronized
physical presentation.

Therefore main-loop integration now would either invent platform timing/sync
inside Application or duplicate clock ownership. Both would violate A002 and
clean architecture.

### Recovered exact clock authority

A002 requires a session-scoped common media epoch and explicit synchronization
safe enough to replace H1's volatile + `EE_SYNCL()` representation.

Frozen H1 `h1_media_clock.c` proves the PS2 time domain:

- current timer: `GetTimerSystemTime()`;
- timer rate: `kBUSCLK`;
- cooperative yield: `DelayThread(microseconds)`.

P4's accepted platform graphics seam already records first synchronized MPEG
presentation ticks with `GetTimerSystemTime()` and records `kBUSCLK`, so
R22 current-tick observations must use that same exact domain.

Frozen H1 profile authority `h1_profiles.py` supplies the selected values
relevant to the reconstructed clock:

- `media_epoch_lead_us = 0`;
- `video_presentation_offset_us = 0`;
- `audio_presentation_offset_us = 0`.

A002 classifies these as narrow product/profile values rather than requiring
H1's 61-field laboratory CONFIG surface.

The clean clock API already expects injected synchronization and time ops, so no
clock mechanism rewrite is required.

## Packet ordering decision

The next bounded dependency is:

`A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26`.

R26 must establish only:

- one narrow Configuration-selected media-clock profile at exact 0/0/0 values;
- one session-scoped PS2 synchronization object;
- callbacks for the existing media-clock sync seam;
- exact `GetTimerSystemTime()` current-tick observation;
- exact `kBUSCLK` tick-rate authority;
- exact `DelayThread()` delay observation;
- explicit init/release/fresh-session lifecycle.

It must not:

- arm the media clock;
- modify ordinary `src/app.c`;
- choose any controller/physical binding;
- map START+SELECT;
- start P9/P10/R21-R24;
- start AUDIO;
- alter Wire/protocol/Pi behavior;
- copy H1 volatile epoch state.

This keeps the next shift a coherent mechanism-enabling tranche and prevents the
future main-loop packet from inventing timing/synchronization policy.

After R26 is independently reviewed, the Foreman must re-evaluate dependency
truth. A likely later boundary is trigger-agnostic Application session
composition driven by a semantic MPEG-calibration action; the actual user-facing
binding remains separately governed by A005/B10 product policy.

## Foreman-owned state publication

Published Foreman State revision 0065:

`ee41686f274ac814e58d2e5bc0271e275eade8a3`

message:

`docs(foreman): accept R25 and activate media clock binding`

State 0065:

- accepts R25 and all twelve criteria;
- records exact R25 source/evidence;
- preserves P10 PS2 PT_LOAD identity as current accepted PS2 loadable authority;
- records R25's new maintained Pi product source authority without claiming
  operator/hardware qualification;
- corrects the previous START+SELECT product-binding assumption against A005;
- activates exactly one next packet, R26;
- changes the active Reconstruction work key to
  `a002-config-audio-clock`;
- keeps semantic MPEG product entry/main-loop activation deferred.

Immediately before this immutable log write, branch authority remained exactly
at State 0065.

## Current hardware debt

Current fully Foreman-accepted PS2 loadable authority remains P10 because R25
does not change PS2 linked bytes:

`PT_LOAD_SHA256=38eb2ed29857687d232d604f21a8e9b2bb267a24fa8605a57149fd1f204033fb`
`PT_LOAD_BYTES=512788`

It is repository-reproducible and hardware-pending.

Accepted R25 changes maintained Pi product/runtime source at:

`60b7759fb78d5f555a589b9ce8cb58ce96096945`

with no new Pi operator/hardware qualification claim.

R26 is expected to add linked PS2 Configuration/Platform code and may create a
new PS2 PT_LOAD identity. Its worker must measure, not infer, the result.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R26_FROM_FOREMAN_STATE_0065

The Reconstruction worker must independently recover live branch authority,
consume State 0065 and governing A002/A005 authority, execute only R26, create
exactly one immutable Reconstruction record using WORK_ITEM_KEY
`a002-config-audio-clock`, stop, and return the baton.

Do not begin ordinary `app.c` MPEG activation, do not choose a controller
binding, do not revive START+SELECT as a product gesture, do not start AUDIO,
and do not claim hardware qualification.
