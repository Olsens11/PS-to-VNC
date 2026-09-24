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
