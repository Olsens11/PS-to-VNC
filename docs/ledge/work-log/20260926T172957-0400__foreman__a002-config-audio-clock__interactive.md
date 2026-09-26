# Ledge Foreman work log — accept R37 and issue AUDIO completion publication

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T17:29:57-04:00
COMPLETED_AT=2026-09-26T17:34:39-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=93e3271b42b5dcad1d83d7126d989306629d0e11
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R37 PS2 AUDIO execution-binding work.

Live pickup authority was:

`93e3271b42b5dcad1d83d7126d989306629d0e11`
— `docs(work-log): record R37 PS2 audio execution binding`.

The round independently recovered current Foreman State revision 0079, the exact
R37 immutable Reconstruction closeout, the six-commit returned source/test/build
range, A002/A006 audit authority, accepted R26/R36 lower dependencies, current
clean AUDIO session/playback/AUDSRV owners, product PS2 platform/build rules,
and exact source-head and closeout-head canonical GitHub Actions evidence.

No user terminal, Pi-local proxy, hardware action, Validation work, or Foreman
product-behavior patch was required.

## Returned R37 range

Assigning Foreman/log authority:

`78c4623f9c9b9838fbfc091e380380cd49b32974`.

Final pre-log R37 source authority:

`87069d82787c4b8c66a759ef9ef2a1bc7af03b53`.

Immutable Reconstruction closeout:

`93e3271b42b5dcad1d83d7126d989306629d0e11`.

Independent compare proves:

- 6 commits ahead;
- 0 behind.

Behavior-bearing source changes are confined to:

- `src/audio/ps2_runtime.c`;
- `src/audio/ps2_runtime.h`;
- clean build enrollment for the PS2 runtime and embedded AUDSRV IRX;
- focused host kernel/module stubs and runtime tests.

The remaining changed files are lifecycle/topology documentation and dictionary
reconciliation.

No ordinary Application, Transport, AUDIO session/playback/AUDSRV service,
Configuration profile, media-clock, MPEG/RFB/Input/UI/Display, Pi or
Wire/protocol implementation changed.

## Independent R37 source findings

R37 introduces one narrow AUDIO-domain PS2 runtime with explicit owner state for:

- one AUDIO session lock semaphore;
- one retained worker-completion semaphore;
- one exact EE worker slot;
- at most two exact aligned allocations.

The separate resident state tracks LIBSD and AUDSRV module preparation
independently.

Resident preparation is truthful and retryable:

- failed LIBSD load publishes no success;
- successful LIBSD followed by failed AUDSRV load preserves only LIBSD success;
- retry does not reload already-proven LIBSD;
- repeated full success is inert;
- no `audsrv_quit()`, IOP reset or per-session module unload/reload path exists.

The clean build now embeds exact pinned input:

`$(PS2SDK)/iop/irx/audsrv.irx`

through the same deterministic `bin2c` mechanism used for current resident IRX
inputs.

Memory ownership is exact. Allocation rejects zero/invalid alignment and
size-overflow cases, publishes a bounded allocation record only after actual
allocation/alignment success, remembers the raw pointer separately from the
aligned caller pointer, and prevents runtime release while any allocation
remains live.

The session synchronization operation table is backed by its own AUDIO-owned
semaphore. It is not the media-clock binding, Transport synchronization or a
global mutex.

Thread ownership is monotonic and fail closed:

- failed CreateThread creates no slot;
- successful create publishes exactly one not-started slot;
- failed StartThread leaves that slot not-started and owned;
- started authority is published only after successful StartThread;
- the trampoline invokes the accepted session entry, signals retained
  completion and exits normally;
- join retains a consumed completion observation so a later status failure
  cannot make it wait for the one-shot event again;
- join succeeds only after exact `THS_DORMANT` proof;
- repeated non-dormant observations merely yield and never become success by
  count or elapsed time;
- DeleteThread failure preserves the slot and dormancy fact for retry;
- a never-started slot is also deleted only after actual dormant proof.

Runtime release rejects live thread/allocation ownership. Semaphore IDs are
cleared only after each successful DeleteSema, so partial release failure
preserves the remaining exact owner for retry.

R37 exports no `pstvnc_media_clock_time_ops_t`, timer read, offset, arm or
deadline policy. Accepted R26 remains sole common-clock time-operation owner.

## Independent R37 criterion disposition

All twelve R37 requirements are independently accepted:

- A002-R37-C1=MET
- A002-R37-C2=MET
- A002-R37-C3=MET
- A002-R37-C4=MET
- A002-R37-C5=MET
- A002-R37-C6=MET
- A002-R37-C7=MET
- A002-R37-C8=MET
- A002-R37-C9=MET
- A002-R37-C10=MET
- A002-R37-C11=MET
- A002-R37-C12=MET

R37_SOURCE_COMPLETE=YES
R37_FOREMAN_ACCEPTED=YES

## Exact machine evidence

Final-source GitHub Actions run:

`36272813140`

independently confirms:

- branch `ledge/h1-all-guns`;
- exact head `87069d82787c4b8c66a759ef9ef2a1bc7af03b53`;
- attempt 1;
- conclusion SUCCESS.

Successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

The immutable closeout head
`93e3271b42b5dcad1d83d7126d989306629d0e11`
has GitHub Actions run `36272934134`, attempt 1, with host-unit,
project-check, dictionary-long, ps2-compile and ps2-link all successful.

Focused/canonical evidence includes:

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
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact accepted linked identity:

`ELF_PRISTINE_SHA256=f7270f3f738ab497dbe9e5163df24984056b6d5fdb0ca5286459d59fc32687a5`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=776dd4923ca3302585bd31e630f7e9914a45cd2c4b5980c4766ad0dfdbf4b2b9`

`PT_LOAD_BYTES=550164`.

Evidence classification:

- HOST_TESTED=PASS
- PROJECT_CHECK=PASS
- STRICT_DICTIONARIES=PASS
- PS2_COMPILE=PASS
- PS2_LINK=PASS
- CURRENT_SOURCE_REPRODUCIBILITY=PASS
- MACHINE_EVIDENCE=GITHUB_ACTIONS
- INDEPENDENT_VALIDATION=NOT_RUN
- OPERATOR_OBSERVED=NO
- HARDWARE_QUALIFIED=NO
- HARDWARE_PENDING=YES

This becomes the newest fully Foreman-accepted source/build identity.

## Newly exposed AUDIO completion dependency

Ordinary Application AUDIO composition is not yet safe.

A006 explicitly requires finite PCM completion and owner failures to become
explicit lifecycle facts observable by the steady-state Application
coordinator.

Current clean AUDIO session ownership exposes:

- blocking `pstvnc_audio_session_join()`;
- `pstvnc_audio_session_outcome()` only after successful join;
- no nonblocking owner completion/status seam.

Therefore Application has only two currently-invalid choices:

1. call blocking join in the ordinary steady-state loop, potentially stalling
   RFB/Input/MPEG service for the remaining AUDIO producer lifetime; or
2. infer AUDIO completion from Transport `producer_done + empty`, which is not
   proof that the AUDIO worker has completed AUDSRV work or is reclaimable.

The correct lower seam is a nonblocking AUDIO-owner completion publication.

That status must report only whether the exact started worker completed. It may
not publish outcome before join, and it may not infer lifecycle state from
Transport queue/end, clock state, diagnostics, elapsed time or polling count.

The accepted R37 retained completion/dormancy mechanism is the correct concrete
PS2 source for that owner fact.

## Foreman state write

Foreman State advanced from revision 0079 to revision 0080 in:

`5575434111c963749faeff7055abaf589eb82dd1`
— `docs(ledge): accept R37 and issue audio completion seam`.

Revision 0080:

- accepts R37 at exact source/log authority;
- records the new accepted linked identity;
- publishes exactly one AUDIO completion-publication prerequisite;
- keeps Application AUDIO composition and Transport AUDIO product activation
  deferred;
- defers final AUDIO start policy to the later Application packet rather than
  silently coupling it to manual MPEG activation;
- keeps automatic MPEG recalibration as optional/deferred policy.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A002-AUDIO-SESSION-COMPLETION-PUBLICATION-R38`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a002-config-audio-clock`
`WORKER_KEY=interactive`.

R38 must add one nonblocking AUDIO-session owner completion/status seam.

Its central contract is:

1. started-and-active returns ACTIVE immediately;
2. actual worker completion becomes COMPLETED without blocking join;
3. one-shot completion observation is retained across retries;
4. unknown/kernel status failure returns an error, not ACTIVE;
5. COMPLETED still does not authorize outcome/release until existing join proves
   visibility/dormancy;
6. Transport producer/end state is never used as a substitute completion fact;
7. partial-start and R37 reclamation behavior remain unchanged;
8. no Application AUDIO composition enters R38.

After R38 acceptance, the expected next dependency is ordinary A006 AUDIO
Application composition using R36 selected values, R37 PS2 mechanism, R38
nonblocking completion, R26 common clock and the existing Transport AUDIO rider.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A002-AUDIO-SESSION-COMPLETION-PUBLICATION-R38`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A002-AUDIO-SESSION-COMPLETION-PUBLICATION-R38
