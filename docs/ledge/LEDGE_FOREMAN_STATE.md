# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0012
RECORDED_AT=2026-09-16T14:18:56-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0029
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A002_AUDIT_REVISION=0001
BASED_ON_A003_AUDIT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision consumes the completed A002 audio-worker/resource/reservoir/common-clock Reconstruction handoff, directly inspects the actual lifecycle source and deterministic fixture, performs only Foreman-owned canonical integration/evidence and governance synchronization, consumes the generated dictionary child without treating it as behavior authority, closes A002 Reconstruction source work as ready for independent Validation, and issues the first bounded A003 MPEG reconstruction packet.

The live branch was re-read immediately before this state write at `0ed3ceea505271f730ef007249f7d26ebf45df83`. Unknown external/Pi-local dirty work remains outside this GitHub-native surface and is neither overwritten nor declared absent.

A001 machine/source PASS remains independent Validation authority. A001 physical PS2 qualification remains `HARDWARE_PENDING`. A002 is now Foreman-integrated and Validation-ready, but no statement below promotes it to independent Validation PASS or physical qualification.

## Current Foreman phase

`A003_MPEG_TRANSPORT_DECODER_CORE_PACKET_ISSUED`

The A002 W1-W8 audio lifecycle packet is Reconstruction-complete and Foreman-accepted as `MET` after direct source/test inspection. Together with the previously accepted CONFIG, common-clock, Transport AUDIO, and synchronous PCM/AUDSRV packets, the clean A002 source tranche now has coherent component ownership and canonical machine integration. No further A002 product-behavior packet is required before independent Validation review.

The next dependency-ordered source responsibility is A003 MPEG decode/exact-generation work. The first A003 packet deliberately reconstructs the logical MPEG Transport consumer plus decoder ownership/safe-stop core. It stops before the high-level exact START/retirement orchestration, Pi producer/capture/suppression process, GS composition, or first-presentation media-clock arm.

## Reconstruction result consumed

WORK_ITEM_KEY=`a002-audio-clock`
WORKER=`interactive`
WORKER_LOG_COMMIT=`da0a27e4258c9afc55d6446f74d181f6a6b56f9f`
WORKER_LOG=`docs/ledge/work-log/20260916T133333-0400__reconstruction__a002-audio-clock__interactive.md`
WORKER_RESULT=`FOREMAN_GOAL_RESULT=MET`

Substantive worker commits inspected:

- `5fce6b48287b26e0b40fe247e6bca20daa030ed6` — define the clean audio session lifecycle owner;
- `b5e020bc7b93d9816a09196b58c1bcdd1c571f27` — implement explicit worker/resource/reservoir/common-clock lifecycle;
- `4dca53b58db338d9ecfe600cd6ed751041e2af37` — deterministic audio-session lifecycle fixture;
- `1824f25ff04d061d88e7a8bbcb664f2064e441d3` — extend clean audio source-domain topology;
- `05d2b8a3a44ecf3807f4bd6cdadf09948696e254` — update audio source dictionary;
- `da0a27e4258c9afc55d6446f74d181f6a6b56f9f` — immutable Reconstruction handoff.

Concurrent Architecture scout commit `7b2ae6b13f16aeac7b3ac327cdd33a2e7af3f239` was consumed as non-authoritative planning support only. Its recommendation to proceed into A003 was independently checked against A002/A003 audit authority and current source.

Directly inspected product files included `src/audio/session.{c,h}`, `tests/unit/audio_session_test.c`, accepted `src/audio/playback.{c,h}`, `src/media/clock.c`, the public Transport AUDIO bridge, A002 audit revision 0001, A003 audit revision 0001, and current Transport channel vocabulary.

### W1-W8 Foreman disposition

W1 `MET` — session lifecycle values are explicit caller authority: worker stack bytes/priority, playback buffer capacity, startup reservoir bytes, reservoir poll interval, and common-clock poll interval. No production defaults or H1 laboratory knob surface were invented.

W2 `MET` — the session owner allocates buffer/stack, creates and starts one worker, records completion, joins before destruction, and refuses release while a started worker remains unjoined. Create/start/join failures preserve the worker/resource reclamation fence.

W3 `MET` — startup reservoir readiness observes only public Transport AUDIO status/activity and does not dequeue PCM. The activity sequence is persistent across waits; unchanged activity does not cause repeated status polling. A finite producer with a nonempty short final reservoir may proceed, while producer-done plus empty is normal finite completion.

W4 `MET` — after reservoir readiness the worker calls the accepted common-clock audio waiter. The current real waiter was independently inspected and already handles the initially-unarmed state by bounded delay/recheck until another owner publishes the epoch, then waits to the audio deadline. Audio never calls `pstvnc_media_clock_arm()` and cannot move the epoch.

W5 `MET` — after the timing gate the worker delegates unchanged to the accepted synchronous PCM playback core and preserves its exact playback result/report rather than duplicating Transport/AUDSRV submission logic.

W6 `MET` — local stop is honored during reservoir and clock waits; active-playback Transport convergence remains deliberately outside this component and must be driven later by top-level Transport stop/abort plus worker join. Audio does not seize Transport ownership or hide a second abort policy.

W7 `MET` — deterministic tests cover explicit resource values, allocation/create/start/join/reclaim failures, activity-driven reservoir crossing, unchanged-activity anti-spin behavior, short finite producer, empty finite completion, externally armed epoch transition, no audio arm call, stop during both pre-play waits, media-clock sync/timer/delay failures, exact downstream playback result preservation, and invalid/missing lifecycle authority.

W8 `MET` — no application orchestration, MPEG/video behavior, presentation/epoch-arm ownership, production tuning defaults, receive-poison repair, or hardware qualification was introduced.

No product-source correction packet is required.

## Foreman-owned canonical integration and governance

Foreman made only non-behavioral integration/evidence changes after the baton returned:

- `6125033da8eb23edc46698164929863068868a3f` — synchronized `scripts/work-log-check.py` with governing work-log contract revision 0005 by adding the exact ninth already-frozen Diagnostics compatibility record. No immutable history was rewritten.
- `6f76c685e09df51b90f0cda28ed88574a1e3c2ed` — added `src/audio/session.c` to the strict every-clean PS2 compile inventory.
- `0027a73851130b5a8bc71684da17cc8a3b0694f0` — linked `audio_session.o` into the clean PS2 executable graph with explicit source/header dependencies.
- `b4037585beea138fb67ba6377f89faecbac32ef2` — registered `audio_session_test` in canonical `make -C tests unit`.
- `b0431db9bc9f769c137463066dfaca4500cfc06f` — registered the audio session lifecycle in the living file/service ownership map.
- `685b15febb5e0ea5c0064c2e6bbd510c297af30d` — exact deterministic dictionary-reconciliation trigger.
- generated child `0ed3ceea505271f730ef007249f7d26ebf45df83` — regenerated the source-dictionary portal. The generated diff changed only the `src/audio` portal count from `82` to `242`; the worker-authored audio dictionary itself required no generated repair.

The worker noted an accidentally created auxiliary branch named `tmp-noop-do-not-use`. It was never used for product authority and does not affect `ledge/h1-all-guns`; it remains ordinary repository-cleanup debt rather than a product defect.

## Canonical evidence

Integration workflow run `35133071733` at `b4037585beea138fb67ba6377f89faecbac32ef2` reported:

- canonical host-unit `PASS`, including `audio_session_test: PASS`;
- strict dictionary `PASS`;
- pinned PS2 compile `PASS`;
- clean PS2 link and linked reproducibility `PASS`;
- project-check red only because that pre-reconciliation tree's generated source-dictionary portal had not yet been regenerated.

Exact project-check reason on that integration tree was `SOURCE_DICTIONARY_PORTAL_OUT_OF_DATE`, with the canonical required action to regenerate `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`. This was mechanical generated-evidence lag, not product behavior failure.

Deterministic reconciliation workflow run `35133280175` at trigger authority `685b15febb5e0ea5c0064c2e6bbd510c297af30d` reported:

- canonical host-unit `PASS`, including `audio_playback_test: PASS`, `audio_audsrv_service_test: PASS`, `audio_session_test: PASS`, Transport AUDIO/runtime, CONFIG/profile, media-clock, RFB, application, input/UI, and other existing fixtures;
- strict dictionary `PASS`;
- pinned PS2 compile `PASS` with `src/audio/session.c` included;
- clean PS2 linked-build `PASS` with `audio_session.o` included;
- pristine linked ELF SHA256 `1681787dc50ce76ee67354b322ea980fca19a3fa12084835d54fcc2f9c82c6d6` on both builds;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=3eb899151389a721b57504a852fe516f7c52d565a9f8c72fadb435d490b62ac9`;
- `PT_LOAD_BYTES=425864`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `ISSUE7_RUNTIME_IDENTITY_LINKED=YES`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`;
- dictionary reconciliation `PASS`, producing generated child `0ed3ceea505271f730ef007249f7d26ebf45df83`.

The trigger-tree project-check necessarily evaluated the pre-generated portal and therefore remained red for portal lag. This state write occurs after the generated child and intentionally triggers the settled-tree canonical recheck, including the newly synchronized nine-record work-log policy.

