# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0014
RECORDED_AT=2026-09-16T22:10:43-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0035
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A003_AUDIT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision corrects the A003-P2 packet authority after re-reading the governing A003 semantic audit, current clean Transport protocol/runtime ownership, and frozen H1 START/RETIRE evidence. It does not implement P2 product behavior.

The live branch was re-read immediately before this state write at `c41f0972ff64711ff3f27951006a1966359e6c66`. The previously established recovery result remains intact: failed interactive P2 attempts left no committed half-P2 product implementation, so no product rollback is required. Immutable historical/support records remain untouched.

A003-P1 remains Foreman-accepted as `MET` from revision 0013. Independent Validation status and all physical qualification remain separate; this state does not declare Validation PASS or hardware qualification.

## Current Foreman phase

`A003_P2A_EXPLICIT_START_CONTROL_PACKET_ISSUED`

Foreman state revision 0013's monolithic P2 packet is superseded. In particular, its requirement that production START reuse the exact historical `DATA` / MPEG channel-4 envelope is withdrawn.

## Authority correction — START semantics versus H1 envelope

The governing A003 audit classifies exact START as:

- historical mechanism: `44-byte START v1 + Pi prepared-generation state`;
- disposition: `KEEP_BEHAVIOR + REWRITE` representation.

Therefore the binding behavior is the exact-session/exact-generation prepared-state contract and its validated MPEG CALIBRATION / MPEG presentation geometry, not the experimental choice to encode START as ordinary `DATA` on MPEG channel 4.

Revision 0013 incorrectly promoted that H1 envelope choice into a production invariant. This revision supersedes that interpretation.

The preserved START semantics are:

- exact active session identity;
- nonzero exact generation identity;
- one immutable prepared generation at a time;
- stale/repeated generation rejection;
- exact MPEG base/draw rectangle;
- exact MPEG suppression rectangle;
- structural, signed-range, alignment, containment, and active-desktop bounds validation;
- malformed/invalid START never prepares or activates a producer.

This is MPEG CALIBRATION / MPEG presentation geometry authority only. It is distinct from DESKTOP CALIBRATION and does not begin A004 reconstruction.

## Clean control representation decision

Current clean Transport protocol authority owns PSTV framing and currently defines frame kinds 1 through 7 plus logical control channel 0 and MPEG2 channel 4. Frozen H1 authority extends the same namespace with:

- kind 8 = `MEDIA_END`;
- kind 9 = `SESSION_RESULT`;
- kind 10 = exact-generation `RETIRE`.

The smallest collision-free explicit START control identity is therefore the next frame-kind value after the frozen namespace:

- START frame kind = `11`;
- START channel = control / `0`;
- START flags = `0`;
- START payload = the proven 44-byte v1 payload, unchanged in semantic fields and byte order.

RETIRE remains:

- RETIRE frame kind = `10`;
- RETIRE channel = control / `0`;
- RETIRE flags = `0`;
- RETIRE payload = exact 12-byte big-endian `>3I` tuple `(version=1, session_id, generation)`.

Kinds 8 and 9 are not repurposed by P2A. The START assignment is not derived from payload length: it is a named control-frame identity chosen only after inspecting the current clean namespace and frozen H1 assignments.

Ordinary MPEG media remains exclusively:

- frame kind = `DATA`;
- channel = MPEG2 / `4`;
- payload = opaque MPEG bytes of any valid Transport length, including exactly 44 bytes.

No receiver may recognize START by sniffing MPEG media payload length or contents. A legitimate 44-byte MPEG DATA payload is MPEG media, never START.

## Ownership boundary

Transport owns:

- PSTV frame-kind/channel identity;
- ordered physical send/receive;
- the sole physical receive owner;
- dispatch of explicit START/RETIRE control envelopes to a narrow exact-generation control seam;
- ordinary MPEG DATA dispatch to the accepted P1 MPEG queue.

Transport does not own exact-generation business semantics and must not parse MPEG media payloads to infer START.

Application / exact-generation coordination owns START/RETIRE lifecycle meaning and prepared-generation state. MPEG owns decoder/worker behavior. Pi generation/producer ownership will own preparation/emission/retirement behavior in later packets.

## Fresh bounded Reconstruction packet — A003-P2A

WORK_ITEM_KEY=`a003-mpeg-generation`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`c41f0972ff64711ff3f27951006a1966359e6c66`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A003_MPEG_GENERATION.md:0001`
ASSIGNING_FOREMAN_STATE=`0014`

### Objective

Reconstruct only the exact-generation control-plane representation and prepared-generation contract around the accepted P1 MPEG path. Make START and RETIRE explicit Transport control messages while preserving the proven START payload semantics and keeping channel-4 DATA pure MPEG media.

### Required deliverables

1. **Explicit START control envelope.** Add the smallest owner-correct named Transport START control identity: frame kind `11`, control channel `0`, flags `0`. Do not encode START as MPEG channel-4 DATA and do not classify it by payload length/content on the MPEG receive path.

2. **Preserved START v1 payload semantics.** Preserve the proven exact 44-byte, eleven-big-endian-word START payload contract: version, session id, generation, base/draw x/y/width/height, suppression x/y/width/height. Preserve signed-range conversion rules, nonzero generation, 16-pixel base dimension alignment/minimums, positive suppression size, containment, and active-desktop bounds validation through the owner-correct exact-generation seam.

3. **Prepared-generation state.** Preserve one immutable prepared generation at a time. Reject wrong-session, zero-generation, stale, repeated, malformed, out-of-bounds, or conflicting START without mutating the accepted prepared generation. P2A does not activate an MPEG producer.

4. **Explicit RETIRE wire codec/identity only.** Add/preserve kind `10`, control channel `0`, flags `0`, exact 12-byte big-endian `>3I` `(1, session_id, generation)` encode/decode and exact identity validation. P2A does not implement the Pi cleanup/ack fence or PS2 worker/join/drain fence.

5. **Sole-owner dispatch.** Route START/RETIRE through the existing physical Transport owner and a narrow application/exact-generation control seam. Preserve exactly one PSTV connection and one receive owner. Ordinary `DATA/channel 4` continues unchanged to the accepted P1 MPEG queue.

6. **Media/control separation proof.** Add deterministic host coverage proving a valid 44-byte MPEG DATA payload is accepted as MPEG media and is never reclassified as START. START recognition must depend on explicit frame-kind/control-channel identity, not payload sniffing.

7. **Malformed/stale/control mismatch coverage.** Cover wrong frame kind/channel/flags/length/version/session/generation, signed-range/geometry/alignment/containment/desktop-bound failures, repeated/stale START, conflicting prepared state, malformed RETIRE, and ordinary MPEG DATA regression.

8. **Boundary discipline.** Do not implement Pi emission leases/producer cleanup, PS2 decoder retirement/join/residual drain/credit fence, successor-generation reopen, presentation/common-clock arm, A004 MPEG calibration reconstruction, DESKTOP CALIBRATION changes, per-MPEG-DATA generation tags, another socket/receive owner, guessed production tuning, or hardware qualification.

### Acceptance criteria

P2A-1 `START_CONTROL_IDENTITY` — START is an explicit named control frame (kind 11/channel 0/flags 0), not DATA/channel 4 and not payload-sniffed.

P2A-2 `START_SEMANTICS` — the proven 44-byte v1 fields and exact session/generation/MPEG-presentation-geometry validation are preserved, with one immutable prepared generation and stale/repeat rejection.

P2A-3 `RETIRE_WIRE_IDENTITY` — RETIRE remains kind 10/channel 0/flags 0 with exact 12-byte `>3I` identity validation; no cleanup/ack lifecycle is claimed yet.

P2A-4 `PURE_MPEG_DATA` — ordinary MPEG remains DATA/channel 4 opaque media, and a legitimate 44-byte MPEG payload is demonstrably not START.

P2A-5 `SOLE_TRANSPORT_OWNER` — one physical PSTV connection and one physical receive owner remain authoritative; START/RETIRE dispatch adds no competing reader/socket.

P2A-6 `OWNERSHIP` — Transport owns envelope/dispatch only; exact-generation coordination owns START/RETIRE semantic validation/prepared state; MPEG decoder ownership remains unchanged.

P2A-7 `NO_GENERATION_TAGGING` — no per-MPEG-DATA generation tag, magic 44-byte discriminator, or equivalent media sniffing appears.

P2A-8 `BOUNDED_SCOPE` — no Pi emission fence, PS2 retirement/join/drain/credit fence, successor transaction, presentation/deadline policy, A004 reconstruction, or hardware claim is introduced.

### Blocker burden

Hardware is not a blocker for P2A. If an existing public Transport seam cannot dispatch a named control frame without exposing private runtime state, add only the smallest reusable owner-correct control-dispatch primitive. Do not reach around the sole receiver and do not broaden into P2B/P2C behavior.

If current source authority moves before implementation, re-read it and preserve these semantics unless newer governing authority explicitly supersedes this state.

## Deferred successor packets — not authorized yet

- P2B: Pi exact-generation producer preparation/emission-admission/lease/cleanup fence.
- P2C: PS2 exact-generation retirement completion, decoder stop/join, residual queue discard and Transport credit restoration.
- P2D: complete ordered `N -> RETIRE(N) -> N+1` transaction proof.

These labels are dependency planning only. No worker is authorized to implement them from this state.

## Next Foreman pickup

Consume only a P2A Reconstruction handoff. Inspect explicit START/RETIRE control identity, START payload/prepared-state equivalence, sole-owner dispatch, the 44-byte MPEG media regression, and P2A boundary discipline criterion-by-criterion before any canonical integration or successor packet.

PENDING_LOCAL=A003-P2A worker handoff and later Foreman integration/independent Validation; existing repository-governance evidence gaps remain separate from P2A product behavior
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification
