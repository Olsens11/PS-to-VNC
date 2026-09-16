# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0013
RECORDED_AT=2026-09-16T18:52:38-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0034
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A002_AUDIT_REVISION=0001
BASED_ON_A003_AUDIT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision consumes the completed A003-P1 MPEG Transport/decoder-core Reconstruction handoff, directly reviews the product source/tests against M1-M8, performs only Foreman-owned canonical build/test/dictionary integration, reconciles concurrent Validation/Diagnostics/Continuity/Architecture support, and issues the next bounded exact-generation START/retirement packet from frozen H1 forensic authority.

The live branch was re-read immediately before this state write at `ba43004df44fb067bf9c0760a501c3a8644d52e3`. Unknown external/Pi-local dirty work remains outside this GitHub-native surface and is neither overwritten nor declared absent.

A001 machine/source PASS remains independent Validation authority while A001 physical PS2 qualification remains `HARDWARE_PENDING`. A002 physical audio/common-clock qualification remains separate. No statement below promotes A003 to independent Validation PASS or physical qualification.

## Current Foreman phase

`A003_MPEG_GENERATION_START_RETIREMENT_PACKET_ISSUED`

A003-P1 Transport MPEG plus decoder ownership/safe-stop reconstruction is Foreman-accepted as `MET`. No P1 product-source correction packet is required. The next dependency-ordered source responsibility is exact-generation START and retirement fencing across the existing ordered Transport stream and Pi producer lifecycle. Presentation/compositor/deadline policy remains outside this packet.

## Reconstruction result consumed — A003-P1

WORK_ITEM_KEY=`a003-mpeg-generation`
WORKER=`interactive`
WORKER_LOG_COMMIT=`8916f682c7c7d130ce72a3b680e12b2b3b921839`
WORKER_RESULT=`FOREMAN_GOAL_RESULT=MET`

Direct source/test review confirmed:

- MPEG2 DATA is logical channel 4 behind the existing sole physical Transport receive owner; no competing reader/socket/receive thread was introduced.
- MPEG queue, credit, activity, producer completion, and consumer status are independent of RFB/AUDIO and use explicit caller-owned bounded configuration rather than guessed production defaults.
- finite MPEG producer exhaustion remains distinct from owner stop/cancellation.
- the clean MPEG decoder owner has a narrow injected decoder/platform boundary, validates sequence/feed bounds, preserves payload-byte accounting, and owns decoder call/init/destroy ordering.
- local stop does not synthesize EOF from inside an active decoder data callback; stop retirement occurs only after decoder-call ownership returns.
- deterministic fixtures exercise Transport MPEG pressure/credit/activity/exhaustion and decoder preparation/call/destroy/safe-stop behavior.
- no presentation, common-clock arm, exact-generation START/retirement orchestration, MPEG per-packet generation tag, receive-poison repair, or hardware claim was introduced.

### M1-M8 Foreman disposition

M1 `MET` — MPEG2 is the third independent logical DATA path on the existing sole Transport receiver.

M2 `MET` — MPEG bounded storage/credit/activity/producer state is independent; credit is earned only for bytes actually removed.

M3 `MET` — the public MPEG consumer seam is activity-driven and distinguishes finite exhaustion from stop/closed/failure without exposing Transport internals.

M4 `MET` — the decoder owner establishes/uses the narrow decoder boundary with explicit caller-owned bounds and known-state preparation rather than a hidden H1 toggle/default.

M5 `MET` — sequence/feed bounds and actual payload-byte accounting are explicit and deterministic.

M6 `MET` — owner cancellation is not decoder EOF; active decoder calls continue to receive truthful Transport data/exhaustion/failure until the call boundary returns.

M7 `MET` — decoder acquisition/call/release ordering and unexpected-return/error propagation are covered without premature reclamation.

M8 `MET` — packet boundary discipline is preserved; exact-generation lifecycle and presentation remain deferred.

No product correction packet is required.

## Foreman-owned P1 canonical integration

Foreman integration commits on the live branch include:

- `3fb8aacec4e92daa11ac980f86ab1daf1946a7bb` — compile MPEG source in the strict clean PS2 inventory;
- `a2f907bbdca5f70b9146f2705a40fd63ea8291e0` — link MPEG Transport and decoder core into the clean executable graph;
- `f4427dec759673e1dcbbc2fcc9c1f2a4a66b4e75` — register MPEG Transport and decoder host fixtures and required runtime linkage;
- `a92081d253fc88e0d5dc57b5356d4452dcb66890` — admit/reconcile the MPEG source-dictionary domain;
- `4ef4b08c2a567a8ef2b8eeb6365a6e9871cd9df1` — stage MPEG dictionary reconciliation in canonical CI;
- `16565459ed8e757765f7b7609f9220b81e4106cd` — exact deterministic dictionary-reconciliation trigger;
- `a6a3a7e9ce5192ef6d60fb781c03df01c378adf5` — reconciled generated source-symbol portal after a concurrent branch movement caused the automated generated-child push to correctly reject a non-fast-forward update.

The portal collision was reconciled by carrying only the generated portal delta onto current authority; concurrent Diagnostics/Validation work was preserved. No product behavior was rewritten during that reconciliation.

## Canonical machine evidence and current CI caveat

Settled workflow run `35157582329` at `a6a3a7e9ce5192ef6d60fb781c03df01c378adf5` reports:

- `host-unit=SUCCESS`;
- `ps2-compile=SUCCESS`;
- `ps2-link=SUCCESS`, including current-source linked reproducibility;
- `dictionary-long=SUCCESS`;
- clean topology and source-dictionary portal synchronization `PASS` inside canonical project-check before the final work-log gate.

The same run's overall workflow conclusion is `failure` solely because canonical `project-check` reaches `WORK_LOG_CHECK=FAIL` on two already-frozen support logs carrying unsupported `LOG_FORMAT_REVISION=0005`:

- `20260916T160445-0400__integration__global-dictionary-prep__dictionary.md`;
- `20260916T162043-0400__validation__a003-mpeg-generation__validation.md`.

The governing contract for new records still uses `LOG_FORMAT_REVISION=0001`. Those immutable records are not rewritten here, and this governance/checker debt is not classified as an A003 product defect.

Fresh support authority consumed:

- Validation immutable log at `acdd65fdb75504cb4f9dbad04a4e8b16dc0af1c0` self-paused during active Foreman integration and explicitly made no independent A003 PASS/BLOCKED disposition; it reported no product-source failure.
- Diagnostics immutable log at `64780a6b8cf5895a9a0bd3e1a94ed436b68dea22` found product-specific host/compile/link/dictionary gates healthy while classifying the then-current overall red as generated/governance evidence rather than product behavior.
- Continuity advanced global reconciliation to revision 0034 and left successor A003 behavior to a fresh Foreman packet.
- Architecture scout `ba43004df44fb067bf9c0760a501c3a8644d52e3` is non-authoritative planning support only; its successor-boundary recommendation was independently checked against frozen H1 forensic evidence.

PENDING_LOCAL=independent Validation disposition for the settled integrated A003-P1 tranche; later packet integration/evidence; physical MPEG/video qualification
HARDWARE_PENDING=A001 physical PS2 qualification; A002 physical audio/common-clock qualification; A003 physical MPEG/video qualification

## Frozen forensic authority for A003-P2

Exact-generation lifecycle is reconstructed from frozen H1 authority at commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, especially:

- `experiments/media-harness-h1/CP2P_LIVE_GENERATION_BOUNDARY.md`;
- `experiments/media-harness-h1/CP2P_PI_START_RECEIVE_VALIDATION.md`;
- `experiments/media-harness-h1/CP2P_MPEG_SAFE_STOP_LIFECYCLE.md`;
- `experiments/media-harness-h1/CP2P_ITEM10_ALL_GUNS_ACTIVATION.md`;
- `experiments/media-harness-h1/h1_cp2p_retirement_control.py`;
- the corresponding H1 runtime implementation.

Forensic facts that are binding for this packet:

1. MPEG DATA packets do **not** carry per-packet generation tags.
2. START uses the existing H1 44-byte v1 channel-4 control payload and its exact historical validation rather than a newly invented wire shape.
3. RETIRE is control frame kind `10` on channel `0`, with an exact 12-byte big-endian `>3I` payload `(version=1, session_id, generation)`.
4. PS2 sends the RETIRE request. Pi returns the identical RETIRE payload only after exact-generation producer cleanup succeeds.
5. Pi retirement first closes exact-generation emission admission, allows any already-leased in-flight emission to finish, then cleans the exact producer, then acknowledges RETIRE.
6. PS2 accepts RETIRE completion only for its exact current generation, closes MPEG channel-4 admission, joins the exact decoder worker, discards residual queued MPEG bytes/returns their Transport credit, and only then permits a successor generation.
7. The safety proof is ordered same-stream fencing behind the existing sole Transport owner, not generation tagging of each DATA packet.
8. cancellation/abort remains distinct from normal RETIRE completion and finite producer EOF.

## Fresh interactive Reconstruction packet — A003-P2

