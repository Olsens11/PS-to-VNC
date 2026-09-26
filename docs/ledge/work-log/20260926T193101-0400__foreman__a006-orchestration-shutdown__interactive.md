# Ledge Foreman work log — accept R40 and issue AUDIO Application coordinator

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T19:31:01-04:00
COMPLETED_AT=2026-09-26T19:40:17-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=b6c539b7cdae87effe1135554e43eccbe487db60
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R40 deferred AUDIO initial-credit activation work.

Live pickup authority was:

`b6c539b7cdae87effe1135554e43eccbe487db60`
— `docs(work-log): record R40 deferred audio activation`.

The round independently recovered:

- Foreman State revision 0082;
- the exact immutable R40 Reconstruction closeout;
- the exact eighteen-commit returned source/test range;
- accepted R26/R36-R39 clock/AUDIO authority;
- current Transport AUDIO activation and consumer seams;
- current Pi lazy AUDIO factory behavior;
- current AUDIO session/R37/R38 completion ownership;
- A002/A003/A006 architecture authority;
- exact source-head and log-head canonical GitHub Actions evidence.

No user terminal, Pi-local proxy, hardware action, Validation work, or Foreman
product-behavior patch was required.

## Returned R40 range

Assigning Foreman/log authority:

`ce02a1b06708aa6054c1ca493f75df467d33cf00`.

Final pre-log R40 source authority:

`add629833183f655533825e69286b95300817936`.

Immutable Reconstruction closeout:

`b6c539b7cdae87effe1135554e43eccbe487db60`.

Independent compare proves:

- 18 commits ahead;
- 0 behind.

Changed product source is confined to:

- `src/transport/runtime.c/.h`;
- `src/transport/bridge.c/.h`;
- `pi/wire_server.py`;
- focused Transport/Pi tests and dictionaries.

No Application source, `pi/wire_runtime.py`, R36 values, AUDIO
session/playback/R37 runtime, R26 clock, MPEG/RFB/Input/UI/Display product source,
H1 forensic source or Wire compatibility version changed.

## Independent R40 findings

Transport now has exact one-way AUDIO admission states:

- DORMANT;
- ACTIVATING;
- ACTIVE.

AUDIO-capable session open allocates selected AUDIO storage/rendezvous resources
but receiver startup sends no channel-2 initial CREDIT.

RFB and MPEG startup credits remain unchanged.

The public exact-ticket activation path:

`pstvnc_transport_audio_activate()`

publishes ACTIVATING under the AUDIO queue owner before invoking the existing
synchronous sole-owner outbound CREDIT rendezvous.

That ordering is the correct race fence: an immediate peer DATA frame caused by
the first CREDIT can be received while the activating caller is still waiting
for physical send completion, without being rejected as stale DORMANT state.

Successful initial-CREDIT submission publishes ACTIVE. Duplicate activation,
stale access, disabled AUDIO or terminal/contradictory state cannot emit another
initial CREDIT.

Failed initial-CREDIT submission terminalizes Transport and leaves ACTIVATING
irreversible; no retry-to-DORMANT path exists.

Nonzero AUDIO DATA and zero-length producer-done are protocol-invalid while
DORMANT. AUDIO read/status/activity consumer seams likewise require exact ACTIVE
state and reject pre-activation use immediately.

Dormant teardown reclaims allocated AUDIO resources without requiring producer
completion.

On Pi, an injected AUDIO factory is now dormant authority at Q4. Accepted Wire
establishment alone does not call it.

The first valid channel-2 CREDIT:

1. decodes the amount once;
2. creates one exact-session R39 owner through the injected factory;
3. attaches that owner to the same session;
4. applies the same credit exactly once.

Later CREDIT reuses the same owner. Factory/attachment/first-credit failures
fail the Wire session and clean created ownership through the correct local
owner. A session ending before first AUDIO CREDIT creates no capture owner.

`pi/wire_runtime.py` remains AUDIO-disabled.

## Independent R40 criterion disposition

All twelve R40 requirements are independently accepted:

- A002-R40-C1=MET
- A002-R40-C2=MET
- A002-R40-C3=MET
- A002-R40-C4=MET
- A002-R40-C5=MET
- A002-R40-C6=MET
- A002-R40-C7=MET
- A002-R40-C8=MET
- A002-R40-C9=MET
- A002-R40-C10=MET
- A002-R40-C11=MET
- A002-R40-C12=MET

R40_SOURCE_COMPLETE=YES
R40_FOREMAN_ACCEPTED=YES

## Exact machine evidence

Final-source GitHub Actions run:

`36278839400`

checked out exact source head
`add629833183f655533825e69286b95300817936`,
attempt 1, and completed SUCCESS.

Successful canonical jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

Observed evidence includes:

- `transport bridge tests passed`;
- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `TRANSPORT_AUDIO_ACTIVATION_SOURCE_TEST=PASS`;
- `transport_mpeg_test: PASS`;
- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`;
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`;
- `media_clock_test: PASS`;
- `audio_playback_test: PASS`;
- `audio_audsrv_service_test: PASS`;
- `audio_session_test: PASS`;
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`;
- `audio_ps2_runtime_test: PASS`;
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`;
- current MPEG/RFB/Application regressions PASS;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact immutable-log-head run:

`36278989589`

at `b6c539b7cdae87effe1135554e43eccbe487db60`, attempt 1, also completed
SUCCESS across host-unit, project-check, dictionary-long, ps2-compile and
ps2-link.

Exact accepted linked identity:

`ELF_PRISTINE_SHA256=b7fc2805a2ac4351466594376df6c3a59ef9c9f3acccea0f3f952ecedebac963`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=7216b06d319ce6cb3e81085427ef7696bf36a0361c46c05a1cd7fe8bc1baebff`

`PT_LOAD_BYTES=551316`.

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

## Post-R40 synchronization dependency

R40 makes the AUDIO start edge explicit, but direct A006 integration would still
be premature without one Application-owned synchronization rule.

R36 selects:

`startup_reservoir_bytes=458752`.

Accepted A002 semantics allow AUDIO to prefill before presentation
synchronization. Accepted A003/R26 semantics require the shared epoch to arm only
at first real MPEG presentation.

If Application were to:

1. complete protected MPEG START;
2. activate/start AUDIO;
3. immediately service the first MPEG frame;

then the first video presentation could arm the common epoch while AUDIO is still
building its selected startup reservoir. AUDIO would later begin from older
queued PCM against an already-running epoch.

The clean dependency is therefore not another Transport or AUDIO mechanism. It
is an Application composition fact:

- successful protected MPEG START is the AUDIO start trigger;
- AUDIO activation/session start follows immediately;
- first MPEG presentation remains held while the AUDIO reservoir is below its
  selected threshold and producer remains live;
- once reservoir-ready, first MPEG presentation may proceed and remains the sole
  R26 epoch-arm boundary;
- if AUDIO finite-producer completion is already explicit, it must not block
  video forever;
- later MPEG generations must not restart the same one-shot session AUDIO owner.

The AUDIO session worker already waits on the common clock after reservoir
readiness. A separate Application coordinator can therefore prove this policy
without modifying lower mechanisms.

## Foreman state write

Foreman State advanced from revision 0082 to revision 0083 in:

`69dc1f11856925bf4f8dbe62d9a719177d772db2`
— `docs(ledge): accept R40 and issue audio app coordinator`.

Revision 0083:

- accepts R40 at exact source/log authority;
- records the new accepted PS2 linked identity;
- publishes exactly one A006 AUDIO Application lifecycle coordinator packet;
- keeps ordinary `app.c` integration and Pi AUDIO factory activation deferred;
- keeps R36 selected values and Wire version unchanged;
- keeps automatic MPEG recalibration optional/deferred.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A006-AUDIO-APPLICATION-LIFECYCLE-COORDINATOR-R41`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a006-orchestration-shutdown`
`WORKER_KEY=interactive`.

R41 must build a narrow, independently testable Application AUDIO coordinator
without touching ordinary `app.c` or Pi product activation.

Its key obligations are:

1. inert initialization from exact Transport/profile/clock/time authority;
2. explicit R37 runtime -> R40 activation -> AUDIO session start order;
3. local rollback before activation and retained-session failure after activation;
4. R38 nonblocking steady-state completion service;
5. exact finite-completion classification with no in-session AUDIO restart;
6. non-consuming selected-reservoir readiness for first-presentation gating;
7. no media-clock arm/reset inside AUDIO composition;
8. exact retained Transport proof before abnormal AUDIO reclamation;
9. request-stop/join/outcome/release/runtime-release teardown ordering;
10. retryable preservation of partial-start ownership;
11. no Transport/media-clock close ownership absorbed into the coordinator;
12. no ordinary Application/Pi activation yet.

After R41 acceptance, the expected successor is the ordinary cross-platform A006
composition packet that wires this coordinator into `app.c`, opens
AUDIO+MPEG Transport, prepares resident AUDIO service, starts AUDIO at the first
successful protected MPEG start, gates the actual first-frame service boundary,
and injects the lazy R39 factory into ordinary Pi runtime.

## Next pickup

The permanent Interactive Reconstruction Worker must recover live repository
authority and execute only:

`A006-AUDIO-APPLICATION-LIFECYCLE-COORDINATOR-R41`.

It must emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A006-AUDIO-APPLICATION-LIFECYCLE-COORDINATOR-R41
