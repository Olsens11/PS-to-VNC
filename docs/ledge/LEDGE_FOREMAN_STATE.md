# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0019
RECORDED_AT=2026-09-17T17:01:10-04:00
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
BASED_ON_A003_MANUAL_COMPLETION_REVISION=0002
SUPERSEDES_FOREMAN_STATE_REVISION=0018
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This revision activates the first bounded user-assisted A003 Transport/Pi manual engineering slice after the normal development execution-seat gate was satisfied. It does not implement product behavior, does not declare A003 complete, does not authorize later producer/retirement lifecycle work, and does not wake A004/A005/A006 product work.

The live branch was re-read immediately before the governing manual-authority write and had authority:

`b7c9e3575987876672bb28d74d31053177b5f71b`

The manual-completion authority was then advanced to revision `0002` by:

`7b8fd1cdcd5c20bc2f847a4a3eeb09708053c924` — `docs(a003): activate first manual Pi ownership slice`

Independent Validation remains separate authority. Cross-machine Pi/PS2 proof and physical qualification remain pending unless separately and explicitly evidenced later.

## Current Foreman phase

`A003_MANUAL_P2A_START_PREPARED_ACTIVE__A004_MPEG_CALIBRATION_CORE_HELD`

## Execution-seat gate — satisfied for the manual session

The user-assisted manual session reports and has already exercised the normal development seat at:

`/home/ps2/src/PS-to-VNC-ledge-manual`

at exact authority:

`b7c9e3575987876672bb28d74d31053177b5f71b`

with:

- fresh standalone checkout from current remote authority;
- old `/home/ps2/src/PS-to-VNC` checkout left untouched;
- branch `ledge/h1-all-guns` and clean worktree confirmed;
- ordinary filesystem create/read/remove PASS;
- ordinary Git index/staging and cleanup PASS;
- `scripts/resume-state.sh` PASS;
- `scripts/check.sh` PASS;
- post-check worktree clean PASS;
- `git push --dry-run origin HEAD:refs/heads/ledge/h1-all-guns` PASS;
- `ps2run-interactive` available as the normal interactive Pi command path.

This satisfies the execution-route admission problem that previously blocked useful interactive work. It is not product evidence and does not itself advance A003 criteria.

## Governing A003 manual authority

`docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md`

is now revision `0002`, status:

`A003_TRANSPORT_PI_MANUAL_COMPLETION_ACTIVE`

with exactly one active packet:

`A003-P2A-MANUAL-START-PREPARED-R1`

The prior revision-0001 deferred status is superseded for this first slice only. The remaining later manual sequence stays deferred until a later Foreman packet.

## Stable work / role / log identity

The active manual engineering shift continues the existing A003 work identity:

- `ROLE_KEY=reconstruction`
- `WORK_ITEM_KEY=a003-mpeg-generation`
- `WORKER_KEY=interactive`
- `EXECUTION_MODE=USER_ASSISTED_MANUAL`
- `EXECUTION_SEAT=/home/ps2/src/PS-to-VNC-ledge-manual`
- `NORMAL_COMMAND_PATH=ps2run-interactive`

The worker must capture a fresh truthful exact `STARTED_AT`. Its eventual immutable work log is:

`docs/ledge/work-log/<started-at-safe>__reconstruction__a003-mpeg-generation__interactive.md`

Do not reuse any historical timestamp or rename the continuing work item.

## Active packet — A003-P2A-MANUAL-START-PREPARED-R1

PACKET_STATUS=`ACTIVE`
WORK_ITEM_KEY=`a003-mpeg-generation`
TARGET_WORKER=`interactive`
ROLE_KEY=`reconstruction`
WORKER_KEY=`interactive`
ASSIGNING_BASE_HEAD=`7b8fd1cdcd5c20bc2f847a4a3eeb09708053c924`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A003_MPEG_GENERATION.md:0001`
ASSIGNING_ARCHITECTURE_OVERLAY=`LEDGE_ARCHITECTURE_OVERLAY.md:0003`
ASSIGNING_MANUAL_AUTHORITY=`LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md:0002`
ASSIGNING_FOREMAN_STATE=`0019`

### Objective

Complete only the first real-Pi P2A slice:

1. inspect the actual running Pi PSTV/VNC/network/service topology and determine present-day ownership;
2. discuss that ownership evidence with the user **before** selecting the production owner/path;
3. adopt/implement exactly one real Pi PSTV session owner;
4. establish/prove sole Pi physical receive ownership;
5. receive and explicitly classify the already-accepted exact START control through that sole receiver;
6. route START through the narrow owner/public seam to the exact-generation semantic owner rather than storing generation business state in Transport;
7. implement/prove exact START semantic validation and exactly one immutable prepared generation;
8. reject malformed, wrong-session, zero, stale, repeated, or conflicting START without mutating accepted prepared state.

### Immediate read-only authorization

**Read-only real-Pi runtime/service archaeology may begin immediately under this packet.**

The worker may inspect live processes, sockets, service/unit definitions, launch scripts, VNC/Xvnc/session ownership, networking/service configuration, production Pi companion source/configuration, actual runtime paths, and retained H1 evidence needed to determine the genuine owner boundary.

This read-only archaeology does not require a second Foreman wake.

### Mandatory discussion gate before production writes

After collecting the ownership evidence, the worker must present it to the user and discuss the viable production owner/path before making behavior-bearing product or production-runtime/service changes.

No preselected `pi/pstv/` path, historical H1 class shape, or caller convenience may substitute for that evidence-led user-assisted decision.

### Authorized write scope after the discussion gate

After the owner/path is explicitly settled with the user, Reconstruction may make only the behavior-bearing changes needed to this slice:

- adopt or implement the one real Pi PSTV session owner at the evidence-supported production location;
- make that owner the sole physical receive owner for the one PS2-facing PSTV session;
- implement explicit START envelope reception/classification through that owner;
- expose/use the narrow owner seam from Transport/session mechanism to exact-generation semantic ownership;
- implement exact-generation START validation and one immutable prepared value;
- inject immutable active Pi desktop/session bounds from the actual desktop/session authority;
- add behavior-specific tests for valid/invalid START semantics, sole receive ownership, and no-mutation rules;
- add owner-local file synopses/documentation and local `SYMBOLS.md` if a genuine new local-cooperation component is established;
- make the smallest service/config/source changes genuinely necessary to adopt that production owner.

If canonical/global topology registration, generated dictionary portal reconciliation, generic build/test registration, or similar non-behavioral integration becomes necessary after coherent behavior lands, commit/preserve the worker-owned slice and return that exact chore to Foreman rather than broadening the manual packet.

### Required START semantics

Preserve the accepted exact START wire identity and validate semantics at the exact-generation owner against:

- START v1 exact representation/version;
- exact active session id;
- nonzero generation;
- monotonic/stale-generation authority;
- safe integer conversion/range before signed geometry use;
- base width and height minimum 16 and 16-pixel alignment;
- positive suppression width/height;
- suppression fully containing the exact base rectangle;
- base and suppression inside immutable active Pi desktop/session bounds;
- overflow-safe right/bottom arithmetic.

A valid START stores exactly one immutable prepared value containing session id, generation, base rectangle, and suppression rectangle. Once prepared, malformed, wrong-session, zero, stale, repeated, or conflicting START must fail without mutation. Preparation must not activate a producer.

Active desktop bounds are Pi-local immutable session metadata from the actual desktop/session owner. They are not derived from START, not hardcoded as 704x462 architecture, and not PS2 DESKTOP CALIBRATION state.

## Acceptance criteria for the active packet

- `M1-C1 ACTUAL_PI_OWNERSHIP_EVIDENCE = PENDING_MANUAL`: real running ownership inspected and discussed with the user before production path selection.
- `M1-C2 PI_SOLE_SESSION_OWNER = PENDING_MANUAL`: exactly one real Pi PSTV session owner owns the accepted socket/session and sole physical receive chain.
- `M1-C3 EXPLICIT_START_RECEIVE = PENDING_MANUAL`: exact START is received/classified only by explicit control identity through the sole owner; no MPEG DATA payload sniffing.
- `M1-C4 OWNER_SPLIT = PENDING_MANUAL`: Transport/session owns mechanism; exact-generation owner owns semantic validation/prepared state.
- `M1-C5 IMMUTABLE_PREPARED_GENERATION = PENDING_MANUAL`: one valid START produces exactly one immutable prepared generation.
- `M1-C6 REJECTION_NO_MUTATION = PENDING_MANUAL`: malformed/wrong-session/zero/stale/repeated/conflicting START rejection leaves accepted state unchanged.
- `M1-C7 BOUNDED_SCOPE = PENDING_MANUAL_HANDOFF`: no later lifecycle/product stage is introduced.

No criterion is pre-awarded by this Foreman activation.

## Accepted A003 facts that remain binding

Preserve without rework unless a concrete defect is found:

- START kind `11` / control channel `0` / flags `0`;
- exact 44-byte START v1 representation;
- RETIRE kind `10` / control channel `0` / flags `0`;
- exact 12-byte RETIRE representation;
- MPEG media `DATA/channel 4`;
- no payload-length sniffing;
- no per-MPEG-packet generation tags;
- START-shaped exactly-44-byte `DATA/channel 4` remains MPEG media;
- existing ordered PS2 outbound START framing.

Current P2A disposition at packet issue:

- `P2A-1 START_CONTROL_IDENTITY = MET`
- `P2A-2 START_SEMANTICS = PARTIAL / ACTIVE_MANUAL_SLICE`
- `P2A-3 RETIRE_WIRE_IDENTITY = MET`
- `P2A-4 PURE_MPEG_DATA = MET`
- `P2A-5 SOLE_TRANSPORT_OWNER = PARTIAL / ACTIVE_MANUAL_SLICE`
- `P2A-6 OWNERSHIP = PARTIAL / ACTIVE_MANUAL_SLICE`
- `P2A-7 NO_GENERATION_TAGGING = MET`
- `P2A-8 BOUNDED_SCOPE = MET`

A003 remains incomplete.

## Binding architecture invariant

Architecture overlay revision `0003` remains binding:

> Inside one genuine component/local-cooperation directory, internal implementation files may cooperate directly. Across a real component boundary, communication must pass through the owning component's defined bridge or public process seam.

Transport owns mechanism. It must not become the exact-generation business-state owner or a generic event bus.

## Explicit non-goals / unauthorized work

This packet does **not** authorize:

- MPEG producer activation or MPEG emission;
- producer preparation beyond the immutable START state required by this slice;
- emission admission/fencing;
- retirement send-lease or in-flight emission fencing;
- RETIRE completion/ack lifecycle;
- PS2 decoder stop/join or worker-resource finalization;
- residual MPEG queue discard or credit return;
- generation `N -> RETIRE(N) -> N+1` completion;
- repeated-generation lifecycle completion;
- A004/A005/A006 product work;
- A003 completion claims;
- independent Validation PASS;
- cross-machine or hardware qualification claims beyond evidence actually obtained.

RETIRE wire identity remains accepted evidence only; retirement semantics are not part of this packet.

## Required authority to re-read before proceeding

At manual shift start/resume, read/re-read at minimum:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/status.md`;
- `docs/README.md`;
- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/development/README.md`;
- `docs/development/source-naming-and-symbols.md` before product-source changes;
- `docs/development/source-topology.md` before adding/moving product files/directories;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision `0005`;
- `docs/ledge/work-log/README.md` revision `0006`;
- this Foreman state revision `0019`;
- `docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md` revision `0002`;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision `0003`;
- `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision `0001`;
- newest relevant `a003-mpeg-generation` immutable logs.

