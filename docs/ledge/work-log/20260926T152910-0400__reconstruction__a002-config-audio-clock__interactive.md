# Ledge Reconstruction work log — R36 selected AUDIO runtime profile

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T15:29:10-04:00
COMPLETED_AT=2026-09-26T15:35:56-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-config-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=b8bd7e71beedf7b8798b976df8ffc7275589fa8b
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Reconstruction round received the baton at live branch authority
`b8bd7e71beedf7b8798b976df8ffc7275589fa8b` under Foreman State revision
0078.

The exact active packet was:

`A002-AUDIO-RUNTIME-PROFILE-AUTHORITY-R36`.

R36 authorized only Configuration's selected immutable production AUDIO runtime
profile, grounded in forensic H1 commit
`3426f28b93de9519ca93e5f0e0aaf8b67cfca845` and the qualified
`P11_COMPAT_PLUS_PCM` lineage.

PS2 AUDIO execution binding, Application AUDIO composition, Transport AUDIO
activation, AUDSRV product start, automatic MPEG recalibration, persistence/
editor/reload and hardware qualification remained deferred.

No user terminal, Pi-local proxy, hardware action, Validation work or Foreman
state mutation was performed.

## Final pre-log source authority

Final pre-log source/test/dictionary head:

`a5528b08fa48382545ee9e325c5f103b4e38c0b9`
— `test(config): enforce R36 audio profile boundary`.

The returned range is exactly four commits ahead of the assigning Foreman/log
head and zero behind:

1. `d7b7c5ebe489ddf707a899c72854bb00b11be8eb`
   — `config(audio): add selected R36 runtime profile`
2. `a865a39f31dc1d5a918318eee2614236d26a55f0`
   — exact empty deterministic dictionary-reconciliation trigger
3. `9f3cefde8f7f15b9004102511f29d2f5e673f7df`
   — automation-generated clean dictionary reconciliation
4. `a5528b08fa48382545ee9e325c5f103b4e38c0b9`
   — `test(config): enforce R36 audio profile boundary`

Exact changed paths are confined to the authorized Configuration/profile,
focused-test, build/check, topology and generated dictionary surface:

- `src/config/audio_runtime_profile.json`;
- `src/config/audio_runtime_profile_generated.h`;
- `src/config/audio_runtime_profile.h`;
- `src/config/audio_runtime_profile.c`;
- `scripts/generate-audio-runtime-profile.py`;
- `tests/unit/config_audio_runtime_profile_test.c`;
- `tests/unit/config_audio_runtime_profile_generation_test.py`;
- `tests/unit/config_audio_runtime_profile_source_test.py`;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `scripts/check.sh`;
- `docs/development/source-topology.md`;
- `src/config/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No `src/audio/session.*`, playback, AUDSRV mechanism, Transport implementation,
media-clock mechanism, Application, Input/UI/RFB/Display/MPEG, Pi, Wire/protocol
or forensic source changed.

## R36 implementation

R36 adds one canonical selected profile at
`src/config/audio_runtime_profile.json`. Its runtime projection contains only
three visibly distinct existing owner values:

- `pstvnc_transport_audio_channel_config_t transport`;
- `pstvnc_config_pcm_profile_t pcm`;
- `pstvnc_audio_session_values_t session`.

The exact selected values are:

Transport AUDIO channel:

- queue capacity 524288 bytes;
- initial credit 524288 bytes;
- credit batch 4096 bytes;
- flush-on-empty enabled;
- credit return enabled.

PCM:

- 48000 Hz;
- 2 channels;
- 16 bits/sample;
- volume 100%.

AUDIO session:

- playback buffer capacity 4096 bytes;
- startup reservoir 458752 bytes;
- worker priority 65;
- worker stack 16384 bytes;
- reservoir poll 1000 us;
- common-clock poll 1000 us.

The reservoir and clock polling values are independent struct fields and
independent generated constants even though the current selected value is 1000
for each.

R26 remains sole common-media-clock offset authority. R36 contains no
`audio_presentation_offset_us` value or generated offset constant.

The selected runtime object is private static-const Configuration authority.
`pstvnc_config_audio_runtime_profile_selected()` copies that object into
caller-owned storage only after validation, so mutating a returned copy cannot
alter Configuration-owned authority.

`pstvnc_config_audio_runtime_profile_valid()` rejects malformed owner
projections including:

- zero queue;
- initial credit or credit batch exceeding queue;
- invalid flow-control booleans or disabled credit return under this selected
  profile contract;
- invalid PCM rate/channel/sample-width/volume;
- zero or impossible playback/session buffer relationships;
- invalid worker priority/stack bounds;
- zero reservoir or clock poll cadence.

The deterministic generator independently validates the canonical JSON schema,
uint32 bounds, queue/credit relationships, PCM shape, session buffering,
worker priority/stack bounds, separate nonzero poll values and exact R36
provenance. It renders only the checked-in C constants and is included in the
canonical project check. Runtime JSON parsing was not added.

The canonical profile JSON retains qualification provenance as development
evidence, but the C runtime profile type/storage does not expose H1 profile ID,
forensic commit identity or any diagnostic/laboratory field.

## Deterministic evidence

Focused C tests prove:

- exact equality of every selected Transport/PCM/session value;
- exact owner grouping;
- 4096-byte playback capacity;
- 458752-byte startup reservoir;
- priority 65 and stack 16384;
- independently addressable reservoir/clock poll fields, each 1000 us;
- invalid zero/range/structural values reject;
- caller mutation of one selected copy does not alter the next selected copy.

Generator tests prove malformed queue/credit, PCM, session, overflow/bound,
poll and provenance cases fail closed. They also prove the checked-in generated
header matches the canonical JSON and contains no media-clock offset.

The source-boundary test proves profile selection contains no allocation,
thread/semaphore creation, AUDSRV call, Transport session open/access,
media-clock arm/wait, Application effect or PS2 platform effect. It also proves
the hand-written aggregate uses the existing narrow owner types and does not
duplicate R26 audio-offset authority.

Existing A002 AUDIO playback/session/media-clock regressions and existing
RFB/MPEG selected-profile generation checks remain green.

## Canonical machine evidence

Exact final-source GitHub Actions run:

`36266535951`

checked out exact head
`a5528b08fa48382545ee9e325c5f103b4e38c0b9` on
`ledge/h1-all-guns` and completed SUCCESS.

Successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED on the final non-trigger commit.

Observed focused/regression output includes:

- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`;
- `AUDIO_RUNTIME_PROFILE_GENERATED=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`;
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`;
- `audio_playback_test: PASS`;
- `audio_session_test: PASS`;
- `media_clock_test: PASS`;
- `RFB_RUNTIME_PROFILE_GENERATED=PASS`;
- `MPEG_RUNTIME_PROFILE_GENERATED=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `WORK_LOG_CHECK=PASS records=240 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Initial R36 CI already showed host-unit, pinned PS2 compile and linked build
green before dictionary reconciliation. Its only project failures were the
expected new-file dictionary/topology drift. The exact deterministic dictionary
job then changed only `src/config/SYMBOLS.md` and
`docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

