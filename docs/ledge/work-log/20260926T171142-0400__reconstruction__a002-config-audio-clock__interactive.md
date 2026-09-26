# Ledge Reconstruction work log — R37 PS2 AUDIO execution binding

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T17:11:42-04:00
COMPLETED_AT=2026-09-26T17:25:17-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=78c4623f9c9b9838fbfc091e380380cd49b32974
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Reconstruction round received the baton at live branch authority
`78c4623f9c9b9838fbfc091e380380cd49b32974` under Foreman State revision
0079.

The exact active packet was:

`A002-PS2-AUDIO-EXECUTION-BINDING-R37`.

R37 authorized only the lower AUDIO-domain PlayStation 2 execution binding for
the already accepted AUDIO session owner: resident LIBSD/AUDSRV preparation,
aligned session memory operations, one session-local synchronization primitive,
one exact EE worker slot with proof-driven completion/dormancy/destruction, and
monotonic retryable runtime release.

Ordinary Application AUDIO activation, Transport AUDIO rider composition,
media-clock ownership, PCM policy changes, Pi AUDIO work, automatic MPEG
recalibration, persistence/editor work, independent Validation and hardware
qualification remained out of scope.

The round consumed Reconstruction Contract revision 0006, work-log contract
revision 0007, Architecture Overlay revision 0007, Wire Runtime Decisions
revision 0011, Q1-Q12 Reconciliation revision 0001, the A002 semantic audit,
accepted R26 media-clock authority, accepted R36 selected AUDIO profile
authority, the current AUDIO session/playback/AUDSRV owners, current build and
source-topology policy, and the assigning Foreman immutable log at the starting
commit.

## Final pre-log source authority

Final source/test/build/dictionary head before this immutable record:

`87069d82787c4b8c66a759ef9ef2a1bc7af03b53`
— `test(audio): inject R37 allocation failure`.

The returned range is exactly six commits ahead of the assigning Foreman/log
head and zero behind:

1. `b6255c9bed864f978893151a0626870957e455e5`
   — `audio(ps2): add proof-driven R37 execution binding`
2. `f31a04814fa94aff660ca685fcca23fbbbfe250a`
   — `test(audio): include R37 host kernel contract`
3. `ac602d1a4f1b5939e4a6253e0c8ee18e56b5dc3a`
   — exact deterministic dictionary-reconciliation trigger
4. `6e171338db7567e7d23acd2bc67674fbdb736590`
   — automation-generated clean dictionary reconciliation
5. `3f54facab79ecaac8b2170ae5c65dea4184a5ced`
   — `test(audio): honor PS2 thread ABI in R37 host fixture`
6. `87069d82787c4b8c66a759ef9ef2a1bc7af03b53`
   — `test(audio): inject R37 allocation failure`

Exact changed paths are confined to the authorized AUDIO execution binding,
clean build enrollment, focused tests/stubs, lifecycle/topology documentation
and generated dictionary surface:

- `src/audio/ps2_runtime.c`
- `src/audio/ps2_runtime.h`
- `src/audio/SYMBOLS.md`
- `mk/issue7-clean.mk`
- `tests/Makefile`
- `tests/unit/audio_ps2_runtime_test.c`
- `tests/unit/audio_ps2_runtime_source_test.py`
- `tests/unit/audio_ps2_runtime_host_stubs/kernel.h`
- `tests/unit/audio_ps2_runtime_host_stubs/delaythread.h`
- `tests/unit/audio_ps2_runtime_host_stubs/loadfile.h`
- `docs/development/source-topology.md`
- `docs/development/module-lifecycle.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

No ordinary Application source, Transport implementation, AUDIO session/
playback/AUDSRV service implementation, Configuration profile implementation,
media-clock implementation, MPEG/RFB/Input/UI/Display implementation, Pi code,
Wire protocol or forensic source changed.

## R37 implementation

R37 adds one narrow `pstvnc_audio_ps2_runtime_t` under the AUDIO owner. One
session runtime explicitly owns:

- one session-state semaphore;
- one worker-completion semaphore;
- one exact EE thread slot and its start/completion/dormancy facts;
- at most two exact aligned allocation records, matching the accepted session
  playback-buffer and worker-stack ownership.

A separate caller-owned `pstvnc_audio_ps2_resident_t` tracks resident module
preparation across AUDIO sessions. `pstvnc_audio_ps2_resident_prepare()` loads
`rom0:LIBSD` and then the embedded PS2SDK `AUDSRV_irx`. Each success is
recorded independently, so an AUDSRV-load failure after successful LIBSD load
retries only AUDSRV. Repeated successful preparation is inert. R37 contains no
`audsrv_quit()`, IOP reset or per-session module unload/reload path.

The clean linked build now deterministically generates the embedded AUDSRV
object from:

`$(PS2SDK)/iop/irx/audsrv.irx`

through the same `bin2c` pattern as the other current IRX inputs and links
`AUDSRV_irx.o` plus `audio_ps2_runtime.o`.

Memory allocation validates nonzero power-of-two alignment and byte-count
overflow before allocation. Ownership is published only after allocation and
alignment succeed. The runtime records the raw allocation separately from the
aligned address so the exact underlying allocation is released, and release is
forbidden while any allocation remains live.

The session synchronization projection supplies only AUDIO-session
`WaitSema`/`SignalSema` ownership. It exports no common-media-clock time
operations; R26 remains the sole PS2 timer/time-operation owner.

The EE thread adapter preserves exact state:

- failed `CreateThread` creates no thread-slot authority;
- successful create records one exact created-but-not-started slot;
- `thread_started` becomes true only after successful `StartThread`;
- the trampoline invokes the accepted session worker, publishes the completion
  semaphore, then exits;
- join consumes the completion semaphore at most once and retains
  `completion_observed`;
- after completion, join succeeds only after the exact thread reports
  `THS_DORMANT`;
- non-dormant observations only yield with `DelayThread(1)`; no count,
  timeout or elapsed duration can manufacture success;
- a status failure after the completion event was consumed leaves
  `completion_observed` intact, so a retry does not wait for an already
  consumed event;
- destroy requires actual dormancy and, for a started thread, retained
  completion authority;
- failed `DeleteThread` preserves the thread slot and dormancy authority for
  retry;
- a never-started created thread is also deleted only after concrete dormant
  status.

Runtime release rejects a live thread slot or live allocation. Semaphore IDs
are cleared only after successful `DeleteSema`; partial semaphore deletion
therefore leaves exact remaining ownership and a monotonic retry path.

## Deterministic focused evidence

The R37 host fixture proves:

- LIBSD load failure publishes no resident success;
- successful LIBSD followed by failed AUDSRV load retains only LIBSD success;
- retry after that partial failure does not reload LIBSD;
- repeated successful resident preparation performs no duplicate loads;
- injected raw allocation failure leaves `live_allocations == 0`;
- the 4096-byte playback-buffer allocation and 16384-byte/16-byte-aligned stack
  allocation are both tracked;
- allocation-size overflow fails without creating ownership;
- runtime release refuses live allocations;
- failed `CreateThread` owns no slot;
- failed `StartThread` leaves an exact never-started created slot;
- a non-dormant never-started slot cannot be deleted, while later dormant proof
  permits deletion;
- worker execution and completion publication precede successful join;
- repeated non-dormant status observations do not become success;
- join waits through those observations until actual `THS_DORMANT`;
- a status-query failure after completion consumption preserves
  `completion_observed`, and the retry does not consume the completion event
  again;
- failed `DeleteThread` preserves exact thread ownership for retry;
- runtime release refuses a live thread;
- partial `DeleteSema` failure preserves the remaining semaphore ID and
  succeeds on a later retry.

The source/build boundary test additionally proves that R37 contains the
required PS2 module/thread/semaphore/dormancy mechanisms, contains no
`audsrv_quit`, IOP reset, forced thread termination, bounded-join success,
R26 timer mechanism or common-clock time-op owner, and has no ordinary
Application AUDIO callsite. It also proves the current clean makefile uses the
pinned PS2SDK AUDSRV IRX path.

Existing R36 Configuration AUDIO profile, synchronous playback, AUDSRV service,
AUDIO session and common media-clock regression tests remain green.

## Canonical machine evidence

Exact final-source GitHub Actions run:

`36272813140`

checked out exact head
`87069d82787c4b8c66a759ef9ef2a1bc7af03b53` on
`ledge/h1-all-guns`, attempt 1, and completed SUCCESS.

Successful canonical jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED because the final head was not the
  deterministic reconciliation trigger.

Observed focused/regression output includes:

- `audio_ps2_runtime_test: PASS`;
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`;
- `audio_playback_test: PASS`;
- `audio_audsrv_service_test: PASS`;
- `audio_session_test: PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The initial pre-reconciliation/focused-fix run exposed only packet-local issues:
new-file dictionary/topology drift and host ISO-pedantic rejection of PS2SDK's
intentional EE thread function-pointer/object-pointer ABI representation.
Dictionary reconciliation then changed only `src/audio/SYMBOLS.md` and the
generated dictionary portal. The focused host compile was narrowed with
`-Wno-pedantic` only for this PS2 ABI fixture; the actual pinned PS2 compile
remained green throughout.

## Exact linked identity and qualification boundary

Final R37 linked identity:

`ELF_PRISTINE_SHA256=f7270f3f738ab497dbe9e5163df24984056b6d5fdb0ca5286459d59fc32687a5`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=776dd4923ca3302585bd31e630f7e9914a45cd2c4b5980c4766ad0dfdbf4b2b9`

`PT_LOAD_BYTES=550164`

Newest Foreman-accepted R36 identity was:

`PT_LOAD_SHA256=5c59e5a914d38f5dbe7c0ad9d72df602f91753a1a002b2bfa112f48e4978d423`

`PT_LOAD_BYTES=528020`.

Therefore the R37 PT_LOAD changed and creates new physical qualification debt.

Evidence classification:

- `R37_SOURCE_COMPLETE=YES_WITHIN_PACKET`
- `R37_HOST_TESTED=PASS`
- `R37_PROJECT_CHECK=PASS`
- `R37_STRICT_DICTIONARIES=PASS`
- `R37_PS2_COMPILE=PASS`
- `R37_PS2_LINK=PASS`
- `R37_CURRENT_SOURCE_REPRODUCIBILITY=PASS`
- `R37_MACHINE_EVIDENCE=GITHUB_ACTIONS`
- `R37_PS2_PT_LOAD_CHANGED=YES`
- `R37_INDEPENDENT_VALIDATION=NOT_RUN`
- `R37_OPERATOR_OBSERVED=NO`
- `R37_HARDWARE_QUALIFIED=NO`
- `R37_HARDWARE_PENDING=YES`.

No physical qualification transfers from R36 or any earlier ELF.

## Reconstruction Worker requirement disposition

These are Reconstruction Worker dispositions only. They are not Foreman
acceptance.

1. Narrow AUDIO-domain PS2 runtime with explicit owned resources — MET.
2. Resident LIBSD/AUDSRV preparation is explicit/idempotent and no
   `audsrv_quit()` exists — MET.
3. Pinned PS2SDK AUDSRV IRX is deterministically embedded in the clean build —
   MET.
4. Alignment/allocation overflow/failure/live ownership is exact and
   fail-closed — MET.
5. Session synchronization is AUDIO-local — MET.
6. Create/start state transitions preserve exact EE thread ownership — MET.
7. Completion plus actual `THS_DORMANT` are required; no timeout/poll count
   creates join success — MET.
8. Completion observation survives a later status failure and remains retryable
   without re-consuming the event — MET.
9. Started and never-started thread destruction is proof-driven and
   `DeleteThread` failure preserves authority — MET.
10. Runtime/semaphore release is monotonic, retryable and ownership-exact — MET.
11. R26 remains sole common-clock/time-operation owner — MET.
12. Ordinary Application AUDIO activation remains absent; accepted neighboring
    mechanisms remain unchanged and green — MET.

## Scope, limitations and next pickup

R37 does not open the Transport AUDIO rider, start an ordinary product AUDIO
session, consume R36 from Application, arm/wait the common clock from a new
owner, alter PCM/AUDSRV service semantics, modify Pi AUDIO behavior, implement
automatic MPEG recalibration, or claim independent Validation, operator
observation or physical qualification.

There is no Reconstruction blocker at this stopping point. Physical
qualification remains pending because the linked PT_LOAD changed.

The Interactive Reconstruction Worker stops here. The Foreman must
independently inspect the six-commit returned range, exact R37 ownership/
dormancy mechanisms, focused/canonical evidence, changed linked identity and
this immutable closeout before accepting or rejecting R37 and selecting any
successor packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R37_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION
