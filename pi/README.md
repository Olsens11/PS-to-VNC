# Raspberry Pi product source

DIRECTORY=pi
GENERATION=CLEAN_RECONSTRUCTION
OWNER=RASPBERRY_PI_COMPANION_RUNTIME

This is the maintained product-source root for custom Raspberry Pi companion
runtime code. It is distinct from:

- `src/`, which is the PlayStation 2 executable source tree;
- `scripts/pi/`, which is provisioning/staging/development tooling;
- `experiments/`, which is evidence and apparatus rather than product runtime.

Current R13 ownership:

- `wire_protocol.py` — exact product Wire framing, Q4 establishment, and exact
  logical RFB DATA/CREDIT representation;
- `wire_server.py` — persistent Pi listener, provisional establishment,
  process-local session IDs, the sole accepted physical Wire recv/send and
  sequence owner, plus optional composition of one explicitly supplied
  session-scoped RFB attachment;
- `rfb_relay.py` — the accepted R10 provider-neutral bounded raw-RFB courier.
  It owns only a supplied connected provider socket, RFB credit, and one finite
  PS2->provider queue; it never reads from or writes to the PS2-facing Wire
  socket;
- `rfb_attachment.py` — the R13 one-Wire-Session RFB attachment/lifecycle
  mechanism. It owns lazy nonblocking connection to the selected Pi-local
  `127.0.0.1:5900` provider, explicit injected flow limits, composition of the
  R10 Relay only after connect success, RFB-local provider failure, and ordered
  REQUEST -> BOUNDARY -> COMMIT -> COMPLETE retirement;
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

The ordinary installed `ps-to-vnc-wire.service` remains establishment-only:
`serve_forever()` supplies no RFB attachment factory and no flow profile.
R13 therefore adds mechanism and an explicit composition seam without inventing
Application RFB ON/OFF policy, automatic reconnect/backoff, CONFIG delivery, or
service auto-attachment.

`127.0.0.1:5903` remains development/operator tooling and is not product
provider authority. R13 does not alter the PS2 `src/` peer contract and makes
no live Pi or physical PS2/Pi qualification claim.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-PI-RFB-ATTACHMENT-QUIESCE-R13`.
