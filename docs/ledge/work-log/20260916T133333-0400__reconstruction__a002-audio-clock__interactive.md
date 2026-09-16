# A002 Audio Worker / Reservoir / Common-Clock Lifecycle Reconstruction — interactive

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T13:33:33-04:00
COMPLETED_AT=2026-09-16T13:55:34-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a7b9c4430a87ce0d66dc0eb75d09b407160c72c9
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

FOREMAN_GOAL_RESULT=MET
ASSIGNING_FOREMAN_STATE_REVISION=0011
CONSUMED_WORK_LOG_CONTRACT_REVISION=0005

## Objective and authority consumed

Executed only Foreman state revision `0011`'s active A002 W1-W8 audio-worker/resource/reservoir/common-clock lifecycle packet as one sustained interactive Reconstruction round.

The exact live branch authority captured at shift start was `a7b9c4430a87ce0d66dc0eb75d09b407160c72c9`. Foreman revision 0011 had been issued from assigning authority `ad1ee46731fd0413f2b3226cb53f7b528ac4a3ad`; the intervening commits through the shift-start head were state/log/support movement rather than a superseding product packet. Foreman revision 0011 remained active throughout this Reconstruction round.

Authority consumed included:

- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision `0011`, phase `A002_AUDIO_WORKER_CLOCK_PACKET_ISSUED`;
- `docs/ledge/LEDGE_WORK_STATE.md` revision `0029` at shift start;
- `docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` revision `0001`;
- completed and accepted A002 CONFIG/profile authority;
- completed and accepted Transport AUDIO logical-channel/status/activity/exhaustion authority;
- completed and accepted common-media-clock authority;
- completed and accepted synchronous PCM/AUDSRV playback core;
- pinned H1 forensic source authority `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, especially historical audio worker allocation/start/reservoir/clock/join ordering, as evidence rather than a production implementation template;
- immutable work-log contract revision `0005`;
- current source naming/topology/dictionary policy.

A current-source fact differed from Foreman 0011's planning prose and was resolved in favor of repository authority: the live `pstvnc_media_clock_wait_audio()` implementation already waits through an initially-unarmed clock using finite nonzero-delay polling, stop observation, and the existing deadline path. It does not simply return `PSTVNC_MEDIA_CLOCK_UNARMED` at first observation. Therefore no media-clock source extension was required or made.

The concurrent Architecture packet-scout record `docs/ledge/work-log/20260916T134704-0400__architecture__global-next-packet-prep__packet-scout.md` landed during this round. It is explicitly non-authoritative conditional archaeology for later work and did not supersede or alter W1-W8. It was consumed without starting A003 or competing with its planned work.

## Reconstruction work performed

### Session-scoped worker owner and explicit authority

Created `src/audio/session.h` and `src/audio/session.c` as the smallest coherent A002 session audio lifecycle owner around the accepted synchronous playback core.

`pstvnc_audio_session_values_t` makes every unresolved production resource/timing choice explicit caller authority:

- worker stack bytes;
- worker priority;
- playback-buffer capacity;
- startup-reservoir bytes;
- reservoir polling cadence;
- common-clock polling cadence.

The implementation contains no production defaults for those values and does not expand the CONFIG wire profile merely to house tuning. Zero/nonrepresentable required values are rejected instead of silently substituted. Six fixture values are deliberately test values, not product defaults.

The owner uses narrow injected memory, thread, synchronization, and time operation tables. A concrete PS2 thread/resource/time adapter was intentionally not added: Foreman 0011 marked that as stretch work only, and no qualified production stack/priority/buffer/reservoir/poll selections presently exist.

### Resource lifecycle and reclaim fence

The session owner allocates one playback buffer and one explicitly aligned worker stack, creates exactly one worker, starts it, records worker completion/outcome, exposes explicit stop/join/release operations, and refuses release while a started worker has not been joined.

The lifecycle is:

`allocate buffer -> allocate aligned stack -> create -> start -> worker finish -> join -> destroy thread object -> free stack/buffer`

Allocation, create, start, join, destroy, synchronization and not-finished/live-worker outcomes are explicit. Start failure attempts to destroy the unstarted thread before freeing resources. If destruction cannot complete, the session remains initialized so later release may retry rather than freeing memory still associated with a live thread object. Release is idempotent after successful reclamation and does not double-free.

The injected `join()` contract is explicitly the completion/visibility fence: a successful join must mean the worker is finished and worker-written outcome state is visible before the owner marks the session joined or permits reclamation.

### Non-consuming startup reservoir

Reservoir readiness uses only the public Transport AUDIO observation seam:

- `pstvnc_transport_audio_activity_snapshot()`;
- `pstvnc_transport_audio_status()`.

The reservoir owner never calls `pstvnc_transport_audio_read_available()` and never reaches into Transport queue/semaphore/runtime internals. PCM consumption remains exclusively inside the already-accepted synchronous playback core after all lifecycle gates are satisfied.

Reservoir semantics are:

- `available >= explicit threshold` -> reservoir ready;
- `producer_done && available > 0` -> finite short final reservoir is ready rather than deadlocking below the threshold;
- `producer_done && available == 0` -> normal empty completion and no AUDSRV playback;
- temporary shortage -> wait for a Transport AUDIO activity-generation change before rechecking status.

The public blocking Transport activity waiter is deliberately not used for this session-owned startup wait because local audio stop has no owner-correct way to signal Transport's private waiter without making audio a second Transport lifecycle owner. Instead the lifecycle observes the persistent Transport activity generation at the explicit nonzero `reservoir_poll_us` cadence. This is not a busy spin: status is not repeatedly polled while activity is unchanged, generation changes remain persistent and cannot be lost, and local stop has a finite cancellation bound without aborting/closing Transport.

### Initially-unarmed -> armed -> audio-deadline gating

After reservoir readiness, the worker calls the existing accepted:

`pstvnc_media_clock_wait_audio(clock, 0, clock_poll_us, time_ops, stop_observer)`

with the session common clock stored as `const pstvnc_media_clock_t *` authority.

Current live media-clock source already provides the required publication-aware behavior: while unarmed it delays at the explicit nonzero cadence and rechecks stop/publication; once armed it computes the configured signed audio deadline and delays/observes time until that deadline is satisfied; synchronization/timer/delay failures remain explicit.

Audio does not arm or move the common epoch. The committed `src/audio/session.c` contains no `pstvnc_media_clock_arm()` call. The deterministic fixture also supplies a mock arm function and asserts its call counter remains zero while simulating external publication of the common epoch.

A final stop observation occurs after the clock gate and before playback invocation so cancellation cannot race directly into a newly started synchronous run after the deadline has already returned.

### Accepted synchronous playback composition

Only after reservoir-ready and common-clock audio-deadline-ready does the worker call the accepted:

`pstvnc_audio_playback_run()`

with the exact immutable PCM profile, session-owned playback buffer/capacity, and accepted resident-service operation table.

The lifecycle wrapper does not reimplement Transport dequeue, normal exhaustion, AUDSRV setup, wait-before-play, submission, byte/chunk accounting, stream-stop cleanup, or resident-service lifetime semantics. The exact accepted playback result/report are preserved in the worker outcome for later orchestration/diagnostics.

### Stop / Transport convergence boundary

`pstvnc_audio_session_request_stop()` cancels session-owned reservoir and common-clock waits through the injected synchronized stop flag. It does not abort, close, receive from, or otherwise become a lifecycle owner of Transport.

Once the accepted synchronous playback core is active, an audio-local stop alone cannot safely wake every possible Transport/AUDSRV blocking point without violating ownership. The public interface documents the intended later application convergence explicitly: application-level teardown pairs audio stop with the already-existing Transport abort/stop ordering, then joins the audio worker, then reclaims session resources. This preserves sole Transport ownership and makes the required stop -> Transport convergence -> worker join -> reclaim ordering available to the later application packet rather than hiding a second Transport owner inside audio.

## Deterministic host fixture

Added `tests/unit/audio_session_test.c` with deterministic memory/thread/sync/Transport/time/clock/playback seams.

The fixture covers:

- exact explicit stack bytes, priority, playback-buffer capacity, reservoir threshold, reservoir cadence and clock cadence;
- buffer allocation failure;
- stack allocation failure;
- thread create failure;
- thread start failure and cleanup;
- join failure with no premature free;
- release-before-join refusal;
- successful finish/join/destroy/free ordering;
- idempotent repeated release/no double free;
- reservoir threshold crossing only after Transport activity changes;
- no repeated status polling while activity generation remains unchanged;
- producer-done with a short nonzero final reservoir proceeding to presentation;
- producer-done plus empty queue completing without playback;
- initially-unarmed common clock followed by externally simulated arm publication;
- audio deadline gating before playback observation;
- proof that the audio session does not call `pstvnc_media_clock_arm()`;
- stop during reservoir waiting;
- stop during common-clock publication waiting;
- media-clock synchronization failure propagation;
- media-clock timer failure propagation;
- media-clock delay failure propagation;
- exact Transport terminal outcome preservation;
- exact accepted synchronous playback result/report preservation;
- rejection of absent/zero required lifecycle authority.

A direct local execution of the deterministic lifecycle fixture during implementation passed under strict host warnings:

`-std=c99 -Wall -Wextra -Werror -pedantic`

Result:

`audio_session_test: PASS`

The new test is intentionally not registered in canonical `make -C tests unit` by this worker because Foreman 0011 reserves canonical test/build registration and settled machine evidence for post-handoff integration. The canonical host-unit green jobs observed during this round are therefore regression evidence for the already-registered graph, not falsely claimed execution of the new unregistered lifecycle fixture.

## Source ownership / dictionary evidence

Updated `docs/development/source-topology.md` so the existing clean `src/audio/` domain now explicitly owns session worker/resources, non-consuming startup reservoir, common-clock audio gating, synchronous PCM composition and resident AUDSRV stream operations while continuing to exclude Transport abort/close, common-epoch arming, application orchestration, MPEG/video presentation, receive-poison repair and hardware qualification.

Updated `src/audio/SYMBOLS.md` with all mechanically discovered `session.{c,h}` definitions.

Canonical workflow run `35130903833` at exact worker source/dictionary head `05d2b8a3a44ecf3807f4bd6cdadf09948696e254` reported:

- `SOURCE_DICTIONARY_CHECK_MODE=LONG`;
- `DEFINITION_SCOPE_COUNT=70`;
- `DEFINITION_DISCOVERY_STATUS=READY`;
- `SOURCE_DICTIONARIES=PASS`.

At that same head, existing canonical `host-unit` and `ps2-compile` jobs were green and the linked/reproducibility job had completed both build/rebuild comparison stages successfully; these are regression evidence for the previously registered graph only because Foreman has not yet registered `session.c` or `audio_session_test.c` canonically.

The current canonical `project-check` failure is the expected post-source-inventory integration boundary:

`ERROR=SOURCE_DICTIONARY_PORTAL_OUT_OF_DATE`

The generated source-dictionary portal is intentionally left for Foreman-owned canonical reconciliation, exactly as the packet requested. No source-definition completeness defect remains: the independent long/complete/strict dictionary job passes.

## Exact Reconstruction commits

Product/test/source-ownership commits authored by this Reconstruction round on `ledge/h1-all-guns` are:

- `5fce6b48287b26e0b40fe247e6bca20daa030ed6` — `feat(a002): define audio worker lifecycle owner`;
- `b5e020bc7b93d9816a09196b58c1bcdd1c571f27` — `feat(a002): implement audio worker lifecycle owner`;
- `4dca53b58db338d9ecfe600cd6ed751041e2af37` — `test(a002): prove audio worker lifecycle semantics`;
- `1824f25ff04d061d88e7a8bbcb664f2064e441d3` — `docs(a002): extend audio domain to worker lifecycle`;
- `05d2b8a3a44ecf3807f4bd6cdadf09948696e254` — `docs(a002): inventory audio worker lifecycle symbols`.

A concurrent Architecture support log landed between the test and documentation commits; it changed no W1-W8 product source and is not attributed to Reconstruction work.

One non-product repository side effect is recorded for completeness: during connector/schema probing, an auxiliary branch ref named `tmp-noop-do-not-use` was accidentally created at the shift-start authority `a7b9c4430a87ce0d66dc0eb75d09b407160c72c9`. It was never used for source work and does not affect `ledge/h1-all-guns`. No branch-delete action was available in the connected GitHub tool surface during this round, so the ref is left untouched for later repository cleanup rather than hidden or force-moved.

## W1-W8 criterion disposition

W1 `MET` — one clean session-scoped audio worker owner now exists with explicit caller authority for stack bytes, priority, playback-buffer capacity, startup-reservoir bytes, reservoir cadence and clock cadence. No production defaults or CONFIG-wire expansion were introduced.

W2 `MET` — buffer, aligned stack and exactly one thread have explicit allocate/create/start/finish/join/destroy/reclaim state. Release refuses a started unjoined worker, join requires worker-finished proof, failed lifecycle operations are explicit, and successful repeated release cannot double-free.

W3 `MET` — startup reservoir observes only public Transport AUDIO status/activity, never dequeues PCM, waits on persistent activity generation at a nonzero explicit cadence, handles threshold crossing, allows a finite short final nonzero reservoir, and treats producer-done plus zero bytes as normal empty completion without playback.

W4 `MET` — accepted common-clock wait authority safely spans initially-unarmed -> externally armed -> signed audio deadline, with finite stop/sync/timer/delay handling. Playback is unreachable before that gate returns ready. Audio stores only const clock authority and never calls `pstvnc_media_clock_arm()` or otherwise moves the epoch.

W5 `MET` — after both gates, the worker delegates actual PCM consumption/submission to the already-accepted `pstvnc_audio_playback_run()` core and preserves its exact result/report. It does not duplicate Transport dequeue, AUDSRV setup/wait/play, accounting or stream-retirement semantics.

W6 `MET` — session stop finitely cancels reservoir/clock waits; active-playback teardown is deliberately composed with the existing Transport stop/abort path by the later application owner rather than creating a second Transport owner. The exposed stop/join/release sequence lets later orchestration converge Transport first, prove worker completion, then reclaim resources while preserving the worker's exact Transport/clock/playback outcome.

W7 `MET` — deterministic host fixture source covers explicit authority, allocation/create/start/join failures, reservoir transitions/activity gating, finite short/empty producer completion, initially-unarmed publication/deadline gating, stop in both pre-play waits, clock failures, exact playback-result propagation, no-audio-arm proof, and join/reclaim fencing. Supplemental strict host execution passed; canonical fixture registration/execution remains Foreman-owned integration evidence rather than being misclaimed here.

W8 `MET` — no application orchestration, MPEG/video presentation, audio-owned common-epoch arm, guessed production stack/priority/buffer/reservoir/poll defaults, inherited receive-poison repair, or hardware qualification was introduced. The optional concrete PS2 worker/resource/time adapter stretch was not begun because W1-W8 did not require it and qualified production values remain intentionally unresolved.

FOREMAN_GOAL_RESULT=MET

## Evidence gaps / pending integration

PENDING_LOCAL=Foreman independent review of actual W1-W8 source/tests; canonical registration/execution of `audio_session_test`; clean PS2 compile/link registration of `src/audio/session.c`; generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` reconciliation; any living file/service-map and build dependency updates owned by integration; settled canonical project-check/build/test evidence; independent Validation disposition; Foreman decision whether A002 needs one final bounded application-orchestration packet before dependency advance

HARDWARE_PENDING=A001 exact current-DUT physical PS2 qualification remains pending; no A002 worker/resource/reservoir/common-clock hardware qualification is claimed; concrete PS2 worker scheduling/resource/timing values and repeated-session timing/endurance remain future qualification work

No W1-W8 product blocker remains open at this Reconstruction boundary.

## Exact next pickup

FOREMAN: independently inspect the committed worker/resource/reservoir/common-clock lifecycle and criterion-by-criterion result. Perform only Foreman-owned canonical integration/evidence work: register the host fixture and session source in canonical test/build graphs, reconcile the generated dictionary portal and any living map/build dependencies, run settled canonical checks, and route coherent evidence to independent Validation. Then decide from current dependency authority whether A002 requires one final bounded application orchestration packet or is ready to hand off toward A003.

RECONSTRUCTION: stop here. Do not begin application orchestration, A003 MPEG/video presentation, receive-poison repair, concrete hardware qualification, or unissued follow-on work without a fresh Foreman packet.