## Exact linked identity and qualification boundary

Final R36 linked identity:

`ELF_PRISTINE_SHA256=8dc8b419cc80d655bdaaccb9b9f5534f6578f5d57c4623b396ab3c35d92ff8db`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=5c59e5a914d38f5dbe7c0ad9d72df602f91753a1a002b2bfa112f48e4978d423`

`PT_LOAD_BYTES=528020`

Newest Foreman-accepted R35 identity was:

`PT_LOAD_SHA256=908f31b526fc51d2d819b9ac092e218b048c475576568ca26cfe13a3fc84bb66`

`PT_LOAD_BYTES=527380`.

Therefore:

- `R36_SOURCE_COMPLETE=YES_WITHIN_PACKET`
- `R36_HOST_TESTED=PASS`
- `R36_PROJECT_CHECK=PASS`
- `R36_STRICT_DICTIONARIES=PASS`
- `R36_PS2_COMPILE=PASS`
- `R36_PS2_LINK=PASS`
- `R36_CURRENT_SOURCE_REPRODUCIBILITY=PASS`
- `R36_MACHINE_EVIDENCE=GITHUB_ACTIONS`
- `R36_PS2_PT_LOAD_CHANGED=YES`
- `R36_INDEPENDENT_VALIDATION=NOT_RUN`
- `R36_OPERATOR_OBSERVED=NO`
- `R36_HARDWARE_QUALIFIED=NO`
- `R36_HARDWARE_PENDING=YES`.

No physical qualification transfers from R35 or any earlier image.

## Reconstruction Worker requirement disposition

These are Reconstruction Worker dispositions only. They are not Foreman
acceptance.

1. Configuration-owned aggregate uses existing narrow owner types — MET.
2. Exactly one selected immutable current profile is published — MET.
3. Exact selected qualified values only; no H1 laboratory surface — MET.
4. Transport/session/PCM owner separation is preserved — MET.
5. Reservoir and clock poll fields remain distinct — MET.
6. Deterministic validation fails closed on malformed values — MET.
7. Deterministic checked-in generation is enrolled in canonical validation —
   MET.
8. Profile access performs no runtime side effects — MET.
9. Existing AUDIO/Transport/media-clock/Application/Pi mechanisms are untouched
   — MET.
10. Generic `pstvnc_config_session_profile_t` parser behavior is unchanged —
    MET.
11. Selected profile is enrolled in topology/dictionaries and pinned PS2
    compile/link without Application consumption — MET.
12. Focused plus canonical host/project/dictionary/PS2/reproducibility evidence
    closes the R36 fence without hardware-success claim — MET.

## Scope and no-claim boundary

R36 does not initialize AUDSRV, create AUDIO worker/runtime resources, open
Transport with AUDIO, start/stop PCM playback, arm/wait the media clock,
integrate AUDIO into Application startup/steady-state/shutdown, mutate Pi AUDIO
producer behavior, implement automatic MPEG recalibration, add user-editable
audio tuning, add timeout/watchdog success, or claim independent Validation,
operator observation or physical qualification.

## Next pickup

The Interactive Reconstruction Worker stops here.

The Foreman must independently inspect the exact four-commit returned range,
selected values/provenance, owner-type aggregation, deterministic generator,
focused tests, canonical run `36266535951`, changed linked identity and this
immutable closeout before accepting or rejecting R36 and choosing any next
packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R36_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION
