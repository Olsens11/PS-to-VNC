# Ledge Next-Packet / H1 Archaeology Scout — A003 conditional preflight

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T13:47:04-04:00
COMPLETED_AT=2026-09-16T13:54:00-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=4dca53b58db338d9ecfe600cd6ed751041e2af37
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Authority and concurrency

This seat is read-mostly planning support only. It does not issue Foreman authority, implement behavior, alter dictionaries/tests/build files, or perform independent Validation.

Live branch authority was captured at `4dca53b58db338d9ecfe600cd6ed751041e2af37` (`test(a002): prove audio worker lifecycle semantics`) and re-read immediately before this immutable write; it remained unchanged. Current Foreman state revision 0011 still has `A002_AUDIO_WORKER_CLOCK_PACKET_ISSUED`. The live branch has advanced inside that active packet through `b5e020bc7b93d9816a09196b58c1bcdd1c571f27` (`feat(a002): implement audio worker lifecycle owner`) and `4dca53b5...` (deterministic lifecycle fixture), but no completed immutable Reconstruction handoff was present at the observed boundary. Therefore everything below is `NON_AUTHORITATIVE_PENDING_FOREMAN` and must be consumed only after the active A002 worker packet returns and Foreman settles it.

Semantic-audit state revision 0007 remains `SEMANTIC_AUDIT_COMPLETE`; A001-A006 are already reconstruction-ready and there is no new seeded audit family to wait for.

## Evidence inspected

Current authority / moving A002 packet:

- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0011;
- `docs/ledge/LEDGE_AUDIT_STATE.md` revision 0007;
- current commits `b5e020bc7b93d9816a09196b58c1bcdd1c571f27` and `4dca53b58db338d9ecfe600cd6ed751041e2af37`;
- current clean `src/` topology, which has CONFIG, media clock, audio and Transport owners but no clean MPEG owner yet.

Next dependency audit / forensic H1:

- `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision 0001;
- forensic H1 `experiments/media-harness-h1/h1_video_runtime.c` at `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- `experiments/media-harness-h1/CP2P_LIVE_GENERATION_BOUNDARY.md`;
- `experiments/media-harness-h1/CP2P_MPEG_SAFE_STOP_LIFECYCLE.md`;
- A003's cited CP2P START/event-wake/all-guns evidence as summarized by the audit.

## Conditional next responsibility dossier

`NON_AUTHORITATIVE_PENDING_FOREMAN`

If Foreman accepts the active A002 worker/resource/clock packet and determines no final bounded A002 application-composition packet is required, the next dependency-ordered tranche is A003. The safest first A003 packet is **MPEG logical-channel consumption + decoder ownership/safe-stop core**, stopping before exact-generation application/Pi orchestration and before A004 presentation/compositor policy.

This split is preferable to beginning with the whole A003 lifecycle because the current clean tree has no MPEG component yet, while A001 already established sole Transport receive ownership and A002 established the pattern of narrow media owners consuming public logical-channel seams. The first packet can establish one clean MPEG owner and its decoder-call boundary without simultaneously inventing the application-level generation coordinator.

### Historical call chain that proves the decoder-side semantics

H1 `pstvnc_h1_video_run_session()` constructs one session, allocates aligned picture/feed resources, establishes renderer/decoder environment, performs the qualified IPU/DMAC preparation, calls `MPEG_Initialize(h1_video_feed_ipu, ...)`, selects RGB16 with `_MPEG_Set16(1)`, waits for bounded startup prefill, then enters `MPEG_Picture()` decode. `h1_video_feed_ipu()` obtains bytes only through the H1 Transport MPEG read seam, distinguishes payload bytes from 16-byte DMA padding, waits for TO_IPU ownership, and submits aligned DMA. `h1_video_sequence_init()` rejects zero/oversize sequence geometry before using the picture surface. Session release waits GIF/TO_IPU ownership, calls `MPEG_Destroy()`, frees packets and owned buffers, and only then permits reuse.

The safe-stop evidence sharpens an essential boundary that the older monolithic runtime does not express cleanly: owner stop must remain a request while `MPEG_Picture()` is active. The data callback must continue supplying real MPEG bytes; it must never translate local cancellation into synthetic EOF. Stop is observed only after `MPEG_Picture()` returns to project code. The hardware-proven sequence then destroys/restores decoder ownership, marks the worker finished, and only later lets higher-level exact-generation retirement/join/finalization proceed.

### Current clean owners / bridges

- `src/transport` must remain sole physical receive owner and own channel-4 queue/admission/credit/wake facts. A new MPEG owner may consume only a public Transport MPEG bridge; it must not reach into queue/semaphore/runtime internals.
- A new `src/mpeg` (name subject to current source-naming authority) is the natural owner for libmpeg initialization/destruction, IPU/DMAC acquisition/preparation, aligned feed/picture resources, sequence bounds, decoder worker/call ownership, decoder result propagation, and safe-stop observation at completed decoder-call boundaries.
- `src/config` should supply narrow immutable qualified decoder/resource/profile facts rather than recreating H1's broad 61-field runtime tuning surface.
- `src/media` remains the common-clock owner, but first-presentation epoch arm belongs to later presentation composition; decoder start/prefill must not arm it.
- application remains future owner of exact-generation START/retire coordination through component bridges; do not put that coordinator into the first decoder-core packet.

### Behavior that must survive

1. MPEG remains one logical channel on the existing PSTV connection; no second socket/receiver.
2. Bounded/aligned feed and picture resources, sequence width/height validation, and actual-payload-vs-DMA-padding accounting remain explicit.
3. Decoder acquisition establishes the qualified known IPU/DMAC state before `MPEG_Initialize`; H1 field 44 does not survive as a product switch that can disable required preparation.
4. Empty MPEG queue waiting should use Transport producer/receiver-driven wake semantics, with wake-before-sleep/lost-wakeup safety, not H1's older timer-backed hot-path polling.
5. Finite producer exhaustion is distinct from owner cancellation.
6. Owner stop is observed only after `MPEG_Picture()` returns. The feed callback must not synthesize EOF because stop was requested.
7. `MPEG_Destroy()` and local resource restoration/free happen only after decoder-call ownership returns.
8. Unexpected decoder exit without owner stop remains failure; an earlier displayed frame cannot mask later decoder/runtime failure.

### H1 scaffolding / behavior not to carry forward

- stage colors, E2xx/stage/result telemetry as correctness state;
- `VIDEO_IPU_RESET_EACH_SESSION` as a user/product toggle;
- timer-backed `MPEG_EMPTY_DELAY_US` hot-path polling when event wake is available;
- arbitrary H1 queue/feed/prefill/thread tuning values as hidden production defaults;
- scheduler-mode comparison, draw/drop/calibration/compositor policy in the decoder-core packet;
- direct H1 coupling of decoder and GS draw-packet construction where A004 now owns presentation/compositor semantics.

### Known defect boundary

The false-EOF stop path is a proven defect and must be prevented explicitly. The separate spontaneous active-session MPEG read stall remains unresolved forensic evidence and must not be silently 'fixed' or conflated with safe stop. A generic timeout must not convert unexplained MPEG silence into successful completion.

## Likely packet split

**A003-P1 — decoder/Transport-consumer core (recommended next packet):** establish the clean MPEG owner; narrow explicit decoder/profile/resource values; public Transport MPEG consumption/wake seam as needed; feed alignment/accounting; sequence bounds; decoder acquisition/release; finite exhaustion; safe-stop-at-`MPEG_Picture()` boundary; deterministic host seams/fixtures. Stop before START/retire generation orchestration and presentation.

**A003-P2 — exact-generation lifecycle/orchestration:** exact START validation and prepared generation, one-way retiring state, producer emission lease/fence, exact RETIRE ACK ordering, PS2 admission close, exact worker stop/join, residual queue discard + owed credit return, and fresh-generation reopen rules. This necessarily crosses application/Transport/MPEG and Pi producer boundaries and deserves a separate Foreman packet after the decoder owner exists.

**A003-P3 only if Foreman finds it cannot be cleanly deferred:** narrow first-presentation arm handoff to A004. Prefer leaving actual epoch arm/presentation scheduling to A004; A003 should expose only the decoded-frame/presentation boundary required for later composition.

Meaningful P1 stretch: concrete PS2 libmpeg/IPU adapter and pinned clean compile/link proof, but only after host-visible decoder ownership/safe-stop semantics are complete. Do not stretch into presentation or generation coordination.

## Tests / evidence likely required

Host-testable in P1:

- invalid/oversize sequence rejection and resource-capacity math;
- feed-size rounding/alignment with payload bytes distinct from padded DMA bytes;
- temporary empty queue -> event wake -> resumed feed, including wake-before-sleep interleavings;
- true producer exhaustion -> decoder EOF/completion only when Transport proves exhaustion;
- owner stop while decoder call is active does not alter callback into false EOF; stop is observed at returned-picture boundary;
- unexpected decoder return without owner stop is failure;
- create/init/sequence/feed/decode/destroy failure propagation and release ordering;
- no decoder-visible buffer/resource reclaim before decoder-call/worker completion.

P2 host/state-machine evidence:

- exact session/generation/geometry START rejection;
- stale/repeated generation prevention;
- retiring latch is one-way;
- close vs in-flight emission-lease race;
- exact RETIRE completion closes PS2 admission;
- stop/join precedes residual discard and credit return;
- wrapped residual queue discard returns exact withheld credit;
- fresh generation cannot open until old queue/credit/retirement state is fully clear.

PS2 build/hardware-only or hardware-critical:

- actual IPU/DMAC known-state preparation and repeated decoder initialize/destroy;
- SMS libmpeg callback/DMA behavior and RGB16 path;
- event-wake endurance under active all-guns load;
- repeated start/stop/relaunch generations and finite completion;
- proof that safe-stop prevents the historical global freeze on reconstructed source;
- later A/V timing/presentation qualification after A004 composition exists.

Canonical PS2 compile/link identity, linked-symbol/reproducibility/PT_LOAD evidence remain required for any hardware-facing P1/P2 code but are not hardware qualification.

## Foreman decisions / open authority questions

1. **A002 exit gate first:** current worker has not returned. Foreman must decide whether accepted W1-W8 are enough to close source-level A002 or whether one final bounded A002 application-composition packet is required. If such a packet is required, this A003 dossier waits one more cycle.
2. **Transport MPEG seam shape:** inspect current Transport internals only after the A002 packet settles and decide the smallest public status/read/activity/exhaustion/wake contract. Do not copy H1 private queue APIs merely because they exist historically.
3. **First A003 packet boundary:** recommended P1 excludes exact-generation/Pi orchestration. Foreman may combine only if current source proves the bridge contract cannot be tested coherently in isolation; do not combine for convenience.
4. **PS2 adapter timing:** decide whether concrete libmpeg/IPU bindings belong in P1 acceptance or stretch. Host policy can prove ownership/order, but A003 cannot be machine-complete without eventual pinned PS2 compile/link evidence.
5. **Presentation seam:** decoder core needs a decoded-frame handoff but must not choose A004 draw geometry, scheduler/drop policy, suppression/matte behavior, or arm the common epoch merely to make a test convenient.

## Exact Foreman consumption point

Consume this dossier only after the current A002 audio-worker/common-clock Reconstruction worker publishes its immutable completion handoff and Foreman has inspected/integrated W1-W8. If Foreman closes A002 without another product packet, use this dossier to issue the first bounded A003 decoder/Transport-consumer packet. If Foreman issues a final A002 orchestration packet, retain this as conditional archaeology and refresh it against the later settled source before issuing A003.

NEXT_PICKUP=Refresh branch and completed A002 handoff. If A002 is settled, inspect the newly final Transport/media/audio seams and refine A003-P1 exact public MPEG bridge/API/test targets; otherwise remain one dependency step ahead without competing with active A002 writes.
