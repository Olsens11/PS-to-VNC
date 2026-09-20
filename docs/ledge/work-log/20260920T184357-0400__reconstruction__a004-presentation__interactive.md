# Reconstruction shift — A004 shared compositor / first synchronized presentation R4

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T18:43:57-04:00
COMPLETED_AT=2026-09-20T18:54:36-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=2f234d140213ed79d37c6bc2c228f12dfae90fdf
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Role correction / authority note

This conversation's permanent role is Reconstruction worker.

Immediately before this shift, the same conversation had incorrectly interpreted
the relay baton as transferring roles and created the following docs-only NOOP
Foreman log:

`docs/ledge/work-log/20260920T183725-0400__foreman__a004-presentation__interactive.md`

at commit:

`2f234d140213ed79d37c6bc2c228f12dfae90fdf`

The user corrected the relay rule before this Reconstruction work began:
the baton controls turn-taking only and never changes this conversation's role.

That mistaken log changed no Foreman state, no active packet, no product source,
and no planning authority. Current Foreman State remained revision 0026 at
Foreman commit:

`bc52143d346041ffb6113952220c218417d83d8d`

with active packet:

`A004-SHARED-COMPOSITOR-FIRST-SYNC-R4`

The immutable-work-log contract prohibits deleting, rewriting, replacing or
renaming a committed shift record. Therefore the erroneous NOOP log is preserved
as historical role-error evidence rather than illegally rewritten. It is not a
Foreman state snapshot and did not supersede revision 0026.

This shift performed Reconstruction-worker work only.

## Objective

Execute only Foreman packet
`A004-SHARED-COMPOSITOR-FIRST-SYNC-R4`.

The bounded goal was to reconstruct the clean single-owner physical MPEG
compositor and the first synchronized MPEG presentation seam:

- retain one PS2 GS/dmaKit owner;
- define one decoder-independent RGB16 16x16-macroblock surface value;
- fence physical presentation against the exact P3 run/generation and resolved
  geometry snapshot;
- preserve the qualified physical layer order:
  remote desktop -> outer suppression -> MPEG -> inner matte -> local overlay;
- use bounded reusable video resources;
- report physical completion only after the existing synchronized GS flip;
- capture the exact Platform timer observation after that boundary;
- arm the one session media epoch only after the first real synchronized MPEG
  presentation if that epoch is still unarmed;
- then perform exact P3 first-frame promotion;
- preserve the same session epoch for later MPEG runs;
- keep subsequent exact-run frames stable without re-arm or re-promotion;
- add direct canonical PS2 compile/link coverage for P3/P4 source.

This shift did not begin scheduler/drop, active-Q7 retirement, decoded-frame
worker/queue integration, final Application orchestration, P2 runtime wiring,
calibration runtime wiring, Transport START/RETIRE, Pi producer activation, or
hardware qualification.

## Execution classification

EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO

No user terminal or physical PS2 action was requested. Source/history work,
writes, dictionary reconciliation, direct PS2 compile/link evidence and CI
inspection were completed through GitHub/GitHub Actions.

## Authority consumed

Consumed:

- Foreman State revision 0026;
- Reconstruction Contract revision 0006;
- work-log contract revision 0006;
- Wire runtime decisions revision 0011, especially Q6/Q7;
- architecture overlay revision 0004;
- A002 common-clock audit revision 0001;
- A003 MPEG-generation audit revision 0001;
- A004 presentation/calibration audit revision 0001;
- clean architecture and source-topology policy.

The assigned Foreman packet was already repository-authoritative before the
mistaken NOOP log and remained unchanged by it.

## Current clean source inspected

Inspected current:

- `src/display/display.{c,h}`;
- `src/display/mpeg_presentation.{c,h}`;
- `src/platform/ps2_graphics.{c,h}`;
- `src/platform/ps2_system.{c,h}`;
- `src/media/clock.{c,h}`;
- `src/mpeg/decoder.{c,h}`;
- `src/ui/local_ui_presentation.{c,h}`;
- current local OSK/render presentation interfaces;
- `tests/Makefile`;
- `scripts/check-clean-ps2-compile.sh`;
- `mk/issue7-clean.mk`;
- current Ledge reconstruction workflow.

Recovered current facts:

- Platform `ps2_graphics` is already the single GS/dmaKit owner.
- Existing desktop presentation queues one frame and returns after
  `gsKit_sync_flip()`.
- Local UI/OSK is represented as a generic caller-owned overlay and is already
  intended to remain above remote presentation.
- P3 owns exact-run visible state and immutable
  base/inner-content/suppression geometry but deliberately performs no physical
  draw or media-clock arm.
- The session media clock accepts an explicit observed tick and is one-shot:
  `pstvnc_media_clock_arm()` leaves an already-armed epoch unchanged.
- MPEG decoder owns decode buffers/calls but explicitly does not own
  presentation/clock policy.
- Before P4, strict PS2 compile/link manifests did not directly include the
  new P3 translation unit.

## Frozen H1 evidence inspected

Frozen authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Inspected:

- `experiments/media-harness-h1/h1_cumulative39_graphics.{c,h}`;
- `h1_video_runtime.{c,h}` around picture layout, video presentation and first
  synchronization;
- `h1_media_clock.{c,h}`;
- `MEDIA_OBJECT_COMPOSITION_NOTES.md`;
- P3 ownership/start evidence only as necessary to bind run semantics.

Recovered qualified mechanism facts:

- decoded RGB16 picture memory is 16x16 macroblock ordered;
- H1 detiled macroblocks into one reusable linear video texture;
- one graphics owner rendered:
  desktop -> black outer suppression -> MPEG base -> black symmetric inner matte
  -> local overlay;
- remote desktop updates could continue underneath suppression while MPEG stayed
  visible;
- video success was returned only after synchronized physical presentation;
- video VRAM/resources were bounded and reused;
- first real video presentation owned common-media-clock arming.

Historical shapes deliberately not restored:

- no textual inclusion of another graphics C file;
- no second GS presenter;
- no semaphore created solely because multiple historical callers competed for
  GS;
- no decoder worker direct GS ownership;
- no direct active `clear_video()` shortcut.

## Clean P4 source shape

### Neutral decoded frame

Added:

- `src/display/mpeg_frame.h`
- `src/display/mpeg_frame.c`

The neutral value
`pstvnc_mpeg_rgb16_macroblock_surface_t` carries only:

- caller-owned RGB16 pixel pointer;
- pixel capacity;
- source width;
- source height.

It contains no decoder-private handle, run generation, presentation state,
Transport state, GS resource or clock state.

Validation requires positive 16-pixel-aligned dimensions bounded by the
qualified 704x480 H1 surface and enough caller capacity.

`pstvnc_mpeg_rgb16_detile()` deterministically converts 16x16 macroblock order
to row-major RGB16 into caller-owned bounded output storage.

### Sole physical GS owner

Evolved only the existing:

- `src/platform/ps2_graphics.h`
- `src/platform/ps2_graphics.c`

Platform remains the only file/domain that includes and invokes gsKit/dmaKit.

New Platform mechanism facts:

- one maximum-size reusable MPEG VRAM allocation;
- one bounded aligned EE-side linear detile buffer;
- no per-frame/per-run permanent allocation;
- cached desktop texture;
- cached optional local-overlay texture/placement;
- retained current MPEG surface plus neutral base/inner/suppression rectangles;
- generic synchronized-completion result containing:
  - synchronized flag;
  - post-sync `GetTimerSystemTime()` observation;
  - `kBUSCLK` tick-domain identity.

`pstvnc_ps2_graphics_present_video_macroblocks()` carries no MPEG run
generation or lifecycle meaning.

Ordinary `pstvnc_ps2_graphics_present()` now updates desktop/local-overlay
resources and, if a video surface is retained, redraws the same composite rather
than revealing RFB beneath the active suppression footprint. Thus future RFB
truth updates can advance physically without destroying MPEG-local visibility.

The single render path structurally orders:

1. desktop texture;
2. black suppression footprint;
3. MPEG texture;
4. black inner matte;
5. local overlay;
6. one `gsKit_queue_exec()`;
7. one `gsKit_sync_flip()`;
8. timer observation after sync.

There is no Platform active-clear API in P4.

### Display-owned synchronized coordinator

Added:

- `src/display/mpeg_compositor.h`
- `src/display/mpeg_compositor.c`

The coordinator imports no decoder, Transport, RFB or calibration owner and
contains no gsKit/dmaKit call.

For each frame it:

1. validates the neutral macroblock surface;
2. obtains the exact current P3 snapshot;
3. requires exact nonzero run-generation equality;
4. requires source width/height to equal the P3 base width/height;
5. requires base/inner/suppression to remain inside the fixed logical display;
6. translates the neutral resolved geometry into Platform rectangles;
7. invokes the sole Platform video presenter;
8. requires a successful synchronized result in the same tick domain as the
   session media clock;
9. observes whether the session clock is already armed;
10. in WAIT_FIRST_FRAME only:
    - arms the clock from the returned post-sync tick only if still unarmed;
    - then invokes exact P3 first-frame promotion;
11. in MPEG_OWNED:
    - requires the session epoch already to exist;
    - does not re-arm it;
    - does not duplicate P3 promotion.

The result/effects structure preserves a truthful physical synchronized fact
even if a later clock or promotion step fails, while the function returns a
failure and never reports a successful first-presentation transaction.

## Reconstruction-owned corrections during P4

### Executable-mode repair

The first low-level Git write that modified
`scripts/check-clean-ps2-compile.sh` accidentally changed its mode from
executable to `100644`.

CI run `35542801038` therefore failed the strict compile job with:

`Permission denied`

before compilation began.

No C source defect was implicated. Commit:

`bbf7e52f45ec577ccd9eeea688cd4023d87d075f`

restored only the executable mode.

Later direct PS2 compile passed.

### Full inner-matte geometry repair

Final review against accepted P1/P3 semantics exposed a P4 validator bug:
P1 permits an inner matte equal to half the base dimension, so the resolved
`inner_content` may legitimately have zero width and/or height.

The first P4 validator required positive inner-content size and would have
rejected that accepted upstream value.

Commit:

`37bbd71ca5afaf3dfb63568e85f9d31dbf043d08`

changed only P4 validation and its host regression fixture so:

- base and suppression remain positive rectangles;
- inner content may be empty but not negative or out of display bounds;
- Platform and Display coordinator agree on that contract.

The host test now proves a fully matted 32x16 base can still cross the
synchronized first-frame seam.

## Build-manifest integration

Updated:

- `scripts/check-clean-ps2-compile.sh`;
- `mk/issue7-clean.mk`.

Direct strict PS2 compile now includes:

- `src/display/mpeg_frame.c`;
- `src/display/mpeg_presentation.c`;
- `src/display/mpeg_compositor.c`;
- evolved `src/platform/ps2_graphics.c`.

The current linked build now contains objects:

- `mpeg_frame.o`;
- `mpeg_presentation.o`;
- `mpeg_compositor.o`;
- `ps2_graphics.o`.

This closes the P3 evidence gap where presentation ownership had only host
execution evidence and was not directly compiled/linked into the current ELF.

## Exact commits

Worker-owned P4 range after the mistaken historical NOOP record:

1. `75d5c87cd47a3d8b7108d790eed890de4bbaf7b9`
   — `feat(display): add synchronized MPEG compositor`
2. `bbf7e52f45ec577ccd9eeea688cd4023d87d075f`
   — `fix(tooling): restore PS2 compile check executable mode`
3. `25081e22864db4f09c67c10dd7c4662cfa975f50`
   — tree-identical deterministic dictionary-reconciliation trigger
4. `8f147edc97219df4b59c5118566a377f0a0bd0ab`
   — automation-created `docs(symbols): reconcile current clean definitions`
5. `f0e1da106035063c63555f93d2b480c36306128c`
   — `docs(symbols): describe synchronized MPEG compositor ownership`
6. `37bbd71ca5afaf3dfb63568e85f9d31dbf043d08`
   — `fix(display): preserve full inner matte geometry`

## Exact changed paths for P4

Compared with this Reconstruction shift's starting head
`2f234d140213ed79d37c6bc2c228f12dfae90fdf`, P4 changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`
- `mk/issue7-clean.mk`
- `scripts/check-clean-ps2-compile.sh`
- `src/display/SYMBOLS.md`
- `src/display/mpeg_compositor.c`
- `src/display/mpeg_compositor.h`
- `src/display/mpeg_frame.c`
- `src/display/mpeg_frame.h`
- `src/platform/SYMBOLS.md`
- `src/platform/ps2_graphics.c`
- `src/platform/ps2_graphics.h`
- `tests/Makefile`
- `tests/unit/mpeg_compositor_test.c`

No Application, MPEG decoder, Transport, RFB, UI/calibration, input, audio,
config or framebuffer product source changed.

## Dictionary integration

The deterministic reconciler reported:

- Display before=113, removed=0, added=73;
- Platform before=88, removed=6, added=65;
- every other maintained domain unchanged.

The six removed Platform rows correspond to symbols genuinely removed/moved by
the shared-renderer refactor:

- old `pstvnc_ps2_graphics_present` local clear/texture constants;
- old local overlay x0/y0/x1/y1 temporaries.

They were replaced by current shared-renderer/retained-resource definitions.

Strict reconciliation passed.

Maintained responsibility prose was then updated to state:

- Display owns neutral decoded-frame adaptation and synchronized MPEG compositor
  coordination;
- Platform owns the single GS/dmaKit desktop/suppression/MPEG/matte/overlay
  mechanism plus synchronized completion/timer observation.

## Host behavior evidence

Focused P4 host fixture:

`MPEG_COMPOSITOR_TEST=PASS`

The fixture covers:

- macroblock surface validation;
- deterministic 32x16 two-macroblock detile;
- exact generation and source/base fencing before Platform calls;
- first synchronized frame:
  - physical synchronized result;
  - exact returned timer tick;
  - session-clock first arm;
  - exact P3 promotion;
- subsequent exact-run frame:
  - no re-arm;
  - no duplicate promotion;
  - unchanged session epoch;
- later distinct run using an already-armed session clock:
  - no replacement epoch;
  - exact run still promoted after synchronized presentation;
- Platform failure;
- unsynchronized Platform result;
- wrong tick-domain result;
- clock synchronization failure after physical sync;
- promotion failure after physical sync;
- fully consumed/zero-size inner-content matte geometry.

Final host run also preserved:

- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`.

## Direct PS2 evidence

Final coherent source authority:

`37bbd71ca5afaf3dfb63568e85f9d31dbf043d08`

GitHub Actions run:

`35542987831`

completed SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

The strict PS2 compile log explicitly contains:

- `PS2_COMPILE=src/display/mpeg_frame.c`;
- `PS2_COMPILE=src/display/mpeg_presentation.c`;
- `PS2_COMPILE=src/display/mpeg_compositor.c`;
- `PS2_COMPILE=src/platform/ps2_graphics.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

The linked build log explicitly compiles and links:

- `mpeg_frame.o`;
- `mpeg_presentation.o`;
- `mpeg_compositor.o`;
- `ps2_graphics.o`

into the current ELF on both clean builds, then reports:

`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Therefore P3/P4 source is now directly covered by canonical PS2 compile and
linked whole-ELF/PT_LOAD reproducibility machinery.

This remains machine/build evidence, not physical hardware qualification.

## A004 P4 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.
They are not Foreman acceptance.

### A004-P4-C1 SINGLE_GS_OWNER — MET

Only existing `src/platform/ps2_graphics.c` includes/calls gsKit/dmaKit.
Display coordinator makes no GS/dmaKit calls.

There is no second presenter and no graphics semaphore.

### A004-P4-C2 FRAME_CONTRACT — MET

`pstvnc_mpeg_rgb16_macroblock_surface_t` is an explicit bounded neutral RGB16
macroblock surface carrying pointer/capacity/width/height only, independent of
decoder-private state.

### A004-P4-C3 EXACT_RUN_GEOMETRY — MET

Display coordinator requires exact P3 generation equality and exact
source-width/base-width plus source-height/base-height agreement before physical
presentation. Stale/wrong generation and geometry mismatch fail before Platform
is called.

### A004-P4-C4 COMPOSITION_ORDER — MET

The sole Platform render body structurally orders:

desktop -> suppression -> MPEG -> inner matte -> local overlay ->
queue -> synchronized flip.

### A004-P4-C5 SUPPRESSION_TRUTH — MET

Desktop texture remains a separate authoritative presentation surface.
Ordinary desktop updates while Platform retains video redraw the composite rather
than replacing framebuffer truth or revealing suppression pixels.

Suppression is a black physical layer, not a second framebuffer authority.

### A004-P4-C6 RESOURCE_DISCIPLINE — MET

Platform uses one maximum-size reusable video VRAM allocation and one fixed
bounded EE detile buffer. Desktop/overlay resources are cached. MPEG frame
presentation uploads video only; unchanged full desktop/overlay upload is not a
per-MPEG-frame correctness requirement.

No per-frame/per-run permanent allocation was added.

### A004-P4-C7 SYNC_BOUNDARY — MET

Platform video success is returned only after one `gsKit_queue_exec()` and one
`gsKit_sync_flip()`.

The timer observation occurs after that sync. Upload/queue submission alone
cannot produce a synchronized result.

### A004-P4-C8 SESSION_CLOCK_ARM — MET

WAIT_FIRST_FRAME checks the session clock only after synchronized Platform
success. If unarmed, it arms from the returned post-sync tick.

If already armed, the existing epoch is preserved. Host evidence proves a later
run does not replace the session epoch.

### A004-P4-C9 FIRST_FRAME_PROMOTION — MET

Exact P3 promotion occurs only after synchronized Platform success and clock
state has been validated/armed as required.

No START, decode, detile, upload or queue-only path invokes promotion.

### A004-P4-C10 SUBSEQUENT_FRAME_STABILITY — MET

MPEG_OWNED exact-run frames use the composited path without re-arming the clock
or re-running first-frame promotion. The session epoch remains unchanged.

### A004-P4-C11 Q7_GUARD — MET

No active `MPEG_OWNED -> RFB_ONLY` clear/stop shortcut exists.

No `clear_video()` API was added. Retained suppression/video resources remain
available for the later current-Q7 retirement/rebuild/final-reveal process.

### A004-P4-C12 CLEAN_DIRECT_BUILD_EVIDENCE — MET

Focused host tests, complete host suite, canonical project check, strict
dictionary audit, direct PS2 compile, linked PS2 build and linked
reproducibility all pass on the exact final source authority.

P3 and P4 translation units are now directly compiled and linked by those
canonical jobs.

Physical hardware qualification remains separate and unclaimed.

## Physical/evidence boundary

PENDING_LOCAL=NONE_FOR_A004_P4_REQUIRED_REPOSITORY_MACHINE_GATES

PHYSICAL_PS2_PRESENTATION=NOT_RUN_NOT_CLAIMED
TELEVISION_LAYER_ORDER=NOT_PHYSICALLY_QUALIFIED
FIRST_REAL_MPEG_FRAME_CLOCK_ARM=NOT_PHYSICALLY_QUALIFIED
AUDIO_VIDEO_SYNC=NOT_PHYSICALLY_QUALIFIED
Q7_RETIREMENT=NOT_IMPLEMENTED
MPEG_WORKER_HANDOFF=NOT_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
FULL_A004_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

The host and PS2 toolchain evidence prove source/build contracts only. Only a
later physical PS2/television run can qualify actual visible layering, timing,
first-frame presentation and endurance.

## Exact next pickup

Return the baton to Foreman at pre-log P4 source authority:

`37bbd71ca5afaf3dfb63568e85f9d31dbf043d08`

plus this immutable Reconstruction log commit.

Foreman should independently inspect:

- one-GS-owner source shape;
- frame/detile contract;
- physical sync/timer seam;
- session-clock first-arm/later-run behavior;
- exact P3 promotion ordering;
- direct PS2 compile/link coverage;
- absence of active direct-clear/stop behavior;
- the preserved role-correction note concerning the earlier erroneous NOOP
  Foreman log.

Foreman then chooses the next bounded scheduler/Q7-retirement/runtime seam from
actual returned P4 source.

This Reconstruction worker must not select, publish or execute that later packet
in this shift.