WORK_ITEM_KEY=`a003-mpeg-generation`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`ba43004df44fb067bf9c0760a501c3a8644d52e3`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A003_MPEG_GENERATION.md:0001`
ASSIGNING_FOREMAN_STATE=`0013`

### Objective

Reconstruct exact-generation START and retirement lifecycle around the accepted P1 MPEG Transport/decoder core, preserving the historical ordered same-stream fence across PS2 and Pi. Make the exact session/generation lifecycle explicit, finite, testable, and restart-safe without adding per-MPEG-packet generation tags, a second Transport receive owner, guessed resource/timing defaults, or presentation behavior.

### Required deliverables

1. **Exact START control and validation.** Reconstruct the existing 44-byte v1 channel-4 START control payload and historical parser/validation exactly enough to establish the intended session/generation producer lifecycle. Do not invent a replacement wire format and do not tag every MPEG DATA packet with generation metadata.

2. **Exact RETIRE control.** Implement/own the existing control frame kind `10`, channel `0`, exact 12-byte big-endian `>3I` payload `(1, session_id, generation)`. PS2 sends the request; Pi returns the identical tuple only after exact-generation retirement succeeds. Malformed, wrong-version, wrong-session, stale-generation, or otherwise mismatched retirement messages must not count as completion.

3. **Pi exact-generation producer/emission fence.** On retirement, close admission for new emissions from that exact generation before producer cleanup. An already leased/in-flight emission may finish, but no new generation-owned emission may begin after admission closes. Cleanup the exact producer before sending RETIRE completion. Preserve first meaningful failure and finite stop/cleanup behavior; cancellation/error is not successful retirement.

4. **PS2 exact-generation MPEG retirement fence.** Accept retirement completion only for the exact current session/generation. Then close MPEG channel-4 admission, retire/join the exact decoder worker, discard residual MPEG bytes belonging to the retired stream while returning their Transport credit, and only after those fences complete allow successor generation admission. Never reclaim worker-visible resources before join/completion.

5. **Ordered same-stream restart safety.** Preserve the single ordered physical Transport stream and sole receive owner as the ordering authority. Prove that old-generation residual MPEG bytes cannot be reinterpreted as successor-generation payload. Do not solve this with per-DATA-packet generation tags or an auxiliary physical connection.

6. **Finite stop/error semantics.** Session cancel/abort remains distinct from normal finite producer EOF and normal RETIRE completion. Stop during producer activity, retirement wait, decoder retirement, or cleanup must converge finitely through owner-correct seams. Do not turn timeouts/mismatches into success; preserve the first meaningful error.

7. **Deterministic host coverage.** Cover at minimum:
   - exact START payload acceptance and rejection cases;
   - exact RETIRE frame kind/channel/12-byte tuple and identical completion tuple;
   - malformed/version/session/generation mismatch rejection;
   - an in-flight Pi emission lease delaying retirement completion until released;
   - no new Pi emission admission after retirement begins;
   - exact producer cleanup preceding RETIRE completion;
   - PS2 MPEG admission closing before decoder retirement/join/drain completion;
   - residual MPEG discard returning the corresponding Transport credit;
   - successor generation unable to start/accept MPEG before the full prior-generation fence completes;
   - cancellation/error remaining distinct from normal retirement/EOF;
   - repeated retirement/cleanup being finite and not double-reclaiming resources;
   - the sole Transport receiver invariant and accepted P1 Transport/decoder semantics remaining intact.

8. **Boundary discipline.** Preserve source synopsis/naming/dictionary/topology rules. Reconstruction may update source-side dictionaries/topology for new clean source. Foreman owns canonical test/build registration, generated portal reconciliation, and settled CI evidence. Do not add frame presentation, common-clock/video deadline/late-frame policy, per-packet generation tags, guessed queue/thread/timing defaults, receive-poison repair, or physical hardware qualification in this packet.

### Acceptance criteria

P2-1. START reuses the exact historical 44-byte v1 channel-4 control representation and exact lifecycle validation; no substitute protocol or per-DATA generation tag appears.

P2-2. RETIRE uses kind 10/channel 0/exact 12-byte `>3I` tuple and completes only on an identical exact-session/exact-generation acknowledgment after Pi cleanup.

P2-3. Pi retirement closes exact-generation emission admission before cleanup, permits only already-leased work to drain, and acknowledges only after exact producer cleanup.

P2-4. PS2 retirement verifies exact completion, closes MPEG admission, joins the exact decoder worker, drains/discards residual MPEG bytes with credit restoration, and only then reopens for a successor generation.

P2-5. Same-stream ordering plus the retirement fence prevents retired MPEG bytes from becoming successor payload without adding per-packet generation metadata or a second receiver.

P2-6. cancellation, finite EOF, retirement, mismatch, and failure remain distinct and finite; first meaningful errors survive cleanup.

P2-7. Deterministic host fixtures prove the ordering/failure/restart cases above and preserve accepted P1 behavior.

P2-8. No presentation/deadline policy, guessed production tuning, receive-poison repair, hardware claim, or unrelated application orchestration is introduced.

### Blocker burden and stretch rule

Hardware is not a blocker for this packet. Use injected producer/process/control/Transport/worker seams for deterministic host proof. Missing production tuning values are not blockers; keep such values explicit caller authority.

If the current clean Transport/control/Pi seams cannot express one required exact-generation fence, prove the precise limitation and add only the smallest owner-correct reusable public primitive. Do not reach around private Transport state and do not seize another physical receive owner.

Stretch only after P2-1 through P2-8 are complete: a narrow concrete Pi/PS2 adapter may be reconstructed where frozen authority proves it, but do not cross into presentation/compositor/deadline policy or hardware qualification.

## Next Foreman pickup

On the next Reconstruction handoff, Foreman must inspect the actual START/RETIRE control representation, Pi admission/lease/cleanup ordering, PS2 channel-admission/decoder-join/residual-credit fence, mismatch/cancel/error paths, and deterministic fixtures criterion-by-criterion P2-1 through P2-8 before any canonical integration. If coherent, Foreman will then re-read A003 audit/current source and choose the next bounded presentation/deadline/orchestration responsibility rather than assuming it in advance.

Independent Validation remains authoritative for Validation status. No Foreman or Reconstruction record may convert pending physical qualification into PASS.
