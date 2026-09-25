# Pi MPEG generation control and producer lifecycle

## Purpose

This note records the maintained R17 Raspberry Pi boundary that consumes the
already-accepted PS2 MPEG START/RETIRE control representation.

The central rule is:

> **The generation owner owns MPEG behavior; WireConnectionOwner owns physical
> Wire I/O and sequence.**

R17 does not create another socket reader/sender and does not move global PSTV
sequence ownership out of the Wire server.

## Exact control representation

The maintained Pi codec mirrors `src/transport/protocol.*`:

- START: frame kind 11, control channel 0, flags 0, 44-byte payload;
- RETIRE: frame kind 10, control channel 0, flags 0, 12-byte payload;
- generation-control payload version 1;
- all fields are big-endian uint32;
- MPEG DATA: ordinary DATA kind 3 on logical channel 4, non-empty and no larger
  than the accepted PSTV payload maximum;
- MPEG CREDIT: ordinary CREDIT kind 4 on logical channel 4 with one nonzero
  big-endian uint32 amount.

This extension changes neither fixed PSTV header version 1 nor Q4 product
compatibility version 2.

## Exact generation authority

One `MpegGenerationController` belongs to one active Wire Session. A START is
valid only when its session identity matches, generation is nonzero and newer
than every generation previously admitted by that controller, and its geometry
is valid against current injected desktop dimensions and Configuration limits.

The START base rectangle is the capture rectangle. The START outer rectangle is
the generation-specific suppression footprint. They are intentionally distinct:
inner matte remains a PS2 rendering concern.

Only one generation may be PREPARING, LIVE, RETIRING, or awaiting RETIRE
completion. A dead generation is never rebound.

## Configuration projection

`src/config/mpeg_runtime_profile.json` is the machine-readable form of the
already-selected A003 R7 MPEG profile. The C runtime consumes generated constants
with the same prior values, while the Pi projection carries only facts needed by
R17: channel window, producer buffer bound, maximum capture dimensions and frame
rate.

`scripts/generate-mpeg-runtime-profile.py --check` fails closed when either
projection is stale.

R17 does not import H1 bitrate, GOP, VBV, timeout, or other laboratory tuning as
new product authority.

## Producer and backpressure

`BufferedMpegProducer` owns one FFmpeg subprocess and one reader thread. Its
stdout buffer cannot exceed the projected Configuration capacity. A full buffer
blocks that local reader rather than accumulating unbounded bytes.

The producer exposes bytes to the generation controller only. It cannot access
the PS2 socket and cannot allocate Wire sequence.

Channel-4 credit is session-scoped, matching PS2 Transport's one initial grant
plus credit returns. The controller consumes that authority only when it admits
one physical-send lease. RFB credit remains separate.

## Retirement proof

RETIRE has this ordered meaning:

1. exact session/generation match is required;
2. state moves LIVE -> RETIRING, closing new emission admission;
3. every already-admitted emission lease leaves the sole physical-send call;
4. the producer process and reader thread are proven retired;
5. the exact suppression preparation is retired;
6. only then may the Wire owner serialize RETIRE completion;
7. after successful completion serialization, local generation state returns to
   IDLE while the monotonic stale-generation fence remains.

A deadline may reveal a stuck lease/process/thread. Expiration is failure, never
proof of retirement. Failed producer ownership cannot be converted into a
successful EOF/completion.

## Suppression boundary

R17 owns and validates the exact generation-specific suppression footprint, but
does not modify the maintained RFB attachment/relay implementation. The default
Pi composition supplies no MPEG factory, so the mechanism is dormant until the
later final Application composition binds the prepared footprint into its
cross-domain presentation/RFB policy.

This keeps R17 inside its authorized Pi MPEG mechanism scope and prevents a
hidden RFB redesign.

## Evidence boundary

Host tests prove codec symmetry, malformed rejection, geometry/session/generation
validation, bounded credit, producer launch/retirement failure, in-flight lease
drain, no timeout-as-success, fresh successor authority, and sole-Wire
serialization. Canonical project/dictionary/PS2 build checks remain required.

These are source/machine facts, not physical PS2/Pi qualification.

## R18 PS2 Transport run boundary

R18 adds the complementary PS2-side **Transport** mechanism required to reuse
the already-allocated channel-4 queue safely across successive MPEG runs within
one Wire Session. It does not move generation identity or Application policy
into Transport.

The public Transport bridge now exposes three run-boundary operations:

- `pstvnc_transport_mpeg_run_open()` — opens one DATA-admission interval only
  from clean idle Transport state;
- `pstvnc_transport_mpeg_run_abort_pre_start()` — closes an opened boundary
  before START only when no run bytes, credit debt, completion, producer-done or
  activity ownership exists;
- `pstvnc_transport_mpeg_run_finalize()` — after exact RETIRE completion has
  been taken and the higher owner has retired its consumer, discards residual
  old-run bytes, returns all owed channel-4 credit once through the existing sole
  Transport sender, clears run-local state and leaves the queue allocation ready
  for a later explicit open.

Transport does not allocate a generation number for these operations. The
existing START/RETIRE payload continues to carry the owning MPEG generation
identity; the Transport facts only describe whether this session-scoped queue is
currently allowed to admit channel-4 DATA and whether retirement/finalization is
still latched.

### Completion and DATA ordering

Exact RETIRE completion closes DATA admission under the MPEG queue lock **before**
the completion value is published into the control slot. Transport retains the
exact submitted RETIRE payload only as transaction-correlation evidence; the
decoded completion must match that payload exactly before it can close
admission. This does not allocate or interpret MPEG generation identity in
Transport.

Taking that completion clears only the one value slot; it does not reopen
admission or clear the retirement latch.

Consequently, DATA arriving after completion is a protocol failure even if the
higher owner has already taken the completion. A later run cannot open until
finalization succeeds.

### Residual bytes and session-scoped credit

Residual discard is deliberately different from decoder consumption.
`pstvnc_transport_mpeg_channel_discard_all()` returns the exact queued byte
count and resets ring offsets without claiming those bytes were decoded.

The initial MPEG credit window remains Wire-session scoped. Normal decoder reads
may leave consumed-byte credit withheld below the configured batch threshold.
At run finalization Transport combines that pending consumed-credit debt with the
exact residual-discard count and returns the sum once through its ordinary
outbound CREDIT path. No new initial-credit grant is minted when a successor run
opens.

This is what lets one session-scoped channel window survive run transitions
without allowing old-run queue capacity or bytes to leak into N+1.

### Consumer fence and fresh reuse

The higher MPEG/Application owner remains responsible for proving decoder/worker
retirement before requesting finalization. Transport does not invent a decoder
stop token or timeout. It does, however, refuse finalization while its own
protected MPEG activity-wait state proves a live waiter still owns the channel.

A successful MPEG read remains a Transport-visible consumer transaction until
its consumed-byte credit bookkeeping and any resulting physical CREDIT send
finish. Finalization also refuses while that transaction is active. Once
finalization starts, Transport refuses new MPEG reads/status snapshots/activity
waits/producer-done publication until the old-run reset and credit return are
complete. This closes the race in which a late consumer could otherwise create
new old-run state while finalization temporarily releases the queue lock to use
the sole physical sender.

Successful finalization resets residual queue state, producer-done, pending
credit, completion/retirement facts, activity sequence and START/RETIRE
submission facts. The same bounded queue allocation is then reusable only
through a fresh explicit run-open. No old run state is rebound to the successor.

R18 changes no fixed PSTV framing, START/RETIRE bytes, Pi R17 producer
mechanism, RFB/AUDIO policy, MPEG decoder/worker/presentation ownership, or
Application activation policy.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-MPEG-TRANSPORT-RUN-BOUNDARY-R18`.


## R20 Transport-private Wire identity binding

R20 changes the **public Transport bridge seam**, not the accepted MPEG wire
representation.

Above Transport, callers now express only run-owned meaning:

- START request: nonzero generation plus base/suppression rectangles;
- RETIRE request: nonzero generation;
- RETIRE completion: exact completed generation.

The public values deliberately have no Wire `session_id` and no
generation-control protocol version. Pi still allocates the nonzero Q4 session
identity, and PS2 Transport remains its sole owner after ACCEPT.

When a current access ticket submits START or RETIRE, the bridge stamps:

- `PSTVNC_MPEG_GENERATION_CONTROL_VERSION`;
- the current private Q4 session identity;
- the caller-owned generation/geometry.

It then passes the unchanged full wire payload to the private runtime. Runtime
continues to retain and compare the full submitted RETIRE transaction, so
generation/session/version correlation remains below the bridge.

Completion projection is deliberately one-way. The runtime yields the full
wire completion internally; the bridge verifies its private version/session
authority and publishes only the generation above Transport. An impossible
private-identity contradiction makes the current runtime terminal rather than
leaking or repairing identity upward.

R20 also validates only geometry facts that Transport can know without stealing
Display/calibration authority: nonzero generation, 16-pixel-aligned nonempty
base dimensions, nonempty overflow-safe rectangles, and suppression containment.
Desktop bounds and selected MPEG profile limits remain with their existing
higher/Pi owners.

A replacement Wire Session therefore receives a new private Q4 identity while
the higher-level request type remains reusable run meaning. A stale access
ticket cannot submit anything; a current ticket always stamps the current
session's identity. No public session-ID accessor exists.

The fixed control bytes remain unchanged:

- START: kind 11, channel 0, flags 0, 44-byte version-1 payload;
- RETIRE: kind 10, channel 0, flags 0, 12-byte version-1 payload.

R18 run-open/retirement/finalization and R17 Pi exact-session/generation
validation remain unchanged.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-MPEG-PRIVATE-SESSION-BINDING-R20`.


