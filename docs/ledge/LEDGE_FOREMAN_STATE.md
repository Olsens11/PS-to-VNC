# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0025
RECORDED_AT=2026-09-20T18:08:52-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0024
SUPERSEDES_FOREMAN_STATE_REVISION=0024
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0006
BASED_ON_WORK_LOG_CONTRACT_REVISION=0006
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0025 independently accepts the completed
`A004-RFB-FREEZE-REFRESH-R2` Reconstruction baton and advances A004 into the
presentation-owned start/WAIT_FIRST_FRAME/first-frame ownership seam. The next
packet deliberately does not restore H1's direct active-stop behavior because
current Q7 requires retirement to remain a later ordered transition.

Revision 0024 is an execution-policy correction only. It adopts Reconstruction
Contract revision 0006, applies autonomous repository execution to the currently
active A004 P2 packet, and leaves that packet's engineering scope, required
behavior, C1-C9 acceptance criteria, invariants, non-goals, dependency ordering,
and worker role unchanged.

Revision 0023 independently accepts the completed
`A004-MPEG-CALIBRATION-CORE-R1` Reconstruction baton and advances A004 into
the next bounded owner seam: generic RFB freeze/request/full-refresh policy at
the already-proven RFB safe scheduling boundary.

## Current Foreman phase

`A004_P2_INTEGRATED__A004_PRESENTATION_OWNERSHIP_RECONSTRUCTION_ACTIVE__A003_APPLICATION_ORCHESTRATION_DEPENDS_ON_A004_PRESENTATION_SEAMS`

ARCHITECTURE_BLOCKER=NONE
A004_P1_FOREMAN_ACCEPTED=YES
A004_P2_FOREMAN_ACCEPTED=YES
A004_P3_ACTIVE=YES
A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## A004 P1 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe`;
- final pre-log source/dictionary authority
  `094f07fff17801152bcd0d17ac69c570d8bfad27`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T171848-0400__reconstruction__a004-presentation__interactive.md`.

The worker landed five commits after Foreman base `672f67eb...`:

- `ad00a7a07c15c8d56f89e5ac664e07507e50c312` —
  `feat(ui): add MPEG calibration core`;
- `51025be8553c3dad8b5a11002fb57f9bb7a5af62` —
  `docs(symbols): index MPEG calibration core`;
- `3d9f4a821f16d51a3d22597b4a539009aac7705f` —
  `refactor(ui): name MPEG calibration action enum`;
- `094f07fff17801152bcd0d17ac69c570d8bfad27` —
  `docs(symbols): bind calibration actions to named enum`;
- `b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe` —
  the required immutable Reconstruction work log.

Independent source review confirms:

- MPEG CALIBRATION is separate from DESKTOP CALIBRATION;
- `src/ui/mpeg_calibration.{c,h}` owns only local calibration value/state;
- base, inner-content/matte, and outer/suppression meanings remain distinct;
- width/height are bounded and 16-pixel aligned while x/y remain pixel precise;
- geometry calculations use widened arithmetic before bounded int32 storage;
- REVIEW accept remains unarmed while X/accept is held and requires release
  followed by a fresh press;
- successful acceptance copies one immutable caller-visible region and does not
  imply START, producer admission, decode, first presentation, clock arm, or
  MPEG visual ownership;
- cancel publishes no false accepted region and exit remains quarantined until
  all calibration actions are released;
- no RFB, Transport, MPEG decoder, Display, Pi, GS, or Application dependency
  was imported into the calibration owner;
- no new top-level source directory was created.

A004-P1-C1 through A004-P1-C9 are accepted as MET within the packet's bounded
source scope.

## Independent P1 evidence

GitHub Actions run `35539055477` at pickup HEAD
`b566b500c832d9d98bdb2bd4a5ccce7dbd6f05fe` completed SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected after already-coherent dictionaries.

These are machine/repository checks. They do not constitute physical PS2
qualification.

No physical hardware run was performed for A004 P1.

## Forward-authority correction for A004

`docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision 0001 is a
2026-09-15 temporal audit tranche and remains historically truthful. It must not
be destructively rewritten merely because later architecture work refined
retirement semantics.

For forward reconstruction, Q7 in
`docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision 0011 is governing
where it conflicts with the older audit's retirement wording.

Current Q7 requires:

1. MPEG retirement first closes new MPEG production/admission;
2. already accepted MPEG may drain;
3. RFB suppression may then be released so RFB can request, receive and rebuild
   the underlying desktop while valid retiring MPEG/mattes remain visible;
4. final safe MPEG retirement removes MPEG/mattes and reveals the already
   refreshing RFB desktop.

Therefore **active or retiring MPEG is not equivalent to a global RFB freeze**.
The global freeze reconstructed by P2 is for ownership gaps such as calibration
foreground and WAIT_FIRST_FRAME-style protection. Later presentation ownership
will use region-local suppression/composition while RFB framebuffer truth may
continue to advance.

## Why P2 is RFB flow protection before presentation ownership

Current clean RFB already owns:

- exact request serialization through
  `pstvnc_rfb_session_request_update()`;
- complete server-message parsing;
- a nonblocking live service boundary that returns IDLE only before consuming
  any byte of the next server message;
- parser-safe finite-session quiesce at that same complete-message boundary.

Current Application still drives a simple one-request/one-response cadence and
has no MPEG lifecycle.

Frozen H1 evidence separately recovered:

- a calibration RFB gate that blocks new requests and remote presentation while
  frozen without destroying the RFB/PSTV session;
- one outstanding framebuffer request at a time;
- an in-flight pre-freeze response still completing protocol ownership while
  visual publication remains suppressed;
- one coalesced FULL/nonincremental request after thaw before normal
  incremental cadence resumes.

Those facts form a small RFB-owned policy seam that can be reconstructed before
the later presentation owner exists. This avoids putting MPEG/calibration types
inside RFB and avoids inventing temporary Application callbacks.

## A004 P2 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `773cdd905cde5a0c38a1e732c10d4d5f2d123c46`;
- final pre-log source/dictionary authority
  `5aa84f1dc0e3017b88214e20637ab1a0605cc38a`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T175245-0400__reconstruction__a004-presentation__interactive.md`.

The worker landed five commits after Foreman authority `aaa452899...`:

- `7e779fdb113ee37dfda1e60f4887c7d7c608037a` —
  generic RFB freeze/refresh flow policy plus focused host contract;
- `7600d77f158892d1b850c686371cb3c9e72d67b1` —
  tree-identical deterministic dictionary-reconciliation trigger;
- `a64b6427356ff1c2fcbc87f2e40cd126048b7589` —
  automated clean-symbol reconciliation;
- `5aa84f1dc0e3017b88214e20637ab1a0605cc38a` —
  RFB domain responsibility prose;
- `773cdd905cde5a0c38a1e732c10d4d5f2d123c46` —
  required immutable Reconstruction work log.

Independent source review confirms:

- `src/rfb/flow_policy.{c,h}` contains only generic RFB live-flow facts:
  global frozen/thawed state, one outstanding request, one pending FULL debt,
  request choice and publication permission;
- no MPEG, calibration, decoder, producer, GS, presentation-owner, Transport
  lifecycle or Application type/header is imported;
- `next_request()` is non-mutating, so merely inspecting policy cannot consume
  FULL debt;
- FULL debt is cleared only after the currently owed FULL request is recorded as
  successfully sent;
- freeze while one request is outstanding does not discard protocol ownership;
  the response may complete at the existing RFB complete-message boundary and
  clear that outstanding fact while visual publication remains blocked;
- repeated freeze/thaw intent before recovery advances coalesces, while a later
  genuine interval after recovery send creates distinct debt;
- existing strict initial-frame authority and parser/session source remain
  unchanged;
- current Q7 remains possible because active/retiring MPEG is not represented
  inside the global RFB freeze policy.

A004-P2-C1 through A004-P2-C9 are accepted as MET within the packet's bounded
source scope.

GitHub Actions run `35540415181` at pickup HEAD completed SUCCESS.
Host evidence includes both `RFB_FLOW_POLICY_TEST=PASS` and
`transport_runtime_test: PASS`; project-check, strict dictionary, PS2 compile
and PS2 link/current-source reproducibility also passed.

The earlier behavior-head run retained another occurrence of the already-seen
timing-sensitive `transport_runtime_test` assertions at lines 787/789. No
Transport source changed in P2, the starting authority was green, later runs
passed unchanged source, and the final coherent head is green. This remains
test/infrastructure debt rather than an A004 P2 product defect.

No physical PS2 run was performed and P2 is not yet wired into product
Application runtime. Those facts remain explicit evidence boundaries.

## Why P3 is presentation ownership state before compositor/runtime wiring

Current clean authority now has:

- P1: one accepted immutable MPEG CALIBRATION region value;
- P2: one generic RFB freeze/request/full-refresh policy;
- A003: MPEG decoder ownership plus the settled rule that Presentation owns the
  first-real-presentation media-clock arm boundary;
- Platform: one existing PS2 graphics path whose synchronized flip is already
  the application-visible physical completion boundary.

Frozen H1 `h1_mpeg_presentation_owner` and `h1_mpeg_start_handoff` recover a
useful start-side ownership model: RFB_ONLY -> WAIT_FIRST_FRAME -> MPEG_OWNED,
with immutable region/suppression facts and stale-generation rejection.

Two H1 details must not be copied literally:

1. H1 minted presentation generations internally; current architecture treats
   MPEG run/generation identity as caller-owned exact-run authority. Presentation
   consumes that identity and must not create a competing generation namespace.
2. H1's active `stop()` returned directly from MPEG_OWNED to RFB_ONLY. Current
   Q7 supersedes that behavior: active MPEG retirement requires a later ordered
   retirement phase where RFB may rebuild underneath still-visible retiring
   MPEG before final reveal.

P3 therefore reconstructs only the start-side visible-ownership contract and
reserves actual physical composition/clock-arm and Q7 retirement for later
tranches.

## Active bounded Reconstruction packet

PACKET_ID=`A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3`
PACKET_STATUS=ACTIVE
ROLE_KEY=`reconstruction`
WORK_ITEM_KEY=`a004-presentation`
WORKER_KEY=`interactive`
EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`
USER_TERMINAL_POLICY=`EXCEPTION_ONLY`
PI_LOCAL_USER_PROXY_REQUIRED=`NO`
EXECUTION_SEAT=`/home/ps2/src/PS-to-VNC-ledge-manual`
EXECUTION_SEAT_USE=`OPTIONAL_LOCAL_SURFACE_ONLY_WHEN_EXPLICITLY_REQUIRED`
WORKTREE_PREFLIGHT_REQUIRED=`CONDITIONAL_ON_EXPLICIT_PI_LOCAL_EXECUTION`
ASSIGNING_BASE_HEAD=`REFRESH_CURRENT_LEDGE_HEAD_AT_WAKE`

### Objective

Reconstruct the clean **Presentation-owned MPEG visible-ownership core** for the
start side of A004:

`RFB_ONLY -> WAIT_FIRST_FRAME -> MPEG_OWNED`

The owner must consume one already-resolved presentation geometry snapshot and
one caller-supplied nonzero MPEG run/generation identity, preserve those facts
immutably for the run, distinguish WAIT_FIRST_FRAME from real MPEG ownership,
reject stale/wrong generation events, and expose the presentation mode needed by
later Application/RFB/compositor orchestration.

This packet defines the source-level first-physical-frame **promotion contract**.
It does not yet implement the physical MPEG compositor/GS draw itself and does
not arm the media clock. Those actions remain the next presentation-mechanism
tranche so the clock arm can be attached to the actual synchronized physical
presentation boundary rather than to a synthetic state transition.

### Execution policy

This is ordinary autonomous source reconstruction. Perform repository/history
archaeology, source/test/docs changes, commits/ref updates, dictionary work and
CI inspection through available GitHub/GitHub Actions authority.

No live-Pi observation or physical PS2 action is required by this packet.
Do not request user terminal proxy work.

### Required authority / forensic trace

Before behavior-bearing writes, read current:

- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` rev 0006;
- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` rev 0001;
- `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` rev 0001, especially the
  first-real-presentation clock-arm contract;
- `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` rev 0011, especially Q6/Q7;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` rev 0004;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/development/source-topology.md`;
- current `src/display/`, `src/platform/ps2_graphics.{c,h}`,
  `src/ui/mpeg_calibration.{c,h}`, `src/rfb/flow_policy.{c,h}`,
  `src/media/clock.{c,h}`, and `src/mpeg/decoder.{c,h}`.

Trace frozen H1 commit
`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, at minimum:

- `h1_mpeg_presentation_owner.{c,h}` and host test;
- `h1_mpeg_start_handoff.{c,h}` and host test;
- calibration geometry helpers only far enough to recover base/inner/suppression
  relationships;
- any directly referenced first-frame ownership fixture needed to distinguish
  state promotion from physical composition.

Historical H1 generation minting and direct active-stop behavior are evidence,
not current product authority.

