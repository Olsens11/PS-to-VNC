# Ledge All-Guns Architecture Overlay

DOCUMENT=LEDGE_ARCHITECTURE_OVERLAY
DOCUMENT_REVISION=0005
RECORDED_AT=2026-09-21T19:38:46.622-04:00
SOURCE_COMMIT=SELF
BASED_ON_DOCUMENT_REVISION=0004
SUPERSEDES_DOCUMENT_REVISION=0004
BASED_ON_WIRE_RUNTIME_DECISIONS_REVISION=0011
BASED_ON_RECONCILIATION_REVISION=0001
TEMPORAL_CLASS=ARCHITECTURE_POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_LEDGE_ARCHITECTURE_AT_RECORDED_TIME
BASE_ARCHITECTURE=docs/CLEAN_ARCHITECTURE.md:ARCHITECTURE_VERSION_1
SCOPE=ledge/h1-all-guns

This overlay is the current ledge amendment to the clean architecture. Revision
0005 records the provisional R13 reconstructed mechanism for one session-scoped
Pi RFB attachment and its finite quiesce lifecycle pending independent Foreman
acceptance. Revision 0004 remains historical in Git; where its pre-R13
attachment wording conflicts with this revision or
`LEDGE_WIRE_RUNTIME_DECISIONS.md` revision 0011, the later authority governs.

## Physical Wire ownership

Wire Transport owns one physical PS2↔Pi product connection/session, the sole
physical receive path, framing/multiplexing, ordered physical send, Transport
flow-control bookkeeping, connection/session validity, and mechanism failure.
RFB, PCM/audio, MPEG, control and future riders use logical Wire Channels and may
not own or bypass the physical socket.

The mature Pi product contains a product-owned Wire server. A Wire Session is one
successfully established physical Wire lifetime. It may remain completely idle.
Connection loss permanently ends that session; reconnect establishes a new Wire
Session and never resumes the dead one.

Connection establishment is provisional until compatibility establishment
succeeds. Wire Protocol begins immediately, ACTIVE is published only after
successful establishment, and the Pi assigns authoritative session identity.

As of A003 R8/R9, this Q4 establishment boundary exists in maintained product
source on both peers. The Pi product server owns provisional accept and
authoritative session-ID allocation; PS2 Transport owns HELLO/result handling,
descriptor transfer, private current-session identity, and the public
INACTIVE/ACTIVE availability fact. Successful establishment consumes sequence 1
independently in each direction and preserves the same physical lineage at
sequence 2 for later rider activation.

This is source/host/build authority, not physical qualification of the final
product path. The earlier Q1-Q12 proofs remain hardware evidence and reference
provenance rather than substitute qualification for the R8/R9 product source.

## Wire Channel Relay boundary

Logical riders attach through Transport-owned Wire Channel Relays. Relay payload
is domain-opaque. Transport may own bounded capacity, credit, readiness,
consumption/release accounting and channel-local mechanism failure, but it does
not acquire rider-domain semantics merely because it moves those bytes.

A stalled or failed rider does not by itself make Wire unhealthy. Failure
containment begins at the smallest owner that can prove a safe state and
escalates only when necessary.

As of A003 R10, maintained product source contains the first concrete
provider-neutral RFB Relay on both sides of that boundary. The Pi relay owns an
explicitly injected local provider socket, finite provider-write storage, and
RFB-specific credit only; the Pi Wire connection owner alone performs physical
Wire recv/send. Provider reads stop at zero PS2 credit, provider writes are
nonblocking/readiness-driven, and Pi capacity is re-granted only after bytes
actually leave the queue. On PS2, outbound RFB DATA is released only against
Pi-granted channel-1 CREDIT through an RFB-specific wait/counter, while the sole
Transport I/O thread remains the only physical Wire sender.

The installed Pi Wire service still auto-attaches no provider, and R10 does not
migrate or replace the qualified direct-RFB deployment. Zero-length channel-1
DATA remains reserved for the existing finite quiesce lifecycle; full product
quiesce orchestration and provider-selection policy remain later work.

As of A003 R11, the selected **direct-RFB provider authority** is separately
reproducible: the existing LightDM/Xorg `:0` desktop is exposed by
`/usr/bin/X0tigervnc` through the unchanged generic systemd-owned
`192.168.50.1:5900` socket. An additive provider-service drop-in owns this
selection; it clears the historically qualified Xtigervnc `:1` ExecStart and
uses X0tigervnc native socket activation with `-rfbport -1`. The historical
dedicated `:1` unit remains evidence/control authority rather than being
rewritten. The `127.0.0.1:5903` development listener is not product
architecture.

R11 still does not attach that selected provider to the R10 Wire Relay.

As of A003 R12, the selected provider-side endpoint is no longer the direct
PS2-facing socket. Maintained authority now defines:

    127.0.0.1:5900
        -> ps-to-vnc-rfb-internal.socket
        -> ps-to-vnc-rfb-internal-x0tigervnc.service
        -> X0tigervnc
        -> existing LightDM/Xorg :0

That loopback listener is Pi-local provider infrastructure for a future bounded
R10 Relay connector. It is neither a Wire Protocol endpoint nor a second
physical PS2 connection. The R12 units own bidirectional systemd conflict plus
ordering dependencies against the preserved direct socket/provider/persistent
control, making the direct and internal alternatives mutually exclusive without
rewriting R11 evidence.

The R11 `192.168.50.1:5900` route remains direct-RFB qualification/fallback
authority, and `127.0.0.1:5903` remains development-only. R12 still does not
attach the R10 Relay or implement quiesce/provider-failure orchestration.

As of A003 R13, maintained Pi product source adds one session-scoped RFB
attachment mechanism around the R10 Relay. Merely reaching Wire ACTIVE is inert:
the first exact nonzero channel-1 CREDIT is the bounded lazy-start edge for one
nonblocking connection attempt to the selected `127.0.0.1:5900` provider.
Finite provider-read credit, provider-write capacity, and maximum DATA payload
are explicit injected configuration, not Pi daemon defaults. The R10 Relay is
created only after provider connect succeeds.

The Wire connection owner remains the sole PS2-facing recv/send and global
sequence owner. Provider failure is RFB-local and does not by itself retire
Wire; the attachment has no reconnect/retry/backoff mechanism and may never be
rebound to another Wire Session. A later Wire Session receives fresh attachment,
credit, and quiesce state.

Zero-length channel-1 DATA now has the strict attachment-local lifecycle:

    REQUEST -> BOUNDARY -> COMMIT -> COMPLETE

REQUEST is an explicit mechanism seam, not Application ON/OFF policy. Provider
reads may continue after REQUEST but stop immediately when BOUNDARY is accepted.
Already accepted provider writes then drain or fail locally; provider I/O is
fully retired before COMMIT. No ordinary attachment DATA/CREDIT is emitted after
COMMIT. COMPLETE stops only the attachment and leaves the containing Wire
Session ACTIVE. Duplicate or out-of-order lifecycle markers fail at the
smallest safe RFB scope.

The default installed Wire service still supplies no attachment factory or
flow profile, so R13 does not create unvalidated service auto-attachment,
CONFIG delivery, Application RFB orchestration, AUDIO/MPEG ownership, or a new
physical product connection. R13 is source/host/project/build authority, not a
live Pi or physical qualification claim.

## One physical-I/O execution context

The current architecture requires one Transport-owned physical framed-I/O
execution context. Hardware characterization showed the tested split-I/O
execution stalled while the single owner completed the transaction. That is the
claim boundary; it is **not** proof that PS2IP is generally non-thread-safe.

On PS2, the Transport owner must cooperatively relinquish EE execution often
enough during idle polling for networking progress. The tested 1000-us delay is
a hardware-supported implementation baseline, not Wire ABI and not a declared
final scheduling mechanism or optimum.

## Wire availability and module lifecycle

Wire exposes only the containing communication fact:

    INACTIVE
    ACTIVE(session_id)

ACTIVE means cross-Wire communication is available. It does not mean every
module is ready or should start.

Each cross-Wire module owns its own start/readiness/stop/failure lifecycle and
must completely retire its old running instance before replacement startup or
reuse of module-owned live resources. Wire loss is a stop condition for the
affected cross-Wire behavior.

Transport authority issued to a module instance is valid only for the Wire
Session under which it was created. Old Session-A authority cannot become
Session-B authority, cannot consume B inbound state, and cannot send through B.
Transport owns that communication-validity fence. Transport does **not** own a
generic module-retirement manager or project-wide admitted-call drain.

If a module cannot prove complete local retirement, that module may add the
smallest module-specific identity/fencing its actual asynchronous behavior earns.
See `docs/development/module-lifecycle.md`.

## Wire Session identity versus MPEG generation identity

Do not collapse these two dimensions.

Wire Session identity fences work across physical connections.

MPEG generation/run identity fences stale MPEG-run work across MPEG activations
inside a Wire Session. Multiple MPEG generations may occur inside one Wire
Session. An MPEG generation cannot survive loss of its owning Wire Session.
Reconnect does not silently restart MPEG.

Generation identity remains legitimate runtime/run fencing; Q12 does not remove
it. Q6 does remove the older assumption that "generation" itself is the
user-facing product meaning of MPEG activation.

## MPEG activation and retirement

MPEG activation is an Application/domain lifecycle transaction. The MPEG region
is durable domain meaning; manual MPEG calibration is one current source of that
region definition, not the definition of MPEG behavior itself.

On MPEG stop:

1. close admission of **new** MPEG production first;
2. already accepted work may safely drain under its owning run;
3. release the RFB suppression constraint so RFB restoration can begin
   underneath while valid retiring MPEG presentation remains visible;
4. continue module-owned MPEG retirement to a safe final boundary;
5. remove MPEG/mattes only when safe, revealing the already-refreshing RFB state.

This ordering explicitly supersedes older A003/A004 wording that required
complete local MPEG retirement before RFB restoration could even begin.
Historical audits/logs remain immutable evidence of the earlier understanding.

No old MPEG bytes, callbacks, decoder completion or retirement completion may
contaminate a later MPEG generation. Wire-session fencing and MPEG-generation
fencing are independent protections.

## Geometry and calibration ownership

Persistent Pi configuration owns calibrated desktop/display geometry. The
runtime desktop/session owner publishes one immutable active geometry snapshot.
The normal product desktop target is the existing Raspberry Pi LightDM/Xorg
desktop `:0`. R12 selects socket-activated X0tigervnc behind the Pi-local
provider endpoint `127.0.0.1:5900`; the future Wire Relay connector will
consume that local endpoint. The direct `192.168.50.1:5900` route is preserved
fallback/qualification evidence rather than the mature provider endpoint.

The separate `127.0.0.1:5903` path is Windows/operator development tooling,
not product architecture. The staged dedicated Openbox/lxpanel `:1` candidate
is historical/evaluation evidence and is not the current product target.

DESKTOP CALIBRATION and MPEG CALIBRATION remain distinct. MPEG calibration owns
its base rectangle, inner matte and outer/suppression footprint semantics.
Desktop geometry is not inferred from MPEG START, and inner matte remains
presentation-local rather than Pi capture geometry.

## Existing component ownership retained

RFB owns RFB protocol/session behavior, parser/framebuffer truth, safe
complete-message freeze/quiesce and fresh-full-update obligations through its
public bridge.

MPEG owns decoder/worker behavior and decode/upload readiness, including
module-local retirement.

Presentation owns one physical GS/composition path, resolved draw geometry,
matte/suppression rendering, visible ownership, physical presentation boundary
and common-clock arm callsite.

Application owns cross-domain product transactions and recovery policy through
public owner seams. It does not become the physical Wire mechanism owner.

Input/UI owns controller/keyboard/local-foreground semantics and does not own
Wire, RFB protocol, MPEG decode or Presentation internals.

## Evidence boundaries

The Q1-Q12 hardware detour supplies architecture evidence, not blanket product
qualification:

- Proof 1A: establishment/session identity and completely idle session worked;
- Proof 1B: incompatible establishment was rejected and never became ACTIVE;
- Proof 2: same PS2 process survived loss and established a distinct new session;
- Proof 3: RFB relay capacity/credit/backpressure and consumption-gated credit worked;
- Proof 4: single physical-I/O owner completed the tested transaction where the
  split execution stalled; root-cause PS2IP thread safety remains NOT PROVEN;
- Proof 4C: RFB and PCM were independent logical riders and PCM failure remained local;
- Proof 4I plus productized requalification: cooperative idle scheduling and
  single-I/O product Transport source worked in the bounded hardware workload;
- Proof 5/Q12: stale A outbound authority did not enter B, stale A inbound access
  did not consume B payload, and fresh B authority remained usable.

The unexecuted MPEG Proof 6 apparatus is not product or hardware authority.

## Qualification debt is not a source blocker

Source reconstruction may proceed under the settled ownership/lifecycle
contract without pretending hardware qualification exists. Preserve distinct
evidence states such as SOURCE_COMPLETE, HOST_TESTED, PS2_COMPILE_PASS,
PS2_LINK_PASS, MACHINE_EVIDENCE, OPERATOR_OBSERVED and HARDWARE_PROVEN.

Remaining physical qualification includes final product Q4 establishment, exact
product Wire service/runtime, reconstructed module lifecycle, MPEG one-run and
repeated-generation behavior, stale generation N vs N+1, Wire loss during MPEG,
Q7 overlapped RFB restoration, presentation-visible handoff, all-guns endurance
and exact product ELF qualification.

## Source reuse rule

The hardware-proof branch is reference authority, not a merge target. Reuse
product-shaped work only through bounded Reconstruction packets or
Foreman-owned documentation/evidence integration. Proof harnesses and the
rejected generic Transport admitted-call drain are not promoted into product
merely because they exist.
