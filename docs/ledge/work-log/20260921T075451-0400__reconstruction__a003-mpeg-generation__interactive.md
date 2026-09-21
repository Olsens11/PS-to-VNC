# Reconstruction shift — A003 MPEG runtime-profile authority R7

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T07:54:51-04:00
COMPLETED_AT=2026-09-21T08:02:06-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=d9f8e803048574bb5b02296b92f7571a37693675
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7`

R7 reconstructs one Configuration-owned selected MPEG runtime profile containing
the already-accepted narrow owner values required by:

- Transport's logical MPEG channel;
- the synchronous MPEG decoder;
- the asynchronous MPEG worker;
- the PS2 worker execution binding;
- the Display MPEG scheduler.

R7 is value authority only.

It does not widen the production CONFIG wire, mint generation identity, select
active presentation geometry, start a Transport session, start decoder/worker
execution, initialize Presentation/scheduler runtime, send START/RETIRE, create
Pi service ownership, or wire any Application MPEG lifecycle.

## Live authority consumed

Worker pickup refreshed live branch authority to:

`d9f8e803048574bb5b02296b92f7571a37693675`

Current Foreman State was revision 0035 and assigned:

- PACKET_ID=A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7
- PACKET_STATUS=ACTIVE
- ROLE_KEY=reconstruction
- WORK_ITEM_KEY=a003-mpeg-generation
- WORKER_KEY=interactive

Required current authority was read, including:

- AGENTS.md;
- CONTRIBUTING.md;
- docs/PROJECT_INTENT.md;
- docs/CLEAN_ARCHITECTURE.md;
- current development source-topology/naming/lifecycle guidance;
- Reconstruction Contract rev 0006;
- work-log contract rev 0007;
- Wire Runtime Decisions rev 0011;
- Architecture Overlay rev 0004;
- A002 CONFIG/audio/clock audit;
- A003 MPEG/generation audit;
- A004 Presentation/calibration audit;
- newest Foreman immutable log;
- current public owner types in Transport, MPEG decoder/worker/PS2 binding and
  Display scheduler;
- current production CONFIG profile source/test authority;
- frozen H1 value evidence named by the Foreman.

No user terminal proxy or physical hardware action was required.

## Historical value provenance reviewed

Frozen H1 evidence supports the Foreman-selected initial clean values.

`h1_profiles.py` records:

- MPEG queue capacity 524288;
- MPEG initial credit 524288;
- MPEG credit batch 8192;
- MPEG flush-on-empty enabled;
- MPEG credit return enabled;
- MPEG feed 2048;
- video cadence 30000/1001;
- RGB16;
- maximum decoder dimensions 704x480;
- drop disabled and threshold zero in the current selected profile.

`h1_video_runtime.c` additionally shows:

- 704x480 resource bounds;
- two bytes per pixel for RGB16;
- 2048-byte feed;
- 64-byte aligned retained feed/picture backing;
- 16-byte DMA/feed transfer rounding.

`h1_cp2p_mpeg_worker.c` records:

- 64-KiB worker stack;
- worker priority 67;
- 1000-us stop/dormancy poll interval;
- 3000 maximum observations.

`CP2P_MPEG_EVENT_WAKE_HARDWARE_RESULT.md` records the 600-second all-guns
confirmation using 524288-byte MPEG queue/initial credit and 8192-byte batch.

These facts are provenance for the initial clean selected profile only. They do
not hardware-qualify the reconstructed R7 source.

## New Configuration-owned profile seam

Added:

- `src/config/mpeg_runtime_profile.h`;
- `src/config/mpeg_runtime_profile.c`.

The public aggregate is:

`pstvnc_config_mpeg_runtime_profile_t`

It does not reproduce mechanism-owner fields into Config-specific lookalike
structures.

Its members are the exact existing public owner types:

- `pstvnc_transport_mpeg_channel_config_t transport`;
- `pstvnc_mpeg_decoder_config_t decoder`;
- `pstvnc_mpeg_worker_values_t worker`;
- `pstvnc_mpeg_ps2_worker_runtime_values_t ps2_worker_runtime`;
- `pstvnc_mpeg_scheduler_profile_t scheduler`.

The sole public selector is:

`pstvnc_config_mpeg_runtime_profile_selected()`

It returns a pointer to one static const selected profile.

Configuration owns the selected values. Mechanism owners continue to own each
type's validation and execution semantics.

## Exact selected Transport MPEG values

The selected `pstvnc_transport_mpeg_channel_config_t` is:

- queue_capacity = 524288;
- initial_credit_bytes = 524288;
- credit_batch_bytes = 8192;
- credit_flush_on_empty = 1;
- credit_return_enabled = 1.

No Transport runtime default or hidden substitution was added.

## Exact selected decoder values

The selected `pstvnc_mpeg_decoder_config_t` is:

- max_width = 704;
- max_height = 480;
- bytes_per_pixel = 2;
- feed_payload_capacity = 2048;
- transfer_alignment = 16;
- buffer_alignment = 64.

The 704x480 values are decoder resource bounds only.

R7 creates no active MPEG base/inner/outer rectangle and changes no calibration,
START geometry or Presentation geometry authority.

## Exact selected worker values

The selected `pstvnc_mpeg_worker_values_t` is:

- worker_stack_bytes = 65536;
- worker_priority = 67.

R7 does not start a worker.

## Exact selected PS2 worker-runtime policy

The selected `pstvnc_mpeg_ps2_worker_runtime_values_t` is:

- join_poll_delay_us = 1000;
- join_poll_max_count = 3000.

These values bound fail-closed kernel dormancy observation.

They do not authorize:

- treating timeout as successful retirement;
- force termination;
- deleting a possibly-live EE thread;
- bypassing the already-accepted R5 dormancy proof.

## Exact selected Display scheduler values

The selected `pstvnc_mpeg_scheduler_profile_t` is:

- fps_numerator = 30000;
- fps_denominator = 1001;
- drop_enabled = 0;
- drop_threshold_milliframes = 0.

R7 preserves the existing absolute common-clock scheduler.

No scheduler-mode selector, comparison mode or arbitrary runtime tuning surface
was restored.

## Production CONFIG wire unchanged

R7 did not modify:

- `src/config/profile.h`;
- `src/config/profile.c`;
- `tests/unit/config_profile_test.c`.

The production wire authority therefore remains:

- PSTVNC_CONFIG_PROFILE_VERSION = 1;
- PSTVNC_CONFIG_PROFILE_HEADER_BYTES = 8;
- PSTVNC_CONFIG_PROFILE_ENTRY_BYTES = 8;
- PSTVNC_CONFIG_PROFILE_FIELD_COUNT = 19;
- highest current field ID =
  PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD = 19.

Focused R7 host coverage explicitly asserts those facts.

No MPEG queue/feed/thread/scheduler parameter was added to the CONFIG wire.

## H1 laboratory knobs intentionally not productized

R7 does not expose or preserve as product profile fields:

- MPEG_EMPTY_DELAY_US;
- MPEG prefill/start-target wait loops;
- VIDEO_IPU_RESET_EACH_SESSION;
- stage markers or stage holds;
- H1 profile IDs;
- queue allocation-order experimentation;
- scheduler-mode selection;
- socket-buffer sweep knobs;
- arbitrary drop tuning;
- draw rectangle;
- encode rectangle;
- capture rectangle;
- active suppression rectangle.

Video presentation offset remains in the pre-existing media-clock/session
profile authority and is not duplicated by R7.

## Immutability / copy safety

The selected profile object has static const storage.

The API returns only:

`const pstvnc_config_mpeg_runtime_profile_t *`

Focused host testing copies the selected aggregate, mutates every owner
subprofile in the local copy, and proves:

- the selected Config-owned object is unchanged;
- a second selector call returns the same selected immutable object.

Later Application wiring can therefore copy each narrow owner value explicitly
without gaining mutable Configuration-owned storage.

## No live activation

Static final-source review confirms the new selected-profile C module contains
no call to:

- `pstvnc_transport_session_open*`;
- `pstvnc_mpeg_decoder_initialize()`;
- `pstvnc_mpeg_worker_start()`;
- `pstvnc_mpeg_ps2_worker_runtime_init()`;
- `pstvnc_mpeg_scheduler_init_after_first_sync()`;
- `pstvnc_transport_mpeg_send_start()`;
- `pstvnc_transport_mpeg_send_retire()`;
- Presentation activation;
- Application runtime orchestration.

Compared with the Foreman base, no Application, Transport runtime, MPEG
mechanism, Display mechanism, media-clock, Pi runtime or RFB product source was
changed.

The selected profile is linked into the current product ELF but has no live
caller yet.

## Focused test

Added:

`tests/unit/config_mpeg_runtime_profile_test.c`

It proves:

- exact Transport values;
- exact decoder values;
- exact worker stack/priority;
- exact PS2 dormancy observation values;
- exact Display scheduler values;
- compile-time reuse of the owner types through typed assignments;
- selected object const/copy-safe behavior;
- unchanged production CONFIG version/header/entry/field-count/highest-ID
  authority.

Final host log reports:

`config_mpeg_runtime_profile_test: PASS`

The existing production CONFIG decoder regression also remains:

`config_profile_test: PASS`

## Build integration

Updated `tests/Makefile` with the focused host target:

`config-mpeg-runtime-profile-unit`

Updated `mk/issue7-clean.mk` with:

`build/reconstruction/issue7/config_mpeg_runtime_profile.o`

Updated `scripts/check-clean-ps2-compile.sh` so strict direct R5900 compile
includes:

`src/config/mpeg_runtime_profile.c`

The executable mode of `scripts/check-clean-ps2-compile.sh` is preserved.

## Intermediate failures and corrections

The first build-integration commit accidentally changed
`scripts/check-clean-ps2-compile.sh` from executable to non-executable because
the repository tree edit supplied mode 100644.

That same run found the focused copy-safety fixture had a local `copy` value
that was mutated but not read, causing `-Werror=unused-but-set-variable`.

Both packet-owned defects were corrected in:

`148755cc8c05113db74e4912f168570e7ee925db`

The repair:

- restored script mode 100755;
- added explicit assertions over the mutated local copy.

The next host workflow reached the pre-existing timing-sensitive
`transport_runtime_test` cascade beginning at its established RFB activity
assertions. R7 changed no Transport source. No Transport repair was made.

On the unchanged source tree, the canonical reconciliation-trigger run later
reported:

`transport_runtime_test: PASS`

and also:

`config_mpeg_runtime_profile_test: PASS`

The R7 behavior/source defect was therefore the focused test warning, not a
Transport regression.

## Dictionary and topology integration

Before reconciliation, strict dictionary/project gates truthfully reported:

- missing file coverage for
  `src/config/mpeg_runtime_profile.c`;
- missing file coverage for
  `src/config/mpeg_runtime_profile.h`;
- eleven missing Config definitions.

The canonical deterministic reconciliation was triggered with a tree-identical
commit and produced:

`7a15d8a34b15762ff5fef415f54362198b52367b`

It changed only:

- `src/config/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

