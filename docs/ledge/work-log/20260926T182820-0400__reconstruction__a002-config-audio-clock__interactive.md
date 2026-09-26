# Ledge Reconstruction work log — R39 Pi AUDIO PCM producer/rider

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T18:28:20-04:00
COMPLETED_AT=2026-09-26T18:43:26-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=88d5d86717e516e4588579d05c7bea7af6f79e54
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Interactive Reconstruction Worker round received the baton at exact live
authority `88d5d86717e516e4588579d05c7bea7af6f79e54`, Foreman State revision
0081, with exactly one active packet:

`A002-PI-AUDIO-PCM-PRODUCER-R39`.

The packet authorized only the clean Pi-side A002 PCM producer/rider mechanism:
selected default-sink PCM capture, bounded session-local buffering, exact
channel-2 CREDIT consumption, readiness publication into the existing sole
WireServer I/O loop, exact AUDIO DATA serialization, deterministic generated Pi
profile authority, and proof-driven cleanup.

The packet explicitly deferred ordinary `pi/wire_runtime.py` AUDIO activation,
PS2 Application AUDIO composition, final AUDIO start policy, Transport AUDIO
mechanism changes, Wire compatibility changes, MPEG/RFB policy changes,
automatic recalibration, operator observation and hardware qualification.

## Final pre-log source authority

Final source/test/documentation/dictionary authority before this immutable log:

`66b44a2e68db99b511504e998e13def4a6acb7cf`
— `test(pi-audio): verify reconciled R39 source`.

It is exactly nine commits ahead and zero behind the assigning Foreman head:

1. `cb3e8a67a1f298637a79126e07ad2c849c08ebcd`
   — `config(audio): generate R39 Pi PCM projection`
2. `3d71229fcf44d299a6b5a8387823bfaf765707fa`
   — `pi(audio): add R39 PCM producer owner`
3. `ec8f43a38106385930e46f20754548fe15bd55f8`
   — `pi(wire): compose optional R39 audio rider`
4. `b74d51fb87db285f101519643cc2e0e15baffc83`
   — `pi(audio): preserve failed retirement proof`
5. `e05bc211d6bd479c2ffe386e40e68f127e4be15c`
   — `test(pi-audio): prove R39 producer and Wire ownership`
6. `993104db81b573adc18d8c4d1dd7351cd0a98d3e`
   — `docs(pi-audio): record R39 owner lifecycle`
7. `81bec58f31934c5856d6198279dbb844e11ae2c0`
   — deterministic dictionary-reconciliation trigger
8. `5eceea6ce42cc6527046b29cfb71fd388d007407`
   — automation-generated current clean dictionary reconciliation
9. `66b44a2e68db99b511504e998e13def4a6acb7cf`
   — exact reconciled-source verification trigger

The exact changed range is confined to:

- `pi/audio_pcm_producer.py`
- `pi/audio_runtime_profile_generated.py`
- `pi/wire_protocol.py`
- `pi/wire_server.py`
- `pi/SYMBOLS.md`
- `scripts/generate-audio-runtime-profile.py`
- `tests/unit/config_audio_runtime_profile_generation_test.py`
- `tests/unit/pi_audio_pcm_producer_test.py`
- `tests/unit/pi_audio_wire_server_test.py`
- `tests/Makefile`
- `docs/development/source-topology.md`
- `docs/development/module-lifecycle.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No `pi/wire_runtime.py`, PS2 product source, Transport implementation,
Application source, media-clock source, MPEG producer/control policy, RFB
provider policy, Input/UI/Display source, H1 forensic source, systemd live
activation, or Wire/product compatibility version changed.

## Generated Pi AUDIO authority

The existing R36 canonical source remains:

`src/config/audio_runtime_profile.json`.

`scripts/generate-audio-runtime-profile.py` now deterministically projects the
same selected authority to both the accepted C header and the narrow Pi producer
projection `pi/audio_runtime_profile_generated.py`.

The Pi projection is exactly:

- channel window = 524288 bytes;
- PCM rate = 48000 Hz;
- channels = 2;
- bits per sample = 16;
- frame size = channels * bytes-per-sample = 4 bytes.

The generation check proves the Pi artifact becomes stale when it differs from
the canonical profile. No hand-maintained duplicate numeric tuning authority was
added to the producer or WireServer.

The Wire payload maximum remains independently owned by
`wire_protocol.MAX_PAYLOAD_BYTES=8192`.

## Pi AUDIO PCM owner

`pi/audio_pcm_producer.py` is the new session-scoped mechanism owner.

It:

- binds to one exact nonzero Wire Session identity;
- resolves `@DEFAULT_AUDIO_SINK@` with
  `wpctl inspect @DEFAULT_AUDIO_SINK@`;
- extracts the qualified PipeWire `node.name` and uses its `.monitor`;
- launches `pw-record --target <monitor> --rate 48000 --format s16 --channels 2 -`
  from generated authority;
- owns one capture process, one reader thread, one condition-bounded byte spool,
  exact PS2 AUDIO credit and one session-local readiness socketpair;
- buffers at most the selected 524288-byte channel window;
- emits only complete 4-byte PCM frames;
- emits at most the smallest of caller maximum, Wire maximum, available CREDIT
  and complete buffered PCM;
- decrements credit by exactly the bytes admitted for one outgoing DATA payload;
- publishes local activity through its private wake descriptor rather than
  writing the PS2 socket itself.

No independent PS2-facing socket or sequence allocator exists in the AUDIO
owner.

## Sole Wire I/O and sequencing

`pi/wire_server.py` accepts one optional injected exact-session AUDIO owner in
parallel with the already-accepted RFB and MPEG riders.

The existing sole `WireConnectionOwner`:

- dispatches exact channel-2 CREDIT to the injected AUDIO owner;
- includes the AUDIO owner's local readiness descriptor in the same
  `select.select()` readiness loop used for existing riders;
- asks the AUDIO owner for at most one credit-authorized aligned PCM payload;
- serializes that payload only through the existing
  `_send_active_frame()` path;
- therefore preserves one physical PS2 writer and one global send sequence
  across RFB, AUDIO and MPEG.

The deterministic combined-rider fixture observes one RFB frame, one AUDIO DATA
frame and one MPEG DATA frame with consecutive WireServer-owned global send
sequence numbers.

A channel-2 CREDIT with no injected AUDIO owner is unsupported and fails the
active Wire Session closed rather than creating hidden AUDIO policy.

## Failure and cleanup semantics

Unexpected capture EOF is failure, not normal finite completion.

A malformed final PCM tail that is not frame-aligned is failure.

Capture discovery/launch failure is owner failure.

Reader failure is retained and surfaced through owner health.

Credit overflow beyond the selected channel window is rejected.

Ordinary R39 cleanup does not serialize the existing zero-length channel-2 DATA
producer-done representation. That representation remains explicitly encoded
and classified in `pi/wire_protocol.py`, but R39 neither invents nor emits it
during EOF/cleanup.

Close is proof-driven:

- publish retiring state and wake any buffer wait;
- request process termination;
- wait within explicit injected retirement authority;
- if still live, kill and wait only within the remaining bound;
- join the reader thread within the remaining bound;
- require both process and reader retirement for successful proof.

A timeout or still-live process/thread is failure, never successful cleanup.
Failed retirement proof is retained across repeated `close()` calls rather than
being converted to success by idempotence.

WireServer marks the session failed when AUDIO cleanup cannot prove retirement.
A newly-created owner is also closed if exact-session attachment itself fails.

## Ordinary product activation remains absent

`pi/wire_runtime.py` was not changed.

The ordinary composed product server still supplies only the accepted RFB and
MPEG factories. Its `audio_pcm_factory` remains `None`.

Therefore R39 creates no ordinary AUDIO capture, no final AUDIO start policy,
and no PS2 Application AUDIO startup. Those remain successor A006 composition
work after Foreman acceptance.

## Deterministic focused evidence

The R39 producer fixture proves:

- exact generated 524288/48000/2/16 authority and derived 4-byte frame size;
- exact default-sink monitor discovery;
- exact selected `pw-record` command;
- missing/malformed monitor identity fails closed;
- process launch failure fails closed;
- no CREDIT means no AUDIO emission despite buffered PCM;
- sub-frame CREDIT cannot emit;
- exact aligned CREDIT emits and decrements exactly;
- one payload is bounded by 8192 and complete PCM frames;
- credit cannot exceed the selected channel window;
- the producer spool never exceeds the selected channel window;
- unexpected EOF fails;
- malformed final frame tail fails;
- stubborn process/thread retirement cannot become success by timeout;
- successful close is idempotently successful while failed proof remains failed.

The R39 Wire fixture proves:

- exact AUDIO CREDIT framing/classification;
- ordinary nonempty AUDIO DATA framing/classification;
- explicit zero-length AUDIO producer-done framing remains distinct;
- no-owner channel-2 traffic fails closed;
- RFB/AUDIO/MPEG output shares the one WireServer global sequence;
- failed AUDIO retirement makes the Wire Session fail;
- consecutive Wire Sessions receive fresh AUDIO owners and distinct session IDs;
- ordinary product runtime has no AUDIO factory and imports no R39 AUDIO owner.

Current Pi Wire/RFB/MPEG suites remained green in canonical host evidence.

## Canonical exact-source evidence

Exact final-source GitHub Actions run:

`36277134548`

checked out exact head
`66b44a2e68db99b511504e998e13def4a6acb7cf`, attempt 1, and completed
SUCCESS.

Canonical job disposition:

- host-unit = SUCCESS
- project-check = SUCCESS
- dictionary-long = SUCCESS
- ps2-compile = SUCCESS
- ps2-link = SUCCESS
- dictionary-reconcile = correctly SKIPPED

Relevant exact-source output includes:

- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`
- `transport_audio_test: PASS`
- `audio_playback_test: PASS`
- `audio_audsrv_service_test: PASS`
- `audio_session_test: PASS`
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`
- `audio_ps2_runtime_test: PASS`
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`
- `media_clock_test: PASS`
- current Pi Wire/RFB/MPEG unit suites = PASS
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`
- `SOURCE_TOPOLOGY_CONTRACT=PASS`
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`
- `SOURCE_DICTIONARIES=PASS`
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`
- `WORK_LOG_CHECK=PASS`
- `PS_TO_VNC_PROJECT_CHECK=PASS`
- `ISSUE7_LINKED_BUILD=PASS`
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The first behavior-bearing R39 run correctly exposed only the expected new-file
dictionary/topology coverage debt after focused host tests passed. Deterministic
dictionary reconciliation added exactly the Pi dictionary and generated portal
coverage, and the exact reconciled-source run above closes every canonical gate
green.

## PS2 identity and qualification boundary

R39 changes only Pi/tooling/tests/documentation and therefore must not change PS2
loadable bytes. Canonical exact-source link evidence confirms exact identity with
accepted R38:

`ELF_PRISTINE_SHA256=3b8319a17aa57e50a75399b1eb4ac0c35e59168f2805a1d7d531bd68d92450c6`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=a5a048b8e96650bcce751c3899fb1491d7a41d5b7c2615e60e3cf4779f726313`

`PT_LOAD_BYTES=550804`.

R39_PS2_PT_LOAD_CHANGED=NO

R39 creates no new PS2 hardware-qualification debt. The inherited R38 physical
qualification debt remains exactly what it was; this Reconstruction Worker makes
no operator-observed or hardware-qualified claim.

Evidence classification:

- R39_SOURCE_COMPLETE=YES_WITHIN_PACKET
- R39_HOST_TESTED=PASS
- R39_PROJECT_CHECK=PASS
- R39_STRICT_DICTIONARIES=PASS
- R39_PS2_COMPILE=PASS
- R39_PS2_LINK=PASS
- R39_CURRENT_SOURCE_REPRODUCIBILITY=PASS
- R39_MACHINE_EVIDENCE=GITHUB_ACTIONS
- R39_PS2_PT_LOAD_CHANGED=NO
- R39_PI_LOCAL_STAGING=NOT_REQUIRED_FOR_SOURCE_PACKET
- R39_INDEPENDENT_VALIDATION=NOT_RUN
- R39_OPERATOR_OBSERVED=NO
- R39_HARDWARE_QUALIFIED=NO
- R39_INHERITED_HARDWARE_PENDING=YES

## Reconstruction Worker requirement disposition

These are Reconstruction Worker findings only, not Foreman acceptance.

1. Generated Pi profile projection uses only R36 selected authority — MET.
2. Exact default-sink PipeWire PCM capture mechanism is reconstructed — MET.
3. AUDIO owner is exact-session scoped and optional/injected — MET.
4. Exact PS2 channel-2 CREDIT is the only DATA admission authority — MET.
5. AUDIO readiness integrates with the existing sole WireServer I/O loop — MET.
6. Producer buffering is bounded and PCM-frame aligned — MET.
7. Existing AUDIO wire semantics and global sequence ownership are preserved —
   MET.
8. Unexpected EOF/error/malformed tail fail closed and do not become finite
   success — MET.
9. Process/thread cleanup requires actual retirement proof — MET.
10. Ordinary Pi AUDIO activation and PS2 Application composition remain absent —
    MET.
11. PS2 linked identity is unchanged from accepted R38 — MET.
12. Focused/current-Pi/A002/R26/R36-R38/canonical evidence closes green — MET.

## Scope, limitations and next pickup

R39 does not activate AUDIO in the ordinary product runtime, decide the final
AUDIO start relationship to Wire admission or MPEG, initialize PS2 Application
AUDIO, change Transport AUDIO semantics, alter Wire/product compatibility,
change MPEG or RFB policy, stage/start systemd services, perform live Pi capture,
run independent Validation, or claim operator/hardware qualification.

There is no Reconstruction blocker at this stopping point.

The Interactive Reconstruction Worker stops here. The Foreman must independently
inspect the returned nine-commit source range, generated Pi authority, one-writer
Wire integration, exact credit/buffering/terminality semantics, canonical
evidence and this immutable closeout before accepting or rejecting R39 and
selecting any successor packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R39_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION
