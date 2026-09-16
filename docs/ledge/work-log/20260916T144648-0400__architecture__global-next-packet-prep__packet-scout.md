# Ledge Next-Packet / H1 Archaeology Scout — A003 exact-generation lifecycle

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T14:46:48-04:00
COMPLETED_AT=2026-09-16T14:51:00-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=3bd3689f24676a6823bcf317f131625e6ffb04ea
ENDING_BRANCH_COMMIT=f90f21e87fbb1a70d5ba469a460dc963fa50a2ed
SELF_PAUSED=NO

NON_AUTHORITATIVE_PENDING_FOREMAN=YES

## Authority observed

The branch moved during this scout from `3bd3689f24676a6823bcf317f131625e6ffb04ea` to `f90f21e87fbb1a70d5ba469a460dc963fa50a2ed` while the interactive A003 worker was actively adding/fixing MPEG Transport/decoder tests. Foreman state revision 0012 remains the current packet authority and defines `A003_MPEG_TRANSPORT_DECODER_CORE_PACKET_ISSUED`; this scout does not treat the moving worker tree as completed or accepted.

A002 has subsequently received independent machine/source PASS in the immutable Validation log at `c243fb53d7169b063c055de9eb9850c857c54367`; physical A001/A002 qualification remains separate.

Evidence inspected included current Foreman state 0012, A003 audit revision 0001, A004 audit revision 0001 for the next-tranche boundary, forensic H1 `experiments/media-harness-h1/h1_video_runtime.c`, `CP2P_PI_START_RECEIVE_VALIDATION.md`, `CP2P_LIVE_GENERATION_BOUNDARY.md`, current `src/transport/bridge.h`, current `src/mpeg/decoder.h`, current `src/mpeg/` inventory, and live recent A003 commits through `f90f21e8`.

## Active packet observed

The active packet remains A003-P1: sole-receiver MPEG Transport queue/event/credit semantics plus the clean synchronous decoder ownership/safe-stop core. Current source now exposes `pstvnc_transport_mpeg_*` read/status/activity/wait and `pstvnc_transport_mpeg_mark_producer_done()`, and `src/mpeg/decoder.{c,h}` explicitly excludes exact producer generations, worker orchestration, presentation, and clock arming. Recent worker commits prove this packet is still moving; no completion handoff was observed during this scout.

## Conditional next coherent responsibility

If and only if Foreman accepts A003-P1, the next coherent behavior responsibility is **A003 exact-generation START / producer-admission / retirement orchestration**, not A004 presentation yet.

The natural boundary is the generation state machine that turns accepted immutable geometry into one exact live producer epoch and then retires it without stale MPEG bytes crossing into the next generation. It should coordinate existing Transport and MPEG owners rather than move their internals into application code.

### Historical call/process chain that proves the semantics

1. PS2 emits fixed START v1 on existing PSTV channel 4: 44 bytes / eleven big-endian u32 words: version, session, generation, base x/y/w/h, suppression x/y/w/h.
2. Pi's existing sole `H1Session.reader()` chain recognizes START; no second socket/reader/thread is introduced. It validates exact session, nonzero/non-stale generation, 16-pixel-aligned base dimensions, containment/bounds, then freezes one immutable prepared generation. A second START cannot replace it while prepared.
3. Live activation later opens only the exact prepared generation's producer/capture/suppression state. Generation identity is an epoch/fence fact, not a tag copied onto every MPEG DATA frame.
4. Each Pi MPEG send must acquire/release an exact-generation emission lease. Retirement first closes new admission, then waits for an already-held send lease to finish before producer stop/drain.
5. Exact RETIRE completion is emitted only after Pi cleanup succeeds. Ordered TCP guarantees every generation-N DATA frame is before that completion on the same byte stream.
6. PS2 receiver accepts channel-4 DATA until exact RETIRE completion; accepting completion immediately closes further MPEG DATA admission. Any later channel-4 DATA is protocol error.
7. Application requests stop of exact N decoder worker and joins it. Only with producer sends fenced and consumer gone may Transport atomically discard residual N queue bytes, return withheld/batched credit, clear retirement state, and permit a fresh generation.
8. Residual-discard bytes are not decoder-consumed bytes. Owner cancellation is not finite producer EOF. `pstvnc_transport_mpeg_mark_producer_done()` must therefore be called only after a higher owner has actually proved finite producer completion/fence, exactly as its current bridge comment states.

Primary forensic authorities: `CP2P_PI_START_RECEIVE_VALIDATION.md` and `CP2P_LIVE_GENERATION_BOUNDARY.md`; A003 audit requirements 2-5 and exact-generation responsibility table agree with that later lifecycle evidence.

## Current clean receiving boundaries

- `src/transport/bridge.h`: Transport owns MPEG storage/activity/credit/finite-producer facts and deliberately does not own generation policy. Current public MPEG bridge already provides bounded read/status/activity/wait and the narrow producer-done publication seam.
- `src/mpeg/decoder.{c,h}`: MPEG owns synchronous decoder acquisition/feed/call/release and safe-stop boundary; header explicitly excludes worker/exact-generation/application orchestration.
- Application/top-level process should own exact-generation transition sequencing through narrow component bridges, per A003 audit. Do not make Transport decide decoder cancellation and do not make MPEG reach into queue/credit internals.
- Pi producer/capture/suppression owns prepared generation and exact send-admission fence on the Pi side. The clean representation/path must be chosen from current repository authority rather than guessed from H1 wrapper names.

