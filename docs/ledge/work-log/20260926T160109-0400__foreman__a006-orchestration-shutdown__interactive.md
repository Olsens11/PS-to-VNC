# Ledge Foreman work log — accept R36 and issue PS2 AUDIO execution binding

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T16:01:09-04:00
COMPLETED_AT=2026-09-26T16:06:00-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=146d9c831b48c48caadeb9276622febe136468eb
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R36 selected AUDIO runtime profile work.

Live pickup authority was:

`146d9c831b48c48caadeb9276622febe136468eb`
— `docs(work-log): record R36 selected audio runtime profile`.

The round independently recovered:

- Foreman State revision 0078;
- the exact R36 immutable Reconstruction closeout;
- the complete four-commit R36 source/test/dictionary range;
- A002 CONFIG/AUDIO/common-clock audit authority;
- accepted R26 common-media-clock binding;
- clean AUDIO playback, AUDSRV service and session lifecycle owners;
- the current product PS2 platform/build seams;
- exact source-head and closeout-head canonical GitHub Actions evidence.

No user terminal, Pi-local proxy, hardware action, Validation work, or Foreman
product-behavior change was required.

## Returned R36 range

Assigning Foreman/log authority:

`b8bd7e71beedf7b8798b976df8ffc7275589fa8b`.

Final pre-log R36 source authority:

`a5528b08fa48382545ee9e325c5f103b4e38c0b9`.

Immutable Reconstruction closeout:

`146d9c831b48c48caadeb9276622febe136468eb`.

Independent compare proves the source range is exactly:

- 4 commits ahead;
- 0 behind.

Changed paths are confined to the packet-authorized Configuration/profile,
generator, focused-test, canonical build/topology and dictionary surface.

No `src/audio/session.*`, playback, AUDSRV mechanism, Transport, media-clock,
Application, Input/UI/RFB/Display/MPEG, Pi or Wire/protocol implementation was
modified.

## Independent R36 findings

R36 defines one Configuration-owned AUDIO runtime profile using existing narrow
owner types:

- `pstvnc_transport_audio_channel_config_t`;
- `pstvnc_config_pcm_profile_t`;
- `pstvnc_audio_session_values_t`.

The exact selected values are independently confirmed as:

Transport AUDIO channel:

- queue capacity 524288;
- initial credit 524288;
- credit batch 4096;
- flush-on-empty enabled;
- credit return enabled.

PCM:

- 48000 Hz;
- 2 channels;
- 16 bits/sample;
- volume 100 percent.

AUDIO session:

- playback buffer 4096 bytes;
- startup reservoir 458752 bytes;
- worker priority 65;
- worker stack 16384 bytes;
- reservoir poll 1000 us;
- clock poll 1000 us.

The two poll cadences remain separately named owner fields despite sharing the
current selected numeric value.

The runtime C object contains no H1 profile ID/forensic string authority and no
media-clock offset. R26 therefore remains sole owner of the selected common
audio presentation offset.

The selected Configuration object is private static-const state copied into
caller-owned storage only after validation. Mutating a returned copy cannot
change later selected authority.

The deterministic JSON generator validates schema, bounds, queue/credit
relations, PCM shape, buffer/reservoir limits, worker bounds, the two poll
values and provenance. No runtime JSON parser or clamping path was introduced.

## Independent R36 criterion disposition

All twelve R36 requirements are independently accepted:

- A002-R36-C1=MET
- A002-R36-C2=MET
- A002-R36-C3=MET
- A002-R36-C4=MET
- A002-R36-C5=MET
- A002-R36-C6=MET
- A002-R36-C7=MET
- A002-R36-C8=MET
- A002-R36-C9=MET
- A002-R36-C10=MET
- A002-R36-C11=MET
- A002-R36-C12=MET

R36_SOURCE_COMPLETE=YES
R36_FOREMAN_ACCEPTED=YES

## Exact machine evidence

Final-source GitHub Actions run:

`36266535951`

independently confirms:

- branch `ledge/h1-all-guns`;
- exact head `a5528b08fa48382545ee9e325c5f103b4e38c0b9`;
- attempt 1;
- conclusion SUCCESS.

Successful canonical jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