## R21 Application run-start transaction

R21 adds a trigger-agnostic Application-owned start transaction without
activating MPEG in the ordinary product loop.

One session-scoped coordinator owns only cross-domain ordering and the monotonic
nonzero MPEG run generation. It consumes the selected R7 MPEG profile, one
already-resolved P3 geometry value, an already-frozen P2 flow policy, current
Transport access, Presentation state, and the session media clock.

The successful ordering is:

1. validate profile/geometry and existing RFB publication protection;
2. allocate a fresh never-reused generation;
3. open one clean R18 Transport MPEG run;
4. create the fresh R5 PS2 worker runtime and operation tables;
5. create the fresh R3 decoder backend operation table;
6. start the R4 worker for that exact generation;
7. arm P3 Presentation in WAIT_FIRST_FRAME and verify its immutable snapshot;
8. initialize the P7 frame consumer for that same worker/generation/snapshot;
9. serialize R20 START as the final startup action.

START contains the exact base and suppression rectangles from the retained P3
snapshot. The inner matte remains Presentation-only and is never serialized.

Any failure before START attempts reverse-order retirement. Returning to IDLE is
legal only when all acquired owners are proven released and R18 pre-START abort
succeeds. If cleanup cannot be proven, the coordinator faults and requires outer
session teardown.

Calling START is deliberately the irreversible boundary. A non-OK START result
cannot prove that no bytes escaped, so the coordinator faults and must not call
the R18 pre-START abort path.

R21 does not choose a user gesture, freeze or thaw RFB, service MPEG frames,
perform retirement, send RETIRE, mark producer completion, finalize Transport,
reveal RFB, or enable the Pi MPEG product path. Ordinary `src/app.c` remains
unchanged.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-APPLICATION-MPEG-RUN-START-R21`.


## R22 Application live-service composition

R22 extends the trigger-agnostic R21 coordinator with one explicit
`MPEG_OWNED` Application state and one nonblocking live-service operation.
The operation delegates frame work exclusively to the existing P7 consumer for
the coordinator's exact current generation and returns P7's detailed service
result unchanged for later main-loop policy.

Before the first synchronized frame, benign P7 `IDLE` with an unfinished
worker preserves `STARTED_WAIT_FIRST_FRAME`. Application advances to
`MPEG_OWNED` only when P7 reports a successful first `PRESENTED` result whose
effects prove synchronization and first-frame promotion, while P3 independently
reports `MPEG_OWNED` for the same exact generation.

P7/compositor remain sole owners of physical synchronization, shared media-clock
arm and P3 first-frame promotion. Application does not recreate those actions.

After promotion, P7 `IDLE`, `WAIT`, `PRESENTED` and `DROPPED` are ordinary
live outcomes. The wrapper preserves P7's exact WAIT claim/deadline and keeps
the same Application generation/state. Unexpected worker completion, negative
P7 service, a faulted P7 status, wrong generation or impossible P3/Application
state faults the run and requires outer teardown.

R22 deliberately performs no direct cleanup on live-service failure: it does
not clear P7, abort Presentation, stop/join/release the MPEG worker, release the
runtime, send RETIRE, finalize Transport, thaw/reveal RFB or manufacture
retirement success. Lower-owner evidence remains intact for later failure/
retirement orchestration.

Ordinary `src/app.c` and Pi MPEG product activation remain unchanged.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-APPLICATION-MPEG-LIVE-SERVICE-R22`.


## R23 Application retirement drain

