DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T22:08:35-04:00
COMPLETED_AT=2026-09-16T22:11:27-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=c41f0972ff64711ff3f27951006a1966359e6c66
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman — A003-P2 recovery / P2A packet correction

## Objective and authority consumed

Correct the A003-P2 packet authority without repeating the already-settled rollback archaeology or implementing P2 product source.

Consumed current branch authority plus:

- Reconstruction contract revision 0005;
- immutable work-log contract revision 0005;
- current global work state revision 0035;
- Foreman state revision 0013;
- A003 semantic audit revision 0001;
- current clean `src/transport/protocol.h` and sole-receiver dispatch in `src/transport/runtime.c`;
- frozen H1 START validation evidence and H1 frame namespace/RETIRE evidence at forensic commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The prior recovery result was retained: failed P2 attempts did not leave a committed half-P2 product implementation. No destructive rollback was required and no immutable historical/support record was modified.

## Authority correction

A003 audit revision 0001 classifies exact START as historical `44-byte START v1 + Pi prepared-generation state` with disposition `KEEP_BEHAVIOR + REWRITE` representation.

Foreman state 0013 therefore overconstrained reconstruction when it required the production START envelope itself to remain ordinary `DATA` on MPEG channel 4. That interpretation is superseded by Foreman state revision 0014.

The preserved behavior remains exact session identity, nonzero exact generation, one immutable prepared generation, stale/repeated rejection, exact MPEG base/draw and suppression geometry, structural/alignment/containment/active-desktop bounds validation, and no producer activation from malformed/invalid START.

MPEG CALIBRATION / MPEG presentation geometry terminology is used for this START contract. DESKTOP CALIBRATION remains a separate system; no A004 work was authorized.

## Clean START control decision

Current clean Transport protocol defines frame kinds 1-7 and control channel 0 / MPEG2 channel 4. Frozen H1 evidence occupies frame kind 8 (`MEDIA_END`), kind 9 (`SESSION_RESULT`), and kind 10 (`RETIRE`).

The corrected P2A packet therefore uses the smallest collision-free explicit control identity:

- START = frame kind 11, control channel 0, flags 0;
- START payload = preserved 44-byte v1 eleven-word big-endian payload;
- RETIRE = frame kind 10, control channel 0, flags 0;
- RETIRE payload = exact 12-byte big-endian `>3I` `(1, session_id, generation)` tuple;
- kinds 8 and 9 are not repurposed.

Ordinary MPEG remains `DATA` on channel 4 with opaque media payload. START recognition may not depend on payload length/content sniffing. A legitimate 44-byte MPEG DATA payload must remain MPEG media and never be reclassified as START.

Transport owns envelope/framing, sole physical receive ownership, ordered dispatch, and control-envelope delivery. Application/exact-generation coordination owns START/RETIRE semantics and prepared-generation state. MPEG owns decoder/worker behavior. Pi producer lifecycle remains deferred.

## State produced

Updated `docs/ledge/LEDGE_FOREMAN_STATE.md` to revision 0014 in commit:

`5e0bde1965f7ca2d464b6fecc67a50f313716beb` — `docs(foreman): correct A003 P2A control packet`

Revision 0014:

- explicitly supersedes the monolithic P2 packet from Foreman 0013;
- records that no product rollback was required;
- corrects START from historical MPEG DATA envelope to explicit control-plane representation;
- preserves RETIRE forensic identity;
- issues only bounded P2A to `interactive`;
- defers P2B Pi emission fence, P2C PS2 retirement/join/drain/credit fence, and P2D full N->RETIRE(N)->N+1 proof;
- does not authorize A004, Validation PASS, or hardware qualification.

## P2A acceptance packet issued

The active worker must prove:

1. START is explicit kind 11 / control channel 0 / flags 0 and is never recognized by sniffing MPEG media.
2. Proven START v1 payload semantics and immutable prepared-generation rules remain equivalent to H1 behavior.
3. RETIRE wire identity remains kind 10 / control channel 0 / flags 0 / exact 12-byte `>3I` tuple, without claiming cleanup/ack lifecycle yet.
4. MPEG DATA remains pure channel-4 media, including a legitimate exactly-44-byte media payload.
5. Exactly one PSTV connection and one physical receive owner remain intact.
6. Transport owns envelope/dispatch while exact-generation coordination owns lifecycle semantics.
7. No per-MPEG-packet generation tags or magic 44-byte classifier appear.
8. No Pi emission fence, PS2 retirement/join/drain fence, successor-generation transaction, presentation/deadline policy, A004 reconstruction, or hardware claim is introduced.

## Checks / evidence

No product source was modified in this Foreman round. This was a planning/authority correction, so no new runtime/build qualification is claimed.

Final pre-log branch authority was re-read as `5e0bde1965f7ca2d464b6fecc67a50f313716beb` with no intervening branch movement.

PENDING_LOCAL=A003-P2A Reconstruction handoff, Foreman integration/evidence after that handoff, and independent Validation; unrelated repository-governance evidence gaps remain separate
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification

## Exact next pickup

Interactive Reconstruction should re-read live branch authority and Foreman state revision 0014, execute only the bounded P2A packet, and emit exactly one immutable Reconstruction handoff with criterion-by-criterion `FOREMAN_GOAL_RESULT=MET|PARTIAL|BLOCKED`.
