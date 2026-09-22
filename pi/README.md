# Raspberry Pi product source

DIRECTORY=pi
GENERATION=CLEAN_RECONSTRUCTION
OWNER=RASPBERRY_PI_COMPANION_RUNTIME

This is the maintained product-source root for custom Raspberry Pi companion
runtime code. It is distinct from:

- `src/`, which is the PlayStation 2 executable source tree;
- `scripts/pi/`, which is provisioning/staging/development tooling;
- `experiments/`, which is evidence and apparatus rather than product runtime.

Current R15 ownership:

- `wire_protocol.py` — exact product Wire framing, Q4 establishment, and exact
  logical RFB DATA/CREDIT representation;
- `wire_server.py` — persistent Pi listener, provisional establishment,
  process-local session IDs, the sole accepted physical Wire recv/send and
  sequence owner, plus optional composition of one explicitly supplied
  session-scoped RFB attachment;
- `wire_runtime.py` — R15 ordinary product composition. It consumes the R14
  selected RFB flow and supplies `WireServer` with a factory that creates one
  fresh R13 attachment per physical Wire connection. Semantic OFF supplies no
  factory, so the same server remains establishment-only;
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
fresh attachment, wake, Relay, credit, and quiesce state. R15 adds no provider
retry/backoff, no multi-run restart policy, no Wire CONFIG frame, and no
AUDIO/MPEG activation.

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
