# Ledge All-Guns Architecture Overlay

DOCUMENT=LEDGE_ARCHITECTURE_OVERLAY
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T10:29:00-04:00
TEMPORAL_CLASS=ARCHITECTURE_POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_LEDGE_ARCHITECTURE_AT_RECORDED_TIME
BASE_ARCHITECTURE=docs/CLEAN_ARCHITECTURE.md:ARCHITECTURE_VERSION_1
SCOPE=ledge/h1-all-guns

This overlay is the governing architecture amendment for the ledge all-guns reconstruction. `docs/CLEAN_ARCHITECTURE.md` version 1 remains the primary clean-generation architecture except where this overlay explicitly supersedes it for the ledge branch. The overlay exists because the proven H1 all-guns behavior introduces one shared physical PSTV transport carrying logical channels, a behavior that postdates the pre-media clean architecture.

Earlier architecture remains historical evidence. This overlay does not project the new transport model backward into work completed under architecture version 1.

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

## Source-shape guidance

A dedicated transport component/directory is architecturally permitted and expected if it is the smallest coherent owner of the responsibilities above. Its cross-component operations must obey the ledge reconstruction contract: one component bridge body organized into coherent process sections, with internal files cooperating directly where clearer.

This overlay authorizes A001 source reconstruction only. It does not declare A001 implemented, validated, build-qualified, PT_LOAD-qualified, or hardware-qualified.

## Validation obligations

Architecture reconciliation clears only validation finding V001's prerequisite gate. Reconstruction and validation must still prove the A001 obligations already recorded by audit/validation, including one physical stream, sole receiver, framing/sequence/error behavior, logical RFB queue/fragment/residual/credit behavior, disabled-channel lifecycle, repeated sessions, explicit receiver-dispatch quiescence, RFB operation over the logical stream, architecture/include/symbol documentation checks, reproducible build evidence, and PS2 qualification after PT_LOAD-changing integration.
