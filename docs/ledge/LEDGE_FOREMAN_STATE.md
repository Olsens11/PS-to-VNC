# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0023
RECORDED_AT=2026-09-20T17:38:46-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0022
SUPERSEDES_FOREMAN_STATE_REVISION=0022
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0006
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0023 independently accepts the completed
`A004-MPEG-CALIBRATION-CORE-R1` Reconstruction baton and advances A004 into
the next bounded owner seam: generic RFB freeze/request/full-refresh policy at
the already-proven RFB safe scheduling boundary.

## Current Foreman phase

`A004_P1_INTEGRATED__A004_RFB_FREEZE_REFRESH_RECONSTRUCTION_ACTIVE__A003_APPLICATION_ORCHESTRATION_DEPENDS_ON_A004_PRESENTATION_SEAMS`

ARCHITECTURE_BLOCKER=NONE
A004_P1_FOREMAN_ACCEPTED=YES
A004_P2_ACTIVE=YES
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

## Active bounded Reconstruction packet

PACKET_ID=`A004-RFB-FREEZE-REFRESH-R2`
PACKET_STATUS=ACTIVE
ROLE_KEY=`reconstruction`
WORK_ITEM_KEY=`a004-presentation`
WORKER_KEY=`interactive`
EXECUTION_MODE=`NORMAL_RECONSTRUCTION`
EXECUTION_SEAT=`/home/ps2/src/PS-to-VNC-ledge-manual`
WORKTREE_PREFLIGHT_REQUIRED=YES
ASSIGNING_BASE_HEAD=`REFRESH_CURRENT_LEDGE_HEAD_AT_WAKE`

### Objective

Reconstruct the clean **generic RFB freeze/request/full-refresh policy seam**
needed by later A004 ownership transitions.

This tranche owns only RFB flow-policy facts: whether new framebuffer requests
may be issued, whether completed RFB updates may be visually published, whether
one request is already outstanding, and whether one post-thaw FULL request is
owed.

Do not wire MPEG CALIBRATION, MPEG start/stop, first-frame promotion,
presentation suppression/composition, or final Application orchestration in
this packet.

### Required authority / forensic trace

Before behavior-bearing writes, read current:

- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` rev 0001;
- `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` rev 0011, especially current
  Q7;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` rev 0004;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/development/source-topology.md`;
- `src/rfb/rfb_session.{c,h}`;
- `src/rfb/bridge.{c,h}`;
- current RFB host tests covering async framing, initial-frame authority and
  request serialization.

Trace frozen H1 commit
`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, at minimum:

- `h1_mpeg_calibration_rfb_gate.{c,h}` and test;
- `h1_mpeg_calibration_rfb_schedule.{c,h}` and test;
- `h1_mpeg_calibration_rfb_flow.{c,h}` and test;
- `h1_mpeg_presentation_owner.{c,h}` and test only far enough to distinguish
  WAIT_FIRST_FRAME global protection from later MPEG-owned composited
  presentation.

Historical adapters are evidence, not mandatory clean structure.

### Required behavior

1. **Generic RFB vocabulary.** The new RFB policy must not include MPEG,
   calibration, decoder, producer, GS, or presentation-owner types/headers.
   Callers supply generic freeze/thaw intent; RFB remains ignorant of why.
2. **Freeze without session destruction.** While globally frozen:
   - issue no new framebuffer-update request;
   - disallow publication of newly completed remote RFB visuals;
   - keep the RFB logical session and Transport/Wire session alive.
3. **In-flight request completion.** Freeze may begin while one framebuffer
   request is already outstanding. Its response remains protocol-owned and must
   be consumed to a complete RFB server-message boundary. Completion clears the
   outstanding-request fact even though visual publication is suppressed.
4. **One live request outstanding.** After the initial authoritative desktop is
   established, ordinary live cadence must never manufacture duplicate
   framebuffer requests while one response is still outstanding.
5. **Exactly one post-thaw FULL obligation.** A real frozen->thawed ownership
   interval creates one nonincremental/full-desktop request obligation before
   normal incremental cadence resumes.
6. **Obligation survives blocking conditions.** The pending FULL obligation is
   not lost merely because an older request is still outstanding, because the
   system is still frozen, or because policy is inspected repeatedly. Repeated
   observations of the same thaw do not multiply the obligation.
7. **Distinct later intervals remain distinct.** If another genuine freeze/thaw
   interval occurs after prior recovery work, it may create its own one-shot
   FULL obligation; coalescing must not erase a later ownership transition.
8. **Safe-boundary authority remains singular.** Request-policy decisions are
   made only at the existing proven complete-server-message/request scheduling
   boundary. Do not create a second RFB parser boundary, mid-message benign
   interruption, or MPEG-specific quiesce protocol.
9. **Fail closed.** Invalid state transitions or request-accounting misuse must
   not silently clear an outstanding-request fact or consume a required FULL
   recovery.
10. **Current-Q7 compatibility.** The policy must be capable of protecting a
    calibration/WAIT_FIRST_FRAME-style global ownership gap, but it must not
    encode "MPEG active" or "MPEG retiring" as globally frozen. Later, once first
    physical MPEG presentation promotes ownership, RFB may refresh underneath
    presentation-local suppression; during current-Q7 retirement, RFB may also
    refresh underneath still-visible retiring MPEG.
