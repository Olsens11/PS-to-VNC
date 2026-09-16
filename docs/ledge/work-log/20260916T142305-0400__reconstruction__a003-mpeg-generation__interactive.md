DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T14:23:05-04:00
COMPLETED_AT=2026-09-16T18:13:12-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=2341c9bee5eaef89dd6e4b4917012815a45d7bd7
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction — A003 MPEG Transport + decoder ownership/safe-stop core

FOREMAN_GOAL_RESULT=MET

## Round continuity

This is the single immutable closeout record for the interactive A003-P1 Reconstruction round that began at `2026-09-16T14:23:05-04:00` from `2341c9bee5eaef89dd6e4b4917012815a45d7bd7`. The earlier interactive conversation reached a context limit after the substantive implementation and evidence work; a replacement interaction continued the same round from repository authority. No new Reconstruction round, `STARTED_AT`, worker identity, or alternate log path was created.

Immediately before this immutable create, `ledge/h1-all-guns` was re-read at `38c2a6065b8703584f799833691fcd4914ea59d0`. The A003-P1 source closeout commit remains its ancestor at `0ec6223d45a80871c8fdb0b1ccd77328779c15cd`. Concurrent post-closeout movement observed before this log was support/governance activity, including Validation's self-paused overlap record and Architecture successor-packet scouting; this Reconstruction record does not overwrite or reinterpret those immutable support records.

## Authority consumed

