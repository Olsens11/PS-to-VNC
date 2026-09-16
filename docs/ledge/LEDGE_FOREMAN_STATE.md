# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0010
RECORDED_AT=2026-09-16T12:27:51-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0004
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0027
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A002_AUDIT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision consumes the completed A002 Transport AUDIO-channel Reconstruction handoff, post-handoff Architecture/Dictionary/Integration support records, a concurrent Validation self-pause during the moving integration tree, the Foreman-owned canonical integration/evidence performed after the baton returned, and the diagnostics sentinel that correctly classified the pre-reconciliation project-check red as mechanical integration debt rather than product behavior failure.

The live branch was re-read immediately before this state write at `13f7e6cc13d839a3b9183928da2185d9a29abf9f`. Unknown external/Pi-local dirty work is outside this GitHub-native surface and is neither overwritten nor declared absent.

A001 machine/source PASS remains independent Validation authority. A001 physical PS2 qualification remains `HARDWARE_PENDING`. Nothing below converts Foreman machine evidence into Validation PASS or physical qualification.

## Current Foreman phase

`A002_PCM_PLAYBACK_CORE_PACKET_ISSUED`

The A002 Transport AUDIO-channel A1-A8 packet is Reconstruction-complete and Foreman-accepted as `MET` after direct source/test inspection. The worker established a real Transport-owned AUDIO logical byte source behind the existing sole physical receiver, so the next dependency-ordered A002 responsibility is the PCM/AUDSRV playback core that consumes that seam.

The next packet deliberately stops before playback-worker allocation/thread lifecycle and before startup-reservoir/common-clock gating. Those concerns depend on still-unqualified tuning/resource policy and can follow once the byte-consumer/playback semantics are clean and testable.

## Reconstruction result consumed

WORK_ITEM_KEY=`a002-audio-clock`
WORKER=`interactive`
WORKER_LOG_COMMIT=`6b3cb3ebab4c5ec84d3e514090c35350eab23370`
WORKER_LOG=`docs/ledge/work-log/20260916T103630-0400__reconstruction__a002-audio-clock__interactive.md`
WORKER_RESULT=`FOREMAN_GOAL_RESULT=MET`

Substantive worker commits inspected:

- `95e4a816...` / `41b9297a...` — bounded logical AUDIO byte-channel storage;
- `f357b5c5...` — explicit AUDIO Transport configuration/result vocabulary;
- `208de2c5...` / `c39fda05...` — sole-receiver runtime dispatch with independent AUDIO queue/credit/activity state;
- `20758568...` / `7dd9b56c...` — opt-in logical AUDIO bridge preserving the existing RFB-only open path;
- `f09490c3...` — deterministic AUDIO behavior fixture;
- `c8401803...` — RFB/lifecycle regression preservation;
- exploratory waiter state `23fd27de...` reverted by `74b9620a...`;
- `f61fe39f...` / `894d0c52...` / `d5934757...` — coherent AUDIO waiter-lifetime/reclaim fence and terminal-path assertions.

Files directly inspected included `src/transport/audio_channel.{c,h}`, `src/transport/transport.h`, `src/transport/runtime.{c,h}`, `src/transport/bridge.{c,h}`, `tests/unit/transport_audio_test.c`, canonical `tests/Makefile`, clean linked-build authority, and A001/A002 audit authority.

### A1-A8 Foreman disposition

A1 `MET` — Transport owns an explicit narrow AUDIO-channel configuration boundary for queue capacity, initial credit, batching/flush, and credit-return enablement. The existing eight A001 Transport session fields remain intact; no application magic defaults or H1 61-field public profile were introduced.

A2 `MET` — exactly one Transport receiver thread owns the physical receive loop and dispatches valid RFB and AUDIO DATA to independent logical paths. No competing socket reader was added.

A3 `MET` — AUDIO owns separate storage, occupancy, activity/wait state, earned-credit batching, and credit-return accounting. Credit is earned only for bytes actually dequeued by the AUDIO consumer.

A4 `MET` — zero-length AUDIO DATA is an ordered one-shot producer-done marker; producer-done plus empty queue yields normal exhaustion; later non-empty AUDIO payload is rejected and drives terminal failure rather than silently reopening the producer.

A5 `MET` — the bridge exposes bounded AUDIO reads plus status/activity/wait primitives sufficient for a future audio owner to distinguish availability, normal finite exhaustion, stop, and terminal Transport failure without reaching into Transport internals.

A6 `MET` — existing A001 RFB/receiver/quiesce semantics remain present; fatal EOF/protocol/overflow paths wake AUDIO observers and preserve terminal state. Release refuses to reclaim a signaled-but-not-returned AUDIO waiter, preserving the worker-lifetime fence.

