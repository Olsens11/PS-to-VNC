# Ledge A003 — Manual Transport / Pi Completion

DOCUMENT=LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION
DOCUMENT_REVISION=0003
RECORDED_AT=2026-09-17T16:59:51-04:00
SOURCE_COMMIT=SELF
BASED_ON_DOCUMENT_REVISION=0002
BASED_ON_A003_AUDIT_REVISION=0001
BASED_ON_ARCHITECTURE_OVERLAY_REVISION=0003
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0001
BASED_ON_FOREMAN_STATE_REVISION=0018
TEMPORAL_CLASS=MANUAL_WORK_ITEM
TEMPORAL_SEMANTICS=GOVERNING_UNTIL_SUPERSEDED
STATUS=A003_TRANSPORT_PI_MANUAL_COMPLETION_ACTIVE
ACTIVE_PACKET=A003-P2A-MANUAL-START-PREPARED-R1
WORK_ITEM_KEY=a003-mpeg-generation
ROLE_KEY=reconstruction
WORKER_KEY=interactive
EXECUTION_MODE=USER_ASSISTED_MANUAL
EXECUTION_SEAT=/home/ps2/src/PS-to-VNC-ledge-manual
NORMAL_COMMAND_PATH=ps2run-interactive

Revision 0003 preserves the active bounded A003 manual engineering slice and incorporates the user-assisted Wire runtime decisions recorded in `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision `0001`. It supersedes revision 0002 only by making those runtime ownership/lifetime decisions explicit authority for the active packet. It does **not** declare A003 complete, does not authorize later producer/retirement work, and does not change the accepted A003 wire/control representation.

The manual session's execution-seat gate has been satisfied at branch authority `b7c9e3575987876672bb28d74d31053177b5f71b`: a fresh standalone checkout exists at `/home/ps2/src/PS-to-VNC-ledge-manual`; the older `/home/ps2/src/PS-to-VNC` checkout was left untouched; branch identity and clean state were confirmed; ordinary filesystem and Git-index operations passed; `scripts/resume-state.sh` passed; `scripts/check.sh` passed; post-check state was clean; and `git push --dry-run origin HEAD:refs/heads/ledge/h1-all-guns` passed. This evidence opens the manual execution route. It does not itself satisfy any A003 product criterion.

## Accepted A003 behavior preserved

The following accepted behavior remains binding and must not be reworked absent a concrete defect:

- A003-P1 Transport MPEG logical-channel and decoder-owner core remains accepted;
- START is explicit kind `11`, control channel `0`, flags `0`;
- START v1 has the exact accepted 44-byte representation;
- RETIRE is explicit kind `10`, control channel `0`, flags `0`;
- RETIRE has the exact accepted 12-byte big-endian `(version, session_id, generation)` representation;
- MPEG media remains opaque `DATA/channel 4`;
- no payload-length sniffing may classify START;
- no per-MPEG-packet generation tags may be introduced;
- START-shaped exactly-44-byte `DATA/channel 4` remains ordinary MPEG media;
- the accepted PS2 outbound START framing remains on the existing ordered physical Transport send path;
- generation business state must not be moved into Transport or the MPEG decoder.

Current Foreman P2A disposition remains:

- `P2A-1 START_CONTROL_IDENTITY = MET`;
- `P2A-2 START_SEMANTICS = PARTIAL / ACTIVE_MANUAL_SLICE`;
- `P2A-3 RETIRE_WIRE_IDENTITY = MET`;
- `P2A-4 PURE_MPEG_DATA = MET`;
- `P2A-5 SOLE_TRANSPORT_OWNER = PARTIAL / ACTIVE_MANUAL_SLICE`;
- `P2A-6 OWNERSHIP = PARTIAL / ACTIVE_MANUAL_SLICE`;
- `P2A-7 NO_GENERATION_TAGGING = MET`;
- `P2A-8 BOUNDED_SCOPE = MET`.

A003 as a whole remains incomplete.

## Binding architecture rule

Architecture overlay revision `0003` remains binding:

> Within one genuine component/local-cooperation directory, internal implementation files may cooperate directly. Across a real component boundary, communication must pass through the owning component's defined bridge or public process seam.

Transport owns mechanism: one physical PSTV session/receive chain per endpoint, framing, ordered receive/send, dispatch, flow control, quiescence, mechanism failure, and narrow publication of explicit control identity.

Transport does **not** own exact-generation business state. The exact-generation owner owns active-session validation, nonzero/monotonic generation rules, one immutable prepared START value, rejection of stale/repeated/conflicting START without mutation, and the `GENERATION_PREPARED` fact.

The historical `pi/pstv/` name is architectural vocabulary, not a preselected production path. The active manual session must first inspect the real present-day Pi runtime/service ownership and then place/adopt code according to genuine ownership.


### Settled Wire runtime decisions from the user-assisted discussion

`docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision `0001` is now binding for
the runtime owner/lifetime portion of this packet.

