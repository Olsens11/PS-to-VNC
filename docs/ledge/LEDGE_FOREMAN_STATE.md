# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0018
RECORDED_AT=2026-09-17T16:20:34-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0006
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0035
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A003_AUDIT_REVISION=0001
BASED_ON_A004_AUDIT_REVISION=0001
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0003
BASED_ON_A003_MANUAL_COMPLETION_REVISION=0001
SUPERSEDES_FOREMAN_STATE_REVISION=0017
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This revision records the mandatory-preflight block returned by the interactive
A004 Reconstruction worker. It does not declare A003 complete, does not accept
A004 product implementation, and does not weaken the normal-development-worktree
requirement established by revision `0017`.

The branch was re-read immediately before this state write at:

`e327f29a8dffa79594da81e08df87cefe836b05f`

Independent Validation remains separate authority. Cross-machine Pi/PS2 proof
and physical qualification remain pending.

## Current Foreman phase

`A003_TRANSPORT_PI_MANUAL_DEFERRED__A004_MPEG_CALIBRATION_CORE_PAUSED_FOR_NORMAL_WORKTREE`

## A003 remains deliberately deferred

The real Transport/Pi exact-generation implementation and proof work remains
governed by:

`docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md`

revision `0001`, status:

`A003_TRANSPORT_PI_MANUAL_COMPLETION_DEFERRED`

The autonomous A003-P2A production Pi/Transport completion route remains
superseded. Nothing in the A004 execution-path block authorizes reopening it or
manufacturing substitute Transport/Pi mechanics.

Current A003 criterion disposition remains unchanged:

- `P2A-1 START_CONTROL_IDENTITY = MET`
- `P2A-2 START_SEMANTICS = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`
- `P2A-3 RETIRE_WIRE_IDENTITY = MET`
- `P2A-4 PURE_MPEG_DATA = MET`
- `P2A-5 SOLE_TRANSPORT_OWNER = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`
- `P2A-6 OWNERSHIP = PARTIAL / PENDING_MANUAL_TRANSPORT_PI`
- `P2A-7 NO_GENERATION_TAGGING = MET`
- `P2A-8 BOUNDED_SCOPE = MET`

A003 as a whole is **not complete**.

## Binding component / directory / bridge invariant

Architecture overlay revision `0003` remains binding:

> Inside one genuine component/local-cooperation directory, internal
> implementation files may cooperate directly. Across a real component
> boundary, communication must pass through the owning component's defined
> bridge or public process seam.

The settled owner for the first A004 pure core remains:

`src/mpeg_calibration/`

This is a genuine owner boundary. `src/mpeg/` remains decoder/resource/feed
lifetime ownership; `src/display/` remains display/presentation conversion;
DESKTOP CALIBRATION remains a separate historical system.

## A004 R1 Reconstruction wake — mandatory-preflight block

The interactive Reconstruction worker for
`A004-MPEG-CALIBRATION-CORE-R1` captured exact:

`STARTED_AT=2026-09-17T16:16:18-04:00`

The worker independently refreshed remote authority at:

`e327f29a8dffa79594da81e08df87cefe836b05f`

and then correctly stopped during the mandatory execution preflight.

Reported execution facts:

- no normal PS-to-VNC Git worktree existed in the worker environment;
- local `HEAD`, current worktree identity, staged/unstaged/untracked state, and
  local-only commits therefore could not be truthfully inspected;
- the required disposable worktree read/write proof could not be performed;
- no current-authority host command/test could be run because no checked-out
  source tree existed;
- normal Git index/stage/commit capability could not be proved;
- a Git executable existed, but direct Git network access could not resolve
  `github.com`;
- the connected GitHub repository interface could inspect remote authority but
  did not provide the normal development worktree/execution route required by
  the packet;
- no raw Git-object, remote-tree, alternate-role, or lower-level workaround was
  used;
- no archaeology, A004 product source, tests, dictionaries, integration, or
  product commit was attempted;
- the remote branch remained unchanged at the same authority when the worker
  returned the baton.

This is a correct role stop. It is an execution-environment admission failure,
not an A004 design failure and not evidence against the settled
`src/mpeg_calibration/` boundary.

No A004 criterion is promoted from this wake.

## Reconstruction-log accounting for the blocked wake

No canonical Reconstruction work log landed because the same missing normal
worktree prevented ordinary file creation, staging, commit, and publication.

The exact worker `STARTED_AT` is preserved in this Foreman state so it is not
lost or approximated later.

Foreman does **not** impersonate Reconstruction by manufacturing a retroactive
Reconstruction-role log through remote Git-object manipulation. The blocked
wake is reconciled here as Foreman state/history; future Reconstruction work
must create its own canonical shift log through the normal repository workflow
when that workflow is available.

## Blocker classification

Current blocker:

`A004_EXECUTION_ROUTE_BLOCKED__NORMAL_DEVELOPMENT_WORKTREE_UNAVAILABLE`

This blocker precedes the later Foreman-owned canonical new-domain integration
identified in revision `0017`.

The dependency remains:

1. Reconstruction must first run in a normal development worktree and pass the
   execution preflight.
2. Reconstruction then lands coherent owner-local `src/mpeg_calibration/`
   source/API, local `SYMBOLS.md`, focused deterministic host-test source, and
   directly corresponding local test dictionary if required.
3. Reconstruction executes its focused host test, commits coherent worker-owned
   work, writes its own immutable shift log, and returns the baton.
4. Foreman/Integration then performs canonical new-domain admission: topology,
   living map, continuity expected-domain registration, generated dictionary
   portal, canonical build/test/tool registration actually required by landed
   files, and strict/project checks.
5. Only a behavior-owned defect exposed by that integration may justify a
   smaller Reconstruction continuation.

The current execution block does **not** justify an empty Foreman scaffold,
early canonical domain admission, or weakening the test/worktree gate.

## Binding execution-seat admission rule

Do not wake another A004 Reconstruction shift in a connector-only seat that has
no normal repository worktree.

A Reconstruction seat is eligible only if it can, through the normal development
route:

1. see a real PS-to-VNC Git worktree;
2. refresh/compare `ledge/h1-all-guns` authority;
3. report actual local `HEAD` and staged/unstaged/untracked state;
4. identify local-only commits without discarding them;
5. create and remove an ordinary disposable untracked worktree file;
6. run an existing applicable host command/test against checked-out source;
7. use the normal Git index and commit workflow.

If any of these capabilities are absent, the A004 product packet remains paused.
Do not start another archaeology/implementation attempt in that seat.

## Paused bounded Reconstruction packet — A004-MPEG-CALIBRATION-CORE-R2

WORK_ITEM_KEY=`a004-presentation-calibration`
TARGET_WORKER=`interactive`
PACKET_STATUS=`PAUSED_UNTIL_NORMAL_WORKTREE_AVAILABLE`
WAKE_CONDITION=`NORMAL_PS_TO_VNC_DEVELOPMENT_WORKTREE_AND_HOST_EXECUTION_ROUTE_AVAILABLE`
ASSIGNING_HEAD=`e327f29a8dffa79594da81e08df87cefe836b05f`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md:0001`
ASSIGNING_CALIBRATION_INVARIANT=`LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md:0001`
ASSIGNING_ARCHITECTURE_OVERLAY=`LEDGE_ARCHITECTURE_OVERLAY.md:0003`
ASSIGNING_FOREMAN_STATE=`0018`
H1_FORENSIC_SOURCE=`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`
OWNER_DIRECTORY=`src/mpeg_calibration/`
BATON_SEQUENCE=`RECONSTRUCTION_SOURCE_TEST_LOCAL_DICTIONARY__THEN_FOREMAN_CANONICAL_INTEGRATION`

### R2 wake rule

Do **not** launch this packet merely because it exists in Foreman state.

Launch it only in a seat/environment where the normal-worktree capabilities
listed above are available. Once such a seat is actually running, it must
capture a new exact `STARTED_AT` immediately and rerun the full mandatory
preflight before archaeology or editing.

The blocked R1 worker's timestamp is historical evidence only; it must not be
reused as R2's shift start.

### Objective after admission

Reconstruct only the pure owner-local MPEG CALIBRATION geometry and
`default/draft/committed` state core into `src/mpeg_calibration/`, expose a
narrow public owner seam, and prove it with deterministic host tests.

This packet remains independent of deferred real Pi/Transport work.

### Required authority after successful preflight

Read at minimum:

- `AGENTS.md`
- `CONTRIBUTING.md`
- `docs/CLEAN_ARCHITECTURE.md`
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md`
- `docs/ledge/work-log/README.md`
- this Foreman state revision
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`
- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md`
- `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md`
- current `docs/development/source-topology.md`
- current source naming/symbol and test conventions
- frozen H1 MPEG-calibration header/geometry/state source and focused tests at
  `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

Do not use DESKTOP CALIBRATION source as MPEG-calibration mechanics authority.

### Authorized worker-owned changes after admission

Reconstruction may:

- create behavior-owned clean source/API under `src/mpeg_calibration/`;
- create/update `src/mpeg_calibration/SYMBOLS.md`;
- add focused deterministic host-test source under current test conventions;
- update the directly corresponding local test dictionary when current
  conventions require it;
- execute direct focused host compile/run commands for the isolated core;
- commit coherent worker-owned work and its canonical Reconstruction log.

Reconstruction may **not** update:

- `docs/development/source-topology.md`;
- the living file/service map;
- `scripts/continuity-check.sh` expected-domain registration;
- the generated product dictionary portal;
- canonical/global build registration;
- canonical/global test registration;
- other generated/global integration metadata.

Those remain the next Foreman baton point.

### Required mechanics

Preserve the already-verified frozen-H1 meanings:

- base = exact MPEG capture/presentation rectangle;
- inner matte = PS2-local presentation-only inset;
- outer matte = MPEG visual-ownership / RFB-suppression expansion;
- centered half-canvas default with 16-pixel base sizing alignment;
- 16-pixel base width/height minimum;
- center-preserving 16-pixel resize;
- pixel-precise movement with canvas clamp;
- independently bounded inner matte;
- suppression expansion from the same authoritative geometry clipped to canvas;
- one owner-local default/draft/committed state model;
- commit = one accepted immutable geometry authority;
- later draft edits cannot mutate the accepted committed value;
- cancel = committed geometry when present, otherwise defaults, without
  fabricating acceptance.

Historical deterministic 704x462 vector remains:

- default `(176,119,352,224)`;
- horizontal `+16` resize -> `x=168 width=368`;
- subsequent `+1,+1` move -> `x=169 y=120`;
- inner `1,1`;
- outer `1,1`;
- suppression `(168,119,370,226)`.

### Acceptance criteria

- `A004-C1 MPEG_CALIBRATION_OWNER_BOUNDARY = PENDING_RECONSTRUCTION_SOURCE`
- `A004-C2 GEOMETRY_MEANINGS = PENDING_RECONSTRUCTION_SOURCE_TEST`
- `A004-C3 SINGLE_COMMITTED_AUTHORITY = PENDING_RECONSTRUCTION_SOURCE_TEST`
- `A004-C4 DESKTOP_SEPARATION = PENDING_RECONSTRUCTION_SOURCE_TEST`
- `A004-C5 CONTRACT_HOST_TESTED = PENDING_RECONSTRUCTION_HOST_TEST`
- `A004-C6 BOUNDED_SCOPE = PENDING_RECONSTRUCTION_HANDOFF`

No criterion is accepted `MET` from R1.

### Explicitly out of scope

Do not implement controller acquisition, held-X review acceptance, release
quarantine, A005 input routing, RFB safe-freeze/full-refresh scheduling,
accept-to-first-frame orchestration, START/RETIRE sends, real Pi/Transport or
exact-generation mechanics, MPEG decoder/worker changes, Presentation/GS
compositor/raster/overlay work, first-physical-frame promotion,
common-media-clock arm, scheduler/drop policy, DESKTOP CALIBRATION changes, or
hardware qualification.

## Foreman next pickup

There is no product-integration work for Foreman yet.

First obtain a Reconstruction seat with the normal worktree/execution route and
run R2. If coherent worker source/tests/local dictionary and focused host
evidence land, Foreman then performs the canonical new-domain integration from
revision `0017` and dispositions A004-C1 through A004-C6 from repository
evidence.

Do not reopen autonomous A003 Transport/Pi work. Do not advance to A005/A006.

PENDING_EXECUTION_ROUTE=A004 normal PS-to-VNC development worktree; local-state inspection; host execution; normal Git index/commit path
PENDING_MANUAL_TRANSPORT=A003 real Pi PSTV owner; exact START semantic/prepared-generation implementation; producer/emission/retirement fence; PS2 exact completion/decoder join/drain-credit; repeated-generation same-stream proof
PENDING_CROSS_MACHINE=A003 real PS2/Pi generation transaction; downstream all-guns owner integration
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification; A004-A006 later physical/all-guns qualification
