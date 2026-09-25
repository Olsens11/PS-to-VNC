# Raspberry Pi product source

DIRECTORY=pi
GENERATION=CLEAN_RECONSTRUCTION
OWNER=RASPBERRY_PI_COMPANION_RUNTIME

This is the maintained product-source root for custom Raspberry Pi companion
runtime code. It is distinct from:

- `src/`, which is the PlayStation 2 executable source tree;
- `scripts/pi/`, which is provisioning/staging/development tooling;
- `experiments/`, which is evidence and apparatus rather than product runtime.

Current maintained Pi ownership through R25:

- `wire_protocol.py` — exact product Wire framing/Q4 establishment, RFB
  representation, and the symmetric R17 MPEG START/RETIRE/DATA/CREDIT bytes;
- `wire_server.py` — persistent Pi listener, provisional establishment,
  process-local session IDs, the sole accepted physical Wire recv/send and
  sequence owner, plus optional composition of one explicitly supplied
  session-scoped RFB attachment and optional exact-generation MPEG owner;
- `wire_runtime.py` — ordinary product composition. R15 consumes the R14
  selected RFB flow and supplies one fresh R13 attachment factory; R25 also
  supplies one fresh exact-session R17 MPEG-generation factory after resolving
  the selected Pi MPEG product profile and existing producer profile;
- `rfb_relay.py` — the accepted R10 provider-neutral bounded raw-RFB courier.
  It owns only a supplied connected provider socket, RFB credit, and one finite
  PS2->provider queue; it never reads from or writes to the PS2-facing Wire
  socket;
- `rfb_attachment.py` — the R13 one-Wire-Session RFB attachment/lifecycle
  mechanism. It owns lazy nonblocking connection to the selected Pi-local
  `127.0.0.1:5900` provider, explicit injected flow limits, composition of the
  R10 Relay only after connect success, RFB-local provider failure, and ordered
  REQUEST -> BOUNDARY -> COMMIT -> COMPLETE retirement;
- `rfb_runtime_profile.py` and `rfb_runtime_profile_generated.py` — R14 narrow
  Pi projection of the single canonical selected RFB profile;
- `mpeg_runtime_profile.py` and `mpeg_runtime_profile_generated.py` — R17
  narrow Pi projection of Configuration's already-selected A003 MPEG limits and
  frame rate; it adds no independent Pi tuning;
- `mpeg_product_profile.py` — R25 composition-only product authority for
  native display `:0`, exact 704x462 desktop bounds and the grounded 8.0-second
  retirement deadline; it owns none of R17's channel/buffer/frame-rate tuning;
- `mpeg_generation.py` — R17 exact-generation START/RETIRE owner, capture-plan
  builder, suppression-footprint preparation, bounded FFmpeg stdout owner,
  session-scoped channel-4 credit bookkeeping, and emission/retirement fence.
  It never reads or writes the PS2-facing socket;
- `SYMBOLS.md` — local clean product symbol authority.

An attachment is inert when constructed. The first exact nonzero channel-1
CREDIT is the only R13 lazy provider-start edge. A healthy idle ACTIVE Wire
Session therefore does not connect or start the RFB provider.

The R13 quiesce mechanism stops new provider reads at BOUNDARY, drains or
locally fails already accepted provider writes, retires provider I/O before
COMMIT, and treats COMPLETE as an RFB-attachment stop rather than a Wire-Session
stop. One attachment is permanently session-scoped and is never rebound to a
replacement Wire Session.

The corrective R13 wake is also session-scoped. Each attachment owns one private
nonblocking socketpair used only to interrupt the sole Wire owner's readiness
wait after `request_quiesce()` publishes REQUEST intent. The requester never
sends Wire bytes or advances Wire sequence; `WireConnectionOwner` drains the
local wake and remains the only serializer of REQUEST. Both wake descriptors are
retired on attachment stop/failure/session close, and a replacement Wire Session
constructs fresh wake state.

`127.0.0.1:5903` remains development/operator tooling and is not product
provider authority. The mature product provider remains exactly the R12
internal loopback `127.0.0.1:5900` endpoint. R15 does not resurrect the
historical direct `192.168.50.1:5900` path as ordinary product composition.

## R14 shared RFB runtime profile projection

A003 R14 adds configuration authority without changing R13 lifecycle mechanics.
`src/config/rfb_runtime_profile.json` is the single selected numeric RFB
profile. `scripts/generate-rfb-runtime-profile.py` deterministically projects
that record into checked-in C and Python constants and `--check` fails when a
projection is stale.

On the Pi, `rfb_runtime_profile.py` maps semantic ON onto exactly one
`RfbFlowConfig`. Both `provider_read_credit_limit` and
`provider_write_capacity` derive from the same canonical 32768-byte window;
`max_data_payload` derives from the same canonical profile. Semantic OFF
returns no flow configuration rather than a zero-valued running attachment.

The selected values preserve CP2N's hardware-qualified historical RFB-only
baseline and CP2J's recorded starting-profile values. Equality of the new R13
provider-write capacity to that 32768-byte window does **not** independently
hardware-qualify the Pi provider-write direction. R14 is configuration/source
authority only.

## R15 ordinary RFB product composition

R15 activates RFB selection at the normal product composition seam without
moving physical Wire or provider lifecycle ownership. The tracked systemd unit
runs `wire_runtime.py`; that module resolves the selected R14 Pi projection
before constructing the listener and passes a fresh-attachment factory into the
existing `WireServer`. The server itself remains the sole Q4, physical recv/send,
sequence, and sequential-session owner.

The current selected R14 profile therefore supplies
`provider_read_credit_limit=32768`, `provider_write_capacity=32768`, and
`max_data_payload=8192` to each fresh R13 attachment. Those values are not
hardcoded in `wire_runtime.py` or the systemd unit. Merely accepting TCP,
receiving HELLO, or publishing ACTIVE still leaves the provider untouched; the
first valid nonzero RFB CREDIT remains the only provider-connect edge.

`WireServer.serve_connection()` closes the current attachment before its
sequential listener may own a replacement connection, so a later Session B gets
fresh attachment, wake, Relay, credit, and quiesce state. R15 adds no provider retry/backoff, no multi-run restart policy, and no Wire
CONFIG frame. R17 later adds the dormant MPEG mechanism seam described below;
the ordinary `wire_runtime.py` composition still does not activate it.

`scripts/pi/install-wire-runtime.sh` stages, verifies, syntax-checks, and removes
the R14 Pi profile projection files plus the R15 composition runtime alongside
the accepted Wire/RFB mechanism files. The tool retains its fail-closed inactive
service fence and performs no daemon-reload, enable/disable, start/stop/restart,
or provider/display mutation.

R15 is repository/source/host/build authority only. The ordinary composed Pi
service has not been live-activated or physically PS2/Pi-qualified in this
packet.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15`.


## R17 Pi MPEG generation-control and producer mechanism

R17 mirrors the accepted PS2 MPEG control ABI exactly: START is kind 11 on
control channel 0 with a 44-byte version-1 payload, RETIRE is kind 10 on control
channel 0 with a 12-byte version-1 payload, and encoded MPEG DATA uses channel 4.
The fixed PSTV header remains version 1 and Q4 product establishment remains
version 2.

`MpegGenerationController` owns one nonzero generation at a time under one
already-ACTIVE Wire Session. START must match that session, advance the local
generation monotonically, and describe a 16-pixel-aligned base capture rectangle
inside current injected desktop geometry. The outer suppression rectangle is
validated separately and must contain the base. Inner matte remains PS2-local.

The controller prepares an exact capture command and exact suppression footprint
before opening emission admission. `BufferedMpegProducer` owns one FFmpeg
process, reader thread, and Configuration-bounded stdout buffer. R17 deliberately
does not copy historical H1 bitrate/GOP/VBV tuning: only values already selected
by A003 Configuration are projected from
`src/config/mpeg_runtime_profile.json`.

Channel-4 CREDIT is Wire-Session authority, matching the accepted PS2 Transport
model that grants the initial MPEG window once and returns consumed/discarded
bytes. Generation-local bytes, producer identity, suppression state and emission
leases never transfer to a successor generation. Every physical MPEG frame still
passes through `WireConnectionOwner`, which owns global sequence allocation and
the only PS2-facing send call.

RETIRE first closes new generation emission, waits for any already-admitted
local send lease to leave the physical-send call, then requires real producer
process/thread retirement and exact suppression-footprint retirement. A timeout
can detect failure but can never manufacture retirement success. Only after
those proofs does the Wire owner serialize the exact RETIRE completion and let
the controller return to IDLE.

At the R17 acceptance point these maintained modules were staged while the
ordinary `wire_runtime.py` deliberately supplied no MPEG generation factory.
R25 later activates that already-accepted factory seam without changing R17
mechanisms. PS2 permanent trigger/main-loop activation, active-MPEG
recalibration, AUDIO, heartbeat and final all-guns composition remain outside
R17/R25.

Context: `docs/development/mpeg-generation-control.md`;
`docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-PI-MPEG-CONTROL-PRODUCER-R17`.


## R25 ordinary MPEG product composition

R25 adds one narrow `mpeg_product_profile.py` authority and uses it only from
ordinary `wire_runtime.py` to construct the accepted R17 owner after exact Q4
ACTIVE.

The selected composition values are:

- display `:0`;
- desktop `704x462`;
- retirement deadline `8.0` seconds.

The deadline is grounded in the frozen H1 exact-generation producer default at
forensic commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`; it is a failure
deadline, never a success condition. Existing R17 Configuration-owned producer
profile values are unchanged.

Both RFB and MPEG factories are now present in the ordinary composed server.
RFB remains lazy on its own CREDIT edge. The MPEG controller is created only
after Q4 ACTIVE, and its producer remains absent until exact START. CREDIT by
itself only records Wire-session channel capacity.

One Wire Session receives one fresh MPEG controller and may run successive
generations through that controller's monotonic high-water fence. The next Wire
Session receives a distinct controller. All channel-4 DATA and RETIRE-completion
frames still flow only through `WireConnectionOwner`.

Session finish requires `MpegGenerationController.close()` to prove local
retirement. A close/factory failure makes that physical session failed rather
than causing an in-place rebind or RFB-only fallback.

R25 host tests use fake producer ownership only. They do not launch real FFmpeg,
touch the live provider/display, or mutate systemd state. PS2 ordinary MPEG
trigger/main-loop activation remains deferred.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-PI-MPEG-ORDINARY-PRODUCT-COMPOSITION-R25`.
