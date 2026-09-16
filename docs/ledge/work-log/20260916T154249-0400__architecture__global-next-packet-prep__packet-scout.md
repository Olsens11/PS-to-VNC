DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T15:42:49-04:00
COMPLETED_AT=2026-09-16T15:48:30-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=501de013191c0156f29d315a71b079322d959515
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Next-Packet / H1 Archaeology Scout — A003 exact-generation lifecycle

NON_AUTHORITATIVE_PENDING_FOREMAN=YES

## Authority observed

Live branch authority was refreshed at start and immediately before this immutable write. Both reads resolved to `501de013191c0156f29d315a71b079322d959515`.

Current Foreman state remains revision 0012 with active packet `A003-P1`: Transport-owned MPEG logical-channel ingestion plus synchronous decoder ownership/safe-stop core. Continuity revision 0031 confirms no immutable A003 Reconstruction completion handoff exists yet; A003 therefore remains RECONSTRUCTING and this dossier does not authorize successor work.

Recent Diagnostics/Validation movement was consumed only as state evidence. Intermediate CI red is mechanical integration/topology/dictionary/link-graph debt while P1 is moving, not authority to reinterpret P1 behavior.

Evidence inspected included current `docs/ledge/LEDGE_FOREMAN_STATE.md`, `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md`, current `src/transport/bridge.h`, current `src/mpeg/decoder.h`, current branch history/Continuity log, forensic H1 `CP2P_LIVE_GENERATION_BOUNDARY.md`, `CP2P_PI_START_RECEIVE_VALIDATION.md`, and the H1 media-harness evidence inventory at audit source authority `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

## Conditional exact next responsibility

If and only if the interactive Reconstruction worker returns A003-P1 and the Foreman accepts/integrates M1-M8, the next coherent behavior responsibility is:

**A003-P2 — exact-generation START/prepared-generation admission and ordered retirement/finalization lifecycle.**

This should be one process-oriented packet spanning the minimum PS2 application/MPEG/Transport bridge state plus the Pi producer-generation seam needed to prove a real generation fence. It must not absorb A004 presentation/compositor/calibration behavior.

The clean P1 boundary already makes this split natural: `src/mpeg/decoder.h` explicitly excludes worker ownership, exact producer generations, application orchestration, GS/compositor presentation, common-clock arming, scheduler/drop/calibration, while `src/transport/bridge.h` exposes MPEG consumption and a higher-owner `pstvnc_transport_mpeg_mark_producer_done()` fact without deciding generation policy.

## Concrete H1 call/process chain to preserve

### 1. START is exact immutable generation state

Forensic `CP2P_PI_START_RECEIVE_VALIDATION.md` proves the Pi receives START on the existing physical PSTV session through the existing sole receive chain. START v1 is a 44-byte channel-4 DATA control payload containing version, exact session id, nonzero generation, base/draw rectangle and suppression rectangle. Validation rejects wrong size/version/session, stale or zero generation, invalid signed geometry, unaligned/subminimum base dimensions, containment failures, and desktop-bound violations. Successful validation freezes one prepared generation; a second START is rejected until the exact prepared generation is released, and an already prepared generation or older generation cannot later replace it.

Clean implication: do not turn START into an advisory flag and do not create a second physical reader. Representation may be rewritten cleanly, but exact session/generation/geometry identity and stale-generation rejection survive.

### 2. Generation admission is a producer-side exact-generation lease fence

Forensic `CP2P_LIVE_GENERATION_BOUNDARY.md` proves the one ordered TCP stream is itself the epoch fence; H1 deliberately avoided generation tags on every MPEG DATA frame. The Pi producer emission fence begins closed. The later producer/scheduler opens emission only for the current prepared generation, and every MPEG send must acquire/release an exact-generation emission lease around the PSTV write.

Clean implication: application/process coordination may authorize the current generation, but Transport remains sole physical owner and MPEG DATA stays ordinary channel-4 DATA. Do not add per-packet generation fields or a competing MPEG socket/thread merely to simplify retirement.

### 3. Retirement closes new admission before producer teardown

H1 retirement first latches the generation one-way RETIRING, closes new scheduler/emission admission, and waits for any send already holding a lease to finish before producer stop/drain. Only after Pi cleanup succeeds is exact RETIRE completion emitted. Because completion and all prior DATA share ordered TCP, every generation-N DATA frame is earlier than exact retirement completion on the PS2 receive stream.

Hazard: closing producer process first but leaving emission admission open permits a late N send to cross the boundary. Conversely, treating local decoder stop as producer EOF recreates the defect boundary P1 is explicitly preventing.

### 4. PS2 retirement completion closes channel-4 admission, not decoder EOF

The sole PS2 receiver continues accepting N DATA until exact RETIRE completion is observed. Acceptance of that completion immediately closes MPEG DATA admission; any later channel-4 DATA is protocol error. The retirement latch remains active while the exact N decoder worker is stopped/joined.

This is distinct from finite producer completion. `pstvnc_transport_mpeg_mark_producer_done()` should only be used when a higher owner has proven real finite producer completion. Retirement/cancellation must not manufacture `producer_done` merely to wake/finish the decoder.

### 5. Exact worker retirement precedes residual queue finalization

After exact retirement completion, application stops and joins the exact N worker at the safe P1 decoder boundary. Only with producer sends fenced and the consumer no longer reading may Transport finalization atomically discard residual N bytes under MPEG queue authority, return all withheld credit (batched credit plus discarded queued bytes), and clear the retirement latch.

Residual discarded bytes are not decoder-consumed bytes and must not increment decode-consumption accounting.

### 6. N+1 opens only after N is fully finalized

Fresh generation admission requires no prior active/retiring generation, empty MPEG ring, no pending old-generation credit, and released exact prepared-generation state. This is the critical stale-byte prevention invariant.

## Current clean owners / likely receiving seams

- **Application/process owner (`src/app.*` or a narrowly split application-owned generation coordinator):** exact high-level transition sequencing only. It coordinates through bridges; it must not import Transport queue internals or MPEG decoder internals.
- **Transport (`src/transport/*`, public bridge):** channel-4 admission state, bounded queue, exact retirement admission close, atomic residual discard and owed-credit return. Preserve sole receiver ownership.
- **MPEG (`src/mpeg/*`):** exact decoder worker/session wrapper around the already synchronous decoder core, request-stop/join/retire proof, but not presentation policy.
- **Pi producer bridge/process:** immutable prepared START generation, exact-generation emission lease/open/close/retire state, producer cleanup and exact retirement completion. Do not preserve H1 experiment runner/scoreboard/evidence plumbing as product architecture.
- **Presentation:** intentionally not in P2 except as a future dependency. First real presentation remains the owner of common-clock arm.

## Behavior to keep vs scaffolding to discard

KEEP:
- exact session/generation identity and stale/repeat rejection;
- validated immutable START geometry;
- one prepared/active/retiring generation at a time;
- one-way retiring latch;
- exact-generation send leases;
- ordered TCP retirement fence;
- close PS2 MPEG admission at exact retirement completion;
- stop/join exact worker before residual discard;
- atomic residual discard plus exact outstanding-credit return;
- no N+1 until ring/credit/retirement state is clean;
- finite producer EOF distinct from cancellation/retirement.

DO NOT PROMOTE:
- H1 scoreboard/item numbering, evidence JSON/console witnesses, experiment runner plumbing, public-gate staging vocabulary, dynamic laboratory tuning knobs, per-packet generation tagging, second socket/reader, or timer-backed MPEG polling.

KNOWN DEFECT BOUNDARY:
- P2 must preserve P1's safe-stop rule: owner stop cannot cause an active libmpeg feed callback to synthesize EOF. Retirement requests stop at a project-code boundary and waits for decoder-call ownership to return.

## Dependency/order hazards

1. START must not activate producer emission before PS2-side generation/worker readiness is established.
2. A retiring generation must never reopen emission admission.
3. Exact retirement completion cannot be sent while an old send lease remains in flight.
4. PS2 admission close must happen on the ordered retirement completion, before worker retirement/final queue cleanup.
5. Queue discard before worker join races the decoder consumer; worker join before producer fence permits late producer bytes. Required order is producer fence -> ordered completion/admission close -> exact worker stop/join -> residual discard/credit return.
6. Residual discard credit must include queued bytes plus already batched-but-unsent credit exactly once; avoid double return or stranded credit.
7. START-send failure needs an abort/release path that cannot leave a phantom active generation.
8. Do not let generation lifecycle arm the common media epoch. Audit authority reserves arm for first real presentation in A004.
9. Do not let suppression/capture geometry ownership pull A004 compositor/calibration behavior into P2; P2 validates/freezes exact geometry identity only.

## Suggested deterministic evidence

Host-testable:
- START parse/validation: exact size/version/session/generation/geometry; stale/repeat/zero rejection;
- only one immutable prepared generation;
- generation open failure / START-send failure abort leaves no active phantom;
- exact emission lease acquisition only for current generation;
- retirement latch is one-way and rejects reopen/new leases;
- close-vs-in-flight-send interleaving waits for lease return before completion;
- exact retirement completion closes PS2 MPEG admission; later MPEG DATA is protocol error;
- retirement does not set finite `producer_done`;
- exact worker stop/join precedes queue finalization;
- wrapped residual queue discard returns exactly discarded bytes plus pending credit once;
- N+1 rejected until N worker/ring/credit/retirement/prepared state is fully clear, then accepted after finalization;
- decoder-consumed accounting excludes residual discard;
- safe-stop callback regression from P1 remains green;
- sole physical Transport receiver invariant remains green.

Canonical PS2 build evidence:
- all new generation coordinator/Transport/MPEG source compiles under pinned PS2 toolchain and links in clean executable graph;
- linked reproducibility/identity remains canonical Foreman work after worker handoff.

Hardware-only / later qualification:
- repeated START/retire/reopen generations on PS2;
- retirement while MPEG DATA is actively flowing;
- finite session completion;
- repeated decoder start/stop/relaunch and all-guns endurance.
Repository evidence must not claim these qualified.

## Natural packet split and stretch target

Recommended P2 core: exact START/prepared-generation state + emission admission/lease fence + PS2 retirement/admission-close + exact worker stop/join + residual queue/credit finalization + reopen proof.

Meaningful stretch target only if the core closes cleanly: wire the real Pi prepared-generation seam through the exact emission lease state machine sufficiently to prove ordered retirement end-to-end in deterministic host/process fixtures. Do not stretch into RFB suppression/capture activation, GS composition, first-presentation clock arm, scheduler/drop policy, or calibration.

If Pi-side production architecture cannot be modified in the same bounded worker packet without violating repository worker/tool ownership, the Foreman should split P2 at the bridge contract: first reconstruct/test the exact PS2 generation coordinator and Transport finalization plus a fake producer-generation bridge; then a separate bounded Pi producer implementation packet. Do not guess across that tool/ownership boundary.

## Foreman decisions still required

1. Exact clean START/RETIRE control representation: H1 proves semantics and ordered-stream requirements, but the Foreman must decide whether the clean protocol adopts the historical 44-byte START v1/control representation or a narrower rewritten process representation. Do not infer zero-length MPEG DATA as EOF/control.
2. Location/name of the high-level generation coordinator: application owns the process semantically, but Foreman should decide whether it belongs directly in `src/app.*` or a narrow application-owned generation module to avoid bloating existing app orchestration.
3. Whether P2 is one cross-host packet or split at a producer bridge. Preserve the same lifecycle order either way.
4. Exact finite-producer completion representation remains separate from retirement. If current clean wire authority still lacks a proven MEDIA_END representation, do not invent one; leave `mark_producer_done` to a later proven producer completion fact.
5. Presentation ownership remains deferred: P2 must carry validated geometry identity forward without deciding draw/suppression/calibration policy that A004 owns.

## Exact Foreman consumption point

Consume this dossier only after an immutable A003-P1 Reconstruction handoff exists and the Foreman has independently accepted/integrated P1 M1-M8. If P1 returns PARTIAL/BLOCKED or materially changes Transport/MPEG bridge ownership, re-derive P2 against that accepted source before issuing authority.

PENDING_LOCAL=A003-P1 immutable worker handoff; Foreman M1-M8 acceptance/integration; only then A003-P2 packet authority
HARDWARE_PENDING=A001 physical qualification; A002 audio/common-clock qualification; later A003 repeated-generation/finite-session/all-guns hardware qualification