Config dictionary count advanced:

`133 -> 144`

Maintained Config responsibility prose was then updated to state that Config
owns the one selected internal MPEG runtime profile assembled directly from
existing owner value types while excluding all runtime/lifecycle behavior.

Final topology and dictionary checks pass.

## Commit chronology

R7 range after Foreman base
`d9f8e803048574bb5b02296b92f7571a37693675`:

1. `a1bee54645ae59bf431ea7f4fb73bafe8b8e95ef`
   — `feat(config): select MPEG runtime profile`;
2. `16eb1c1119297c507f2dcf6c7db481fde1f8fe45`
   — `build(config): integrate MPEG runtime profile`;
3. `148755cc8c05113db74e4912f168570e7ee925db`
   — `fix(config): preserve profile test and tool contracts`;
4. `a0e22368ed0481284c3899383ea20131b3ca57bf`
   — tree-identical deterministic dictionary-reconciliation trigger;
5. `7a15d8a34b15762ff5fef415f54362198b52367b`
   — automated current-clean dictionary reconciliation;
6. `5cdd5670a1924bde8e7348bd863c07b0cd28bc4c`
   — `docs(config): describe selected MPEG runtime profile`.

## Exact changed paths

Compared with Foreman base, final pre-log authority changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `src/config/SYMBOLS.md`;
- `src/config/mpeg_runtime_profile.c`;
- `src/config/mpeg_runtime_profile.h`;
- `tests/Makefile`;
- `tests/unit/config_mpeg_runtime_profile_test.c`.

