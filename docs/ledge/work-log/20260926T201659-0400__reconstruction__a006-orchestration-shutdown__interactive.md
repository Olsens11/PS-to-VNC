# Ledge Reconstruction work log — R42 ordinary cross-platform AUDIO composition

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T20:16:59-04:00
COMPLETED_AT=2026-09-26T21:17:57-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=b1153979b1e013110a16349e3b7051155431c1c6
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Authority and scope

Recovered Foreman State revision 0084 and executed only
`A006-ORDINARY-CROSS-PLATFORM-AUDIO-COMPOSITION-R42`.

Assigning Foreman State commit:
`560d4518781e98e77247cc12c9bbb4e84da56234`.

Assigning Foreman log / Reconstruction starting authority:
`b1153979b1e013110a16349e3b7051155431c1c6`.

Final pre-log reconciled source authority:
`66d07aacb1ce76aa55d820ea14d4ef7bf999bbcf`
— `test(audio): verify reconciled R42 source`.

That source is 49 commits ahead and zero behind the assigning Foreman log.

R42 did not change Wire version, selected R36 numeric values, lower Transport
mechanisms, lower AUDIO playback/session/runtime mechanism, R26 clock mechanism,
MPEG/RFB mechanisms, Input/UI/Display product mechanism, or H1 forensic source.

## Ordinary PS2 AUDIO composition

Product entry now resolves selected RFB, R36 AUDIO, MPEG, and media-clock
authority before platform startup. Missing/invalid AUDIO authority therefore
acquires no platform/session ownership.

After IOP readiness, Application prepares the accepted R37 resident LIBSD/AUDSRV
seam exactly once for the resident process. Each physical Wire attempt then:

1. opens one AUDIO+MPEG-capable Transport session with AUDIO still dormant;
2. acquires one exact Transport ticket;
3. constructs one fresh R26 clock binding/time-ops set;
4. constructs one fresh inert R41 AUDIO coordinator using that exact ticket,
   selected R36 values and the current common clock.

Application crosses the one-shot AUDIO start edge only after
`pstvnc_app_mpeg_product_has_started_run()` proves a genuinely successful
protected MPEG start. Calibration cancellation or rolled-back/nonstarted MPEG
cannot start AUDIO. No same-session AUDIO restart is permitted.

ACTIVE R41 service is called at ordinary main-loop cadence and remains
nonblocking. Normal finite AUDIO completion remains terminal for that local AUDIO
instance and is not interpreted as permission to start a replacement inside the
same Wire Session.

## First-presentation synchronization

Before the common clock is armed, ordinary MPEG live presentation service is
withheld unless R41's public non-consuming readiness seam reports ready.

R41's focused accepted owner proof establishes the selected threshold semantics:

- 458751 live AUDIO bytes => not ready;
- 458752 live AUDIO bytes => ready;
- finite producer_done => ready, including empty finite input.

The R42 integration fixture scripts false then true readiness and proves
Application suppresses the first MPEG live-service edge until that public fact
becomes true. It does not fabricate byte counts inside the Application fixture.

Application does not read/dequeue AUDIO, return CREDIT, infer readiness from
elapsed time/poll count, or arm/reset the common clock. The first actual MPEG
presentation remains the clock-owner arm edge. After the clock is armed, startup
reservoir readiness is no longer consulted.

Normal R35 MPEG retirement does not stop or restart AUDIO.

## Abnormal teardown

Application now converges MPEG and AUDIO media debt through one retained-session
teardown sequence:

1. prove Input worker shutdown/dormancy;
2. call Transport begin-abort once when either media owner has session debt;
3. service MPEG local abort progress if owned;
4. service AUDIO R41 abort progress if owned;
5. if either remains pending, yield at existing 1 ms cadence and retry;
6. release the R26 media-clock binding only after both media owners are ready;
7. close retained Transport storage;
8. only then admit a replacement physical Wire attempt.

The cadence delay is never completion authority. No timeout or retry count
manufactures MPEG/AUDIO readiness.

AUDIO activation history remains enclosing-session teardown debt even after clean
FINITE_COMPLETE, so later Wire failure still uses retained-session close rather
than a plain abort. A no-media attempt preserves R16B's ordinary one-shot
Transport-abort path.

## Ordinary Pi AUDIO composition

New `pi/audio_product_profile.py` owns only ordinary composition values and
factory policy:

- generated/selected R36 PCM authority;
- adopted `2.0` second retirement escalation horizon;
- one lazy exact-session R39 `AudioPcmProducer` factory.

`pi/wire_runtime.py` now injects that lazy factory into the existing
WireServer. Q4 acceptance creates no AUDIO capture owner. The first exact
channel-2 CREDIT creates one R39 owner for that accepted session; later CREDIT
reuses it. Sessions with no AUDIO CREDIT create no capture owner.

The 2.0-second value is only terminate-to-kill escalation timing. It is not
success/dormancy authority. R39 still requires actual process exit and
reader-thread dormancy for successful retirement.

The tracked Pi staging path now includes:

- `audio_runtime_profile_generated.py`;
- `audio_pcm_producer.py`;
- `audio_product_profile.py`.

Stage/verify/remove remain exact-byte/fail-closed and do not reload, enable,
start, stop, or restart the systemd service.

## Regression-boundary updates

Historical R27/R37/R40/R41 source checks were advanced only where R42
intentionally crosses their former deferred-composition boundary:

- R27 now requires exactly one AUDIO+MPEG Transport constructor and AUDIO profile
  selection/resident preparation in the accepted startup order;
- R37 permits exactly the resident-prepare seam while continuing to forbid
  direct Application session-runtime ownership;
- R40 requires the selected lazy Pi AUDIO factory while retaining dormant
  PS2 activation and first-CREDIT lazy-capture guarantees;
- R41 now requires ordinary Application to consume only its public API and
  forbids bypass into lower AUDIO/Transport queue mechanisms.

All corresponding historical regressions pass on the final R42 source head.

## Build, topology, dictionaries and documentation

R42 updated ordinary build/test dependencies, Pi staging, systemd explanatory
comments, module lifecycle, source topology, Pi status documentation, and the
living file/service map.

Deterministic dictionary reconciliation was triggered by exact message
`tooling(symbols): run deterministic dictionary reconciliation`.
Automation commit `da00cec1d7892a1ef4e7cf8e344817c9906a3d03` changed only:

- `src/SYMBOLS.md`;
- `pi/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

Final dictionary counts include:

- `src:1130`;
- `pi:922`;
- total product symbols: `7347`.

## Exact final evidence

Canonical exact-source GitHub Actions run:
`36285048528`, attempt 1, exact head
`66d07aacb1ce76aa55d820ea14d4ef7bf999bbcf`, completed SUCCESS.

Job disposition:

- host-unit = SUCCESS
- project-check = SUCCESS
- dictionary-long = SUCCESS
- ps2-compile = SUCCESS
- ps2-link = SUCCESS
- dictionary-reconcile = correctly SKIPPED

Selected exact-head evidence:

- `APP_AUDIO_PRODUCT_TEST=PASS`
- `APP_AUDIO_PRODUCT_SOURCE_TEST=PASS`
- `APP_AUDIO_COMPOSITION_SOURCE_TEST=PASS`
- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`
- `PI_AUDIO_PRODUCT_COMPOSITION_TEST=PASS`
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`
- `TRANSPORT_AUDIO_ACTIVATION_SOURCE_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`
- `transport_audio_test: PASS`
- `media_clock_test: PASS`
- `APP_MPEG_PRODUCT_TEST=PASS`
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`
- `SOURCE_TOPOLOGY_CONTRACT=PASS`
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`
- `WORK_LOG_CHECK=PASS`
- `SOURCE_DICTIONARIES=PASS`
- `PS_TO_VNC_PROJECT_CHECK=PASS`
- `ISSUE7_LINKED_BUILD=PASS`
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

## Linked identity / hardware boundary

Accepted R41:

`PT_LOAD_SHA256=dd7d5f7bca743d3a505c809c53f78474559105244d91b29f62c5d83ec2bb4dc0`
`PT_LOAD_BYTES=555284`.

R42 final source:

`ELF_PRISTINE_SHA256=bfc6a2fb9b22db2a170b6687b8625c7b6b85bb99fad6b7accc7002e9dea72021`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=6375db546f08a60b2b8bf34aa6786e8f740f44213b64fedecfb4081f5fe9f065`
`PT_LOAD_BYTES=556180`.

Therefore the loadable program changed and R42 creates new hardware
qualification debt.

R42_SOURCE_COMPLETE=YES_WITHIN_PACKET
R42_HOST_TESTED=PASS
R42_PROJECT_CHECK=PASS
R42_STRICT_DICTIONARIES=PASS
R42_PS2_COMPILE=PASS
R42_PS2_LINK=PASS
R42_CURRENT_SOURCE_REPRODUCIBILITY=PASS
R42_MACHINE_EVIDENCE=GITHUB_ACTIONS
R42_PS2_PT_LOAD_CHANGED=YES
R42_INDEPENDENT_VALIDATION=NOT_RUN
R42_OPERATOR_OBSERVED=NO
R42_HARDWARE_QUALIFIED=NO
R42_HARDWARE_PENDING=YES

## Worker criterion disposition

Worker findings only; not Foreman acceptance.

1. Selected R36 AUDIO authority resolved before startup — MET.
2. R37 resident preparation is process-scoped and once-only — MET.
3. Every Wire attempt gets dormant AUDIO+MPEG Transport and a fresh R41 owner —
   MET.
4. AUDIO starts only after genuinely successful protected MPEG start and at most
   once per Wire Session — MET.
5. R41 steady service is nonblocking; finite AUDIO completion cannot restart —
   MET.
6. First MPEG presentation is gated only by R41 public reservoir/producer_done
   readiness until first clock arm — MET.
7. Application does not consume AUDIO bytes or take lower AUDIO/clock ownership —
   MET.
8. Normal MPEG retirement does not retire/restart AUDIO — MET.
9. Abnormal MPEG+AUDIO teardown shares one retained-session fence and real-owner
   readiness; delay/count never manufactures success — MET.
10. Pi ordinary runtime injects exactly one lazy selected R39 AUDIO factory and
    creates capture only on first exact AUDIO CREDIT — MET.
11. 2.0-second Pi retirement value remains escalation-only; actual
    process/thread dormancy remains required — MET.
12. Wire version/R36 selected values/lower mechanisms remain unchanged and all
    canonical deterministic gates are green — MET.

## Next pickup

The Interactive Reconstruction Worker stops here.

The Foreman must independently inspect R42 source, tests, selected-value
composition, first-presentation synchronization, retained-session teardown,
Pi lazy factory/staging, changed PT_LOAD identity, machine evidence, and this
immutable record before accepting/rejecting R42 or issuing any successor packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R42_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION
