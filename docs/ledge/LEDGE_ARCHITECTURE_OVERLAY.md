# Ledge All-Guns Architecture Overlay

DOCUMENT=LEDGE_ARCHITECTURE_OVERLAY
DOCUMENT_REVISION=0002
RECORDED_AT=2026-09-17T06:38:54-04:00
BASED_ON_DOCUMENT_REVISION=0001
SUPERSEDES_DOCUMENT_REVISION=0001
TEMPORAL_CLASS=ARCHITECTURE_POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_LEDGE_ARCHITECTURE_AT_RECORDED_TIME
BASE_ARCHITECTURE=docs/CLEAN_ARCHITECTURE.md:ARCHITECTURE_VERSION_1
SCOPE=ledge/h1-all-guns

This overlay is the governing architecture amendment for the ledge all-guns reconstruction. `docs/CLEAN_ARCHITECTURE.md` version 1 remains the primary clean-generation architecture except where this overlay explicitly supersedes it for the ledge branch. The overlay exists because the proven H1 all-guns behavior introduces one shared physical PSTV transport carrying logical channels, a behavior that postdates the pre-media clean architecture.

Earlier architecture remains historical evidence. This overlay does not project the new transport model backward into work completed under architecture version 1.

Revision 0002 retains the A001 PS2 transport ownership below and additionally designates the clean production Pi counterpart required by A003 exact-generation control reconstruction. Historical `experiments/media-harness-h1/H1Session.reader()` remains forensic evidence, not production source.

## A001 transport ownership reconciliation

For the ledge all-guns generation, supersede the architecture-version-1 statements that make the application/main thread the sole owner of VNC socket operations and that make RFB depend directly on socket/PS2IP facilities.

Adopt one explicit **PSTV transport owner** with these responsibilities:

- own the one physical PS2-facing PSTV TCP socket;
- own the sole physical receive path;
- validate physical PSTV framing and sequence state;
- dispatch received payloads into transport-owned logical-channel state;
- serialize all physical PSTV sends;
- own transport/channel lifecycle and explicit receiver-dispatch/quiescence state;
- expose narrow logical-channel operations to protocol/media components without exposing the physical socket as their authority.

The application coordinator still owns product-level startup/shutdown ordering, recovery policy, and cross-domain decisions. It requests transport lifecycle operations; it does not become the physical socket mechanism owner.

## RFB ownership under shared transport

RFB continues to own the synchronized RFB protocol session, handshake/security/ServerInit, exact RFB receive buffering/framing, encoding negotiation, requests, server-message parsing, rectangle decoding, and pointer/key wire serialization.

When the shared PSTV transport is active, RFB consumes an explicit logical RFB byte-stream boundary supplied through its component bridge. RFB does **not** own or open a second physical PS2-facing socket and does not depend directly on PS2IP socket facilities for that path.

The historical H1 build-time symbol-renaming/global-bind adapter is not architecture. Reconstruction may replace it with the explicit logical byte-stream interface approved by semantic audit A001.

## Concurrency invariant

The essential all-guns concurrency rule is now:

> Exactly one transport receive owner advances the physical PSTV stream; protocol/media components consume logical-channel state and may not independently receive from the physical socket.

The application remains the sole owner of cross-domain product side effects. Controller/input threads still may not perform RFB or transport socket operations.

A complete RFB-message boundary remains the safe ordinary application-service boundary. Hard replacement/recovery policy remains application-owned.

## Shutdown/quiescence invariant

A001 requires proof that a received logical RFB frame has completed receiver dispatch before transport/media teardown can retire the resources it may still touch. The H1 counter-equality plus bounded `DelayThread()` fence is evidence of this required ordering, not the required production mechanism.

The clean implementation must represent receiver dispatch/quiescence explicitly. Diagnostic counters/witnesses may observe this state but must not be the synchronization authority.

## Dependency direction amendment

For the ledge all-guns generation, conceptually:

    application -> transport
    application -> rfb
    rfb -> transport logical RFB stream boundary
    transport -> socket/PS2IP facilities

Future audited PCM/MPEG logical channels may depend on the same transport owner only after their semantic tranches become reconstruction-ready. This amendment does not pre-authorize unaudited media behavior.

Prohibited shortcuts include:

- RFB, PCM, or MPEG opening a competing PS2-facing PSTV socket;
- multiple physical receive owners;
- logical-channel consumers reaching through transport to raw socket state;
- transport making product recovery/policy decisions;
- diagnostic counters becoming required synchronization state.

## A003 clean production Pi endpoint ownership

The clean production Pi endpoint is a **Pi PSTV Session** component. Its clean runtime source namespace is `pi/pstv/`. This namespace is the production successor boundary for the transport/session responsibilities historically exercised by H1 `H1Session.reader()`; the historical experiment itself remains read-only forensic authority.

The Pi PSTV Session owns mechanism only:

- one accepted PS2-facing PSTV TCP connection for the session;
- the Pi endpoint's sole PSTV receive chain and receive sequence;
- PSTV header framing/version/length validation;
- ordered physical sends and the Pi endpoint's send sequence;
- explicit frame-kind/channel/flags dispatch;
- Transport-level control publication and ordinary logical-channel transport facts;
- connection/session teardown at the mechanism boundary.

It does **not** own exact-generation business state, MPEG producer lifecycle, MPEG decoder state, presentation/calibration policy, or DESKTOP CALIBRATION.