## Behaviors to preserve

- one physical PSTV connection and sole receive owner in each direction;
- immutable exact session/generation + base/suppression geometry after START acceptance;
- monotonic stale-generation rejection and only one prepared/active/retiring generation;
- one-way retiring latch; retirement never reopens the same generation;
- close new emission before waiting for in-flight lease;
- ordered completion fence on the same TCP stream;
- close PS2 channel-4 admission at exact retirement completion;
- stop/join exact MPEG worker before residual queue finalization;
- atomically discard residual bytes and return all owed credit before fresh generation opens;
- distinguish decoder-consumed bytes, discarded residual bytes, finite producer completion, and owner cancellation.

## Scaffolding not to preserve

Do not reproduce H1's scoreboard/public-gate staging, experiment evidence JSON/print witnesses, dynamic laboratory tuning surface, source-wrapper/adapter layering, per-packet generation tags, second queues/sockets/readers, or diagnostics as correctness authority. `VIDEO_IPU_RESET_EACH_SESSION`, timer-backed MPEG empty polling, scheduler comparison modes, and presentation stage markers remain outside this packet by existing audit disposition.

## Known defects / hazards

- Never convert owner stop into synthetic decoder EOF; A003-P1 owns the proven safe-stop rule and the generation coordinator must use that public stop/join boundary rather than bypass it.
- Do not mark producer-done merely because retirement was requested. Finite exhaustion and cancellation are distinct.
- Do not discard residual MPEG queue bytes before the decoder worker is joined; that can race an active libmpeg callback.
- Do not reopen generation N or N+1 until old queued bytes and pending credit are fully finalized.
- Do not restore RFB visual presentation or remove suppression merely because local decoder stop completed. A004 proves restoration is later than exact Pi suppression/capture retirement plus local owner retirement.
- START acceptance does not promote MPEG visual ownership and must not arm the common media clock.

## Behavior-specific evidence likely needed

Host-testable:
- exact 44-byte START parse/serialize and version/session/generation/geometry rejection;
- stale/repeat/prepared-generation rejection;
- one prepared/active/retiring generation state machine;
- emission lease close versus in-flight send race and one-way retirement latch;
- ordered completion acceptance closes PS2 MPEG admission;
- DATA after retirement completion fails closed;
- worker stop/join precedes residual discard;
- wrapped residual queue discard returns exact queued + pending/batched credit while not incrementing decoder-consumed accounting;
- fresh generation cannot open with residual bytes/credit/retirement state;
- START-send failure aborts the just-opened generation cleanly;
- finite producer completion publication is distinct from cancellation/retirement;
- failure paths preserve first-cause Transport/application result.

PS2-build/static evidence:
- canonical compile/link of generation coordinator plus current Transport/MPEG bridge symbols;
- no second receive owner/socket and no forbidden cross-component internal includes;
- linked identity/PT_LOAD/current-source reproducibility.

Hardware-only later qualification:
- repeated exact start/stop/relaunch generations;
- finite session completion;
- retirement during active decode/send;
- stale DATA rejection/no bleed into next generation;
- repeated-session and all-guns endurance. Repository tests must not claim these physical results.

## Natural packet split and stretch target

Recommended next packet: exact-generation control/state and retirement on the PS2/application side plus the narrow Pi prepared-generation/emission-fence counterpart needed to make the protocol state machine coherent and host-testable. Stop before GS presentation/compositor/calibration ownership.

A meaningful stretch target, only if the core state machine lands cleanly, is deterministic repeated N -> retire -> N+1 reopen coverage including residual wrapped-queue credit reconciliation. Do not stretch into first-frame ownership/media-clock arm or RFB suppression rendering; those are A004 responsibilities even though generation-specific suppression installation/retirement proof is an A003 lifecycle dependency.

## Foreman decisions / unresolved authority

1. Choose the clean Pi producer/capture/suppression module boundary from current source. H1 proves semantics but its experiment class/file organization is not product architecture.
2. Decide whether A003 next packet includes the full Pi suppression/capture installation implementation or only an injected producer-generation bridge with exact lifecycle semantics. The audit requires exact Pi generation state before emission, but A004 owns visual suppression/presentation policy.
3. Define the clean wire representation for retirement completion from current protocol authority. Do not invent a new frame kind if existing framing already proves the representation.
4. Decide the application worker owner/bridge shape for the synchronous `pstvnc_mpeg_decoder_run()` core after A003-P1 acceptance. The current decoder intentionally does not own a worker.
5. Preserve A004 boundary: first physically presented MPEG frame, visual-owner promotion, common-clock arm, shared GS compositor, RFB freeze/full-refresh policy, and scheduler/drop presentation remain later work.

## Exact consumption point

Foreman should consume this dossier only after the active A003-P1 Reconstruction worker leaves its immutable completion handoff and Foreman has independently inspected/accepted or corrected that packet. Revalidate all current Transport/MPEG bridge names and branch HEAD before issuing authority. If P1 changes its public seams or returns PARTIAL/BLOCKED, adapt this dossier rather than forcing it.
