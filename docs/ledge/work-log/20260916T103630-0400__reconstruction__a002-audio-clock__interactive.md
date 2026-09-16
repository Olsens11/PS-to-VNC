# A002 Transport AUDIO-channel Reconstruction — interactive

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T10:36:30-04:00
COMPLETED_AT=2026-09-16T11:36:35-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=b5818fcfe67f9759ef3866a7e20e3b9d8fa315ec
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

FOREMAN_GOAL_RESULT=MET
ASSIGNING_FOREMAN_STATE_REVISION=0009
CONSUMED_WORK_LOG_CONTRACT_REVISION=0004
CONSUMED_GLOBAL_STATE_REVISION=0027

## Objective and authority consumed

Executed only Foreman revision 0009's active A002 A1-A8 Transport AUDIO-channel packet as one sustained interactive Reconstruction round. The exact starting branch authority was recovered from repository history as `b5818fcfe67f9759ef3866a7e20e3b9d8fa315ec` (`docs(foreman): issue A002 audio channel packet`, committed at 2026-09-16T10:35:22-04:00), the newest branch commit before this shift's exact `STARTED_AT`.

Authority consumed during the round included:

- Foreman state revision 0009 and A002 audit revision 0001;
- work-log contract revision 0004 after its concurrent governance repair;
- Validation's self-pause on active Reconstruction overlap, which opened no product finding;
- diagnostics commit `312e20a0a2a1068d45477b5d80a2d6192f474500`, which classified intermediate CI red status as mechanical linkage/registration plus dictionary/topology lag and found no confirmed architecture regression;
- Continuity/global work-state revision 0027 and immutable Continuity log `48f1bd29545f9c244a641ce5d3ef2c3b88f39357`, which preserved A002 as RECONSTRUCTING until this handoff and routed post-handoff canonical registration/build/dictionary/topology/evidence work to Foreman.

The packet boundary remained authoritative throughout: preserve one physical PSTV receive owner and existing RFB/CONFIG/common-clock behavior; reconstruct only the Transport-owned AUDIO logical byte seam; do not begin AUDSRV/PCM playback, audio worker lifecycle, media-clock audio callsites, MPEG/presentation, application orchestration, H1 receive-poison repair, or hardware qualification.

## Reconstruction work performed

### Narrow AUDIO Transport authority

Added `pstvnc_transport_audio_channel_config_t` as a five-field explicit caller/profile-owned Transport value boundary:

- queue capacity;
- initial credit;
- credit batch;
- flush-on-empty policy;
- credit-return enable.

No production tuning values or application defaults were invented, and the previously completed 19-field CONFIG wire/profile contract was not expanded or rewritten by this packet. Existing `pstvnc_transport_session_config_t` remains the A001 eight-field Transport authority.

### One receiver / independent AUDIO logical channel

Added `src/transport/audio_channel.c` and `src/transport/audio_channel.h` as a bounded byte-ring owner with one-shot producer completion. Extended the existing Transport runtime receiver so the same sole physical `receive_frame()` owner dispatches:

- RFB DATA to the existing logical RFB path;
- AUDIO DATA to the independent AUDIO queue when AUDIO is explicitly enabled.

No second socket reader, physical receive path, or receiver thread was introduced. Unsupported/invalid DATA channel/kind combinations remain fail-closed.

### Independent queue, wakeup, and credit accounting

AUDIO now owns session-local queue storage, queue semaphore, activity semaphore/sequence, credit-pending accumulator, initial/batch/flush/return policy, and producer-done state independently from RFB. Only bytes actually dequeued by the logical AUDIO consumer earn channel-2 CREDIT. RFB credits remain channel-1 facts and are not cross-accounted with AUDIO.

### Finite producer completion

Zero-length AUDIO DATA is the ordered finite-producer marker. It is accepted once, exposes producer-done, preserves already-queued bytes for normal draining, and causes producer-done + empty to report normal `PSTVNC_TRANSPORT_EXHAUSTED`. Duplicate completion or later non-empty AUDIO DATA is rejected rather than reopening the producer.

### Narrow logical consumer bridge

Added an opt-in `session_open_with_audio()` path while preserving the existing RFB-only `session_open()` behavior. The AUDIO bridge exposes only bounded logical operations:

- read available bytes;
- query availability / producer-done status;
- activity snapshot / wait.

The bridge returns distinct `WOULD_BLOCK`, `EXHAUSTED`, `STOPPED`, `CLOSED`, and `FAILED` outcomes as applicable and never exposes the adopted physical descriptor or queue internals.

### Terminal wake and reclaim lifetime fence

Receiver EOF/fatal convergence and application stop wake the independent AUDIO activity rendezvous without being classified as normal producer exhaustion.

Final A6 review found a narrower reclaim race: merely signaling an AUDIO waiter did not prove that waiter had returned through the queue-lock path before its activity semaphore could be reclaimed. The committed correction makes `audio_activity_wait_armed` a protected three-state rendezvous:

- `0` = no waiter owns the rendezvous;
- `1` = waiter armed and not yet signaled;
- `2` = signaled, but waiter has not yet returned through the AUDIO queue lock.

Activity/terminal producers move `1 -> 2` exactly once; the waiter alone clears `2 -> 0` after reacquiring the queue lock. `pstvnc_transport_runtime_release()` refuses reclamation while that protected state is nonzero. Because receiver completion is already required before release, no new AUDIO wait can arm after the reclaim check. A deterministic fixture case proves release preserves all waiter-visible resources while state `2` is live and succeeds only after the waiter-lifetime state is cleared.

## Reconstruction-owned commits

Substantive Reconstruction commits in this sustained round include:

- `95e4a816bf548f6ddf1d5e389b2bddc46626278b` / `41b9297aec93c9f38aa12461ec201f6453970bea` — introduce the bounded logical AUDIO channel source/header;
- `f357b5c5576fed699aaf6296d069a9deed1d7c90` — explicit AUDIO Transport profile values/result vocabulary;
- `208de2c599ae9344fa2382c71eba9666fdce477d` / `c39fda05ee1f646fb3f9b8dbb0e7839cb1259df4` — extend the sole-receiver runtime with independent AUDIO queue/credit/activity state;
- `20758568d5ea53f8d4300635dbe23e3fb8f45f27` / `7dd9b56c6e4ca4f698b7325918efd300271eb487` — add the opt-in logical AUDIO bridge while preserving the existing RFB-only opening path;
- `f09490c3baace4f24d0a0d05895902310961afbb` — direct deterministic A1-A7 AUDIO behavior fixture;
- `c8401803adf55c11356c824b43d313baf95762e6` — preserve existing Transport bridge/session/RFB lifecycle fixture coverage against the expanded private runtime API;
- `23fd27de6185932d0c317f1743ccad0c9c1c423a` followed by `74b9620a19ae4549e422bc6a58fe9a05db2fe379` — exploratory waiter-lifetime header state introduced then deliberately reverted rather than leaving a partial implementation;
- `f61fe39fd69a63811ab16e735af60c66544e18a8` / `894d0c5230187731112ee6dc511026edf9a861ee` — coherent AUDIO waiter-lifetime/reclaim fence;
- `d593475729a9b0fdfc5f45669cc270f68b5e66e1` — deterministic reclaim-fence and terminal-waiter assertions.

Concurrent governance/diagnostics/continuity commits were consumed as authority rather than counted as Reconstruction-owned work.

## Criterion-by-criterion disposition

### A1 — MET
Transport exposes explicit narrow AUDIO-channel queue/credit configuration. No hidden application defaults or H1 giant public profile were introduced. Qualification tuning remains caller/profile authority.

### A2 — MET
Exactly one Transport-owned receiver thread remains the only physical receive owner and dispatches both RFB and AUDIO DATA. No competing socket read path was added.

### A3 — MET
AUDIO storage, synchronization, activity state, and credit accounting are independent from RFB. Only bytes actually dequeued by the AUDIO consumer return AUDIO credit.

### A4 — MET
Finite producer completion is explicit and deterministic: zero-length AUDIO DATA is accepted once, producer-done is observable, queued bytes remain drainable, done + empty reports normal exhaustion, and later payload is rejected.

### A5 — MET
The narrow logical bridge supports bounded consumption and distinct live/no-data, normal exhaustion, stop, close, and failure outcomes without exposing Transport internals or the physical socket.

