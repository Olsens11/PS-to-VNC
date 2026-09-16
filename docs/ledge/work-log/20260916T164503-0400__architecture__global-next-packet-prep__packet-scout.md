DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T16:45:03-04:00
COMPLETED_AT=2026-09-16T16:45:40-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=1cb50c5dc8418ce5cd642ac62e4f9e49be6b709c
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Next-packet / H1 archaeology scout

## Authority consumed

Branch `ledge/h1-all-guns` was read at `1cb50c5dc8418ce5cd642ac62e4f9e49be6b709c` at shift start and re-read at the same authority immediately before this immutable write.

Current Foreman authority remains `LEDGE_FOREMAN_STATE` revision 0012. Its active packet is A003-P1: Transport MPEG logical-channel ingestion plus decoder ownership/safe-stop core. Current Continuity revision 0032 still records A003-P1 as active with no immutable Reconstruction completion handoff; the newest Validation wake self-paused for that reason. A001 and A002 remain machine/source PASS; A003 is not yet Validation-ready. The newest CI sentinel evidence cited by Continuity reports intermediate MPEG canonical-linkage/topology/dictionary debt rather than an established product-behavior defect.

Governing work-log contract revision 0005 was read. Historical `LEDGE_RECONSTRUCTION_STATE` revision 0007 was also checked and is explicitly stale A001-era lane state, not current A003 authority.

Evidence inspected this wake included current branch history through `1cb50c5d`, Foreman state 0012, Continuity reconciliation commit `360d2805`, work-log contract revision 0005, historical reconstruction state revision 0007 for staleness classification, and `LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` revision 0001 to sharpen the downstream ownership boundary.

## Conditional next responsibility

`NON_AUTHORITATIVE_PENDING_FOREMAN`

Because A003-P1 has not yet returned an immutable completion handoff, the next packet remains conditional. If the Foreman accepts P1 M1-M8, the exact next coherent behavior responsibility is still A003-P2: exact-generation START / producer-admission / retirement lifecycle. This wake does not issue that packet.

The required ordered lifecycle remains:

1. validate and freeze one exact START generation and its generation-scoped geometry/profile facts;
2. establish the PS2-side readiness/admission state for that exact generation before producer emission becomes live;
3. admit Pi sends only through exact-generation emission authority/leases;
4. on retirement, close producer admission first and drain any in-flight exact-generation send;
5. emit/observe the ordered retirement completion fence for that exact generation;
6. close PS2 MPEG admission for N;
7. request stop and join the exact local decoder worker without converting stop into finite producer EOF;
8. only after consumer ownership has retired, discard residual generation-N MPEG bytes and return the exact outstanding Transport credit;
9. reopen admission for N+1 only after all N retirement fences are proven.

This ordering preserves the H1 design in which the ordered TCP stream acts as the epoch fence rather than adding generation tags to every MPEG DATA packet. It also preserves the P1 safe-stop invariant: retirement/cancellation is not producer EOF, and local stop must not synthesize EOF inside an active decoder callback.

## Sharpened downstream boundary from A004

A004 audit revision 0001 makes the P2 stopping point especially important. A003-P2 may carry exact generation identity plus the accepted base/suppression geometry needed to prepare capture/emission, but it must not absorb presentation ownership.

The following belong to A004 and must remain outside A003-P2:

- accepted calibration does not itself transfer visual ownership to MPEG;
- RFB remains protected during the accept-to-first-physically-presented-frame gap;
- the first valid physically presented MPEG frame is the visual-ownership promotion boundary and the common-media-epoch arm boundary;
- suppression is a presentation ownership rule over the generation-specific outer footprint, not a second framebuffer truth;
- one shared GS compositor remains the sole physical presentation owner;
- absolute common-clock video scheduling and optional lateness drop are presentation policy, not decoder/generation policy;
- RFB restoration after MPEG retirement must wait for exact Pi generation retirement plus local MPEG owner retirement, then schedule exactly one fresh full RFB request.

This gives the Foreman a clean packet boundary: P2 should make exact generation N safely startable and safely retireable, while A004 later decides when decoded N becomes physically visible and when RFB presentation ownership changes.

## Current clean receiving owners / likely seams

Current P1 source establishes Transport as sole physical receive owner and `src/mpeg/decoder` as synchronous decode/feed/resource owner. A003-P2 therefore needs a higher generation/session coordinator rather than moving generation authority into Transport or the decoder. Transport should continue to expose only MPEG channel state/read/activity/finite-producer facts; decoder should continue to own decode-call lifetime and safe stop. Application-level orchestration should not be broadened beyond the minimum generation coordinator unless the current source/Foreman packet proves that ownership belongs there.

The Pi producer side must own exact-generation capture/emission admission and retirement proof. The PS2-side generation owner must own exact START identity, local worker lifetime, residual-queue retirement, and the N -> N+1 fence. Presentation consumes the resulting exact generation later through its own bridge.

## Behavior that must survive

- one exact frozen generation at a time; stale/replacement START is rejected rather than silently replacing live authority;
- producer emission is generation-fenced before DATA is admitted;
- retirement closes producer admission before the ordered retirement proof;
- local decoder worker stop/join precedes residual queue reclamation;
- residual old-generation bytes cannot leak into N+1 and their withheld credit is returned exactly once;
- cancellation/retirement remains distinct from true finite producer completion;
- no per-packet generation tagging is invented when ordered-stream fencing is sufficient;
- no first-presentation/common-clock arm is moved into generation start/decode readiness.

## Experiment scaffolding not to preserve

Do not promote H1 sweep knobs, scheduler-comparison modes, stage markers/holds, or diagnostic witnesses into production generation configuration. A004 explicitly classifies scheduler comparison and stage machinery as laboratory/diagnostic surfaces. Do not reproduce adapter proliferation merely because H1 acquired these seams incrementally.

## Dependency/order hazards

The highest-risk race remains producer-fence -> ordered retirement completion -> PS2 admission close -> exact worker stop/join -> residual discard/credit return. Closing or discarding on the PS2 before the producer fence can admit late N bytes; discarding while the decoder still owns the consumer path races the active callback; reopening N+1 before residual-credit settlement permits cross-generation contamination or credit skew.

A second boundary hazard is premature presentation promotion. START accepted, producer admitted, queue prefilled, decoder initialized, or a decoded/upload-ready surface are all insufficient to claim MPEG visual ownership or arm the common clock. A004 reserves that authority for the first valid physically presented frame.

## Likely deterministic evidence

Host-testable P2 evidence should cover exact START validation/freeze, stale/replacement generation rejection, producer lease/admission close races, retirement while a send is in flight, exact retirement acknowledgement/fence ordering, local worker stop/join, wrapped residual queue discard with exact credit return, N -> N+1 reopening only after complete retirement, START failure rollback, and regression of P1's no-false-EOF safe-stop behavior.

PS2-build evidence should prove the coordinator and any new platform-facing generation seams compile/link in the canonical graph without creating a second Transport or graphics owner. Physical hardware evidence remains later: repeated exact-generation start/retire behavior, real producer/decoder convergence, and ultimately A004 visual ownership/timing/endurance cannot be qualified by repository evidence alone.

## Natural packet split / stretch target

Natural packet: exact START identity + producer admission/emission fence + ordered retirement + PS2 worker retirement + residual queue/credit reset + N -> N+1 reopening.

Meaningful stretch target, only if the natural packet is already coherent: deterministic repeated N -> N+1 -> N+2 lifecycle tests including failure rollback. Do not stretch into first-frame presentation, RFB suppression/compositor policy, common-clock arm, calibration, or scheduler/drop policy.

## Foreman decisions still required

1. Choose the clean generation/session coordinator location after inspecting the final accepted P1 source; do not bury it in Transport or decoder for convenience.
2. Confirm the exact clean representation of START and retirement completion from current protocol/Pi authority before implementation; do not infer a zero-length DATA pseudo-control.
3. Decide the minimum geometry/profile facts A003-P2 must freeze for exact producer preparation while leaving all presentation ownership and draw policy to A004.
4. Decide whether producer-side Pi changes belong in the same bounded P2 packet or require a paired subpacket while preserving one exact cross-side retirement proof.

## Consumption point

The interactive Foreman should consume this dossier only after an immutable A003-P1 Reconstruction handoff exists and after independently checking P1 M1-M8. If P1 requires correction, this dossier remains planning evidence only and must be refreshed against the corrected source before packet issuance.

PENDING_LOCAL=final A003-P1 Reconstruction handoff; Foreman M1-M8 acceptance/integration; exact P2 source ownership decision; canonical machine evidence after implementation
HARDWARE_PENDING=A001/A002 physical qualification remains outstanding; A003/A004 reconstructed hardware-facing behavior has no physical qualification claim

## Exact next pickup

On the next scout wake, refresh branch authority. If P1 has returned, inspect the accepted final P1 source plus Foreman disposition and convert this dossier into source-exact P2 call-chain/preflight evidence. If P1 is still active, do not invent new authority; only deepen unresolved START/retirement representation or Pi-side exact-generation evidence where current repository history supports it.