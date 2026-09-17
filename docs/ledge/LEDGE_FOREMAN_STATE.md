# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0017
RECORDED_AT=2026-09-17T16:09:34-04:00
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
SUPERSEDES_FOREMAN_STATE_REVISION=0016
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This revision is a Foreman recovery/role-boundary revision. It does not declare
A003 complete and it does not accept A004 product implementation. It preserves
the A003 manual Transport/Pi deferral, settles the owner boundary for the first
A004 MPEG CALIBRATION core packet, records the failed replacement-worker wake as
forensic information only, makes the worker/Foreman baton rule explicit, and
reissues exactly one bounded A004 Reconstruction packet with an execution
admission preflight.

The live branch was re-read immediately before this state write at
`9151ba9895d53aeecb17b996a9bf92aa42f69c24`.

Independent Validation remains separate authority. Cross-machine Pi/PS2
validation and physical qualification remain pending.

## Current Foreman phase

`A003_TRANSPORT_PI_MANUAL_DEFERRED__A004_MPEG_CALIBRATION_CORE_REISSUED_WITH_PREFLIGHT`

## A003 remains deliberately deferred

The real Transport/Pi exact-generation implementation and proof work remains
governed by:

`docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md`

revision `0001`, status:

`A003_TRANSPORT_PI_MANUAL_COMPLETION_DEFERRED`

The autonomous A003-P2A production Pi/Transport completion route remains
superseded. Reconstruction must not create substitute production Transport/Pi
mechanics merely to make downstream work appear complete.

Accepted A003 source/evidence remains preserved. The current criterion
disposition remains:

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

Consequences remain:

- directory membership follows coherent ownership, not call-site convenience;
- unrelated responsibilities may not be co-located merely to avoid a bridge;
- no bridge-per-caller or bridge-per-destination proliferation;
- Application coordinates owners only through public owner bridges;
- tests fake the same public owner boundary production callers consume;
- private owner internals remain private.

## Failed A004 replacement-worker wake — administrative accounting

After Foreman state revision `0016`, a replacement interactive Reconstruction
worker attempted the A004 MPEG CALIBRATION core packet and returned a
user-visible `BLOCKED` report.

Repository authority proves:

- no A004 product/source/test commit landed from that attempt;
- no branch update resulted from that attempt;
- no canonical Reconstruction work log landed;
- the worker's exact `STARTED_AT` is unavailable and must not be fabricated;
- no A004 acceptance criterion is accepted `MET` from that attempt;
- scratch reasoning/code from that attempt is not product evidence.

The report is retained only as forensic handoff information. This Foreman shift
independently re-checked its useful mechanical claims against the frozen H1
MPEG-calibration lineage before relying on them.

The missing worker timestamp is an administrative defect in that failed wake,
not a reason to weaken `docs/ledge/work-log/README.md`. No retroactive worker
log is authorized.

## A004 MPEG CALIBRATION owner decision

The first A004 slice is the pure MPEG CALIBRATION geometry and
draft/default/committed-state owner.

The clean owner boundary is settled as:

`src/mpeg_calibration/`

This is a genuine owner boundary, not a convenience directory.

Rationale:

- `src/mpeg/` owns MPEG decoder/resource/feed/sequence lifetime and does not own
  calibration geometry or accepted presentation-region state;
- `src/display/` owns display/presentation conversion and does not own the
  MPEG-calibration draft/default/committed business state;
- DESKTOP CALIBRATION is a separate historical system and is not an authority
  for MPEG CALIBRATION mechanics;
- later Presentation, RFB, Input/UI, and Application consumers cross a real
  owner boundary and therefore must consume the MPEG CALIBRATION public seam
  rather than its internals.

The component may contain owner-local geometry/state implementation files that
cooperate directly. Cross-component consumers receive only the narrow public
MPEG CALIBRATION seam. Precise private file decomposition remains a
Reconstruction implementation choice so long as this ownership is preserved.

## Verified A004 pure-mechanics authority

The Foreman independently checked frozen H1 authority at:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

under:

`experiments/media-harness-h1/mpeg_presentation_calibration/`

The pure MPEG CALIBRATION meanings remain:

- base rectangle = exact MPEG capture/presentation region;
- inner matte = PS2-local presentation-only inset;
- outer matte = MPEG visual-ownership / RFB-suppression expansion;
- default = centered half-canvas with 16-pixel size alignment;
- base width/height minimum = 16 pixels;
- resize = 16-pixel size steps with recovered center-preserving behavior;
- movement = pixel precise with canvas clamp;
- inner matte = bounded independently of capture geometry;
- outer suppression = expanded from the authoritative base and clipped to the
  canvas;
- state = explicit default/draft/committed authority;
- cancel = committed geometry when present, otherwise defaults;
- commit = one accepted immutable geometry authority.

The deterministic historical 704x462 vector is:

- default base: `(176,119,352,224)`
- horizontal `+16` resize: `x=168 width=368`
- then `+1,+1` move: `x=169 y=120`
- inner matte: `1,1`
- outer matte: `1,1`
- derived suppression: `(168,119,370,226)`

These are reconstruction mechanics evidence only; they do not constitute a
landed A004 implementation.

## Exact Foreman-owned blocker and required baton sequence

The blocker was not missing product semantics.

Creating `src/mpeg_calibration/` is a new clean-domain topology change. Current
`docs/development/source-topology.md` requires the same logical adoption to
reconcile the topology policy, living file/service map, local dictionary,
`scripts/continuity-check.sh`, build/include/test/tool paths, architecture
navigation, chronological record, strict dictionary discovery, and generated
dictionary portal.

State revision `0016` already reserved canonical topology, generated portal,
canonical test registration, and build integration to Foreman/Integration rather
than Reconstruction.

The current continuity checker also compares the exact clean directory set,
directory-dictionary set, and generated-portal directory set. Therefore a
Foreman-created empty `src/mpeg_calibration/` scaffold would not be a truthful
or passing prerequisite: Git does not preserve an empty directory, and adding
global expected-directory state before real clean source/local dictionary
exists would deliberately make the canonical topology check fail.

The required sequence is therefore **A**:

1. Reconstruction, in a normal development worktree, lands a coherent
   owner-local `src/mpeg_calibration/` behavior source/API, its local
   `SYMBOLS.md`, and focused deterministic host test source.
2. Reconstruction runs the focused host test directly using the established
   host toolchain if no canonical aggregate registration exists yet.
3. Reconstruction commits coherent worker-owned work and returns the baton
   immediately when canonical topology/build/test/generated-dictionary
   integration is required.
4. Foreman/Integration then performs the canonical new-domain admission:
   topology/map/continuity registration, generated portal reconciliation,
   canonical build/test/tool registration actually required by the landed
   files, and project checks.
5. Only if that integration exposes a behavior-owned defect does Foreman issue
   a smaller Reconstruction continuation.

This sequencing follows the already-used A003 MPEG-domain pattern: behavior
source can land first; canonical owner/topology closure follows as its own
role-owned integration action. It does not authorize a long-lived broken
integration state; it defines the controlled baton boundary between the two
roles.

## Binding role-baton rule

For every Reconstruction packet from this revision forward:

If progress requires an action owned by Foreman, Validation, Integration, or
another role, Reconstruction must:

1. preserve/commit coherent worker-owned work already completed when possible;
2. identify the exact blocking action;
3. identify the role that owns it;
4. disposition completed criteria truthfully;
5. write the required canonical worker log when exact log metadata was captured;
6. return the baton immediately.

Reconstruction must **not**:

- find a workaround for the role boundary;
- emulate the Foreman/Validation/Integration action;
- use a lower-level route to manufacture the same integration result;
- broaden its own authority;
- manipulate generated/global integration state indirectly;
- use raw Git-object mutation as a replacement development workflow.

When Foreman receives such a handoff, Foreman must classify the blocker, perform
Foreman-owned work when it is genuinely Foreman-owned, and issue only the
smallest required continuation. A correct role stop is not a reason to redesign
the product or send the worker back to perform Foreman chores.

## Reconstruction execution-environment admission rule

Every newly issued Reconstruction packet must begin with an execution preflight
**before archaeology or implementation**.

The worker must:

1. capture exact `STARTED_AT` immediately;
2. refresh remote branch authority and record actual local `HEAD`;
3. inspect and report staged, unstaged, and untracked state;
4. identify local-only commits, if any, before changing anything;
5. prove normal worktree read/write ability without altering tracked product
   content;
6. run one existing applicable host command/test successfully;
7. prove normal Git staging/commit capability without creating a bogus commit.

If the normal development route is unavailable, the worker must return
`BLOCKED` immediately. It must not spend the shift doing product archaeology,
attempt raw Git-object substitutes, or use a lower-level route to bypass the
missing worktree.

## Active bounded Reconstruction packet — A004-MPEG-CALIBRATION-CORE-R1

WORK_ITEM_KEY=`a004-presentation-calibration`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`9151ba9895d53aeecb17b996a9bf92aa42f69c24`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md:0001`
ASSIGNING_CALIBRATION_INVARIANT=`LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md:0001`
ASSIGNING_ARCHITECTURE_OVERLAY=`LEDGE_ARCHITECTURE_OVERLAY.md:0003`
ASSIGNING_FOREMAN_STATE=`0017`
H1_FORENSIC_SOURCE=`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`
OWNER_DIRECTORY=`src/mpeg_calibration/`
BATON_SEQUENCE=`RECONSTRUCTION_SOURCE_TEST_LOCAL_DICTIONARY__THEN_FOREMAN_CANONICAL_INTEGRATION`

### Objective

Reconstruct only the pure owner-local **MPEG CALIBRATION geometry and
draft/default/committed-state core** into `src/mpeg_calibration/`, with a narrow
public owner seam and deterministic host tests.

This packet is independent of the deferred real Pi/Transport implementation.

### Mandatory execution preflight — do this first

Before archaeology or editing:

1. capture exact `STARTED_AT`;
2. fetch/refresh `ledge/h1-all-guns` and record remote branch authority;
3. record local `HEAD`;
4. report `git status` with staged/unstaged/untracked state explicitly;
5. identify local-only commits relative to refreshed branch authority;
6. prove normal read/write worktree access using a disposable untracked
   temporary file that is created and removed before implementation;
7. run one existing applicable host command/test from current repository
   authority, preferring `./scripts/check.sh` when the local environment can run
   it; if a narrower known-good host command is used, record exactly why;
8. prove normal Git staging/commit capability using a non-mutating/dry-run route
   or equivalent that does not create a bogus commit and does not disturb the
   index.

If any of these cannot be performed through the normal development worktree,
return `BLOCKED` immediately with no product archaeology or implementation.

Do not substitute GitHub object plumbing, raw tree/blob creation, or another
role's integration machinery.

### Required authority after preflight

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
- current source naming/symbol policy
- frozen H1:
  `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration.h`
- frozen H1:
  `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration_geometry.c`
- frozen H1:
  `experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration_state.c`
- focused frozen H1 MPEG-calibration tests and any later H1 evidence that proves
  a correction to the pure geometry/commit semantics.

Do not use DESKTOP CALIBRATION source as MPEG-calibration mechanics authority.

### Authorized worker-owned changes

Reconstruction is authorized to:

- create the behavior-owned clean source/API under `src/mpeg_calibration/`;
- create/update `src/mpeg_calibration/SYMBOLS.md`;
- add the focused deterministic host test source under current test conventions;
- update the directly corresponding local test dictionary when current
  conventions require it;
- use direct focused host compile/run commands needed to prove this isolated
  pure core before canonical aggregate registration exists.

Reconstruction is **not** authorized to update:

- `docs/development/source-topology.md`;
- the living file/service map;
- the expected-domain set in `scripts/continuity-check.sh`;
- the generated product dictionary portal;
- canonical/global build registration;
- canonical/global test registration;
- other generated/global integration metadata.

Those are the baton point. Commit coherent worker-owned work if possible, log
truthfully, identify the exact Foreman-owned follow-up, and stop.

### Required behavior

1. Keep base, inner matte, and outer/suppression geometry semantically distinct.
2. Preserve 16-pixel base size minimum/alignment.
3. Preserve center-aware 16-pixel resize behavior proved by frozen H1 evidence.
4. Preserve pixel-precise movement with canvas clamping.
5. Keep inner matte bounded and presentation-only; it must not alter base
   capture geometry.
6. Derive outer suppression from the same authoritative geometry and clip it to
   the canvas.
7. Maintain one owner-local default/draft/committed model.
8. Commit produces one accepted immutable MPEG CALIBRATION geometry authority.
9. Later draft edits cannot mutate the previously accepted committed value.
10. Cancel restores committed geometry when present, otherwise defaults, without
    fabricating a committed acceptance.
11. Expose only the narrow owner seam needed by later consumers.
12. Do not emit Presentation/GS, RFB, Input, Application, Transport, Pi,
    exact-generation, or DESKTOP CALIBRATION behavior.

### Required host-test evidence

Focused deterministic tests must cover at minimum:

- valid aligned default geometry;
- 16-pixel size minimum/step;
- center-preserving resize;
- pixel move/clamp behavior;
- independent inner matte behavior;
- independent outer matte and clipped suppression behavior;
- default/draft/committed transitions;
- commit immutability;
- cancel-to-committed and cancel-to-default behavior;
- base/inner/outer semantic separation;
- the historical 704x462 deterministic vector:
  default `(176,119,352,224)`, resize to `x=168 width=368`, move to
  `x=169 y=120`, inner `1,1`, outer `1,1`, suppression
  `(168,119,370,226)`.

Record exact focused compile/run command and output. A direct focused host test
is valid worker evidence for this packet; aggregate/canonical registration is a
later Foreman integration action.

### Explicitly out of scope

Do not implement:

- controller acquisition or calibration foreground input;
- held-X review acceptance or release quarantine;
- A005 input routing;
- RFB safe freeze/full-refresh scheduling;
- accept-to-first-frame orchestration;
- START/RETIRE sends or real Pi/Transport mechanics;
- exact-generation preparation/retirement;
- MPEG decoder/worker changes;
- Presentation/GS compositor/raster/overlay work;
- first physical frame promotion;
- common-media-clock arm;
- presentation scheduler/drop policy;
- DESKTOP CALIBRATION changes;
- hardware qualification.

### Acceptance criteria

`A004-C1 MPEG_CALIBRATION_OWNER_BOUNDARY` — source is owned by coherent
`src/mpeg_calibration/` with a narrow public owner seam and no imported
cross-owner internals.

`A004-C2 GEOMETRY_MEANINGS` — base, inner matte, and outer/suppression retain
their distinct audited meanings and deterministic invariants.

`A004-C3 SINGLE_COMMITTED_AUTHORITY` — one accepted immutable MPEG CALIBRATION
geometry is the sole committed authority; later draft edits/cancel cannot mutate
it.

`A004-C4 DESKTOP_SEPARATION` — DESKTOP CALIBRATION remains untouched and is not
used as MPEG behavior authority.

`A004-C5 CONTRACT_HOST_TESTED` — deterministic focused host tests prove the
owner-local contract. Direct focused execution is sufficient worker evidence;
canonical aggregate registration remains a Foreman integration chore.

`A004-C6 BOUNDED_SCOPE` — no RFB/Input/Transport/Pi/Presentation/Application
lifecycle behavior is reconstructed.

No criterion is currently accepted `MET` from the failed prior attempt. The next
worker must disposition all six from its own landed source/evidence.

### Worker handoff

Return and record:

- exact `STARTED_AT`;
- starting branch authority and actual starting local `HEAD`;
- preflight result including dirty/local-only state;
- substantive source commit(s);
- exact files changed and owner-boundary rationale;
- criterion disposition A004-C1 through A004-C6;
- exact focused host-test command/output;
- exact Foreman-owned canonical integration action now required;
- any precise remaining evidence labels;
- exactly one canonical immutable Reconstruction work log if the worker reaches
  shift end and has the exact required log metadata.

Do not perform the Foreman-owned integration. Do not advance to another A004
slice. Stop and return the baton.

## Foreman next pickup

Consume only the A004 MPEG CALIBRATION core handoff. If coherent worker source,
tests, local dictionaries, and focused host evidence have landed, perform the
canonical new-domain integration required by current topology policy:

- topology policy;
- living file/service map;
- continuity expected-domain registration;
- generated dictionary portal;
- canonical test/build/tool registration actually required by the landed files;
- strict/project checks.

Then disposition A004-C1 through A004-C6 from repository evidence and issue at
most the smallest necessary continuation/next A004 slice.

Do not reopen autonomous A003 Transport/Pi work.

PENDING_MANUAL_TRANSPORT=A003 real Pi PSTV owner; exact START semantic/prepared-generation implementation; producer/emission/retirement fence; PS2 exact completion/decoder join/drain-credit; repeated-generation same-stream proof
PENDING_CROSS_MACHINE=A003 real PS2/Pi generation transaction; downstream all-guns owner integration
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification; A004-A006 later physical/all-guns qualification
