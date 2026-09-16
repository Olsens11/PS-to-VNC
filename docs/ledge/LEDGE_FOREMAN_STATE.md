# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0009
RECORDED_AT=2026-09-16T10:34:10-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0003
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0026
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A002_AUDIT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision consumes the completed A002 common-media-clock Reconstruction handoff, the independent provisional Validation review of that source, Continuity global revision 0026, and the Foreman-owned canonical integration/evidence performed after the baton returned. The live branch was re-read immediately before this state write at generated dictionary/portal authority `e78d1834d2eecdeae5904478bdd613c406c3b9b6`.

A001 machine/source PASS remains independent Validation authority. A001 physical PS2 qualification remains `HARDWARE_PENDING`. No statement below converts Foreman machine evidence into Validation PASS or physical qualification.

## Current foreman phase

`A002_AUDIO_CHANNEL_PACKET_ISSUED`

The A002 common-media-clock M1-M8 packet is complete from Reconstruction's side and is accepted by Foreman as `MET` after direct source/test inspection. Independent Validation has reviewed the source provisionally, opened no product-source finding, and correctly withheld PASS until integrated canonical evidence is independently consumed.

The next dependency-ordered A002 behavior is the Transport-owned AUDIO logical byte channel that the PCM owner will later consume. Current clean Transport already defines protocol channel `AUDIO=2` but its runtime receiver/bridge behavior remains RFB-only. The next worker packet therefore reconstructs the AUDIO queue/credit/finite-producer seam on the existing sole receiver before any AUDSRV playback worker is built on top of it.

## Reconstruction result consumed

WORK_ITEM_KEY=`a002-audio-clock`
WORKER=`interactive`
WORKER_STARTING_HEAD=`a0d09656306e8e17ec3b67d777536bf39241134f`
WORKER_LOG_COMMIT=`911b846e7f604c1e0d690a47f54f7bc87a80b6c4`

Substantive worker commits inspected:

- `9f13f5875dcba5a679126961b0840b3b1fa7157e` — clean common-media-clock boundary;
- `dfc211a217687ff0d79869511c62d6ca935610aa` — one-shot publication, deadline math, and wait semantics;
- `8e132c46aa3842cdbfe6ae35f700e6eaedb5cc76` — deterministic common-media-clock host fixture;
- `636990a432d5e133ef441d55d53bf38a2fc7be75` — media source dictionary inventory;
- `8edf4e7e8a1a301eb8169a6c34f2a4f9d1350efa` / `1fe64b7ccc85d51f10e3fc7968b208a9649bb38e` — clean media topology/ownership documentation;
- `4b13b4c658b604061c268c4e975e300da7fb7e56` — immutable publication-contract clarification.

Files directly inspected included `src/media/clock.h`, `src/media/clock.c`, `src/media/SYMBOLS.md`, `tests/unit/media_clock_test.c`, `src/config/profile.h`, current Transport runtime/protocol/bridge headers and source, the A001 Transport/RFB audit, and A002 CONFIG/audio/clock audit authority.

### M1-M8 disposition

M1 `MET` — one session-scoped common media-clock owner consumes the narrow immutable media-clock profile without leaking mutable timing ownership into CONFIG, application, or Transport.

M2 `MET` — epoch publication is lock-protected, writes the epoch before exposing `armed`, and repeated arm is idempotent without moving the session origin.

M3 `MET` — audio and neutral-video deadlines derive from one shared epoch with exact signed `int32_t` offsets, saturating underflow/overflow, saturating additional-tick addition, and fail-closed unarmed behavior.

M4 `MET` — state/math uses injected synchronization and time/delay seams rather than embedding PS2 timer/fence mechanics, so the contract is deterministic on host and leaves the concrete PS2 binding separable.

M5 `MET` — waits distinguish unarmed/not-ready from ready, honor stop, surface synchronization/timer/delay failures, and reject zero polling cadence instead of accidentally busy-spinning.

