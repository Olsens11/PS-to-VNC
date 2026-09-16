# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0007
RECORDED_AT=2026-09-16T08:19:46-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0003
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0023
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A002_AUDIT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision consumes the independent A001 machine/source Validation PASS recorded after Foreman revision 0006 and opens the first bounded A002 reconstruction packet. The live branch was re-read immediately before this state write at `3552e8790e63eb53866c87f97fad98cd9653ac13`.

Global work state revision 0023 and Reconstruction state revision 0007 are older point-in-time snapshots and are stale relative to the accepted A001 source/machine tranche. They remain historical authority for what was known at their recorded times; they do not block this planning transition.

## Current foreman phase

`A002_CONFIG_PROFILE_PACKET_ISSUED`

A001 is independently accepted at machine/source level. Validation state revision 0006 records `VALIDATION_A001_MACHINE_SOURCE=PASS`, findings revision 0005 records V003/V004/V005 PASS, and `A002_PLANNING_BLOCKED_BY_VALIDATION=NO`.

A001 physical PS2 qualification remains explicitly `HARDWARE_PENDING`. This packet does not convert machine evidence into physical qualification and does not remove that debt.

## A002 planning conclusion

A002 is not a task to fill eight numeric constants and is not authority to reproduce H1's complete 61-field laboratory CONFIG vocabulary as a production API.

The first dependency is the clean session/profile authority that A001 intentionally left absent:

- `pstvnc_app_run()` remains fail-closed because no production producer exists for the validated Transport session subprofile;
- the existing `pstvnc_transport_session_config_t` is the stable A001 value boundary and contains eight Pi-supplied per-session values;
- current clean `src/config/` contains generic text helpers but no typed session/profile owner;
- H1 proves that CONFIG is received before runtime resource allocation/start, is versioned, rejects malformed/unknown/duplicate/missing fields, and carries per-session values from the Pi;
- A002 audit revision 0001 requires simplifying that laboratory mechanism into narrow owner-specific immutable values while preserving completeness and structural rejection behavior;
- PCM runtime and shared-media-clock behavior are also A002-owned, but they should build on a coherent typed profile rather than force configuration ownership to emerge accidentally from the audio worker.

The first A002 packet therefore establishes the pure configuration/profile foundation only. Runtime CONFIG negotiation, PCM playback, AUDSRV lifecycle, and the media-clock runtime remain subsequent A002 work after this value/decoder boundary is proven.

## Fresh interactive Reconstruction packet