### A6 — MET
Existing A001 RFB receiver/quiesce ownership remains separate and unchanged in policy. Invalid kind/channel behavior remains fail-closed. EOF/fatal/stop wake AUDIO waiters, and the final three-state activity rendezvous prevents reclamation of waiter-visible resources until the signaled waiter has returned through the protected queue-lock path.

### A7 — MET
Deterministic test source covers RFB-only regression, interleaved RFB/AUDIO through one receiver, independent initial/earned credit, partial bounded reads, finite marker/drain/exhaustion, post-marker rejection, overflow with committed-byte drain, invalid kind/channel failure, EOF/fatal/stop wakeups, one-receiver identity, and the final waiter/reclaim lifetime fence.

### A8 — MET
No AUDSRV/PCM playback worker, startup-reservoir timing, media-clock audio callsite, MPEG/presentation behavior, application orchestration, or hardware qualification was introduced. The genuinely new AUDIO files live inside the already-clean `src/transport/` domain whose topology authority already owns logical-channel storage/flow control. Canonical dictionary reconciliation and build/test registration remain the explicitly routed Foreman integration chores described below; they are not used here to manufacture a worker behavior failure.

## Evidence and checks

### Direct Reconstruction evidence

Before the final waiter-lifetime refinement, the direct strict-host AUDIO runtime fixture executed successfully as:

`transport_audio_test: PASS`

That run covered the same receiver/queue/credit/marker/terminal matrix except for the subsequently added explicit release-fence assertion.

The final A6 source at `d5934757...` was then compiled by canonical GitHub host-unit under `-O2 -std=c99 -Wall -Wextra -Werror -pedantic`; compilation reached link successfully. The canonical host-unit failure occurs only because Foreman has not yet added `src/transport/audio_channel.c` to the existing `transport_runtime_test` link line. Undefined references are exclusively the new AUDIO-channel functions. Existing canonical Transport protocol/RFB-channel/bridge/physical-stream tests shown before that link point pass.

Pinned `ps2-compile` also reports PASS on the intermediate/final behavior tree. Canonical `ps2-link` is red for the same missing AUDIO object registration, not for a demonstrated product-source compile defect.

### Dictionary / topology evidence

Strict `dictionary-long` correctly reports generated/integration lag rather than a behavior defect:

- missing coverage for new `src/transport/audio_channel.c/.h` and new AUDIO/runtime/bridge definitions;
- three stale pre-AUDIO private runtime rows: `pstvnc_transport_runtime_publish_activity_locked`, `pstvnc_transport_runtime_return_consumed_credit`, and `pstvnc_transport_runtime_signal_activity`.

Diagnostics and Continuity route canonical registration, build, dictionary/topology reconciliation, and generated portal evidence to Foreman after this handoff. This worker does not falsify a strict dictionary PASS or take over generated portal/build registration.

PENDING_LOCAL=Foreman canonical registration of audio_channel.c into host/runtime and clean PS2 linked build; registration/execution of transport_audio_test in the canonical unit graph; strict source-dictionary reconciliation/generated portal evidence; independent Validation disposition
HARDWARE_PENDING=A001 current-DUT physical PS2 qualification remains pending; no A002 hardware qualification claimed

## Preserved behavior / non-goals

- existing A001 sole physical receiver ownership preserved;
- existing RFB queue/credit/quiesce behavior preserved as separate logical-channel authority;
- completed CONFIG/profile behavior not expanded or defaulted;
- completed common-media-clock source untouched;
- no direct application physical receive path;
- no AUDSRV/LIBSD calls, PCM playback thread, reservoir policy, media-clock wait/arm callsite, MPEG/presentation, or A003 work;
- no H1 receive-poison repair;
- no hardware PASS claim.

## Exact next pickup

FOREMAN: consume this `FOREMAN_GOAL_RESULT=MET` handoff, inspect the exact settled AUDIO source/tests, perform only Foreman-owned canonical registration/link/dictionary/topology/generated-portal/evidence work, and disposition A1-A8 before issuing the next dependency-ordered A002 PCM/AUDSRV packet.

VALIDATION: after Foreman produces a coherent integrated tree/evidence bundle, independently judge the settled A002 AUDIO-channel authority; do not infer PASS from this Reconstruction result alone.

RECONSTRUCTION: stop here. Do not begin PCM/AUDSRV playback, MPEG/presentation, or another A002 behavior packet without fresh Foreman authority.
