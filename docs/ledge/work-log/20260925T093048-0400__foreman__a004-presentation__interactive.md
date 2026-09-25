DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T09:30:48-04:00
COMPLETED_AT=2026-09-25T09:43:09-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=e7baae8177882edbba8c19fba3069886e972b7bb
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — P10 accepted; Pi MPEG ordinary composition R25 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A004-APPLICATION-MPEG-PROTECTED-START-HANDOFF-P10`, independently recover
live repository authority, inspect the worker's exact source/test/build range,
disposition P10, and select one next dependency-correct Reconstruction packet
without performing product implementation from the Foreman seat.

Truthful round start:

`STARTED_AT=2026-09-25T09:30:48-04:00`

Live pickup authority:

`e7baae8177882edbba8c19fba3069886e972b7bb`

The branch was seven commits ahead / zero behind the previous Foreman closeout
`8bc28bd4cb023cbb0cf29625c5da165a37392fc2`.

Governing/relevant authority consumed included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/status.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Foreman State revision 0063;
- Wire Runtime Decisions revision 0011;
- Architecture Overlay revision 0007;
- A004 Calibration Separation Invariant revision 0001;
- A004 presentation/calibration audit revision 0001;
- accepted P9 foreground authority;
- accepted R21-R24 Application MPEG lifecycle;
- accepted R15 ordinary Pi RFB composition;
- accepted R17 Pi exact-generation MPEG owner;
- accepted R11/R12 native provider authority;
- frozen H1 MPEG CALIBRATION / all-guns forensic source at
  `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260925T091219-0400__reconstruction__a004-presentation__interactive.md`.

No newer Validation/Continuity/Diagnostics/support authority displaced the P10
assignment during pickup.

## Exact P10 worker range

Reconstruction started from:

`8bc28bd4cb023cbb0cf29625c5da165a37392fc2`

and landed six pre-log commits:

1. `eb76d3303abea04bf87de38cd1142466b26b8092` —
   `app: add P10 protected MPEG start handoff`;
2. `a2a95f2ea04d736bb7639e7cea8d37f49babbf28` —
   `test(app): prove P10 protected start handoff`;
3. `07a1e734817722972abe94d667743b5724272728` —
   `build: enroll P10 protected start handoff`;
4. `7c06b18f3a21a91c49882f091d864ac465ef77d8` —
   deterministic dictionary-reconciliation trigger;
5. `f2c0e23dc71727237b9e38d42ed576c825797a92` —
   `docs(symbols): reconcile current clean definitions`;
6. `2daf7194c0c92412464d10cee67990c3bb270f63` —
   `test: verify final P10 protected start handoff authority`.

Final pre-log source authority:

`2daf7194c0c92412464d10cee67990c3bb270f63`

The next and only following commit was the required immutable Reconstruction
record:

`e7baae8177882edbba8c19fba3069886e972b7bb`.

Independent compare proved six commits ahead / zero behind from the assigning
Foreman log to final source and exactly one log-only commit afterward.

The complete final-source changed-path range is:

- `src/app_mpeg_activation.c`;
- `src/app_mpeg_activation.h`;
- `src/app_mpeg_calibration.c`;
- `src/app_mpeg_calibration.h`;
- `tests/unit/app_mpeg_activation_test.c`;
- `tests/unit/app_mpeg_calibration_test.c`;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `scripts/continuity-check.sh`;
- `docs/development/mpeg-generation-control.md`;
- `docs/development/source-topology.md`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No ordinary `src/app.c`, `src/app_mpeg_run.*`, P8/P1 behavior, RFB/input/UI
lower owner, Display/P3/compositor/Platform mechanism, P7/MPEG execution,
Transport, Configuration, Pi product, AUDIO, DESKTOP CALIBRATION or Wire
protocol behavior changed.

## Independent P10 source review

The new `app_mpeg_activation` coordinator is generation-stateless and composes
only accepted P9 and R21 public seams.

Admission proves:

- run state IDLE;
- run current generation zero;
- no run teardown requirement;
- P9 accepted geometry is copyable;
- P9-owned P2 pointer exists, is frozen and denies remote publication;
- P9-owned P3 pointer exists in exact RFB_ONLY/no-snapshot state.

Accepted geometry is copied before any run mutation.

The coordinator then invokes exactly one
`pstvnc_app_mpeg_run_start()` with:

- that copied geometry;
- the exact same P2 owner;
- caller-supplied current Transport access;
- the exact P3 owner;
- caller-supplied session media clock.

No run-start mechanism is duplicated.

### Success proof

A returned R21 OK is not treated as sufficient.

P10 independently re-reads the run status and P3 snapshot and requires:

- Application run state STARTED_WAIT_FIRST_FRAME;
- nonzero exact current generation;
- teardown not required;
- P2 still frozen/publication denied;
- P3 exact WAIT_FIRST_FRAME;
- P3 snapshot valid;
- P3 generation exactly equals run current generation;
- P3 geometry exactly equals the P9 geometry copied before start.

Only then does P10 call the new narrow P9 protected-handoff commit seam.

That P9 seam independently verifies the frozen P2 + exact P3 WAIT_FIRST_FRAME
generation/geometry relationship, clears calibration-protected geometry and
borrowed foreground references, and returns P9 to reusable IDLE without
thawing P2, promoting P3, changing run ownership or touching generation
history.

P10 then re-proves the downstream run/P3 state after commit.

### Failure/rollback truth

A non-OK R21 start result may use P9's existing abort only when independent
proof shows:

- run status is healthy;
- run state is IDLE;
- current generation is zero;
- run teardown is not required;
- P3 is exact RFB_ONLY with no snapshot;
- P2 is still frozen.

Only that state proves the R21 pre-START unwind completed cleanly.

If proven, P9 abort thaws once and therefore preserves normal P2 FULL/HOLD debt.

If the run is FAULTED/non-idle, generation remains owned, P3 has acquired
WAIT_FIRST_FRAME or another contradictory state, cleanup cannot be proven, or
the P9 abort/commit proof fails, P10 never manufactures rollback. P9 enters its
new fault-contained handoff state without P2 thaw, and the result explicitly
requires outer teardown.

The P9 fault seam preserves protected geometry/owner evidence and prevents a
normally reusable ACCEPTED_PROTECTED interpretation after downstream authority
has become uncertain.

### Ownership result

On exact success:

- P9 no longer exposes accepted geometry;
- P9 is IDLE;
- P2 remains continuously frozen;
- P3 remains exact WAIT_FIRST_FRAME;
- app_mpeg_run is the sole Application MPEG lifecycle owner;
- generation allocation/high-water state remains exclusively in app_mpeg_run.

No P10 defect was found.

### P10 acceptance criteria

A004-P10-C1=MET
A004-P10-C2=MET
A004-P10-C3=MET
A004-P10-C4=MET
A004-P10-C5=MET
A004-P10-C6=MET
A004-P10-C7=MET
A004-P10-C8=MET
A004-P10-C9=MET
A004-P10-C10=MET
A004-P10-C11=MET
A004-P10-C12=MET

Overall disposition:

`P10_DISPOSITION=FOREMAN_ACCEPTED`

## Exact P10 machine/build evidence

Final pre-log source authority:

`2daf7194c0c92412464d10cee67990c3bb270f63`

Canonical GitHub Actions workflow:

`Ledge reconstruction checks`
run `36140699072`
attempt `1`
conclusion `success`

The run object independently reports exact head SHA
`2daf7194c0c92412464d10cee67990c3bb270f63`, branch
`ledge/h1-all-guns`, push event, and display title
`test: verify final P10 protected start handoff authority`.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile skipped as expected.

Observed host evidence includes:

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

Observed repository/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=210 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=7a25c34ccbd1b0047ee346acce4b273c85cc4dae5a38be6ed8e153089817d64d`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=38eb2ed29857687d232d604f21a8e9b2bb267a24fa8605a57149fd1f204033fb`
`PT_LOAD_BYTES=512788`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

Evidence classification:

SOURCE_COMPLETE=YES
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
LOADABLE_BYTES_CHANGED=YES
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
HARDWARE_PENDING=YES
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

## Next dependency analysis

P10 makes it mechanically possible for a successful MPEG CALIBRATION acceptance
to enter a real R21 START transaction.

The permanent H1 entry gesture has also been recovered exactly:

- held START+SELECT;
- 750000 microseconds uninterrupted hold;
- ordinary RFB/desktop foreground only;
- the initial hold and intermediate samples are consumed so SELECT cannot open
  OSK;
- early release cancels and consumes that cancellation sample;
- a nonblocking caller-supplied monotonic clock owns deadline observation;
- threshold crossing produces one activation edge.

That gesture is not selected for the next packet yet.

The reason is dependency correctness: the ordinary Pi product currently remains
RFB-only even though the R17 MPEG mechanism exists. Current
`pi/wire_runtime.py` supplies the R15 RFB attachment factory but deliberately
supplies no `mpeg_generation_factory`. If PS2 ordinary input/main-loop wiring
were activated first, P10 could successfully serialize START into a product Pi
Wire Session with no MPEG rider, making the newly exposed product path
structurally incomplete.

The Pi product composition must therefore come first.

## Pi MPEG composition authority recovered

The accepted lower seams already exist:

- WireServer accepts an optional
  `session_id -> MpegGenerationController` factory;
- WireServer invokes it only after exact Q4 ACTIVE/session identity;
- the attached controller never owns the PS2-facing socket;
- WireConnectionOwner remains sole recv/send/global-sequence owner;
- session finish invokes controller `close()`;
- R17's MpegGenerationController is inert until exact START;
- R17 already owns generation identity, producer, suppression, channel-4 credit,
  emission leases and exact RETIRE proof.

Current ordinary provider authority selects:

`existing LightDM/Xorg :0 -> X0tigervnc -> 127.0.0.1:5900 internal provider`.

That exact `:0` source is therefore the current ordinary product capture
display.

The clean PS2 display contract fixes:

`PSTVNC_DISPLAY_WIDTH=704`
`PSTVNC_DISPLAY_HEIGHT=462`.

Ordinary RFB session startup rejects ServerInit unless it matches the exact
expected 704x462 values before READY. These are therefore not speculative Pi
capture dimensions: they are the exact ordinary desktop coordinate contract for
a usable current product session.

The previously unresolved Pi producer retirement deadline was recovered from
frozen H1 exact-generation producer authority at forensic commit
`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`:

`DEFAULT_RETIRE_TIMEOUT_SECONDS = 8.0`

in
`experiments/media-harness-h1/h1_cp2p_mpeg_producer.py`.

That product path closes new emission, waits out admitted physical-send leases,
requests producer stop, drains/discards unsent producer bytes and treats deadline
expiration only as failure detection. It never creates retirement success from
the passage of time.

The historical H1 wrapper also had separate process-termination grace machinery.
R25 is not authorized to copy that obsolete wrapper into R17. The recovered
8.0-second value is the composition input to R17's already-accepted retirement
mechanism, not permission to rewrite the mechanism.

Historical all-guns evidence later exercised Pi MPEG producer retirement after a
600-second workload, but reconstructed R17/R25 source remains independently
hardware-unqualified.

## Packet ordering decision

The next bounded dependency is therefore:

`A003-PI-MPEG-ORDINARY-PRODUCT-COMPOSITION-R25`

R25 must:

- publish one narrow selected Pi MPEG composition/profile authority for
  `:0`, 704x462 and 8.0 seconds;
- supply one fresh exact-session R17 MPEG factory alongside R15's RFB factory;
- keep controller construction inert;
- preserve exact START as the sole producer launch edge;
- preserve WireConnectionOwner as sole physical serializer;
- preserve same-session generation high-water state and fresh-session
  replacement;
- preserve exact close/retirement proof;
- leave PS2 ordinary trigger/main-loop activation completely deferred.

This is intentionally narrower than an all-guns activation packet.

After R25 is independently accepted, the Foreman can re-evaluate the next
dependency. The likely downstream boundary is RFB_ONLY ordinary PS2 MPEG entry
and main-loop lifecycle composition using the recovered 750 ms START+SELECT
gesture, P9/P10, R22 live service and R23/R24 retirement/restoration. Active-MPEG
recalibration remains separately packetizable because frozen H1 authority
requires old-generation retirement plus a new post-retirement FULL RFB refresh
before fresh calibration.

## Foreman-owned state publication

Published Foreman State revision 0064:

`cf0fa37ff0d7ef5b0c74fc6ee79cf207ac14919d`

message:

`docs(foreman): accept P10 and activate Pi MPEG composition`

State 0064:

- accepts P10 and all twelve criteria;
- records exact P10 source/evidence/new linked hardware debt;
- marks protected-start handoff Foreman-accepted;
- activates exactly one next packet, R25;
- changes the active Reconstruction work key to `a003-mpeg-generation`;
- keeps PS2 permanent trigger/main-loop activation deferred;
- records the recovered H1 8.0-second retirement-policy authority;
- does not claim reconstructed Pi or PS2 hardware qualification.

Immediately before this immutable log write, branch authority remained exactly
at State 0064.

## Current hardware debt

Current fully Foreman-accepted behavior-bearing PS2 loadable authority is P10:

`PT_LOAD_SHA256=38eb2ed29857687d232d604f21a8e9b2bb267a24fa8605a57149fd1f204033fb`
`PT_LOAD_BYTES=512788`

It is repository-reproducible and not physically hardware-qualified.

R25 is primarily Pi-side composition, but its worker must still report the exact
canonical PS2 linked identity and may not infer load neutrality from source
scope.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A003-PI-MPEG-ORDINARY-PRODUCT-COMPOSITION-R25
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_R25_FROM_FOREMAN_STATE_0064

The Reconstruction worker must independently recover live branch authority,
consume State 0064, execute only R25, create exactly one immutable Reconstruction
record using WORK_ITEM_KEY `a003-mpeg-generation`, stop, and return the baton.

Do not wire START+SELECT or ordinary PS2 `src/app.c` MPEG activation, do not
implement active-MPEG recalibration, do not change R17/WireServer lower
mechanisms unless an accepted-public-seam defect blocks the packet, do not mutate
live Pi/systemd/provider state, and do not claim hardware qualification.