### Required behavior

1. **Presentation owns visible-state facts.** Place the new owner in the existing
   Display/Presentation responsibility family. It must not live in UI, RFB,
   MPEG decoder, Transport, Platform or Application.
2. **Neutral resolved geometry.** Presentation consumes a caller-supplied
   resolved geometry snapshot that keeps the exact MPEG base rectangle,
   presentation-local inner matte/content meaning, and outer/suppression
   footprint distinct. Do not make Display depend on UI/calibration internals
   merely to obtain these values, and do not create a second calibration store.
3. **Caller-owned generation.** Arming requires one nonzero caller-supplied MPEG
   run/generation identity. Presentation copies and fences that identity; it
   does **not** mint, increment or redefine MPEG generation identity.
4. **Arm only from RFB_ONLY.** A valid arm from ordinary RFB presentation enters
   WAIT_FIRST_FRAME and snapshots geometry/generation immutably. Nested/repeated
   arm attempts fail closed without replacing live state.
5. **WAIT_FIRST_FRAME is not MPEG ownership.** During WAIT_FIRST_FRAME:
   - MPEG is not yet the visible owner;
   - the last already-presented RFB desktop is the remote presentation mode;
   - a generic global RFB protection requirement is exposed for later
     Application coordination with P2;
   - START, producer admission, decode completion, upload readiness or accepted
     calibration alone cannot promote ownership.
6. **Exact first-frame promotion.** Only the explicit exact-generation
   first-physical-frame-presented event may transition WAIT_FIRST_FRAME to
   MPEG_OWNED. Wrong, zero, stale or duplicate generation events fail closed.
   Repository/host tests exercise the state contract; they are not physical
   evidence that a frame actually reached the television.
7. **MPEG_OWNED is composited, not globally frozen.** After promotion:
   - the presentation mode is COMPOSITED;
   - global RFB freeze is no longer required;
   - authoritative RFB framebuffer truth may continue advancing;
   - the exact generation's suppression footprint remains owned by Presentation
     for the later compositor.
8. **Pending-start abort only.** An exact-generation abort while
   WAIT_FIRST_FRAME may return to RFB_ONLY and release that pending snapshot.
   Stale/wrong aborts fail closed. Do not create false MPEG ownership or own RFB
   FULL-refresh debt; later Application orchestration will thaw P2.
9. **Current-Q7 retirement guard.** Do **not** reconstruct H1's direct
   MPEG_OWNED -> RFB_ONLY `stop()` semantics in this packet. Do not remove the
   active generation/suppression snapshot merely because stop was requested.
   Active retirement/overlapped RFB restoration/final reveal remain a later Q7
   tranche.
10. **No premature physical/clock claim.** Do not call
    `pstvnc_ps2_graphics_present()`, `pstvnc_media_clock_arm()`, or MPEG
    decode/Transport APIs from this owner. The public contract must make clear
    that the first-frame promotion event is to be invoked by the later
    presentation mechanism only at the real synchronized physical boundary.
11. **No cross-domain orchestration.** Do not wire calibration acceptance,
    P2 freeze/thaw, MPEG START, decoder worker startup, Pi producer state,
    pointer/mouse handling or Application transaction sequencing here.
12. **Fail closed and remain inspectable.** Null/invalid geometry, zero
    generation, invalid state transition, stale event or geometry inconsistency
    must not mutate live owner state. Read-only queries must expose enough
    state/geometry/mode for later clean orchestration without leaking mutable
    internals.

### Placement / topology constraint

Prefer the existing `src/display/` domain. Do not create a new top-level
`src/presentation` directory merely to mirror historical naming.

The worker may choose the smallest coherent file representation after inspecting
current Display ownership. Any new clean source must receive synopsis,
`src/display/SYMBOLS.md` coverage, canonical host-test registration and
generated dictionary reconciliation.

Avoid a `src/display -> src/ui` dependency. If presentation needs a value type,
define it with the owner whose meaning it represents or use an already-neutral
type justified by current architecture.

### Acceptance criteria

- `A004-P3-C1 OWNER_BOUNDARY`: visible MPEG ownership state resides with
  Display/Presentation and introduces no forbidden domain dependency.
- `A004-P3-C2 RESOLVED_GEOMETRY`: one immutable start snapshot preserves
  base, inner-matte/content and suppression meanings without duplicating the
  calibration owner.
