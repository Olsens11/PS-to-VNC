# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0020
RECORDED_AT=2026-09-17T17:42:23-04:00
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
BASED_ON_WIRE_TERMINOLOGY_REVISION=0001
SUPERSEDES_FOREMAN_STATE_REVISION=0019
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

Revision `0020` is a terminology-governance advance only. It keeps the currently active manual A003 engineering packet active, adopts the architecture-neutral Wire terminology from `docs/ledge/LEDGE_WIRE_TERMINOLOGY.md` revision `0001`, and queues one separate bounded terminology migration for later. It does not rename product source, does not change wire behavior, does not alter A003 acceptance criteria, and does not activate A004/A005/A006 product work.

The live branch was refreshed immediately before the terminology-policy write at:

`4aff9d920fd61895a1aab0d9b8f09d004fc3537f`

The terminology policy was then committed as:

`53274f12dcd4988a8e77985db20a4c7a96d97ba8` — `docs(architecture): adopt neutral Wire terminology`

## Current Foreman phase

`A003_MANUAL_P2A_START_PREPARED_ACTIVE__WIRE_TERMINOLOGY_DECIDED_MIGRATION_DEFERRED__A004_HELD`

## Active A003 manual packet remains unchanged in scope

The active product packet remains:

`A003-P2A-MANUAL-START-PREPARED-R1`

with stable execution identity:

- `ROLE_KEY=reconstruction`
- `WORK_ITEM_KEY=a003-mpeg-generation`
- `WORKER_KEY=interactive`
- `EXECUTION_MODE=USER_ASSISTED_MANUAL`
- `EXECUTION_SEAT=/home/ps2/src/PS-to-VNC-ledge-manual`
- `NORMAL_COMMAND_PATH=ps2run-interactive`

Its governing behavior authority remains `docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md` revision `0002` plus architecture overlay revision `0003`, A003 audit revision `0001`, and the new terminology policy revision `0001`.

Nothing in this terminology decision changes the active packet objective, mandatory ownership-discussion gate, accepted START/RETIRE behavior, acceptance criteria `M1-C1` through `M1-C7`, explicit non-goals, or later deferred A003 sequence.

Read-only real-runtime/service archaeology remains authorized immediately under the active packet. Behavior-bearing production writes still require the existing user discussion/owner-path decision first.

## Canonical architecture-neutral terminology going forward

`docs/ledge/LEDGE_WIRE_TERMINOLOGY.md` revision `0001` is governing terminology authority.

Use:

- **Wire Transport** = physical peer connection/session, ordered send/receive, framing, multiplexed logical channels, Transport flow control, and mechanism-level control delivery;
- **Wire Protocol** = the project-owned framed protocol carried by Wire Transport, including header representation, frame kinds, channel identities, sequence semantics, START/RETIRE wire representations, and related framing/control representation;
- **Wire Session**;
- **Wire Frame**;
- **Wire Channel**.

For architecture-neutral project-owned concepts, do not introduce endpoint-specific `PS2`, `Pi`, or `PSTV` names merely because those are the current deployed endpoints. Use responsibility-based names.

Externally defined or genuinely platform-specific names remain specific when materially true, including names such as `ps2sdk`, `ps2ip`, `libpad`, `GS`, `EE`, `IOP`, and `audsrv`.

Historical/forensic source and immutable historical logs remain unchanged.

## Transition rule for the active A003 packet

The active packet and older governing documents contain existing `PSTV`, `Pi PSTV Session`, and similar transition-era vocabulary. Those existing words do **not** require an opportunistic rename during tonight's functional work.

Interpret architecture-neutral uses according to the new Wire terminology, but:

- existing committed identifiers may remain as-is;
- existing source/API names must not be half-renamed inside unrelated functional changes;
- new architecture-neutral concepts created by the active worker must use neutral responsibility vocabulary where available;
- real endpoint/platform references may still say `PS2` or `Pi` when the endpoint itself is actually relevant;
- current ownership archaeology may quote actual process/service/file names exactly even when they contain legacy vocabulary.

This is an interpretation/naming rule, not a second engineering objective for the active worker.

## Wire behavior remains frozen by this naming decision

No terminology work may change:

- wire magic bytes;
- protocol version;
- frame kinds;
- channel IDs;
- header representation;
- sequence behavior;
- START identity or payload layout;
- RETIRE identity or payload layout;
- accepted MPEG `DATA/channel 4` behavior;
- any other accepted wire behavior.

The existing A003 facts remain preserved, including START kind `11` / control channel `0` / flags `0`, exact 44-byte START v1, RETIRE kind `10` / control channel `0` / flags `0`, exact 12-byte RETIRE, no payload-length sniffing, no per-MPEG-packet generation tags, and the accepted ordered outbound START framing.

## Deferred bounded migration — WIRE-TERMINOLOGY-MIGRATION-R1

MIGRATION_ID=`WIRE-TERMINOLOGY-MIGRATION-R1`
WORK_ITEM_KEY=`global-wire-terminology`
PACKET_STATUS=`DEFERRED__DO_NOT_WAKE_DURING_ACTIVE_A003_MANUAL_SESSION`
TARGET_WORKER=`UNASSIGNED_UNTIL_FUTURE_FOREMAN_WAKE`
GOVERNING_TERMINOLOGY=`LEDGE_WIRE_TERMINOLOGY.md:0001`

Objective: perform one bounded terminology/source-name migration after the current interactive engineering packet returns its baton, without changing behavior.

The migration must inventory and classify each candidate occurrence before editing:

- `A` project-owned architecture term -> migrate to canonical Wire terminology;
- `B` external/platform named entity -> preserve;
- `C` historical/forensic evidence -> preserve;
- `D` stable wire-format identifier/constant -> preserve where compatibility requires;
- `E` ambiguous occurrence -> inspect; never mechanically replace.

The migration must specifically inspect rather than assume:

- whether `pstvnc_` is a stable project namespace or deprecated component terminology;
- whether `src/transport/` is already sufficiently responsibility-based and should remain unchanged.

A global string replacement is prohibited. The migration may not alter wire behavior or accepted lifecycle semantics.

## A004 disposition

`A004-MPEG-CALIBRATION-CORE-R2` remains held. The terminology migration is also held. Tonight's active work remains only `A003-P2A-MANUAL-START-PREPARED-R1`.

## Foreman next pickup

Take the baton back when the active A003 packet reaches its existing Foreman stop point. Do not wake `WIRE-TERMINOLOGY-MIGRATION-R1` merely because it is now queued.

When a later Foreman deliberately activates that migration, size it as one bounded non-behavioral naming tranche with explicit inventory/classification evidence and canonical checks.

PENDING_MANUAL_TRANSPORT=A003 actual Wire Transport/session owner; sole receive ownership; exact START semantic/prepared-generation implementation; later producer/emission/retirement fence; PS2 exact completion/decoder join/drain-credit; repeated-generation same-stream proof
PENDING_TERMINOLOGY_MIGRATION=global-wire-terminology / WIRE-TERMINOLOGY-MIGRATION-R1
PENDING_CROSS_MACHINE=A003 real PS2/Pi generation transaction and later all-guns owner integration
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification; A004-A006 later physical/all-guns qualification