M6 `MET` — behavior tests cover initialization/unarmed state, one-shot/idempotent arm, publication witness, shared A/V epoch, normal signed offsets, `INT32_MIN`/`INT32_MAX`, saturation, stop, timer/delay failure, synchronization failure, and zero-poll rejection.

M7 `MET` — A001 Transport/RFB and completed A002 CONFIG/profile behavior were not changed by the worker; no timing default was invented.

M8 `MET` — no PCM/AUDSRV runtime, MPEG decoder/presentation callsite, late-frame policy, or hardware claim was introduced.

No product-source correction packet is required from this handoff.

## Validation consumed

Validation commit `6880801d288bf049a32fadb0b3e51454856fd92c` independently reviewed the common-clock source. It opened no product-source finding and classified the clock `REVIEWED_SOURCE_PROVISIONAL`. Validation explicitly did not claim canonical host/project/PS2-link/reproducibility evidence or A002 PASS, and handed canonical registration/topology/dictionary/build evidence to Foreman.

This Foreman state preserves that lane boundary. The common clock is Reconstruction-complete and Foreman-integrated; independent A002 readiness/PASS remains Validation-owned.

## Foreman-owned integration and evidence

Foreman performed only non-behavioral integration/evidence work:

- `44a42d47c311f64db8f9c805b445338906a4edf9` — registered `media_clock_test` in the canonical host `unit` graph;
- `3689de2a082915259d2db74831fa71b73823aaf9` — linked `src/media/clock.c` into the clean PS2 build graph;
- `b52a9a5ab9e00dd1507d1178a7cbf7aa890771e3` — adopted `src/media` in deterministic dictionary reconciliation;
- `3ccc340167135adcc3703cc382c69c23f2ce1f7d` — allowed the reconciliation workflow to stage the media dictionary;
- `01838f361ee7b23859ad3b93651aec254708844c` — adopted `src/media` in the clean source-topology checker;
- `020c9674cfe94035e75fbb8617513283ba12e456` — exact deterministic reconciliation trigger;
- generated child `e78d1834d2eecdeae5904478bdd613c406c3b9b6` — regenerated the canonical dictionary portal with `src/media` COMPLETE, count 146; the worker-authored `src/media/SYMBOLS.md` itself already matched mechanically discovered authority and needed no generated row correction.

Workflow run `35109206736` at integration trigger `020c9674...` reports:

- canonical host-unit job `PASS` with the registered common-media-clock fixture in the `unit` graph;
- strict long dictionary job `PASS`;
- pinned PS2 compile job `PASS`;
- clean PS2 linked-build job `PASS`, including the new `media_clock.o` build graph and current-source linked reproducibility `PASS`;
- deterministic dictionary reconciliation job `PASS`, producing `e78d1834...`;
- project-check `FAIL` on that pre-generated-portal trigger tree. That trigger-tree project result is not a settled-tree disposition because the generated portal child had not yet landed there.

This state write occurs on the settled generated portal tree and intentionally triggers a standard workflow pass for final Foreman evidence. The immutable Foreman round log must inspect that result before closure.

The concrete PS2 synchronization/timer/delay binding for the common clock is still a future runtime-integration obligation. Its absence is not a defect in the pure clock contract and is not a blocker for the next Transport AUDIO-channel packet.

PENDING_LOCAL=settled-tree canonical workflow result after this state write; concrete PS2 media-clock synchronization/time binding when a runtime media owner is wired; independent Validation disposition
HARDWARE_PENDING=A001 physical PS2 qualification remains pending; no A002 physical qualification is claimed

## Dependency conclusion for next Reconstruction work

A002 audit revision 0001 requires PCM playback to consume bytes from the Transport-owned AUDIO logical channel and to treat finite producer completion normally. A001 architecture already requires AUDIO/RFB/MPEG logical channels to share one physical PSTV connection with one sole Transport receiver. Current protocol vocabulary defines AUDIO channel 2, but current clean runtime dispatches only RFB DATA and exposes only the RFB logical bridge.

Building an AUDSRV worker before the AUDIO Transport seam exists would force the audio component to invent or bypass its byte source. The next clean dependency is therefore a bounded Transport AUDIO-channel packet: add an independent AUDIO byte queue and credit state behind the existing sole receiver, expose a narrow consumer seam, and prove ordered finite-producer completion. Playback, AUDSRV lifetime, audio worker allocation, startup-reservoir timing, and media-clock presentation gating remain later A002 packets.

H1 is evidence, not an implementation template. Preserve the one-receiver/multi-logical-channel semantics, independent queue/credit accounting, and ordered finite producer marker, but do not copy H1's giant profile or arbitrary experimental tuning as public runtime knobs.

## Fresh interactive Reconstruction packet

WORK_ITEM_KEY=`a002-audio-clock`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`e78d1834d2eecdeae5904478bdd613c406c3b9b6`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md:0001`
ASSIGNING_FOREMAN_STATE=`0009`

### Objective

Reconstruct the A002 Transport-owned AUDIO logical byte-channel seam on the existing sole physical receiver. Add only the queue/credit/finite-producer behavior needed for a future PCM consumer to read AUDIO bytes safely and independently of RFB. Preserve A001's one-socket/one-recv-owner architecture and do not begin AUDSRV playback or audio worker lifecycle.

This packet is deliberately substantial Transport behavior, but it stops at the logical byte source. The following Reconstruction packet can then build the PCM/AUDSRV consumer against a real clean seam rather than combining two major responsibilities in one round.

### Required deliverables

1. **Explicit narrow AUDIO Transport authority.** Introduce the smallest immutable Transport-owned audio-channel configuration needed by the queue/credit mechanism. Queue capacity and credit behavior must be explicit caller/profile authority in the clean type boundary; do not invent application magic defaults and do not import the H1 61-field public profile. If qualification has not selected production tuning values, keep the values explicit at the owner boundary rather than guessing them.

2. **One sole receiver, two independent logical DATA paths.** Extend the existing Transport receiver so valid RFB DATA continues to reach the current RFB path and valid AUDIO DATA reaches a Transport-owned AUDIO queue. Do not create a second socket reader or receiver thread. CONTROL and unsupported/invalid channel/kind behavior must remain fail-closed according to current protocol ownership.

3. **Independent AUDIO queue/credit accounting.** AUDIO storage, occupancy, earned credit, batching/flush policy, and wakeup state must be independent from RFB so backpressure on one logical channel does not corrupt accounting on the other. Consumer dequeue earns AUDIO credit only for bytes actually removed from the AUDIO queue.

4. **Finite producer marker.** Preserve the audited ordered finite-producer semantic using a zero-length AUDIO DATA marker (or the clean equivalent proven from current protocol framing) after all PCM payload. Once the marker is accepted, Transport exposes producer-done state; `producer_done && audio_queue_empty` is normal AUDIO exhaustion. Non-empty AUDIO DATA after producer-done must be rejected rather than silently reopening the producer.

5. **Narrow consumer bridge.** Expose only the logical operations a later audio component needs: bounded byte read/dequeue, availability/progress as necessary, finite exhaustion observation, and terminal transport failure/stop propagation. Do not expose queue internals or require audio to mutate Transport diagnostics.

6. **Preserve terminal/quiesce semantics.** Receiver EOF/fatal protocol error/stop must wake AUDIO consumers and remain distinguishable from normal producer-done exhaustion. Do not weaken A001 terminal-error first-cause behavior or RFB quiesce guarantees.

7. **Deterministic behavior tests.** Add host tests that prove at minimum:
   - RFB-only regression remains unchanged;
   - AUDIO-only enqueue/read and partial bounded reads;
   - RFB/AUDIO interleaving through one receiver;
   - independent queue pressure and credit earning/batching/flush behavior;
   - zero-length AUDIO finite-producer marker ordering;
   - normal `producer_done + empty` exhaustion;
   - rejection of later non-empty AUDIO DATA after producer-done;
   - receiver terminal failure/EOF/stop waking the AUDIO side without masquerading as normal exhaustion;
   - invalid channel/kind combinations remain fail-closed;
   - no second receive owner appears.

8. **Boundary discipline.** Update clean source-side dictionary/topology ownership for any genuinely new source file. Do not perform generated portal or canonical build/test registration work owned by Foreman. Do not initialize AUDSRV/LIBSD, allocate an audio playback worker, apply PCM format/volume, implement startup-reservoir timing, arm/wait on the media clock from audio, add MPEG behavior, or claim hardware qualification.

### Acceptance criteria

A1. Transport exposes explicit narrow AUDIO-channel configuration with no hidden application defaults and without recreating H1's giant public profile.

A2. Exactly one Transport-owned physical receiver dispatches both RFB and AUDIO DATA; no competing socket read path exists.

A3. AUDIO queue storage, synchronization, and credit accounting are independent from RFB and return credit only for bytes dequeued by the AUDIO consumer.

A4. Ordered finite producer completion is explicit and deterministic: marker accepted once, producer-done observable, queue drain yields normal exhaustion, and payload after producer-done is rejected.

A5. A later audio owner can consume bounded bytes and distinguish normal exhaustion, stop, and terminal Transport failure through a narrow bridge without touching Transport internals.

A6. Existing A001 RFB/receiver/quiesce behavior remains stable and invalid protocol/channel cases remain fail-closed.

A7. Deterministic host tests cover interleaving, pressure/credit behavior, finite completion, terminal paths, and RFB regression rather than only happy-path AUDIO reads.

A8. No AUDSRV/PCM playback worker, startup-reservoir timing, media-clock audio callsite, MPEG behavior, application orchestration, or hardware qualification is introduced.

### Preserved invariants / non-goals

- One physical PSTV socket/stream and one sole Transport recv owner remain mandatory.
- RFB and AUDIO have independent logical queue/credit state despite sharing the physical receiver.
- Existing A001 Transport/RFB behavior is regression authority, not a rewrite target.
- The common media clock reconstructed in M1-M8 remains untouched by this packet.
- Do not invent production AUDIO queue/credit numbers merely because H1 swept values; keep unresolved tuning explicit at the clean owner/profile boundary.
- `MEDIA_END + empty AUDIO queue` / equivalent producer-done semantics are normal completion, not an error.
- No `audsrv_wait_audio`, `audsrv_play_audio`, `audsrv_stop_audio`, `audsrv_quit`, PCM thread, or resident-service lifecycle belongs in this packet.
- Do not begin A003 MPEG/video/presentation behavior.
- A001 physical status remains `HARDWARE_PENDING`.

### Blocker burden

Physical PS2 hardware is not a blocker: queue, protocol dispatch, credit, finite-producer, terminal, and one-receiver semantics are host-testable.

Absence of final production tuning values is not a blocker and must not be solved by guessed defaults. Keep resource/credit values explicit at the Transport owner boundary and document any qualification choice still pending.

If the current clean frame parser cannot represent the audited zero-length AUDIO producer marker without changing shared physical framing, prove that limitation from source/tests and return `PARTIAL` with the narrowest protocol correction required; do not smuggle completion through a different unverified sentinel.

### Stretch only if core A1-A8 are complete

A small pure audio-start reservoir policy value/state helper may be added only if it does not consume Transport bytes, start a worker, call AUDSRV, or wait on the media clock. It must keep `reservoir-ready` distinct from `presentation-deadline-ready`. Do not use the stretch target to expand the core packet.

## Exact next Foreman pickup

After the Reconstruction worker returns, inspect the actual AUDIO Transport source/tests and criterion-by-criterion result, verify one-receiver ownership and independent credit/exhaustion semantics, perform only canonical registration/build/dictionary/evidence chores, and then issue the next dependency-ordered A002 PCM/AUDSRV consumer packet if the AUDIO seam is coherent.

Validation should independently consume the fully integrated common-clock authority and later the coherent AUDIO/PCM tranche; Foreman evidence is not Validation PASS.