Refresh live `ledge/h1-all-guns` authority and actual local worktree state before writes. Do not discard inherited/local work.

## A004 disposition

The former A004 execution-route blocker has been demonstrated solvable by the fresh normal checkout, but A004 is **not activated** by that fact.

`A004-MPEG-CALIBRATION-CORE-R2` remains held while this bounded A003 manual slice is active. Do not perform A004/A005/A006 product work under the A003 manual packet.

## Remaining manual sequence after this slice

The later A003 sequence remains deferred until a subsequent Foreman packet:

1. exact-generation producer preparation;
2. MPEG emission;
3. retirement admission close;
4. deliberate in-flight emission lease during retirement;
5. producer cleanup-before-RETIRE-completion proof;
6. PS2 exact RETIRE completion publication;
7. PS2 decoder stop/join resource fence;
8. residual MPEG queue discard and Transport credit return;
9. generation N to N+1 contamination fence;
10. repeated-generation and finite cancel/error/EOF/retirement behavior;
11. later all-guns owner integration;
12. separate cross-machine/hardware qualification.

## Foreman next pickup

Take the baton back when one of these occurs:

- M1A ownership archaeology reaches the mandatory user discussion/production-path decision and a Foreman clarification is actually required;
- the worker completes coherent M1B behavior/tests and reaches canonical/global integration chores;
- a genuine authority/ownership collision blocks safe progress;
- or the bounded M1 packet reaches its acceptance/stop point.

Do not broaden into producer emission or retirement merely because the first START-prepared path works.

PENDING_MANUAL_TRANSPORT=A003 actual Pi PSTV owner; sole receive ownership; exact START semantic/prepared-generation implementation; later producer/emission/retirement fence; PS2 exact completion/decoder join/drain-credit; repeated-generation same-stream proof
PENDING_CROSS_MACHINE=A003 real PS2/Pi generation transaction and later all-guns owner integration
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification; A004-A006 later physical/all-guns qualification
