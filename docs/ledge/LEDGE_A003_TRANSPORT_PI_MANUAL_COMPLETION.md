# Ledge A003 — Manual Transport / Pi Completion

DOCUMENT=LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION
DOCUMENT_REVISION=0004
RECORDED_AT=2026-09-20T15:54:52-04:00
SOURCE_COMMIT=SELF
BASED_ON_DOCUMENT_REVISION=0003
SUPERSEDES_DOCUMENT_REVISION=0003
BASED_ON_A003_AUDIT_REVISION=0001
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0004
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=MANUAL_WORK_ITEM
TEMPORAL_SEMANTICS=GOVERNING_UNTIL_SUPERSEDED
STATUS=SUPERSEDED_RECONSTRUCTION_RESUMED
ACTIVE_PACKET=NONE
WORK_ITEM_KEY=a003-mpeg-generation

The former `A003-P2A-MANUAL-START-PREPARED-R1` packet is no longer active.
Its owner-archaeology purpose was consumed by the Q1-Q12 decision/hardware
detour. Clean reconstruction is explicitly resumed under the current Foreman
state and `LEDGE_Q1_Q12_RECONCILIATION.md`.

This revision does not declare A003 complete. It removes the obsolete
user-assisted manual gate and preserves the still-valid A003 product and
qualification obligations under the settled architecture.

## Accepted Wire representation remains preserved

Absent a separate wire-format decision, preserve the accepted A003 protocol
facts:

- START kind 11 / control channel 0 / flags 0;
- exact accepted 44-byte START v1 representation;
- RETIRE kind 10 / control channel 0 / flags 0;
- exact accepted 12-byte RETIRE representation;
- MPEG media is opaque DATA/channel 4;
- no payload-length sniffing to classify START;
- no per-MPEG-packet generation tags;
- START-shaped 44-byte DATA/channel-4 payload remains ordinary MPEG media;
- accepted outbound control/media ordering remains Transport-owned.

## Superseded manual assumptions

The old packet required the worker to rediscover the Pi owner, discuss the owner
with the user before behavior-bearing writes, and model a durable
"exact-generation semantic owner" around preparation. Those requirements are
superseded where they conflict with Q1-Q12.

The mature authority now establishes the Pi product-owned Wire server, Wire
Session lifetime, Wire Channel Relay boundary, establishment contract, module
lifecycle split, reconnect semantics and stale-session Transport fencing.

MPEG generation identity remains available as runtime/run fencing between MPEG
runs. It is distinct from Wire Session identity and is not eliminated by Q12.
It is also not the user-facing definition of MPEG activation.

## A003 current ownership

Wire Transport owns physical connection/session validity, sole physical framed
I/O, Wire framing/mux, ordered send, channel capacity/credit/readiness and stale
Transport authority.

Application owns MPEG activation/deactivation transactions through public owner
seams.

MPEG owns decoder/run behavior and complete local retirement. Presentation owns
visible MPEG/RFB handoff. RFB owns protocol/framebuffer/request behavior.

Each affected module must completely retire its old instance before replacement
startup/resource reuse. Do not add a generic Transport admitted-call drain to
compensate for module retirement.

## Current Q7 retirement ordering

The older A003 sequence that required complete producer/decoder/local retirement
before RFB restoration begins is superseded.

Current order is: close new MPEG production/admission; allow valid accepted work
to drain; begin RFB restoration underneath while valid retiring MPEG remains
visible; then complete MPEG/presentation retirement and reveal the refreshed RFB
state. Old-run/new-run isolation remains mandatory.

## Remaining A003 work

A003 still needs clean product implementation/integration and deterministic
validation for the settled architecture, including:

- product Transport single physical-I/O owner and hardware-supported cooperative
  idle scheduling;
- session-bound opaque Transport access for cross-Wire modules;
- final product establishment seam and Pi Wire-server owner shape;
- channel-relay integration through module public boundaries;
- MPEG one-run activation/retirement structure and generation fencing;
- repeated MPEG runs, stale N vs N+1 isolation and Wire-loss handling;
- Q7 overlapped RFB restoration and presentation-visible handoff;
- application orchestration through public bridges.

The current bounded packet is published by `LEDGE_FOREMAN_STATE.md`; do not
resume the historical M1A/M1B packet from Git history.

## Evidence / qualification boundary

The hardware-proof branch provides bounded hardware evidence for establishment
mechanics, reconnect, relay/credit behavior, the single-I/O scheduling
invariant, rider failure containment and Q12 Transport stale-access fencing.
That evidence does not hardware-qualify the reconstructed exact product ELF or
the still-unimplemented final product Q4 establishment path.

Remaining hardware work is qualification debt, not permission to keep unrelated
source reconstruction frozen.
