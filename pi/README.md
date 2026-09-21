# Raspberry Pi product source

DIRECTORY=pi
GENERATION=CLEAN_RECONSTRUCTION
OWNER=RASPBERRY_PI_COMPANION_RUNTIME

This is the maintained product-source root for custom Raspberry Pi companion
runtime code. It is distinct from:

- `src/`, which is the PlayStation 2 executable source tree;
- `scripts/pi/`, which is provisioning/staging/development tooling;
- `experiments/`, which is evidence and apparatus rather than product runtime.

Current R10 ownership:

- `wire_protocol.py` — exact product Wire framing, Q4 establishment, and exact
  logical RFB DATA/CREDIT representation;
- `wire_server.py` — persistent Pi listener, provisional establishment,
  process-local session IDs, the sole accepted physical Wire I/O owner, and
  optional composition of an explicitly supplied session-scoped RFB relay;
- `rfb_relay.py` — provider-neutral bounded raw-RFB courier. It owns only the
  supplied local provider socket, PS2-granted provider-read credit, Pi-granted
  provider-write capacity, and one finite PS2->provider queue. It never reads
  from or writes to the PS2-facing Wire socket;
- `SYMBOLS.md` — local clean product symbol authority.

The ordinary installed `ps-to-vnc-wire.service` remains establishment-only:
`serve_forever()` supplies no provider attachment and R10 adds no automatic
provider selection, endpoint migration, or direct-RFB service replacement.
A later authorized owner may compose `RfbRelay` with an explicit provider
socket without changing which object owns physical Wire I/O.

R10 implements only the raw bidirectional RFB Relay core. It does not implement
full RFB quiesce lifecycle orchestration, CONFIG delivery, Application RFB
startup, RFB presentation/input policy, AUDIO/MPEG riders, MPEG producer policy,
or reconnect/backoff.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-PI-RFB-WIRE-RELAY-R10`.