- `A004-P3-C3 GENERATION_FENCE`: generation is caller-supplied/nonzero and
  stale/wrong/duplicate events cannot seize or replace ownership.
- `A004-P3-C4 WAIT_FIRST_FRAME`: arm enters a genuine protected gap that is
  explicitly not MPEG visual ownership and reports frozen-desktop/global
  protection semantics.
- `A004-P3-C5 FIRST_FRAME_PROMOTION`: only the exact first-frame-presented
  event promotes WAIT_FIRST_FRAME -> MPEG_OWNED exactly once.
- `A004-P3-C6 COMPOSITED_MODE`: MPEG_OWNED reports composited presentation,
  releases the global-freeze requirement, and retains exact suppression facts
  while permitting RFB truth to advance underneath.
- `A004-P3-C7 ABORT_PENDING`: exact pending-start abort returns safely to
  RFB_ONLY; stale aborts fail closed and no RFB debt is duplicated.
- `A004-P3-C8 Q7_RETIREMENT_GUARD`: there is no H1-style direct active stop
  that reveals RFB or destroys active suppression; current-Q7 retirement
  remains attachable as a later state/process.
- `A004-P3-C9 CLOCK_PHYSICAL_BOUNDARY`: no START/decode/upload/state-only path
  arms the media clock or claims hardware presentation; the later physical
  compositor retains the real first-frame/clock-arm callsite.
- `A004-P3-C10 CLEAN_INTEGRATION`: focused tests, full host suite, canonical
  project check and strict dictionary audit pass; broad PS2 build evidence is
  classified accurately and no hardware qualification is claimed.

### Explicit non-goals

Do not implement in this packet:

- physical MPEG GS texture upload/draw/compositor mechanics;
- shared desktop -> suppression/matte -> MPEG -> inner matte -> local overlay
  render plan;
- media-clock arm callsite or timer observation;
- absolute video scheduler/drop;
- active MPEG retirement or current-Q7 overlapped restoration/final reveal;
- calibration controller-entry gesture/timing;
- calibration-to-Presentation/Application runtime wiring;
- P2 RFB flow-policy runtime wiring;
- Transport START/RETIRE or Pi producer orchestration;
- MPEG decoder worker integration;
- final A003/A004 Application transaction;
- A005 general input reconstruction;
- physical hardware qualification.

### Worker return

Return:

- exact current/frozen forensic files inspected and behavior recovered;
- exact source/test/docs files and commits changed;
- placement and value-type ownership decision;
- A004-P3-C1 through C10 disposition;
- focused/full checks and exact results;
- explicit generation-ownership rationale;
- explicit confirmation that H1 direct-stop semantics were **not** restored;
- ambiguity/evidence boundaries preserved;
- execution classification and any genuinely unavailable evidence;
- exact next dependency/baton point.

Emit exactly one immutable Reconstruction log using:

- ROLE_KEY=`reconstruction`
- WORK_ITEM_KEY=`a004-presentation`
- WORKER_KEY=`interactive`

Do not begin the physical compositor/clock-arm tranche in the same shift.

## Deferred dependency graph

After P2 returns, Foreman must choose the next owner seam from actual source.

Expected remaining A004 work after active P3 is:

1. physical shared compositor plus first synchronized MPEG presentation and the
   Presentation-owned media-clock arm callsite;
2. generation/run-scoped suppression/matte layering and local-overlay ordering
   as part of that one physical presentation path;
3. absolute scheduler/drop plus current-Q7 retirement/overlapped RFB
   restoration/final visible handoff;
4. final Application orchestration consuming A003 + A004 public seams.

This is planning only, not worker authority to pre-implement later tranches.

## Hardware qualification debt

HARDWARE_PENDING=A004 visual geometry/matte/suppression/first-frame qualification; MPEG one-run/repeated-run stale fencing and Wire-loss behavior; current-Q7 overlapped RFB restoration; R1 product runtime where later required; final product Q4 establishment; exact product Wire service/runtime; all-guns endurance; exact product ELF; native Pi RFB reproducibility/lifecycle qualification

## Foreman next pickup

Consume the `A004-PRESENTATION-OWNERSHIP-FIRST-FRAME-R3` Reconstruction
baton, independently inspect source/tests/generation semantics and confirm that
current Q7 remains attachable without an H1-style direct active stop. Then
decide the next bounded physical compositor/clock-arm seam.

Do not execute the packet from the Foreman seat.
