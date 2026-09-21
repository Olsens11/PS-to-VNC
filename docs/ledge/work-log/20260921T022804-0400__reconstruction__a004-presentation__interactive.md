# Reconstruction shift — A004 main-thread MPEG frame consumer P7

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T02:28:04-04:00
COMPLETED_AT=2026-09-21T02:43:21-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a04c775cf0bff2d9b89a01f1764c769ef59fe1a9
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A004-MAIN-THREAD-FRAME-CONSUMER-P7`

The bounded objective was to reconstruct the narrow Application/main-thread
consumer that bridges one exact A003 R4 worker claim into the already accepted
A004 P4 compositor and P6 scheduler seams.

P7 consumes already-started exact-run state only. It does not create/start/join
or destroy the MPEG worker, arm Presentation from calibration, start/retire the
Pi producer, execute P5 begin/seal/reveal, own Transport residual/RFB restoration,
or perform final Application MPEG lifecycle orchestration.

## Authority consumed

Consumed current authority at wake:

- Foreman State revision 0032;
- Reconstruction Contract revision 0006;
- work-log contract revision 0007;
- accepted R4 `src/mpeg/worker.{c,h}`;
- accepted P3-P6:
  - `src/display/mpeg_presentation.{c,h}`;
  - `src/display/mpeg_frame.{c,h}`;
  - `src/display/mpeg_compositor.{c,h}`;
  - `src/display/mpeg_scheduler.{c,h}`;
- `src/media/clock.{c,h}`;
- clean Application/source-topology ownership rules;
- canonical host/direct-PS2/link manifests.

Starting branch authority:

`a04c775cf0bff2d9b89a01f1764c769ef59fe1a9`

Execution classification:

`AUTONOMOUS_RECONSTRUCTION`

No user terminal proxy or physical PS2 action was required or requested.

## Placement / ownership

Added:

- `src/app_mpeg_frame.h`;
- `src/app_mpeg_frame.c`.

The module is deliberately Application-owned at the clean source root because it
coordinates public seams across MPEG worker and Display presentation domains.
It does not own either domain's private mechanism and does not justify a new
top-level source directory.

The clean root allowlist and source-topology documentation were updated narrowly
to admit:

- `src/app_mpeg_frame.c`;
- `src/app_mpeg_frame.h`.

No new source directory was created.

## Public consumer state

`pstvnc_app_mpeg_frame_consumer_t` owns only main-thread coordination facts:

- exact caller-owned nonzero run generation;
- references to one R4 worker, one Presentation owner and one session media
  clock;
- copied immutable caller scheduler profile;
- P6 scheduler state;
- one currently claimed R4 borrowed-frame metadata value;
- last consumed picture ordinal;
- presented/drop accounting;
- scheduler-initialized, claim-outstanding and faulted facts.

It owns no decoder buffer independently, GS resource, Transport queue, RFB state
or Pi producer state.

Public operations:

- `pstvnc_app_mpeg_frame_consumer_init()`;
- `pstvnc_app_mpeg_frame_consumer_service()`;
- `pstvnc_app_mpeg_frame_consumer_status()`.

## Exact run fence

Initialization requires:

- nonzero caller generation;
- Presentation already in WAIT_FIRST_FRAME;
- immutable Presentation snapshot generation exactly equal to the caller run;
- R4 worker status accepting that same generation;
- caller-supplied scheduler profile and existing session clock.

The generation is copied verbatim and never minted, incremented or rebased by P7.

Every service/status operation validates the exact run generation.

## Nonblocking acquisition

When no frame is currently held, one service call attempts at most one
`pstvnc_mpeg_worker_claim()`.

`PSTVNC_MPEG_WORKER_FRAME_UNAVAILABLE` is ordinary P7 IDLE. In that case P7 may
observe R4 worker status and report the synchronized `worker_finished` fact but
does not join, destroy or reinterpret the worker's terminal outcome.

P7 never blocks waiting for a worker frame.

## Borrowed RGB16 mapping

A successful R4 claim is mapped directly into one
`pstvnc_mpeg_compositor_frame_t` without copying the full image.

Mapping requires:

- exact worker-frame generation;
- nonzero claim token equal to decoder picture ordinal;
- nonnull picture pixels;
- alignment suitable for `uint16_t`;
- RGB16 bytes-per-pixel;
- nonzero even usable byte count;
- usable byte count not exceeding decoder allocation capacity;
- width/height representable by the neutral frame contract;
- final
  `pstvnc_mpeg_rgb16_macroblock_surface_valid()`
  success.

The neutral surface receives:

- the exact borrowed decoder pointer;
- pixel capacity = usable bytes / 2;
- validated width/height;
- exact run generation in the compositor frame.

No second full RGB16 frame allocation/copy is introduced.

## Exact ordinal continuity

The expected ordinal is exactly:

`last_consumed_ordinal + 1`

The first consumable frame must therefore be ordinal 1.

Every later consumed frame must advance by exactly one. Duplicate, stale,
skipped or out-of-order ordinals fail before scheduler/presentation and enter the
borrow-safe containment path.

Ordinal accounting advances only after PRESENT or DROP has completed its exact
release.

## First-frame authority

Ordinal 1:

- is legal only while Presentation is WAIT_FIRST_FRAME;
- bypasses P6 scheduling;
- passes once through `pstvnc_mpeg_compositor_present()`;
- preserves all compositor effects;
- requires compositor result OK plus truthful:
  - `synchronized`;
  - `first_frame_promoted`.

P7 itself performs no GS call and does not call
`pstvnc_mpeg_presentation_first_frame_presented()` directly.

P4 remains the first-physical-frame/Presentation-promotion authority.

## Scheduler initialization after real sync

Only after successful ordinal-1 P4 composition does P7 initialize P6 using:

- the existing session media clock;
- exact run generation;
- immutable caller scheduler profile;
- P4's returned post-synchronized `observed_sync_tick`.

P7 never calls `pstvnc_media_clock_arm()` and therefore cannot arm or re-arm the
session epoch.

The scheduler is initialized exactly once for the run.

## Later-frame scheduler behavior

Ordinal >=2 requires:

- initialized scheduler;
- Presentation in MPEG_OWNED or RETIRING.

The caller supplies `current_tick` on each service invocation.

P7 invokes:

`pstvnc_mpeg_scheduler_decide()`

and preserves its exact:

- WAIT_UNTIL_DEADLINE;
- PRESENT_NOW;
- DROP_PRESENTATION_ONLY

classification.

No scheduler decision is locally rebased or reinterpreted.

## WAIT pins the exact claim

WAIT:

- retains the same R4 claim;
- returns the absolute P6 deadline;
- performs no release;
- performs no second claim;
- performs no compositor call;
- does not sleep, spin, VBlank-wait or call a media-clock wait helper.

A later main-loop service invocation re-evaluates that same ordinal with a newer
caller-supplied tick.

## PRESENT

PRESENT_NOW:

1. passes the exact mapped run/frame through the sole P4 compositor;
2. requires successful synchronized effects;
3. only then attempts exact R4 release;
4. only after successful release updates:
   - last consumed ordinal;
   - presented count.

The MPEG worker never calls the compositor.

## DROP

DROP_PRESENTATION_ONLY:

- performs no compositor call;
- changes no decoder reference state;
- changes no Transport/generation/Presentation ownership;
- releases the exact R4 claim;
- after exact release updates only:
  - last consumed ordinal;
  - dropped count.

The worker may then advance to the next decoder picture.

## RETIRING compatibility

Presentation RETIRING remains legal for later ordinals.

Those exact frames continue through the same P6 WAIT/PRESENT/DROP path.

P7 does not begin retirement and does not seal/reveal it.

## REVEAL_PENDING / RFB_ONLY fence

When no claim is held, Presentation REVEAL_PENDING or RFB_ONLY prevents P7 from
claiming a new worker frame.

If a higher-level owner nevertheless changes Presentation into a sealed state
while P7 is already holding a claim, P7 fails closed and runs the same
borrow-safe containment path instead of presenting through the sealed state.

Consumer status explicitly exposes:

- `claim_outstanding`;
- held ordinal.

The later retirement owner can therefore prove no P7-held frame before P5 seal /
final reveal.

## Failure containment

Mapping, ordinal, scheduler initialization, scheduler decision, compositor or
sealed-state failure never silently frees a borrowed frame.

When a failed claim is held P7:

1. marks the consumer faulted;
2. requests exact R4 worker stop;
3. only if stop is successfully established does it attempt exact frame release;
4. if stop cannot be established, it retains the claim and publishes
   `claim_outstanding`;
5. if exact release itself cannot be proven, the held metadata remains visible
   and the consumer stays faulted.

This preserves R2/R4 borrowed-storage safety: decoder storage cannot be
intentionally released for overwrite while P7 still reports a live reference.

A normal PRESENT/DROP release failure likewise faults P7 and requests worker stop
while conservatively retaining the outstanding-claim fact.

## Physical-effect truth

P7 copies P4 compositor result/effects into its service result.

If the compositor reports that physical synchronization occurred before a later
clock/promotion failure, P7 preserves:

- synchronized=true;
- exact `observed_sync_tick`;
- other returned effects.

It does not rewrite the operation as though no physical presentation happened.

## Worker terminal boundary

No-frame service may observe the R4 `worker_finished` status fact.

P7 does not call:

- worker join;
- worker destroy/release;
- worker outcome.

It therefore does not reinterpret decoder COMPLETE/STOPPED/FAILED as
presentation success.

Final lifecycle orchestration remains outside this packet.

## Explicit owner exclusions

Static final-source review confirms P7 contains no:

- Transport private runtime/channel/physical-stream dependency;
- RFB dependency;
- direct gsKit/dmaKit/PS2 graphics call;
- worker start/join/destroy lifecycle call;
- P5 begin-retirement/seal/reveal call;
- media-clock arm call;
- sleep/delay/media-clock wait helper;
- generation mint/increment.

Exact worker `request_stop()` is used only for bounded borrow-safety failure
containment as required by the packet.

## Focused host evidence

Added:

- `tests/unit/app_mpeg_frame_test.c`.

The fixture uses fake public worker and compositor seams while exercising real:

- MPEG RGB16 frame validation;
- Presentation;
- P6 scheduler;
- media clock.

It proves:

- ordinal-1 zero-copy mapping to P4;
- first physical sync/promotion effects;
- scheduler initialization from P4 `observed_sync_tick`;
- WAIT returns the exact absolute deadline and retains the same claim;
- later PRESENT uses the same claim and then releases;
- DROP performs no compositor call and releases presentation-only;
- RETIRING accepts drain presentation;
- REVEAL_PENDING refuses a new claim;
- invalid RGB mapping stops before release;
- first ordinal mismatch stops before release;
- worker-stop failure retains the exact borrowed claim;
- compositor failure preserves synchronized physical effects;
- scheduler-init failure after successful first physical presentation is
  contained;
- post-first-frame scheduler-decision failure is contained before release;
- IDLE may report worker-finished without terminal reinterpretation;
- wrong generation fails before claim.

Final host evidence reports:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`.

## Build integration

Updated:

- `tests/Makefile`;
- `scripts/check-clean-ps2-compile.sh`;
- `mk/issue7-clean.mk`;
- `scripts/continuity-check.sh`;
- `docs/development/source-topology.md`.

Focused host target:

`app-mpeg-frame-unit`

Strict PS2 compile now includes:

`src/app_mpeg_frame.c`

Current linked build now includes:

`build/reconstruction/issue7/app_mpeg_frame.o`

Root clean-source topology now explicitly admits the new Application-owned pair.

## Dictionary integration

Behavior-head strict audit correctly reported missing root dictionary coverage
only for:

- `src/app_mpeg_frame.c`;
- `src/app_mpeg_frame.h`;
- their new symbols.

Canonical deterministic reconciliation commit added:

- 135 root/Application definitions;
- zero removals in root;
- zero additions/removals in every other maintained domain.

Only:

- `src/SYMBOLS.md`;
- generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

changed in that generated reconciliation.

Maintained root responsibility prose explicitly names the exact-run worker-claim
to Display scheduler/compositor main-thread bridge while keeping MPEG, Display,
RFB, Transport and Platform mechanisms in their earned domains.

## Commit chronology

P7 worker range after Foreman base
`a04c775cf0bff2d9b89a01f1764c769ef59fe1a9`:

1. `c9141fe436ab099383e3f5db84eacacfd3802720`
   — `feat(app): add MPEG frame consumer`;
2. `8d0110fab005010173b0f0c547128f435992bc35`
   — tree-identical deterministic dictionary-reconciliation trigger;
3. `6f2844d92789500f6b9893f245c9228343bf5267`
   — automation-created `docs(symbols): reconcile current clean definitions`;
4. `5fb13033e83e2d7626af6f9596b59018e439ec5c`
   — `docs(app): describe MPEG frame-consumer ownership`;
5. `d0c9bcb99725bfdaf6b07c5396a0fa1255039225`
   — `test(app): cover scheduler decision containment`.

## Exact changed paths

Compared with Foreman base, final pre-log authority changes only:

- `docs/development/source-topology.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `scripts/continuity-check.sh`;
- `src/SYMBOLS.md`;
- `src/app_mpeg_frame.c`;
- `src/app_mpeg_frame.h`;
- `tests/Makefile`;
- `tests/unit/app_mpeg_frame_test.c`.

No MPEG worker/decoder/backend, Display, media-clock, Transport, RFB, Platform
graphics, audio, input, UI, framebuffer, config or existing Application runtime
source changed.

## CI history / known unrelated flake

Behavior-head run:

`35569065871`

proved:

- host-unit PASS, including `APP_MPEG_FRAME_TEST=PASS`;
- direct PS2 compile PASS;
- linked current-source build PASS;
- expected project/dictionary failure only because the two new root files had not
  yet been reconciled.

At coherent head
`5fb13033e83e2d7626af6f9596b59018e439ec5c`,
run `35569273458` initially hit the repository's already-known timing-sensitive
unrelated `transport_audio_test.c:698` assertion:

`available == 4u && producer_done == 1`.

No Transport/audio source changed in P7.

The failed host job was rerun on the identical commit with no tree change and
passed. That workflow attempt concluded SUCCESS with every canonical job green.

Final test-closure authority:

`d0c9bcb99725bfdaf6b07c5396a0fa1255039225`

GitHub Actions run:

`35569493813`

completed SUCCESS on the first attempt.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Direct PS2 evidence explicitly reports:

- `PS2_COMPILE=src/app_mpeg_frame.c`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Linked evidence compiles/links `app_mpeg_frame.o` in both clean builds and
reports:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

This is repository/source/build evidence only, not physical hardware
qualification.

## A004 P7 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A004-P7-C1 APPLICATION_OWNER — MET

The consumer is rooted in Application/main-thread source and coordinates only
public MPEG/Display/media-clock seams. Private domain mechanisms remain outside.

### A004-P7-C2 EXACT_RUN_AND_ORDINAL — MET

One nonzero generation is copied verbatim and never minted. Presentation/worker
must match it. Picture ordinals begin at 1 and advance exactly by one.

### A004-P7-C3 BORROWED_RGB16_MAPPING — MET

Claimed R4 metadata maps directly to the neutral RGB16 macroblock surface with
pointer/alignment/bpp/byte/capacity/geometry validation and no full-frame copy.

### A004-P7-C4 FIRST_FRAME_P4_ONLY — MET

Ordinal 1 bypasses P6 and reaches only P4 while WAIT_FIRST_FRAME. P7 requires
truthful synchronized + first-frame-promoted effects and never directly promotes
Presentation.

### A004-P7-C5 SCHEDULER_AFTER_SYNC — MET

P6 initializes exactly once only after successful first P4 sync, using returned
`observed_sync_tick`, exact generation, existing session clock and caller
profile. P7 never arms the clock.

### A004-P7-C6 WAIT_PINS_CLAIM — MET

WAIT retains the exact R4 claim, returns the absolute deadline, does not release,
reclaim, sleep or wait, and re-evaluates the same ordinal on later service.

### A004-P7-C7 PRESENT_RELEASES — MET

PRESENT invokes the sole P4 compositor, then exact worker release, then updates
consumer ordinal/present accounting.

### A004-P7-C8 DROP_RELEASES_ONLY_PRESENTATION — MET

DROP performs no compositor call and changes no decoder/Transport/Presentation
ownership; it exact-releases the claim and updates only consumer ordinal/drop
facts.

### A004-P7-C9 RETIRING_DRAIN — MET

Later exact frames continue through P6/P4 while Presentation is RETIRING. P7
never begins or seals retirement.

### A004-P7-C10 SEALED_STATE_FENCE — MET

REVEAL_PENDING/RFB_ONLY prevent new claims. Outstanding claim state/ordinal are
publicly observable, and an unexpectedly sealed held claim is contained rather
than presented.

### A004-P7-C11 FAILURE_BORROW_SAFETY — MET

Mapping, ordinal, scheduler-init/decision, compositor and sealed-state failures
request exact worker stop before failed-claim release. Stop failure retains the
claim. Release failure retains outstanding state. Compositor physical effects
remain truthful.

### A004-P7-C12 CLEAN_EVIDENCE — MET

Focused/full host tests, canonical project check, complete strict dictionaries,
direct PS2 compile, preserved SMS dedicated compilation, clean linked build and
linked current-source reproducibility pass on the exact final authority.

Physical qualification remains separate.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A004_P7_REQUIRED_REPOSITORY_MACHINE_GATES
P7_WIRED_INTO_CURRENT_APP_LIVE_LOOP=NO
CONCRETE_PS2_WORKER_THREAD_EVENT_BINDING=NOT_IMPLEMENTED
PRESENTATION_ARM_FROM_CALIBRATION=NOT_IMPLEMENTED
PRODUCER_START_ACTIVATION=NOT_IMPLEMENTED
P5_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
PRODUCER_RETIRE_ACK=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
WIRE_LOSS_MPEG_RECOVERY=NOT_IMPLEMENTED
FINAL_APPLICATION_MPEG_TRANSACTION=NOT_IMPLEMENTED
PHYSICAL_PS2_MPEG_PRESENTATION=NOT_RUN_NOT_CLAIMED
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

The P7 module is compiled and linked into the current product ELF, but the
existing `src/app.c` live product loop does not yet instantiate/call it. That
final activation/orchestration belongs to later bounded Application work.

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`d0c9bcb99725bfdaf6b07c5396a0fa1255039225`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- Application ownership and topology placement;
- exact run/ordinal continuity;
- zero-copy borrowed RGB16 mapping;
- first-frame P4-only path;
- post-sync scheduler initialization;
- WAIT-held claim behavior;
- PRESENT/DROP exact release semantics;
- RETIRING drain and sealed-state fence;
- failure containment and physical-effect truth;
- absence of later lifecycle ownership;
- final host/direct-PS2/link evidence.

The Foreman, not this worker, chooses any remaining concrete PS2 worker-binding
or activation/retirement transaction packet.

This Reconstruction worker must not begin producer/retirement/final Application
orchestration in this shift.