The Pi PSTV Session publishes exact-generation control through one narrow typed seam to a separate internal owner, the **Pi Exact-Generation Coordinator**. The seam publishes the already-classified control identity (`START` or `RETIRE`) plus the exact control payload and immutable session context. The session owner must not infer START from MPEG DATA payload bytes or length and must not inspect MPEG media to decide generation semantics.

Within the clean Pi runtime, `pi/pstv/` is one local cooperation boundary: the session owner and exact-generation coordinator may be separate modules/objects inside that component and may call through the narrow typed control seam directly. This avoids inventing an extra generic bridge framework merely to separate two owners that cooperate within one endpoint process. Later Pi MPEG producer work may receive explicit generation commands from the coordinator, but that producer lifecycle is not authorized by this architecture revision or by A003-P2A.

### Pi Exact-Generation Coordinator

The Pi Exact-Generation Coordinator owns exact-generation/application semantics:

- the exact active PSTV session identity supplied at session creation;
- the highest generation accepted/retained for stale-generation prevention;
- the optional one-and-only immutable prepared START request;
- exact START v1 semantic decoding/validation after Transport envelope classification;
- nonzero generation enforcement;
- signed-range conversion for geometry words used by the implementation;
- MPEG CALIBRATION / MPEG presentation base-rectangle minimum/alignment validation;
- positive suppression geometry and containment of the exact base rectangle;
- base and suppression bounds against immutable active Pi desktop geometry;
- repeated, stale, wrong-session, malformed, and conflicting START rejection without mutating already-accepted prepared state;
- publication of a semantically accepted prepared generation to later lifecycle work through an explicit owner seam.

P2A does not authorize producer activation, suppression/capture mutation, emission admission, retirement cleanup/acknowledgment, successor reopening, or the complete `N -> RETIRE(N) -> N+1` transaction.

RETIRE control is published through the same typed session-to-coordinator seam. P2A may establish its exact wire identity and publication path, but RETIRE's producer-cleanup/acknowledgment lifecycle remains deferred to later Foreman packets.

### Active Pi desktop geometry authority

START does not establish its own bounds authority. The coordinator receives one immutable **active Pi desktop bounds** value when the PSTV session/generation coordinator is created.

That value comes from the Pi companion's active PS2-facing desktop/session owner — the same runtime/configuration authority that creates or adopts the active Xtigervnc desktop — and is Pi-local session metadata. Historical H1 `desktop_width` / `desktop_height` are evidence of this ownership. The current 704x462 desktop is a qualified/candidate session value, not a permanent architectural maximum and not a constant to bake into exact-generation logic.

This active Pi desktop bounds value is distinct from both START's MPEG geometry and the PS2's **DESKTOP CALIBRATION** system. START's base rectangle and suppression footprint are **MPEG CALIBRATION / MPEG presentation geometry** carried into P2A only as already-accepted generation-preparation inputs. P2A does not reconstruct A004 MPEG calibration behavior.

### Endpoint responsibility split

The governing split is:

    PS2 Transport
        PS2 endpoint framing / one socket / sole receive / logical-channel dispatch

    PS2 MPEG
        decoder / worker behavior only

    Pi PSTV Session (`pi/pstv/`)
        Pi endpoint one socket / sole receive / framing / ordered send / envelope dispatch

    Pi Exact-Generation Coordinator (`pi/pstv/` internal owner)
        START/RETIRE semantic state and immutable prepared generation

    Pi MPEG producer
        later producer preparation / capture / suppression / emission / retirement behavior

No endpoint may borrow the other endpoint's receiver implementation merely because both use the same PSTV wire format. In particular, Pi START receive semantics do not belong in PS2 `src/transport/runtime.c`.

## PS2 RETIRE control publication boundary

The existing PS2 Transport sole receiver remains the only Pi-to-PS2 PSTV receive owner. Because explicit RETIRE completion will later travel Pi -> PS2 as control kind 10/channel 0, clean Transport must eventually publish that already-classified control message through a narrow application-facing control seam rather than treating it as MPEG DATA or giving another owner access to the physical socket.

A003-P2A may reconstruct that receive/publication mechanism only. It must not attach P2C decoder-stop/join/residual-drain/credit or successor-generation lifecycle semantics to the Transport receive path.

## Source-shape guidance

A dedicated PS2 transport component/directory is architecturally permitted and expected if it is the smallest coherent owner of the responsibilities above. Its cross-component operations must obey the ledge reconstruction contract: one component bridge body organized into coherent process sections, with internal files cooperating directly where clearer.

For the Pi endpoint, `pi/pstv/` is the newly designated clean runtime component namespace. Reconstruction should keep PSTV session mechanism and exact-generation semantics as distinct owners inside that one local cooperation boundary. Do not create additional Pi component directories, sockets, readers, or generic indirection layers unless later evidence demonstrates a real independent responsibility.

This overlay authorizes only the ownership boundaries described here. It does not declare P2A implemented, validated, build-qualified, or hardware-qualified, and it does not authorize P2B/P2C/P2D or A004 behavior.

## Validation obligations

Architecture reconciliation clears ownership ambiguity only. Reconstruction and Validation must still prove the applicable A001/A003 obligations, including one physical stream per endpoint, sole receiver, framing/sequence/error behavior, explicit control/data separation, exact START semantic validation and prepared-generation rules, ordinary 44-byte MPEG DATA remaining media, and later retirement/generation lifecycle only when separately authorized. Repository evidence cannot substitute for physical qualification.
