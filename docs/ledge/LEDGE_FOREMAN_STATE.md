# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0011
RECORDED_AT=2026-09-16T13:13:45-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_WORK_LOG_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0028
BASED_ON_VALIDATION_STATE_REVISION=0006
BASED_ON_VALIDATION_FINDINGS_REVISION=0005
BASED_ON_A002_AUDIT_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This interactive Foreman revision consumes the completed A002 synchronous PCM/AUDSRV playback-core Reconstruction handoff, directly inspects the product source/tests and the relevant current PS2SDK EE AUDSRV API behavior, performs only canonical integration/evidence work, consumes the generated dictionary child and concurrent support records without treating them as behavior authority, and issues the next bounded A002 lifecycle/startup-timing packet.

The live branch was re-read immediately before this state write at `ad1ee46731fd0413f2b3226cb53f7b528ac4a3ad`. Unknown external/Pi-local dirty work remains outside this GitHub-native surface and is neither overwritten nor declared absent.

A001 machine/source PASS remains independent Validation authority. A001 physical PS2 qualification remains `HARDWARE_PENDING`. No statement below promotes A002 to independent Validation PASS or physical qualification.

## Current Foreman phase

`A002_AUDIO_WORKER_CLOCK_PACKET_ISSUED`

The A002 synchronous PCM/AUDSRV playback-core P1-P8 packet is Reconstruction-complete and Foreman-accepted as `MET` after direct source/test inspection. The clean tree now has a synchronous PCM owner that consumes only the public Transport AUDIO seam, applies exact immutable PCM values, preserves resident AUDSRV service lifetime, performs wait-before-play submission, and preserves truthful post-submit accounting/error cleanup.

The next dependency-ordered responsibility is session-scoped playback-worker lifecycle plus startup-reservoir and common-clock presentation gating. This packet deliberately wraps the accepted synchronous core rather than rewriting it, and it stops before application orchestration or any A003 MPEG/video presentation behavior.

## Reconstruction result consumed

WORK_ITEM_KEY=`a002-audio-clock`
WORKER=`interactive`
WORKER_LOG_COMMIT=`2f1e71d9f277417fbe0292ed7aa52eb2c12abf32`
WORKER_LOG=`docs/ledge/work-log/20260916T123335-0400__reconstruction__a002-audio-clock__interactive.md`
WORKER_RESULT=`FOREMAN_GOAL_RESULT=MET`

Substantive worker commits inspected:

- `103791438a4afae42edd67224466f0767a37b790` — define synchronous PCM playback core;
- `e1d6a8f60e54cf933d2b326182e184605850cb5f` — implement playback core;
- `ad36d3a88da9c3a0bc4520975ba6f8c6219afbf2` — declare resident AUDSRV adapter;
- `c774fdda51acaa4b107793ee3b81e6eaf9c78b0c` — bind playback core to resident AUDSRV;
- `2ce299ca6af66b0cb6c1accb69137a705949915d` — host AUDSRV stub;
- `206a07162d8d14ea01e189c533ce73a02cf1a643` — deterministic playback semantics fixture;
- `87af39599449a68849b69c45513e4d4dfcd95a42` — concrete AUDSRV adapter fixture;
- `d21eb53ce41859ba1d04cfa94846c0d665709927` — clean `src/audio` dictionary;
- `7d00270af88149ab7c0b61d771cf6f5ef7b65f72` — clean audio-domain topology adoption;
- `2f1e71d9f277417fbe0292ed7aa52eb2c12abf32` — immutable Reconstruction handoff.

Files directly inspected included `src/audio/playback.{c,h}`, `src/audio/audsrv_service.{c,h}`, `src/audio/SYMBOLS.md`, `tests/unit/audio_playback_test.c`, `tests/unit/audio_audsrv_service_test.c`, `tests/unit/audio_host_stubs/audsrv.h`, current `src/config/profile.{c,h}`, `src/transport/bridge.h`, `src/media/clock.h`, the A002 audit, and current PS2SDK EE `audsrv` implementation authority.

### P1-P8 Foreman disposition

P1 `MET` — `src/audio` is a coherent narrow clean owner. The synchronous core consumes only immutable `pstvnc_config_pcm_profile_t`, the public Transport AUDIO bridge, and injected service operations; it does not reach into Transport queue/runtime internals.

