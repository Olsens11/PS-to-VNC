# Ledge Reconstruction Work Log — Interactive A003 Manual Shift

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-17T17:06:40-04:00
COMPLETED_AT=2026-09-17T23:36:56-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=SELF_PAUSED
STARTING_BRANCH_COMMIT=6c416293435203ea77a7afb95db218f3b5a977fe
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=YES

## Objective and authority consumed

Continue the user-assisted A003 manual reconstruction on branch
`ledge/h1-all-guns`, recover the mature Wire runtime and MPEG activation
architecture from repository/runtime evidence plus direct user design intent,
and record only explicitly approved architectural decisions.

Repository authority consumed during the shift included:

- `AGENTS.md` / reconstruction bootstrap and repository rules;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision `0005`;
- `docs/ledge/work-log/README.md` revision `0006`;
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision `0020`;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision `0003`;
- `docs/ledge/LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md` revision `0003`;
- `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision `0001`;
- the canonical Wire terminology and the live branch authority refreshed before
  each repository write.

The shift retained:

- `ROLE_KEY=reconstruction`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=USER_ASSISTED_MANUAL`;
- execution seat `/home/ps2/src/PS-to-VNC-ledge-manual`.

## Work performed

No behavior-bearing production source or production runtime/service state was
changed in this shift.

The shift performed user-assisted architecture recovery and recorded the
approved decisions in
`docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md`.

### Q1 — RFB provider attachment and lifetime

Closed.

Mature rule: RFB is a replaceable Pi-local provider feeding a provider-neutral
adapter into the RFB Wire Channel. RFB does not own or terminate Wire Session
lifetime.

Historical evidence was preserved: pre-mux provider switching at the same
endpoint required PS2VNC reconnect/relaunch and therefore did not provide
connection-lifetime independence.

### Q2 — Pi-side Wire owner and lifetime

Closed.

Mature rule: a dedicated continuously running Pi Wire server, supervised by
systemd, owns the listener, accepted connection, Wire Session, sole physical
receive path, Wire framing/multiplexing, ordered send, Transport flow-control
mechanism, and Transport-level connection state.

Every PS-to-VNC application byte on the physical TCP connection is Wire
Protocol traffic for the entire connection lifetime. An established Wire
Session may remain completely idle and does not depend on RFB, PCM/audio, MPEG,
heartbeat, or another rider for validity.

### Q3 — Wire Transport / rider boundary

Closed.

Canonical term: **Wire Channel Relay**.

Each logical Wire Channel reaches its owning domain through a Transport-owned
Wire Channel Relay. The Relay is payload-opaque and may own Transport
bookkeeping such as bounded capacity, credit/release accounting, readiness, and
backpressure, but may not parse or mutate rider-private domain state.

### Q4 — provisional establishment and session activation

Closed.

TCP accept creates a provisional Wire connection. Application bytes remain Wire
Protocol from byte one. The PS2 initiates a minimal establishment exchange with
Wire Protocol version plus ELF/product version.

The initial version tuple is lightweight accidental-peer identification and
compatibility checking, not strong authentication. The establishment mechanism
is deliberately replaceable.

On acceptance the Pi assigns the authoritative session identity and promotes
the connection to an active Wire Session. On rejection, Transport reports
`NOT_ACCEPTED` upward.

The PS2 owns a generally named **Wire Establishment Policy** seam. Its initial
policy is intentionally only:

`NOT_ACCEPTED -> shutdown`

That is application policy, not a Transport invariant. The seam is explicitly
documented so later retry, update, local-only continuation, or stronger
authentication can be added without redesigning ordinary Wire Transport.

### Q5 — desktop/session geometry authority

Closed.

Persistent Pi configuration owns display modes and calibrated safe-desktop
geometry. The Pi desktop/session owner resolves the selected mode and publishes
an immutable active-geometry snapshot for the active desktop-session state.

The configuration is expected to support multiple display modes, per-mode
safe-desktop dimensions and X/Y offsets, and a known-safe default, currently
intended as 480p.

Future display-mode selection, same-mode calibration, rollback, startup chord,
or first-boot calibration remain future UI/policy. They are not required by the
active A003 packet.

### Q6 — MPEG activation, region definition, calibration, and suppression

Closed.

MPEG START is an internal part of activating MPEG presentation, not an
independent user-facing product operation.

The initial/bootstrap interaction may be `START+SELECT`, but that trigger is
not permanent architecture. Manual MPEG calibration is likewise one initial
source of an MPEG region definition rather than the permanent definition of
MPEG behavior.

The proven calibration model is preserved:

- MPEG width/height remain aligned to a 16x16 coding grid;
- the grid may move at one-pixel X/Y precision for exact visual alignment;
- internal and external matte behavior remains part of the region definition;
- the outer boundary of the external matte defines the RFB suppression region;
- the prior manual MPEG calibration may be persisted in Pi configuration.

Application owns MPEG activation/deactivation coordination through public owner
seams. Wire Transport carries control/media but does not own MPEG-region
semantics.

The architecture intentionally leaves room for future automatic video-region
detection and application-specific/window-pinned MPEG-region contracts without
redesigning MPEG, Wire Transport, or Presentation.

Run/generation identity remains only where concretely useful as old-run/new-run
fencing, stale-completion isolation, and retirement safety. It is not the
product-level definition of MPEG launch.

## Repository changes made

Approved architecture/documentation commits landed during this shift:

- `35fead4357e7a1bfcbbe019455410593d1b00c12`
  — `docs(architecture): record Wire runtime Q1-Q2 decisions`
- `f9ee4720c248748c327ff20a6fcdcdedc4e8479e`
  — `docs(a003): bind Wire runtime Q1-Q2 decisions`
- `d67b7c4de53f5eb6342fc075fdaf3f9cfbfb2f50`
  — `docs(architecture): record Wire Channel Relay decision`
- `3d2337bf19d292f7840956fd4f3227355e9b3f3d`
  — `docs(architecture): record Wire establishment decision`
- `e2badb6422960206479cf62d116e2eb745aa2f5b`
  — `docs(architecture): record desktop geometry authority`
- `224cb4dd8f8d0a88c17caa7e89a4cacb49156b4b`
  — `docs(architecture): record MPEG region activation decision`

The decision ledger is now
`docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision `0005`.

## Evidence / checks

- Branch authority was refreshed before each repository-changing decision write.
- The final pre-log branch tip was
  `224cb4dd8f8d0a88c17caa7e89a4cacb49156b4b`.
- The live Pi/runtime ownership discussion used read-only evidence; no
  production service restart, enable/disable, install, or runtime migration was
  performed by this shift.
- Historical H1/A003 evidence was inspected where needed to distinguish proven
  calibration/decoder mechanisms from generation-centric scaffolding.
- No physical hardware qualification was claimed.
- No product source compile, link, cross-machine, or hardware run was required
  or performed for these documentation-only decisions.

## Authority conflict discovered at shift end

The current Foreman state and active A003 manual packet predate most of the
approved decisions above.

At shift end:

- `LEDGE_FOREMAN_STATE.md` remains revision `0020` and still describes the
  active packet as unchanged under much older Wire-decision authority;
- `LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md` remains revision `0003` and
  binds Wire runtime decisions revision `0001`;
- that manual packet still requires generation-centric semantics including
  exact active-session validation, nonzero/monotonic generation rules, an
  immutable prepared-generation START object, and geometry/suppression
  validation framed as an independent exact-generation semantic owner;
- approved Q6 revision `0005` explicitly narrows those semantics and requires
  reconciliation before behavior-bearing implementation continues.

Therefore a Reconstruction worker must **not blindly resume the old M1B
behavior-bearing packet**.

This is an authority/planning reconciliation blocker, not a missing physical
computer/PS2/Pi action.

## Pending evidence / qualification

PENDING_LOCAL=Foreman must reconcile active A003 packet/state against Wire runtime decisions revision 0005 before new behavior-bearing M1B work.
PENDING_LOCAL=Clean production source placement still requires an owner-correct source-shape census before implementation.
PENDING_CROSS_MACHINE=A003 real PS2/Pi MPEG activation/retirement transaction remains unproven in the reconstructed product.
HARDWARE_PENDING=A003 reconstructed MPEG/video activation, repeated start/stop, retirement isolation, and all-guns behavior remain physically unqualified.

## Exact next pickup

**Foreman first.**

The next Foreman round should:

1. refresh current branch authority and consume this immutable shift log;
2. read `LEDGE_WIRE_RUNTIME_DECISIONS.md` revision `0005`;
3. reconcile `LEDGE_FOREMAN_STATE.md` and
   `LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md` with Q1-Q6;
4. explicitly remove or reframe generation-centric M1B obligations that conflict
   with Q6 while preserving still-useful wire identity, retirement fencing,
   residual-byte isolation, and safety validation;
5. decide the next bounded worker packet from the reconciled architecture;
6. only then release a Reconstruction worker to behavior-bearing source work.

No additional user-side PS2/Pi runtime mutation is required before that
Foreman reconciliation.

After reconciliation, if the next packet requires a real runtime/hardware action
that cannot be performed by the worker environment, the Foreman should name that
exact user-assisted action rather than carrying forward a generic manual gate.
