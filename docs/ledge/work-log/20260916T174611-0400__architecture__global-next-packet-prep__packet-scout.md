DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T17:46:11-04:00
COMPLETED_AT=2026-09-16T17:46:39-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=502df93f1b6c29f6bc85bddb9c6173098be7cf87
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Next-Packet / H1 Archaeology Scout

NON_AUTHORITATIVE_PENDING_FOREMAN=YES

## Authority observed

Live branch authority was refreshed at start and immediately before this write. Both reads resolved `ledge/h1-all-guns` to `502df93f1b6c29f6bc85bddb9c6173098be7cf87`, whose tip is the 17:29 Continuity reconciliation. Foreman state revision 0012 remains the latest governing packet authority observed and still assigns A003-P1: MPEG Transport logical-channel ingestion plus decoder ownership/safe-stop core. No newer Foreman packet was inferred from support or Continuity commits.

The active Reconstruction source has nevertheless advanced beyond the earlier scout snapshots. Commit `0ec6223d45a80871c8fdb0b1ccd77328779c15cd` closes the new `src/mpeg` source ownership documentation/dictionary/topology around `decoder.{c,h}`. The branch also contains CI/Continuity activity after that source work. Because no Foreman acceptance of A003-P1 was observed, this dossier remains conditional and does not authorize A003-P2.

## Evidence inspected

- current `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0012 and its A003-P1 M1-M8 contract;
- current `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision 0001;
- live branch history through `502df93f...`, including `0ec6223d...` (`a003: close MPEG decoder source ownership`), `3519df7b...` (Validation overlap pause), `79edfd19...` (CI sentinel pass), `0ca9420b...` / `502df93f...` Continuity reconciliation;
- current clean ownership description added for `src/mpeg/`, which explicitly excludes exact-generation START/retirement, Pi producer control, presentation/compositor, first-presentation clock arm, scheduler/drop, calibration, application orchestration and hardware qualification;
- A003 audit forensic evidence inventory: `experiments/media-harness-h1/h1_video_runtime.c`, `h1_config.h`, `CP2P_PI_START_RECEIVE_VALIDATION.md`, `CP2P_LIVE_GENERATION_BOUNDARY.md`, `CP2P_ITEM10_ALL_GUNS_ACTIVATION.md`, `CP2P_MPEG_EVENT_WAKE_HARDWARE_RESULT.md`, `CP2P_MPEG_SAFE_STOP_LIFECYCLE.md`, and `H1_INTEGRATION_INTENT.md`.

## Conditional next responsibility

If and only if the interactive Foreman accepts A003-P1 M1-M8, the next coherent responsibility is **A003-P2 exact-generation START / producer-admission / retirement lifecycle**.

The natural process boundary is:

`validate+freeze START(N)` -> `prepare exact generation` -> `open exact-generation producer emission admission` -> `MPEG DATA for N` -> `close producer admission` -> `drain any in-flight emission lease` -> `ordered retirement completion on the same TCP stream` -> `close PS2 MPEG admission for N` -> `request stop and join the exact decoder worker` -> `atomically discard residual N queue bytes and return withheld credit` -> `permit N+1`.

This is not decoder-core policy and should not be pushed down into `src/mpeg/decoder`. A003 audit authority assigns the high-level exact-generation transition to the application/process coordinator, Transport the channel-4 storage/credit/admission facts, MPEG the exact decoder-worker lifecycle, and the Pi producer side its exact-generation emission fence. Cross-component coordination should occur only through narrow process bridges.

## Historical H1 semantics that must survive

- START is exact session/exact generation state, not advisory metadata. Geometry is validated before producer activation. One generation may be prepared/active/retiring at a time; stale generation replacement is rejected.
- CONFIG authorizes capability only. Live activation is ordered: accepted profile -> qualified geometry -> generation worker arm -> START(N) -> Pi producer/capture/suppression preparation -> exact-generation emission admission -> MPEG DATA.
- The TCP byte stream itself is the generation epoch fence. H1 did not require generation tags on every MPEG DATA packet.
- Producer retirement is one-way. New emission admission closes first and an already-held send lease must finish before ordered retirement completion is emitted.
- PS2 closes generation-N MPEG admission on exact retirement completion before stopping/joining the exact decoder worker.
- Residual queued N bytes are discarded only after the consumer is proven stopped/joined. Those bytes are not decoder-consumed bytes, but their withheld Transport credit must be returned exactly once during finalization.
- Only after residual finalization may N+1 be prepared/admitted.
- Exact retirement/cancellation is not finite producer EOF. The existing P1 finite-producer fact must remain reserved for real exhaustion; owner stop must continue to obey the proven safe-stop boundary and must never manufacture callback EOF.
- DATA arriving after exact retirement completion is a protocol error, not a reason to silently reopen the retired generation.