R23 adds the trigger-agnostic retirement transaction for one exact healthy
Application `MPEG_OWNED` generation. It deliberately separates execution
retirement from later RFB restoration/reveal.

Retirement admission requires exact P3/P7 generation ownership, live R18/R4/R5
owners, nonfaulted P7 state, a naturally running worker, and P2 still frozen.
Application first advances P3 to `RETIRING`, preserving the retained MPEG
composite, then records and invokes exactly one current-generation R20 RETIRE.
That invocation is irreversible: failure faults and requires outer teardown
without returning P3 to MPEG_OWNED or pretending the run is reusable.

While RETIRE completion is pending, Application continues only the accepted P7
consumer in P3 `RETIRING`. P7 IDLE, WAIT, PRESENTED and DROPPED remain ordinary
drain outcomes; WAIT preserves its exact claim and deadline. RETIRE-completion
`WOULD_BLOCK` is benign. Only an exact current-generation completion is
accepted, after which Application publishes Transport producer-done exactly
once.

Worker completion before that producer fence is a fault. After producer-done,
Application continues P7 drain until P7 reports worker-finished with no
outstanding borrow. Clean retirement then requires exact worker join and an
exact-generation `COMPLETED` outcome; Application does not request worker stop
on this normal path.

After clean outcome proof, Application retires the empty P7 value, releases R4,
releases R5, and calls R18 Transport finalization last. Any failure preserves the
ownership facts already reached and faults for outer teardown instead of
manufacturing cleanup.

The original R23 candidate entered explicit `RESTORE_PENDING`, retained the
exact generation and P3 `RETIRING` snapshot, and left P2 frozen. Foreman State
0059 identified that last P2-frozen requirement as a planning conflict with
governing Q7 overlap semantics. R23C below supersedes only that suppression/
restoration edge; the R23 completion/producer/worker/reclaim/finalize fences
remain the execution-retirement foundation.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-APPLICATION-MPEG-RETIREMENT-DRAIN-R23`.


## R23C Q7 restoration-overlap correction

R23C preserves R23 retirement admission: P2 must already be frozen, P3 must be
exact-generation `MPEG_OWNED`, and the accepted P7/R4/R5/R18 owners must be
healthy. P3 still enters `RETIRING` before the one irreversible exact-generation
RETIRE call.

The correction begins immediately after RETIRE returns OK. Application invokes
only P2's public `pstvnc_rfb_flow_policy_set_frozen(..., 0)` transition. That
real frozen-to-thawed transition creates P2's accepted coalesced one-shot FULL
refresh debt. Application verifies only the public consequence: remote
publication is allowed and the next request is FULL when no request is already
outstanding, otherwise HOLD. It never clears FULL debt, records request send,
or manufactures update completion itself.

After that release, ordinary R19 RFB service may request and receive desktop
updates underneath the still-visible MPEG composition. P3 remains exact-
generation `RETIRING`, and P7 continues unchanged IDLE/WAIT/PRESENTED/DROPPED
drain. R23C retirement service therefore requires P2 to remain thawed rather
than frozen; an unexpected re-freeze fails closed.

All later R23 fences remain unchanged: exact RETIRE completion precedes
producer-done, natural no-borrow worker completion precedes join and exact
COMPLETED outcome, P7/R4/R5 reclaim precedes R18 finalization, and normal
retirement never requests worker stop. A failure after Q7 release does not
re-freeze P2 or relabel restoration as complete.

Successful execution retirement still ends `RESTORE_PENDING` with exact
generation and P3 `RETIRING` snapshot retained, but P2 is thawed. Its FULL
obligation may still be pending, may be in flight, or may already have been
satisfied by the outer ordinary RFB loop; this coordinator does not infer
freshness from retirement completion.

R23C still does not seal/commit P3, execute synchronized no-MPEG reveal, choose
a user trigger, modify ordinary `src/app.c`, activate the Pi MPEG product
factory, or alter lower-owner P2/P3/P7/Transport/MPEG mechanisms.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-APPLICATION-MPEG-Q7-RESTORE-OVERLAP-R23C`.


## R24 final restored-RFB reveal

R24 completes the trigger-agnostic visible retirement transaction without
activating MPEG in the ordinary product loop.

The accepted R23C endpoint is exact-generation `RESTORE_PENDING`: execution
owners are retired, P3 still retains the visible `RETIRING` MPEG snapshot and
P2 is thawed so ordinary RFB restoration can run underneath. R24 deliberately
does not treat that P2 protocol state as proof that the restored authoritative
desktop has actually crossed the graphics presentation/upload boundary.

Application therefore owns one additional run-scoped fact:
`rfb_restoration_presented`. It may be recorded only for the exact current
`RESTORE_PENDING` generation after P2 proves the post-thaw FULL transaction is
complete: P2 is thawed, no request is outstanding and its next request is
ordinary INCREMENTAL. The API contract additionally requires its caller to have
successfully presented/uploaded the corresponding authoritative FULL-refreshed
desktop through the existing graphics path before recording the fact. R24 does
not itself perform that desktop presentation and does not manufacture the proof
from protocol completion.

FULL debt still pending, an in-flight RFB request, frozen P2, missing explicit
presentation proof, wrong generation or fabricated execution-retirement
authority blocks final reveal before P3 seal.

Once exact RESTORE_PENDING authority, current P2 protocol freshness and the
explicit graphics-presentation proof all agree, Application seals P3 exactly
once into `REVEAL_PENDING`. Application records the corresponding explicit
`PSTVNC_APP_MPEG_RUN_REVEAL_PENDING` state, then invokes only
`pstvnc_mpeg_compositor_reveal_retired()` for the exact current generation.

The accepted compositor remains sole owner of the physical synchronized
desktop/no-MPEG reveal and the final P3 logical commit. Application never calls
Platform graphics directly and never duplicates
`pstvnc_mpeg_presentation_commit_reveal()`.

Compositor `PLATFORM_FAILED` and `SYNC_INVALID` outcomes are retryable
pre-sync failures. Application preserves REVEAL_PENDING, the exact generation,
the RFB-presentation proof and the P3 retained snapshot without setting session
teardown merely for those results. A later exact retry bypasses P3 seal and
invokes the compositor reveal again.

Other compositor failures or state/effect contradictions fail closed and
require containing recovery. In particular, an OK return is accepted only when
its effects prove synchronized retirement reveal and P3 independently reports
RFB_ONLY with no retained snapshot and no MPEG visual ownership.

Only that exact physical boundary returns the coordinator to reusable IDLE.
Current-generation/transient run state is cleared, `current_generation`
becomes zero, and `last_allocated_generation` is preserved so a later caller
that re-establishes the existing P2-frozen start precondition receives N+1
rather than reusing the retired generation. R24 never resets the externally
owned session media-clock object.

R24 does not choose a product trigger, modify ordinary `src/app.c`, activate
the Pi MPEG factory, alter P2/P3/compositor/Platform/P7/MPEG/Transport
mechanisms, or change AUDIO, Input/UI/calibration or Wire protocol behavior.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-APPLICATION-MPEG-FINAL-RFB-REVEAL-R24`.


## P8 manual MPEG CALIBRATION region source

A004 P8 reconstructs one UI-owned manual source for an MPEG presentation region.
It composes the accepted P1 MPEG CALIBRATION core rather than replacing or
duplicating that core.

The manual source remains explicitly begin-driven. It contains no START+SELECT
entry detector, hold timer, or permanent product trigger. A future Application
owner may choose a trigger and is responsible for consuming that entry sample
before calling `pstvnc_mpeg_calibration_manual_begin()`.

After begin, the source translates only normalized
`pstvnc_controller_state_t` facts into the existing P1 action vocabulary:

- D-pad edits base size around center;
- R1 + D-pad moves base placement at pixel precision;
- R2 + D-pad edits presentation-local inner matte;
- L2 + D-pad edits outer/suppression matte;
- START resets the P1 candidate;
- CROSS enters review and, only after release/re-press, accepts;
- CIRCLE backs out of review or cancels from edit.

The adapter does not poll libpad. SELECT is not an MPEG-calibration action and
no physical entry chord is encoded in this region source.

While P1 owns EDIT, REVIEW, or release quarantine, the service result marks the
normalized controller sample consumed so a later coordinator can prevent the
same sample from also reaching ordinary desktop/OSK routing. P8 does not itself
suspend input runtime, neutralize/rebase remote pointer state, freeze RFB, or
otherwise acquire those cross-domain responsibilities.

Acceptance is only the P1 one-shot accepted edge. P8 copies the accepted
`pstvnc_mpeg_calibration_region_t` into its result; it does not infer
acceptance from committed state or foreground exit. Cancel similarly produces
no accepted region, and calibration retains controller ownership through P1's
existing release quarantine until a fully released sample proves completion.

The visible plan is platform-neutral and contains the exact P1 candidate plus
the geometry returned by `pstvnc_mpeg_calibration_resolve_geometry()`:
base rectangle, distinct inner-content rectangle and clipped outer/suppression
footprint. EDIT and REVIEW are the only visible states. The frozen H1 Controls
screen and adapter/checkpoint proliferation are not promoted into clean product
architecture.

The companion CT16 rasterizer copies a caller-supplied frozen desktop into a
distinct caller-owned output surface, darkens the exact suppression footprint,
restores the exact base, applies the inner matte as the complement of P1's
resolved inner-content rectangle, and draws the selector on P1's exact base
edge. It never mutates the frozen input. Buffer overlap, insufficient capacity,
invalid rectangles and inconsistent plans fail before raster writes.

The output remains region-source data only. P8 does not mutate P2, request RFB
updates, start Transport/MPEG execution, arm P3, call Platform graphics, persist
Pi configuration, touch DESKTOP CALIBRATION, or modify ordinary `src/app.c`.
Future automatic or Application-pinned MPEG region sources may produce the same
region value without depending on this manual controller adapter.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A004-MPEG-CALIBRATION-MANUAL-REGION-SOURCE-P8`.


## P9 Application-owned manual calibration foreground

A004 P9 composes the accepted manual MPEG CALIBRATION region source into one
trigger-agnostic Application-owned foreground transaction. It deliberately does
not select a permanent controller chord and does not start MPEG.

Admission is exact and RFB-only: the local UI must be ordinary DESKTOP with no
transition quarantine, P3 must be RFB_ONLY with no retained run snapshot, P2
must be thawed, P8 must be inactive, and the Application calibration
coordinator must be idle.

After admission the ordering is fixed:

1. freeze P2;
2. suspend mouse interpretation while physical controller polling remains live;
3. if a successfully-published remote click is held, serialize a button-up at
   the exact published cursor and only then record the published click state as
   neutral;
4. rebase the suspended mouse interpreter to that exact neutral published
   state;
5. copy the caller-supplied last physically presented ordinary CT16 desktop into
   dedicated caller-owned frozen storage;
6. begin P8;
7. raster visible P8 EDIT/REVIEW state into a distinct work surface and present
   that surface through the ordinary Platform desktop seam.

The authoritative RFB framebuffer is neither used nor mutated as calibration
drawing storage. The frozen snapshot is immutable for the transaction.

P8 controller samples remain exclusively calibration-owned while P1 owns EDIT,
REVIEW or release quarantine. The Application coordinator reports the
consumption fact for later ordinary-main-loop integration; it does not alter
local-controller/OSK routing mechanisms.

On P8's one-shot accepted edge, P9 resolves the copied accepted region through
`pstvnc_mpeg_calibration_resolve_geometry()` and copies the resulting base,
inner-content and suppression rectangles field-for-field into
`pstvnc_mpeg_presentation_geometry_t`. P9 performs no independent geometry or
suppression arithmetic.

Accept and cancel both remain protected while release quarantine is pending.
Only after P8 proves complete release does P9 physically present the exact
frozen ordinary desktop. Mouse interpretation resumes only after that successful
restore.

The endpoints then diverge:

- cancel thaws P2 exactly once and returns reusable idle; the existing P2
  frozen-to-thawed transition creates the normal one-shot FULL-refresh debt,
  which may remain HOLD while an older request is still outstanding;
- accept keeps P2 frozen and P3 RFB_ONLY while retaining exact resolved geometry
  in explicit ACCEPTED_PROTECTED state. No MPEG run has started.

Reading/copying protected accepted geometry has no protection side effect. A
separate explicit abort operation may abandon ACCEPTED_PROTECTED without
starting MPEG; it thaws P2 exactly once, preserves normal FULL-refresh debt,
clears retained protected geometry and returns idle.

Any failure after P2 freeze fails closed. P9 does not silently thaw or resume
uncertain input ownership after pointer serialization, rebase, raster or
Platform-presentation failure.

P9 remains separate from DESKTOP CALIBRATION and does not modify ordinary
`src/app.c`, call any `pstvnc_app_mpeg_run_*` operation, mutate P3, start a
decoder/worker, persist Pi configuration, alter Transport/Wire semantics or own
any lower-component mechanism.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A004-APPLICATION-MPEG-CALIBRATION-FOREGROUND-P9`.