Notably unchanged:

- `src/config/profile.h`;
- `src/config/profile.c`;
- all Application source;
- all Transport product source;
- all MPEG mechanism source;
- all Display mechanism source;
- all Pi product source.

## Final coherent machine evidence

Final pre-log authority:

`5cdd5670a1924bde8e7348bd863c07b0cd28bc4c`

Canonical GitHub Actions workflow:

`35597128137`

completed SUCCESS on attempt 1.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Final host evidence explicitly reports:

- `transport_runtime_test: PASS`;
- `config_profile_test: PASS`;
- `config_mpeg_runtime_profile_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`.

Final project/dictionary evidence explicitly reports:

- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

Final direct PS2 compile explicitly reports:

- `PS2_COMPILE=src/config/mpeg_runtime_profile.c`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Final linked evidence compiles
`src/config/mpeg_runtime_profile.c` into
`config_mpeg_runtime_profile.o`, links that object into both current-source
reproducibility builds, and reports:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

## A003 R7 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R7-C1 CONFIG_OWNS_SELECTED_MPEG_PROFILE — MET

One focused Config module owns one immutable selected MPEG runtime profile.

### A003-R7-C2 EXACT_TRANSPORT_VALUES — MET

The selected existing Transport MPEG channel type carries exactly
524288/524288/8192/1/1.