PENDING_LOCAL=settled-tree workflow for Foreman state revision 0012; independent Validation disposition for the complete integrated A002 source tranche; later PS2 hardware qualification
HARDWARE_PENDING=A001 physical PS2 qualification remains pending; A002 reconstructed audio/common-clock behavior also has no physical qualification claim

## A002 handoff to Validation

A002 is now `VALIDATION_READY` at Foreman/source-integration level, not `PASS`.

Independent Validation should consume the complete integrated A002 authority rather than only the earlier synchronous PCM slice. It should specifically re-check:

- one Transport-owned AUDIO channel with independent queue/credit/finite producer semantics;
- decoded immutable PCM/common-clock owner values and absence of hidden defaults;
- one-shot common epoch and audio wait semantics;
- strict AUDSRV wait-before-play and post-submit accounting;
- resident-service session cleanup without per-session quit;
- audio worker/reservoir/clock lifecycle and stop/join/reclaim fencing;
- the settled canonical machine evidence after this state commit.

Foreman does not pre-declare that independent result.

## Fresh interactive Reconstruction packet — A003-P1

WORK_ITEM_KEY=`a003-mpeg-generation`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`0ed3ceea505271f730ef007249f7d26ebf45df83`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A003_MPEG_GENERATION.md:0001`
ASSIGNING_FOREMAN_STATE=`0012`

### Objective

Reconstruct the first coherent A003 unit: Transport-owned MPEG2 logical-channel ingestion plus a clean, host-testable MPEG decoder ownership/safe-stop core. Preserve the sole physical PSTV receiver, event-driven empty-queue wake behavior, real finite-producer exhaustion, decoder call ownership, known-state decoder acquisition/release, sequence/feed bounds, and the proven rule that a local stop request must never synthesize false EOF inside an active decoder data callback.

This packet deliberately stops before high-level exact-generation START/retirement orchestration and before presentation/compositor/media-clock arm ownership.

### Required deliverables

1. **Explicit narrow MPEG Transport authority.** Add the smallest immutable caller-owned MPEG channel configuration needed for bounded queue/credit behavior. Preserve the existing RFB/AUDIO values and do not invent application defaults. Queue/credit/resource tuning that is not yet production-qualified remains explicit caller authority rather than guessed constants.

2. **Sole-receiver MPEG dispatch.** Extend the existing one Transport receive owner so valid channel-4 MPEG2 DATA enters a Transport-owned MPEG queue independently of RFB and AUDIO. Do not add another physical reader, socket, or receive thread. Existing invalid channel/kind and first-cause terminal behavior must remain fail-closed.

3. **Independent MPEG queue/credit/event wake seam.** MPEG owns independent bounded storage, occupancy, dequeue-earned credit, batching/flush, activity/wakeup, finite-producer state, and terminal wake semantics. Expose only a narrow consumer bridge sufficient for bounded reads, status/activity/wait, normal exhaustion, stop, closed, and failure. Normal empty waiting must be receiver/producer-driven with wake-before-sleep safety; do not restore timer-backed hot-path polling.

4. **Finite exhaustion distinct from cancellation.** Preserve a real finite-producer MPEG completion fact using the existing framing only if current protocol authority proves the representation. `producer_done && queue_empty` may produce real decoder EOF/exhaustion. Owner-requested stop/cancellation is a different state and must not be represented as producer EOF. Do not introduce per-packet generation tags as a shortcut for exact-generation lifecycle.

5. **Clean decoder owner and injected decoder/platform boundary.** Create the smallest coherent MPEG component (a clean `src/mpeg/` domain is appropriate if current topology rules are followed). It owns decoder acquisition, decoder-call ownership, feed policy/state, sequence/resource bounds, and release. Establish the required known IPU/DMAC decoder state before initialization through a narrow injected/concrete platform seam where current PS2 authority proves the mechanism. Do not expose H1's `VIDEO_IPU_RESET_EACH_SESSION` laboratory toggle as product configuration.

6. **Feed and sequence correctness.** Validate sequence dimensions against explicit caller/profile maximums before using the picture surface. Keep decoder feed payload bytes bounded; if the concrete PS2 path requires DMA alignment/padding, keep actual payload-byte accounting distinct from padded transfer bytes. Do not invent production feed-size/max-dimension defaults merely to make the component compile.

7. **Safe decoder stop and truthful result propagation.** A local stop request may be observed only at a completed decoder-picture/project-code boundary. If the decoder is currently inside a picture/decode call and asks its data callback for bytes, the callback must continue to report real Transport data/exhaustion/failure according to Transport state and must never synthesize EOF solely because owner stop is pending. After the decoder call returns, the owner may retire for stop. Unexpected decoder-worker/core exit without an owner stop request is failure even if a frame was previously produced. Decoder destruction/reclamation occurs only after decoder-call ownership has returned.

8. **Deterministic host tests and boundary discipline.** Cover at minimum:
   - RFB/AUDIO regressions plus MPEG dispatch under the same sole receiver;
   - MPEG bounded reads, pressure, independent credit earning/batching/flush;
   - wake-before-sleep/event activity without timer polling;
   - real finite producer completion/exhaustion versus owner stop/cancellation;
   - Transport EOF/fatal error/stop waking MPEG correctly;
   - invalid channel/kind fail-closed and no second receive owner;
   - decoder prepare/init/call/destroy ownership ordering;
   - sequence dimension rejection before picture-surface use;
   - payload-byte versus padded-feed accounting where applicable;
   - an active decoder call plus owner stop where the data callback still receives real data and never false EOF from the stop request;
   - unexpected decoder return classified as failure absent requested stop;
   - no first-presentation media-clock arm in MPEG decode merely for convenience.

Perform Reconstruction-owned source-side dictionary/topology adoption for a new clean MPEG domain, but leave generated portal, canonical test/build registration, and CI evidence to Foreman.

### Acceptance criteria

M1. MPEG2 is a third independent logical DATA path on the existing sole physical Transport receiver; there is no competing receive owner.

M2. MPEG queue/credit/activity/finite-producer state is bounded and independent from RFB/AUDIO; consumer credit is earned only for bytes actually removed.

M3. The public MPEG consumer seam provides event-driven availability and distinguishes normal finite exhaustion from stop/closed/failure without exposing Transport internals.

M4. A clean decoder owner establishes known decoder state, owns initialize/call/destroy ordering, validates sequence/feed bounds, and does not expose the H1 reset toggle as a product knob.

M5. Local stop cannot synthesize EOF inside an active decoder data callback; stop is honored only after the decoder call returns, while true producer exhaustion remains real EOF/exhaustion.

M6. Payload consumption and padded/aligned transfer accounting remain distinct where the platform path requires padding, and no unqualified resource/timing defaults are invented.

M7. Unexpected decoder return without owner stop is a failure, decoder resources are not reclaimed while decoder call ownership is live, and deterministic tests prove the required interleavings/error paths.

M8. Existing A001/A002 behavior remains regression-stable; this packet does not implement exact START/generation retirement orchestration, Pi producer/capture/suppression, GS presentation/compositor, first-presentation epoch arming, scheduler/drop/calibration policy, application orchestration, or hardware qualification.

### Preserved invariants / explicit non-goals

- one physical PSTV connection and exactly one Transport receive owner;
- RFB and AUDIO existing logical semantics remain unchanged;
- A002 common-clock and audio ownership remain unchanged;
- MPEG stop is not fake EOF;
- no timer-backed MPEG empty hot-path polling;
- no per-packet generation tagging substituted for A003's later ordered retirement fence;
- no exact START/retire/reopen process yet;
- no Pi-side producer/capture/suppression changes yet;
- no presentation/compositor/GS behavior and no clock arm;
- no arbitrary scheduler/drop/calibration choices;
- no physical PASS claim.

### Blocker burden / fallback

Hardware is not a blocker for the logical queue/event-wake contract, pure decoder ownership state machine, safe-stop interleavings, bounds, and accounting tests.

If the exact libmpeg/IPU/DMAC concrete signatures or required preparation mechanism cannot be proven from current repository/PS2SDK authority, isolate a narrow injected decoder/platform contract and return `PARTIAL` with the exact missing concrete binding proof rather than inventing a fake platform mechanism.

If production MPEG queue/feed/max-dimension tuning remains unresolved, keep those values explicit caller authority. Missing tuning is not permission to guess defaults.

If current Transport framing cannot represent a real finite MPEG producer marker without a shared protocol change, prove the limitation and return the narrowest protocol correction required; do not invent an unverified sentinel.

### Stretch target only after M1-M8 are complete

Add a pure, side-effect-free exact-generation START value validator/state skeleton that can represent one prepared/active/retiring generation and reject stale/repeated generation identities, but do not wire application/Pi emission or retirement/reopen behavior. Foreman will otherwise issue that exact-generation lifecycle as A003-P2 after reviewing this packet.

## Foreman next pickup

When this worker returns:

1. inspect actual Transport MPEG and decoder source/tests criterion-by-criterion;
2. preserve A001/A002 and safe-stop invariants;
3. perform only canonical test/build/dictionary/topology/living-map integration;
4. consume independent A002 Validation if it has landed, without manufacturing it;
5. if A003-P1 is coherent, issue A003-P2 for exact START/generation retirement/emission-fence/application process;
6. preserve A001/A002/A003 hardware obligations separately from machine evidence.