The evidence/discussion has settled these facts:

- the mature Pi endpoint is a dedicated, continuously running **Wire server**
  installed as part of the Pi-side product bundle;
- systemd starts/supervises that server, while the server itself owns its
  listener, accepted physical connection, Wire Session, sole receive path,
  Wire framing/multiplexing, ordered send, Transport flow-control mechanism,
  and Transport-level connection state;
- every PS-to-VNC application byte on the physical PS2↔Pi TCP connection is a
  Wire Protocol frame for the entire connection lifetime; no rider may bypass
  the mux or temporarily become the physical protocol;
- RFB is a provider-neutral logical rider through a local RFB adapter and does
  not own Wire lifetime;
- an established Wire Session is valid while idle; no RFB, PCM/audio, MPEG,
  heartbeat, or other rider traffic is required merely to keep the healthy
  session open;
- the Wire Session is prerequisite infrastructure for PS2↔Pi cooperative
  functions, while no individual cooperative rider is a prerequisite for the
  Wire Session;
- local-only PS2 ELF behavior remains outside this dependency and must not be
  artificially disabled merely because Wire is unavailable.

The historical/current direct-RFB/audio service topology and H1 coordinator
shape remain evidence, not automatic production structure.

This discussion settles the **runtime owner and lifetime model**. It does not by
itself settle the final clean source-directory/file placement. Before
behavior-bearing production source/runtime writes, the active manual session
must still inspect the clean source topology and deliberately choose the
smallest owner-correct production placement rather than transplanting H1
scaffolding.

## Active bounded packet — A003-P2A-MANUAL-START-PREPARED-R1

PACKET_STATUS=ACTIVE
ASSIGNING_BASE_HEAD=b7c9e3575987876672bb28d74d31053177b5f71b
ASSIGNING_AUDIT=LEDGE_AUDIT_A003_MPEG_GENERATION.md:0001
ASSIGNING_ARCHITECTURE_OVERLAY=LEDGE_ARCHITECTURE_OVERLAY.md:0003
ASSIGNING_MANUAL_AUTHORITY=LEDGE_A003_TRANSPORT_PI_MANUAL_COMPLETION.md:0002

### Objective

Complete only the first owner-correct real-Pi portion of A003 P2A: establish the actual present-day Pi PSTV ownership from evidence, agree the production placement with the user, make exactly one real Pi PSTV session/receive owner, route the already-accepted exact START control through that sole receiver to the exact-generation semantic owner, and implement one immutable prepared-generation START state with exact rejection/no-mutation behavior.

### Phase M1A — read-only real-Pi ownership archaeology

This phase is authorized immediately.

Inspect the **actual running Pi** and repository/runtime/service environment needed to determine present-day ownership, including as relevant:

- running PSTV/VNC/network-related processes and parent/launch relationships;
- listening/established sockets and which process owns the PS2-facing PSTV path;
- systemd system/user units, service definitions, enablement, dependencies, and launch scripts;
- active VNC/Xvnc/TigerVNC/Openbox/session ownership and configuration that supplies the PS2-facing desktop/session;
- NetworkManager/network/service topology that materially affects the PSTV process boundary;
- existing production Pi companion code, scripts, service/config files, and retained H1 evidence needed to map responsibility;
- current checkout/runtime paths actually used by the running system.

M1A is **read-only** with respect to product source and production runtime/service state. Do not restart, replace, enable/disable, edit, install, or migrate the running owner merely to discover it.

### Mandatory user discussion gate

Before choosing the production owner/path or making behavior-bearing production/runtime writes, present the ownership evidence to the user and discuss the viable owner boundary. The production placement decision must be evidence-led and user-assisted.

Do not treat an old proposed `pi/pstv/` directory name, historical H1 shape, or caller convenience as the decision by itself.

### Phase M1B — authorized implementation after the discussion gate

After the ownership evidence has been discussed with the user and the production owner/path is explicitly settled, the interactive Reconstruction worker may:

1. adopt or implement **one real Pi PSTV session owner** for the one accepted PS2-facing physical PSTV stream;
2. establish and prove that this owner is the **sole Pi physical receive owner** for that session;
3. receive/classify START only by the accepted explicit control identity: kind `11`, control channel `0`, flags `0`, exact START-v1 representation;
4. publish START from the Transport/session mechanism owner through the narrow owner seam to the exact-generation semantic owner;
5. keep generation business state outside Transport;
6. implement exact START semantic validation against immutable current session/desktop facts;
7. create exactly one immutable prepared-generation value on valid START;
8. reject invalid/wrong-session/zero/stale/repeated/conflicting START without mutating already accepted prepared state;
9. add behavior-specific tests and owner-local documentation/synopses needed to prove this slice;
10. make only the smallest production service/config/source changes actually required by the adopted owner.

If a genuine new clean component/local-cooperation directory is established, the worker may create its behavior-bearing files, local documentation, and local `SYMBOLS.md`. Canonical/global topology registration, generated dictionary portal reconciliation, generic build/test registration, and other non-behavioral integration chores remain the next Foreman baton unless a current repository rule makes a particular local registration inseparable from creating a valid owner.

### Exact START semantic obligations in this slice

The exact-generation semantic owner must validate at least:

- START v1 exact representation/version;
- exact active session id;
- nonzero generation;
- monotonic/stale-generation authority;
- every geometry word fitting the implementation's signed-use range before signed conversion/use;
- base width and height each at least 16 pixels and 16-pixel aligned;
- positive suppression width and height;
- suppression rectangle fully containing the exact base rectangle;
- base and suppression rectangles inside immutable active Pi desktop/session bounds;
- overflow-safe right/bottom arithmetic.

Successful START stores one immutable prepared value containing the accepted session id, generation, exact base rectangle, and suppression rectangle. Once prepared, malformed, wrong-session, zero, stale, repeated, or conflicting START must fail without changing that value. No producer is activated by preparation in this packet.

Active desktop bounds must come from the actual Pi desktop/session owner as immutable session context. They must not be hardcoded from the current qualified 704x462 value, derived from START, or confused with PS2 DESKTOP CALIBRATION.

### Acceptance criteria for this packet

- `M1-C1 ACTUAL_PI_OWNERSHIP_EVIDENCE`: running Pi PSTV/VNC/network/service ownership is inspected and the production ownership evidence is discussed with the user before the path decision.
- `M1-C2 PI_SOLE_SESSION_OWNER`: one real Pi PSTV session owner owns the one accepted socket/session and sole physical receive chain; no competing reader/socket is introduced.
- `M1-C3 EXPLICIT_START_RECEIVE`: accepted exact START control is received/classified through that sole owner without DATA/channel-4 sniffing.
- `M1-C4 OWNER_SPLIT`: Transport/session mechanism publishes START through its narrow seam; exact-generation semantic state lives with the exact-generation owner, not Transport and not MPEG.
- `M1-C5 IMMUTABLE_PREPARED_GENERATION`: one valid START produces exactly one immutable prepared generation after the required semantic validation.
- `M1-C6 REJECTION_NO_MUTATION`: malformed, wrong-session, zero, stale, repeated, and conflicting START are rejected without mutating accepted prepared state.
- `M1-C7 BOUNDED_SCOPE`: no producer activation/emission, retirement fence/completion, PS2 decoder finalization, successor-generation lifecycle, A004-A006 work, or hardware-completion claim is introduced.

The worker must return exact source/runtime files changed, commands/tests/evidence, the user-discussed owner decision, criterion disposition, and the next exact blocker/baton point.

## Explicit non-goals — still unauthorized

This active packet does **not** authorize:

- MPEG producer activation or MPEG emission;
- exact-generation producer preparation beyond the immutable START state needed by this slice;
- emission admission/open-close behavior;
- retirement send-lease or in-flight emission fencing;
- RETIRE completion/acknowledgment lifecycle;
- PS2 RETIRE completion handling beyond already accepted wire facts;
- PS2 decoder stop/join or worker-resource finalization;
- residual MPEG queue discard or Transport credit return;
- generation `N -> RETIRE(N) -> N+1` completion;
- repeated-generation lifecycle completion;
- A004, A005, or A006 product work;
- declaration that A003 is complete;
- independent Validation PASS;
- cross-machine or hardware qualification claims beyond evidence actually obtained.

RETIRE kind `10` / channel `0` / flags `0` and its exact 12-byte wire identity remain preserved facts, but this slice does not implement retirement semantics.

## Required authority before M1A/M1B work

At the start/resume of the manual Reconstruction shift, read/re-read at minimum:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/status.md`;
- `docs/README.md`;
- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/development/README.md`;
- `docs/development/source-naming-and-symbols.md` before product source changes;
- `docs/development/source-topology.md` before creating/moving product directories/files;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision `0005`;
- `docs/ledge/work-log/README.md` revision `0006`;
- current `docs/ledge/LEDGE_FOREMAN_STATE.md`;
- this document revision `0002`;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision `0003`;
- `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision `0001`;
- `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision `0001`;
- newest relevant `a003-mpeg-generation` work logs, especially the partial Reconstruction handoff and the Foreman deferral/activation history.

Refresh `ledge/h1-all-guns`, inspect the actual local checkout state, and rerun/confirm the normal development-seat preflight before behavior-bearing writes. Preserve any newly discovered local work; do not reset/clean/discard it.

## Stable shift/log identity

This is a continuation of the existing A003 engineering task. The manual interactive session uses:

- `ROLE_KEY=reconstruction`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`.

Capture a fresh truthful exact `STARTED_AT` for the manual Reconstruction shift. Its eventual immutable log path is therefore:

`docs/ledge/work-log/<started-at-safe>__reconstruction__a003-mpeg-generation__interactive.md`

Do not reuse a historical timestamp.

## Remaining manual completion sequence after this packet

Only sequence steps 1-5 are activated by `A003-P2A-MANUAL-START-PREPARED-R1`. The later sequence remains deferred until a subsequent Foreman packet:

6. prove exact-generation producer preparation;
7. prove MPEG emission;
8. prove emission admission closes at retirement;
9. deliberately exercise an in-flight emission lease during retirement;
10. prove exact producer cleanup completes before RETIRE completion is emitted;
11. prove PS2 exact RETIRE completion reception through the sole PS2 receiver/public seam;
12. prove PS2 decoder stop/join without reclaiming worker-visible resources early;
13. prove residual MPEG queue discard and corresponding Transport credit return;
14. prove generation N cannot contaminate generation N+1 across the ordered same-stream fence;
15. prove repeated generations and finite cancel/error/EOF/retirement behavior;
16. integrate later reconstructed A004-A006 owners through their public bridges;
17. proceed into real PS2/Pi cross-machine and hardware qualification under separate evidence authority.

## Completion gate

This manual item must still close before A003 can be declared complete, before all-guns integration can claim the real exact-generation transaction, and before hardware qualification can close. Completion of this first packet may advance only the exact P2A portions directly proven by its evidence; every later owner/lifecycle fact remains explicitly pending.