### A003-R7-C3 EXACT_DECODER_VALUES — MET

The selected existing decoder type carries exactly 704x480, 2 bytes per pixel,
2048-byte feed, 16-byte transfer alignment and 64-byte buffer alignment.

### A003-R7-C4 EXACT_WORKER_EXECUTION_VALUES — MET

The existing worker and PS2 worker-runtime types carry exactly
65536/67 and 1000/3000 respectively.

### A003-R7-C5 EXACT_SCHEDULER_VALUES — MET

The existing Display scheduler profile carries exactly 30000/1001 with drop
disabled and zero threshold.

### A003-R7-C6 EXISTING_OWNER_TYPES_REUSED — MET

The Config aggregate fields are the existing public owner types directly.
Focused typed assignments compile and pass.

### A003-R7-C7 CONFIG_WIRE_NOT_BROADENED — MET

Production CONFIG profile source is unchanged. Version remains 1 and field count
remains 19 with highest field ID 19. Focused test asserts the schema constants.

### A003-R7-C8 LAB_KNOBS_NOT_PRODUCTIZED — MET

No H1 profile ID, empty-delay/prefill/start-target timer, IPU reset selector,
stage marker/hold, allocation-order, scheduler-mode, socket-buffer, geometry or
arbitrary drop-tuning field entered the clean selected profile.

### A003-R7-C9 GEOMETRY_OWNERSHIP_PRESERVED — MET

The selected Config profile contains only decoder max resource bounds and no
active/calibrated/capture/draw/suppression rectangle.

### A003-R7-C10 NO_LIVE_ACTIVATION — MET

No runtime activation/open/start/send/scheduler-init/Application call was added.
No product caller consumes the selected profile yet.

### A003-R7-C11 FOCUSED_TESTS_AND_STRICT_DICTIONARIES — MET

The focused host test passes, strict dictionaries pass, and new source has
complete local-file coverage.

### A003-R7-C12 CLEAN_BUILD_EVIDENCE — MET

Direct R5900 compile and current-source linked reproducibility both pass on exact
final pre-log authority.

## Evidence boundary / non-claims

PENDING_LOCAL=NONE_FOR_A003_R7_REQUIRED_REPOSITORY_MACHINE_GATES
A003_R7_SELECTED_PROFILE_IMPLEMENTED=YES
A003_R7_CONFIG_WIRE_EXPANDED=NO
A003_R7_LIVE_ACTIVATION=NO
A003_R7_PS2_COMPILED_AND_LINKED=YES
PI_PRODUCT_WIRE_CONTROL_OWNER=NOT_IMPLEMENTED
PI_PRODUCT_MPEG_PRODUCER_OWNER=NOT_IMPLEMENTED
APPLICATION_MPEG_GENERATION_ALLOCATION=NOT_IMPLEMENTED
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PRESENTATION_ARM_FROM_CALIBRATION=NOT_IMPLEMENTED
R5_R4_LIVE_WORKER_START=NOT_IMPLEMENTED
P7_LIVE_APP_LOOP=NOT_IMPLEMENTED
PRODUCER_DONE_RETIREMENT_PUBLICATION=NOT_IMPLEMENTED
MPEG_RETIRE_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
MPEG_RETIRE_CREDIT_FINALIZATION=NOT_IMPLEMENTED
Q7_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
WIRE_LOSS_MPEG_RECOVERY=NOT_IMPLEMENTED
PHYSICAL_PS2_R7_PROFILE_PATH=NOT_RUN_NOT_CLAIMED
HARDWARE_QUALIFICATION_R7=PENDING
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED

R7 required no physical qualification because it adds value authority only.
Historical hardware evidence is provenance, not qualification transfer.

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`5cdd5670a1924bde8e7348bd863c07b0cd28bc4c`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- one Configuration-owned selected profile;
- exact Transport/decoder/worker/PS2-runtime/scheduler values;
- direct reuse of existing owner types;
- unchanged 19-field production CONFIG wire;
- absence of H1 laboratory knobs;
- preservation of geometry/media-clock ownership;
- no live activation call graph;
- focused test and copy-safety proof;
- strict dictionary/topology result;
- direct PS2 compile and linked reproducibility evidence;
- machine-vs-hardware claim boundary.

The Foreman, not this worker, chooses the next Pi product Wire/control-owner or
other dependency packet.

This Reconstruction worker stops after R7 and does not begin Pi or Application
work.
