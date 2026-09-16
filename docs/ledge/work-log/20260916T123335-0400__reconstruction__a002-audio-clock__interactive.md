# A002 PCM/AUDSRV Playback-Core Reconstruction — interactive

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T12:33:35-04:00
COMPLETED_AT=2026-09-16T12:55:32-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=393b967367875ec6c7a965065627a0ad94f5b517
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

FOREMAN_GOAL_RESULT=MET
ASSIGNING_FOREMAN_STATE_REVISION=0010
CONSUMED_WORK_LOG_CONTRACT_REVISION=0005

## Objective and authority consumed

Executed only Foreman state revision 0010's active A002 P1-P8 synchronous PCM/AUDSRV playback-core packet as one sustained interactive Reconstruction round. The exact branch authority at shift start was `393b967367875ec6c7a965065627a0ad94f5b517`, the Foreman integration/log commit that issued the current packet.

Authority consumed included:

- Foreman state revision 0010, phase `A002_PCM_PLAYBACK_CORE_PACKET_ISSUED`;
- A002 semantic audit `docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` revision 0001;
- accepted/canonically integrated A001 sole-receiver/RFB authority and A002 CONFIG/profile, Transport AUDIO-channel, and common-media-clock authority;
- pinned H1 forensic source authority `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, specifically historical AUDSRV setup/wait/play/stop semantics, as evidence rather than an implementation template;
- current PS2SDK EE AUDSRV API/implementation evidence proving `audsrv_wait_audio(int)`, `audsrv_play_audio(const char *, int)`, `audsrv_stop_audio()`, exact `audsrv_fmt_t` fields, and `audsrv_play_audio()`'s returned submitted-byte count;
- work-log contract revision 0005, which landed concurrently during this round without changing the canonical filename/schema required for this shift;
- the concurrent Architecture packet-scout record `docs/ledge/work-log/20260916T124537-0400__architecture__global-next-packet-prep__packet-scout.md`, consumed only as explicitly non-authoritative next-packet archaeology. It did not alter current P1-P8 authority and no worker/reservoir/clock packet was started.

The packet boundary remained authoritative throughout: preserve Transport as the sole physical receive owner; consume only its public logical AUDIO bridge; preserve normal finite AUDIO exhaustion and resident AUDSRV/LIBSD service lifetime; use exact immutable PCM profile values; wait before every nonzero playback submission; account bytes only after successful submission; and do not begin playback-worker lifecycle, startup-reservoir/common-clock audio timing, application orchestration, MPEG/presentation, inherited receive-poison repair, or hardware qualification.

## Reconstruction work performed

### Synchronous playback owner

Created `src/audio/playback.h` and `src/audio/playback.c` as the clean synchronous PCM playback owner. The component accepts only:

- immutable `pstvnc_config_pcm_profile_t` authority;
- caller-owned bounded playback storage/capacity, keeping chunk/resource tuning explicit rather than inventing defaults;
- the existing public `pstvnc_transport_audio_*` bridge;
- a narrow injectable resident-service operation table for deterministic testing;
- an explicit report for successful submitted bytes/chunks and cleanup facts.

No socket descriptor, Transport queue internals, receiver ownership, common-media-clock object, application object, thread object, or hidden tuning profile is exposed or acquired.

### Exact PCM profile and resident-service semantics

The playback boundary revalidates the existing exact PCM authority (`rate_hz`, `channels`, `bits_per_sample`, `volume_percent`) using the same representability/range constraints needed at the hardware-facing boundary. Values are forwarded unchanged to the service adapter; there is no fallback format, volume coercion, or application magic default.

Created `src/audio/audsrv_service.h` and `src/audio/audsrv_service.c` as the concrete PS2SDK adapter. It maps exact format/volume values to AUDSRV, accepts a chunk submission as successful only when `audsrv_play_audio()` reports the entire requested byte count submitted, and stops only the current stream on session retirement.

Resident service lifetime is structural: `pstvnc_audio_service_ops_t` contains no quit operation, the concrete adapter does not call or expose `audsrv_quit()`, and the host adapter fixture includes a mock quit counter that remains zero. Repeated session use therefore cannot reach per-session service dismantling through this playback core.

### Public Transport AUDIO consumption and finite completion

The playback loop uses only:

- `pstvnc_transport_audio_activity_snapshot()`;
- `pstvnc_transport_audio_read_available()`;
- `pstvnc_transport_audio_wait_activity()`.

Temporary `PSTVNC_TRANSPORT_WOULD_BLOCK` is handled by the existing generation-based Transport activity rendezvous rather than polling/spinning. The core snapshots activity before its bounded read, so activity racing the read is observed by Transport's generation check and cannot be slept past.

`PSTVNC_TRANSPORT_EXHAUSTED` is normal playback completion, preserving the accepted `producer_done + queue_empty` semantic. `STOPPED`, `CLOSED`, `INVALID`, and `FAILED` remain distinct playback outcomes and are never reclassified as normal exhaustion.

### Wait-before-play and truthful accounting

For every successful nonzero Transport dequeue, the exact ordering is:

1. verify report counters can represent the prospective successful submission;
2. `wait_audio(byte_count)`;
3. `play_audio(bytes, byte_count)`;
4. only after full successful playback submission, increment `submitted_bytes` and `submitted_chunks`.

A wait failure prevents playback. A partial/failed AUDSRV submission is a playback failure and does not advance successful-submission accounting. Zero-length playback submission is not permitted.

### Deterministic failure and cleanup convergence

The playback result vocabulary distinguishes normal completion, stop, closed Transport, invalid Transport use, terminal Transport failure, setup failures, wait failure, play failure, cleanup stop failure, and impossible accounting overflow.

Once AUDSRV initialization succeeds, session retirement attempts exactly one current-stream stop. If stop cleanup fails after otherwise normal completion, the result is `SERVICE_STOP_FAILED`. If a meaningful primary error already exists, cleanup failure is recorded in the report but does not overwrite that earlier result. AUDSRV initialization failure does not manufacture a stream-stop call against a service that was not successfully established.

No playback operation occurs after a terminal Transport/service result is selected.

## Source-domain adoption

Created `src/audio/SYMBOLS.md` with `COVERAGE=COMPLETE` and updated `docs/development/source-topology.md` to adopt `src/audio/` as the clean synchronous PCM/AUDSRV domain.

The domain boundary is explicit: synchronous logical AUDIO consumption, exact PCM profile application, wait-before-play, truthful post-submit accounting, and current-stream AUDSRV stop live here. Worker lifecycle, startup reservoir/common-clock timing, application orchestration, MPEG/presentation, and hardware qualification do not.

Generated dictionary portal reconciliation, continuity-tool domain-set integration, living-map integration, canonical test registration, and clean PS2 build/link registration are intentionally left for post-handoff integration rather than mixed into product-behavior Reconstruction.

## Deterministic tests added

Added:

- `tests/unit/audio_playback_test.c`;
- `tests/unit/audio_audsrv_service_test.c`;
- `tests/unit/audio_host_stubs/audsrv.h`.

The playback fixture covers exact setup/profile mapping, partial and multi-chunk ordering, wait-before-play order, temporary unavailability/activity wait, normal finite exhaustion, distinct stop/closed/failure outcomes, wait/play/setup/cleanup failure injection, first-error preservation, truthful byte/chunk accounting, invalid authority rejection, and no playback after terminal convergence.

The concrete AUDSRV adapter fixture covers exact SDK field/value mapping, byte-count representability, exact wait/play sizes, complete-submission requirement, partial-positive/negative play failure, SDK failure propagation, current-stream stop, and structural no-quit reachability.

## Exact Reconstruction commits

The product/test/topology commits authored by this Reconstruction round are:

- `103791438a4afae42edd67224466f0767a37b790` — `feat(a002): define synchronous PCM playback core`;
- `e1d6a8f60e54cf933d2b326182e184605850cb5f` — `feat(a002): implement synchronous PCM playback core`;
- `ad36d3a88da9c3a0bc4520975ba6f8c6219afbf2` — `feat(a002): declare resident AUDSRV adapter`;
- `c774fdda51acaa4b107793ee3b81e6eaf9c78b0c` — `feat(a002): bind PCM core to resident AUDSRV`;
- `2ce299ca6af66b0cb6c1accb69137a705949915d` — `test(a002): add host AUDSRV declaration seam`;
- `206a07162d8d14ea01e189c533ce73a02cf1a643` — `test(a002): prove synchronous PCM playback semantics`;
- `87af39599449a68849b69c45513e4d4dfcd95a42` — `test(a002): prove concrete AUDSRV adapter semantics`;
- `d21eb53ce41859ba1d04cfa94846c0d665709927` — `docs(a002): add clean audio source dictionary`;
- `7d00270af88149ab7c0b61d771cf6f5ef7b65f72` — `docs(a002): adopt clean audio source domain`.

Concurrent documentation/policy movement was consumed without overwrite and is not attributed to Reconstruction product work.

## Checks and evidence

### Supplemental exact-source host execution

The two new direct fixtures were compiled and executed from the exact source drafts subsequently committed, under the repository warning profile:

`cc -std=c99 -Wall -Wextra -Werror -pedantic ... audio_playback_test.c src/audio/playback.c ...`

Result:

`audio_playback_test: PASS`

`cc -std=c99 -Wall -Wextra -Werror -pedantic ... audio_audsrv_service_test.c src/audio/audsrv_service.c ...`

Result:

`audio_audsrv_service_test: PASS`

These are direct supplemental executions, not a claim that the new fixtures are already registered in canonical `make -C tests unit`.

### Canonical strict dictionary evidence

Workflow run `35124384938` at exact source-dictionary commit `d21eb53ce41859ba1d04cfa94846c0d665709927` reported:

- `SOURCE_DICTIONARY_CHECK_MODE=LONG`;
- `DEFINITION_SCOPE_COUNT=68`;
- `DEFINITION_DISCOVERY_STATUS=READY`;
- `SOURCE_DICTIONARIES=PASS`.

The later workflow run `35124492172` at `7d00270af88149ab7c0b61d771cf6f5ef7b65f72` again reported dictionary-long `SUCCESS`.

### Canonical integration state

At `7d00270...`, the existing canonical host-unit, PS2 compile, and PS2 linked/reproducibility jobs remain `SUCCESS`; these green results are preserved-regression evidence for the already-registered graph only and are NOT claimed as execution/compile/link proof for the newly unregistered `src/audio` sources or new audio fixtures.

`project-check` fails at the expected mechanical new-domain boundary:

- expected source-domain set omits `src/audio`;
- actual clean source-domain set includes `src/audio`.

That exact failure is topology/integration debt, not a P1-P8 product-behavior defect. Canonical generated portal/domain-set/build/test registration remains pending for Foreman integration.

## P1-P8 criterion disposition

P1 `MET` — a coherent synchronous/testable clean audio owner now depends only on immutable PCM profile authority, the public Transport AUDIO bridge, caller-supplied bounded storage, and a narrow injectable resident-service operation boundary; the concrete PS2SDK adapter is separate and proven from current SDK API/source evidence.

P2 `MET` — resident AUDSRV/LIBSD lifetime is explicit and structurally protected: no quit operation exists in the playback service boundary, the concrete adapter never calls `audsrv_quit()`, exact format/volume values are forwarded without defaults/coercion, and the adapter fixture proves zero quit calls.

P3 `MET` — AUDIO bytes are consumed only through the public Transport bridge. Temporary no-data uses the activity-generation wait rather than spinning, normal `EXHAUSTED` is successful finite completion, and stop/closed/invalid/failure remain distinct.

P4 `MET` — every submitted nonzero chunk waits for exact capacity before playback; full successful submission is required; bytes/chunks are counted only after successful playback submission; wait/play failure cannot inflate accounting.

P5 `MET` — setup, Transport, wait, play, accounting, and cleanup outcomes converge deterministically; no later playback occurs after a terminal outcome; one current-stream cleanup stop is attempted after successful initialization; cleanup failure does not overwrite an earlier meaningful primary error.

P6 `MET` — deterministic host fixtures cover profile mapping, chunk ordering, temporary unavailability, finite completion, terminal distinctions, injected setup/wait/play/cleanup failures, truthful accounting, SDK adapter mapping, and no-quit reachability. Both direct supplemental fixture executions passed under strict host warnings.

P7 `MET` — no accepted A001 RFB/Transport, A002 CONFIG/profile, Transport AUDIO, or common-clock product behavior was rewritten. The new component consumes those existing authorities, introduces no hidden tuning defaults, and its local source dictionary passes strict long definition discovery.

P8 `MET` — no playback worker/thread lifecycle, startup-reservoir/common-clock audio timing/callsite, application orchestration, MPEG/presentation behavior, inherited receive-poison repair, or hardware qualification was introduced.

FOREMAN_GOAL_RESULT=MET

## Evidence gaps / pending integration

PENDING_LOCAL=Foreman canonical registration and execution of `audio_playback_test` and `audio_audsrv_service_test`; clean PS2 compile/link registration of `src/audio/playback.c` and `src/audio/audsrv_service.c` with required AUDSRV linkage; `scripts/continuity-check.sh` adoption of the new `src/audio` domain; living file/service-map update as appropriate; generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` reconciliation; settled canonical project-check/build/test evidence; independent Validation disposition

HARDWARE_PENDING=A001 exact current-DUT physical PS2 qualification remains pending; no A002 playback hardware qualification is claimed by this host-testable synchronous-core packet

No P1-P8 product blocker remains open at this Reconstruction boundary.

## Exact next pickup

FOREMAN: independently inspect the actual P1-P8 source/tests and this criterion-by-criterion result. Perform only canonical integration/evidence work: register the two host fixtures; register/compile/link the audio product sources and required AUDSRV library; reconcile the continuity domain set, living map, and generated symbol portal; run settled canonical checks; then route coherent evidence to independent Validation. Do not treat this worker result itself as Validation PASS.

RECONSTRUCTION: stop here. Do not begin the playback worker lifecycle, startup reservoir/common-clock audio timing, application orchestration, MPEG/presentation, receive-poison repair, or hardware qualification without a fresh Foreman packet.