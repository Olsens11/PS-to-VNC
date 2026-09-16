# Ledge Validation — Findings

DOCUMENT=LEDGE_VALIDATION_FINDINGS
DOCUMENT_REVISION=0004
RECORDED_AT=2026-09-15T23:21:03-04:00
TEMPORAL_CLASS=APPEND_ONLY_FINDING_REGISTER

Findings are validation-owned. They describe evidence and required disposition; they do not redesign product behavior or rewrite audit/reconstruction history.

## V001 — A001 source validation blocked by primary architecture contradiction

SEVERITY=GATE
STATUS=RESOLVED
AFFECTED_TRANCHE=A001
AFFECTED_RECONSTRUCTION_STATE=0001
RESOLVED_BY=docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md:0001

Evidence at opening:

- semantic audit revision 0002 marked A001 transport/RFB responsibilities `RECONSTRUCTION_READY` and required one physical PSTV socket, one sole receiver, logical-channel dispatch, serialized send, and an explicit logical RFB byte-stream boundary;
- reconstruction state revision 0001 correctly identified that no product-source reconstruction could begin without reconciling that ownership against the primary architecture;
- `docs/CLEAN_ARCHITECTURE.md` version 1 remained `PRIMARY_ARCHITECTURE_AUTHORITY=YES` and assigned all RFB writes/requests and receive orchestration to the main/application thread while describing RFB as depending directly on socket/PS2IP facilities;
- that reconstruction shift changed documentation only; there was therefore no A001 reconstructed source tranche to test or mark PASS.

Resolution evidence:

Governance revision `LEDGE_ARCHITECTURE_OVERLAY` 0001 explicitly supersedes the conflicting architecture-version-1 socket ownership for `ledge/h1-all-guns`, establishes one PSTV transport owner and sole receiver, makes RFB consume a logical byte stream, preserves application policy ownership, and carries forward the complete A001 validation obligations. Reconstruction state 0002 consumed that authority and began the transport interface without recreating a competing physical socket.

Validation disposition:

`RESOLVED` for the architecture prerequisite only. This does not validate A001 behavior or hardware.

## V002 — Reconstruction lane respected source/authority boundary

SEVERITY=INFO
STATUS=PASS
AFFECTED_TRANCHE=A001_GATE_HANDLING

Evidence:

The reconstruction shift did not modify forensic H1 source, reconstructed product source, audit records, or validation records. Its two reconstruction-owned continuity documents explicitly preserve the semantic requirements while refusing to create a competing transport architecture.

Validation disposition:

`PASS` for gate handling and lane ownership only. This is not behavioral validation of A001 and is not hardware qualification.

## V003 — A001 interface/topology tranche is intentionally incomplete

SEVERITY=INFO
STATUS=OPEN
AFFECTED_TRANCHE=A001
AFFECTED_RECONSTRUCTION_STATE=0002

Evidence at opening and current disposition remain as previously recorded: A001 requires a coherent implementation, integration, host checks/builds, and exact source/build identity before validation-ready handoff.

Validation disposition:

`WAIT_FOR_COHERENT_TRANCHE`.

## V004 — Transport symbol dictionary is not yet complete enough for a clean-source PASS

SEVERITY=INFO
STATUS=OPEN
AFFECTED_TRANCHE=A001
AFFECTED_RECONSTRUCTION_STATE=0004

Evidence at opening and current disposition remain as previously recorded: definition-level `src/transport/SYMBOLS.md` coverage plus topology/portal/build integration and strict gates are required before `VALIDATION_READY`.

Validation disposition:

`REQUIRED_BEFORE_VALIDATION_READY`.

## V005 — Application fatal teardown can wait for receiver completion without initiating convergence

SEVERITY=HIGH
STATUS=OPEN
AFFECTED_TRANCHE=A001
AFFECTED_COMMIT=fe07a9c22b684a7bd2b9d59a24b7bc6ab328ac27
AFFECTED_FILES=src/app.c;src/transport/bridge.h
AFFECTED_PROCESS=application/Transport fatal-session teardown
OWNER=reconstruction

Evidence:

- after successful `pstvnc_transport_session_open()`, `src/app.c` sets `transport_session_active = 1` and enters the logical RFB/application path;
- every later application/RFB/input/presentation failure converges on `fail:`;
- when Transport is active, that cleanup path calls `pstvnc_transport_session_wait_receiver_done()` first and calls `pstvnc_transport_session_close()` only if the wait reports completion;
- the public Transport bridge exposes wait-for-receiver-done and close, but no application-requested stop/abort operation that can cause a healthy receiver blocked in physical receive to terminate;
- RFB finite-session quiescence is server REQUEST -> RFB-owned safe BOUNDARY -> COMMIT -> residual discard -> COMPLETE. It is not an application-local fatal-abort primitive and cannot be assumed to occur merely because presentation/input/RFB application processing failed;
- therefore an application-local failure while the physical connection/receiver remains healthy can enter a wait whose completion condition has not been caused by that failure path. This risks an indefinite teardown stall and does not yet prove the required first-failure/fail-closed convergence or repeated-session lifecycle.

Required disposition:

Reconstruction must provide an authority-consistent fatal-session convergence path that preserves one physical receiver, receiver-completion-before-reclaim, RFB safe-boundary ownership where finite quiescence applies, and the inherited H1 receive-poison boundary. Do not mask the issue with a guessed timeout or unsafe descriptor close. Add behavior-specific evidence showing that an application-local post-adoption failure cannot wait forever for a receiver that was never asked/caused to finish. Foreman should route this as product-behavior work to Reconstruction A/B, not as an integration chore.

Validation disposition:

`BLOCKS_VALIDATION_READY` until source behavior and evidence demonstrate deterministic fatal-session convergence without violating the sole-receiver/quiescence contract.
