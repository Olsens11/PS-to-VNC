DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T18:48:18-04:00
COMPLETED_AT=2026-09-16T18:49:08-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a305afd6d1a35fc634e7858228e890a7647ca03a
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Next-Packet / H1 Archaeology Scout — post-A003-P1 successor boundary

NON_AUTHORITATIVE_PENDING_FOREMAN

## Authority observed

The branch was read at `a305afd6d1a35fc634e7858228e890a7647ca03a` at shift start and re-read at the same HEAD immediately before this immutable create. Governing work-log contract revision 0005 and Foreman state revision 0012 remain the newest committed policy/planning snapshots. Foreman state 0012 still describes A003-P1 as issued, but newer immutable evidence has overtaken that progress statement: the A003-P1 Reconstruction handoff reports M1-M8 `MET`, subsequent Foreman-class integration chores materially converged, and Continuity revision 0034 records the tree between worker completion and formal Foreman acceptance. No newer Foreman state/acceptance packet was present at this wake.

Evidence inspected/reconciled this wake included current branch authority; `docs/ledge/work-log/README.md`; `docs/ledge/LEDGE_FOREMAN_STATE.md`; current Continuity/work-state evidence; the completed A003-P1 Reconstruction handoff `20260916T142305-0400__reconstruction__a003-mpeg-generation__interactive.md`; A003 audit revision 0001; A004 audit revision 0001; and the current clean MPEG/Transport ownership described by the completed worker/integration records. Earlier scout dossiers remain planning evidence only.

## Active tranche observed

A003-P1 is no longer an active worker implementation packet in temporal truth. Reconstruction completed its bounded MPEG Transport + decoder ownership/safe-stop objective and reported every M1-M8 criterion `MET`. Canonical compile/link/test/dictionary integration has subsequently converged, but Foreman has not yet published its independent acceptance/disposition or a successor worker packet. Therefore no Reconstruction worker is presently authorized by this scout to begin successor behavior.

## Exact conditional next coherent responsibility

If the interactive Foreman independently accepts A003-P1, the next coherent A003 behavior responsibility remains the exact-generation START / producer-admission / retirement lifecycle. The natural packet boundary is:

`START(N) validate/freeze -> prepare exact Pi generation -> open exact-generation emission admission -> MPEG DATA under one-way emission leases -> close producer admission and drain any in-flight lease -> ordered exact retirement completion -> close PS2 MPEG admission -> request stop and join the exact decoder worker -> atomically discard residual N queue bytes and return their withheld credit -> only then permit N+1`.

This is not per-packet generation tagging. H1's qualified design uses the ordered PSTV stream plus exact generation state as the epoch fence. Retirement/cancellation remains distinct from the real finite-producer completion/exhaustion fact reconstructed in P1.

## Historical H1 semantics that must survive

- START is exact session/generation state with validated geometry, not an advisory hint; only one generation is prepared/active/retiring.
- Producer emission uses a begin/finish lease and a one-way retiring latch. Closing admission must prevent a new lease while allowing an already-held lease to finish before retirement completion is emitted.
- Exact retirement completion is ordered behind the last admitted old-generation send on the same TCP stream.
- On the PS2, retirement completion closes channel-4 admission before decoder-owner retirement. MPEG DATA after that exact boundary is a protocol error.
- Local decoder stop still obeys P1's safe-stop invariant: it cannot synthesize EOF inside an active decoder callback; stop is honored only after decoder-call ownership returns.
- Exact worker join precedes residual queue destruction. Residual discarded bytes are not decoder-consumed bytes and their outstanding credit must be returned exactly once.
- N+1 cannot reopen until N's residual queue/credit state is finalized.

Experiment scaffolding that should not survive includes dynamic laboratory tuning knobs, per-packet generation tags invented for convenience, timer-backed MPEG empty polling, H1 stage witnesses/counters as correctness gates, or a new product toggle for decoder reset behavior.

## Current clean ownership / receiving seams

- Transport remains sole physical PSTV receive owner and owns channel-4 storage, credit, admission and terminal facts. It must not become the high-level generation coordinator.
- `src/mpeg/decoder.{c,h}` owns synchronous decoder acquisition/feed/call/destroy and the safe-stop boundary. It deliberately does not own exact-generation orchestration.
- A new/narrow application-level generation coordinator is the likely clean owner of START/retirement sequencing, calling Transport, MPEG and producer bridges rather than importing their internals.
- Pi producer-side exact-generation preparation/emission fencing belongs to the producer/transport process, not to the PS2 decoder.
- Presentation remains outside this packet. A004 audit proves that accepted START/preparation, queue prefill, decode and upload do not promote MPEG visual ownership and do not arm the common epoch.

## Dependency/order hazards

1. Producer admission must close and the in-flight lease must drain before ordered retirement completion; otherwise late N data can appear after the retirement fence.
2. PS2 MPEG admission must close on exact retirement completion before local owner teardown; otherwise late DATA can refill a retiring generation.
3. Decoder stop/join must finish before residual queue destruction; otherwise the callback/discard paths can race the same bytes.
4. Residual discard and exact credit return must complete before N+1 reopening; otherwise queue/credit state can leak across generations.
5. Cancellation/retirement must not be mapped onto P1's true producer EOF/exhaustion fact.
6. START acceptance must not imply visual ownership or clock arm. A004 owns the first-valid-physically-presented-frame promotion boundary.

## Behavior-specific evidence likely required

Host-testable: exact START identity/geometry validation; stale/repeat generation rejection; one-way prepared/active/retiring state; begin/finish emission lease races; close-versus-in-flight-send ordering; DATA-after-retirement rejection; stop during active decoder call preserving truthful callback behavior; exact worker join before queue finalization; wrapped/nonempty residual queue discard; exact credit return; failure rollback; N->N+1 reopening only after finalization; finite producer completion versus retirement distinction; bridge/dependency checks proving no second physical receiver and no component-internal reach-through.

PS2-build / hardware-only or hardware-required qualification: canonical PS2 compile/link identity for new hardware-facing orchestration; repeated START/retire/relaunch generations; finite completion; repeated decoder acquisition/release; all-guns active endurance; real producer/capture/suppression retirement races; proof that no reconstructed path reintroduces the historical global-freeze safe-stop defect.

## Natural packet split and stretch target

Recommended bounded packet: exact START + exact producer admission/retirement + PS2 generation-owner stop/join/finalization, ending at a safely reopenable N+1 boundary. Meaningful stretch target only if the base packet is already complete: deterministic multi-generation stress over many N->N+1 cycles with randomized close/send/stop interleavings. Do not stretch into first-frame presentation, compositor, RFB suppression rendering, common-clock arm, scheduler/drop policy or calibration.

## One-step-ahead A004 boundary

A004 audit revision 0001 confirms the next dependency family after A003 generation lifecycle: presentation/compositor/calibration. Its first critical boundary is accept-to-first-frame protection: RFB remains protected while generation preparation/decode advances, and only the first valid physically presented MPEG frame promotes MPEG visual ownership and arms the shared media epoch. Presentation then owns the single GS composition path, generation-specific suppression/matte footprint and absolute common-clock scheduling. This is deliberately recorded here to prevent A003-P2 from seizing A004 ownership, not to authorize A004 work.

## Foreman decisions still required

- Independently accept/partially accept/block A003-P1 M1-M8 on the settled integrated tree; this scout cannot promote it.
- Decide the exact clean location/name of the high-level generation coordinator and the narrow Pi producer bridge representation; H1 proves ordering semantics, not the final clean file/API names.
- Decide whether the successor packet includes both Pi producer fencing and PS2 exact-generation owner in one bounded round or splits them at the ordered retirement-message seam. If split, neither half may claim end-to-end retirement correctness without the other.
- Preserve START geometry as exact generation authority while leaving A004's final calibration/presentation transform ownership untouched.

## Exact Foreman consumption point

Consume this dossier after independently closing A003-P1 acceptance/integration and immediately before publishing the next worker-targeted Foreman packet. Re-check live HEAD and any newer Validation/Continuity evidence first. If P1 acceptance requires correction, that correction packet takes precedence and this successor dossier remains conditional.

No product source, tests, dictionaries, build files, audit dispositions, Validation findings, or Foreman state were modified by this scout.