WORK_ITEM_KEY=`a002-audio-clock`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`3552e8790e63eb53866c87f97fad98cd9653ac13`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md:0001`
ASSIGNING_VALIDATION_STATE=`0006`

### Objective

Reconstruct A002's clean, bounded, versioned session CONFIG/profile owner and pure decoder/validator so the existing A001 Transport subprofile and the A002-owned PCM/common-clock session facts can come from one explicit immutable authority without guessed defaults or a 61-field production god object.

This packet is intentionally a value/decoding tranche. It must not begin the PCM worker, AUDSRV service lifecycle, shared-clock wait/arming runtime, or A003 MPEG/presentation behavior.

### Required deliverables

1. **Clean configuration ownership.** Establish the smallest coherent typed session/profile values in `src/config/` needed by current A001/A002 responsibilities. Use narrow owner-specific subprofiles rather than one H1-shaped struct shared by every component.

2. **A001 Transport subprofile provenance.** The decoded profile must carry or produce the existing `pstvnc_transport_session_config_t` without changing that A001 boundary or manufacturing defaults. All eight values remain explicit session authority:
   - RFB queue capacity;
   - RFB initial credit;
   - RFB credit batch;
   - RFB flush-on-empty;
   - RFB credit-return enable;
   - receiver-thread stack size;
   - receiver-thread priority;
   - maximum DATA payload.

3. **A002-owned product/session facts.** Represent the stable A002 semantics needed by later PCM/common-clock work, including at minimum session identity, audio activation OFF/PCM, PCM rate/channels/bits/volume, signed audio presentation offset, and the common-clock/profile inputs that current A002 audit genuinely owns. RFB activation may be represented as the composition fact required to select the A001 Transport subprofile. VIDEO activation/common-clock facts may be represented only to the extent the A002 audit establishes a stable neutral contract; do not import A003 decoder/presentation tuning.

4. **Versioned bounded decode.** Define a clean production CONFIG payload/decoder appropriate to the current PSTV CONTROL/CONFIG framing. Exact clean wire encoding is an implementation decision, not a requirement to preserve H1's 61-field layout. The decoder must be side-effect free and reject:
   - unsupported version;
   - malformed length/entry shape;
   - unknown field/tag identity;
   - duplicate fields;
   - missing required fields;
   - structurally impossible combinations.

5. **Owner-level validation.** Preserve real representation/API invariants without inventing speculative safe ceilings. At minimum, validation must cover:
   - the existing Transport queue/credit/thread/payload relationships required by A001;
   - receiver priority/stack representability and alignment;
   - PCM format constraints (8/16-bit, mono/stereo), positive/representable rate, volume range, and frame-aligned byte policies when those values are part of the clean profile;
   - activation/value contradictions;
   - exact signed-offset interpretation rather than accidental unsigned policy.

6. **Behavior-specific unit-test source.** Add deterministic host-test source for the new pure profile decoder/validation covering a complete valid profile plus wrong version, malformed input, unknown field, duplicate field, missing required field, invalid Transport relationships, invalid PCM format/volume, signed-offset preservation, and proof that absent A001 Transport fields are rejected rather than defaulted.

7. **Source-writing/topology discipline.** Before adding clean files, read and obey current `docs/development/source-naming-and-symbols.md` and `docs/development/source-topology.md`. Every maintained clean C/H file receives the required synopsis and readable naming/comments. If current topology policy requires `src/config/SYMBOLS.md` or deliberate directory adoption for the new definitions, make the reconstruction-owned source-side update that policy requires; routine canonical registration/generated-portal/checker work remains Foreman-owned.

### Acceptance criteria

C1. One explicit config-owned immutable session/profile authority exists; no consumer must recover session values from unrelated globals and no H1 61-field god object is reproduced.

C2. All eight existing A001 Transport session values have an explicit decoded/profile provenance and no code path substitutes fixed application defaults for a missing CONFIG value.

C3. The clean decoder is complete and deterministic: version, malformed length, unknown, duplicate, missing, and structural-invalid cases are rejected before runtime side effects.

C4. A002 PCM product semantics are represented and validated with signed offset preserved, while A003 MPEG/presentation details remain absent.

C5. The resulting typed boundaries are narrow enough that Transport, the future audio owner, and the future media-clock owner can consume only their relevant immutable values instead of depending on the whole profile representation.

C6. Deterministic behavior-specific host-test source covers the required positive/negative decoder and validator cases, including the no-default A001 boundary.

C7. Current A001 runtime behavior remains unchanged in this packet: `pstvnc_app_run()` may remain fail-closed; no live CONFIG handshake, Transport receive ownership, RFB lifecycle, fatal-abort, or quiesce behavior is weakened or redesigned.

C8. No PCM worker/AUDSRV runtime, no shared-clock wait/arming runtime, no MPEG/video reconstruction, and no physical-hardware claim is introduced.

### Preserved invariants / non-goals

- A001 machine/source PASS and its exact Transport/RFB ownership rules remain intact.
- One physical PSTV connection and one Transport-owned sole receiver remain the architectural foundation.
- Do not hardcode the eight Transport CONFIG values.
- Do not reproduce H1's mandatory 61-field production interface merely for wire familiarity.
- Do not make configuration a cross-domain mutable god object.
- Do not begin A003 decoder/generation/presentation behavior.
- Do not introduce `audsrv_quit()` or any per-session AUDSRV teardown policy in this packet.
- Do not silently address the inherited H1 next-session receive-poison defect.
- A001 hardware status remains `HARDWARE_PENDING`.

### Blocker burden

A design question about the exact clean CONFIG encoding is not by itself a blocker. First reconcile the A002 audit, H1 `h1_config.*` behavior, current PSTV CONFIG frame vocabulary, current clean config architecture, and source-writing/topology policy. Preserve required semantic behavior while deliberately simplifying the wire/profile shape.

If a particular future-media field cannot be classified without A003/A004 authority, omit/defer that field rather than guessing. That does not block completing the A002-owned subset.

### Fallback / stretch

If C1-C8 are complete early, the permitted stretch is pure, side-effect-free common-clock value/deadline math and its tests **only if** it can be implemented entirely within A002's audited ownership without selecting the video arm callsite or introducing runtime waiting/polling. Otherwise stop at the completed CONFIG/profile boundary and hand the baton back to Foreman.

## Assignment status

RECONSTRUCTION_A_PACKET=NONE
RECONSTRUCTION_B_PACKET=NONE
INTERACTIVE_PACKET=A002_CONFIG_PROFILE_FOUNDATION
A002_PACKET=ACTIVE
A001_MACHINE_SOURCE_STATUS=PASS
A001_HARDWARE_STATUS=HARDWARE_PENDING
SCHEDULED_A_B_FOREMAN_STATUS=OPERATOR_PAUSED_UNCHANGED

## Exact next pickup

The High-thinking interactive Reconstruction worker should consume this packet from current repository authority and execute C1-C8 as one sustained round, leaving exactly one canonical immutable `reconstruction/a002-audio-clock/interactive` work log before ending. It must reserve enough tool capacity for that handoff.

After the worker returns, Foreman should independently inspect current HEAD/source/tests/log, register any new host fixtures canonically, run/preserve available machine evidence, and decide the next A002 packet. Independent Validation should judge a coherent A002 tranche later; A001 physical qualification remains separately pending.