- governing Reconstruction contract revision 0005;
- governing immutable work-log contract revision 0005;
- Foreman state revision 0012 and its bounded `A003-P1` packet;
- `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision 0001;
- current clean architecture, source-topology, source-dictionary, Reconstruction/global/Validation state, and relevant recent immutable cross-lane logs.

Repository authority overrode prompt/session recollection whenever they differed.

## Objective completed

Completed only the bounded A003-P1 unit: Transport-owned MPEG2 logical-channel ingestion plus the clean MPEG decoder ownership/safe-stop core. The implementation preserves the single physical PSTV receive owner, gives MPEG independent bounded logical flow control and producer activity semantics, provides real finite-producer exhaustion distinct from cancellation, and adds a clean decoder owner whose active data callback never converts a pending local stop into false EOF.

This round did **not** begin A003-P2 exact-generation START/retirement orchestration, Pi producer/capture/suppression fencing, application generation coordination, A004 presentation/compositor/calibration, first-presentation common-clock arming, scheduler/drop policy, inherited receive-poison repair, guessed production tuning/defaults, or physical PS2 qualification.

## Product/source work performed

The round reconstructed and exercised the following source behavior:

- `src/transport/mpeg_channel.{c,h}` — independent bounded MPEG logical-channel storage/credit authority;
- `src/transport/runtime.{c,h}`, `src/transport/bridge.{c,h}`, and `src/transport/transport.h` — MPEG dispatch through the existing sole physical receiver, public logical consumer seam, producer activity/completion, terminal wake behavior, and distinct stop/closed/failure/exhaustion outcomes;
- `src/mpeg/decoder.{c,h}` — clean decoder lifecycle owner with explicit injected known-state/platform boundary, caller-owned feed/sequence/resource limits, decoder-call lifetime fencing, safe-stop semantics, and separate payload-versus-padded-transfer accounting;
- `tests/unit/transport_mpeg_test.c` — same-receiver RFB/AUDIO/MPEG interleaving, independent credits, activity wake, overflow, finite completion/exhaustion, zero-length/DATA-after-completion rejection, and stop-not-exhaustion behavior;
- `tests/unit/mpeg_decoder_test.c` — prepare/init/call/destroy ordering, sequence/feed bounds, resource lifetime fencing, unexpected-return failure classification, and the active-call stop interleaving proving that the data callback continues to consume truthful Transport data while stop is pending.

The MPEG feed callback deliberately does not inspect owner-local stop while the decoder picture/decode call is active. Stop is observed only after call ownership returns. Decoder-visible resources are not destroyed while call ownership is live.

A waiter-lifetime race was also hardened so activity sequence and waiter-live state are observed under the queue lock, preserving the intended waiter/reclaim proof.

## Closeout corrections after continuation takeover

One Reconstruction-owned test-source defect was found during exact host compilation: `tests/unit/transport_mpeg_test.c` used EE semaphore/thread host-stub types without including `<kernel.h>`. That fixture include was corrected; no corresponding production MPEG/Transport/decoder behavior defect was found.

Source-side ownership/governance was then reconciled without importing disposable evidence harness files:

- `tests/unit/transport_mpeg_test.c` — corrected host-stub include;
- `src/transport/SYMBOLS.md` — strict complete reconciliation, retaining A001 RFB and A002 AUDIO ownership while assigning only MPEG/new genuinely shared-media Transport symbols to A003;
- `src/mpeg/SYMBOLS.md` — complete clean MPEG source dictionary with decoder resource/call, feed/sequence/bounds, safe-stop, and accounting semantics;
- `docs/development/source-topology.md` — adopted `src/mpeg/` as the clean decoder resource/call ownership domain and explicitly excluded Transport physical-stream, exact-generation, application, presentation, clock-arm, scheduler, calibration, and hardware ownership;
- `scripts/continuity-check.sh` — added `src/mpeg` to the maintained clean-source topology inventory while preserving executable mode.

Those five final qualified source-owned file states were transplanted atomically onto current live authority in:

`0ec6223d45a80871c8fdb0b1ccd77328779c15cd` — `a003: close MPEG decoder source ownership`

The disposable `evidence/a003-p1-host-20260916` branch and its `.github` helper workflows/scripts were not merged into live product authority.

## Deterministic evidence

The final source-owned candidate on the disposable evidence branch was committed at `7aa327f4aebabe878a91eb2146ffb01845c90995` after the candidate gates were green. Evidence workflow run `35150150904` completed the final A003-P1 host/source gate with the qualified source state.

Observed PASS coverage included:

- strict complete source dictionary check;
- `git diff --check` / source-topology assertions;
- `transport-protocol-unit`;
- `transport-rfb-channel-unit`;
- `transport-bridge-unit`;
- `rfb-bridge-unit`;
- `transport-physical-stream-unit`;
- `transport_runtime_test` when manually compiled with the new MPEG channel object;
- `transport_audio_test`;
- `transport_mpeg_test`;
- `mpeg_decoder_test`;
- `audio_playback_test`;
- `audio_audsrv_service_test`;
- `audio_session_test`;
- `media_clock_test`;
- `config_profile_test`.

The existing canonical host-unit build on the pre-Foreman-integration tree stopped at the expected mechanical link-registration gap because Foreman-owned canonical build plumbing did not yet link the new MPEG channel object. That was recorded as integration registration lag, not as a demonstrated MPEG product-behavior failure, and Reconstruction did not seize Foreman-owned Makefile/test-manifest authority to hide it.

A concurrent Validation shift beginning from `0ec6223d45a80871c8fdb0b1ccd77328779c15cd` made provisional source observations consistent with the implementation above, opened no findings, and correctly self-paused because this Reconstruction completion handoff did not yet exist. This log supplies that missing immutable handoff; it does not pre-declare independent Validation PASS.

## M1-M8 disposition

M1 `MET` — MPEG2 is a third logical DATA path on the existing sole physical Transport receive owner. No competing physical reader/socket/receive thread was introduced.

M2 `MET` — MPEG queue, occupancy, credit, producer activity/generation, finite-producer completion, waiter state, and terminal wake semantics are independent from RFB/AUDIO and bounded by explicit caller authority. Consumer credit is earned only for bytes actually removed.

M3 `MET` — the public MPEG consumer bridge provides bounded reads plus producer/activity waiting and distinguishes real finite exhaustion from stop, closed, and failure without exposing the physical socket or Transport internals. Empty waiting is producer/receiver-driven rather than timer-backed polling.

M4 `MET` — `src/mpeg/decoder.{c,h}` is a clean decoder owner. It orders injected known-state preparation before initialization, owns picture/decode call and destroy ordering, validates caller-supplied sequence/feed/resource bounds, and does not expose H1's reset laboratory toggle as product configuration.

M5 `MET` — local owner stop does not synthesize EOF inside an active decoder data callback. The callback continues to report real Transport data/exhaustion/failure while call ownership is active; pending stop is honored only after the decoder call returns. True finite producer exhaustion remains real exhaustion.

M6 `MET` — payload bytes actually consumed are accounted separately from padded/aligned transfer bytes where alignment is required. Queue/feed/dimension/resource values remain explicit caller authority; no unqualified production defaults were invented.

M7 `MET` — unexpected decoder return without requested owner stop is classified as failure, decoder resources are fenced from reclamation while decoder-call ownership is live, and deterministic fixtures cover ordering, bounds, errors, stop-during-active-call, and lifetime interleavings.

M8 `MET` — A001 RFB and A002 AUDIO/common-clock behavior remained regression-stable in the qualified host set. This packet did not implement exact generation orchestration, Pi producer/capture/suppression, presentation/compositor, first-presentation epoch arm, scheduler/drop/calibration, application orchestration, or hardware qualification.

## Evidence gaps intentionally left to other owners

PENDING_LOCAL=Foreman-owned canonical test/build registration and manifest plumbing for the new MPEG files; generated source-dictionary portal reconciliation; settled-tree canonical CI/linked-build packaging as required by Foreman; independent Validation disposition after consuming this completion handoff

HARDWARE_PENDING=physical PS2 qualification for A003 reconstructed behavior; no physical PASS is claimed by this Reconstruction round

These gaps do not reduce the A003-P1 Reconstruction source objective below `MET`; the governing packet explicitly assigns generated portal, canonical test/build registration, and CI evidence to Foreman and keeps physical qualification separate.

## Known-defect accounting

- production/source defects found after final exact review: `NONE`;
- Reconstruction-owned fixture defect corrected: missing host `kernel.h` include in `transport_mpeg_test.c`;
- Foreman-owned integration lag observed: canonical build/test registration for new MPEG source/tests;
- Validation findings opened against this completed packet before handoff: `NONE`;
- scope leakage into A003-P2/A004/hardware: `NONE`.

## State / contract effects

CONSUMED_WORK_LOG_CONTRACT_REVISION=0005
CONSUMED_FOREMAN_STATE_REVISION=0012
CONSUMED_RECONSTRUCTION_CONTRACT_REVISION=0005
CONSUMED_A003_AUDIT_REVISION=0001
RECONSTRUCTION_STATE_WRITE=NONE
FOREMAN_STATE_WRITE=NONE

## Exact next pickup

Foreman should consume this immutable Reconstruction handoff, independently inspect M1-M8 against live source, perform only Foreman-owned canonical test/build/generated-dictionary/living-map/CI integration, and then allow independent Validation to resume on settled authority. If concurrent support work has already prepared some of that integration or the A003-P2 successor packet, reconcile it against this completed handoff rather than rewriting immutable history.

Reconstruction A003-P1 work is complete. Do not extend this round into A003-P2.
