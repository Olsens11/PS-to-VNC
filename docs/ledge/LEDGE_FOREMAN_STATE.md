# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0022
RECORDED_AT=2026-09-20T16:46:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0021
SUPERSEDES_FOREMAN_STATE_REVISION=0021
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0006
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision 0022 independently accepts the completed
`A003-WIRE-SESSION-FOUNDATION-R1` Reconstruction baton and advances the
dependency graph into the first bounded A004 product tranche.

## Current Foreman phase

`A003_R1_INTEGRATED__A004_MPEG_CALIBRATION_CORE_RECONSTRUCTION_ACTIVE__A003_APPLICATION_ORCHESTRATION_DEPENDS_ON_A004_OWNER_SEAMS`

ARCHITECTURE_BLOCKER=NONE
R1_FOREMAN_ACCEPTED=YES
A004_MAY_ADVANCE=YES
HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## R1 Foreman acceptance

Live pickup authority was independently refreshed as:

- branch HEAD `921dc29dd67baadc07c8f648aa4fdb46954b2167`;
- final source/dictionary authority `170e2198d41b3d703a5fe86bd5c329f2248d031a`;
- immutable Reconstruction log
  `docs/ledge/work-log/20260920T160357-0400__reconstruction__a003-mpeg-generation__interactive.md`.

The worker's three substantive commits are coherent with the issued packet:

- `da9053b1488ead933dde1408cdab875885ffa37c` — single Transport physical-I/O owner plus cooperative idle scheduling;
- `18025d9278940cec7cf0d8513f99e0a93aeefbe3` — opaque Wire-session-bound Transport access across RFB/audio/MPEG;
- `170e2198d41b3d703a5fe86bd5c329f2248d031a` — final symbol/dictionary reconciliation.

The rejected generic Transport drain and divergent resume-branch Q12 candidate
remain absent. The governing Q12 split remains:

    Transport -> cross-session communication validity
    module    -> complete local retirement

R1-C1 through R1-C8 are accepted as MET within the packet's bounded source
scope.

## Independent R1 evidence

GitHub Actions run `35536021547` for pickup HEAD `921dc29d...` completed
SUCCESS:

- canonical host unit suite — PASS;
- canonical project check — PASS;
- complete strict dictionary audit — PASS;
- current clean-core PS2 compile job — PASS;
- current-source PS2 link/reproducibility job — PASS.

These CI results are additional Foreman-observed machine evidence. They do not
change the worker's appropriately narrow evidence vocabulary and do not create
a new HARDWARE_PROVEN claim.

No physical hardware run was performed for R1.

## Dependency decision after R1

A003 is **not complete**.

Current `src/app.c` still opens the ordinary RFB-only Transport session through
`pstvnc_transport_session_open()`, does not construct an MPEG-enabled session,
does not own MPEG activation/retirement, and the default
`pstvnc_app_run()` remains intentionally fail-closed until concrete validated
configuration authority exists.

Those facts are not R1 defects; they are outside the R1 packet.

Do not immediately build a temporary A003 application coordinator around
invented region/presentation callbacks. Q6/Q7 and A004 make the MPEG region,
MPEG CALIBRATION acceptance, RFB/presentation ownership, first physical frame,
and visible retirement handoff real owner boundaries. Those seams should exist
before final Application orchestration consumes them.

Therefore A004 may advance now. A003 application-level MPEG orchestration remains
dependency-queued and will resume after the required A004 owner seams exist.

## Current A004 authority

The active A004 worker must read:

- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision 0001;
- `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` revision 0001;
- `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision 0011, especially Q6/Q7;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision 0004;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/development/source-topology.md`;
- `docs/development/source-naming-and-symbols.md`;
- the frozen H1 MPEG CALIBRATION lineage under
  `experiments/media-harness-h1/mpeg_presentation_calibration/` at
  `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The current Q7 retirement semantics supersede the older A004-audit wording that
would delay all RFB restoration until complete local/Pi MPEG teardown. Later
A004 retirement work must allow RFB refresh underneath once new MPEG production
is closed, while valid retiring MPEG presentation may remain visible until its
safe final boundary.

That Q7 correction is not part of the active P1 source scope below, but workers
must not reintroduce the obsolete ordering while reconstructing shared state.

## Active bounded Reconstruction packet

PACKET_ID=`A004-MPEG-CALIBRATION-CORE-R1`
PACKET_STATUS=ACTIVE
ROLE_KEY=`reconstruction`
WORK_ITEM_KEY=`a004-presentation`
WORKER_KEY=`interactive`
EXECUTION_MODE=`NORMAL_RECONSTRUCTION`
EXECUTION_SEAT=`/home/ps2/src/PS-to-VNC-ledge-manual`
WORKTREE_PREFLIGHT_REQUIRED=YES
ASSIGNING_BASE_HEAD=`REFRESH_CURRENT_LEDGE_HEAD_AT_WAKE`

### Objective

Reconstruct only the clean **MPEG CALIBRATION** owner/model needed to define one
accepted MPEG region value and its foreground acceptance semantics. Do not yet
wire live MPEG activation, RFB freeze/suppression, first-frame promotion, GS
composition, Pi producer behavior, or final Application orchestration.

This packet creates the owner-correct value/state seam that later A004/A003 work
can consume rather than inventing temporary application callbacks.

### Mandatory calibration separation invariant

> **CALIBRATION SEPARATION INVARIANT:** DESKTOP CALIBRATION and MPEG CALIBRATION
> are separate historical systems with separate state, geometry authority,
> ownership, and evidence. Do not derive MPEG-calibration behavior from the older
> desktop-calibration implementation merely because both manipulate screen
> rectangles or use similar UI mechanics. For MPEG calibration, trace
> `experiments/media-harness-h1/mpeg_presentation_calibration/` and A004 audit
> authority. If any relationship is unclear, preserve the separation and flag
> the ambiguity rather than merging the systems.

### Required forensic trace before source writes

At minimum inspect the frozen MPEG CALIBRATION lineage's:

- `README.md`;
- `mpeg_presentation_calibration.h`;
- `mpeg_presentation_calibration_geometry.c`;
- `mpeg_presentation_calibration_state.c`;
- `mpeg_presentation_calibration_test.c`;
- `h1_mpeg_calibration_foreground.{c,h}` and its host test;
- `h1_mpeg_calibration_entry_hold.{c,h}` and its host test;
- `h1_mpeg_calibration_accept_edge_test.c`;
- any directly referenced adapter/coordinator test needed to resolve acceptance
  semantics.

Historical adapters are evidence, not mandatory production structure.

### Required behavior

1. Represent **MPEG CALIBRATION base rectangle**, **inner matte**, and
   **outer/suppression footprint** as distinct meanings. Do not collapse them
   into DESKTOP CALIBRATION or one generic screen-fit rectangle.
2. Preserve MPEG-compatible base dimensions: width/height are positive,
   at least 16 pixels where the recovered product rule requires it, and aligned
   to the 16-pixel MPEG macroblock grid. X/Y placement remains one-pixel
   precision rather than 16-pixel grid placement.
3. Preserve bounded/overflow-safe geometry and the recovered relationships among
   base, inner matte, outer matte and suppression footprint. Do not invent Pi
   desktop bounds from this calibration value.
4. Preserve the MPEG CALIBRATION foreground state needed to edit/review/accept
   or cancel the candidate value without importing RFB, Transport, MPEG decoder,
   Pi producer or GS internals.
5. Preserve the held-X safety rule: entering/reviewing with X already held must
   not immediately accept. Acceptance requires the recovered release/re-press
   edge semantics.
6. One successful accept publishes exactly one immutable committed MPEG
   CALIBRATION region value for the caller. Do not create duplicate accepted-edge
   detectors or a second competing geometry authority merely to cross files.
7. Cancel does not manufacture an accepted MPEG region or claim MPEG ownership.
8. Acceptance means only "region accepted". It does **not** mean START sent,
   producer admitted, decoder started, first physical frame presented, media
   clock armed, or MPEG visual ownership promoted.

### Placement / topology constraint

Use the smallest current clean owner-correct placement.

The worker must inspect the existing `src/ui/` and `src/display/`
responsibilities before choosing file placement. MPEG CALIBRATION foreground/edit
state belongs with the owner that understands those semantics; presentation-only
mechanism belongs to display/presentation, not UI merely for convenience.

Do not create a new top-level source directory unless current responsibilities
cannot express the clean owner boundary. If a new directory is genuinely earned,
perform the complete topology/dictionary/checker integration required by
`source-topology.md` in the same coherent change.

### Acceptance criteria

- `A004-P1-C1 CALIBRATION_SEPARATION`: DESKTOP CALIBRATION remains untouched
  as an independent authority; no MPEG value is sourced from it by inference.
- `A004-P1-C2 MPEG_REGION_VALUE`: base, inner matte and outer/suppression
  meanings are explicit, bounded, and host-testable.
- `A004-P1-C3 GEOMETRY_RULES`: macroblock-aligned base dimensions and
  pixel-precision placement are validated with overflow/bounds cases supported
  by the recovered authority.
- `A004-P1-C4 FOREGROUND_STATE`: MPEG CALIBRATION edit/review/accept/cancel
  state is owner-local and does not depend on RFB/Transport/MPEG/GS internals.
- `A004-P1-C5 ACCEPT_EDGE`: held-X entry cannot auto-accept; release/re-press
  acceptance is deterministic and exactly-once.
- `A004-P1-C6 ACCEPTED_VALUE`: accepted region is one immutable caller-visible
  value; cancel/non-accept paths do not mutate/publish false acceptance.
- `A004-P1-C7 ACCEPT_NOT_OWNERSHIP`: no code path equates calibration accept
  with first physical presentation, clock arm or MPEG visual ownership.
- `A004-P1-C8 CLEAN_INTEGRATION`: source synopses, local symbol dictionaries,
  build/test registration and generated portal are coherent; focused host tests,
  full host suite, project check and strict dictionary audit pass.
- `A004-P1-C9 EVIDENCE_BOUNDARY`: source/host/build evidence is reported
  separately from hardware. No hardware-qualified or full-A004 completion claim
  is made.

### Explicit non-goals

Do not implement in this packet:

- live Application MPEG start/stop orchestration;
- Transport START/RETIRE transaction wiring;
- Pi capture/producer service behavior;
- RFB safe-boundary freeze, suppression or restoration;
- accept-to-first-frame protection outside the calibration owner's own accepted
  value;
- first physically presented frame ownership promotion;
- shared GS compositor integration;
- media clock arm callsite;
- MPEG presentation scheduler/drop policy;
- final retirement/visible-handoff behavior;
- A005 general input reconstruction;
- physical hardware qualification.

### Worker return

Return:

- exact H1 forensic files inspected and the behavior recovered from each;
- exact product/test/docs files changed;
- placement/ownership decision and why it follows current topology;
- A004-P1-C1 through C9 disposition;
- all executed tests/checks and results;
- any ambiguity preserved rather than inferred;
- exact next dependency/baton point.

Emit exactly one immutable Reconstruction log using:

- ROLE_KEY=`reconstruction`
- WORK_ITEM_KEY=`a004-presentation`
- WORKER_KEY=`interactive`

Do not begin the next A004 tranche from the same shift.

## Deferred dependency graph

After A004 P1 returns, Foreman should packetize the next owner seam based on the
actual reconstructed shape. Expected later boundaries include:

1. RFB freeze/full-refresh ownership plus accept-to-first-frame protection;
2. presentation ownership/compositor/first-physical-frame seam;
3. scheduler/drop and retirement-visible handoff under current Q7;
4. final Application orchestration consuming A003 + A004 public seams.

This ordering is a planning dependency, not authority to pre-implement later
steps.

## Hardware qualification debt

HARDWARE_PENDING=R1 physical product qualification where later required; final product Q4 establishment; exact product Wire service/runtime; A004 visual geometry/matte/suppression/first-frame qualification; MPEG one-run/repeated generations/stale N-vs-N+1/Wire-loss; Q7 overlapped RFB restoration; full all-guns endurance; exact product ELF; native Pi RFB reproducibility/lifecycle qualification

## Foreman next pickup

Consume the `A004-MPEG-CALIBRATION-CORE-R1` Reconstruction baton, independently
inspect source/tests/evidence, and decide whether the next coherent A004 owner
seam is RFB freeze/request ownership or presentation/first-frame ownership.