A7 `MET` — deterministic tests cover AUDIO-only reads, bounded partial reads, RFB/AUDIO interleaving through one receiver, independent initial credit and dequeue-driven batching/flush, finite marker/exhaustion/post-marker rejection, overflow/drain/terminal failure, EOF/invalid/stop wakeups, one-receiver identity, and reclaim-fence behavior. Existing Transport/RFB fixtures remain regression authority.

A8 `MET` — no AUDSRV/LIBSD playback, PCM worker, startup reservoir, audio media-clock callsite, MPEG/presentation, application orchestration, inherited H1 receive-poison repair, or hardware qualification was introduced.

No product-behavior correction packet is required.

## Concurrent/support movement consumed

Architecture packet-scout commit `72d4b5ed887a68d7975b4ce7b52272ab463576f5` is non-authoritative support evidence only. It independently identified the same dependency order: once the AUDIO seam is accepted, the next coherent A002 owner is a clean audio component consuming Transport bytes and preserving audited AUDSRV ordering/lifetime semantics.

Dictionary preparation commit `3e4560b8fe99291c6352b09956ac04680f235f0f` and integration record `82d978d30393038c1cab64fcd1127147db341c76` were consumed as support records, not as behavior authority.

Validation commit `defbbe8635eb510e5f4f20fddb2082aa61833fab` landed while Foreman integration was moving and correctly self-paused instead of independently judging a non-settled tree. No Validation PASS is inferred from that record.

Diagnostics sentinel commit `13f7e6cc13d839a3b9183928da2185d9a29abf9f` inspected the integration-trigger CI and classified the red project-check as mechanical source-topology/dictionary coverage on the pre-generated tree, not a product defect. It also confirmed host-unit, PS2 compile, and PS2 link/reproducibility were green and routed the remaining topology coverage back to Foreman. The deterministic dictionary child subsequently populated the missing AUDIO file coverage, so this state write triggers the required settled-tree recheck.

## Foreman-owned integration and evidence

Foreman performed only canonical non-behavioral integration/evidence work:

- `b1b9a68112cb684bd1f1ce6c802dbd3acb907215` — registered `transport_audio_test` in canonical `make -C tests unit` and linked `audio_channel.c` into the existing Transport runtime fixture;
- `5913401208aa3ac3c6c09ae34881e1d6198804c0` — linked `src/transport/audio_channel.c` as `transport_audio_channel.o` in the clean PS2 build graph;
- `7ddc22068692474662f4a39f4980c356f3425290` — adopted `src/transport` into deterministic dictionary reconciliation;
- `3bd92974e988b68278aac3fa8de7efb9f4a60121` — allowed the reconciliation workflow to stage `src/transport/SYMBOLS.md`;
- `688dc1031e83bfa1734f286a91882af3dd4d02f5` — exact deterministic dictionary-reconciliation trigger;
- generated child `88cbb73332d5f20b24bf9cb8a9a05612b53169b7` — reconciled current Transport definitions and regenerated the canonical source-dictionary portal.

Deterministic reconciliation reported:

- `DICTIONARY_RECONCILED=src/transport before=422 removed=11 added=192`;
- other selected domains removed/added `0`;
- `DICTIONARY_RECONCILIATION_REMOVED=11`;
- `DICTIONARY_RECONCILIATION_ADDED=192`;
- `SOURCE_DICTIONARY_CHECK_MODE=LONG`;
- `DEFINITION_SCOPE_COUNT=64`;
- `SOURCE_DICTIONARIES=PASS`;
- generated portal `src/transport` count `603`.

Workflow run `35121707966` at trigger authority `688dc103...` reports:

- canonical host-unit `PASS`;
- `transport_runtime_test: PASS` with `audio_channel.c` linked into that regression fixture;
- `transport_audio_test: PASS` in the canonical unit graph;
- existing Transport protocol/channel/bridge, RFB, CONFIG/profile, common-clock, application, input/UI, and other registered host fixtures remained passing;
- pinned PS2 compile `PASS`;
- clean PS2 linked-build `PASS`;
- `src/transport/audio_channel.c` explicitly compiled to `transport_audio_channel.o` and linked into the ELF;
- pristine linked ELF SHA256 `668215f9c4bd1eef0cefbda9dbf342bc7b22bbe871c0b2256e8255b9f3cc84c6` on both builds;
- `PT_LOAD_SEGMENTS=1`;
- PT_LOAD SHA256 `267f42a61ce0bdff47dfcaca9d363cd497ddf25bcc797a94a0e631dcda34cef1`;
- `PT_LOAD_BYTES=420488`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `ISSUE7_RUNTIME_IDENTITY_LINKED=YES`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The trigger-tree ordinary strict dictionary and project-check jobs were expectedly red before generated Transport dictionary coverage landed. The dictionary reconciliation job then passed strict long checking and produced `88cbb733...`. This state write occurs after that generated child and intentionally triggers a settled-tree workflow; Foreman must inspect that result before closing this round.

PENDING_LOCAL=settled-tree workflow after state revision 0010; independent Validation disposition for integrated A002 Transport AUDIO/common-clock authority; later concrete PS2 AUDSRV/media-clock runtime binding and physical qualification
HARDWARE_PENDING=A001 physical PS2 qualification remains pending; no A002 physical qualification is claimed

## Dependency conclusion for the next Reconstruction packet

A002 audit revision 0001 requires PCM playback to consume the Transport-owned AUDIO logical channel, preserve `audsrv_wait_audio(bytes)` -> `audsrv_play_audio(bytes)` ordering, count consumption only after successful playback submission, treat producer-done plus empty queue as normal finite completion, preserve resident-ELF AUDSRV/LIBSD service lifetime, and stop/mute per session without `audsrv_quit()`.

The newly accepted Transport seam now supplies the required clean byte source and terminal/exhaustion distinction. CONFIG already supplies a narrow immutable PCM profile (`rate_hz`, `channels`, `bits_per_sample`, `volume_percent`). Therefore the next coherent unit is a synchronous/testable PCM playback core and AUDSRV service boundary.

Worker thread stack/priority/chunk sizing, startup-reservoir policy, and common-clock presentation gating remain a subsequent packet. The current production CONFIG intentionally does not expose H1's laboratory audio worker/chunk/reservoir tuning. This packet must not reintroduce those knobs as guessed defaults merely to make a worker loop convenient.

## Fresh interactive Reconstruction packet

WORK_ITEM_KEY=`a002-audio-clock`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`13f7e6cc13d839a3b9183928da2185d9a29abf9f`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md:0001`
ASSIGNING_FOREMAN_STATE=`0010`

### Objective

Reconstruct A002's clean PCM/AUDSRV playback core on top of the accepted Transport AUDIO logical-consumer seam and immutable PCM profile. Establish exact per-session format/volume setup, finite Transport consumption, strict `wait_audio(bytes)` then `play_audio(bytes)` submission ordering, truthful post-submit byte accounting, resident-service lifetime semantics, and deterministic stop/error handling without yet creating the playback worker thread or startup/presentation timing policy.

This packet should leave a complete reusable synchronous audio-playback component that a later lifecycle/timing packet can run in a session worker without changing its byte-consumption or AUDSRV correctness semantics.

### Required deliverables

1. **Clean audio owner and narrow dependencies.** Add the smallest coherent clean audio component (a new `src/audio/` domain is appropriate if justified by current topology policy). It consumes only the immutable `pstvnc_config_pcm_profile_t`, the public/narrow Transport AUDIO bridge, and a narrow injected AUDSRV/service operation boundary. Audio must not inspect Transport queue internals, mutate Transport diagnostics, or make CONFIG/application own playback state.

2. **Resident service contract.** Represent the audited service lifetime explicitly: LIBSD/AUDSRV is ELF-resident service authority; session playback may initialize/use the service as required by the proven API contract, set the session format/volume, and stop/mute the current stream at session completion, but this component must not call or expose per-session `audsrv_quit()`. Keep host-testable service operations injectable; do not hide a second mutable service owner in application code.

3. **Exact immutable PCM setup.** Apply rate/channels/bits/volume from the decoded PCM subprofile exactly. Do not invent fallback values. If an AUDSRV-facing representation cannot express a profile value, reject/fail before playback rather than silently coercing it.

4. **Finite Transport consumption without busy polling.** Consume AUDIO bytes only through the accepted public Transport bridge. Bounded nonzero chunks supplied by the caller/component buffer may be processed, but this packet must not invent a production worker chunk-size default. On temporary unavailability use the Transport activity/status seam rather than spin polling. Treat Transport `EXHAUSTED` as normal finite producer completion; distinguish `STOPPED`, `CLOSED`, invalid use, and terminal `FAILED` as separate outcomes.

5. **Strict AUDSRV submission ordering and accounting.** For every nonzero byte chunk selected for playback, invoke the equivalent of `audsrv_wait_audio(bytes)` before `audsrv_play_audio(bytes)`. Do not submit if the wait fails. Count/report bytes as successfully submitted/consumed only after the play call succeeds. A wait or play failure must not falsely advance accounting or consume a different semantic result.

6. **Session completion/stop semantics.** Normal Transport exhaustion and explicit stop/failure paths must terminate deterministically. Session cleanup stops/mutes current audio through the narrow service operation and preserves first-error semantics where cleanup itself also fails. Never use `audsrv_quit()` as a session cleanup mechanism, and never continue playback after a terminal outcome.

7. **Deterministic host tests.** Add behavior-specific tests proving at minimum:
   - exact format/volume values and setup ordering;
   - `wait_audio(bytes)` occurs before each `play_audio(bytes)`;
   - one partial chunk and multiple chunks preserve byte order/length;
   - no zero-byte submission;
   - temporary Transport unavailability uses activity/wait behavior without unbounded busy spin;
   - normal producer-done + empty / `EXHAUSTED` retires as success;
   - Transport stop and terminal failure are distinguishable from normal exhaustion;
   - AUDSRV wait failure prevents play and leaves successful-byte accounting unchanged;
   - AUDSRV play failure does not falsely count the chunk;
   - session stop/mute executes once on completion/error as specified;
   - cleanup failure preserves the first meaningful error rather than overwriting it;
   - no per-session quit operation is reachable/exposed.

8. **Boundary discipline.** Obey current source synopsis/naming/topology/dictionary rules. Perform Reconstruction-owned source-side domain/dictionary adoption if `src/audio/` is created, but leave generated portal and canonical test/build registration to Foreman. Do not allocate/create the playback thread or its stack, choose production chunk/stack/priority values, implement startup-reservoir policy, wait on/arm the common media clock from audio, add application orchestration, begin MPEG/presentation, repair the inherited H1 receive-poison defect, or claim hardware qualification.

### Acceptance criteria

P1. A coherent clean audio playback owner exists and depends only on narrow PCM profile, public Transport AUDIO seam, and a testable AUDSRV/service boundary.

P2. Resident-service semantics are explicit and session cleanup cannot call `audsrv_quit()`; format/volume are applied from immutable profile authority without defaults.

P3. AUDIO bytes are consumed through the Transport bridge with bounded/non-spinning unavailability handling and normal finite `EXHAUSTED` completion distinct from stop/failure.

P4. Every submitted chunk obeys wait-before-play ordering, and byte accounting advances only after successful play submission.

P5. Wait/play/Transport/setup/cleanup failures are deterministic, preserve meaningful first-error semantics, and do not permit further playback after terminal outcome.

P6. Deterministic host tests prove setup, ordering, chunking, temporary unavailability, finite completion, stop/failure distinction, failure injection, truthful accounting, cleanup, and no-quit behavior.

P7. Existing A001 RFB/Transport, completed A002 CONFIG/profile, AUDIO channel, and common media-clock behavior remain stable; no hidden production tuning defaults are introduced.

P8. No playback-worker allocation/thread lifecycle, startup-reservoir/common-clock presentation callsite, MPEG/presentation, application orchestration, receive-poison repair, or hardware qualification is introduced.

### Preserved invariants / non-goals

- Transport remains sole owner of the physical PSTV receiver and AUDIO logical queue/credit state.
- Audio is a Transport consumer, never a competing socket reader.
- `EXHAUSTED` means the ordered producer has finished and the AUDIO queue is empty; it is normal completion.
- `audsrv_wait_audio(bytes)` must precede `audsrv_play_audio(bytes)` for each submitted chunk.
- Successful-byte accounting occurs only after successful playback submission.
- AUDSRV/LIBSD service lifetime is ELF-resident; session cleanup stops/mutes but does not quit/reinitialize the resident service by policy invention.
- The common media clock remains one session timing origin, but this packet does not connect audio playback to it yet.
- Do not resurrect H1's experimental chunk/worker/reservoir knobs as public CONFIG or hidden constants.
- A001 physical status remains `HARDWARE_PENDING`.

### Blocker burden

Lack of physical PS2 hardware is not a blocker for the synchronous playback contract because Transport and AUDSRV operations are deliberately injectable for host tests.

Absence of final production chunk/thread/reservoir tuning is not a blocker. Keep the playback core buffer/chunk boundary explicit to its caller/test harness and leave resource/timing policy for the next packet rather than guessing values.

If exact PS2SDK AUDSRV representation or API binding cannot be proven without importing the concrete PS2 service adapter into this packet, establish the narrow operation contract and deterministic behavior tests, document the concrete binding as `PENDING_LOCAL`, and return `PARTIAL` only for the missing adapter. Do not weaken ordering/lifetime/accounting semantics or invent an API shape contradicted by repository evidence.

### Stretch only if P1-P8 are complete

No behavioral stretch target is assigned. Keep the packet focused so the subsequent worker-lifecycle + startup/presentation-timing tranche has a stable playback core to consume.

## Exact next Foreman pickup

After Reconstruction returns, inspect the actual audio source/tests and criterion-by-criterion P1-P8 result; verify that no competing receive/service owner or hidden tuning defaults appeared; perform only canonical test/build/dictionary/topology evidence; then issue the dependency-ordered playback-worker + startup-reservoir/common-clock gating packet if coherent.

Validation should independently consume the integrated Transport AUDIO/common-clock authority and later the audio tranche. Foreman evidence is not Validation PASS.