P2 `MET` — resident-service semantics are explicit. The concrete adapter exposes init/format/volume/wait/play/stop only; no session `audsrv_quit()` path is exposed. Current PS2SDK EE authority confirms repeated `audsrv_init()` is idempotent after RPC binding and that `audsrv_set_volume()` accepts the public 0-100 scale used by `volume_percent`.

P3 `MET` — the playback loop uses only the accepted Transport AUDIO bridge, snapshots activity before reading, waits on Transport activity for temporary unavailability instead of busy-spinning, treats `EXHAUSTED` as normal completion, and keeps stop/closed/failure outcomes distinct.

P4 `MET` — every nonzero chunk executes service wait before play; successful byte/chunk accounting advances only after `play_audio()` reports exact full-byte success. Current PS2SDK EE authority confirms `audsrv_play_audio()` returns the total byte count actually sent, so exact-count acceptance is a valid fail-closed contract.

P5 `MET` — setup, Transport, wait, play, and cleanup errors terminate deterministically. Playback does not continue after a terminal outcome, and cleanup failure does not overwrite an earlier meaningful failure.

P6 `MET` — deterministic host coverage proves exact setup/order, partial/multiple chunks, no zero-byte submission, activity rendezvous on temporary unavailability, normal finite exhaustion, stop/closed/failure distinction, wait failure, partial/negative play failure, truthful accounting, cleanup behavior, and absence of a quit operation.

P7 `MET` — the concrete AUDSRV adapter maps exact profile/service operations without creating a second lifetime owner; PS2-facing return semantics match the current EE wrapper behavior inspected by Foreman.

P8 `MET` — the worker did not create the playback worker/thread/stack lifecycle, choose production resource/timing defaults, implement startup reservoir or common-clock presentation gating, add application orchestration, begin MPEG/presentation, repair inherited receive-poison behavior, or claim hardware qualification.

No product-behavior correction packet is required from P1-P8.

## Foreman-owned integration and evidence

Foreman changed only canonical integration/evidence surfaces after the baton returned:

- `b2e93dceac75d80d29351bad380cae96d5f206f6` — allowed deterministic reconciliation to stage `src/audio/SYMBOLS.md`;
- `ba1ab97c1e8926d7ce05dc5505ce7e969601e945` — corrected the canonical strict clean-PS2 compile inventory so it actually compiles every currently adopted clean C translation unit, including earlier A002 CONFIG/media/Transport AUDIO source and the new audio source;
- `4c3628d21b190783757dc77f48a59a56d7bf3b24` — linked `audio_playback.o` and `audio_audsrv_service.o`, added `src/audio` includes, and linked PS2SDK `-laudsrv` in the clean PS2 graph;
- `1f4bb786a300b508e68b13472748b7a559d8562b` — registered `audio_playback_test` and `audio_audsrv_service_test` in canonical `make -C tests unit`;
- `473b4b111f5dbdd2b5db830269e23fc4c8d89387` — admitted `src/audio` to the clean source-topology checker;
- `37ae75801f7ab558c3b6b0cbb2e8a7879790421f` — adopted `src/audio` in deterministic dictionary reconciliation using the exact trigger commit message;
- generated child `6d25e2c0cea036d1f7cd57b42b4bf55df8d05548` — regenerated the canonical source-dictionary portal; the worker-authored local `src/audio/SYMBOLS.md` already matched discovered authority and required no generated correction;
- `edd0d7a78f315234119c9f81f9eee507d955bf24` — registered the new clean audio owner in the living file/service map.

The living-map replacement in `edd0d7a7...` also changed one old M0 sentence from “durable executable byte authority” to the semantically equivalent “durable binary authority.” This was incidental documentation drift, not a product/source behavior change. No historical evidence was rewritten and no behavior conclusion depends on that wording.

Concurrent dictionary-prep records `ea2df09c8247260e47eb03c224dbf9e87b0e1655` and `ad1ee46731fd0413f2b3226cb53f7b528ac4a3ad` are support evidence only. They correctly observe that the canonical dictionary/portal/living-map work is already settled and do not supersede Foreman behavior judgment.

## Canonical machine evidence

Integration-trigger workflow run `35126397761` at `37ae75801f7ab558c3b6b0cbb2e8a7879790421f` reports:

- canonical host-unit `PASS`;
- `audio_playback_test: PASS`;
- `audio_audsrv_service_test: PASS`;
- existing Transport/RFB/CONFIG/media-clock/application/input/UI fixtures remained passing;
- strict long source-dictionary audit `PASS` before generated portal reconciliation;
- corrected pinned PS2 compile `PASS`, including all adopted A002 CONFIG/media/Transport AUDIO and new `src/audio` translation units;
- clean PS2 linked build `PASS`;
- linked current-source reproducibility `PASS`;
- deterministic dictionary reconciliation `PASS`, producing child `6d25e2c0...`.

The clean linked build explicitly compiled:

- `src/audio/playback.c -> audio_playback.o`;
- `src/audio/audsrv_service.c -> audio_audsrv_service.o`;

and linked both objects with `-laudsrv`. Exact identity/evidence:

- `ELF_PRISTINE_SHA256=001e22e49d3b9c945f3e4a6d13862617fca1d91696745bdca78b7c03e594ac41` on both builds;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=4ba33bd1097800e7a926895ca5ce88e7332e1e22ad256139c4ebc08cc74e05c7`;
- `PT_LOAD_BYTES=423304`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `ISSUE7_RUNTIME_IDENTITY_LINKED=YES`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Generated portal authority now records `src/audio` COMPLETE with symbol count `82`.

Settled workflow run `35126537427` at `edd0d7a78f315234119c9f81f9eee507d955bf24` reports:

- host-unit `PASS`;
- strict long dictionary `PASS`;
- pinned PS2 compile `PASS`;
- PS2 linked build/reproducibility `PASS`;
- dictionary reconciliation skipped as expected for the ordinary living-map commit;
- documentation check `PASS`;
- development continuity/topology check `PASS`, including `src/audio`, clean-file count `68`, local-file coverage `PASS`, portal sync `PASS`, and total dictionary count `2552`;
- project-check overall `FAIL` only at the immutable work-log checker.

The remaining project-check error is governance/tooling mismatch, not A002 product behavior: work-log contract revision `0005` explicitly grandfathered `20260916T122559-0400__diagnostics__global-ci-regression__ci-sentinel.md` as the ninth exact exception, but current `scripts/work-log-check.py` still contains the revision-0004 eight-record grandfather set. The checker therefore reports that already-frozen Diagnostics record as missing `ENDING_BRANCH_COMMIT`, `LOG_FORMAT_REVISION`, and `SELF_PAUSED`. Foreman does not rewrite the immutable Diagnostics record or silently alter governance policy. The contract/checker synchronization debt belongs to governance/continuity tooling and does not block the next A002 product packet.

PENDING_LOCAL=independent Validation disposition for integrated A002 authority; governance synchronization of work-log contract revision 0005 into scripts/work-log-check.py; later concrete PS2 audio worker/thread/time binding and physical qualification
HARDWARE_PENDING=A001 physical PS2 qualification remains pending; no A002 physical qualification is claimed

## Dependency conclusion for next Reconstruction packet

The accepted synchronous core intentionally stops at the point where a caller supplies a buffer and runs playback synchronously. A002 still requires the session lifecycle that H1 previously combined with worker resources, startup fill, and the shared media timeline.

Current clean authority already supplies the required pieces:

- CONFIG owns immutable PCM and common-media-clock facts;
- Transport owns AUDIO status/activity/read and finite producer completion;
- `src/audio` owns synchronous PCM/AUDSRV correctness;
- `src/media` owns one-shot common epoch and signed/saturating audio deadlines.

The missing clean responsibility is a session-scoped audio worker that owns its execution resources, waits for a startup reservoir without consuming bytes, waits for the common clock to become armed and then for the audio presentation deadline, invokes the accepted synchronous playback core, and proves finite stop/join/reclaim ordering.

A key current-source fact must be handled deliberately: `pstvnc_media_clock_wait_audio()` currently returns `PSTVNC_MEDIA_CLOCK_UNARMED` when called before the one-shot epoch is armed. A002 permits audio to prefill before the common epoch is published but forbids presentation before the audio deadline. Therefore the lifecycle packet must provide a finite, stop-aware, non-busy way to wait across the initially-unarmed -> armed publication transition without letting audio arm or move the clock. A minimal reusable media-clock wait extension is allowed if required; silently treating `UNARMED` as ready is prohibited.

Final production values for worker stack size, thread priority, playback-buffer/chunk capacity, startup reservoir, and timing-poll cadence remain qualification/configuration choices. Their absence is not a blocker and must not be solved with hidden magic defaults. Keep unresolved values explicit in a narrow lifecycle owner/caller boundary and test with fixture values.

## Fresh interactive Reconstruction packet

WORK_ITEM_KEY=`a002-audio-clock`
TARGET_WORKER=`interactive`
ASSIGNING_HEAD=`ad1ee46731fd0413f2b3226cb53f7b528ac4a3ad`
ASSIGNING_AUDIT=`LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md:0001`
ASSIGNING_FOREMAN_STATE=`0011`

### Objective

Reconstruct the A002 session-scoped audio-worker lifecycle around the accepted synchronous PCM playback core. Add explicit worker/resource authority, startup-reservoir readiness, publication-aware common-clock audio gating, finite stop/join/reclaim semantics, and deterministic host coverage without changing the already-accepted Transport AUDIO or playback submission semantics.

### Required deliverables

1. **Session worker owner with explicit resource authority.** Add the smallest coherent audio-session/worker owner. Worker stack size, thread priority, playback-buffer capacity (which may also bound synchronous playback chunk size), startup-reservoir threshold, and any clock polling cadence required by the implementation must be explicit caller/lifecycle authority. Do not invent production defaults and do not expand the production CONFIG wire profile merely to house laboratory tuning.

2. **Owned resources and finite reclaim fence.** The session owner allocates/owns playback buffer and worker stack/resources, creates/starts exactly one playback worker, records completion/result, and never frees worker-visible memory before the worker is provably finished/dormant. Allocation/create/start/join failures must be explicit. Repeated cleanup must not double-free or reclaim a live worker.

3. **Reservoir readiness without byte consumption.** Before presentation starts, observe the public Transport AUDIO `status`/activity seam rather than dequeuing PCM. Reservoir-ready is distinct from presentation-ready. If available bytes reach the explicit threshold, reservoir is ready. If finite producer completion arrives with a smaller nonzero final reservoir, that finite set may become ready rather than deadlocking forever below the threshold. Producer-done with zero available bytes is normal empty exhaustion and must not start AUDSRV playback. Temporary shortage waits on Transport activity without busy spin or lost-wakeup behavior.

4. **Publication-aware common-clock audio gate.** After reservoir readiness and before the first playback submission, wait until the existing common media clock is armed and then until the audio deadline. Audio must never arm or move the common epoch. Because current `pstvnc_media_clock_wait_audio()` rejects an initially unarmed clock, add the narrowest reusable stop/failure-aware mechanism needed to span future one-shot publication if no existing clean seam suffices. The gate must use explicit nonzero delay/event cadence where polling is used, reject accidental busy-spin configuration, preserve synchronization/timer/delay failures, and remain host-testable.

5. **Wrap, do not rewrite, the accepted synchronous core.** Once reservoir and common-clock gates are satisfied, invoke the existing `pstvnc_audio_playback_run()`/equivalent accepted core with the session-owned buffer and exact immutable PCM profile/service authority. Preserve its wait-before-play, exact accounting, finite exhaustion, first-error, and resident-AUDSRV semantics rather than duplicating those rules in the worker.

6. **Finite cancellation and teardown.** Session stop must cancel reservoir waiting and common-clock waiting promptly. During active playback, teardown must cooperate with the existing Transport/session stop path so a worker blocked on Transport activity can retire; do not create a second Transport receive/close owner. The lifecycle must expose an ordering that allows application orchestration later to request stop/abort, wait/join worker completion, and only then reclaim stack/buffer/session resources. Preserve the first meaningful worker error through stop/join/cleanup.

7. **Deterministic behavior tests.** Prove at minimum:
   - explicit resource/timing values are consumed without hidden defaults;
   - allocation/create/start failure cleanup;
   - below-threshold reservoir waits without consuming bytes;
   - threshold crossing wakes readiness;
   - producer-done with a short final nonzero reservoir proceeds;
   - producer-done with empty queue completes normally without playback;
   - initially unarmed clock does not permit playback and later arm publication releases the gate only at/after the audio deadline;
   - audio never arms or moves the common epoch;
   - stop during reservoir wait and stop during unarmed/deadline clock wait retire finitely;
   - clock synchronization/timer/delay failure is surfaced;
   - accepted playback-core success/failure is propagated without semantic rewriting;
   - join/completion precedes buffer/stack reclamation and repeated cleanup cannot free live resources;
   - resident AUDSRV semantics remain intact and no quit path appears.

8. **Boundary discipline.** Obey source synopsis/naming/dictionary/topology rules and update source-side dictionaries for genuinely new source. Leave generated portal/canonical test/build registration to Foreman. Do not add full application session orchestration, choose production tuning values, arm the media clock from audio, begin MPEG decode/video deadline/presentation/late-frame policy, repair inherited H1 receive-poison behavior, or claim hardware qualification.

### Acceptance criteria

W1. One clean session-scoped audio worker owner exists with explicit resource/timing authority and no hidden production defaults.

W2. Worker buffer/stack/thread resources have a provable create/start/finish/join/reclaim lifecycle with no free-before-finished path.

W3. Startup reservoir is observed without consuming PCM, uses activity-based waiting, handles short finite final reservoirs, and treats producer-done+empty as normal completion.

W4. Playback cannot begin while the common clock is unarmed or before the audio deadline; future arm publication is waited safely, audio never arms the clock, and stop/failure behavior is finite/non-spinning.

W5. The worker calls the accepted synchronous playback core only after reservoir and presentation gates and does not reimplement its submission/accounting/AUDSRV semantics.

W6. Stop/Transport convergence and join ordering let later application orchestration retire the worker before resource reclamation without creating a second Transport owner or hiding first error.

W7. Deterministic host tests cover resource failures, reservoir transitions, unarmed->armed publication, deadline gating, cancellation, clock failures, playback result propagation, and reclaim fencing.

W8. No application orchestration, MPEG/video/presentation policy, audio-owned clock arm, guessed production tuning, receive-poison repair, or hardware qualification is introduced.

### Preserved invariants / non-goals

- The completed CONFIG/profile, common-clock one-shot epoch, Transport AUDIO channel, and synchronous PCM/AUDSRV core are regression authority, not rewrite targets.
- One physical PSTV connection and sole Transport receive owner remain mandatory.
- Reservoir-ready and presentation-deadline-ready are separate states.
- A finite final AUDIO producer may legitimately end below the nominal startup threshold; do not turn that into an infinite wait.
- Producer-done plus empty AUDIO is normal completion and must not initialize/start playback merely to observe emptiness.
- Audio never selects, arms, or moves the common media epoch.
- The existing media clock's `UNARMED` result is not success; if the worker may arrive before arm, bridge that publication interval explicitly and safely.
- Session cleanup stops/mutes current audio but never calls `audsrv_quit()`.
- Resource/timing values unresolved by qualification remain explicit caller authority; do not promote H1 experiment constants into hidden defaults.
- Do not begin A003 MPEG/video behavior or late-frame policy.
- A001 physical status remains `HARDWARE_PENDING`.

### Blocker burden

Physical PS2 hardware is not a blocker for the lifecycle/state-machine contract: resource/thread operations, Transport status/activity, clock publication/time, stop, and playback core can be represented through narrow injected seams and host-tested deterministically.

Missing production tuning values are not blockers. Use explicit fixture/caller values and leave final selection pending qualification.

If current media-clock or Transport public seams cannot support finite cancellation/publication waiting without adding a narrow reusable primitive, prove the exact limitation from source/tests and make the smallest owner-correct extension. Do not bypass ownership by reaching into private runtime state.

### Stretch only if W1-W8 are complete

A concrete PS2 thread/resource/time adapter may be added if it remains narrowly platform-owned, consumes only explicit lifecycle values, compiles cleanly with the pinned SDK, and does not force application orchestration or guessed tuning. Do not use stretch work to begin MPEG/video or hardware qualification.

## Exact next Foreman pickup

After Reconstruction returns, inspect the actual worker/resource/reservoir/common-clock behavior and criterion-by-criterion W1-W8 result. Perform only canonical registration/build/dictionary/evidence work. If coherent, decide from current audit/dependency authority whether A002 is ready for final application orchestration/Validation handoff or requires one final bounded A002 orchestration packet.

Validation remains independent. Foreman machine evidence is not Validation PASS, and A001 physical PS2 qualification remains `HARDWARE_PENDING`.