11. **No premature cross-domain integration.** Do not add MPEG CALIBRATION
    entry gesture/timing, pointer neutralization/rebase, mouse suspension,
    Transport START/RETIRE, presentation owner, compositor, media-clock arm,
    scheduler/drop, or final Application transaction here.
12. **Initial-frame authority remains intact.** Do not weaken the current strict
    initial nonincremental full-frame proof. P2 concerns READY/live request
    cadence after that authoritative baseline unless exact current source
    evidence requires a narrower integration adjustment.

### Placement / topology constraint

Prefer the existing `src/rfb/` domain because this tranche is generic RFB
request/publish flow policy.

Use the smallest owner-correct representation. It may be a small pure policy
module, a narrowly integrated RFB-session extension, or a combination justified
by current ownership, but it must not move cross-domain Application policy into
RFB merely to make tests convenient.

Do not create a new top-level source directory.

Any new clean source file must receive synopsis, local `src/rfb/SYMBOLS.md`
coverage, build/test registration, and generated portal reconciliation.

### Acceptance criteria

- `A004-P2-C1 GENERIC_RFB_POLICY`: no MPEG/calibration/presentation-specific
  dependency or vocabulary is required by the RFB policy.
- `A004-P2-C2 SAFE_BOUNDARY`: freeze/request decisions preserve the existing
  complete-server-message boundary and do not create mid-message interruption.
- `A004-P2-C3 ONE_OUTSTANDING`: live request accounting deterministically
  prevents duplicate outstanding framebuffer requests.
- `A004-P2-C4 INFLIGHT_FREEZE`: a pre-freeze outstanding response can
  complete into RFB framebuffer truth while remote visual publication remains
  blocked and the request fact is cleared exactly once.
- `A004-P2-C5 FULL_REFRESH`: thaw creates exactly one FULL/nonincremental
  recovery before incremental cadence resumes.
- `A004-P2-C6 COALESCED_OBLIGATION`: pending FULL recovery survives old
  outstanding work/freeze/repeated inspection and duplicate restoration intent
  collapses without erasing a later genuine interval.
- `A004-P2-C7 Q7_COMPATIBILITY`: no state model equates active/retiring MPEG
  with global RFB freeze; current Q7 overlapped restoration remains possible.
- `A004-P2-C8 CLEAN_INTEGRATION`: source synopses, RFB dictionary, tests,
  build registration and generated portal are coherent; focused tests, full
  host suite, canonical project check and strict dictionary audit pass.
- `A004-P2-C9 EVIDENCE_BOUNDARY`: repository/source/build evidence remains
  distinct from runtime/hardware qualification; no full A004 or hardware claim
  is made.

### Explicit non-goals

Do not implement in this packet:

- MPEG CALIBRATION controller entry chord/hold timing;
- calibration-to-RFB/Application wiring;
- pointer neutralization/rebase or mouse suspend/resume orchestration;
- Transport START/RETIRE or Pi producer control;
- presentation ownership or first-physical-frame promotion itself;
- MPEG suppression-rectangle drawing or shared GS composition;
- common media-epoch arm callsite;
- MPEG scheduler/drop policy;
- current-Q7 retirement/visible-handoff implementation;
- final A003/A004 Application orchestration;
- A005 general input reconstruction;
- physical hardware qualification.

### Worker return

Return:

- exact frozen/current forensic files inspected and behavior recovered;
- exact product/test/docs files and commits changed;
- placement/ownership decision;
- A004-P2-C1 through C9 disposition;
- focused/full checks and exact results;
- any ambiguity preserved rather than inferred;
- explicit confirmation that current Q7 was not regressed;
- exact next dependency/baton point.

Emit exactly one immutable Reconstruction log using:

- ROLE_KEY=`reconstruction`
- WORK_ITEM_KEY=`a004-presentation`
- WORKER_KEY=`interactive`

Do not begin the presentation/first-frame tranche in the same shift.

## Deferred dependency graph

After P2 returns, Foreman must choose the next owner seam from actual source.

Expected remaining A004 work is:

1. presentation ownership / WAIT_FIRST_FRAME / first-physical-frame promotion
   consuming P1 accepted geometry and P2 RFB protection;
2. shared compositor plus generation/run-scoped suppression and matte layering;
3. absolute scheduler/drop and current-Q7 retirement/visible handoff;
4. final Application orchestration consuming A003 + A004 public seams.

This is planning only, not worker authority to pre-implement later tranches.

## Hardware qualification debt

HARDWARE_PENDING=A004 visual geometry/matte/suppression/first-frame qualification; MPEG one-run/repeated-run stale fencing and Wire-loss behavior; current-Q7 overlapped RFB restoration; R1 product runtime where later required; final product Q4 establishment; exact product Wire service/runtime; all-guns endurance; exact product ELF; native Pi RFB reproducibility/lifecycle qualification

## Foreman next pickup

Consume the `A004-RFB-FREEZE-REFRESH-R2` Reconstruction baton, independently
inspect source/tests/evidence and current Q7 compatibility, then decide the next
bounded presentation/first-frame owner seam.

Do not execute the packet from the Foreman seat.