Observed focused and regression evidence includes:

- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`;
- `AUDIO_RUNTIME_PROFILE_GENERATED=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`;
- `audio_playback_test: PASS`;
- `audio_audsrv_service_test: PASS`;
- `audio_session_test: PASS`;
- `media_clock_test: PASS`;
- `RFB_RUNTIME_PROFILE_GENERATED=PASS`;
- `MPEG_RUNTIME_PROFILE_GENERATED=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The immutable closeout head
`146d9c831b48c48caadeb9276622febe136468eb`
has exact GitHub Actions run `36266656183`, attempt 1, with host-unit,
project-check, dictionary-long, ps2-compile and ps2-link all successful.

Exact accepted R36 linked identity:

`ELF_PRISTINE_SHA256=8dc8b419cc80d655bdaaccb9b9f5534f6578f5d57c4623b396ab3c35d92ff8db`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=5c59e5a914d38f5dbe7c0ad9d72df602f91753a1a002b2bfa112f48e4978d423`

`PT_LOAD_BYTES=528020`.

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

## Next dependency review

The next audited AUDIO dependency is below ordinary Application composition.

Clean `src/audio/session.c` already owns the correct session worker/resource
lifecycle but requires caller-supplied:

- aligned memory operations;
- EE thread create/start/join/destroy operations;
- session-local synchronization;
- common-clock time operations.

The product already has R26's PS2 media-clock binding, which supplies the
required common-clock time operations and must remain the sole clock/timer owner.

What is missing is the AUDIO-domain PS2 execution binding for memory/thread/sync,
plus the resident AUDSRV/LIBSD module foundation required by the accepted A002
service lifetime.

The H1 mechanism confirms the required PS2 primitives, but its fixed
three-second bounded shutdown poll is not product correctness authority. The
clean owner must prove actual worker completion/dormancy; elapsed time or poll
count cannot manufacture successful reclamation.

The binding also has to preserve the accepted AUDIO session partial-start
semantics: if thread creation succeeds but StartThread fails and immediate
destroy cannot complete, exact created-but-never-started ownership must remain
retryable rather than freeing stack/session storage underneath the thread slot.

## Foreman state write

Foreman State advanced from revision 0078 to revision 0079 in:

`a8e65a5835e4956cc993e438127340a930dbc914`
— `docs(ledge): accept R36 and issue PS2 audio binding`.

Revision 0079:

- accepts R36 at exact source/log authority;
- records the new accepted linked identity;
- publishes exactly one lower AUDIO execution-binding packet;
- preserves R26 as sole common-clock mechanism owner;
- keeps Transport AUDIO product activation and ordinary Application AUDIO
  composition deferred;
- keeps automatic MPEG recalibration as deferred optional policy.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A002-PS2-AUDIO-EXECUTION-BINDING-R37`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a002-config-audio-clock`
`WORKER_KEY=interactive`.

R37 must provide the concrete PS2 AUDIO execution adapter and resident service
foundation only.

Its core proof obligations are:

1. resident LIBSD/AUDSRV preparation is explicit, idempotent and never uses
   `audsrv_quit()`;
2. the pinned PS2SDK AUDSRV IRX is embedded deterministically;
3. memory ownership/alignment and live-allocation tracking are exact;
4. synchronization is AUDIO-session local;
5. one EE thread slot has exact create/start ownership;
6. worker completion is retained independently from elapsed time;
7. join survives a consumed completion signal followed by a status failure and
   remains retryable;
8. actual THS_DORMANT proof is required before started-thread destroy;
9. never-started created-thread cleanup is retryable and fail-closed;
10. runtime/semaphore release is monotonic and retryable;
11. no second common-clock/timer owner is introduced;
12. Application AUDIO activation remains out of scope.

After R37 acceptance, the next expected dependency is A006 ordinary AUDIO
Application composition: open the appropriate Transport AUDIO+MPEG rider,
construct the AUDIO session using R36/R37/R26, classify finite completion and
failure, and prove AUDIO dormancy before retained Transport reclamation.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A002-PS2-AUDIO-EXECUTION-BINDING-R37`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A002-PS2-AUDIO-EXECUTION-BINDING-R37