## Current clean receiving owners

- `src/transport/`: retains sole physical PSTV receive ownership, channel-4 queue/credit/activity/finite-producer state, and should expose only the additional narrow generation-admission/finalization facts P2 truly requires. Do not create a second socket/receiver or expose queue internals.
- `src/mpeg/decoder.{c,h}`: synchronous decoder/resource/safe-stop core only. P2 may own a higher MPEG generation/worker lifecycle object, but must not contaminate decoder feed callbacks with generation cancellation or presentation policy.
- application/process layer: exact START identity and ordered cross-component transition owner. It coordinates; it must not import Transport or decoder internals.
- Pi producer/process side: prepared-generation identity plus begin/finish emission lease and one-way retiring latch around each MPEG send. The Foreman must decide the exact clean file/process placement from current Pi topology rather than copy H1 harness layout.

## Behaviors/scaffolding/defect boundary

KEEP: exact session+generation validation; geometry consistency; single prepared/active/retiring generation; ordered stream fence; emission-lease drain; exact retirement; worker stop/join before queue finalization; exact residual credit return; stale/post-retirement rejection; safe N->N+1 reopening.

DO NOT PROMOTE: H1 dynamic tuning knobs, diagnostic stage counters, broad coordinator globals, per-packet generation tags, timer polling, or harness-specific process layout.

KNOWN DEFECT TO PRESERVE AGAINST: local generation cancellation must never synthesize MPEG EOF inside an active decoder callback. P2 retirement must use P1's stop-after-decoder-call boundary rather than reinterpret retirement as producer exhaustion.

## Dependency/order hazards

1. Closing PS2 admission before the Pi emission fence is complete can turn an already-authorized in-flight send into a false protocol error.
2. Stopping/joining the decoder before admission closes allows late N data to repopulate the queue behind the retiring consumer.
3. Discarding residual queue bytes before exact decoder-worker join races consumer dequeue versus finalizer discard and corrupts credit accounting.
4. Reopening N+1 before residual discard/credit return allows generation aliasing in an untagged ordered stream.
5. Treating retirement completion as MEDIA_END/producer_done collapses cancellation into EOF and reopens the proven global-freeze defect class.
6. START success must not imply visual ownership or common-clock arm. A004 owns first physically presented frame, compositor ownership, scheduler/drop and clock-arm presentation semantics.

## Likely deterministic evidence

Host-testable P2 evidence should cover: exact START parse/geometry/session/generation rejection; duplicate/stale START; only-one-prepared generation; begin/finish emission lease with retire racing an in-flight send; retiring latch never reopening; DATA-after-retirement rejection; ordered retirement completion before PS2 finalization; exact worker stop/join; wrapped/nonempty residual queue discard with exact credit return; empty residual finalization; N->N+1 reopening only after finalization; finite producer EOF remaining distinct from cancellation; P1 false-EOF safe-stop regression; failure rollback at each START/preparation boundary.

Canonical PS2 build/link identity is machine evidence after Foreman integration. Hardware-only qualification remains repeated start/stop/relaunch generations, finite session completion, active all-guns endurance, and repeated sessions; repository tests cannot promote those claims.

## Natural packet split and stretch target

Recommended P2 core: exact START identity + prepared-generation state + producer emission fence + ordered retirement + PS2 admission close + exact worker stop/join + residual queue/credit finalization + N->N+1 reopen.

Meaningful stretch only if the core lands cleanly: deterministic rollback of partial START/preparation failures through the same state machine. Do not stretch into A004 presentation or broad application shutdown.

## Foreman decisions still required

- Exact clean application/process file that owns START(N) and cross-component generation transition.
- Exact Pi clean owner for prepared-generation/emission-lease state, based on current Pi topology rather than H1 harness names.
- Whether the current wire protocol already has an authoritative distinct retirement-completion representation sufficient for P2, or whether a narrow protocol representation must be reconstructed from forensic H1. Do not guess zero-length MPEG DATA as EOF/retirement.
- The smallest Transport bridge needed for generation admission close and atomic residual discard/credit return without leaking queue internals.

## Consume point

Interactive Foreman should consume this dossier only after the A003-P1 Reconstruction handoff exists and the Foreman independently verifies M1-M8. If P1 needs correction, hold this dossier and re-evaluate its assumptions against corrected source before issuing P2. If P1 is accepted, this dossier is intended to let the Foreman issue a bounded A003-P2 packet without repeating H1 lifecycle archaeology.
