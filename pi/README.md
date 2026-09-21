# Raspberry Pi product source

DIRECTORY=pi
GENERATION=CLEAN_RECONSTRUCTION
OWNER=RASPBERRY_PI_COMPANION_RUNTIME

This is the maintained product-source root for custom Raspberry Pi companion
runtime code. It is distinct from:

- `src/`, which is the PlayStation 2 executable source tree;
- `scripts/pi/`, which is provisioning/staging/development tooling;
- `experiments/`, which is evidence and apparatus rather than product runtime.

Current R8 ownership:

- `wire_protocol.py` — exact product Wire framing and provisional
  HELLO/ACCEPT/NOT_ACCEPTED representation;
- `wire_server.py` — persistent Pi listener, provisional establishment,
  process-local session IDs, one accepted physical I/O owner, and repeated
  finite session containment;
- `SYMBOLS.md` — local clean product symbol authority.

R8 intentionally has no RFB/AUDIO/MPEG/CONFIG rider implementation and does not
own the current direct-RFB provider lifecycle.

Context: `docs/ledge/LEDGE_FOREMAN_STATE.md`,
`A003-PI-WIRE-SERVER-ESTABLISHMENT-R8`.
